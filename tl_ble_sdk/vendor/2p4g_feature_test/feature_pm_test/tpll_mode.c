/********************************************************************************************************
 * @file    tpll_mode.c
 *
 * @brief   This is the source file for 2.4G SDK
 *
 * @author  2.4G Group
 * @date    2026
 *
 * @par     Copyright (c) 2023, Telink Semiconductor (Shanghai) Co., Ltd. ("TELINK")
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
#include "app_config.h"
#include "tl_common.h"
#include "drivers.h"
#include "../stack/2p4g/tpll/tpll.h"

#if(FEATURE_TEST_MODE == PM && RF_MODE == TPLL)

    #define PTX_PIPE        0
    #define PRI_FLT_MODE_EN 0
    #define TX_PAYLOAD_LEN  16
unsigned char ptx_buffer[TPLL_TX_FIFO_SIZE + TPLL_PIPE_NUM * TPLL_TX_FIFO_SIZE * TPLL_TX_FIFO_NUM] __attribute__((aligned(4))) = {}; //tx buffer should big than pipe_num *fifo_size*32
unsigned char rx_buf[TPLL_PIPE_RX_FIFO_SIZE * TPLL_PIPE_RX_FIFO_NUM] __attribute__((aligned(4)))           = {};

static unsigned char   tx_data[32]   = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0x20};
volatile unsigned char preamble_len  = 0;
volatile unsigned char tmp           = 1;

volatile unsigned int tx_irq_cnt_tx          = 0;
volatile unsigned int tx_irq_cnt_invalid_pid = 0;
volatile unsigned int tx_irq_cnt_max_retry   = 0;
volatile unsigned int tx_irq_cnt_tx_ds       = 0;
volatile unsigned int tx_irq_cnt_rx_dr       = 0;
volatile unsigned int math_cnt               = 0;
volatile unsigned int unmath_cnt             = 0;

volatile unsigned char rx_flag       = 0;
volatile unsigned char ds_flag       = 0;
volatile unsigned char tx_done_flag  = 0;
volatile unsigned char maxretry_flag = 0;
volatile unsigned char rx_dr_flag    = 0;
__attribute__((section(".ram_code"))) __attribute__((optimize("-Os"))) void rf_irq_handler(void)
{
    unsigned char pipe = TPLL_GetTXPipe();

    if (rf_get_irq_status(FLD_RF_IRQ_TX)) {
        reg_rf_irq_status = FLD_RF_IRQ_TX;
        tx_irq_cnt_tx++;
        tx_done_flag = 1;
    }
    if (rf_get_irq_status(FLD_RF_IRQ_INVALID_PID)) {
        reg_rf_irq_status = FLD_RF_IRQ_INVALID_PID;
        tx_irq_cnt_invalid_pid++;
    }
    if (rf_get_irq_status(FLD_RF_IRQ_TX_RETRYCNT)) {
        //tx_retrycnt_irq:Maximum number of TX retransmits interrupt.
        //Asserted when retry counter reaches the max numbers.
        reg_rf_irq_status = FLD_RF_IRQ_TX_RETRYCNT;
        tx_irq_cnt_max_retry++;
        maxretry_flag = 1;
        //adjust rptr
        TPLL_UpdateTXFifoRptr(pipe);
    }
    if (rf_get_irq_status(FLD_RF_IRQ_TX_DS)) {
        //Data Sent TX FIFO interrupt. Asserted when packet transmitted on TX.
        //If AUTO_ACK is activated, this bit is set high only when ACK is received.
        reg_rf_irq_status = FLD_RF_IRQ_TX_DS;
        tx_irq_cnt_tx_ds++;
        ds_flag = 1;
    }
    if (rf_get_irq_status(FLD_RF_IRQ_RX_DR)) {
        //rx_dr_irq:Data Ready RX FIFO interrupt. Asserted when new data arrives RX FIFO.
        //The RX_DR IRQ is asserted by a new packet arrival even
        reg_rf_irq_status = FLD_RF_IRQ_RX_DR;
        tx_irq_cnt_rx_dr++;
        rx_dr_flag = 1;
    }
    if (rf_get_irq_status(FLD_RF_IRQ_PKT_MATCH)) {
        math_cnt++;
        rf_clr_irq_status(FLD_RF_IRQ_PKT_MATCH);
    }
    if (rf_get_irq_status(FLD_RF_IRQ_PKT_UNMATCH)) {
        unmath_cnt++;
        rf_clr_irq_status(FLD_RF_IRQ_PKT_UNMATCH);
    } else {
        rf_clr_irq_status(FLD_RF_IRQ_ALL);
    }
}
PLIC_ISR_REGISTER(rf_irq_handler, IRQ_ZB_RT)

TPLL_CrcConfig_t TPLL_CrcConfig = {
    .init_value    = 0xffffffff,
    .poly          = 0x00001021,
    .xor_out       = 0,
    .byte_order    = 1,
    .start_cal_pos = 0,
    .len           = 2,
};

/**
 *  @note  The rf_pkt_flt function matches from the high bit,
 *         You need to configure rf_pkt_mask and rf_pkt_match from the high bit when configuring it,
 *         the below takes generic packet for example.
*/
rf_pkt_flt_t TPLL_PktFlt = {
    .rf_pkt_flt_start       = 4,  //start at h0
    .rf_pkt_flt_end         = 11,
    .rf_pkt_match_threshold = 24, //max 64 bit
    .rf_pkt_match_low       = 0x00000000,
    .rf_pkt_match_high      = 0x00030201,
    .rf_pkt_mask_low        = 0x00000000,
    .rf_pkt_mask_high       = 0x00ffffff, //if bit is 1 means this bit should matched
};

/**
 *  @note  The TPLL_GenericHeader_t function to set generic packet header,
 *         You need to configure the size in bit and the value of h0,h1 and payload len.
 *         You also can configure the start location of pid and no ack bit if you need.
 *         only generic mode need to configure this!!!
*/
TPLL_GenericHeader_t TPLL_GenericHeader = {
    .h0_size         = 5,
    .h1_size         = 4,
    .length_size     = 15,
    .h0_val          = 26, //no ack 0x22
    .h1_val          = 14,
    .length_val      = TX_PAYLOAD_LEN,
    .pid_start_bit   = 0,  //start at h0
    .noack_start_bit = 2,
};

void user_gpio_init(void)
{
}

void user_init_normal(void)
{
    user_gpio_init();
    #if (TLKAPI_DEBUG_ENABLE)
    tlkapi_debug_init();
    #endif

    //rf configuration
    TPLL_SetFormatMode(TPLL_MODE_GENERIC_FORMAT);
    TPLL_SetBitrate(TPLL_BITRATE_1MBPS);
    TPLL_SetOutputPower(TPLL_POWER_INDEX_P0p0dBm);
    TPLL_SetAddressWidth(ADDRESS_WIDTH_5BYTES);
    #if (PRI_FLT_MODE_EN)
    TPLL_PktFilter(TPLL_PktFlt);
    #endif
    TPLL_ClosePipe(TPLL_PIPE_ALL);

    unsigned char tx_address[5] = {0xe7, 0xe7, 0xe7, 0xe7, 0xe7}; //{0xaa,0xbb,0xcc,0xdd,0xee};
    TPLL_SetAddress(TPLL_PIPE0, tx_address);
    TPLL_OpenPipe(TPLL_PIPE0);
    TPLL_SetTXPipe(TPLL_PIPE0);


    TPLL_DmaInit(rx_buf, ptx_buffer);
    TPLL_ModeSet(TPLL_MODE_PTX);
    TPLL_SetRFChannel(4);
    TPLL_SetAutoRetry(0, 150); //5,150
    TPLL_RxTimeoutSet(500);    //if the mode is 250k ,the rx_time_out need more time, as so 1000us is ok!
    TPLL_RxSettleSet(85);
    TPLL_TxSettleSet(149);
    TPLL_Preamble_Set(8);
    TPLL_CrcSet(TPLL_CrcConfig);

    delay_us(150);
    //configure irq
    core_interrupt_enable();
    plic_interrupt_enable(IRQ_ZB_RT);
    rf_clr_irq_mask(FLD_RF_IRQ_ALL);
    rf_set_irq_mask(FLD_RF_IRQ_TX | FLD_RF_IRQ_TX_DS | FLD_RF_IRQ_TX_RETRYCNT | FLD_RF_IRQ_RX_DR | FLD_RF_IRQ_PKT_UNMATCH | FLD_RF_IRQ_PKT_MATCH);

#if (RF_DEBUG_IO_ENABLE)
    extern _attribute_ram_code_sec_ void rf_enable_debug_IO(void);
    rf_enable_debug_IO();
#endif
    TPLL_WriteTxPayload(PTX_PIPE, ptx_buffer, (unsigned char *)tx_data, 32);
    TPLL_PTXTrig();
}

/**
 * @brief       user initialization when MCU wake_up from deepSleep_retention mode
 * @param[in]   none
 * @return      none
 */
void user_init_deepRetn(void)
{
    user_init_normal();
#if (TLKAPI_DEBUG_ENABLE)
    printf("user_init_deepRetn\n");
#endif
}


_attribute_ram_code_ void sdk_2p4g_main_loop(void)
{
    if(1 == ds_flag) //ptx_success
    {
        ds_flag = 0;
        tx_data[4]++;
        tmp = TPLL_WriteTxPayload(PTX_PIPE, ptx_buffer, (unsigned char *)tx_data, TX_PAYLOAD_LEN);
        if (!tmp) {
            TPLL_PTXTrig();
        }
    }
    else if(maxretry_flag == 1) //ptx_fail
    {
        maxretry_flag = 0;
        //sleep
        unsigned char pm_cnt = analog_read(0x3b);
        if(pm_cnt < 10){
            pm_cnt++;
            analog_write(0x3b, pm_cnt);
            gpio_shutdown(GPIO_ALL);
            pm_sleep_wakeup(DEEPSLEEP_MODE, PM_WAKEUP_TIMER, PM_TICK_STIMER, stimer_get_tick() + 1000 * SYSTEM_TIMER_TICK_1MS);
        }
        else if(pm_cnt < 20){
            pm_cnt++;
            analog_write(0x3b, pm_cnt);
            gpio_shutdown(GPIO_ALL);
            pm_sleep_wakeup(DEEPSLEEP_MODE_RET_SRAM_LOW32K, PM_WAKEUP_TIMER, PM_TICK_STIMER, stimer_get_tick() + 1000 * SYSTEM_TIMER_TICK_1MS);
        }
        else if(pm_cnt < 30){
            pm_cnt++;
            analog_write(0x3b, pm_cnt);
            pm_sleep_wakeup(SUSPEND_MODE, PM_WAKEUP_TIMER, PM_TICK_STIMER, stimer_get_tick() + 1000 * SYSTEM_TIMER_TICK_1MS);
            user_init_normal();
            ds_flag = 1;
        }
        else{
            while(1);
        }
    }
}

_attribute_no_inline_ void main_loop(void)
{
    ////////////////////////////////////// Debug entry /////////////////////////////////
    #if (TLKAPI_DEBUG_ENABLE)
    tlkapi_debug_handler();
    #endif
    sdk_2p4g_main_loop();
}

#endif // FEATURE_TEST_MODE == PM && RF_MODE == GENFSK
