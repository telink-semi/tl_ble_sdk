/********************************************************************************************************
 * @file    lin.c
 *
 * @brief   This is the source file for tl322x
 *
 * @author  Driver Group
 * @date    2025
 *
 * @par     Copyright (c) 2025, Telink Semiconductor (Shanghai) Co., Ltd. ("TELINK")
 *
 *          Licensed under the Apache License, Version 2.0 (the "License");
 *          you may not use this file except in compliance with the License.
 *          You may obtain a copy of the License at
 *
 *              http://www.apache.org/licenses/LICENSE-2.0
 *
 *          Unless required by applicable law or agreed to in writing, software
 *          distributed under the License is distributed on an "AS IS" BASIS,
 *          WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *          See the License for the specific language governing permissions and
 *          limitations under the License.
 *
 *******************************************************************************************************/
#include "lin.h"
#include "uart.h"
#include "dma.h"

enum
{
    LIN_RX_MODE = 0,
    LIN_TX_MODE = 1,
};

static lin_handle_t s_lin_hw_ctb[LIN_NUM] = {{0}, {0}};

static dma_config_t lin_tx_dma_config[LIN_NUM] =
{
    {
        .dst_req_sel    = DMA_REQ_LIN0_TX,    // tx req
        .src_req_sel    = 0,
        .dst_addr_ctrl  = DMA_ADDR_FIX,
        .src_addr_ctrl  = DMA_ADDR_INCREMENT, // increment
        .dstmode        = DMA_HANDSHAKE_MODE, // handshake
        .srcmode        = DMA_NORMAL_MODE,
        .dstwidth       = DMA_CTR_WORD_WIDTH, // must be word
        .srcwidth       = DMA_CTR_WORD_WIDTH, // must be word
        .src_burst_size = 0,                  // must be 0
        .read_num_en    = 0,
        .priority       = 0,
        .write_num_en   = 0,
        .auto_en        = 0,                  // must be 0
    },

    {
        .dst_req_sel    = DMA_REQ_LIN1_TX,    // tx req
        .src_req_sel    = 0,
        .dst_addr_ctrl  = DMA_ADDR_FIX,
        .src_addr_ctrl  = DMA_ADDR_INCREMENT, // increment
        .dstmode        = DMA_HANDSHAKE_MODE, // handshake
        .srcmode        = DMA_NORMAL_MODE,
        .dstwidth       = DMA_CTR_WORD_WIDTH, // must be word
        .srcwidth       = DMA_CTR_WORD_WIDTH, // must be word
        .src_burst_size = 0,                  // must be 0
        .read_num_en    = 0,
        .priority       = 0,
        .write_num_en   = 0,
        .auto_en        = 0,                  // must be 0
    },
};

static dma_config_t lin_rx_dma_config[LIN_NUM] =
{
    {
        .dst_req_sel    = 0,
        .src_req_sel    = DMA_REQ_LIN0_RX,    // rx req
        .dst_addr_ctrl  = DMA_ADDR_INCREMENT,
        .src_addr_ctrl  = DMA_ADDR_FIX,
        .dstmode        = DMA_NORMAL_MODE,
        .srcmode        = DMA_HANDSHAKE_MODE,
        .dstwidth       = DMA_CTR_WORD_WIDTH, // must be word
        .srcwidth       = DMA_CTR_WORD_WIDTH, // must be word
        .src_burst_size = 0,
        .read_num_en    = 0,
        .priority       = 0,
        .write_num_en   = 0,
        .auto_en        = 0,                  // must be 0
    },

    {
        .dst_req_sel    = 0,
        .src_req_sel    = DMA_REQ_LIN1_RX,   // rx req
        .dst_addr_ctrl  = DMA_ADDR_INCREMENT,
        .src_addr_ctrl  = DMA_ADDR_FIX,
        .dstmode        = DMA_NORMAL_MODE,
        .srcmode        = DMA_HANDSHAKE_MODE,
        .dstwidth       = DMA_CTR_WORD_WIDTH, // must be word
        .srcwidth       = DMA_CTR_WORD_WIDTH, // must be word
        .src_burst_size = 0,
        .read_num_en    = 0,
        .priority       = 0,
        .write_num_en   = 0,
        .auto_en        = 0,                  // must be 0
    }
};

/**********************************************************************************************************************
 *                                           local function                                                           *
 *********************************************************************************************************************/
/**
 * @brief      This function serves to calculate protect id.
 * @param[in]  id - ID value of LIN frame.
 * @return     pid
 */
static unsigned char lin_software_pid(unsigned char id) {
    unsigned char pid = 0;

    pid = (LIN_GET_ID_BIT(id,0) ^ LIN_GET_ID_BIT(id,1) ^LIN_GET_ID_BIT(id,2) ^ LIN_GET_ID_BIT(id,4)) << 6;
    pid |= (~(LIN_GET_ID_BIT(id,1) ^ LIN_GET_ID_BIT(id,3) ^LIN_GET_ID_BIT(id,4) ^ LIN_GET_ID_BIT(id,5)) << 7);
    pid |= (0x3F & id);
    return pid;
}

/**
 * @brief      This function serves to reset LIN controller.
 * @param[in]  lin_num - LIN0/LIN1.
 * @return     none
 */
static void lin_reset(lin_num_e lin_num)
{
    switch (lin_num) {
    case LIN0:
        BM_CLR(reg_clk_en6, FLD_CLK6_LIN0_EN);
        BM_CLR(reg_rst6, FLD_RST6_LIN0);
        BM_SET(reg_rst6, FLD_RST6_LIN0);
        BM_SET(reg_clk_en6, FLD_CLK6_LIN0_EN);
        break;
    case LIN1:
        BM_CLR(reg_clk_en6, FLD_CLK6_LIN1_EN);
        BM_CLR(reg_rst6, FLD_RST6_LIN1);
        BM_SET(reg_rst6, FLD_RST6_LIN1);
        BM_SET(reg_clk_en6, FLD_CLK6_LIN1_EN);
        break;
    default:
        break;
    }
    reg_lin_uart_ctrl1(lin_num) = 0;
    reg_lin_uart_ctrl2(lin_num) = 0;
}

/**
 * @brief      This function serves to initialize UART of LIN controller.
 * @param[in]  lin_num - LIN0/LIN1.
 * @return     none
 */
static void lin_uart_init(lin_num_e lin_num)
{
    if(lin_num >= LIN_NUM){
        return;
    }
    reg_lin_uart_ctrl0(lin_num)   = ((reg_lin_uart_ctrl0(lin_num) & (~FLD_LIN_UART_BPWC_O)) | s_lin_hw_ctb[lin_num].bwpc); // set bwpc
    reg_lin_uart_clk_div(lin_num) = (s_lin_hw_ctb[lin_num].div | FLD_LIN_UART_CLK_DIV_EN);                                 // set div_clock

    reg_lin_uart_ctrl1(lin_num) = ((reg_lin_uart_ctrl1(lin_num) & (~FLD_UART_STOP_SEL)) | 0);
    reg_lin_uart_ctrl1(lin_num) &= ~FLD_LIN_UART_RXTIMEOUT_RTS_EN;
    reg_uart_ctrl4(lin_num) &= ~FLD_LIN_UART_RXDONE_RTS_EN;

    /* 10 = (1 start + 8 data + 1 stop); */
    reg_lin_uart_rx_timeout0(lin_num)     = (s_lin_hw_ctb[lin_num].bwpc + 1) * 10; //1 start + 8 data + 1 stop
    reg_lin_uart_rx_timeout1(lin_num)     = (((reg_lin_uart_rx_timeout1(lin_num)) & (~FLD_LIN_UART_TIMEOUT_MUL)) | 1);
    reg_lin_uart_rxtimeout_o_exp(lin_num) = 0;
}

/**
 * @brief      This function serves to set TXD and RXD of LIN controller.
 * @param[in]  lin_num - LIN0/LIN1.
 * @param[in]  tx_pin - GPIO pin set as TXD of LIN controller.
 * @param[in]  rx_pin - GPIO pin set as RXD of LIN controller.
 * @return     none
 */
static void lin_set_pin(lin_num_e lin_num, gpio_func_pin_e tx_pin, gpio_func_pin_e rx_pin)
{
    // When the pad is configured with mux input and a pull-up resistor is required, gpio_input_en needs to be placed before gpio_function_dis,
    // otherwise first set gpio_input_disable and then call the mux function interface,the mux pad will misread the short low-level timing.confirmed by minghai.20210709.

    if ((tx_pin == GPIO_NONE_PIN) || (rx_pin == GPIO_NONE_PIN)) {
        return;
    }

    gpio_output_en((gpio_pin_e)tx_pin);
    gpio_input_en((gpio_pin_e)rx_pin);
    gpio_set_up_down_res((gpio_pin_e)tx_pin, GPIO_PIN_PULLUP_10K);
    gpio_set_up_down_res((gpio_pin_e)rx_pin, GPIO_PIN_PULLUP_10K);
    switch (lin_num) {
    case LIN0:
        gpio_set_mux_function(tx_pin, LIN0_TX);
        gpio_set_mux_function(rx_pin, LIN0_RX_I);
        break;
    case LIN1:
        gpio_set_mux_function(tx_pin, LIN1_TX);
        gpio_set_mux_function(rx_pin, LIN1_RX_I);
        break;
    default:
        break;
    }
    gpio_function_dis((gpio_pin_e)tx_pin);
    gpio_function_dis((gpio_pin_e)rx_pin);
}

/**
 * @brief      This function serves to set time base of LIN controller.
 * @param[in]  lin_num - LIN0/LIN1.
 * @param[in]  val - LIN_TIME_BASE_5MS/LIN_TIME_BASE_10MS.
 * @return     none
 */
static void lin_set_time_base(lin_num_e lin_num, lin_time_base_e val)
{
    unsigned short cnt     = s_lin_hw_ctb[lin_num].uart_clk / 200 - 1;
    reg_lin_ctrl1(lin_num) = (reg_lin_ctrl1(lin_num) & ~FLD_LIN_T_BASE_SEL_RW) | MASK_VAL(FLD_LIN_T_BASE_SEL_RW, val);
    reg_lin_ctrl2(lin_num) = (reg_lin_ctrl2(lin_num) & ~FLD_LIN_T_BASE_CNT_L_RW) | MASK_VAL(FLD_LIN_T_BASE_CNT_L_RW, cnt & 0xff);
    reg_lin_ctrl3(lin_num) = (reg_lin_ctrl3(lin_num) & ~FLD_LIN_T_BASE_CNT_H_RW) | MASK_VAL(FLD_LIN_T_BASE_CNT_H_RW, (cnt >> 8) & 0xff);
}

/**
 * @brief      This function serves to set timeout of LIN controller.
 * @param[in]  lin_num - LIN0/LIN1.
 * @param[in]  bits - bit number to send or receive.
 * @return     none
 */
static void lin_set_timeout(lin_num_e lin_num, unsigned short bits)
{
    unsigned short cnt     = bits * (s_lin_hw_ctb[lin_num].bwpc + 1) * 14 / 10;
    reg_lin_ctrl3(lin_num) = (reg_lin_ctrl3(lin_num) & ~FLD_LIN_TIMEOUT_CNT_H_RW) | MASK_VAL(FLD_LIN_TIMEOUT_CNT_H_RW, (cnt >> 8) & 0xff);
    reg_lin_ctrl4(lin_num) = (reg_lin_ctrl4(lin_num) & ~FLD_LIN_TIMEOUT_CNT_L_RW) | MASK_VAL(FLD_LIN_TIMEOUT_CNT_L_RW, cnt & 0xff);
}

/**
 * @brief      This function serves to set the waiting time after the beginning of slot.
 * @param[in]  lin_num - LIN0/LIN1.
 * @param[in]  time_us - the time value in microseconds.
 * @note       The maximum value of TX waiting is 30us.
 * @return     none
 */
static void lin_master_set_tx_wait(lin_num_e lin_num, unsigned char time_us)
{
    if (time_us > 30) {
        time_us = 30;
    }
    unsigned char cnt     = s_lin_hw_ctb[lin_num].uart_clk * time_us / 1000 / 1000;
    reg_lin_bus3(lin_num) = (reg_lin_bus3(lin_num) & ~FLD_LIN_COMM_TX_WAIT_CNT_RW) | MASK_VAL(FLD_LIN_COMM_TX_WAIT_CNT_RW, cnt);
}

/**
 * @brief      This function serves to set the time of wake-up signal.
 * @param[in]  lin_num - LIN0/LIN1.
 * @param[in]  time_us - the time value in microseconds.
 * @note       The maximum value of the wake-up signal is 500us.
 * @return     none
 */
static void lin_set_tx_wakeup_time(lin_num_e lin_num, unsigned short time_us)
{
    if (time_us > 500) {
        time_us = 500;
    }
    unsigned char cnt     = s_lin_hw_ctb[lin_num].uart_clk * time_us / 1000 / 1000;
    reg_lin_bus0(lin_num) = (reg_lin_bus0(lin_num) & ~FLD_LIN_TX_WAKE_UP_TIME_RW) | MASK_VAL(FLD_LIN_TX_WAKE_UP_TIME_RW, cnt);
}

/**
 * @brief      This function serves to set the monitoring time of inactive bus.
 * @param[in]  lin_num - LIN0/LIN1.
 * @param[in]  time_ms - the time value in milliseconds.
 * @note       The maximum value of the monitoring time is 10000ms.
 * @return     none
 */
static void lin_set_bus_inactive_time(lin_num_e lin_num, unsigned short time_ms)
{
    if (time_ms > 10000) {
        time_ms = 10000;
    }
    unsigned int cnt      = s_lin_hw_ctb[lin_num].baudrate * time_ms / 1000;
    reg_lin_bus1(lin_num) = (reg_lin_bus1(lin_num) & ~FLD_LIN_BUS_INACTIVE_TIME_CNT_L_RW) | MASK_VAL(FLD_LIN_BUS_INACTIVE_TIME_CNT_L_RW, cnt & FLD_LIN_BUS_INACTIVE_TIME_CNT_L_RW);
    reg_lin_bus2(lin_num) = (reg_lin_bus2(lin_num) & ~FLD_LIN_BUS_INACTIVE_TIME_CNT_M_RW) | MASK_VAL(FLD_LIN_BUS_INACTIVE_TIME_CNT_M_RW, (cnt >> 8) & FLD_LIN_BUS_INACTIVE_TIME_CNT_M_RW);
    reg_lin_bus3(lin_num) = (reg_lin_bus3(lin_num) & ~FLD_LIN_BUS_INACTIVE_TIME_CNT_H_RW) | MASK_VAL(FLD_LIN_BUS_INACTIVE_TIME_CNT_H_RW, (cnt >> 16) & FLD_LIN_BUS_INACTIVE_TIME_CNT_H_RW);
}

/**
 * @brief      This function serves to set the sample position of LIN controller.
 * @param[in]  lin_num - LIN0/LIN1.
 * @param[in]  pos - the sample position of LIN controller.
 * @return     none
 */
static void lin_set_rx_sample_position(lin_num_e lin_num, lin_rx_smp_pos_e pos)
{
    reg_lin_resp1(lin_num) = (reg_lin_resp1(lin_num) & ~FLD_LIN_UART_RX_SMP_POSITION_ADJ_RW) | MASK_VAL(FLD_LIN_UART_RX_SMP_POSITION_ADJ_RW, pos);
}

/**
 * @brief      This function serves to set the enable state of LIN-COMM module in LIN controller.
 * @param[in]  lin_num - LIN0/LIN1.
 * @param[in]  val - the enable state.
 * @return     none
 */
static void lin_set_comm_en(lin_num_e lin_num, bool val)
{
    if (val) {
        reg_lin_ctrl0(lin_num) |= FLD_LIN_COMM_EN_RW;
    } else {
        reg_lin_ctrl0(lin_num) &= ~FLD_LIN_COMM_EN_RW;
    }
}

/**
 * @brief      This function serves to set the enable state of LIN-RESP module in LIN controller.
 * @param[in]  lin_num - LIN0/LIN1.
 * @param[in]  val - the enable state.
 * @return     none
 */
static void lin_set_resp_en(lin_num_e lin_num, bool val)
{
    if (val) {
        reg_lin_ctrl0(lin_num) |= FLD_LIN_RESP_EN_RW;
    } else {
        reg_lin_ctrl0(lin_num) &= ~FLD_LIN_RESP_EN_RW;
    }
}

/**
 * @brief      This function serves to set the enable state of the function of automatic-baudrate.
 * @param[in]  lin_num - LIN0/LIN1.
 * @param[in]  val - the enable state.
 * @return     none
 */
static void lin_hw_set_auto_baud(lin_num_e lin_num, bool val)
{
    if (val) {
        reg_lin_ctrl0(lin_num) |= FLD_LIN_RATE_AUTO_RW;
    } else {
        reg_lin_ctrl0(lin_num) &= ~FLD_LIN_RATE_AUTO_RW;
    }
}

/**
 * @brief      This function serves to set the enable state of the function of setting PID by register.
 * @param[in]  lin_num - LIN0/LIN1.
 * @param[in]  val - the enable state.
 * @return     none
 */
static void lin_set_pid_reg_en(lin_num_e lin_num, bool val)
{
    if (val) {
        reg_lin_ctrl0(lin_num) |= FLD_LIN_PID_REG_EN_RW;
    } else {
        reg_lin_ctrl0(lin_num) &= ~FLD_LIN_PID_REG_EN_RW;
    }
}

/**
 * @brief      This function serves to set the enable state of the function of TX-Back checking.
 * @param[in]  lin_num - LIN0/LIN1.
 * @param[in]  val - the enable state.
 * @return     none
 */
static void lin_set_txback_check_en(lin_num_e lin_num, bool val)
{
    if (val) {
        reg_lin_ctrl0(lin_num) |= FLD_LIN_TXBK_CHECK_EN_RW;
    } else {
        reg_lin_ctrl0(lin_num) &= ~FLD_LIN_TXBK_CHECK_EN_RW;
    }
}

/**
 * @brief      This function serves to set the enable state of the function of TX-Loop.
 * @param[in]  lin_num - LIN0/LIN1.
 * @param[in]  val - the enable state.
 * @return     none
 */
static void lin_set_comm_tx_loop_en(lin_num_e lin_num, bool val)
{
    if (val) {
        reg_lin_ctrl1(lin_num) |= FLD_LIN_COMM_TX_LOOP_RW;
    } else {
        reg_lin_ctrl1(lin_num) &= ~FLD_LIN_COMM_TX_LOOP_RW;
    }
}

/**
 * @brief      This function serves to set the enable state of the function to configure PID used in schedule mode.
 * @param[in]  lin_num - LIN0/LIN1.
 * @param[in]  val - the enable state.
 * @return     none
 */
static void lin_set_schedule_flt_pid_en(lin_num_e lin_num, bool val)
{
    if (val) {
        reg_lin_ctrl1(lin_num) |= FLD_LIN_SCHEDULE_FLT_PID_EN_RW;
    } else {
        reg_lin_ctrl1(lin_num) &= ~FLD_LIN_SCHEDULE_FLT_PID_EN_RW;
    }
}

/**
 * @brief      This function serves to set the enable state of the function to configure slot count used in schedule mode.
 * @param[in]  lin_num - LIN0/LIN1.
 * @param[in]  val - the enable state.
 * @return     none
 */
static void lin_set_schedule_dly_table_en(lin_num_e lin_num, bool val)
{
    if (val) {
        reg_lin_ctrl1(lin_num) |= FLD_LIN_SCHEDULE_DLY_TABLE_EN_RW;
    } else {
        reg_lin_ctrl1(lin_num) &= ~FLD_LIN_SCHEDULE_DLY_TABLE_EN_RW;
    }
}

/**
 * @brief      This function serves to set the length of break-field.
 * @param[in]  lin_num - LIN0/LIN1.
 * @param[in]  len - the length of the break-field.
 * @return     none
 */
static inline void lin_set_break_len(lin_num_e lin_num, lin_break_len_e len)
{
    reg_lin_ctrl1(lin_num) = (reg_lin_ctrl1(lin_num) & ~FLD_LIN_BREAK_LEN_RW) | MASK_VAL(FLD_LIN_BREAK_LEN_RW, len);
}

/**
 * @brief      This function serves to set the length of the delimiter between break-field and sync-field.
 * @param[in]  lin_num - LIN0/LIN1.
 * @param[in]  len - the length of the delimiter.
 * @return     none
 */
static inline void lin_set_break1_len(lin_num_e lin_num, lin_break1_len_e len)
{
    reg_lin_ctrl1(lin_num) = (reg_lin_ctrl1(lin_num) & ~FLD_LIN_BREAK_1_LEN_RW) | MASK_VAL(FLD_LIN_BREAK_1_LEN_RW, len);
}

/**
  * @brief     Configures the LIN tx_dma channel control register.
  * @param[in] lin_num  - LIN0/LIN1.
  * @return    none
 */
static void lin_set_tx_dma_config(lin_num_e lin_num)
{
    dma_config(s_lin_hw_ctb[lin_num].tx_dma_chn, &lin_tx_dma_config[lin_num]);
}

/**
 * @brief      Configures LIN rx_dma channel control register.
 * @param[in]  lin_num  - LIN0/LIN1.
 * @return     none
 */
static void lin_set_rx_dma_config(lin_num_e lin_num)
{
    // no_dma mode: rxdone(timeout) function switch; 1:enable,0:disable;dma mode must disable.
    reg_lin_uart_ctrl0(lin_num) &= ~FLD_UART_NDMA_RXDONE_EN;
    dma_config(s_lin_hw_ctb[lin_num].rx_dma_chn, &lin_rx_dma_config[lin_num]);
}

/**
 * @brief      This function serves to clear the enable-state in PID-filter table.
 * @param[in]  lin_num - LIN0/LIN1.
 * @return     none
 */
static inline void lin_clr_all_flt_pid_en(lin_num_e lin_num)
{
    reg_lin_pid_fil_en(lin_num) = 0x0000;
}

/**
 * @brief      This function serves to set the start index of the schedule table.
 * @param[in]  lin_num - LIN0/LIN1.
 * @param[in]  index - the index value, 0~15.
 * @return     none
 */
static inline void lin_set_schedule_start_idx(lin_num_e lin_num, unsigned char index)
{
    reg_lin_comm1(lin_num) = (reg_lin_comm1(lin_num) & ~FLD_LIN_SCHEDULE_LOOP_START_IDX_RW) | MASK_VAL(FLD_LIN_SCHEDULE_LOOP_START_IDX_RW, index);
}

/**
 * @brief      This function serves to set the end index of the schedule table.
 * @param[in]  lin_num - LIN0/LIN1.
 * @param[in]  index - the index value, 0~15.
 * @return     none
 */
static inline void lin_set_schedule_end_idx(lin_num_e lin_num, unsigned char index)
{
    reg_lin_comm1(lin_num) = (reg_lin_comm1(lin_num) & ~FLD_LIN_SCHEDULE_LOOP_END_IDX_RW) | MASK_VAL(FLD_LIN_SCHEDULE_LOOP_END_IDX_RW, index);
}

/**
 * @brief      This function serves to set the enable state of configuring the filter PID.
 * @param[in]  lin_num - LIN0/LIN1.
 * @param[in]  val - enable state.
 * @return     none
 */
static void lin_set_flt_pid_buf_wr_en(lin_num_e lin_num, bool val)
{
    if (val) {
        reg_lin_flt_pid_ctrl(lin_num) = (reg_lin_flt_pid_ctrl(lin_num) & ~(FLD_LIN_FLT_PID_BUF_CLR_W | FLD_LIN_FLT_PID_BUF_WR_EN_RW | FLD_LIN_SCHEDULE_CLR_W | FLD_LIN_COMM_SCHEDULE_IDX_RST_W)) |
                                        FLD_LIN_FLT_PID_BUF_WR_EN_RW;
    } else {
        reg_lin_flt_pid_ctrl(lin_num) = (reg_lin_flt_pid_ctrl(lin_num) & ~(FLD_LIN_FLT_PID_BUF_CLR_W | FLD_LIN_FLT_PID_BUF_WR_EN_RW | FLD_LIN_SCHEDULE_CLR_W | FLD_LIN_COMM_SCHEDULE_IDX_RST_W)) &
                                        (~FLD_LIN_FLT_PID_BUF_WR_EN_RW);
    }
}

/**
 * @brief      This function serves to select the schedule group.
 * @param[in]  lin_num - LIN0/LIN1.
 * @param[in]  group_index - the group state, 0~3.
 * @return     none
 */
static inline void lin_set_schedule_group_sel(lin_num_e lin_num, unsigned char group_index)
{
    reg_lin_flt_pid_ctrl(lin_num) = (reg_lin_flt_pid_ctrl(lin_num) & ~(FLD_LIN_COMM_SCHEDULE_GRP_SEL_RW | FLD_LIN_FLT_PID_BUF_GRP_SEL_RW)) |
                                    MASK_VAL(FLD_LIN_COMM_SCHEDULE_GRP_SEL_RW, group_index & 3) | MASK_VAL(FLD_LIN_FLT_PID_BUF_GRP_SEL_RW, group_index & 3);
}

/**
 * @brief      This function serves to configure the schedule table.
 * @param[in]  lin_num - LIN0/LIN1.
 * @param[in]  index - the index value in group, 0~3.
 * @param[in]  id - the ID value of frame, 0x00~0x3f.
 * @param[in]  slot_cnt - the period of each frame, count of t-base.
 * @return     none
 */
static void lin_set_schedule_header(lin_num_e lin_num, unsigned char index, unsigned char id, unsigned char slot_cnt)
{
    reg_lin_pid_filter(lin_num, index) = (id & FLD_LIN_FILTERED_FRAME_ID_RW);
    reg_lin_schedule(lin_num, index)   = (slot_cnt - 1) & FLD_LIN_COMM_SCHEDULE_DLY_BUF_RW;
}

/**
 * @brief      This function serves to configure the PID-filter table.
 * @param[in]  lin_num - LIN0/LIN1.
 * @param[in]  index - the index value in group, 0~3.
 * @param[in]  id - the ID value of frame, 0x00~0x3f.
 * @param[in]  data_len - the data length of frame.
 * @param[in]  tx_en - the enable state of TX.
 * @return     none
 */
static void lin_set_filter_pid(lin_num_e lin_num, unsigned char index, unsigned char id, unsigned char data_len, bool tx_enable)
{
    reg_lin_pid_filter(lin_num, index) = (id & FLD_LIN_FILTERED_FRAME_ID_RW) | MASK_VAL(FLD_LIN_FILTERED_PID_TX_RW, tx_enable);
    reg_lin_fit_pid_len(lin_num)       = (reg_lin_fit_pid_len(lin_num) & (~(7 << (4 * index)))) | (((data_len - 1) & 7) << (4 * index));
}

/**
 * @brief      This function serves to set the enable state of each item in PID-filter table.
 * @param[in]  lin_num - LIN0/LIN1.
 * @param[in]  index - index in PID-filter table, 0~15.
 * @return     none
 */
static inline void lin_set_filter_pid_en(lin_num_e lin_num, unsigned char index)
{
    reg_lin_pid_fil_en(lin_num) |= BIT(index & 0xf);
}

/**
 * @brief      This function serves to clear all the PID value configured in PID-filter table.
 * @param[in]  lin_num - LIN0/LIN1.
 * @return     none
 */
static inline void lin_clr_pid_buf(lin_num_e lin_num)
{
    reg_lin_flt_pid_ctrl(lin_num) |= FLD_LIN_FLT_PID_BUF_CLR_W;
}

/**
 * @brief      This function serves to reset the current index of schedule table.
 * @param[in]  lin_num - LIN0/LIN1.
 * @return     none
 */
static inline void lin_rst_schedule_table_cur_idx(lin_num_e lin_num)
{
    reg_lin_flt_pid_ctrl(lin_num) |= FLD_LIN_COMM_SCHEDULE_IDX_RST_W;
}

/**
 * @brief      This function serves to set the enable state to clear all the slot-cnt configured in schedule table.
 * @param[in]  lin_num - LIN0/LIN1.
 * @return     none
 */
static inline void lin_clr_schedule_dly_table(lin_num_e lin_num)
{
    reg_lin_flt_pid_ctrl(lin_num) |= FLD_LIN_COMM_SCHEDULE_IDX_RST_W;
}

/**
 * @brief      This function serves to trigger the action of sending frame header.
 * @param[in]  lin_num - LIN0/LIN1.
 * @return     none
 */
static inline void lin_comm_send_header_act(lin_num_e lin_num)
{
    reg_lin_cmd(lin_num) |= FLD_LIN_COMM_HEADER_ACT_W;
}

/**
 * @brief      This function serves to trigger the action of manually clearing the t-base counter.
 * @param[in]  lin_num - LIN0/LIN1.
 * @return     none
 */
static inline void lin_manual_clr_timebase(lin_num_e lin_num)
{
    reg_lin_cmd(lin_num) |= FLD_LIN_T_BASE_MAN_CLR_W;
}

/**
 * @brief      This function serves to trigger the action of force re-sending header.
 * @param[in]  lin_num - LIN0/LIN1.
 * @return     none
 */
static inline void lin_comm_force_run(lin_num_e lin_num)
{
    reg_lin_cmd(lin_num) |= FLD_LIN_COMM_FORCE_RUN_W;
}

/**
 * @brief      This function serves to trigger the action of manually clearing the t-base counter.
 * @param[in]  lin_num - LIN0/LIN1.
 * @return     none
 */
static inline void lin_tx_wakeup_trig(lin_num_e lin_num)
{
    reg_lin_cmd(lin_num) = (reg_lin_cmd(lin_num) & ~FLD_LIN_TX_WAKE_UP_SIG_W) | MASK_VAL(FLD_LIN_TX_WAKE_UP_SIG_W, 1);
}

/**
 * @brief      This function serves to clear the tx-buffer.
 * @param[in]  lin_num - LIN0/LIN1.
 * @return     none
 */
static inline void lin_uart_clr_tx_buffer(lin_num_e lin_num)
{
    reg_lin_uart_irq(lin_num) = FLD_LIN_UART_TX_BUF_IRQ;
}

/**
 * @brief      This function serves to clear the rx-buffer.
 * @param[in]  lin_num - LIN0/LIN1.
 * @return     none
 */
static inline void lin_uart_clr_rx_buffer(lin_num_e lin_num)
{
    reg_lin_uart_irq(lin_num) = FLD_LIN_UART_RX_BUF_IRQ;
}

/**
 * @brief      This function serves to get the count of receiving.
 * @param[in]  lin_num - LIN0/LIN1.
 * @return     The count value of RX.
 */
static inline unsigned char lin_get_rx_cnt(lin_num_e lin_num)
{
    return reg_lin_uart_buf_cnt(lin_num) &= FLD_LIN_UART_RX_BUF_CNT;
}

/**
 * @brief      This function serves to set the id of frame.
 * @param[in]  lin_num - LIN0/LIN1.
 * @param[in]  pid_reg_en - the enable state of setting PID by register.
 * @return     none.
 */
static inline void lin_set_id(lin_num_e lin_num, unsigned char id, bool pid_reg_en)
{
    lin_set_pid_reg_en(lin_num, pid_reg_en);
    if (pid_reg_en) {
        reg_lin_frame0(lin_num) = (reg_lin_frame0(lin_num) & ~FLD_LIN_PID_VOL) | MASK_VAL(FLD_LIN_PID_VOL, id);
    } else {
        lin_uart_clr_tx_buffer(lin_num);
        reg_lin_data_buf(lin_num, 0) = id;
    }
}

/**
 * @brief      This function serves to get the PID from register.
 * @param[in]  lin_num - LIN0/LIN1.
 * @return     The PID value.
 */
static inline unsigned char lin_get_pid(lin_num_e lin_num)
{
    return reg_lin_frame0(lin_num) & FLD_LIN_PID_VOL;
}

/**
 * @brief      This function serves to set the enable state of the function to put checksum value into RX-FIFO.
 * @param[in]  lin_num - LIN0/LIN1.
 * @param[in]  val - the enable state.
 * @return     none.
 */
static inline void lin_set_checksum_sw_en(lin_num_e lin_num, bool val)
{
    if (val) {
        reg_lin_frame1(lin_num) |= FLD_LIN_CHECKSUM_SW_RW;
    } else {
        reg_lin_frame1(lin_num) &= ~FLD_LIN_CHECKSUM_SW_RW;
    }
}

/**
 * @brief      This function serves to set the checksum value.
 * @param[in]  lin_num - LIN0/LIN1.
 * @param[in]  checksum - the checksum value.
 * @return     none.
 * @note       checksum_sw_enable should be set before using this function.
 */
static inline void lin_set_checksum(lin_num_e lin_num, unsigned char checksum)
{
    reg_lin_frame2(lin_num) = (reg_lin_frame2(lin_num) & ~FLD_LIN_CHECKSUM_VOL) | MASK_VAL(FLD_LIN_CHECKSUM_VOL, checksum);
}

/**
 * @brief      This function serves to get checksum value from register.
 * @param[in]  lin_num - LIN0/LIN1.
 * @return     none.
 */
static inline unsigned char lin_get_checksum(lin_num_e lin_num)
{
    return reg_lin_frame2(lin_num) & FLD_LIN_CHECKSUM_VOL;
}

/**
 * @brief      This function serves to set the enable state of PID-discard.
 * @param[in]  lin_num - LIN0/LIN1.
 * @return     none.
 */
static void lin_set_pid_discard_en(lin_num_e lin_num, bool val)
{
    if (val) {
        reg_lin_resp1(lin_num) |= FLD_LIN_NONFLT_PID_DISCARD_EN_RW;
    } else {
        reg_lin_resp1(lin_num) &= (~FLD_LIN_NONFLT_PID_DISCARD_EN_RW);
    }
}

/**
 * @brief      This function serves to set as tx-mode in response period.
 * @param[in]  lin_num - LIN0/LIN1.
 * @return     none.
 */
static inline void lin_set_tx_mode(lin_num_e lin_num)
{
    reg_lin_frame1(lin_num) = (reg_lin_frame1(lin_num) & ~FLD_LIN_FRAME_DIRECTION_VOL) | MASK_VAL(FLD_LIN_FRAME_DIRECTION_VOL, LIN_TX_MODE);
}

/**
 * @brief      This function serves to set as rx-mode in response period.
 * @param[in]  lin_num - LIN0/LIN1.
 * @return     none.
 */
static inline void lin_set_rx_mode(lin_num_e lin_num)
{
    reg_lin_frame1(lin_num) = (reg_lin_frame1(lin_num) & ~FLD_LIN_FRAME_DIRECTION_VOL) | MASK_VAL(FLD_LIN_FRAME_DIRECTION_VOL, LIN_RX_MODE);
}

/**
 * @brief      This function serves to set the data length of frame.
 * @param[in]  lin_num - LIN0/LIN1.
 * @param[in]  len - data length.
 * @return     none.
 */
static inline void lin_set_frame_data_len(lin_num_e lin_num, unsigned char len)
{
    reg_lin_frame1(lin_num) = (reg_lin_frame1(lin_num) & ~(FLD_LIN_FRAME_LEN_RW | FLD_LIN_FRAME_LEN_TRIG_W)) |
                              (MASK_VAL(FLD_LIN_FRAME_LEN_RW, (len - 1)) | MASK_VAL(FLD_LIN_FRAME_LEN_TRIG_W, 1));
}

/**
 * @brief      This function serves to set the type of checksum.
 * @param[in]  lin_num - LIN0/LIN1.
 * @param[in]  type - type of checksum, LIN_CHECKSUM_TYPE_CLASSIC/LIN_CHECKSUM_TYPE_ENHANCED.
 * @return     none.
 */
static inline void lin_set_checksum_type(lin_num_e lin_num, lin_checksum_type_e type)
{
    reg_lin_frame1(lin_num) = (reg_lin_frame1(lin_num) & ~(FLD_LIN_CHECKSUM_TYPE_RW | FLD_LIN_CHECKSUM_SW_RW)) |
                              (MASK_VAL(FLD_LIN_CHECKSUM_TYPE_RW, type) | MASK_VAL(FLD_LIN_CHECKSUM_SW_RW, 0));
}

/**
 * @brief       This function serves to send data in dma mode.
 * @param[in]   lin_num - LIN0/LIN1.
 * @param[in]   addr     - Pointer to data buffer. It must be 4-bytes aligned address
 * @param[in]   len      - Amount of data to be sent in bytes, range from 1 to 0xFFFFFC
 * @return      1  DMA start sending.
 *              0  the length is error.
 * @note
 *              -# After the DMA transfer is complete, the interface needs to be re-invoked to send the next batch of data.
 *              -# If the DMA transfer has not been completed when entering low-power suspend, after suspend, the interface needs to be re-invoked to send the next batch of data.
 */
static bool lin_start_send_dma(lin_num_e lin_num, void *addr, unsigned char len)
{
    dma_chn_dis(s_lin_hw_ctb[lin_num].tx_dma_chn);
    if (len != 0) {
        // In order to prevent the time between the last piece of data and the next piece of data is less than the set timeout time,
        // causing the receiver to treat the next piece of data as the last piece of data.
        lin_uart_clr_tx_buffer(lin_num);
        dma_set_address(s_lin_hw_ctb[lin_num].tx_dma_chn, (unsigned int)(addr), lin_base_addr(lin_num));
        dma_set_size(s_lin_hw_ctb[lin_num].tx_dma_chn, len, DMA_WORD_WIDTH);
        dma_chn_en(s_lin_hw_ctb[lin_num].tx_dma_chn);
        return true;
    } else {
        return false;
    }
}

/**
 * @brief      This function serves to send data in no-dma mode.
 * @param[in]  lin_num - LIN0/LIN1.
 * @param[in]  data - data to send.
 * @param[in]  data_len - data length.
 * @return     none.
 */
static void lin_send_ndma(lin_num_e lin_num, unsigned char *data, unsigned char data_len)
{
    lin_uart_clr_tx_buffer(lin_num);

    unsigned char index = 0;
    for (unsigned char i = 0; i < data_len; i++) {
        reg_lin_data_buf(lin_num, index) = data[i];
        index++;
        index &= 0x03;
    }
}

/**
 * @brief       This function serves to receive data function by DMA, this function tell the DMA to get data from the uart data fifo.
 *              regardless of the length of the DMA configuration, write_num is available,the dma automatically writes back the data receive length to the first four bytes of the rec_buff without manual calculation,
 *              so need addr = the receive buff addr +4.
 * @param[in]   lin_num - LIN0/LIN1.
 * @param[in]   addr     - pointer to the buffer  receive data.
 * @param[in]   rev_size - the receive length of DMA,The maximum transmission length of DMA is 0xFFFFFC bytes, so dont'n over this length.
 * @return      none
 * @note
 *              - addr must be aligned by word (4 bytes), otherwise the program will enter an exception.
 *              - A length greater than XX_len itself and a multiple of 4 is denoted as CEILING(XX_len,4). For example: XX_len=3 ,CEILING(XX_len,4)=4; XX_len=21 ,CEILING(Tx_length, 4)=24. \n
 *                The actual length sent by master  is denoted as Tx_len, The length (param[in]-rev_size) of the interface configuration is denoted as Rx_len. \n
 *                when CEILING(Tx_len,4) > CEILING(Rx_len,4), When the length of the DMA carry reaches Rx_len, the DMA will not stop working and the excess data will not be carried into the buff. \n
 *                For example:Tx_len=21,Rx_len=20,When the DMA stops working the buff is written with a length of 21 and only 20 bytes of data are stored.It is recommended to configure the appropriate Rx_len to avoid this situation.
 */
static void lin_start_receive_dma(lin_num_e lin_num, void *addr, unsigned char rev_size)
{
    dma_chn_dis(s_lin_hw_ctb[lin_num].rx_dma_chn);
    // uart_receive_dma interface dma must be invoked,nodma does not call,so put the uart_rxdone_sel interface in this interface.
    reg_lin_uart_ctrl0(lin_num) &= ~FLD_LIN_UART_NDMA_RXDONE_EN;
    reg_lin_uart_ctrl0(lin_num) |= FLD_LIN_UART_RX_CLR_EN;
    dma_set_address(s_lin_hw_ctb[lin_num].rx_dma_chn, lin_base_addr(lin_num), (unsigned int)(addr));
    dma_set_size(s_lin_hw_ctb[lin_num].rx_dma_chn, rev_size, DMA_WORD_WIDTH);
    dma_chn_en(s_lin_hw_ctb[lin_num].rx_dma_chn);
}

/**
 * @brief      This function serves to receive data in no-dma mode.
 * @param[in]  lin_num - LIN0/LIN1.
 * @param[out]  data - data received.
 * @param[in]  data_len - data length.
 * @return     none.
 */
static unsigned char lin_receive_ndma(lin_num_e lin_num, unsigned char *data, unsigned char data_len)
{
    unsigned char rx_cnt = lin_get_rx_cnt(lin_num);
    unsigned char index = 0, ret = 0;
    for (unsigned char i = 0; i < rx_cnt; i++) {
        if (i < data_len) {
            data[i] = reg_lin_data_buf(lin_num, index);
            index++;
            index &= 3;
            ret++;
        }
    }

    return ret;
}

/**********************************************************************************************************************
 *                                           global function                                                          *
 *********************************************************************************************************************/

/**
 * @brief      This function serves to set EN PIN of LIN controller.
 * @param[in]  lin_num - LIN0/LIN1.
 * @param[in]  val - 1:enable LIN PHY; 0:disable LIN PHY
 * @return     none
 */
void lin_set_hw_en(lin_num_e lin_num, bool val)
{
    gpio_pin_e pin = (gpio_pin_e) s_lin_hw_ctb[lin_num].en_pin;
    gpio_function_en(pin);
    gpio_output_en(pin); //enable output
    gpio_input_dis(pin); //disable input
    if (val) {
        gpio_set_high_level(pin);
    } else {
        gpio_set_low_level(pin);
    }
}

/**
 * @brief      This function serves to generate checksum by software.
 * @param[in]  id - ID value of LIN frame.
 * @param[in]  data - data of LIN frame.
 * @param[in]  len - data length of LIN frame.
 * @param[in]  type - checksum type of LIN frame.
 * @return     none
 */
unsigned char lin_software_checksum(unsigned char id, unsigned char *data, unsigned char len, lin_checksum_type_e type)
{
    unsigned short ret = ((type == LIN_CHECKSUM_TYPE_CLASSIC) ? 0 : lin_software_pid(id));

    for (unsigned char i = 0; i < len; i++) {
        ret += data[i];
        if (ret & 0x100) {
            ret = (ret & 0xff) + 1;
        }
    }

    return ((~ret) & 0xff);
}

/**
 * @brief      This function serves to get the id of frame.
 * @param[in]  handle - operation handle.
 * @return     PID value.
 */
unsigned char lin_get_id(lin_handle_t_ptr handle)
{
    unsigned char ret = 0xff;
    if (handle->init_flag) {
        ret = lin_get_pid(handle->lin_num) & 0x3f;
    }

    return ret;
}

/**
 * @brief      This function serves to send header when acting as a LIN-master node.
 * @param[in]  handle - operation handle.
 * @param[in]  id - the ID of frame.
 * @return     The operation result, true:success, false:failed.
 */
bool lin_master_send_header(lin_handle_t_ptr handle, unsigned char id)
{
    bool ret = false;
    if (handle->init_flag) {
        lin_set_timeout(handle->lin_num, handle->header_bits);
        lin_set_id(handle->lin_num, id, true);
        lin_comm_send_header_act(handle->lin_num);
        ret = true;
    }

    return ret;
}

/**
 * @brief      This function serves to clear the schedule table when acting as a LIN-master node.
 * @param[in]  handle - operation handle.
 * @return     The operation result, true:success, false:failed.
 */
bool lin_master_clr_schedule_table(lin_handle_t_ptr handle)
{
    bool ret = false;
    if (handle->init_flag) {
        lin_set_comm_tx_loop_en(handle->lin_num, false);
        lin_set_flt_pid_buf_wr_en(handle->lin_num, false);
        lin_clr_pid_buf(handle->lin_num);
        lin_set_schedule_dly_table_en(handle->lin_num, false);
        lin_clr_schedule_dly_table(handle->lin_num);
        handle->table_size = 0;
        ret                = true;
    }

    return ret;
}

/**
 * @brief      This function serves to initialize the schedule table when acting as a LIN-master node.
 * @param[in]  handle - operation handle.
 * @return     The operation result, true:success, false:failed.
 */
bool lin_master_init_schedule_table(lin_handle_t_ptr handle)
{
    bool ret = false;
    if (handle->init_flag) {
        lin_clr_all_flt_pid_en(handle->lin_num);
        lin_set_schedule_flt_pid_en(handle->lin_num, true);
        lin_set_comm_tx_loop_en(handle->lin_num, false);

        lin_set_flt_pid_buf_wr_en(handle->lin_num, false);
        lin_clr_pid_buf(handle->lin_num);
        lin_set_schedule_dly_table_en(handle->lin_num, false);
        lin_clr_schedule_dly_table(handle->lin_num);
        lin_set_schedule_start_idx(handle->lin_num, 0);
        lin_set_schedule_end_idx(handle->lin_num, 0);

        lin_rst_schedule_table_cur_idx(handle->lin_num);
        handle->table_size  = 0;
        handle->table_index = 0;
        ret                 = true;
    }

    return ret;
}

/**
 * @brief      This function serves to add item to schedule table when acting as a LIN-master node.
 * @param[in]  handle - operation handle.
 * @param[in]  id - the ID value of frame.
 * @param[in]  slot_cnt - the period of each frame, count of t-base.
 * @return     The operation result, true:success, false:failed.
 */
bool lin_master_add_2_schedule_table(lin_handle_t_ptr handle, unsigned char id, unsigned char slot_cnt)
{
    bool ret = false;
    if (handle->init_flag) {
        if (handle->table_size < 16) {
            handle->table[handle->table_size].id  = id;
            handle->table[handle->table_size].cnt = slot_cnt;
            handle->table_size++;
            ret = true;
        }
    }

    return ret;
}

/**
 * @brief      This function serves to refresh schedule table when acting as a LIN-master node.
 * @param[in]  handle - operation handle.
 * @return     The operation result, true:success, false:failed.
 */
bool lin_master_refresh_schedule_table(lin_handle_t_ptr handle)
{
    bool ret = false;
    if (handle->init_flag) {
        lin_set_flt_pid_buf_wr_en(handle->lin_num, true);
        lin_set_schedule_dly_table_en(handle->lin_num, true);

        unsigned char group_idx = 0, remain = handle->table_size, w_cnt;
        while (remain) {
            w_cnt = ((remain > 4) ? 4 : remain);
            lin_set_schedule_group_sel(handle->lin_num, group_idx);
            for (unsigned char i = 0; i < w_cnt; i++) {
                unsigned char index = 4 * group_idx + i;
                lin_set_schedule_header(handle->lin_num, i, handle->table[index].id, handle->table[index].cnt);
            }
            remain -= w_cnt;
            group_idx++;
            group_idx &= 3;
        }

        lin_set_flt_pid_buf_wr_en(handle->lin_num, false);

        ret = true;
    }

    return ret;
}

/**
 * @brief      This function serves to enable schedule table when acting as a LIN-master node.
 * @param[in]  handle - operation handle.
 * @return     The operation result, true:success, false:failed.
 */
bool lin_master_enable_schedule_table(lin_handle_t_ptr handle)
{
    bool ret = false;
    if (handle->init_flag) {
        lin_set_schedule_start_idx(handle->lin_num, 0);
        lin_set_schedule_end_idx(handle->lin_num, handle->table_size - 1);
        lin_rst_schedule_table_cur_idx(handle->lin_num);
        lin_set_comm_tx_loop_en(handle->lin_num, true);

        ret = true;
    }

    return ret;
}

/**
 * @brief      This function serves to stop schedule table when acting as a LIN-master node.
 * @param[in]  handle - operation handle.
 * @return     The operation result, true:success, false:failed.
 */
bool lin_master_stop_schedule_table(lin_handle_t_ptr handle)
{
    bool ret = false;
    if (handle->init_flag) {
        lin_set_schedule_end_idx(handle->lin_num, lin_master_get_schedule_table_cur_idx(handle));
        ret = true;
    }

    return ret;
}

/**
 * @brief      This function serves to discard response.
 * @param[in]  handle - operation handle.
 * @return     The operation result, true:success, false:failed.
 */
bool lin_discard_response(lin_handle_t_ptr handle)
{
    bool ret = false;
    if (handle->init_flag) {
        lin_set_timeout(handle->lin_num, 10);
        lin_set_response_discard(handle->lin_num);
        ret = true;
    }

    return ret;
}

/**
 * @brief      This function serves to send response.
 * @param[in]  handle - operation handle.
 * @return     The operation result, true:success, false:failed.
 */
bool lin_send_response(lin_handle_t_ptr handle, unsigned char *data, unsigned char data_len, lin_checksum_type_e type)
{
    bool ret = false;
    if ((data_len == 0) || !data || (data_len > 8)) {
        return ret;
    }
    if (handle->init_flag) {
        lin_set_timeout(handle->lin_num, (data_len + 1) * 10);
        lin_set_frame_data_len(handle->lin_num, data_len);
        lin_set_tx_mode(handle->lin_num);

        lin_set_checksum_type(handle->lin_num, type);
        lin_set_checksum_sw_en(handle->lin_num, handle->checksum_sw_enable);
        if (handle->checksum_sw_enable) {
            lin_set_checksum(handle->lin_num, handle->sum_func(lin_get_pid(handle->lin_num), data, data_len, type));
        }

        if (handle->dma_mode_enable) {
            unsigned char *dma_buffer = (unsigned char *)(handle->tx_dma_buffer);
            for (unsigned char i = 0; i < data_len; i++) {
                dma_buffer[i] = data[i];
            }
            if (dma_get_tc_irq_status(BIT(handle->tx_dma_chn))) {
                dma_clr_tc_irq_status(BIT(handle->tx_dma_chn));
            }
            lin_start_send_dma(handle->lin_num, handle->tx_dma_buffer, data_len);
        } else {
            lin_send_ndma(handle->lin_num, data, data_len);
        }

        ret = true;
    }

    return ret;
}

/**
 * @brief      This function serves to start the action to receive response.
 * @param[in]  handle - operation handle.
 * @param[in]  data_len - data length of frame.
 * @param[in]  type - type of checksum, LIN_CHECKSUM_TYPE_CLASSIC/LIN_CHECKSUM_TYPE_ENHANCED
 * @return     The operation result, true:success, false:failed.
 */
bool lin_start_recv_response(lin_handle_t_ptr handle, unsigned char data_len, lin_checksum_type_e type)
{
    bool ret = false;
    if ((data_len == 0) || (data_len > 8)) {
        return ret;
    }

    if (handle->init_flag) {
        lin_set_timeout(handle->lin_num, (data_len + 1) * 10);
        lin_set_frame_data_len(handle->lin_num, data_len);
        lin_set_rx_mode(handle->lin_num);
        lin_set_checksum_type(handle->lin_num, type);
        lin_uart_clr_rx_buffer(handle->lin_num);
        if (handle->dma_mode_enable) {
            handle->check_sum_type = type;
            if (dma_get_tc_irq_status(BIT(handle->rx_dma_chn))) {
                dma_clr_tc_irq_status(BIT(handle->rx_dma_chn));
            }
            lin_start_receive_dma(handle->lin_num, handle->rx_dma_buffer, data_len);
        }

        ret = true;
    }

    return ret;
}

/**
 * @brief      This function serves to start the action to receive response.
 * @param[in]  handle - operation handle.
 * @param[out] data - the data received.
 * @param[in]  data_len - the data length of frame.
 * @param[out] result - the structure to save the result data (PID,length,checksum), can be null.
 * @return     The operation result, true:success, false:failed.
 */
bool lin_get_response_data(lin_handle_t_ptr handle, unsigned char *data, unsigned char data_len, lin_checksum_type_e type, lin_get_response_result_t_ptr result)
{
    bool ret = false;
    if ((data_len == 0) || !data || (data_len > 8)) {
        return ret;
    }

    unsigned char pid = 0, checksum = 0, len = 0;

    if (handle->init_flag) {
        do {
            if (handle->dma_mode_enable) {
                if (dma_get_tc_irq_status(BIT(handle->rx_dma_chn))) {
                    dma_clr_tc_irq_status(BIT(handle->rx_dma_chn));

                    unsigned char *dma_buffer = (unsigned char *)(handle->rx_dma_buffer);
                    for (unsigned char i = 0; i < data_len; i++) {
                        data[i] = dma_buffer[i];
                    }
                    pid      = lin_get_pid(handle->lin_num);
                    checksum = lin_software_checksum(pid, data, data_len, type); // must use software checksum here
                    len = data_len;
                } else {
                    break;
                }
            } else {
                len = lin_receive_ndma(handle->lin_num, data, data_len);
                if (len == data_len) {
                    pid      = lin_get_pid(handle->lin_num);
                    checksum = lin_software_checksum(pid, data, data_len, type); // must use software checksum here
                } else {
                    break;
                }
            }
            ret = true;
        } while (0);
    }

    if (result) {
        result->len      = len;
        result->pid      = pid;
        result->checksum = checksum;
    }

    return ret;
}

/**
 * @brief      This function serves to clear PID-filter table.
 * @param[in]  handle - operation handle.
 * @return     The operation result, true:success, false:failed.
 */
bool lin_slave_clr_pid_filter_table(lin_handle_t_ptr handle)
{
    bool ret = false;
    if (handle->init_flag) {
        handle->table_index = 0;
        handle->table_size  = 0;
        lin_clr_all_flt_pid_en(handle->lin_num);
        lin_set_pid_discard_en(handle->lin_num, false);
        lin_set_flt_pid_buf_wr_en(handle->lin_num, false);
        lin_clr_pid_buf(handle->lin_num);

        ret = true;
    }

    return ret;
}

/**
 * @brief      This function serves to add item to PID-filter table.
 * @param[in]  handle - operation handle.
 * @param[in]  id - the ID value of frame.
 * @param[in]  data_len - the data length of frame.
 * @param[in]  tx_enable - the enable state of TX.
 * @return     The operation result, true:success, false:failed.
 */
bool lin_slave_add_2_pid_filter_table(lin_handle_t_ptr handle, unsigned char id, unsigned char data_len, bool tx_enable)
{
    bool ret = false;
    if (handle->init_flag) {
        if (handle->table_size < 16) {
            handle->table[handle->table_size].id        = id;
            handle->table[handle->table_size].cnt       = data_len;
            handle->table[handle->table_size].tx_enable = tx_enable;
            handle->table_size++;
            ret = true;
        }
    }

    return ret;
}

/**
 * @brief      This function serves to enable the PID-filter table.
 * @param[in]  handle - operation handle.
 * @return     The operation result, true:success, false:failed.
 */
bool lin_slave_enable_pid_filter_table(lin_handle_t_ptr handle)
{
    bool ret = false;
    if (handle->init_flag) {
        lin_set_flt_pid_buf_wr_en(handle->lin_num, true);
        unsigned char group_idx = 0, remain = handle->table_size, w_cnt;
        while (remain) {
            w_cnt = ((remain > 4) ? 4 : remain);
            lin_set_schedule_group_sel(handle->lin_num, group_idx);
            for (unsigned char i = 0; i < w_cnt; i++) {
                unsigned char index = 4 * group_idx + i;
                lin_set_filter_pid(handle->lin_num, i, handle->table[index].id, handle->table[index].cnt, handle->table[index].tx_enable);
                lin_set_filter_pid_en(handle->lin_num, index);
            }
            remain -= w_cnt;
            group_idx++;
            group_idx &= 3;
        }

        lin_set_flt_pid_buf_wr_en(handle->lin_num, false);
        lin_set_pid_discard_en(handle->lin_num, true);

        ret = true;
    }

    return ret;
}

/**
 * @brief      This function serves to send wake-up signal.
 * @param[in]  handle - operation handle.
 * @param[in]  time_us - the signal duration.
 * @return     The operation result, true:success, false:failed.
 * @note       Master node: The duration of the pull-down signal will be slightly shorter than the configuration time(20k baud rate,150us->147us),
 *             due to the internal signal triggering mechanism. So, the value should be set slightly larger than expected.
 *             Slave node:  The duration of the pull-down signal will be slightly longer  than the configuration time(20k baud rate,150us->155us),
 *             due to the internal signal triggering mechanism. So, the value should be set slightly smaller than expected.
 */
bool lin_send_wakeup_signal(lin_handle_t_ptr handle, unsigned short time_us)
{
    bool ret = false;
    if (handle->init_flag) {
        lin_set_tx_wakeup_time(handle->lin_num, time_us);
        lin_tx_wakeup_trig(handle->lin_num);

        ret = true;
    }

    return ret;
}

/**
 * @brief      This function serves to reset t-base counter.
 * @param[in]  handle - operation handle.
 * @return     The operation result, true:success, false:failed.
 */
bool lin_rst_timebase(lin_handle_t_ptr handle)
{
    bool ret = false;
    if (handle->init_flag) {
        lin_manual_clr_timebase(handle->lin_num);
        ret = true;
    }

    return ret;
}

/**
 * @brief      This function serves to force re-send frame header.
 * @param[in]  handle - operation handle.
 * @return     The operation result, true:success, false:failed.
 */
bool lin_master_force_resend_header(lin_handle_t_ptr handle)
{
    bool ret = false;
    if (handle->init_flag) {
        lin_comm_force_run(handle->lin_num);
        ret = true;
    }

    return ret;
}

/**
 * @brief      This function serves to initialize the hardware of LIN controller.
 * @param[in]  handle - operation handle.
 * @param[in]  para - the initialize parameter.
 * @return     The operation handle.
 */
lin_handle_t_ptr lin_hw_init(lin_hw_init_para_t *para)
{
    lin_handle_t_ptr ret = 0;

    if (para->lin_num >= LIN_NUM) {
        return ret;
    }

    s_lin_hw_ctb[para->lin_num].init_flag = false;
    s_lin_hw_ctb[para->lin_num].lin_num   = para->lin_num;

    //DMA initialize
    s_lin_hw_ctb[para->lin_num].dma_mode_enable = para->dma_cfg.dma_mode_enable;
    s_lin_hw_ctb[para->lin_num].tx_dma_chn      = para->dma_cfg.tx_dma_chn;
    s_lin_hw_ctb[para->lin_num].rx_dma_chn      = para->dma_cfg.rx_dma_chn;

    //Checksum initialize
    s_lin_hw_ctb[para->lin_num].checksum_sw_enable = para->checksum_cfg.checksum_sw_enable;
    s_lin_hw_ctb[para->lin_num].sum_func           = ((para->checksum_cfg.sum_func) ? para->checksum_cfg.sum_func : lin_software_checksum);

    if (para->dma_cfg.dma_mode_enable) {
        lin_set_tx_dma_config(para->lin_num);
        lin_set_rx_dma_config(para->lin_num);
    }

    //reset hardware
    lin_reset(para->lin_num);

    //hardware initialize
    s_lin_hw_ctb[para->lin_num].en_pin = para->en_pin;
    s_lin_hw_ctb[para->lin_num].tx_pin = para->tx_pin;
    s_lin_hw_ctb[para->lin_num].rx_pin = para->rx_pin;

    lin_set_hw_en(para->lin_num, 1);
    lin_set_pin(para->lin_num, para->tx_pin, para->rx_pin);

    s_lin_hw_ctb[para->lin_num].baudrate = para->baudrate;
    para->pclk = sys_clk.pclk * 1000 * 1000;
    s_lin_hw_ctb[para->lin_num].pclk     = para->pclk;
    uart_cal_div_and_bwpc(para->baudrate, para->pclk, &(s_lin_hw_ctb[para->lin_num].div), &(s_lin_hw_ctb[para->lin_num].bwpc));
    s_lin_hw_ctb[para->lin_num].uart_clk = para->pclk / (s_lin_hw_ctb[para->lin_num].div + 1);

    lin_uart_init(para->lin_num);

    if (para->role == LIN_MASTER_NODE) {
        lin_set_comm_en(para->lin_num, true);
        lin_set_resp_en(para->lin_num, true);
        lin_hw_set_auto_baud(para->lin_num, false);//auto baud rate is not supported in this version
        lin_set_comm_tx_loop_en(para->lin_num, false);
        lin_set_break_len(para->lin_num, para->hw_cfg.break_len);
        lin_set_break1_len(para->lin_num, para->hw_cfg.break1_len);
    } else if (para->role == LIN_SLAVE_NODE) {
        lin_set_comm_en(para->lin_num, false);
        lin_set_resp_en(para->lin_num, true);
        lin_hw_set_auto_baud(para->lin_num, false);//auto baud rate is not supported in this version
    } else {
        return ret;
    }

    s_lin_hw_ctb[para->lin_num].header_bits = (13 + para->hw_cfg.break_len) + (1 + para->hw_cfg.break1_len) + 10 + 10;
    s_lin_hw_ctb[para->lin_num].role        = para->role;

    lin_set_time_base(para->lin_num, para->hw_cfg.time_base);
    lin_master_set_tx_wait(para->lin_num, para->hw_cfg.jitter_us);
    lin_set_txback_check_en(para->lin_num, false);
    lin_set_bus_inactive_time(para->lin_num, para->hw_cfg.inactive_timeout_ms);
    lin_set_rx_sample_position(para->lin_num, para->hw_cfg.rx_smp_pos);

    lin_set_state(para->lin_num, LIN_STATE_RUN);

    s_lin_hw_ctb[para->lin_num].init_flag = true;

    ret = &(s_lin_hw_ctb[para->lin_num]);

    return ret;
}
