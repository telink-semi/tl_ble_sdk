/********************************************************************************************************
 * @file    usbmouse.c
 *
 * @brief   This is the source file for Telink RISC-V MCU
 *
 * @author  Driver Group
 * @date    2019
 *
 * @par     Copyright (c) 2019, Telink Semiconductor (Shanghai) Co., Ltd. ("TELINK")
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
#include "tl_common.h"
#include "string.h"
#include "usbmouse.h"
#include "usbkb.h"
#include "../usbstd/usb.h"
//#include "../rf_frame.h"

#ifndef USB_MOUSE_REPORT_SMOOTH
    #define USB_MOUSE_REPORT_SMOOTH 0
#endif


#define USBMOUSE_BUFF_DATA_NUM 8

#if ((MCU_CORE_TYPE != CHIP_TYPE_TL322X) || (USB_MOUSE_REPORT_SMOOTH))
static unsigned char usbmouse_wptr, usbmouse_rptr;
#endif

#if (MCU_CORE_TYPE != CHIP_TYPE_TL322X)
static mouse_data_t mouse_dat_buff[USBMOUSE_BUFF_DATA_NUM];
static unsigned int  usbmouse_not_released;
static unsigned int  usbmouse_data_report_time;
#endif

void usbmouse_add_frame(rf_packet_mouse_t *packet_mouse)
{
    unsigned char new_data_num = packet_mouse->pno;
    for (unsigned char i = 0; i < new_data_num; i++) {
        //memcpy4((int*)(&mouse_dat_buff[usbmouse_wptr]), (int*)(&packet_mouse->data[i*sizeof(mouse_data_t)]), sizeof(mouse_data_t));//todo
        //BOUND_INC_POW2(usbmouse_wptr,USBMOUSE_BUFF_DATA_NUM);//todo
        //if(usbmouse_wptr == usbmouse_rptr)
        {
            //BOUND_INC_POW2(usbmouse_rptr,USBMOUSE_BUFF_DATA_NUM);
            break;
        }
    }
}

void usbmouse_release_check(void)
{
#if (MCU_CORE_TYPE != CHIP_TYPE_TL322X)
    if (usbmouse_not_released && clock_time_exceed(usbmouse_data_report_time, USB_MOUSE_RELEASE_TIMEOUT)) {
        unsigned int release_data = 0;

        if (usbmouse_hid_report(USB_HID_MOUSE, (unsigned char *)(&release_data), MOUSE_REPORT_DATA_LEN)) {
            usbmouse_not_released = 0;
        }
    }
#endif
}

void usbmouse_report_frame(void)
{
#if USB_MOUSE_REPORT_SMOOTH
    static unsigned int tick = 0;
    if (usbhw_is_ep_busy(USB_EDP_MOUSE)) {
        tick = stimer_get_tick();
    }

    unsigned char diff = (usbmouse_wptr - usbmouse_rptr) & (USBMOUSE_BUFF_DATA_NUM - 1);
    if (diff < 3 && !clock_time_exceed(tick, 5000)) {
        return;
    }
#endif
#if (MCU_CORE_TYPE != CHIP_TYPE_TL322X)
    if (usbmouse_wptr != usbmouse_rptr) {
        unsigned int data = *(unsigned int *)(&mouse_dat_buff[usbmouse_rptr]); // that is   >  0
        int          ret  = usbmouse_hid_report(USB_HID_MOUSE, (unsigned char *)(&data), MOUSE_REPORT_DATA_LEN);
        if (ret) {
            // BOUND_INC_POW2(usbmouse_rptr,USBMOUSE_BUFF_DATA_NUM);//todo
        }
        if (0 == data && ret) { //  successfully  release the key
            usbmouse_not_released = 0;
        } else {
            usbmouse_not_released     = 1;
            usbmouse_data_report_time = stimer_get_tick();
        }
    }
#endif
    return;
}

/**
 * @brief      This function serves to return report to host
 * @param[in]  report_id - mouse report ID
 * @param[in]  *data - report data
 * @param[in]  cnt - data length
 * @return     1 success   0 fail
 */

int usbmouse_hid_report(unsigned char report_id, unsigned char *data, int cnt)
{
#if (MCU_CORE_TYPE != CHIP_TYPE_TL322X)
    if (usbhw_is_ep_busy(USB_EDP_MOUSE)) {
        unsigned char *pData = (unsigned char *)&usb_fifo[usb_ff_wptr++ & (USB_FIFO_NUM - 1)];
        pData[0]             = DAT_TYPE_MOUSE;
        pData[1]             = report_id;
        pData[2]             = cnt;
        memcpy(pData + 4, data, cnt);

        int fifo_use = (usb_ff_wptr - usb_ff_rptr) & (USB_FIFO_NUM * 2 - 1);
        if (fifo_use > USB_FIFO_NUM) {
            usb_ff_rptr++; //fifo overflow, overlap older data
        }

        return 0;
    }
    reg_usb_ep_ptr(USB_EDP_MOUSE) = 0;

    // please refer to usbmouse_i.h mouse_report_desc
    extern unsigned char usb_mouse_report_proto;

    if (!usb_mouse_report_proto) {
        reg_usb_ep_dat(USB_EDP_MOUSE) = data[0];
        reg_usb_ep_dat(USB_EDP_MOUSE) = data[1];
        reg_usb_ep_dat(USB_EDP_MOUSE) = data[2];
    } else {
        reg_usb_ep_dat(USB_EDP_MOUSE) = report_id;
        for (int i = 0; i < cnt; ++i) {
            reg_usb_ep_dat(USB_EDP_MOUSE) = data[i];
        }
    }

    reg_usb_ep_ctrl(USB_EDP_MOUSE) = FLD_EP_DAT_ACK | (edp_toggle[USB_EDP_MOUSE] ? FLD_USB_EP_DAT1 : FLD_USB_EP_DAT0); // ACK
    edp_toggle[USB_EDP_MOUSE] ^= 1;
#else
    (void)report_id;//avoid warning
    (void)data;//avoid warning
    (void)cnt;//avoid warning
#endif
    return 1;
}
