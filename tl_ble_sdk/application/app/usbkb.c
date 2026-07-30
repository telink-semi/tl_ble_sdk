/********************************************************************************************************
 * @file    usbkb.c
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

#if (1)

    #include "driver.h"
    #include "usbkb.h"
    #include "usbmouse.h"
    #include "../usbstd/usb.h"
    /*
#include "../../usbhw.h"
*/
    #include "../usbstd/usbkeycode.h"

    #include "../rf_frame.h"


unsigned char usb_fifo[USB_FIFO_NUM][USB_FIFO_SIZE];
unsigned char usb_ff_rptr = 0;
unsigned char usb_ff_wptr = 0;

void usbkb_hid_report(kb_data_t *data);

#if (MCU_CORE_TYPE != CHIP_TYPE_TL322X)
static unsigned char vk_sys_map[VK_SYS_CNT] = {
    VK_POWER_V,
    VK_SLEEP_V,
    VK_WAKEUP_V};

static vk_ext_t vk_media_map[VK_MEDIA_CNT] = {
    {VK_W_SRCH_V},
    {VK_HOME_V},
    {VK_W_BACK_V},
    {VK_W_FORWARD_V},
    {VK_W_STOP_V},
    {VK_W_REFRESH_V},
    {VK_W_FAV_V},
    {VK_MEDIA_V},
    {VK_MAIL_V},
    {VK_CAL_V},
    {VK_MY_COMP_V},
    {VK_NEXT_TRK_V},
    {VK_PREV_TRK_V},
    {VK_STOP_V},
    {VK_PLAY_PAUSE_V},
    {VK_W_MUTE_V},
    {VK_VOL_UP_V},
    {VK_VOL_DN_V},
};
#endif

enum
{
    KB_NORMAL_RELEASE_MASK = BIT(0),
    KB_SYS_RELEASE_MASK    = BIT(1),
    KB_MEDIA_RELEASE_MASK  = BIT(2),
};

    #define USBKB_BUFF_DATA_NUM 8
//static
kb_data_t kb_dat_buff[USBKB_BUFF_DATA_NUM];

//static
unsigned char       usbkb_wptr, usbkb_rptr;
static unsigned int usbkb_not_released;
static unsigned int usbkb_data_report_time;

void usbkb_report_frame(void)
{
    if (usbkb_wptr != usbkb_rptr) {
        kb_data_t *data = (kb_data_t *)(&kb_dat_buff[usbkb_rptr]);
        usbkb_hid_report(data);
        //BOUND_INC_POW2(usbkb_rptr,USBKB_BUFF_DATA_NUM);//todo
    }
    return;
}

static void usbkb_release_normal_key(void)
{
    if (usbkb_not_released & KB_NORMAL_RELEASE_MASK) {
        unsigned char normal_keycode[KEYBOARD_REPORT_KEY_MAX] = {0};
        if (usbkb_hid_report_normal(0, normal_keycode)) {
            BM_CLR(usbkb_not_released, KB_NORMAL_RELEASE_MASK);
        }
    }
}

static void usbkb_release_sys_key(void)
{
#if (MCU_CORE_TYPE != CHIP_TYPE_TL322X)
    if (usbkb_not_released & KB_SYS_RELEASE_MASK) {
        unsigned int release_data = 0;
        if (usbmouse_hid_report(USB_HID_KB_SYS, (unsigned char *)(&release_data), 1)) {
            BM_CLR(usbkb_not_released, KB_SYS_RELEASE_MASK);
        }
    }
#endif
}

static void usbkb_release_media_key(void)
{
#if (MCU_CORE_TYPE != CHIP_TYPE_TL322X)
    if (usbkb_not_released & KB_MEDIA_RELEASE_MASK) {
        unsigned char ext_keycode[MOUSE_REPORT_DATA_LEN] = {0};
        if (usbmouse_hid_report(USB_HID_KB_MEDIA, ext_keycode, MEDIA_REPORT_DATA_LEN)) {
            BM_CLR(usbkb_not_released, KB_MEDIA_RELEASE_MASK);
        }
    }
#endif
}

static void usbkb_release_keys(void)
{
    usbkb_release_sys_key();
    usbkb_release_normal_key();
    usbkb_release_media_key();
}

void usbkb_release_check(void)
{
    if (usbkb_not_released && clock_time_exceed(usbkb_data_report_time, USB_KEYBOARD_RELEASE_TIMEOUT)) {
        usbkb_release_keys(); //  release keys
    }
}

int usbkb_separate_key_types(unsigned char *keycode, unsigned char cnt, unsigned char *normal_key, unsigned char *ext_key)
{
    int normal_cnt = 0;
    for (int i = 0; i < cnt; ++i) {
        if (keycode[i] >= VK_EXT_START && keycode[i] < VK_EXT_END) {
            *ext_key = keycode[i];
        } else {
            normal_key[normal_cnt++] = keycode[i];
        }
    }
    return normal_cnt;
}

int usbkb_hid_report_normal(unsigned char ctrl_key, unsigned char *keycode)
{
#if (MCU_CORE_TYPE != CHIP_TYPE_TL322X)
    if (usbhw_is_ep_busy(USB_EDP_KEYBOARD_IN)) {
        unsigned char *pData = (unsigned char *)&usb_fifo[usb_ff_wptr++ & (USB_FIFO_NUM - 1)];
        pData[0]             = DAT_TYPE_KB;
        pData[1]             = ctrl_key;
        memcpy(pData + 2, keycode, 6);

        int fifo_use = (usb_ff_wptr - usb_ff_rptr) & (USB_FIFO_NUM * 2 - 1);
        if (fifo_use > USB_FIFO_NUM) {
            usb_ff_rptr++;
            //fifo overflow, overlap older data
        }

        return 0;
    }

    reg_usb_ep_ptr(USB_EDP_KEYBOARD_IN) = 0;
#endif

    #if USB_SOFTWARE_CRC_CHECK

    unsigned char  crc_in[KEYBOARD_REPORT_KEY_MAX + 2];
    unsigned short crc;
    unsigned int   crch;

    crc_in[0] = ctrl_key;
    crc_in[1] = 0;
    foreach (i, KEYBOARD_REPORT_KEY_MAX) {
        crc_in[i + 2] = keycode[i];
    }
    crc  = USB_CRC16(crc_in, KEYBOARD_REPORT_KEY_MAX + 2);
    crch = crc >> 8;

    if ((crch == 0x06) || (crch == 0x04) || (crch == 0x00)) {
        unsigned int tmp = crc_in[2];
        crc_in[2]        = crc_in[3];
        crc_in[3]        = tmp;
    }
    // please refer to keyboard_report_desc
    foreach (i, (KEYBOARD_REPORT_KEY_MAX + 2)) {
        reg_usb_ep_dat(USB_EDP_KEYBOARD_IN) = crc_in[i];
    }

    #else
#if (MCU_CORE_TYPE != CHIP_TYPE_TL322X)
    // please refer to keyboard_report_desc
    reg_usb_ep_dat(USB_EDP_KEYBOARD_IN) = ctrl_key;
    reg_usb_ep_dat(USB_EDP_KEYBOARD_IN) = 0; //resv
    for (int i = 0; i < KEYBOARD_REPORT_KEY_MAX; ++i) {
        reg_usb_ep_dat(USB_EDP_KEYBOARD_IN) = keycode[i];
    }
#endif
    #endif
#if (MCU_CORE_TYPE != CHIP_TYPE_TL322X)
    //  reg_usb_ep_ctrl(USB_EDP_KEYBOARD_IN) = FLD_EP_DAT_ACK;      // ACK
    reg_usb_ep_ctrl(USB_EDP_KEYBOARD_IN) = FLD_EP_DAT_ACK | (edp_toggle[USB_EDP_KEYBOARD_IN] ? FLD_USB_EP_DAT1 : FLD_USB_EP_DAT0); // ACK
    edp_toggle[USB_EDP_KEYBOARD_IN] ^= 1;
#endif

#if ((MCU_CORE_TYPE == CHIP_TYPE_TL322X) && (!USB_SOFTWARE_CRC_CHECK))
    (void)ctrl_key;//avoid warning
    (void)keycode;//avoid warning
#endif

    return 1;
}

static inline void usbkb_report_normal_key(int ctrl_key, unsigned char *keycode, int cnt)
{
    if (cnt > 0 || ctrl_key) {
        if (usbkb_hid_report_normal(ctrl_key, keycode)) {
            BM_SET(usbkb_not_released, KB_NORMAL_RELEASE_MASK);
        }
    } else {
        usbkb_release_normal_key();
    }
}

static inline void usbkb_report_sys_key(unsigned char ext_key)
{
#if (MCU_CORE_TYPE != CHIP_TYPE_TL322X)
    if (ext_key >= VK_SYS_START && ext_key < VK_SYS_END) {
        int idx = ext_key - VK_SYS_START;
        if (usbmouse_hid_report(USB_HID_KB_SYS, (unsigned char *)(&vk_sys_map[idx]), 1)) {
            BM_SET(usbkb_not_released, KB_SYS_RELEASE_MASK);
        }
    } else {
        usbkb_release_sys_key();
    }
#else
    (void)ext_key;//avoid warning
#endif
}

static inline void usbkb_report_media_key(unsigned char ext_key)
{
#if (MCU_CORE_TYPE != CHIP_TYPE_TL322X)
    if (ext_key >= VK_MEDIA_START && ext_key < VK_MEDIA_END) {
        //  STATIC_ASSERT(VK_EXT_LEN <= MOUSE_REPORT_DATA_LEN);

        unsigned char ext_keycode[MOUSE_REPORT_DATA_LEN] = {0};

        int idx = ext_key - VK_MEDIA_START;

        for (int i = 0; i < VK_EXT_LEN; ++i) {
            ext_keycode[i] = vk_media_map[idx].val[i];
        }
        if (usbmouse_hid_report(USB_HID_KB_MEDIA, ext_keycode, MEDIA_REPORT_DATA_LEN)) {
            BM_SET(usbkb_not_released, KB_MEDIA_RELEASE_MASK);
        }
    } else {
        usbkb_release_media_key();
    }
#else
    (void)ext_key;
#endif
}

void usbkb_report_consumer_key(unsigned short consumer_key)
{
#if (MCU_CORE_TYPE != CHIP_TYPE_TL322X)
    if (consumer_key) {
        unsigned char ext_keycode[MOUSE_REPORT_DATA_LEN] = {0};

        //foreach(i, VK_EXT_LEN)
        for (int i = 0; i < VK_EXT_LEN; ++i) {
            ext_keycode[i] = consumer_key;
            consumer_key >>= 8;
        }

        if (usbmouse_hid_report(USB_HID_KB_MEDIA, ext_keycode, MEDIA_REPORT_DATA_LEN)) {
            BM_SET(usbkb_not_released, KB_MEDIA_RELEASE_MASK);
        }
    } else {
        usbkb_release_media_key();
    }
#else
    (void)consumer_key;
#endif

    usbkb_data_report_time = stimer_get_tick();
}

int kb_is_data_same(kb_data_t *a, kb_data_t *b)
{
    if (!a || !b) {
        return 0;
    }
    if ((a->cnt != b->cnt) || (a->ctrl_key != b->ctrl_key)) {
        return 0;
    }
    for (int i = 0; i < (a->cnt); ++i) {
        if (a->keycode[i] != b->keycode[i]) {
            return 0;
        }
    }
    return 1;
}

static inline int usbkb_check_repeat_and_save(kb_data_t *data)
{
    static kb_data_t last_data;
    int              same = kb_is_data_same(&last_data, data);
    if (!same) {
        //STATIC_ASSERT(sizeof(last_data) == 8);
        ((unsigned int *)(&last_data))[0] = ((unsigned int *)(data))[0];
        ((unsigned int *)(&last_data))[1] = ((unsigned int *)(data))[1];
    }
    return same;
}

void usbkb_hid_report(kb_data_t *data)
{
    unsigned char ext_key = VK_EXT_END, normal_key_cnt = 0;
    unsigned char normal_keycode[KEYBOARD_REPORT_KEY_MAX] = {0};

    if (data->cnt > KB_RETURN_KEY_MAX) { // must,   in case bad packets
        return;
    }

    /*  http://msdn.microsoft.com/en-us/windows/hardware/gg462991.aspx
        It is also important to notice that any re-triggering of events should be done by software timers in the host system and not by hardware timers in the device itself.
        For example, if the user keeps pressing the Volume Increment button, the device should only generate one input report with this state information
    */
    if ((data->cnt > 0 || data->ctrl_key) && usbkb_check_repeat_and_save(data)) {
        if (usbkb_not_released) {
            usbkb_data_report_time = stimer_get_tick();
            return;
        }
    }


    if (data->cnt > 0) {
        normal_key_cnt = usbkb_separate_key_types(data->keycode, data->cnt, normal_keycode, &ext_key);
    }

    usbkb_report_normal_key(data->ctrl_key, normal_keycode, normal_key_cnt);
    usbkb_report_sys_key(ext_key);
    usbkb_report_media_key(ext_key);

    usbkb_data_report_time = stimer_get_tick();
}

int usb_hid_report_fifo_proc(void)
{
    if (usb_ff_rptr == usb_ff_wptr) {
        return 0;
    }

#if (MCU_CORE_TYPE != CHIP_TYPE_TL322X)
    unsigned char *pData = (unsigned char *)&usb_fifo[usb_ff_rptr & (USB_FIFO_NUM - 1)];

    if (pData[0] == DAT_TYPE_KB) {
        if (usbhw_is_ep_busy(USB_EDP_KEYBOARD_IN)) {
            return 0;
        } else {
            usbkb_hid_report_normal(pData[1], pData + 2);

            usb_ff_rptr++;

            return 1;
        }
    } else if (pData[0] == DAT_TYPE_MOUSE) {
        if (usbhw_is_ep_busy(USB_EDP_MOUSE)) {
            return 0;
        } else {
            usbmouse_hid_report(pData[1], pData + 4, pData[2]);

            usb_ff_rptr++;

            return 1;
        }
    }

#endif
    return 0;
}


#endif
