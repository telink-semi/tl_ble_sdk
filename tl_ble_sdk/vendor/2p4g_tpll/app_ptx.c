/********************************************************************************************************
 * @file    app_ptx.c
 *
 * @brief   This is the source file for 2.4G SDK
 *
 * @author  2.4G Group
 * @date    2023
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

#if (TPLL_MODE == TPLL_PTX)
    #define PTX_PIPE        0
    #define PRI_FLT_MODE_EN 0
    #define TX_PAYLOAD_LEN  16
unsigned char ptx_buffer[TPLL_TX_FIFO_SIZE + TPLL_PIPE_NUM * TPLL_TX_FIFO_SIZE * TPLL_TX_FIFO_NUM] __attribute__((aligned(4))) = {}; //tx buffer should big than pipe_num *fifo_size*32
unsigned char rx_buf[TPLL_PIPE_RX_FIFO_SIZE * TPLL_PIPE_RX_FIFO_NUM] __attribute__((aligned(4)))           = {};

static unsigned char   tx_data[32]   = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0x20};
volatile unsigned char preamble_len  = 0;
volatile unsigned char tmp           = 1;

extern volatile unsigned char rx_flag, ds_flag, maxretry_flag, tx_done_flag, rx_dr_flag;

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
    .length_size     = 8, // must be <=8
    .h0_val          = 0, //no ack 0x22
    .h1_val          = 0,
    .length_val      = TX_PAYLOAD_LEN,
    .pid_start_bit   = 0,  //start at h0
    .noack_start_bit = 2,
};

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

    //rf configuration
    TPLL_SetFormatMode(TPLL_MODE_GENERIC_FORMAT);
    TPLL_SetBitrate(TPLL_BITRATE_2MBPS);
    TPLL_SetOutputPower(TPLL_POWER_INDEX_P0p0dBm);
    TPLL_SetAddressWidth(ADDRESS_WIDTH_5BYTES);
    #if (PRI_FLT_MODE_EN)
    TPLL_PktFilter(TPLL_PktFlt);
    #endif
    TPLL_ClosePipe(TPLL_PIPE_ALL);

    #if PTX_PIPE == 0
    unsigned char tx_address[5] = {0xe7, 0xe7, 0xe7, 0xe7, 0xe7}; //{0xaa,0xbb,0xcc,0xdd,0xee};
    TPLL_SetAddress(TPLL_PIPE0, tx_address);
    TPLL_OpenPipe(TPLL_PIPE0);
    TPLL_SetTXPipe(TPLL_PIPE0);
    #endif


    #if PTX_PIPE == 1
    unsigned char tx_address1[5] = {0x55, 0x44, 0x33, 0x22, 0x11};
    TPLL_SetAddress(TPLL_PIPE1, tx_address1);
    TPLL_OpenPipe(TPLL_PIPE1);
    TPLL_SetTXPipe(TPLL_PIPE1);
    #endif

    #if PTX_PIPE == 2
    unsigned char tx_address2[5] = {0x55, 0x44, 0x33, 0x22, 0x11};
    TPLL_SetAddress(TPLL_PIPE1, tx_address2);
    tx_address2[0] = 0x22;
    TPLL_SetAddress(TPLL_PIPE2, &tx_address2[0]);
    TPLL_OpenPipe(TPLL_PIPE2);
    TPLL_SetTXPipe(TPLL_PIPE2);
    #endif

    #if PTX_PIPE == 3
    unsigned char tx_address3[5] = {0x55, 0x44, 0x33, 0x22, 0x11};
    TPLL_SetAddress(TPLL_PIPE1, tx_address3);
    tx_address3[0] = 0x33;
    TPLL_SetAddress(TPLL_PIPE3, &tx_address3[0]);
    TPLL_OpenPipe(TPLL_PIPE3);
    TPLL_SetTXPipe(TPLL_PIPE3);
    #endif

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

    TPLL_WriteTxPayload(PTX_PIPE, ptx_buffer, (unsigned char *)tx_data, 16);
    TPLL_PTXTrig();
}

_attribute_no_inline_ void main_loop(void)
{
    ////////////////////////////////////// Debug entry /////////////////////////////////
    #if (TLKAPI_DEBUG_ENABLE)
    tlkapi_debug_handler();
    #endif

    if (ds_flag || maxretry_flag) {
        if (ds_flag) {
    #if UI_LED_ENABLE
            gpio_toggle(GPIO_LED_GREEN);
    #endif
        }
        ds_flag       = 0;
        maxretry_flag = 0;
        delay_ms(500);
        tx_data[4]++;
        tmp = TPLL_WriteTxPayload(PTX_PIPE, ptx_buffer, (unsigned char *)tx_data, TX_PAYLOAD_LEN);
        if (!tmp) {
            TPLL_PTXTrig();
        }
    }
}
#endif
