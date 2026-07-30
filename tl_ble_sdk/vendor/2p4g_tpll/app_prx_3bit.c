/********************************************************************************************************
 * @file    app_prx_3bit.c
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
#if ((MCU_CORE_TYPE == MCU_CORE_TL323X))
#if (TPLL_MODE == TPLL_PRX_3BIT)
    #define PRX_PIPE        0
    #define PRI_FLT_MODE_EN 0
    #define TX_PAYLOAD_LEN  16
unsigned char ptx_buffer[TPLL_PIPE_NUM * TPLL_TX_FIFO_SIZE * TPLL_TX_FIFO_NUM] __attribute__((aligned(4))) = {};
unsigned char rx_buf[TPLL_PIPE_RX_FIFO_SIZE * TPLL_PIPE_RX_FIFO_NUM] __attribute__((aligned(4)))           = {};

volatile unsigned char  rx_payload[128] = {0};
static unsigned char    ack_payload[32] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f};
volatile unsigned int   Timestamp_value = 0;
volatile unsigned int   rx_interval_us  = 0;
TPLL_ReadRxPayload_t    ReadRxPayload;
volatile unsigned char  crc[4]         = {0};
volatile unsigned char *rx_packet      = NULL;
volatile unsigned int   rx_payload_len = 0;
volatile unsigned int   rx_timestamp   = 0;
volatile unsigned int   rx_header_len  = 0;
volatile signed char    rx_rssi        = 0;
volatile unsigned char  rx_pipe        = 0;
volatile unsigned char  rx_crc_len     = 0;
extern volatile unsigned char rx_flag, rx_dr_flag;
TPLL_CrcConfig_t        TPLL_CrcConfig = {
           .init_value    = 0xffffffff,
           .poly          = 0x00000007,
           .xor_out       = 0,
           .byte_order    = 1,
           .start_cal_pos = 0,
           .len           = 1,
};

/**
 *  @note  The rf_pkt_flt function matches from the high bit,
 *         You need to configure rf_pkt_mask and rf_pkt_match from the high bit when configuring it,
 *         the below takes generic packet for example.
*/
rf_pkt_flt_t TPLL_PktFlt = {
    .rf_pkt_flt_start       = 0,  //start at h0
    .rf_pkt_flt_end         = 9,
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
    .h0_size         = 0,
    .h1_size         = 8,
    .length_size     = 8,
    .h0_val          = 0, //0x22 to enable no ack
    .h1_val          = 0x0,
    .length_val      = TX_PAYLOAD_LEN,
    .pid_start_bit   = 14,
    .noack_start_bit = 13,
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

    delay_ms(1000);
    #if (TLKAPI_DEBUG_ENABLE)
    tlkapi_debug_init();
    #endif

    //rf configuration
    TPLL_SetFormatMode(TPLL_MODE_GENERIC_FORMAT);
    TPLL_SetBitrate(TPLL_BITRATE_2MBPS);
    TPLL_SetOutputPower(TPLL_POWER_INDEX_P0p0dBm);
    TPLL_SetAddressWidth(ADDRESS_WIDTH_5BYTES);
    TPLL_ClosePipe(TPLL_PIPE_ALL);
    #if PRX_PIPE == 0
    unsigned char rx_address[5] = {0xe7, 0xe7, 0xe7, 0xe7, 0xe7};
    TPLL_SetAddress(PRX_PIPE, rx_address);
        #if (PRI_FLT_MODE_EN)
    TPLL_PktFilter(TPLL_PktFlt);
        #endif
    TPLL_OpenPipe(TPLL_PIPE0);
    #endif


    #if PRX_PIPE == 1
    unsigned char rx_address1[5] = {0x55, 0x44, 0x33, 0x22, 0x11};
    TPLL_SetAddress(TPLL_PIPE1, rx_address1);
    TPLL_OpenPipe(TPLL_PIPE1);
    #endif

    #if PRX_PIPE == 2
    unsigned char rx_address2[5] = {0x55, 0x44, 0x33, 0x22, 0x11};
    TPLL_SetAddress(TPLL_PIPE1, rx_address2);
    rx_address2[0] = 0x22;
    TPLL_SetAddress(TPLL_PIPE2, &rx_address2[0]);
    TPLL_OpenPipe(TPLL_PIPE2);
    #endif

    #if PRX_PIPE == 3
    unsigned char rx_address3[5] = {0xaa, 0xbb, 0xcc, 0xdd, 0xee};
    TPLL_SetAddress(TPLL_PIPE1, rx_address3);
    rx_address3[0] = 0x33;
    TPLL_SetAddress(TPLL_PIPE3, &rx_address3[0]);
    TPLL_OpenPipe(TPLL_PIPE3);
    #endif

    #if PRX_PIPE == 4
    unsigned char rx_address4[5] = {0xaa, 0xbb, 0xcc, 0xdd, 0xee};
    TPLL_SetAddress(TPLL_PIPE1, rx_address4);
    rx_address4[0] = 0x44;
    TPLL_SetAddress(TPLL_PIPE4, &rx_address4[0]);
    TPLL_OpenPipe(TPLL_PIPE4);
    #endif

    #if PRX_PIPE == 5
    unsigned char rx_address5[5] = {0x55, 0x44, 0x33, 0x22, 0x11};
    TPLL_SetAddress(TPLL_PIPE1, rx_address5);
    rx_address5[0] = 0x55;
    TPLL_SetAddress(TPLL_PIPE5, &rx_address5[0]);
    TPLL_OpenPipe(TPLL_PIPE5);
    #endif

    TPLL_DmaInit(rx_buf, ptx_buffer);
    TPLL_ModeSet(TPLL_MODE_PRX);
    TPLL_SetRFChannel(4);
    TPLL_TxSettleSet(149);
    TPLL_RxSettleSet(85);
    TPLL_Preamble_Set(8);
    TPLL_CrcSet(TPLL_CrcConfig);

    rf_set_pri_generic_3bit_header_en();

    //configure irq
    core_interrupt_enable();
    plic_interrupt_enable(IRQ_ZB_RT);
    rf_clr_irq_mask(FLD_RF_IRQ_ALL);
    rf_set_irq_mask(FLD_RF_IRQ_TX | FLD_RF_IRQ_TX_DS | FLD_RF_IRQ_RX_DR | FLD_RF_IRQ_RX | FLD_RF_IRQ_PKT_UNMATCH | FLD_RF_IRQ_PKT_MATCH);

    TPLL_WriteAckPayload(PRX_PIPE, ptx_buffer, ack_payload, TX_PAYLOAD_LEN);
    TPLL_PRXTrig();
    rx_timestamp = reg_bb_timer_tick;
}

unsigned int time_out_tick;

_attribute_no_inline_ void main_loop(void)
{
    ////////////////////////////////////// Debug entry /////////////////////////////////
    #if (TLKAPI_DEBUG_ENABLE)
    tlkapi_debug_handler();
    #endif

    if (1 == rx_flag) {
    #if UI_LED_ENABLE
        gpio_toggle(GPIO_LED_GREEN);
    #endif
        rx_flag   = 0;
        rx_packet = TPLL_GetRxPacket(rx_buf);
        TPLL_GetRxPacketCrc(rx_packet, crc);

        ReadRxPayload  = TPLL_ReadRxPayload((unsigned char *)&rx_payload, rx_buf);
        rx_interval_us = (TPLL_GetTimestamp(rx_packet) - rx_timestamp) / SYSTEM_TIMER_TICK_1US; //bb timer 8M
        rx_payload_len = ReadRxPayload.rx_payload_len;
        rx_timestamp   = ReadRxPayload.rx_timestamp;
        rx_header_len  = ReadRxPayload.header_len;
        rx_rssi        = ReadRxPayload.rx_rssi;
        rx_pipe        = ReadRxPayload.rx_pipe;
        rx_crc_len     = ReadRxPayload.crc_len;
        time_out_tick  = rf_stimer_get_tick();
//        while (!TPLL_TxFifoEmpty(0) || clock_time_exceed(time_out_tick, 200 * 1000))
//            ;
        if (rx_dr_flag == 1) {
            rx_dr_flag = 0;
            ack_payload[4]++;
//            TPLL_WriteAckPayload(PRX_PIPE, ptx_buffer, ack_payload, TX_PAYLOAD_LEN);
        }
        TPLL_WriteAckPayload(PRX_PIPE, ptx_buffer, ack_payload, TX_PAYLOAD_LEN);
        printf("rx:0x%x %x %x %x %x %x\n", rx_packet[0], rx_packet[1], rx_packet[2], rx_packet[3], rx_packet[4], rx_packet[5]);
        printf("rx:0x%x %x %x %x %x %x\n", rx_packet[6], rx_packet[7], rx_packet[8], rx_packet[9], rx_packet[10], rx_packet[11]);
    }
}
#endif
#endif
