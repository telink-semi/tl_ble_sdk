/********************************************************************************************************
 * @file    genfsk_mode.c
 *
 * @brief   This is the source file for 2.4G SDK
 *
 * @author  2.4G Group
 * @date    2024
 *
 * @par     Copyright (c) 2024, Telink Semiconductor (Shanghai) Co., Ltd. ("TELINK")
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
#include "../stack/2p4g/genfsk_ll/genfsk_ll.h"

#if(FEATURE_TEST_MODE == PER && RF_MODE == GENFSK)

#define TX  1
#define RX  2
#define RF_TX_RX_MODE   TX
static volatile unsigned char rf_run_step               = 0;

// RF PARAMETER
#define PREAMBLE_LEN_BIT 8
static volatile unsigned char chn                       = 4;

//TX
#define TX_PAYLOAD_LEN 32
#define TX_BUF_LEN     64
#define TX_BUF_NUM     2
#if(RF_TX_RX_MODE == TX)
static unsigned char __attribute__ ((aligned (4))) tx_buffer[64] = {0};
#endif
unsigned char                                      tx_payload[8] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
static volatile unsigned char                      tx_done_flag  = 0;
volatile unsigned int                              irq_cnt_tx    = 0;
volatile unsigned int                              tx_cnt        = 0;

//RX
#define RX_BUF_LEN      128
#define RX_BUF_NUM      4




//crc_config
rf_crc_config_t crc_config = {
    .init_value    = 0xffffffff,
    .poly          = 0x00001021,
    .xor_out       = 0,
    .byte_order    = 1,
    .start_cal_pos = 0,
    .len           = 2,
};

gen_fsk_generic_header_t GEN_FSK_GenericHeader = {
    .h0_size     = 0,
    .length_size = 8,
    .h1_size     = 0,
    .h0_val      = 0,
    .length_val  = TX_PAYLOAD_LEN,
    .h1_val      = 0,
};



static unsigned char rx_buf[RX_BUF_LEN * RX_BUF_NUM] __attribute__ ((aligned (4))) = {};
static volatile unsigned char rx_flag                                                       = 0;
static volatile unsigned char rx_first_timeout_flag                                         = 0;
static volatile unsigned int  irq_cnt_rx                                                    = 0;
static volatile unsigned int  irq_cnt_rx_crc_ok                                             = 0;
static volatile unsigned int  irq_cnt_rx_first_timeout                                      = 0;
static volatile unsigned char *rx_packet                                                    = 0;
static volatile unsigned int  print_time                                                    = 0;

struct rf_rx_info {
    unsigned int rx_cnt;
    unsigned int rx_crc_ok_cnt;
};
struct rf_rx_info g_rx_info_statistic[6] = {0};
unsigned char g_rx_info_idx = 0;

_attribute_ram_code_sec_ void rf_irq_handler(void)
{
    if (rf_get_irq_status(FLD_RF_IRQ_TX)) {
        tx_done_flag = 1;
        irq_cnt_tx++;
        rf_clr_irq_status(FLD_RF_IRQ_TX);
    } else if (rf_get_irq_status(FLD_RF_IRQ_RX)) {
        rx_packet = rf_get_rx_packet_addr(RX_BUF_NUM, RX_BUF_LEN, (void *)rx_buf);
        irq_cnt_rx++;
        if (!rf_get_crc_err()) {
            irq_cnt_rx_crc_ok++;
        }
        rx_flag = 1;
        rf_clr_irq_status(FLD_RF_IRQ_RX);
    } else if (rf_get_irq_status(FLD_RF_IRQ_FIRST_TIMEOUT)) {
        rx_first_timeout_flag = 1;
        irq_cnt_rx_first_timeout++;
        rf_clr_irq_status(FLD_RF_IRQ_FIRST_TIMEOUT);
    } else {
        rf_clr_irq_status(FLD_RF_IRQ_ALL);
    }
}
PLIC_ISR_REGISTER(rf_irq_handler, IRQ_ZB_RT)

void rf_param_init(void)
{

    unsigned char sync_word[7] = {0x53, 0x78, 0x56, 0x52};
    //generic FSK Link Layer configuratioin
//    gen_fsk_packet_format_set(GEN_FSK_MODE_FIXED_FORMAT, TX_PAYLOAD_LEN);
//    gen_fsk_datarate_set(GEN_FSK_DATARATE_1MBPS);    //Note that this API must be invoked first before all other APIs
    gen_fsk_preamble_len_set(PREAMBLE_LEN_BIT);
    gen_fsk_sync_word_len_set(SYNC_WORD_LEN_4BYTE);
    gen_fsk_sync_word_set(GEN_FSK_PIPE0, sync_word); //set pipe0's sync word
    gen_fsk_pipe_open(GEN_FSK_PIPE0);                //enable pipe0's reception
    gen_fsk_tx_pipe_set(GEN_FSK_PIPE0);              //set pipe0 as the TX pipe
    gen_fsk_set_crc_config(&crc_config);
    gen_fsk_radio_power_set(GEN_FSK_POWER_INDEX_P0p0dBm);
#if (RF_TX_RX_MODE == TX)
    gen_fsk_tx_buffer_set(TX_BUF_NUM, TX_BUF_LEN);
#elif (RF_TX_RX_MODE == RX)
    gen_fsk_rx_buffer_set(rx_buf, RX_BUF_NUM, RX_BUF_LEN);
#endif
    gen_fsk_radio_state_set(GEN_FSK_STATE_AUTO); //set transceiver to basic RX state
    gen_fsk_tx_settle_set(149);
    delay_us(150);
    gen_fsk_rx_settle_set(89);
    delay_us(90); //wait for rx settle

    //irq configuration
    plic_set_priority(IRQ_ZB_RT, 3);
    plic_interrupt_enable(IRQ_ZB_RT);
    rf_set_irq_mask(FLD_RF_IRQ_TX|FLD_RF_IRQ_RX|FLD_RF_IRQ_FIRST_TIMEOUT);
    rf_clr_irq_status(FLD_RF_IRQ_ALL);
    core_interrupt_enable();

}

void user_init_deepRetn(void)
{
}

void user_gpio_init(void)
{
    gpio_function_en(GPIO_PC4| GPIO_PC5 | GPIO_PC6 | GPIO_PC7);
    gpio_output_en(GPIO_PC4| GPIO_PC5 | GPIO_PC6 | GPIO_PC7);
    gpio_input_dis(GPIO_PC4| GPIO_PC5 | GPIO_PC6 | GPIO_PC7);
    gpio_set_level(GPIO_PC4| GPIO_PC5 | GPIO_PC6 | GPIO_PC7, 1);
}

void user_init_normal(void)
{
    user_gpio_init();
#if (TLKAPI_DEBUG_ENABLE)
    tlkapi_debug_init();
#endif
    gen_fsk_packet_format_set(GEN_FSK_MODE_FIXED_FORMAT, TX_PAYLOAD_LEN);
    gen_fsk_datarate_set(GEN_FSK_DATARATE_1MBPS);
    rf_param_init();
    gen_fsk_channel_set(chn);                     //set rf freq as 2408MHz,1M step

#if(RF_TX_RX_MODE == TX)
    gen_fsk_write_payload(tx_buffer, tx_payload, TX_PAYLOAD_LEN);
    delay_ms(2000);
    printf("genfsk 2404/1M start TX!\n");
#elif(RF_TX_RX_MODE == RX)
    printf("genfsk 2404/1M start RX!\n");
    print_time = clock_time();
    gen_fsk_srx_start(clock_time() + 50 * 16, 0);
#endif
}

_attribute_ram_code_ void sdk_2p4g_main_loop(void)
{
#if(RF_TX_RX_MODE == TX)
    tx_done_flag = 0;
    gen_fsk_stx_start(tx_buffer, clock_time() + 100 * 16);
    while (tx_done_flag == 0);
    delay_ms(1);
    tx_buffer[4]++;
    tx_cnt++;
    if(tx_cnt >= 10000){
        rf_run_step++;
        dma_reset();
        rf_reset_register_value();
        if(rf_run_step == 1){
            chn = 4;
            gen_fsk_packet_format_set(GEN_FSK_MODE_FIXED_FORMAT, TX_PAYLOAD_LEN);
            gen_fsk_datarate_set(GEN_FSK_DATARATE_2MBPS);
            rf_param_init();
            gen_fsk_channel_set(chn);
            tx_cnt = 0;
            delay_ms(5000);
            printf("genfsk 2404/2M start TX!\r\n");
        } else if (rf_run_step == 2){
            chn = 34;
            gen_fsk_packet_format_set(GEN_FSK_MODE_FIXED_FORMAT, TX_PAYLOAD_LEN);
            gen_fsk_datarate_set(GEN_FSK_DATARATE_1MBPS);
            rf_param_init();
            gen_fsk_channel_set(chn);
            tx_cnt = 0;
            delay_ms(5000);
            printf("genfsk 2434/1M start TX!\r\n");
        } else if(rf_run_step == 3){//2434-2M
            chn = 34;
            gen_fsk_packet_format_set(GEN_FSK_MODE_FIXED_FORMAT, TX_PAYLOAD_LEN);
            gen_fsk_datarate_set(GEN_FSK_DATARATE_2MBPS);
            rf_param_init();
            gen_fsk_channel_set(chn);
            tx_cnt = 0;
            delay_ms(5000);
            printf("genfsk 2434/2M start TX!\r\n");
        } else if(rf_run_step == 4){//2474-1M
            chn = 74;
            gen_fsk_packet_format_set(GEN_FSK_MODE_FIXED_FORMAT, TX_PAYLOAD_LEN);
            gen_fsk_datarate_set(GEN_FSK_DATARATE_1MBPS);
            rf_param_init();
            gen_fsk_channel_set(chn);
            tx_cnt = 0;
            delay_ms(5000);
            printf("genfsk 2474/1M start TX!\r\n");
        } else if(rf_run_step == 5){//2474-2M
            chn = 74;
            gen_fsk_packet_format_set(GEN_FSK_MODE_FIXED_FORMAT, TX_PAYLOAD_LEN);
            gen_fsk_datarate_set(GEN_FSK_DATARATE_2MBPS);
            rf_param_init();
            gen_fsk_channel_set(chn);
            tx_cnt = 0;
            delay_ms(5000);
            printf("genfsk 2474/2M start TX!\r\n");
        } else{
            printf("genfsk_per_test_done!\r\n");
#if (TLKAPI_DEBUG_ENABLE)
            tlkapi_debug_handler();
#endif
            while(1);
        }
    }
#elif(RF_TX_RX_MODE == RX)
    if (1 == rx_flag)
    {
        rx_flag = 0;
        print_time = clock_time();
        gen_fsk_srx_start(clock_time() + 50 * 16, 0);
    }

    if (1 == rx_first_timeout_flag)
    {
        rx_first_timeout_flag = 0;
        gen_fsk_srx_start(clock_time() + 50 * 16, 0);
    }

    if(clock_time_exceed(print_time, 3000000)){ //per_test_done, print result
        rf_set_tx_rx_off();
        g_rx_info_statistic[g_rx_info_idx].rx_cnt = irq_cnt_rx;
        g_rx_info_statistic[g_rx_info_idx].rx_crc_ok_cnt = irq_cnt_rx_crc_ok;
        g_rx_info_idx = g_rx_info_idx + 1;
        irq_cnt_rx = 0;
        irq_cnt_rx_crc_ok = 0;
        rf_run_step++;
        print_time = clock_time();
        dma_reset();
        rf_reset_register_value();
        if(rf_run_step == 1){ //2404-2M
            chn = 4;
            gen_fsk_packet_format_set(GEN_FSK_MODE_FIXED_FORMAT, TX_PAYLOAD_LEN);
            gen_fsk_datarate_set(GEN_FSK_DATARATE_2MBPS);
            rf_param_init();
            rf_clr_rx_rptr();
            gen_fsk_rx_buffer_set(rx_buf, RX_BUF_NUM, RX_BUF_LEN);
            gen_fsk_channel_set(chn);
            printf("genfsk 2404/2M start RX!\r\n");
            gen_fsk_srx_start(clock_time() + 50 * 16, 0);
        }
        else if(rf_run_step == 2){ //2434-1M
            chn = 34;
            gen_fsk_packet_format_set(GEN_FSK_MODE_FIXED_FORMAT, TX_PAYLOAD_LEN);
            gen_fsk_datarate_set(GEN_FSK_DATARATE_1MBPS);
            rf_param_init();
            rf_clr_rx_rptr();
            gen_fsk_rx_buffer_set(rx_buf, RX_BUF_NUM, RX_BUF_LEN);
            gen_fsk_channel_set(chn);
            printf("genfsk 2434/1M start RX!\r\n");
            gen_fsk_srx_start(clock_time() + 50 * 16, 0);
        }
        else if(rf_run_step == 3){ //2434-2M
            chn = 34;
            gen_fsk_packet_format_set(GEN_FSK_MODE_FIXED_FORMAT, TX_PAYLOAD_LEN);
            gen_fsk_datarate_set(GEN_FSK_DATARATE_2MBPS);
            rf_param_init();
            rf_clr_rx_rptr();
            gen_fsk_rx_buffer_set(rx_buf, RX_BUF_NUM, RX_BUF_LEN);
            gen_fsk_channel_set(chn);
            printf("genfsk 2434/2M start RX!\r\n");
            gen_fsk_srx_start(clock_time() + 50 * 16, 0);
        }
        else if(rf_run_step == 4){ //2474-1M
            chn = 74;
            gen_fsk_packet_format_set(GEN_FSK_MODE_FIXED_FORMAT, TX_PAYLOAD_LEN);
            gen_fsk_datarate_set(GEN_FSK_DATARATE_1MBPS);
            rf_param_init();
            rf_clr_rx_rptr();
            gen_fsk_rx_buffer_set(rx_buf, RX_BUF_NUM, RX_BUF_LEN);
            gen_fsk_channel_set(chn);
            printf("genfsk 2474/1M start RX!\r\n");
            gen_fsk_srx_start(clock_time() + 50 * 16, 0);
        }
        else if(rf_run_step == 5){ //2474-2M
            chn = 74;
            gen_fsk_packet_format_set(GEN_FSK_MODE_FIXED_FORMAT, TX_PAYLOAD_LEN);
            gen_fsk_datarate_set(GEN_FSK_DATARATE_2MBPS);
            rf_param_init();
            rf_clr_rx_rptr();
            gen_fsk_rx_buffer_set(rx_buf, RX_BUF_NUM, RX_BUF_LEN);
            gen_fsk_channel_set(chn);
            printf("genfsk 2474/2M start RX!\r\n");
            gen_fsk_srx_start(clock_time() + 50 * 16, 0);
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
#if (TLKAPI_DEBUG_ENABLE)
    tlkapi_debug_handler();
#endif
    ////////////////////////////////////// 2P4G entry /////////////////////////////////
    sdk_2p4g_main_loop();
}

#endif
