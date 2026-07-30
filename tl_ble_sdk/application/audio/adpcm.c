/********************************************************************************************************
 * @file    adpcm.c
 *
 * @brief   This is the source file for BLE SDK
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
#include "tl_common.h"
#include "driver.h"
#include "audio_config.h"
#include "adpcm.h"

#if (TL_AUDIO_MODE & TL_AUDIO_MASK_ADPCM_MODE) //Adpcm mode


static const signed char    idxtbl[]  = {-1, -1, -1, -1, 2, 4, 6, 8, -1, -1, -1, -1, 2, 4, 6, 8};
static const unsigned short steptbl[] = {
    7,
    8,
    9,
    10,
    11,
    12,
    13,
    14,
    16,
    17,
    19,
    21,
    23,
    25,
    28,
    31,
    34,
    37,
    41,
    45,
    50,
    55,
    60,
    66,
    73,
    80,
    88,
    97,
    107,
    118,
    130,
    143,
    157,
    173,
    190,
    209,
    230,
    253,
    279,
    307,
    337,
    371,
    408,
    449,
    494,
    544,
    598,
    658,
    724,
    796,
    876,
    963,
    1060,
    1166,
    1282,
    1411,
    1552,
    1707,
    1878,
    2066,
    2272,
    2499,
    2749,
    3024,
    3327,
    3660,
    4026,
    4428,
    4871,
    5358,
    5894,
    6484,
    7132,
    7845,
    8630,
    9493,
    10442,
    11487,
    12635,
    13899,
    15289,
    16818,
    18500,
    20350,
    22385,
    24623,
    27086,
    29794,
    32767};


    #define NUM_OF_ORIG_SAMPLE 2

    #if (TL_AUDIO_MODE & RCU_PROJECT)                         //RCU

        #if (TL_AUDIO_MODE == TL_AUDIO_RCU_ADPCM_GATT_TELINK) //RCU,GATT TELINK
/////////////////////////////////////////////////////////////////////////////////
//  256-samples split into 2
/////////////////////////////////////////////////////////////////////////////////
void mic_to_adpcm_split(signed short *ps, int len, signed short *pds, int start)
{
    int            i, j;
    unsigned short code        = 0;
    unsigned short code16      = 0;
    static int     predict_idx = 0;
    code                       = 0;
    static signed short *pd;
    static int           predict;

    //byte2,byte1: predict;  byte3: predict_idx; byte4:adpcm data len
    if (start) {
        pd                            = pds;
        *pd++                         = predict;
        *(((signed char *)pds) + 2)   = predict_idx;
        *(((unsigned char *)pds) + 3) = (ADPCM_PACKET_LEN - 4);
        pd++;
    }

    //byte5- byte128: 124 byte(62 sample) adpcm data
    for (i = 0; i < len; i++) {
        s16 di   = ps[i];
        int step = steptbl[predict_idx];
        int diff = di - predict;

        if (diff >= 0) {
            code = 0;
        } else {
            diff = -diff;
            code = 8;
        }

        int diffq = step >> 3;

        for (j = 4; j > 0; j = j >> 1) {
            if (diff >= step) {
                diff  = diff - step;
                diffq = diffq + step;
                code  = code + j;
            }
            step = step >> 1;
        }

        code16 = (code16 >> 4) | (code << 12);
        if ((i & 3) == 3) {
            *pd++ = code16;
        }

        if (code >= 8) {
            predict = predict - diffq;
        } else {
            predict = predict + diffq;
        }

        if (predict > 32767) {
            predict = 32767;
        } else if (predict < -32768) {
            predict = -32768;
        }

        predict_idx = predict_idx + idxtbl[code];
        if (predict_idx < 0) {
            predict_idx = 0;
        } else if (predict_idx > 88) {
            predict_idx = 88;
        }
    }
}

        #elif (TL_AUDIO_MODE == TL_AUDIO_RCU_ADPCM_GATT_GOOGLE) //RCU,GATT GOOGLE

#define ADPCM_ANDROID_ID    0x00

//static int serial_id = 0;

unsigned short adpcm_serial_num = 0;
_attribute_data_retention_ static int predict_idx = 1;
_attribute_data_retention_ static int predict = 0;

void mic_to_adpcm_split (signed short *ps, int len, signed short *pds, int start)
{
    int i, j;
    unsigned short code=0;
    unsigned short code16=0;
    static signed short *pd;
    code = 0;
//  static unsigned short codec_used = 0x02; // 16k
    //Seq# 2bytes; Id: 1bytes; Prev.pred: 2bytes; index: 1bytes
    if (start == GOOGLE_AUDIO_V0P4)
    {
        pd = pds;
        *pd++ = ((adpcm_serial_num>>8)&0x00ff)|((adpcm_serial_num<<8)&0xff00);
        *pd++ = (ADPCM_ANDROID_ID)|((predict&0xff00));
        *pd++ = ((predict)&0x00ff)|((predict_idx<<8)&0xff00);
    }
    // v1.0 pure audio data
    else if(start == GOOGLE_AUDIO_V1P0) {
        pd = pds;
    }
    adpcm_serial_num ++;

    for(i=0 ; i<len; i++){//unit sample

        s16 di = ps[i];
        int step = steptbl[predict_idx];
        int diff = di - predict;

        if(diff >= 0){
            code = 0;
        }else{
            diff = -diff;
            code = 8;
        }

        int diffq = step >> 3;

        for(j=4; j>0; j=j>>1){
            if(diff >= step){
                diff = diff - step;
                diffq = diffq + step;
                code = code + j;
            }
            step = step >> 1;
        }

        code16 = (code16 >> 4) | (code << 12);
        if((i&3) == 3){
            code16 = ((code16&0x0f)<<4)|((code16&0xf0)>>4) | ((code16&0x0f00)<<4)|((code16&0xf000)>>4);
            *pd++ = code16;
        }

        if(code >= 8) {
            predict = predict - diffq;
        }else {
            predict = predict + diffq;
        }

        if(predict > 32767){
            predict = 32767;
        }else if (predict < -32768) {
            predict = -32768;
        }

        predict_idx = predict_idx + idxtbl[code];
        if(predict_idx < 0) {
            predict_idx = 0;
        }else if(predict_idx > 88){
            predict_idx = 88;
        }
    }
}

        #elif (TL_AUDIO_MODE & TL_AUDIO_MASK_HID_SERVICE_CHANNEL)  //RCU,HID SERVICE

int predict     = 0;
int predict_idx = 0;

/////////////////////////////////////////////////////////////////////////////////
//  256-samples split into 2
/////////////////////////////////////////////////////////////////////////////////
_attribute_ram_code_ void mic_to_adpcm_split(signed short *ps, int len, signed short *pds, int start)
{
    (void) start;
    int                  i, j;
    unsigned short       code   = 0;
    unsigned short       code16 = 0;
    static signed short *pd;
    pd   = pds;
    code = 0;

    for (i = 0; i < len; i++) {
        s16 di   = ps[i];
        int step = steptbl[predict_idx];
        int diff = di - predict;

        if (diff >= 0) {
            code = 0;
        } else {
            diff = -diff;
            code = 8;
        }

        int diffq = step >> 3;

        for (j = 4; j > 0; j = j >> 1) {
            if (diff >= step) {
                diff  = diff - step;
                diffq = diffq + step;
                code  = code + j;
            }
            step = step >> 1;
        }

        code16 = (code16 >> 4) | (code << 12);
        if ((i & 3) == 3) {
            code16 = ((code16 & 0x0f) << 4) | ((code16 & 0xf0) >> 4) | ((code16 & 0x0f00) << 4) | ((code16 & 0xf000) >> 4);
            *pd++  = code16;
        }

        if (code >= 8) {
            predict = predict - diffq;
        } else {
            predict = predict + diffq;
        }

        if (predict > 32767) {
            predict = 32767;
        } else if (predict < -32768) {
            predict = -32768;
        }

        predict_idx = predict_idx + idxtbl[code & 15];
        if (predict_idx < 0) {
            predict_idx = 0;
        } else if (predict_idx > 88) {
            predict_idx = 88;
        }
    }
}
        #else

        #endif                                                     //end RCU_PROJECT pcm to adpcm

    #elif (TL_AUDIO_MODE & DONGLE_PROJECT)                         //Dongle

        #if (TL_AUDIO_MODE == TL_AUDIO_DONGLE_ADPCM_GATT_TELINK)   //Dongle,GATT TELINK

void adpcm_to_pcm(signed short *ps, signed short *pd, int len)
{
    int i;

    //byte2,byte1: predict;  byte3: predict_idx; byte4:adpcm data len
    int predict     = ps[0];
    int predict_idx = ps[1] & 0xff;
    //  int adpcm_len = (ps[1]>>8) & 0xff;

    unsigned char *pcode = (unsigned char *)(ps + NUM_OF_ORIG_SAMPLE);

    unsigned char code;
    code = *pcode++;

    //byte5- byte128: 124 byte(62 sample) adpcm data
    for (i = 0; i < len; i++) {
        if (1) {
            int step = steptbl[predict_idx];

            int diffq = step >> 3;

            if (code & 4) {
                diffq = diffq + step;
            }
            step = step >> 1;
            if (code & 2) {
                diffq = diffq + step;
            }
            step = step >> 1;
            if (code & 1) {
                diffq = diffq + step;
            }

            if (code & 8) {
                predict = predict - diffq;
            } else {
                predict = predict + diffq;
            }

            if (predict > 32767) {
                predict = 32767;
            } else if (predict < -32768) {
                predict = -32768;
            }

            predict_idx = predict_idx + idxtbl[code & 15];

            if (predict_idx < 0) {
                predict_idx = 0;
            } else if (predict_idx > 88) {
                predict_idx = 88;
            }

            if (i & 1) {
                code = *pcode++;
            } else {
                code = code >> 4;
            }
        }

        if (0 && i < NUM_OF_ORIG_SAMPLE) {
            *pd++ = ps[i];
        } else {
            *pd++ = predict;
        }
    }
}


        #elif (TL_AUDIO_MODE == TL_AUDIO_DONGLE_ADPCM_GATT_GOOGLE) //Dongle,GATT GOOGLE

            #define ADPCM_ANDROID_ID 0x00

_attribute_ram_code_ void adpcm_to_pcm(signed short *ps, signed short *pd, int len)
{
    int i;

    int            predict     = (s16)((ps[1] & 0xff00) | (ps[2] & 0x00ff));
    int            predict_idx = (s8)((ps[2] & 0xff00) >> 8);
    unsigned char *pcode       = (unsigned char *)(ps + 3);
    unsigned char  code;
    code = *pcode++;
    code = ((code >> 4) & 0x0f) | ((code << 4) & 0xf0);

    //google: byte7-byte134: 128 byte(62 sample) adpcm data
    //t4h: byte5-byte100: 96 byte(48 sample) adpcm data
    for (i = 0; i < len; i++) {
        int step  = steptbl[predict_idx];
        int diffq = step >> 3;

        if (code & 4) {
            diffq = diffq + step;
        }
        step = step >> 1;
        if (code & 2) {
            diffq = diffq + step;
        }
        step = step >> 1;
        if (code & 1) {
            diffq = diffq + step;
        }

        if (code & 8) {
            predict = predict - diffq;
        } else {
            predict = predict + diffq;
        }

        if (predict > 32767) {
            predict = 32767;
        } else if (predict < -32768) {
            predict = -32768;
        }

        predict_idx = predict_idx + idxtbl[code & 15];

        if (predict_idx < 0) {
            predict_idx = 0;
        } else if (predict_idx > 88) {
            predict_idx = 88;
        }

        if (i & 1) {
            code = *pcode++;
            code = ((code >> 4) & 0x0f) | ((code << 4) & 0xf0); //add by qiuwei for android 8
        } else {
            code = code >> 4;
        }

        if (0 && i < NUM_OF_ORIG_SAMPLE) {
            *pd++ = ps[i];
        } else {
            *pd++ = predict;
        }
    }
}

        #elif (TL_AUDIO_MODE & TL_AUDIO_MASK_HID_SERVICE_CHANNEL) //Dongle,HID SERVICE

int predict     = 0;
int predict_idx = 0;

void adpcm_to_pcm(signed short *ps, signed short *pd, int len)
{
    int            i;
    unsigned char *pcode = (unsigned char *)ps;
    unsigned char  code;
    code = *pcode++;
    code = ((code >> 4) & 0x0f) | ((code << 4) & 0xf0);

    for (i = 0; i < len; i++) {
        if (1) {
            int step = steptbl[predict_idx];

            int diffq = step >> 3;

            if (code & 4) {
                diffq = diffq + step;
            }
            step = step >> 1;
            if (code & 2) {
                diffq = diffq + step;
            }
            step = step >> 1;
            if (code & 1) {
                diffq = diffq + step;
            }

            if (code & 8) {
                predict = predict - diffq;
            } else {
                predict = predict + diffq;
            }

            if (predict > 32767) {
                predict = 32767;
            } else if (predict < -32768) {
                predict = -32768;
            }

            predict_idx = predict_idx + idxtbl[code & 15];

            if (predict_idx < 0) {
                predict_idx = 0;
            } else if (predict_idx > 88) {
                predict_idx = 88;
            }

            if (i & 1) {
                code = *pcode++;
                code = ((code >> 4) & 0x0f) | ((code << 4) & 0xf0);
            } else {
                code = code >> 4;
            }
        }

        if (0 && i < NUM_OF_ORIG_SAMPLE) {
            *pd++ = ps[i];
        } else {
            *pd++ = predict;
        }
    }
}
        #else

        #endif                                                    //end DONGLE_PROJECT adpcm to pcm

    #endif                                                        //end RCU_PROJECT OR DONGLE_PROJECT

#endif
