/********************************************************************************************************
 * @file    usbd_audio.h
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
#ifndef __USBD_AUDIO_H__
#define __USBD_AUDIO_H__

#include "usb_audio.h"

unsigned char      usbd_audio_interface_request_handler(unsigned char bus, usb_control_request_t const *setup, unsigned char setup_stage);
WEAK unsigned char usbd_audio_interface_cb(unsigned char bus, usb_control_request_t const *setup);
WEAK unsigned char usbd_audio_endpoint_cb(unsigned char bus, usb_control_request_t const *setup);

void usbd_audio_open(unsigned char bus, unsigned char intf);
void usbd_audio_close(unsigned char bus, unsigned char intf);

#endif
