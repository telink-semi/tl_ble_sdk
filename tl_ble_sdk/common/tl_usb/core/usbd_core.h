/********************************************************************************************************
 * @file    usbd_core.h
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
#ifndef __USB_CODE_H__
#define __USB_CODE_H__

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "../common/usb_log.h"
#include "../common/usb_def.h"
#include "../common/usb_util.h"
#include "../common/usbd_config.h"
#include "compiler.h"

typedef unsigned char (*usbd_request_handler)(unsigned char rhport, usb_control_request_t const *setup, unsigned char setup_stage);
typedef void (*usbd_endpoint_callback)(unsigned char busid, unsigned char ep, unsigned int len);
typedef void (*tl_usb_event_callback)(void);

typedef struct
{
    usbd_request_handler usbd_driver_handler;
    unsigned char        driver_num;
} usbd_driver_t;

unsigned char usbd_control_request_process(const unsigned char bus, const usb_control_request_t *setup, unsigned char setup_stage);
void          usbd_bus_reset(unsigned char bus);
WEAK void     usbd_set_configuration_callback(unsigned char bus, unsigned char config_num);
WEAK void     usbd_set_interface_callback(unsigned char bus, unsigned char intf, unsigned char alt_intf);
void usbd_ep0_in_complete_callback(unsigned char bus, unsigned char ep, unsigned int len);
void usbd_ep0_out_complete_callback(unsigned char bus, unsigned char ep, unsigned int len);

unsigned char *usbd_get_device_descriptor(unsigned char bus);
unsigned char *usbd_get_config_descriptor(unsigned char bus);
unsigned char *usbd_get_string_descriptor(unsigned char bus, unsigned char string_index);
unsigned char *usbd_get_device_qualifier_descriptor(unsigned char bus);
unsigned char *usbd_get_other_speed_configuration_descriptor(unsigned char bus, unsigned char index);

void usbd_ep_open(unsigned char bus, usb_endpoint_descriptor_t *ep_addr);

void usbd_ep_write(unsigned char bus, const unsigned char ep_addr, unsigned char *buf, unsigned int len);
void usbd_ep_read(unsigned char bus, const unsigned char ep_addr, unsigned char *buf, unsigned int len);
void usbd_ep_stall(unsigned char bus, const unsigned char ep_addr);
void usbd_ep_clear_stall(unsigned char bus, const unsigned char ep_addr);
void usbd_set_address(unsigned char bus, unsigned char address);
void usbd_test_mode(unsigned char bus, unsigned char test_mode);

void usbd_driver_register(unsigned char bus, usbd_driver_t *driver, unsigned char intf, unsigned char ep_addr);
void usbd_endpoint_register(unsigned char bus, unsigned char ep_addr, usbd_endpoint_callback callback);

void usbd_epin_complete_handler(unsigned char bus, unsigned char ep_index, unsigned int len);
void usbd_epout_complete_handler(unsigned char bus, unsigned char ep_index, unsigned int len);

void            usbd_endpoint_set_status(unsigned char bus, unsigned char ep_addr, unsigned short ep_status);
unsigned short *usbd_endpoint_get_status(unsigned char bus, unsigned char ep_addr);

void tl_usbd_enum_done_register(tl_usb_event_callback callback);
void tl_usbd_sof_register(tl_usb_event_callback callback);
void tl_usbd_reset_register(tl_usb_event_callback callback);

void usbd_suspend_callback(unsigned char bus);
void usbd_resume_callback(unsigned char bus);

#endif
