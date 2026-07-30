/********************************************************************************************************
 * @file    usb_audio.h
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
#ifndef __USB_AUDIO_H__
#define __USB_AUDIO_H__

#include "../../core/usbd_core.h"

typedef enum
{
    AUDIO_FU_CTRL_UNDEF             = 0x00,
    AUDIO_FU_CTRL_MUTE              = 0x01,
    AUDIO_FU_CTRL_VOLUME            = 0x02,
    AUDIO_FU_CTRL_BASS              = 0x03,
    AUDIO_FU_CTRL_MID               = 0x04,
    AUDIO_FU_CTRL_TREBLE            = 0x05,
    AUDIO_FU_CTRL_GRAPHIC_EQUALIZER = 0x06,
    AUDIO_FU_CTRL_AGC               = 0x07,
    AUDIO_FU_CTRL_DELAY             = 0x08,
    AUDIO_FU_CTRL_BASS_BOOST        = 0x09,
    AUDIO_FU_CTRL_LOUDNESS          = 0x0A,
    AUDIO_FU_CTRL_INPUT_GAIN        = 0x0B,
    AUDIO_FU_CTRL_GAIN_PAD          = 0x0C,
    AUDIO_FU_CTRL_INVERTER          = 0x0D,
    AUDIO_FU_CTRL_UNDERFLOW         = 0x0E,
    AUDIO_FU_CTRL_OVERVLOW          = 0x0F,
    AUDIO_FU_CTRL_LATENCY           = 0x10,
} audio_feature_unit_control_selector_t;

#endif
