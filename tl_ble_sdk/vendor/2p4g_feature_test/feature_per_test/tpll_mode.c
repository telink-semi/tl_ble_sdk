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

#if(FEATURE_TEST_MODE == PER && RF_MODE == TPLL)

#define TX  1
#define RX  2
#define RF_TX_RX_MODE   TX

static volatile unsigned char rf_run_step               = 0;
#define PTX_PIPE                                          0//warning:B80 only support pipe0,B80B support pipe0~5
static volatile unsigned char chn                       = 4;
volatile unsigned char tmp           = 1;

volatile unsigned char tx_flag          = 0;
volatile unsigned char invalid_pid_flag = 0;
volatile unsigned char tx_maxretry_flag    = 0;
volatile unsigned char rx_dr_flag       = 0;
volatile unsigned char rx_flag          = 0;
unsigned char          tx_payload_len   = 8;
unsigned char          preamble_len     = 0;
volatile unsigned char tx_ds_flag          = 0;
int                    ret              = 0;

volatile unsigned char       rx_payload[128]              = {0};

static volatile unsigned int irq_cnt_rx_sync              = 0;
static volatile unsigned int irq_cnt_rx_head_done         = 0;
volatile unsigned int        irq_cnt_invalid_pid          = 0;
volatile unsigned int        irq_cnt_tx_max_retry            = 0;
volatile unsigned int        irq_cnt_tx_ds                = 0;
volatile unsigned int        irq_cnt_tx                   = 0;
volatile unsigned int        irq_cnt_rx                   = 0;
volatile unsigned int        irq_cnt_rx_dr                = 0;
static volatile unsigned int print_time                   = 0;
volatile unsigned int test_tx_cnt = 0;

struct rf_rx_info {
    unsigned int rx_cnt;
    unsigned int rx_crc_ok_cnt;
};
struct rf_rx_info g_rx_info_statistic[6] = {0};
unsigned char g_rx_info_idx = 0;

#define TX_PAYLOAD_LEN  8
#define ACK_PAYLOAD_LEN         6
#if (RF_TX_RX_MODE == RX)
static unsigned char    ack_payload[32] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f};
#endif
#if (RF_TX_RX_MODE == TX)
static unsigned char   tx_data[32]   = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10,
                                        0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0x20};
#endif
unsigned char rx_buf[TPLL_PIPE_RX_FIFO_SIZE * TPLL_PIPE_RX_FIFO_NUM] __attribute__((aligned(4)))           = {};

#if (RF_TX_RX_MODE == TX)
unsigned char ptx_buffer[TPLL_TX_FIFO_SIZE + TPLL_PIPE_NUM * TPLL_TX_FIFO_SIZE * TPLL_TX_FIFO_NUM] __attribute__((aligned(4))) = {}; //tx buffer should big than pipe_num *fifo_size*32
#elif (RF_TX_RX_MODE == RX)
unsigned char ptx_buffer[TPLL_PIPE_NUM * TPLL_TX_FIFO_SIZE * TPLL_TX_FIFO_NUM] __attribute__((aligned(4))) = {};
#endif

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
    .h0_size         = 8,
    .h1_size         = 8,
    .length_size     = 8,
    .h0_val          = 26, //no ack 0x22
    .h1_val          = 14,
    .length_val      = TX_PAYLOAD_LEN,
    .pid_start_bit   = 0,  //start at h0
    .noack_start_bit = 2,
};




__attribute__((section(".ram_code"))) __attribute__((optimize("-Os"))) void rf_irq_handler(void)
{
    unsigned char pipe = TPLL_GetTXPipe();

    if (rf_get_irq_status(FLD_RF_IRQ_TX)) {
        reg_rf_irq_status = FLD_RF_IRQ_TX;
        irq_cnt_tx++;
        tx_flag = 1;
    }
    if (rf_get_irq_status(FLD_RF_IRQ_RX)) {
        reg_rf_irq_status = FLD_RF_IRQ_RX;
        irq_cnt_rx++;
        rx_flag = 1;
    }
    if (rf_get_irq_status(FLD_RF_IRQ_INVALID_PID)) {
        reg_rf_irq_status = FLD_RF_IRQ_INVALID_PID;
        irq_cnt_invalid_pid++;
    }
    if (rf_get_irq_status(FLD_RF_IRQ_TX_RETRYCNT)) {
        //tx_retrycnt_irq:Maximum number of TX retransmits interrupt.
        //Asserted when retry counter reaches the max numbers.
        reg_rf_irq_status = FLD_RF_IRQ_TX_RETRYCNT;
        irq_cnt_tx_max_retry++;
        tx_maxretry_flag = 1;
        //adjust rptr
        TPLL_UpdateTXFifoRptr(pipe);
    }
    if (rf_get_irq_status(FLD_RF_IRQ_TX_DS)) {
        //Data Sent TX FIFO interrupt. Asserted when packet transmitted on TX.
        //If AUTO_ACK is activated, this bit is set high only when ACK is received.
        reg_rf_irq_status = FLD_RF_IRQ_TX_DS;
        irq_cnt_tx_ds++;
        tx_ds_flag = 1;
    }
    if (rf_get_irq_status(FLD_RF_IRQ_RX_DR)) {
        //rx_dr_irq:Data Ready RX FIFO interrupt. Asserted when new data arrives RX FIFO.
        //The RX_DR IRQ is asserted by a new packet arrival even
        reg_rf_irq_status = FLD_RF_IRQ_RX_DR;
        irq_cnt_rx_dr++;
        rx_dr_flag = 1;
        gpio_write(GPIO_PC4, 1);
        gpio_write(GPIO_PC4, 0);
    } else {
        rf_clr_irq_status(FLD_RF_IRQ_ALL);
    }
}
PLIC_ISR_REGISTER(rf_irq_handler, IRQ_ZB_RT)

void user_gpio_init(void)
{
    gpio_function_en(GPIO_PC4| GPIO_PC5 | GPIO_PC6 | GPIO_PC7);
    gpio_output_en(GPIO_PC4| GPIO_PC5 | GPIO_PC6 | GPIO_PC7);
    gpio_input_dis(GPIO_PC4| GPIO_PC5 | GPIO_PC6 | GPIO_PC7);
    gpio_set_level(GPIO_PC4| GPIO_PC5 | GPIO_PC6 | GPIO_PC7, 1);
}

/**
 * @brief       user initialization when MCU wake_up from deepSleep_retention mode
 * @param[in]   none
 * @return      none
 */
void user_init_deepRetn(void)
{
}

void rf_param_init(void)
{
    //rf configuration

    TPLL_SetOutputPower(TPLL_POWER_INDEX_P0p0dBm);
    TPLL_SetAddressWidth(ADDRESS_WIDTH_5BYTES);
    TPLL_ClosePipe(TPLL_PIPE_ALL);


    unsigned char tx_address[5] = {0xe7, 0xe7, 0xe7, 0xe7, 0xe7}; //{0xaa,0xbb,0xcc,0xdd,0xee};
    TPLL_SetAddress(TPLL_PIPE0, tx_address);
    TPLL_OpenPipe(TPLL_PIPE0);
    TPLL_SetTXPipe(TPLL_PIPE0);

    TPLL_DmaInit(rx_buf, ptx_buffer);
#if (RF_TX_RX_MODE == TX)
    TPLL_ModeSet(TPLL_MODE_PTX);
    TPLL_SetAutoRetry(0, 150); //5,150
#elif (RF_TX_RX_MODE == RX)
    TPLL_ModeSet(TPLL_MODE_PRX);
#endif
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
}

void user_init_normal(void)
{
    user_gpio_init();
    #if (TLKAPI_DEBUG_ENABLE)
    tlkapi_debug_init();
    #endif

    TPLL_SetFormatMode(TPLL_MODE_GENERIC_FORMAT);
    TPLL_SetBitrate(TPLL_BITRATE_2MBPS);
    TPLL_SetRFChannel(chn);
    rf_param_init();


#if (RF_TX_RX_MODE == TX)
    printf("\r\n");
    printf("tpll 2404/1M start TX!\r\n");
    tx_ds_flag = 1; // for first start
    delay_ms(2000);
//    TPLL_WriteTxPayload(PTX_PIPE, ptx_buffer, (unsigned char *)tx_data, 16);
//    TPLL_PTXTrig();
#elif (RF_TX_RX_MODE == RX)
    printf("\r\n");
    printf("tpll 2404/1M start RX!\r\n");
    print_time = clock_time();
    TPLL_WriteAckPayload(TPLL_PIPE0, ptx_buffer, ack_payload, ACK_PAYLOAD_LEN);
    TPLL_PRXTrig();
#endif
}

_attribute_ram_code_ void sdk_2p4g_main_loop(void)
{
#if(RF_TX_RX_MODE == TX)
    if (1 == tx_ds_flag || 1 == tx_maxretry_flag)
    {
        if (1 == tx_ds_flag)
        {
        }
        tx_ds_flag = 0;
        tx_maxretry_flag = 0;

        test_tx_cnt++;
        if(test_tx_cnt > 10000){
            rf_run_step++;
            rf_set_tx_rx_off();
            printf("tpll_irq_cnt_tx_ds: %d\r\n", irq_cnt_tx_ds);
            irq_cnt_tx_ds = 0;
            dma_reset();
            rf_reset_register_value();
            if(rf_run_step == 1){ //2404-2M
                chn = 4;
                TPLL_SetFormatMode(TPLL_MODE_GENERIC_FORMAT);
                TPLL_SetBitrate(TPLL_BITRATE_2MBPS);
                TPLL_SetRFChannel(chn);
                rf_param_init();
                test_tx_cnt = 1;
                delay_ms(5000);
                printf("tpll 2404/2M start TX!\r\n");
            }
            else if(rf_run_step == 2){//2434-1M
                chn = 34;
                TPLL_SetFormatMode(TPLL_MODE_GENERIC_FORMAT);
                TPLL_SetBitrate(TPLL_BITRATE_1MBPS);
                TPLL_SetRFChannel(chn);
                rf_param_init();
                test_tx_cnt = 1;
                delay_ms(5000);
                printf("tpll 2434/1M start TX!\r\n");
            }
            else if(rf_run_step == 3){//2434-2M
                chn = 34;
                TPLL_SetFormatMode(TPLL_MODE_GENERIC_FORMAT);
                TPLL_SetBitrate(TPLL_BITRATE_2MBPS);
                TPLL_SetRFChannel(chn);
                rf_param_init();
                test_tx_cnt = 1;
                delay_ms(5000);
                printf("tpll 2434/2M start TX!\r\n");
            }
            else if(rf_run_step == 4){//2474-1M
                chn = 74;
                TPLL_SetFormatMode(TPLL_MODE_GENERIC_FORMAT);
                TPLL_SetBitrate(TPLL_BITRATE_1MBPS);
                TPLL_SetRFChannel(chn);
                rf_param_init();
                test_tx_cnt = 1;
                delay_ms(5000);
                printf("tpll 2474/1M start TX!\r\n");
            }
            else if(rf_run_step == 5){//2474-2M
                chn = 74;
                TPLL_SetFormatMode(TPLL_MODE_GENERIC_FORMAT);
                TPLL_SetBitrate(TPLL_BITRATE_2MBPS);
                TPLL_SetRFChannel(chn);
                rf_param_init();
                test_tx_cnt = 1;
                delay_ms(5000);
                printf("tpll 2474/2M start TX!\r\n");
            }
            else{
#if (TLKAPI_DEBUG_ENABLE)
                tlkapi_debug_handler();
#endif
                printf("tpll_per_test_done!\r\n");
                while(1);
            }
        }

        delay_ms(1);
        ret = TPLL_WriteTxPayload(PTX_PIPE, ptx_buffer, (unsigned char *)tx_data, TX_PAYLOAD_LEN);
        if (!ret)
        {
            TPLL_PTXTrig();
        }
    }
#elif (RF_TX_RX_MODE == RX)
    if (1 == rx_dr_flag)
    {
        print_time = clock_time();
        rx_dr_flag = 0;
        while(!TPLL_TxFifoEmpty(0));
        TPLL_WriteAckPayload(TPLL_PIPE0, ptx_buffer, ack_payload, TX_PAYLOAD_LEN);
    }
    if(clock_time_exceed(print_time, 3000000)){ //per_test_done, print result
        rf_set_tx_rx_off();
        printf("tpll_irq_cnt_rx: %d, tpll_irq_cnt_rx_crc_ok: %d\r\n", irq_cnt_rx, irq_cnt_rx_dr);
        g_rx_info_statistic[g_rx_info_idx].rx_cnt = irq_cnt_rx;
        g_rx_info_statistic[g_rx_info_idx].rx_crc_ok_cnt = irq_cnt_rx_dr;
        g_rx_info_idx = g_rx_info_idx + 1;
        irq_cnt_rx = 0;
        irq_cnt_rx_dr = 0;
        rf_run_step++;
        print_time = clock_time();
        dma_reset();
        rf_reset_register_value();
        if(rf_run_step == 1){ //2404-2M
            chn = 4;
            TPLL_SetFormatMode(TPLL_MODE_GENERIC_FORMAT);
            TPLL_SetBitrate(TPLL_BITRATE_2MBPS);
            TPLL_SetRFChannel(chn);
            rf_param_init();
            printf("tpll 2404/2M start RX!\r\n");
            rx_dr_flag = 0;
            TPLL_PRXTrig();
        }
        else if(rf_run_step == 2){ //2434-1M
            chn = 34;
            TPLL_SetFormatMode(TPLL_MODE_GENERIC_FORMAT);
            TPLL_SetBitrate(TPLL_BITRATE_1MBPS);
            TPLL_SetRFChannel(chn);
            rf_param_init();
            printf("tpll 2434/1M start RX!\r\n");
            rx_dr_flag = 0;
            TPLL_PRXTrig();
        }
        else if(rf_run_step == 3){ //2434-2M
            chn = 34;
            TPLL_SetFormatMode(TPLL_MODE_GENERIC_FORMAT);
            TPLL_SetBitrate(TPLL_BITRATE_2MBPS);
            TPLL_SetRFChannel(chn);
            rf_param_init();
            printf("tpll 2434/2M start RX!\r\n");
            rx_dr_flag = 0;
            TPLL_PRXTrig();
        }
        else if(rf_run_step == 4){ //2474-1M
            chn = 74;
            TPLL_SetFormatMode(TPLL_MODE_GENERIC_FORMAT);
            TPLL_SetBitrate(TPLL_BITRATE_1MBPS);
            TPLL_SetRFChannel(chn);
            rf_param_init();
            printf("tpll 2474/1M start RX!\r\n");
            rx_dr_flag = 0;
            TPLL_PRXTrig();
        }
        else if(rf_run_step == 5){ //2474-2M
            chn = 74;
            TPLL_SetFormatMode(TPLL_MODE_GENERIC_FORMAT);
            TPLL_SetBitrate(TPLL_BITRATE_2MBPS);
            TPLL_SetRFChannel(chn);
            rf_param_init();
            printf("tpll 2474/2M start RX!\r\n");
            rx_dr_flag = 0;
            TPLL_PRXTrig();
        }
        else{
            tlk_printf("\n2404/1M:rx cnt=%5d, rx_crc_ok_cnt=%5d\n2404/2M:rx cnt=%5d, rx_crc_ok_cnt=%5d\n"
                                   "2434/1M:rx cnt=%5d, rx_crc_ok_cnt=%5d\n2434/2M:rx cnt=%5d, rx_crc_ok_cnt=%5d\n"
                                   "2474/1M:rx cnt=%5d, rx_crc_ok_cnt=%5d\n2474/2M:rx cnt=%5d, rx_crc_ok_cnt=%5d\n ",
                                g_rx_info_statistic[0].rx_cnt, g_rx_info_statistic[0].rx_crc_ok_cnt,
                                g_rx_info_statistic[1].rx_cnt, g_rx_info_statistic[1].rx_crc_ok_cnt,
                                g_rx_info_statistic[2].rx_cnt, g_rx_info_statistic[2].rx_crc_ok_cnt,
                                g_rx_info_statistic[3].rx_cnt, g_rx_info_statistic[3].rx_crc_ok_cnt,
                                g_rx_info_statistic[4].rx_cnt, g_rx_info_statistic[4].rx_crc_ok_cnt,
                                g_rx_info_statistic[5].rx_cnt, g_rx_info_statistic[5].rx_crc_ok_cnt);
#if (TLKAPI_DEBUG_ENABLE)
            tlkapi_debug_handler();
#endif
            while(1);
        }
    }
#endif

}

_attribute_no_inline_ void main_loop(void)
{
    ////////////////////////////////////// Debug entry /////////////////////////////////
    #if (TLKAPI_DEBUG_ENABLE)
    tlkapi_debug_handler();
    #endif
    sdk_2p4g_main_loop();
}

#endif // FEATURE_TEST_MODE == PER && RF_MODE == TPLL
