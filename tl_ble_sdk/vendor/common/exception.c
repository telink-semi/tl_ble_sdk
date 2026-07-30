/********************************************************************************************************
 * @file    exception.c
 *
 * @brief   This is the source file for tl521x
 *
 * @author  Driver Group
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
#include "driver.h"
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
 * @brief     Default exception irq handler.
 * @return      none
 */
__attribute__((weak)) void except_handler(void)
{
    /* unhandled trap */
    exception_mtval   = read_csr(NDS_MTVAL);
    exception_mepc    = read_csr(NDS_MEPC);
    exception_mstatus = read_csr(NDS_MSTATUS);
    exception_mcause  = read_csr(NDS_MCAUSE);
    exception_mdcause = read_csr(NDS_MDCAUSE);

    while (1) {
        __asm__("nop");
    }
}

/**
 * @brief       This function service to handle all the platform pre-defined interrupt or exception.
 * @return      none
 * @note        trap_entry() must align 64bytes.
 */
_attribute_ram_code_sec_noinline_ void trap_entry(void) __attribute__((interrupt("machine"), aligned(64)));

__attribute__((weak)) void trap_entry(void)
{
    except_handler();
}
