/********************************************************************************************************
 * @file    app_stx_3bit.c
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
#if (MCU_CORE_TYPE == MCU_CORE_TL323X)
#if (GEN_FSK_MODE == GEN_FSK_STX_3BIT)

    //TX Buffer related
    #define TX_PAYLOAD_LEN 8
    #define TX_BUF_LEN     64
    #define TX_BUF_NUM     2
static unsigned char __attribute__((aligned(4))) tx_buffer[64] = {};
unsigned char                                    tx_payload[8] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
static volatile unsigned char                    tx_done_flag  = 0;
static volatile unsigned int                     irq_cnt_tx    = 0;


//crc_config
rf_crc_config_t crc_config = {
    .init_value    = 0xffffffff,
    .poly          = 0x00001021,
    .xor_out       = 0,
    .byte_order    = 1,
    .start_cal_pos = 0,
    .len           = 2,
};

/**
 *  @note  The TPLL_GenericHeader_t function to set generic packet header,
 *         You need to configure the size in bit and the value of h0,h1 and payload len.
 *         You also can configure the start location of pid and no ack bit if you need.
 *         only generic mode need to configure this!!!
*/
gen_fsk_generic_header_t GEN_FSK_GenericHeader = {
    .h0_size     = 0,
    .length_size = 8,
    .h1_size     = 8,
    .h0_val      = 0,
    .length_val  = TX_PAYLOAD_LEN,
    .h1_val      = 0,
};

_attribute_ram_code_sec_ void rf_irq_handler(void)
{
    if (rf_get_irq_status(FLD_RF_IRQ_TX)) {
        tx_done_flag = 1;
        irq_cnt_tx++;
        rf_clr_irq_status(FLD_RF_IRQ_TX);
    } else {
        rf_clr_irq_status(FLD_RF_IRQ_ALL);
    }
}
PLIC_ISR_REGISTER(rf_irq_handler, IRQ_ZB_RT)

void user_gpio_init(void)
{
}

/**
 * @brief       user initialization when MCU wake_up from deepSleep_retention mode
 * @param[in]   none
 * @return      none
 */
void user_init_deepRetn(void)
{
}

void user_init_normal(void)
{
    user_gpio_init();
    #if (TLKAPI_DEBUG_ENABLE)
    tlkapi_debug_init();
    #endif
    unsigned char sync_word[7] = {0x53, 0x78, 0x56, 0x52};
    //generic FSK Link Layer configuratioin
    gen_fsk_packet_format_set(GEN_FSK_MODE_GENERIC_VARIABLE_FORMAT, TX_PAYLOAD_LEN);
    gen_fsk_datarate_set(GEN_FSK_DATARATE_1MBPS);    //Note that this API must be invoked first before all other APIs
    gen_fsk_preamble_len_set(8);
    gen_fsk_sync_word_len_set(SYNC_WORD_LEN_4BYTE);
    gen_fsk_sync_word_set(GEN_FSK_PIPE0, sync_word); //set pipe0's sync word
    gen_fsk_pipe_open(GEN_FSK_PIPE0);                //enable pipe0's reception
    gen_fsk_tx_pipe_set(GEN_FSK_PIPE0);              //set pipe0 as the TX pipe
    gen_fsk_set_crc_config(&crc_config);
    gen_fsk_radio_power_set(GEN_FSK_POWER_INDEX_P0p0dBm);
    gen_fsk_tx_buffer_set(TX_BUF_NUM, TX_BUF_LEN);
    rf_set_pri_generic_3bit_header_en();
    gen_fsk_channel_set(17);                     //set rf freq as 2408MHz,1M step
    gen_fsk_radio_state_set(GEN_FSK_STATE_AUTO); //set transceiver to basic RX state
    gen_fsk_tx_settle_set(149);

    //irq configuration
    plic_set_priority(IRQ_ZB_RT, 3);
    plic_interrupt_enable(IRQ_ZB_RT);
    rf_set_irq_mask(FLD_RF_IRQ_TX);
    rf_clr_irq_status(FLD_RF_IRQ_ALL);
    core_interrupt_enable();
}

unsigned int time_out_tick;

_attribute_no_inline_ void main_loop(void)
{
    ////////////////////////////////////// Debug entry /////////////////////////////////
    #if (TLKAPI_DEBUG_ENABLE)
    tlkapi_debug_handler();
    #endif

    tx_done_flag = 0;
    gen_fsk_write_payload(tx_buffer, tx_payload, TX_PAYLOAD_LEN);
    gen_fsk_stx_start(tx_buffer, rf_stimer_get_tick() + 100 * RF_SYSTEM_TIMER_TICK_1US);
    time_out_tick = rf_stimer_get_tick();
    while (tx_done_flag == 0 || clock_time_exceed(time_out_tick, 200 * 1000))
       ;
//    tx_payload[4]++;
    #if (UI_LED_ENABLE)
    gpio_toggle(GPIO_LED_WHITE);
    #endif
    delay_ms(200);
}

#endif
#endif
