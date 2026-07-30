/********************************************************************************************************
 * @file    usb_cdc.h
 *
 * @brief   This is the header file for Telink RISC-V MCU
 *
 * @author  Driver Group
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
#ifndef __USB_CDC_H__
#define __USB_CDC_H__

#include "../../core/usbd_core.h"

typedef struct PACKED
{
    unsigned int  bit_rate;
    unsigned char stop_bits;
    unsigned char parity;
    unsigned char data_bits;
} cdc_line_coding_t;

typedef enum
{
    CDC_REQUEST_SEND_ENCAPSULATED_COMMAND = 0x00,
    CDC_REQUEST_GET_ENCAPSULATED_RESPONSE = 0x01,
    CDC_REQUEST_SET_COMM_FEATURE          = 0x02,
    CDC_REQUEST_GET_COMM_FEATURE          = 0x03,
    CDC_REQUEST_CLEAR_COMM_FEATURE        = 0x04,

    CDC_REQUEST_SET_AUX_LINE_STATE = 0x10,
    CDC_REQUEST_SET_HOOK_STATE     = 0x11,
    CDC_REQUEST_PULSE_SETUP        = 0x12,
    CDC_REQUEST_SEND_PULSE         = 0x13,
    CDC_REQUEST_SET_PULSE_TIME     = 0x14,
    CDC_REQUEST_RING_AUX_JACK      = 0x15,

    CDC_REQUEST_SET_LINE_CODING        = 0x20,
    CDC_REQUEST_GET_LINE_CODING        = 0x21,
    CDC_REQUEST_SET_CONTROL_LINE_STATE = 0x22,
    CDC_REQUEST_SEND_BREAK             = 0x23,

    CDC_REQUEST_DIAL_DIGITS          = 0x36,
    CDC_REQUEST_SET_UNIT_PARAMETER   = 0x37,
    CDC_REQUEST_GET_UNIT_PARAMETER   = 0x38,
    CDC_REQUEST_CLEAR_UNIT_PARAMETER = 0x39,
    CDC_REQUEST_GET_PROFILE          = 0x3A,

    CDC_REQUEST_SET_ETHERNET_MULTICAST_FILTERS               = 0x40,
    CDC_REQUEST_SET_ETHERNET_POWER_MANAGEMENT_PATTERN_FILTER = 0x41,
    CDC_REQUEST_GET_ETHERNET_POWER_MANAGEMENT_PATTERN_FILTER = 0x42,
    CDC_REQUEST_SET_ETHERNET_PACKET_FILTER                   = 0x43,
    CDC_REQUEST_GET_ETHERNET_STATISTIC                       = 0x44,

    CDC_REQUEST_SET_ATM_DATA_FORMAT       = 0x50,
    CDC_REQUEST_GET_ATM_DEVICE_STATISTICS = 0x51,
    CDC_REQUEST_SET_ATM_DEFAULT_VC        = 0x52,
    CDC_REQUEST_GET_ATM_VC_STATISTICS     = 0x53,

    CDC_REQUEST_MDLM_SEMANTIC_MODEL = 0x60,
} cdc_management_request_t;

#endif
