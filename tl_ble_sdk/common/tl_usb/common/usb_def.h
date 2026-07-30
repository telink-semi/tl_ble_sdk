/********************************************************************************************************
 * @file    usb_def.h
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
#ifndef __USB_DEF_H__
#define __USB_DEF_H__

#include <stdio.h>
#include "usb_util.h"

typedef enum
{
    USB_DESC_DEVICE                = 0x01,
    USB_DESC_CONFIGURATION         = 0x02,
    USB_DESC_STRING                = 0x03,
    USB_DESC_INTERFACE             = 0x04,
    USB_DESC_ENDPOINT              = 0x05,
    USB_DESC_DEVICE_QUALIFIER      = 0x06,
    USB_DESC_OTHER_SPEED_CONFIG    = 0x07,
    USB_DESC_INTERFACE_POWER       = 0x08,
    USB_DESC_OTG                   = 0x09,
    USB_DESC_DEBUG                 = 0x0A,
    USB_DESC_INTERFACE_ASSOCIATION = 0x0B,

    USB_DESC_BOS               = 0x0F,
    USB_DESC_DEVICE_CAPABILITY = 0x10,

    USB_DESC_FUNCTIONAL = 0x21,

    USB_DESC_CS_DEVICE        = 0x21,
    USB_DESC_CS_CONFIGURATION = 0x22,
    USB_DESC_CS_STRING        = 0x23,
    USB_DESC_CS_INTERFACE     = 0x24,
    USB_DESC_CS_ENDPOINT      = 0x25,

    USB_DESC_SUPERSPEED_ENDPOINT_COMPANION     = 0x30,
    USB_DESC_SUPERSPEED_ISO_ENDPOINT_COMPANION = 0x31
} usb_desc_type_e;

typedef enum
{
    USB_REQ_GET_STATUS        = 0,
    USB_REQ_CLEAR_FEATURE     = 1,
    USB_REQ_RESERVED          = 2,
    USB_REQ_SET_FEATURE       = 3,
    USB_REQ_RESERVED2         = 4,
    USB_REQ_SET_ADDRESS       = 5,
    USB_REQ_GET_DESCRIPTOR    = 6,
    USB_REQ_SET_DESCRIPTOR    = 7,
    USB_REQ_GET_CONFIGURATION = 8,
    USB_REQ_SET_CONFIGURATION = 9,
    USB_REQ_GET_INTERFACE     = 10,
    USB_REQ_SET_INTERFACE     = 11,
    USB_REQ_SYNCH_FRAME       = 12
} usb_request_code_e;

typedef enum
{
    USB_REQ_FEATURE_EDPT_HALT     = 0,
    USB_REQ_FEATURE_REMOTE_WAKEUP = 1,
    USB_REQ_FEATURE_TEST_MODE     = 2
} usb_request_feature_selector_t;

typedef enum
{
    USB_FEATURE_TEST_J = 1,
    USB_FEATURE_TEST_K,
    USB_FEATURE_TEST_SE0_NAK,
    USB_FEATURE_TEST_PACKET,
    USB_FEATURE_TEST_FORCE_ENABLE,
} usb_feature_test_mode_t;

typedef enum
{
    USB_DIR_OUT = 0,
    USB_DIR_IN  = 1,

    USB_DIR_IN_MASK = 0x80
} usb_dir_e;

typedef struct PACKED
{
    union
    {
        struct PACKED
        {
            unsigned char recipient : 5;
            unsigned char type      : 2;
            unsigned char direction : 1;
        } bmRequestType_bit;

        unsigned char bmRequestType;
    };

    unsigned char  bRequest;
    unsigned short wValue;
    unsigned short wIndex;
    unsigned short wLength;
} usb_control_request_t;

typedef struct PACKED
{
    unsigned char  bLength;
    unsigned char  bDescriptorType;
    unsigned char  bEndpointAddress;
    unsigned char  bmAttributes;
    unsigned short wMaxPacketSize;
    unsigned char  bInterval;
} usb_endpoint_descriptor_t;

// Get direction from Endpoint address
static inline usb_dir_e usb_get_ep_dir(unsigned char addr)
{
    return (addr & USB_DIR_IN_MASK) ? USB_DIR_IN : USB_DIR_OUT;
}

// Get Endpoint number from address
static inline unsigned char usb_get_ep_number(unsigned char addr)
{
    return (unsigned char)(addr & (~USB_DIR_IN_MASK));
}

#endif
