/********************************************************************************************************
 * @file    usbd_hid.c
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
#include "usbd_hid.h"

#ifndef USBD_HID_INTERFACE_NUM
    #define USBD_HID_INTERFACE_NUM 1 /* default 1 hid interface */
#endif

// typedef struct usbd_hid
// {
//     unsigned char idle_rate;
//     unsigned char protocol_mode;
//     unsigned char report;
//     unsigned char reg_data[8];
// } usbd_hid_interface_t;

usbd_hid_interface_t usbd_hid[USBD_HID_INTERFACE_NUM];

void usbd_hid_reset_interface_data(void)
{
    for(int i=0;i<USBD_HID_INTERFACE_NUM;i++)
    {
        usbd_hid[i].protocol_mode=1;
    }
    
}
_attribute_ram_code_sec_ unsigned char usbd_hid_request_descriptor(unsigned char bus, usb_control_request_t const *setup)
{
    (void)bus;

    unsigned char      *desc_ptr = 0;
    unsigned char const hid_itf  = U16_LOW_BYTE(setup->wIndex);
    unsigned char       hid_desc = U16_HIGH_BYTE(setup->wValue);

    switch (hid_desc) {
    case HID_DESC_TYPE_HID:
    {
        unsigned char *config_desc      = 0;
        unsigned short current_desc_len = 0;
        unsigned char  cur_iface        = 0xFF;

        config_desc                    = usbd_get_config_descriptor(bus);
        unsigned short config_desc_len = config_desc[2] | (config_desc[3] << 8);

        while (config_desc[0] != 0U) {
            switch (config_desc[1]) {
            case 0x02:
                break;
            case 0x04:
                cur_iface = config_desc[2];
                break;
            case 0x21:
                if (cur_iface == hid_itf) {
                    /* hid descriptor. */
                    usbd_ep_write(bus, 0, config_desc, MIN(setup->wLength, config_desc[0]));
                    break;
                }
            default:
                break;
            }
            config_desc += config_desc[0];
            current_desc_len += config_desc[0];
            if (current_desc_len >= config_desc_len) {
                break;
            }
        }
    } break;
    case HID_DESC_TYPE_REPORT:
    {
        unsigned short len;
        desc_ptr = usbd_hid_get_report_descriptor(bus, setup->wIndex, &len);
        if (desc_ptr) {
            // unsigned char *config_desc = usbd_get_config_descriptor(bus);
            usb_log("total len %d\r\n", MIN(setup->wLength, len));
            usbd_ep_write(bus, 0, desc_ptr, MIN(setup->wLength, len));
        }
        break;
    }
    case HID_DESC_TYPE_PHYSICAL:
        return false;
        break;
    default:
        return false;
        break;
    }

    return true;
}

_attribute_ram_code_sec_ unsigned char usbd_hid_interface_request_handler(unsigned char bus, usb_control_request_t const *setup, unsigned char setup_stage)
{
    (void)bus;

    unsigned char const hid_itf = U16_LOW_BYTE(setup->wIndex);

    switch (setup->bmRequestType_bit.type) {
    case 0:
    {
        /* standard request. */
        switch (setup->bRequest) {
        case USB_REQ_GET_STATUS:
        {
            usb_log("get status\r\n");
            unsigned short *ep_status;
            ep_status = usbd_endpoint_get_status(bus, U16_LOW_BYTE(setup->wIndex));
            usbd_ep_write(bus, 0, (unsigned char *)ep_status, 2);
            break;
        }
        case USB_REQ_CLEAR_FEATURE:
        {
            usb_log("clear feature\r\n");
            if ((setup->bmRequestType_bit.recipient == 2) && (setup->wValue == USB_REQ_FEATURE_EDPT_HALT)) /* endpoint_halt. */
            {
                usbd_endpoint_set_status(bus, U16_LOW_BYTE(setup->wIndex), 0);
                usbd_ep_clear_stall(bus, U16_LOW_BYTE(setup->wIndex));
                usbd_ep_write(bus, 0, 0, 0);
            } else {
                return false;
            }
            break;
        }
        case USB_REQ_SET_FEATURE:
        {
            usb_log("set ep feature\r\n");
            usb_log("  setup data: ");
            usb_log("0x%x 0x%x 0x%x 0x%x 0x%x\r\n", setup->bmRequestType, setup->bRequest, setup->wValue, setup->wIndex, setup->wLength);
            usb_log("\r\n");
            if ((setup->bmRequestType_bit.recipient == 2) && (setup->wValue == USB_REQ_FEATURE_EDPT_HALT)) /* endpoint_halt. */
            {
                usbd_endpoint_set_status(bus, U16_LOW_BYTE(setup->wIndex), (unsigned short)(1 << 0));
                usbd_ep_stall(bus, U16_LOW_BYTE(setup->wIndex));
                usbd_ep_write(bus, 0, 0, 0);
            } else {
                return false;
            }
            break;
        }
        case USB_REQ_GET_DESCRIPTOR:
        {
            usb_log("hid get descriptor\r\n");
            usbd_hid_request_descriptor(bus, setup);
            break;
        }
        default:
            return false;
            break;
        }
    } break;
    case 1:
    {
        /* class request. */
        switch (setup->bRequest) {
        case HID_REQ_CONTROL_GET_REPORT:
            return false;
            break;
        case HID_REQ_CONTROL_GET_IDLE:
        {
            usb_log("hid get idle hid_itf = %d\r\n", hid_itf);
            usbd_ep_write(bus, 0, &usbd_hid[hid_itf].idle_rate, 1);
            break;
        }
        case HID_REQ_CONTROL_GET_PROTOCOL:
        {
            usbd_ep_write(bus, 0, &usbd_hid[hid_itf].protocol_mode, 1);
            break;
        }
        case HID_REQ_CONTROL_SET_REPORT:
        {
            if (setup_stage) {
                /* setup stage. */
                usbd_ep_read(bus, 0, usbd_hid[hid_itf].reg_data, setup->wLength);
            } else {
                usbd_hid_set_report(bus, hid_itf, U16_LOW_BYTE(setup->wValue), U16_HIGH_BYTE(setup->wValue), usbd_hid[hid_itf].reg_data, setup->wLength);
            }
            break;
        }
        case HID_REQ_CONTROL_SET_IDLE:
        {
            usb_log("hid set idle hid_itf = %d\r\n", hid_itf);
            usbd_hid[hid_itf].idle_rate = U16_HIGH_BYTE(setup->wValue);
            usbd_ep_write(bus, 0, 0, 0);
        #if APP_CHANGE_HID_NABLE
            app_usb_status_set(IDLE_SET_FLAG_APP);
        #endif
            break;
        }
        case HID_REQ_CONTROL_SET_PROTOCOL:
        {
            usbd_hid[hid_itf].protocol_mode = U16_LOW_BYTE(setup->wValue);
            // #if APP_CHANGE_HID_NABLE
            // if(setup->wValue&0x01)
            // {
            //     app_usb_status_clear(BIOS_MODE_FLAG_APP);
            // }
            // else
            // {
            //     app_usb_status_set(BIOS_MODE_FLAG_APP);
            // }
            // #endif
            usbd_ep_write(bus, 0, 0, 0);
            break;
        }
        default:
            return false;
            break;
        }
    } break;
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

_attribute_ram_code_sec_ WEAK void usbd_hid_set_report(unsigned char bus, unsigned char hid_itf, unsigned char report_id, unsigned char report_type, unsigned char *report, unsigned short report_len)
{
    (void)bus;
    (void)hid_itf;
    (void)report_id;
    (void)report_type;
    (void)report;
    (void)report_len;
    //printf("set_report=%d,%d",report_len,report[0]);
    #if APP_CHANGE_HID_NABLE 
    app_usb_status_set(REPORT_SET_FLAG_APP);

    extern void app_hid_set_report(unsigned char bus, unsigned char hid_itf, unsigned char report_id, unsigned char report_type, unsigned char *report, unsigned short report_len);
    app_hid_set_report(bus,  hid_itf,report_id,report_type,report, report_len);
    #endif
    
    
}
