/********************************************************************************************************
 * @file    usbd_core.c
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
#include "usbd_core.h"
#include "../class/hid/usbd_hid.h"
#include "gpio.h"
#include "tl_string.h"
#ifndef USBD_BUS_NUM
    #define USBD_BUS_NUM 1
#endif

volatile unsigned char g_usbd_remote_wakeup_en = 1;

typedef struct
{
    usb_control_request_t  setup;
    unsigned char          address;
    unsigned char          config_value;
    unsigned char          test_mode;
    unsigned short         device_status;
    usbd_driver_t         *driver[USBD_SUPPORT_DRIVER_MAX];
    unsigned char          itf2drv[USBD_SUPPORT_INTERFACE_MAX];
    unsigned char          ep2drv[USBD_SUPPORT_ENDPOINT_MAX][USBD_ENDPOINT_DIRECTION];
    unsigned short         ep_status[USBD_SUPPORT_ENDPOINT_MAX][USBD_ENDPOINT_DIRECTION];
    usbd_endpoint_callback epin_cb[USBD_SUPPORT_ENDPOINT_MAX];
    usbd_endpoint_callback epout_cb[USBD_SUPPORT_ENDPOINT_MAX];
} usbd_t;

_attribute_aligned_(4) static usbd_t usbd_info[USBD_BUS_NUM];

_attribute_ram_code_sec_ void usbd_ep0_in_complete_callback(unsigned char bus, unsigned char ep, unsigned int len)
{
    (void)bus;
    (void)ep;
    (void)len;

    if (usbd_info[bus].test_mode)
    {
        usbd_info[bus].test_mode = false;
        unsigned char test_mode  = U16_HIGH_BYTE(usbd_info[bus].setup.wIndex);
        usbd_test_mode(bus, test_mode);
    }

    (void)bus;
}

_attribute_ram_code_sec_ void usbd_ep0_out_complete_callback(unsigned char bus, unsigned char ep, unsigned int len)
{
    (void)bus;
    (void)ep;
    (void)len;
}

_attribute_ram_code_sec_ void usbd_bus_reset(unsigned char bus)
{
    (void)bus;
    usbd_info[bus].address       = 0;
    usbd_info[bus].config_value  = 0;
    usbd_info[bus].device_status = 0;
    tmemset(usbd_info[bus].ep_status, 0, USBD_SUPPORT_ENDPOINT_MAX * USBD_ENDPOINT_DIRECTION * sizeof(short));

    usbd_info[bus].epin_cb[0] = usbd_ep0_in_complete_callback;
    usbd_info[bus].epout_cb[0] = usbd_ep0_out_complete_callback;
    #if APP_CHANGE_HID_NABLE

    extern void app_usb_bus_reset_init(void);
    app_usb_bus_reset_init();
    app_usb_status_clear(0xffff);
    #endif
}

_attribute_ram_code_sec_ static unsigned char usbd_get_descriptor_process(const unsigned char bus, const usb_control_request_t *setup)
{
    (void)bus;
    unsigned char *desc_ptr = 0;

    unsigned char desc_type = U16_HIGH_BYTE(setup->wValue);
    switch (desc_type) {
    case USB_DESC_DEVICE:
    {
        usb_log("get device descriptor\r\n");
        desc_ptr = usbd_get_device_descriptor(bus);
        if (desc_ptr) {
            usbd_ep_write(bus, 0, desc_ptr, MIN((unsigned int)desc_ptr[0], setup->wLength));
        } else {
            return false;
        }
        break;
    }
    case USB_DESC_CONFIGURATION:
    {
        usb_log("get config descriptor\r\n");
        desc_ptr = usbd_get_config_descriptor(bus);
        if (desc_ptr) {
            unsigned int write_len = (desc_ptr[3] << 8) | desc_ptr[2]; /* total len. */
            usbd_ep_write(bus, 0, desc_ptr, MIN(write_len, setup->wLength));
        } else {
            return false;
        }
        break;
    }
    case USB_DESC_STRING:
    {
        usb_log("get string descriptor index = %d\r\n", setup->wValue & 0xff);
        desc_ptr = usbd_get_string_descriptor(bus, U16_LOW_BYTE(setup->wValue));
        if (desc_ptr) {
            usbd_ep_write(bus, 0, desc_ptr, MIN(desc_ptr[0], setup->wLength));
        } else {
            return false;
        }
        break;
    }
    case USB_DESC_DEVICE_QUALIFIER:
    {
        usb_log("get device qualifier descriptor\r\n");
        desc_ptr = usbd_get_device_qualifier_descriptor(bus);
        if (desc_ptr) {
            usbd_ep_write(bus, 0, desc_ptr, MIN(desc_ptr[0], setup->wLength));
        } else {
            return false;
        }
        break;
    }
    case USB_DESC_OTHER_SPEED_CONFIG:
    {
        usb_log("get other speed descriptor\r\n");
        desc_ptr = usbd_get_other_speed_configuration_descriptor(bus, U16_LOW_BYTE(setup->wValue));
        if (desc_ptr) {
            unsigned int write_len = (desc_ptr[3] << 8) | desc_ptr[2]; /* total len. */
            usbd_ep_write(bus, 0, desc_ptr, MIN(write_len, setup->wLength));
        } else {
            return false;
        }
        break;
    }
    default:
        return false;
        break;
    }

    return true;
}

_attribute_ram_code_sec_ static unsigned char usbd_set_configuration(const unsigned char bus, unsigned char config_num)
{
    (void)bus;
    (void)config_num;

    unsigned char *config_desc      = 0;
    unsigned short current_desc_len = 0;

    config_desc                    = usbd_get_config_descriptor(bus);
    unsigned short config_desc_len = config_desc[2] | (config_desc[3] << 8);

    while (config_desc[0] != 0U) {
        switch (config_desc[1]) {
        case 0x02:
            break;
        case 0x04:
            break;
        case 0x05:
            /* endpoint descriptor. */
            usb_log("find endpoint descriptor\r\n");
            usbd_ep_open(bus, (usb_endpoint_descriptor_t *)config_desc);
            break;
        default:
            break;
        }

        config_desc += config_desc[0];
        current_desc_len += config_desc[0];
        if (current_desc_len >= config_desc_len) {
            break;
        }
    }

    return true;
}

_attribute_ram_code_sec_ unsigned char usbd_standard_request_process(const unsigned char bus, const usb_control_request_t *setup, unsigned char setup_stage)
{
    (void)bus;

    switch (setup->bmRequestType_bit.recipient) {
    case 0:
    {
        /* recipient device. */
        switch (setup->bRequest) {
        case USB_REQ_SET_ADDRESS:
        {
            usb_log("set address\r\n");
            usbd_info[bus].address = U16_LOW_BYTE(setup->wValue);
            usbd_set_address(bus, usbd_info[bus].address);
            break;
        }
        case USB_REQ_GET_DESCRIPTOR:
        {
            return usbd_get_descriptor_process(bus, setup);
            break;
        }
        case USB_REQ_GET_CONFIGURATION:
        {
            usb_log("get config\r\n");
            usbd_ep_write(bus, 0, &usbd_info[bus].config_value, 1);
            break;
        }
        case USB_REQ_SET_CONFIGURATION:
        {
            usb_log("set config\r\n");
            usbd_info[bus].config_value = U16_LOW_BYTE(setup->wValue);
            if (usbd_info[bus].config_value) {
                usbd_set_configuration(bus, usbd_info[bus].config_value);
                if (usbd_set_configuration_callback) {
                    usbd_set_configuration_callback(bus, usbd_info[bus].config_value);
                }
            }
            usbd_ep_write(bus, 0, 0, 0);
            break;
        }
        case USB_REQ_GET_STATUS:
        {
            usbd_ep_write(bus, 0, (unsigned char *)&usbd_info[bus].device_status, 2);
            break;
        }
        case USB_REQ_CLEAR_FEATURE:
        {
            if (setup->wValue == USB_REQ_FEATURE_REMOTE_WAKEUP) {
                g_usbd_remote_wakeup_en = 0;
                usb_log("Disable Remote Wakeup\r\n");
                usbd_info[bus].device_status &= ~(1 << 1);
                usbd_ep_write(bus, 0, 0, 0);
            } else {
                return false;
            }
            break;
        }
        case USB_REQ_SET_FEATURE:
        {
            switch (setup->wValue)
            {
            case USB_REQ_FEATURE_REMOTE_WAKEUP:
                g_usbd_remote_wakeup_en = 1;
                usb_log("Enable Remote Wakeup\r\n");
                usbd_info[bus].device_status |= 1 << 1;
                usbd_ep_write(bus, 0, 0, 0);
                break;
            case USB_REQ_FEATURE_TEST_MODE:
                usb_log("Enable Test Mode\r\n");
                usbd_ep_write(bus, 0, 0, 0);
                usbd_info[bus].test_mode = true;
                break;
            default:
                return false;
                break;
            }
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
        /* recipient interface. */
        unsigned char driver_index = usbd_info[bus].itf2drv[U16_LOW_BYTE(setup->wIndex)];
        return usbd_info[bus].driver[driver_index]->usbd_driver_handler(bus, setup, setup_stage);
        break;
    }
    case 2:
    {
        /* recipient endpoint. */
        unsigned char ep_dir       = usb_get_ep_dir(U16_LOW_BYTE(setup->wIndex));
        unsigned char ep_index     = usb_get_ep_number(U16_LOW_BYTE(setup->wIndex));
        unsigned char driver_index = usbd_info[bus].ep2drv[ep_index][ep_dir];
        return usbd_info[bus].driver[driver_index]->usbd_driver_handler(bus, setup, setup_stage);
        break;
    }
    case 3:
        /* recipient other. */
        return false;
        break;
    default:
        return false;
        break;
    }

    return true;
}

_attribute_ram_code_sec_ unsigned char usbd_class_request_process(const unsigned char bus, const usb_control_request_t *setup, unsigned char setup_stage)
{
    (void)bus;

    switch (setup->bmRequestType_bit.recipient) {
    case 0:
        /* recipient device. */
        return false;
        break;
    case 1:
    {
        /* recipient interface. */
        unsigned char driver_index = usbd_info[bus].itf2drv[U16_LOW_BYTE(setup->wIndex)];
        return usbd_info[bus].driver[driver_index]->usbd_driver_handler(bus, setup, setup_stage);
        break;
    }
    case 2:
    {
        /* recipient endpoint. */
        unsigned char ep_dir       = usb_get_ep_dir(U16_LOW_BYTE(setup->wIndex));
        unsigned char ep_index     = usb_get_ep_number(U16_LOW_BYTE(setup->wIndex));
        unsigned char driver_index = usbd_info[bus].ep2drv[ep_index][ep_dir];
        return usbd_info[bus].driver[driver_index]->usbd_driver_handler(bus, setup, setup_stage);
        break;
    }
    case 3:
        /* recipient other. */
        return false;
        break;
    default:
        return false;
        break;
    }

    return true;
}

_attribute_ram_code_sec_ unsigned char usbd_vendor_request_process(const unsigned char bus, const usb_control_request_t *setup, unsigned char setup_stage)
{
    (void)bus;
    (void)setup;
    (void)setup_stage;

    return false;
}

_attribute_ram_code_sec_ unsigned char usbd_control_request_process(const unsigned char bus, const usb_control_request_t *setup, unsigned char setup_stage)
{
    (void)bus;

    tmemcpy(&usbd_info[bus].setup, setup, sizeof(usb_control_request_t));

    switch (setup->bmRequestType_bit.type) {
    case 0:
        /* standard request. */
        return usbd_standard_request_process(bus, setup, setup_stage);
        break;
    case 1:
        /* class request. */
        return usbd_class_request_process(bus, setup, setup_stage);
        break;
    case 2:
        /* vendor request. */
        return usbd_vendor_request_process(bus, setup, setup_stage);
        break;
    default:
        return false;
        break;
    }

    return true;
}

_attribute_ram_code_sec_ void usbd_driver_register(unsigned char bus, usbd_driver_t *driver, unsigned char intf, unsigned char ep_addr)
{
    (void)bus;

    unsigned char ep_dir   = usb_get_ep_dir(ep_addr);
    unsigned char ep_index = usb_get_ep_number(ep_addr);

    if (ep_index <= USBD_SUPPORT_ENDPOINT_MAX) {
        usbd_info[bus].ep2drv[ep_index][ep_dir]   = driver->driver_num;
        usbd_info[bus].itf2drv[intf]              = driver->driver_num;
        usbd_info[bus].driver[driver->driver_num] = driver;
    }
}

_attribute_ram_code_sec_ void usbd_endpoint_register(unsigned char bus, unsigned char ep_addr, usbd_endpoint_callback callback)
{
    (void)bus;

    unsigned char ep_dir   = usb_get_ep_dir(ep_addr);
    unsigned char ep_index = usb_get_ep_number(ep_addr);
    if (ep_index <= USBD_SUPPORT_ENDPOINT_MAX) {
        if (ep_dir == USB_DIR_IN) {
            usbd_info[bus].epin_cb[ep_index] = callback;
        } else {
            usbd_info[bus].epout_cb[ep_index] = callback;
        }
    }
}

_attribute_ram_code_sec_ void usbd_epin_complete_handler(unsigned char bus, unsigned char ep_index, unsigned int len)
{
    (void)bus;

    if (usbd_info[bus].epin_cb[ep_index]) {
        usbd_info[bus].epin_cb[ep_index](bus, ep_index, len);
    }
}

_attribute_ram_code_sec_ void usbd_epout_complete_handler(unsigned char bus, unsigned char ep_index, unsigned int len)
{
    (void)bus;

    if (usbd_info[bus].epout_cb[ep_index]) {
        usbd_info[bus].epout_cb[ep_index](bus, ep_index, len);
    }
}

_attribute_ram_code_sec_ void usbd_endpoint_set_status(unsigned char bus, unsigned char ep_addr, unsigned short ep_status)
{
    (void)bus;

    unsigned char ep_dir   = usb_get_ep_dir(ep_addr);
    unsigned char ep_index = usb_get_ep_number(ep_addr);

    usbd_info[bus].ep_status[ep_index][ep_dir] = ep_status;
}

_attribute_ram_code_sec_ unsigned short *usbd_endpoint_get_status(unsigned char bus, unsigned char ep_addr)
{
    (void)bus;

    unsigned char ep_dir   = usb_get_ep_dir(ep_addr);
    unsigned char ep_index = usb_get_ep_number(ep_addr);

    return &usbd_info[bus].ep_status[ep_index][ep_dir];
}
