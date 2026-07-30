/********************************************************************************************************
 * @file    compiler.h
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
#ifndef COMPILER_H_
#define COMPILER_H_

/******************************* platform code Use  *********************************/
#define _attribute_ram_code_sec_            __attribute__((section(".ram_code")))
#define _attribute_ram_code_sec_noinline_   __attribute__((section(".ram_code"))) __attribute__((noinline))

#define _attribute_text_sec_                __attribute__((section(".text"))) __attribute__((noinline)) //Inlining happens when __attribute__((noinline)) is not added.
#ifndef STD_GCC //standard open source risc-V GCC
#define _attribute_text_sec_optimize_o2_    __attribute__((section(".text"))) __attribute__((optimize("O2"))) __attribute__((noinline)) __attribute__((no_execit))
#else
#define _attribute_text_sec_optimize_o2_    __attribute__((section(".text"))) __attribute__((noinline))
#endif

#ifndef STD_GCC //standard open source risc-V GCC
#define _attribute_flash_code_sec_noinline_     __attribute__((section(".flash_code"))) __attribute__((optimize("O2"))) __attribute__((noinline)) __attribute__((no_execit))
#else
#define _attribute_flash_code_sec_noinline_     __attribute__((section(".flash_code"))) __attribute__((noinline))
#endif

#define _attribute_aes_data_sec_            __attribute__((section(".aes_data")))

//#define _attribute_data_retention_sec_      __attribute__((section(".retention_data")))

#define _attribute_aligned_(s)              __attribute__((aligned(s)))

/**
 *  _always_inline needs to be added in the following two cases:
 * 1. The subfunctions in the pm_sleep_wakeup function need to use _always_inline and _attribute_ram_code_sec_noinline_, as detailed in the internal comments of pm_sleep_wakeup.
 * 2. The BLE SDK uses interrupt preemption, flash interface operations can be interrupted by high-priority interrupts, which requires that the high-priority interrupt handler function,can not have a text segment of code.
 *    So the BLE SDK provides the following requirements: Add the following function to _always_inline: rf_set_tx_rx_off, rf_set_ble_crc_adv, rf_set_ble_crc_value, rf_set_rx_maxlen, stimer_get_tick, clock_time_exceed, rf_receiving_flag, dma_config, gpio_toggle, gpio_set_low_level, gpio_set_level.
 */
#define _always_inline inline __attribute__((always_inline))

/**
 * No_execit must be added here for the following reasons: When compiling at the optimization level of -Os, link may use exec.it for functions compiled at -O2. To disable this behavior,
 * add -mno-exit to the linking phase (see Andes Programming Guide), or add _attribute_((no_execit)) to functions that don't want to use exec.it.
 */
#ifndef STD_GCC //standard open source risc-V GCC
    #define _attribute_ram_code_sec_optimize_o2_          __attribute__((section(".ram_code"))) __attribute__((optimize("O2"))) __attribute__((no_execit))
    #define _attribute_ram_code_sec_optimize_o2_noinline_ __attribute__((noinline)) __attribute__((section(".ram_code"))) __attribute__((optimize("O2"))) __attribute__((no_execit))
#else
    #define _attribute_ram_code_sec_optimize_o2_          __attribute__((section(".ram_code"))) __attribute__((optimize("O2")))
    #define _attribute_ram_code_sec_optimize_o2_noinline_ __attribute__((noinline)) __attribute__((section(".ram_code"))) __attribute__((optimize("O2")))
#endif

/// Pack a structure field
#define __PACKED __attribute__((__packed__))

/*******************************      BLE Stack Use     ******************************/
#include "common/config/user_config.h"

#define _attribute_session_(s)               __attribute__((section(s)))

#define _attribute_no_inline_                __attribute__((noinline))
#define _attribute_data_dlm_                 _attribute_session_(".dlm_data")   // dlm:Data Local Memory
#define _attribute_data_sec_                 __attribute__((section(".data")))  // Force only read data to be stored in data segments to avoid compiler optimization

#define _attribute_iram_noinit_data_         __attribute__((section(".iram_noinit_data")))
#define _attribute_iram_bss_                 __attribute__((section(".iram_bss")))
#define _attribute_iram_rbt_ret_data_        __attribute__((section(".iram_rbt_ret")))  // iram reboot retention data

#if (BLC_PM_DEEP_RETENTION_MODE_EN)
    #define _attribute_data_retention_sec_   __attribute__((section(".retention_data")))

    #ifndef BLC_ZEPHYR_BLE_INTEGRATION
        #define _attribute_data_retention_       __attribute__((section(".retention_data")))    // used for ble
        #define _attribute_ble_data_retention_   __attribute__((section(".retention_data")))    // used for ble
    #else
        #define _attribute_data_retention_       __attribute__((section(".retention_data_ble")))    // used for ble
        #define _attribute_ble_data_retention_   __attribute__((section(".retention_data_ble")))    // used for ble
    #endif
#else
    #ifndef BLC_ZEPHYR_BLE_INTEGRATION
        #define _attribute_data_retention_sec_
        #define _attribute_data_retention_
        #define _attribute_ble_data_retention_
    #else
        #define _attribute_data_retention_sec_   __attribute__((section(".retention_data")))
        #define _attribute_data_retention_       __attribute__((section(".retention_data")))
        #define _attribute_ble_data_retention_   __attribute__((section(".retention_data")))
    #endif
#endif

#ifndef BLC_ZEPHYR_BLE_INTEGRATION
    #define _attribute_ram_code_                  __attribute__((section(".ram_code"))) __attribute__((noinline))
    #define _attribute_ram_code_only_             __attribute__((section(".ram_code")))
    #define _attribute_ble_ram_code_noinline_     __attribute__((section(".ram_code"), noinline))
    #define _attribute_ble_ram_code_              __attribute__((section(".ram_code")))
#else
    #define _attribute_ram_code_                  __attribute__((section(".ram_code_ble"))) __attribute__((noinline))
    #define _attribute_ram_code_only_             __attribute__((section(".ram_code_ble")))
    #define _attribute_ble_ram_code_noinline_     __attribute__((section(".ram_code_ble"), noinline))
    #define _attribute_ble_ram_code_              __attribute__((section(".ram_code_ble")))
#endif

#ifndef STD_GCC //standard open source risc-V GCC, used for ble
    #define _attribute_ble_ram_code_optimize_o2_            __attribute__((section(".ram_code"))) __attribute__((optimize("O2"))) __attribute__((no_execit))
    #define _attribute_ble_ram_code_optimize_o2_noinline_   __attribute__((noinline)) __attribute__((section(".ram_code"))) __attribute__((optimize("O2"))) __attribute__((no_execit))
#else
    #ifndef BLC_ZEPHYR_BLE_INTEGRATION
        #define _attribute_ble_ram_code_optimize_o2_            __attribute__((section(".ram_code"), optimize("O2")))
        #define _attribute_ble_ram_code_optimize_o2_noinline_   __attribute__((noinline)) __attribute__((section(".ram_code"), optimize("O2")))
    #else
        #define _attribute_ble_ram_code_optimize_o2_            __attribute__((section(".ram_code_ble"), optimize("O2")))
        #define _attribute_ble_ram_code_optimize_o2_noinline_   __attribute__((noinline)) __attribute__((section(".ram_code_ble"), optimize("O2")))
    #endif
#endif

#define _attribute_text_code_                 __attribute__((section(".text")))

/// define the force inlining attribute for this compiler
#define __INLINE static __attribute__((__always_inline__)) inline

#ifdef BLC_ZIGBEE_INTEGRATION
    #undef _attribute_data_retention_sec_
    #define _attribute_data_retention_sec_

    #undef  _attribute_data_retention_
    #define _attribute_data_retention_
    #undef  _attribute_ble_data_retention_
    #define _attribute_ble_data_retention_
#endif /* BLC_ZIGBEE_INTEGRATION */

#endif
