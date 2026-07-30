/********************************************************************************************************
 * @file    usbd_audio.c
 *
 * @brief   This is the source file for Telink RISC-V MCU
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
#include "usbd_audio.h"

#ifndef USBD_AUDIO_INTERFACE_NUM
    #define USBD_AUDIO_INTERFACE_NUM 2 /* default 1 audio interface */
#endif

typedef struct usbd_audio
{
    unsigned char cur[3];
} usbd_audio_interface_t;

usbd_audio_interface_t usbd_audio[USBD_AUDIO_INTERFACE_NUM];

unsigned char usbd_audio_interface_request_handler(unsigned char bus, usb_control_request_t const *setup, unsigned char setup_stage)
{
    (void)bus;
    (void)setup;
    (void)setup_stage;

    unsigned char const audio_alt_itf = U16_LOW_BYTE(setup->wValue);
    unsigned char const audio_itf     = U16_LOW_BYTE(setup->wIndex);

    switch (setup->bmRequestType_bit.type) {
    case 0:
    {
        /* standard request. */
        switch (setup->bRequest) {
        case USB_REQ_SET_INTERFACE:
        {
            usb_log("set interface\r\n");
            // if (usbd_set_interface_callback)
            //     usbd_set_interface_callback(bus, audio_itf, audio_alt_itf);
            if (audio_alt_itf) {
                usbd_audio_open(bus, audio_itf);
            } else {
                usbd_audio_close(bus, audio_itf);
            }
            usbd_ep_write(bus, 0, 0, 0);
            break;
        }
        default:
            return false;
            break;
        }
        break;
    }
    case 1:
    {
        /* class request. */
        switch (setup->bmRequestType_bit.recipient) {
        case 1:
        {
            /* recipient interface */
            if (setup_stage) {
                if (((setup->wLength) && ((setup->bmRequestType_bit.direction) == USB_DIR_OUT))) {
                    usbd_ep_read(bus, 0, usbd_audio[0].cur, setup->wLength);
                    return true;
                }

                if (usbd_audio_interface_cb) {
                    return usbd_audio_interface_cb(bus, setup);
                } else {
                    return false;
                }
            }
        } break;

        case 2:
        {
            /* recipient endpoint. */
            if (setup_stage) {
                /* setup stage. */
                usbd_ep_read(bus, 0, usbd_audio[0].cur, setup->wLength);
            } else {
                usbd_ep_write(bus, 0, 0, 0);

                //                if (usbd_audio_endpoint_cb)
                //                {
                //                    return usbd_audio_endpoint_cb(bus, setup);
                //                }
                //                else
                //                {
                //                    return false;
                //                }
            }

            break;
        }
        default:
            return false;
            break;
        }
        break;
    }
    case 2:
    {
        /* vendor request. */
        return false;
        break;
    }
    default:
        return false;
        break;
    }

    return true;
}
