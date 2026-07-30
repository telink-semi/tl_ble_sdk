/********************************************************************************************************
 * @file    usbd_cdc.c
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
#include "usbd_cdc.h"

#ifndef USBD_CDC_INTERFACE_NUM
    #define USBD_CDC_INTERFACE_NUM 1 /* default 1 cdc interface */
#endif

typedef struct usbd_cdc
{
    bool              dtr;
    bool              rts;
    cdc_line_coding_t line_coding;
} usbd_cdc_interface_t;

static usbd_cdc_interface_t usbd_cdc[USBD_CDC_INTERFACE_NUM];

unsigned char usbd_cdc_interface_request_handler(unsigned char bus, usb_control_request_t const *setup, unsigned char setup_stage)
{
    (void)bus;

    unsigned char const cdc_itf = U16_LOW_BYTE(setup->wIndex);

    switch (setup->bmRequestType_bit.type) {
    case 0:
    {
        /* standard request. */
        return false;
        break;
    }
    case 1:
    {
        /* class request. */
        switch (setup->bRequest) {
        case CDC_REQUEST_SET_LINE_CODING:
        {
            if (setup_stage) {
                /* setup stage. */
                usbd_ep_read(bus, 0, (unsigned char *)&usbd_cdc[cdc_itf].line_coding, setup->wLength);
            } else {
                usbd_cdc_set_line_coding(bus, cdc_itf, &usbd_cdc[cdc_itf].line_coding);
            }
            break;
        }
        case CDC_REQUEST_GET_LINE_CODING:
        {
            usb_log("cdc get line coding = %d\r\n", cdc_itf);
            usbd_cdc_get_line_coding(bus, cdc_itf, &usbd_cdc[cdc_itf].line_coding);
            usbd_ep_write(bus, 0, (unsigned char *)&usbd_cdc[cdc_itf].line_coding, sizeof(cdc_line_coding_t));
            break;
        }
        case CDC_REQUEST_SET_CONTROL_LINE_STATE:
        {
            usbd_cdc[cdc_itf].dtr = (setup->wValue & 0x0001);
            usbd_cdc[cdc_itf].rts = (setup->wValue & 0x0002);
            usbd_cdc_set_line_state(bus, cdc_itf, usbd_cdc[cdc_itf].dtr, usbd_cdc[cdc_itf].rts);
            usbd_ep_write(bus, 0, 0, 0);
            break;
        }
        case CDC_REQUEST_SEND_BREAK:
            return false;
            break;
        default:
            return false; // stall unsupported request
            break;
        }
        break;
    }
    case 2:
        /* vendor request. */
        return false;
        break;
    default:
        return false;
        break;
    }

    return true;
}

WEAK void usbd_cdc_get_line_coding(unsigned char bus, unsigned char intf, cdc_line_coding_t *line_coding)
{
    (void)bus;

    if (usbd_cdc[intf].line_coding.bit_rate == 0) {
        /* default line coding. */
        line_coding->bit_rate  = 115200;
        line_coding->data_bits = 8;
        line_coding->parity    = 0;
        line_coding->stop_bits = 0;
    }
}

WEAK void usbd_cdc_set_line_coding(unsigned char bus, unsigned char intf, cdc_line_coding_t *line_coding)
{
    (void)bus;
    (void)intf;
    (void)line_coding;
}

WEAK void usbd_cdc_set_line_state(unsigned char bus, unsigned char intf, bool dtr, bool rts)
{
    (void)bus;
    (void)intf;
    (void)dtr;
    (void)rts;
}
