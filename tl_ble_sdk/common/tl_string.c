/********************************************************************************************************
 * @file    tl_string.c
 *
 * @brief   This is the source file for TLSR/TL
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

#include "tl_string.h"
#ifndef DEBUG_MEM_VIOLATION
    #define DEBUG_MEM_VIOLATION 0
#endif
#ifndef assert
    #define assert(expression)
#endif

_attribute_ram_code_sec_ int tstrlen(const char *pStr)
{
    int len = 0;

    if (pStr != 0) {
        while ((*pStr++) != '\0') {
            len++;
            if (len >= 0xFFFFF) {
                break;
            }
        }
    }
    return len;
}

//_attribute_ram_code_sec_ void *tmemset(void *dest, int val, unsigned int len)
//{
//    volatile unsigned char *ptr = (unsigned char *)dest;
//    while (len--) {
//        *ptr++ = (unsigned char)val;
//    }
//    return dest;
//}

_attribute_ram_code_sec_ int tmemcmp(const void *m1, const void *m2, u32 len)
{
    const unsigned char *st1 = (const unsigned char *)m1;
    const unsigned char *st2 = (const unsigned char *)m2;

    while (len--) {
        if (*st1 != *st2) {
            return (*st1 - *st2);
        }
        st1++;
        st2++;
    }
    return 0;
}

_attribute_ram_code_sec_ int tmemcmp4(void *m1, void *m2, register unsigned int len)
{
    unsigned int *st1      = (unsigned int *)m1;
    unsigned int *st2      = (unsigned int *)m2;
    unsigned int  word_len = len >> 2;
    while (word_len--) {
        if (*st1 != *st2) {
            return 1; // return (*st1 - *st2)
        }
        st1++;
        st2++;
    }
    return 0;
}

#if DEBUG_MEM_VIOLATION
extern unsigned int _RETENTION_DATA_VMA_END, _RAMCODE_VMA_END, _RETENTION_RESET_VMA_END;
#endif

volatile unsigned int tdest_addr;
volatile unsigned int tdest_addr_end;

_attribute_ram_code_sec_ void *tmemset(void *dest, int val, unsigned int len)
{
#if DEBUG_MEM_VIOLATION
    unsigned int retention_reset_end  = (unsigned int)&_RETENTION_RESET_VMA_END;
    unsigned int retention_code_start = (unsigned int)&_RETENTION_DATA_VMA_END;
    unsigned int ram_code_end         = (unsigned int)&_RAMCODE_VMA_END;
    unsigned int dest_addr            = (unsigned int)dest;
    unsigned int dest_addr_end        = (unsigned int)dest + len;
    tdest_addr                    = dest_addr;
    tdest_addr_end                = dest_addr_end;
    //  if(((dest_addr>=retention_code_start) && (dest_addr<ram_code_end))||((dest_addr_end>=retention_code_start) &&
    //(dest_addr_end<ram_code_end)))
    if (((dest_addr >= retention_code_start) && (dest_addr < ram_code_end)) ||
        ((dest_addr_end >= retention_code_start) && (dest_addr_end < ram_code_end)) ||
        (dest_addr < retention_reset_end)) {
        __asm__ volatile("ecall");
        return 0;
    }
#endif
    volatile unsigned char *ptr = (unsigned char *)dest;
    while (len--) {
        *ptr++ = (unsigned char)val;
    }
    return dest;
}

_attribute_ram_code_sec_ void tmemset4(void *dest, int val, unsigned int len)
{
#define TRAP_DEBUG_ENABLE 0
#if TRAP_DEBUG_ENABLE
    unsigned int dest_addr     = (unsigned int)dest;
    unsigned int dest_addr_end = (unsigned int)(dest + len * 4) - 1;
    tdest_addr             = dest_addr;
    tdest_addr_end         = dest_addr_end;
    if (((dest_addr >= retention_code_start) && (dest_addr < ram_code_end)) ||
        ((dest_addr_end >= retention_code_start) && (dest_addr_end < ram_code_end)) ||
        (dest_addr < retention_reset_end)) {
        __asm__ volatile("ecall");
        return;
    }
#endif
    unsigned int *ptr = dest;
    len >>= 2;
    while (len--) {
        *(ptr++) = val;
    }
}

_attribute_ram_code_sec_ void tmemcpy1(void *out, const void *in, unsigned int length)
{
#if DEBUG_MEM_VIOLATION
    unsigned int retention_reset_end  = (unsigned int)&_RETENTION_RESET_VMA_END;
    unsigned int retention_code_start = (unsigned int)&_RETENTION_DATA_VMA_END;
    unsigned int ram_code_end         = (unsigned int)&_RAMCODE_VMA_END;
    unsigned int dest_addr            = (unsigned int)out;
    unsigned int dest_addr_end        = (unsigned int)out + length;
    tdest_addr                    = dest_addr;
    tdest_addr_end                = dest_addr_end;
    //  if(((dest_addr>=retention_code_start) && (dest_addr<ram_code_end))||((dest_addr_end>=retention_code_start) &&
    //(dest_addr_end<ram_code_end)))
    if (((dest_addr >= retention_code_start) && (dest_addr < ram_code_end)) ||
        ((dest_addr_end >= retention_code_start) && (dest_addr_end < ram_code_end)) ||
        (dest_addr < retention_reset_end)) {
        __asm__ volatile("ecall");
        return;
    }
#endif
    const unsigned char *pi = (const unsigned char *)in;
    unsigned char *po = (unsigned char *)out;
    while (length--) {
        *po++ = *pi++;
    }

    // return out;
}
// for performance, assume length % 4 == 0,  and no memory overlapped
_attribute_ram_code_sec_ void tmemcpy4(void *d, void *s, unsigned int length)
{
//  if((((int)d)%4!=0) || (((int)s)%4!=0))
//  {
//        __asm__ volatile("ecall");
//        return;
//  }
    unsigned char* source = s;
    unsigned char* dest   = d;
    unsigned int  int_length  = length/4;
    unsigned int  char_length = length%4;
    if(int_length != 0)
    {
        int* s_4 = (int*)source;
        int* d_4 = (int*)dest;
        for(unsigned int i=0;i<int_length;i++)
        {
            d_4[i] = s_4[i];
        }
    }
    if(char_length != 0)
    {
        source+=(4*int_length);
        dest  +=(4*int_length);
        while(char_length--)
        {
            *dest++ = *source++;
        }
    }
    return;
}

_attribute_ram_code_sec_ void tmemcpy(void *dest, const void *src, unsigned int length)
{
    unsigned char *d = (unsigned char *)dest;
    const unsigned char *s = (const unsigned char *)src;

    unsigned int d_align_offset = (unsigned int)d % 4;
    unsigned int s_align_offset = (unsigned int)s % 4;

    if (d_align_offset == s_align_offset) {
        unsigned int prefix_len = (4 - s_align_offset) % 4;
        unsigned int copy_prefix = (length > prefix_len) ? prefix_len : length;
        for (unsigned int i = 0; i < copy_prefix; ++i) {
            d[i] = s[i];
        }
        d += copy_prefix;
        s += copy_prefix;
        length -= copy_prefix;

        unsigned int words_num = length / 4;
        if (words_num > 0) {
            unsigned int *d_32 = (unsigned int *)d;
            const unsigned int *s_32 = (const unsigned int *)s;
            for (unsigned int i = 0; i < words_num; ++i) {
                d_32[i] = s_32[i];
            }
            d += words_num * 4;
            s += words_num * 4;
            length -= words_num * 4;
        }
    }

    while (length--) {
        *d++ = *s++;
    }
}

#if 0
void __assert_func(const char *mess)
{

}
#endif
