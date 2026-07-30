/********************************************************************************************************
 * @file    usb_util.h
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
#ifndef __USB_UTIL_H__
#define __USB_UTIL_H__

#ifndef PACKED
    #define PACKED __attribute__((packed))
#endif

#ifndef WEAK
    #define WEAK __attribute__((weak))
#endif

#ifndef U16_LOW_BYTE
    #define U16_LOW_BYTE(x) (unsigned char)((x) & 0x00ff)
#endif

#ifndef U16_HIGH_BYTE
    #define U16_HIGH_BYTE(x) (unsigned char)(((x) >> 8) & 0xff)
#endif

#ifndef MAX
    #define MAX(a, b) (((a) > (b)) ? (a) : (b))
#endif

#ifndef MIN
    #define MIN(a, b) (((a) < (b)) ? (a) : (b))
#endif

#endif
