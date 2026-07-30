/********************************************************************************************************
 * @file    tl_audio.h
 *
 * @brief   This is the header file for BLE SDK
 *
 * @author  BLE GROUP
 * @date    06,2022
 *
 * @par     Copyright (c) 2022, Telink Semiconductor (Shanghai) Co., Ltd. ("TELINK")
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
#ifndef TL_AUDIO_H_
#define TL_AUDIO_H_

#include "tl_common.h"
#include "audio_config.h"


#if (TL_AUDIO_MODE & RCU_PROJECT) //RCU
    #ifndef TL_NOISE_SUPPRESSION_ENABLE
        #define TL_NOISE_SUPPRESSION_ENABLE 0
    #endif

    #ifndef IIR_FILTER_ENABLE
        #define IIR_FILTER_ENABLE 0
    #endif

    #ifndef TL_MIC_PACKET_BUFFER_NUM
        #define TL_MIC_PACKET_BUFFER_NUM 4
    #endif

    #ifndef MIC_SHORT_DEC_SIZE
        #define MIC_SHORT_DEC_SIZE 0
    #endif

    #ifndef ADPCM_PACKET_LEN
        #define ADPCM_PACKET_LEN 0
    #endif

    #ifndef TL_MIC_ADPCM_UNIT_SIZE
        #define TL_MIC_ADPCM_UNIT_SIZE 0
    #endif

    #ifndef TL_MIC_BUFFER_SIZE
        #define TL_MIC_BUFFER_SIZE 0
    #endif

    #if TL_MIC_BUFFER_SIZE
s16 buffer_mic[TL_MIC_BUFFER_SIZE >> 1] __attribute__((aligned(4)));
    #endif

    #if TL_NOISE_SUPPRESSION_ENABLE
extern int md_long;
extern int md_short;
extern int md_im;
extern int md_noise;
extern int md_gain;

static inline int noise_suppression(s16 md)
{
    static int md_th = 384;
    //static int md_long =0;
    //static int md_short =0;
    //static int md_im =0;
    //static int md_noise = 0;
    //static int md_gain = 256;
    md_long  = ((md_long * 1023) + abs(md)) >> 10;
    md_short = ((md_short * 127) + abs(md)) >> 7;
    md_im    = ((md_im * 15) + abs(md)) >> 4;

    //md_long = ((md_long * 127) + abs (md)) >> 7;
    //md_short = ((md_short * 31) + abs (md)) >> 5;
    //md_im = ((md_im * 3) + abs (md)) >> 2;

    if ((md_noise && (md_short > md_th)) || (md_im > (md_th << 1))) {
        md_noise = 0;
        md_long  = md_short << 2;
    } else if (!md_noise && md_long < md_th) {
        md_noise = 1;
    }
    if (md_noise) {
        if (md_gain) {
            md_gain--;
        }
    } else {
        if (md_gain < 256) {
            md_gain++;
        }
    }

    return (md * md_gain + 128) >> 8;
}
    #endif

void voice_iir_OOB(signed short *ps, signed short *pd, int *coef, int nsample, u8 shift);
void voice_iir(signed short *ps, signed short *pd, int *coef, int nsample, u8 shift);
void Audio_VolumeSet(unsigned char input_output_select, unsigned char volume_set_value);
void filter_setting(void);

void audio_mic_param_init(void);
void proc_mic_encoder(void);
void proc_mic_encoder_reset(void);
int *mic_encoder_data_buffer(void);
void mic_encoder_data_read_ok(void);

#elif (TL_AUDIO_MODE & DONGLE_PROJECT) //Dongle

void abuf_init();
void abuf_mic_add(u32 *p);
void abuf_mic_dec();
void abuf_dec_usb();

#endif
#endif                                 /* TL_AUDIO_H_ */
