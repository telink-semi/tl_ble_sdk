/********************************************************************************************************
 * @file    usbd_cdc.h
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
#ifndef __USBD_CDC_H__
#define __USBD_CDC_H__

#include "usb_cdc.h"

unsigned char usbd_cdc_interface_request_handler(unsigned char bus, usb_control_request_t const *setup, unsigned char setup_stage);
WEAK void     usbd_cdc_get_line_coding(unsigned char bus, unsigned char intf, cdc_line_coding_t *line_coding);
WEAK void     usbd_cdc_set_line_coding(unsigned char bus, unsigned char intf, cdc_line_coding_t *line_coding);
WEAK void     usbd_cdc_set_line_state(unsigned char bus, unsigned char intf, bool dtr, bool rts);

#endif
