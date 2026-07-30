/********************************************************************************************************
 * @file    tl_string.h
 *
 * @brief   This is the header file for TLSR/TL
 *
 * @author  2.4G Group
 * @date    2025
 *
 * @par     Copyright (c) 2025, Telink Semiconductor (Shanghai) Co., Ltd. ("TELINK")
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
#pragma GCC optimize "no-tree-loop-distribute-patterns"
#pragma once

#include "compiler.h"
#include "types.h"


#ifndef NULL
    #define NULL 0
#endif

//void *tmemset(void *d, int c, unsigned int n);

int   tstrlen(const char *pStr);
int   tmemcmp(const void *m1, const void *m2, u32 len);
int   tmemcmp4(void *m1, void *m2, register unsigned int len);
void *tmemset(void *dest, int val, unsigned int len);
void  tmemcpy1(void *out, const void *in, unsigned int length);
void  tmemcpy4(void *d, void *s, unsigned int length);
void  tmemcpy(void *dest, const void *src, unsigned int length);

/**
 * @brief  set value, for performance, assume length % 4 == 0,  and no memory overlapped.
 *
 * @param[in]  dest: data address
 * @param[in]  val: value
 * @param[in]  length: data length
 * @param[out] none
 *
 * @returns 0-fail other-success
 */
void tmemset4(void *dest, int val, unsigned int length);

extern volatile unsigned int tdest_addr;
extern volatile unsigned int tdest_addr_end;

//#define smemcmp  tmemcmp
//#define smemcmp4 tmemcmp4
//#define smemset  tmemset
//#define smemcpy  tmemcpy1
//#define smemcpy4 tmemcpy4
//#define smemset4 tmemset4
