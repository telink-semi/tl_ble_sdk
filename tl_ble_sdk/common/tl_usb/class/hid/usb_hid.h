/********************************************************************************************************
 * @file    usb_hid.h
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
#ifndef __USB_HID_H__
#define __USB_HID_H__
#include "../../core/usbd_core.h"

typedef enum
{
    HID_DESC_TYPE_HID      = 0x21, /*< HID Descriptor */
    HID_DESC_TYPE_REPORT   = 0x22, /*< Report Descriptor */
    HID_DESC_TYPE_PHYSICAL = 0x23, /*< Physical Descriptor */
} hid_descriptor_e;

typedef enum
{
    HID_REQ_CONTROL_GET_REPORT   = 0x01, /*< Get Report */
    HID_REQ_CONTROL_GET_IDLE     = 0x02, /*< Get Idle */
    HID_REQ_CONTROL_GET_PROTOCOL = 0x03, /*< Get Protocol */
    HID_REQ_CONTROL_SET_REPORT   = 0x09, /*< Set Report */
    HID_REQ_CONTROL_SET_IDLE     = 0x0a, /*< Set Idle */
    HID_REQ_CONTROL_SET_PROTOCOL = 0x0b, /*< Set Protocol */
} hid_request_e;

#endif
