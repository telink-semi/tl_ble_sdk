/********************************************************************************************************
 * @file    app_config.h
 *
 * @brief   This is the header file for 2.4G SDK
 *
 * @author  2.4G Group
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
#pragma once
#include "config.h"
#if defined(__cplusplus)
extern "C"
{
#endif


/* Enable C linkage for C++ Compilers: */
///////////////////////// Feature Configuration////////////////////////////////////////////////
#define TPLL_PTX  1
#define TPLL_PRX  2
#if (MCU_CORE_TYPE == MCU_CORE_TL323X)
#define TPLL_PTX_3BIT  3  // only tl323x support this mode
#define TPLL_PRX_3BIT  4  // only tl323x support this mode
#endif

#define TPLL_MODE TPLL_PTX

/* Flash Protection:
 * 1. Flash protection is enabled by default in SDK. User must enable this function on their final mass production application.
 * 2. User should use "Unlock" command in Telink BDT tool for Flash access during development and debugging phase.
 * 3. Flash protection demonstration in SDK is a reference design based on sample code. Considering that user's final application may
 *    different from sample code, for example, user's final firmware size is bigger, or user have a different OTA design, or user need
 *    store more data in some other area of Flash, all these differences imply that Flash protection reference design in SDK can not
 *    be directly used on user's mass production application without any change. User should refer to sample code, understand the
 *    principles and methods, then change and implement a more appropriate mechanism according to their application if needed.
 */
#define APP_FLASH_PROTECTION_ENABLE 1

/////////////////////// Board Select Configuration ///////////////////////////////
#if (MCU_CORE_TYPE == MCU_CORE_B91)
    #define BOARD_SELECT BOARD_951X_EVK_C1T213A20
#elif (MCU_CORE_TYPE == MCU_CORE_B92)
    #define BOARD_SELECT BOARD_952X_EVK_C1T266A20
#elif (MCU_CORE_TYPE == MCU_CORE_TL721X)
    #define BOARD_SELECT BOARD_721X_EVK_C1T315A20
#elif (MCU_CORE_TYPE == MCU_CORE_TL321X)
    #define BOARD_SELECT BOARD_321X_EVK_C1T335A20 //BOARD_321X_EVK_C1T335A20
#elif (MCU_CORE_TYPE == MCU_CORE_TL323X)
    #define BOARD_SELECT BOARD_323X_EVK_C1T388A20
#endif

#define rf_stimer_get_tick()     stimer_get_tick()
#define RF_SYSTEM_TIMER_TICK_1US SYSTEM_TIMER_TICK_1US
#define RF_SYSTEM_TIMER_TICK_1MS SYSTEM_TIMER_TICK_1MS
///////////////////////// UI Configuration ////////////////////////////////////////////////////
#define UI_LED_ENABLE      0
#define UI_KEYBOARD_ENABLE 1

///////////////////////// DEBUG  Configuration ////////////////////////////////////////////////
#define DEBUG_GPIO_ENABLE    0

#define RF_DEBUG_IO_ENABLE   1

#define TLKAPI_DEBUG_ENABLE  1
#define TLKAPI_DEBUG_CHANNEL TLKAPI_DEBUG_CHANNEL_GSUART

#define APP_LOG_EN           1
#define APP_KEY_LOG_EN       1

   


#include "../common/default_config.h"

///* Disable C linkage for C++ Compilers: */
#if defined(__cplusplus)
}
#endif
