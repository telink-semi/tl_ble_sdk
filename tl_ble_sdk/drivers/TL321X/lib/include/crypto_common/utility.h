/********************************************************************************************************
 * @file    utility.h
 *
 * @brief   This is the header file for TL321X
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
#ifndef UTILITY_H
#define UTILITY_H


#include <stdint.h>    //including int definition
#include <string.h>


#ifdef __cplusplus
extern "C" {
#endif



#define GET_MAX_LEN(a,b)         (((a)>(b))?(a):(b))
#define GET_MIN_LEN(a,b)         (((a)>(b))?(b):(a))
#define GET_WORD_LEN(bitLen)     (((bitLen)+31)/32)
#define GET_BYTE_LEN(bitLen)     (((bitLen)+7)/8)





//APIs

#define PKE_PRINT_BUF
#ifdef PKE_PRINT_BUF
extern void print_buf_U8(unsigned char buf[], unsigned int byteLen, char name[]);
extern void print_buf_U32(unsigned int buf[], unsigned int wordLen, char name[]);
extern void print_BN_buf_U32(unsigned int buf[], unsigned int wordLen, char name[]);
#endif

#define memcpy_  memcpy
#define memset_  memset
#define memcmp_  memcmp

void uint32_set(volatile unsigned int *a, unsigned int value, unsigned int wordLen);

void uint32_copy(volatile unsigned int *dst, volatile const unsigned int *src, unsigned int wordLen);

void uint32_clear(volatile unsigned int *a, unsigned int wordLen);

void uint32_sleep(unsigned int count, unsigned char rand);

void uint32_endian_reverse(unsigned char *in, unsigned char *out, unsigned int wordLen);


void reverse_byte_array(volatile const unsigned char *in, unsigned char *out, unsigned int byteLen);

//void reverse_word(unsigned char *in, unsigned char *out, unsigned int bytelen);

//void dma_reverse_word_array(unsigned int *in, unsigned int *out, unsigned int wordlen, unsigned int reverse_word);

void uint8_XOR(unsigned char *A, unsigned char *B, unsigned char *C, unsigned int byteLen);

void uint32_XOR(unsigned int *A, unsigned int *B, unsigned int *C, unsigned int wordLen);

unsigned int get_valid_bits(const unsigned int *a, unsigned int wordLen);

unsigned int get_valid_words(volatile const unsigned int *a, unsigned int max_words);

unsigned char uint8_BigNum_Check_Zero(unsigned char a[], unsigned int aByteLen);

unsigned int uint32_BigNum_Check_Zero(unsigned int a[], unsigned int aWordLen);

int uint32_BigNumCmp(volatile unsigned int *a, unsigned int aWordLen, volatile const unsigned int *b, unsigned int bWordLen);

unsigned int Get_Multiple2_Number(unsigned int a[]);

unsigned int Big_Div2n(unsigned int a[], int aWordLen, unsigned int n);

unsigned char Bigint_Check_1(volatile unsigned int a[], unsigned int aWordLen);

unsigned char Bigint_Check_p_1(unsigned int a[], unsigned int p[], unsigned int wordLen);

unsigned int uint32_integer_check(unsigned int *k, const unsigned int *n, unsigned int wordLen, unsigned int ret_zero, unsigned int ret_big,
        unsigned int ret_success);

#ifdef __cplusplus
}
#endif

#endif
