/********************************************************************************************************
 * @file    plic_isr.c
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
/*
 * All interrupt entry functions and weak definitions of related processing functions are defined here,If you dont use the relevant interrupt function,
 *  you can delete it to save ram_code space.
 ****************************************************************************************
 */
#include "lib/include/plic.h"
/**
  * @brief The value of the mtval register when the exception is entered.
  */
 static volatile long exception_mtval;

 /**
  * @brief The value of the mepc register when the exception is entered.
  */
 static volatile long exception_mepc;

 /**
  * @brief The value of the mstatus register when the exception is entered.
  */
 static volatile long exception_mstatus;

 /**
  * @brief The value of the mcause register when the exception is entered.
  */
 static volatile long exception_mcause;

 /**
  * @brief The value of the mdcause register when the exception is entered.
  */
 static volatile long exception_mdcause;
 /**
  * @brief  exception handler.this defines an exception handler to handle all the platform pre-defined exceptions.
  * @return none
  */
 _attribute_ram_code_sec_ __attribute__((weak)) void except_handler(void)
 {
        exception_mtval = read_csr(NDS_MTVAL);
        exception_mepc = read_csr(NDS_MEPC);
        exception_mstatus = read_csr(NDS_MSTATUS);
        exception_mcause = read_csr(NDS_MCAUSE);
        exception_mdcause = read_csr(NDS_MDCAUSE);
    while(1){
        /* Unhandled Trap */
        for(volatile unsigned int i = 0; i < 0xffff; i++)
        {
            __asm__("nop");
        }
    }
 }
 _attribute_ram_code_sec_noinline_  __attribute__((weak)) void trap_entry(void) __attribute__ ((interrupt ("machine") , aligned(4)));
 void trap_entry(void)
 {
    except_handler();
 }
