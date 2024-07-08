/********************************************************************************************************
 * @file    utility.c
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
#include "drivers.h"
#include "utility.h"


// general swap/endianness utils

void swapN(unsigned char *p, int n)
{
    int i, c;
    for (i=0; i<n/2; i++)
    {
        c = p[i];
        p[i] = p[n - 1 - i];
        p[n - 1 - i] = c;
    }
}

void swapX(const u8 *src, u8 *dst, int len)
{
    int i;
    for (i = 0; i < len; i++)
        dst[len - 1 - i] = src[i];
}

void swap24(u8 dst[3], const u8 src[3])
{
    swapX(src, dst, 3);
}

void swap32(u8 dst[4], const u8 src[4])
{
    swapX(src, dst, 4);
}

void swap48(u8 dst[6], const u8 src[6])
{
    swapX(src, dst, 6);
}

void swap56(u8 dst[7], const u8 src[7])
{
    swapX(src, dst, 7);
}

void swap64(u8 dst[8], const u8 src[8])
{
    swapX(src, dst, 8);
}

void swap128(u8 dst[16], const u8 src[16])
{
    swapX(src, dst, 16);
}



void flip_addr(u8 *dest, u8 *src){
    dest[0] = src[5];
    dest[1] = src[4];
    dest[2] = src[3];
    dest[3] = src[2];
    dest[4] = src[1];
    dest[5] = src[0];
}



void my_fifo_init (my_fifo_t *f, int s, u8 n, u8 *p)
{
    f->size = s;
    f->num = n;
    f->wptr = 0;
    f->rptr = 0;
    f->p = p;
}

u8* my_fifo_wptr (my_fifo_t *f)
{
    if (((f->wptr - f->rptr) & 255) < f->num)
    {
        return f->p + (f->wptr & (f->num-1)) * f->size;
    }
    return 0;
}

u8* my_fifo_wptr_v2 (my_fifo_t *f)
{
    if (((f->wptr - f->rptr) & 255) < f->num - 3) //keep 3 fifo left for others evt
    {
        return f->p + (f->wptr & (f->num-1)) * f->size;
    }
    return 0;
}

void my_fifo_next (my_fifo_t *f)
{
    f->wptr++;
}

int my_fifo_push (my_fifo_t *f, u8 *p, int n)
{
    if (((f->wptr - f->rptr) & 255) >= f->num)
    {
        return -1;
    }

    if (n >= (int)f->size)
    {
        return -1;
    }
    u8 *pd = f->p + (f->wptr++ & (f->num-1)) * f->size;
    *pd++ = n & 0xff;
    *pd++ = (n >> 8) & 0xff;
    memcpy (pd, p, n);
    return 0;
}

void my_fifo_pop (my_fifo_t *f)
{
    f->rptr++;
}

u8 * my_fifo_get (my_fifo_t *f)
{
    if (f->rptr != f->wptr)
    {
        u8 *p = f->p + (f->rptr & (f->num-1)) * f->size;
        return p;
    }
    return 0;
}

void my_ring_buffer_init (my_ring_buf_t *f,u8 *p, int s)
{
    f->size = s;  //size
    f->mask = s -1;
    f->wptr = 0;  //head
    f->rptr = 0;  //tail
    f->p = p;     //Actual cache
}

bool my_ring_buffer_is_empty(my_ring_buf_t *f) {
  return (f->wptr == f->rptr) ? true : false;
}

u8 my_ring_buffer_is_full(my_ring_buf_t*f) {
  return ((f->wptr - f->rptr) & f->mask) == f->mask;
}

void my_ring_buffer_flush(my_ring_buf_t*f) {
    f->rptr = f->wptr;
}

/**
 * @brief   Cache free space
 * @param  ring_buf
 * @return number.
 */
u16 my_ring_buffer_free_len(my_ring_buf_t *f)
{
    u16 size;
    size = (f->wptr - f->rptr) & f->mask;
    size = f->size - size;
  return size;
}

u16 my_ring_buffer_data_len(my_ring_buf_t *f)
{
     return (f->wptr - f->rptr) & f->mask;
}

bool my_ring_buffer_push_byte(my_ring_buf_t *f, u8 data)
{
  f->p[f->wptr] = data;
  f->wptr = ((f->wptr + 1) & f->mask);
  return true;
}

void my_ring_buffer_push_bytes(my_ring_buf_t *f, u8 *data, u16 size)
{
    u16 i;
    for(i = 0; i < size; i++) {
      my_ring_buffer_push_byte(f, data[i]);
    }
}

u8 my_ring_buffer_pull_byte(my_ring_buf_t *f)
{
    u8 data;
    data = f->p[f->rptr];
    f->rptr = ((f->rptr + 1) & f->mask);
    return data;
}

void my_ring_buffer_pull_bytes(my_ring_buf_t *f, u8 *data, u16 size)
{
    u16 i;
    for(i = 0; i < size; i++) {
        data[i] = f->p[f->rptr];
        f->rptr = ((f->rptr + 1) & f->mask);
    }
}

void my_ring_buffer_delete(my_ring_buf_t *f, u16 size)
{
    f->rptr = ((f->rptr + size) & f->mask);
}

u8 my_ring_buffer_get(my_ring_buf_t *f, u16 offset)
{
    u8 data;
    u16 rptr = ((f->rptr + offset) & f->mask);
    data = f->p[rptr];
    return data;
}




const char *hex_to_str(const void *buf, u8 len)
{
    static const char hex[] = "0123456789abcdef";
    static char str[301];
    const uint8_t *b = buf;
    u8 i;

    len = min(len, (sizeof(str) - 1) / 3);

    for (i = 0; i < len; i++) {
        str[i * 3]     = hex[b[i] >> 4];
        str[i * 3 + 1] = hex[b[i] & 0xf];
        str[i * 3 + 2] = ' ';
    }

    str[i * 3] = '\0';

    return str;
}

const char *addr_to_str(u8* addr)
{
#define BDADDR_STR_LEN        18
    static char addrStr[BDADDR_STR_LEN];
    snprintf(addrStr, sizeof(addrStr), "%02X:%02X:%02X:%02X:%02X:%02X", addr[5], addr[4], addr[3], addr[2], addr[1], addr[0]);
    return addrStr;
}
