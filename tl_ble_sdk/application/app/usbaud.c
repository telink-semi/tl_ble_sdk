/********************************************************************************************************
 * @file    usbaud.c
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
#include "usbaud.h"
#include "../usbstd/usb.h"
#include "../usbstd/AudioClassCommon.h"

/*************************************************
 * g_audio_hid_chg:
 *      0x00: no event
 *      0x01: speaker volume change
 *      0x02: speaker mute change
 *      0x11: microphone volume change
 *      0x12: microphone mute change
 *************************************************/


static speaker_setting_t speaker_setting;
static mic_setting_t     mic_setting;

void usbaud_set_audio_mode(int mono_en)
{
    if (mono_en == 1) {
#if (MCU_CORE_TYPE != CHIP_TYPE_TL322X)
        BM_SET(reg_usb_ep_ctrl(USB_EDP_MIC), FLD_USB_EP_MONO);
#endif
    }
}

#if 0
void usbaud_hid_report(char format, char volume) {
    unsigned char sAudioHidReport[] = { 0x01,/* report ID */
    format,/*[0]:play/pause,
     [1]:scan next track,
     [2]:scan previous track,
     [3]:stop,
     [4]:play,
     [5]:pause,
     [6]:fast forward,
     [7]:rewind,
     */
    volume /*[0]:volume up
     [1]:volume down
     [2]:mute
     [3:7] 0;
     */
    };
    WriteEndPoint(EDP_ID_AUDIO_HID, sAudioHidReport, sizeof(sAudioHidReport));
}
#endif


#if (USB_SPEAKER_ENABLE || USB_MIC_ENABLE) //  use for volume control, mute, next, prev track,  move to mouse hid
int usbaud_hid_report(unsigned char cmd, unsigned char vol)
{
    if (usbhw_is_ep_busy(USB_EDP_AUDIO_IN)) {
        return 0;
    }
    reg_usb_ep_ptr(USB_EDP_AUDIO_IN) = 0;

    // please refer to keyboard_report_desc
    reg_usb_ep_dat(USB_EDP_AUDIO_IN) = USB_HID_AUDIO;
    reg_usb_ep_dat(USB_EDP_AUDIO_IN) = cmd;
    reg_usb_ep_dat(USB_EDP_AUDIO_IN) = vol;

    reg_usb_ep_ctrl(USB_EDP_AUDIO_IN) = FLD_EP_DAT_ACK; // ACK
    return 1;
}
#endif

#if 0
unsigned char usbaud_handle_report(unsigned char c) {
    if (USB_REPORT_NO_EVENT == c) {
        return USB_REPORT_NO_EVENT;
    }
    if(reg_usb_ep_ctrl(USB_EDP_AUDIO) & FLD_USB_EP_BUSY)
        return c;

    if(USB_REPORT_RELEASE == c){
        usbaud_hid_report(0, 0);
        return USB_REPORT_NO_EVENT;
    }else{
        usbaud_hid_report((c < 0x10) ? (1 << c) : 0
            ,(c < 0x10) ? 0 : (1 << (c & 0x0f)));
        return USB_REPORT_RELEASE;
    }
}
#endif

static unsigned short usbaud_cal_speaker_step(signed short vol)
{
    if (vol < SPEAKER_VOL_MIN) {
        return 0;
    }
    return (vol - SPEAKER_VOL_MIN) / SPEAKER_VOL_RES;
}

static unsigned short usbaud_cal_mic_step(signed short vol)
{
    if (vol < MIC_VOL_MIN) {
        return 0;
    }
    return (vol - MIC_VOL_MIN) / MIC_VOL_RES;
}

void usbaud_set_speaker_vol(signed short vol)
{
    speaker_setting.vol_cur  = vol;
    speaker_setting.vol_step = usbaud_cal_speaker_step(vol);
}

void usbaud_set_mic_vol(signed short vol)
{
    mic_setting.vol_cur  = vol;
    mic_setting.vol_step = usbaud_cal_mic_step(vol);
}

usb_audio_status_t g_usb_audio_status;

unsigned char usbaud_speaker_vol_get(void)
{
    //return ((g_usb_audio_status.speaker_mute << 7) | g_usb_audio_status.speaker_vol);
    return (speaker_setting.mute << 7) | (speaker_setting.vol_step & 0x7f);
}

unsigned char usbaud_mic_vol_get(void)
{
    //return ((g_usb_audio_status.speaker_mute << 7) | g_usb_audio_status.speaker_vol);
    return (mic_setting.mute << 7) | (mic_setting.vol_step & 0x7f);
}

void usbaud_mic_en(int en)
{
    mic_setting.mute = en;
}

volatile int aaa_audio_intf_set = 0;

int usb_audio_class_out_intf_proc(unsigned char type, unsigned char feature_id)
{
    int                 ret   = 0;
#if MCU_CORE_TYPE != MCU_CORE_TL322X
    usb_audio_status_t *p_aud = &g_usb_audio_status;
#endif

    aaa_audio_intf_set = 0;
    aaa_audio_intf_set = (type << 8);
    aaa_audio_intf_set |= feature_id;
    switch (type) {
    // mute control and sample
    case 0x01:
        switch (feature_id) {
        case 0x00:                     //set sample frequency
            break;
#if MCU_CORE_TYPE != MCU_CORE_TL322X
        case AUDIO_FEATURE_ID_SPEAKER: // Feature ID 0x02, Speaker
            p_aud->speaker_mute = usbhw_read_ctrl_ep_data();
            p_aud->change |= 0x1;
            break;
        case AUDIO_FEATURE_ID_MIC: // Feature ID 0x05, MIC
            p_aud->mic_mute = usbhw_read_ctrl_ep_data();
            p_aud->change |= 0x2;
            break;
#endif
        default:
            ret = 1;
            break;
        }
        break;

    // volume control
    case 0x02:
        switch (feature_id) {
        case AUDIO_FEATURE_ID_SPEAKER: // Feature ID 0x02, Speaker
#if MCU_CORE_TYPE != MCU_CORE_TL322X
            p_aud->speaker_vol = usbhw_read_ctrl_ep_data() + (usbhw_read_ctrl_ep_data() << 8);
            p_aud->speaker_vol = ((p_aud->speaker_vol - (short)SPEAKER_VOL_MIN + SPEAKER_VOL_STEP) * (10 - 1)) >> 12;
            if (p_aud->speaker_vol < 0) {
                p_aud->speaker_vol = 0;
            }
            p_aud->change |= 0x4;
            break;

        case AUDIO_FEATURE_ID_MIC: // Feature ID 0x05, MIC
            p_aud->mic_vol = usbhw_read_ctrl_ep_data() + (usbhw_read_ctrl_ep_data() << 8);
            p_aud->change |= 0x8;
            break;
#endif
        default:
            ret = 1;
            break;
        }
        break;

    default:
        break;
    }
    return ret;
}

// return -1 on fail,  0 on success
int usbaud_handle_set_speaker_cmd(int type)
{
    if (type == AUDIO_FEATURE_MUTE) {
#if MCU_CORE_TYPE != MCU_CORE_TL322X
        speaker_setting.mute = usbhw_read_ctrl_ep_data();
#endif
    } else if (type == AUDIO_FEATURE_VOLUME) {
#if MCU_CORE_TYPE != MCU_CORE_TL322X
        unsigned short val = usbhw_read_ctrl_ep_u16();
        usbaud_set_speaker_vol(val);
#endif
    } else {
        return -1;
    }
    return 0;
}

// return -1 on fail,  0 on success
int usbaud_handle_set_mic_cmd(int type)
{
#if MCU_CORE_TYPE != MCU_CORE_TL322X
    if (type == AUDIO_FEATURE_MUTE) {

        mic_setting.mute = usbhw_read_ctrl_ep_data();
    } else if (type == AUDIO_FEATURE_VOLUME) {
        unsigned short val = usbhw_read_ctrl_ep_u16();
        usbaud_set_mic_vol(val);
    } else {
    }
#else
    (void)type;
#endif
    return 0;
}

// return -1 on fail,  0 on success
int usbaud_handle_get_speaker_cmd(int req, int type)
{
    if (type == AUDIO_FEATURE_MUTE) {
#if MCU_CORE_TYPE != MCU_CORE_TL322X
        usbhw_write_ctrl_ep_data(speaker_setting.mute);
#endif
    } else if (type == AUDIO_FEATURE_VOLUME) {
        switch (req) {
#if MCU_CORE_TYPE != MCU_CORE_TL322X
        case AUDIO_REQ_GetCurrent:
            usbhw_write_ctrl_ep_u16(speaker_setting.vol_cur);
            break;
        case AUDIO_REQ_GetMinimum:
            usbhw_write_ctrl_ep_u16(SPEAKER_VOL_MIN);
            break;
        case AUDIO_REQ_GetMaximum:
            usbhw_write_ctrl_ep_u16(SPEAKER_VOL_MAX);
            break;
        case AUDIO_REQ_GetResolution:
            usbhw_write_ctrl_ep_u16(SPEAKER_VOL_RES);
            break;
#endif
        default:
            return -1;
        }
    } else {
        return -1;
    }
    return 0;
}

// return -1 on fail,  0 on success
int usbaud_handle_get_mic_cmd(int req, int type)
{
    if (type == AUDIO_FEATURE_MUTE) {
#if MCU_CORE_TYPE != MCU_CORE_TL322X
        usbhw_write_ctrl_ep_data(mic_setting.mute);
#endif
    } else if (type == AUDIO_FEATURE_VOLUME) {
        switch (req) {
#if MCU_CORE_TYPE != MCU_CORE_TL322X
        case AUDIO_REQ_GetCurrent:
            usbhw_write_ctrl_ep_u16(mic_setting.vol_cur);
            break;
        case AUDIO_REQ_GetMinimum:
            usbhw_write_ctrl_ep_u16(MIC_VOL_MIN);
            break;
        case AUDIO_REQ_GetMaximum: //
            usbhw_write_ctrl_ep_u16(MIC_VOL_MAX);
            break;
        case AUDIO_REQ_GetResolution:
            usbhw_write_ctrl_ep_u16(MIC_VOL_RES);
            break;
#endif
        default:
            return -1;
        }
    } else {
        return -1;
    }
    return 0;
}

void usbaud_init(void)
{
    if (USB_MIC_ENABLE && 1 == MIC_CHANNEL_COUNT) {
        usbaud_set_audio_mode(1);
    }
#if (USB_SPEAKER_ENABLE)
    usbaud_set_speaker_vol(SPEAKER_VOL_MAX);
#endif
#if (USB_MIC_ENABLE)
    mic_setting.vol_cur = MIC_VOL_DEF;
#endif
}

/**
 * @brief       This function serves to send data to USB Host or receive data from USB Host
 * @param[in]   none
 * @return      none
 */
void usb_audio_irq_data_process(void)
{
#if (MCU_CORE_TYPE != CHIP_TYPE_TL322X)
    unsigned char irq = usbhw_get_eps_irq();
    if (irq & FLD_USB_EDP7_IRQ) {
        usbhw_clr_eps_irq(FLD_USB_EDP7_IRQ);
    }
    if (irq & FLD_USB_EDP6_IRQ) {
        usbhw_clr_eps_irq(FLD_USB_EDP6_IRQ);
    }
#endif
}
