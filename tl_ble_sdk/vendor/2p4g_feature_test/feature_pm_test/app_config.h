/********************************************************************************************************
 * @file    app_config.h
 *
 * @brief   This is the header file for 2.4G SDK
 *
 * @author  2.4G GROUP
 * @date    02,2025
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
#pragma once

#include "../feature_config.h"

#if (FEATURE_TEST_MODE == PM)

///////////////////////// Mode Configuration/////////////////////////////////////////////
#define GENFSK                        0
//#define TPSLL                         1
#define TPLL                          2

#define RF_MODE                       GENFSK
///////////////////////// Feature Configuration////////////////////////////////////////////////
#define APP_PM_ENABLE                                       1
#define PM_DEEPSLEEP_RETENTION_ENABLE                       1
#define BLE_APP_PM_ENABLE                                   1

/* Flash Protection:
 * 1. Flash protection is enabled by default in SDK. User must enable this function on their final mass production application.
 * 2. User should use "Unlock" command in Telink BDT tool for Flash access during development and debugging phase.
 * 3. Flash protection demonstration in SDK is a reference design based on sample code. Considering that user's final application may
 *    different from sample code, for example, user's final firmware size is bigger, or user have a different OTA design, or user need
 *    store more data in some other area of Flash, all these differences imply that Flash protection reference design in SDK can not
 *    be directly used on user's mass production application without any change. User should refer to sample code, understand the
 *    principles and methods, then change and implement a more appropriate mechanism according to their application if needed.
 */
#define APP_FLASH_PROTECTION_ENABLE                     1

/* User must check battery voltage on mass production application to prevent abnormal writing or erasing Flash at a low voltage !!! */
#define APP_BATT_CHECK_ENABLE                           0

///////////////////////// DEBUG  Configuration ////////////////////////////////////////////////
#define TLKAPI_DEBUG_ENABLE  0
#define TLKAPI_DEBUG_CHANNEL TLKAPI_DEBUG_CHANNEL_GSUART
#define UART_PRINT_DEBUG_ENABLE             0
#define APP_LOG_EN                          1
#define APP_KEY_LOG_EN                      1
#define APP_OTA_LOG_EN                      1
#define APP_FLASH_INIT_LOG_EN               1
#define APP_FLASH_PROT_LOG_EN               1
#define APP_BATT_CHECK_LOG_EN               1
#define RF_DEBUG_IO_ENABLE                  1
/////////////////////// Sample Board Select Configuration ///////////////////////////////
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
#define UI_LED_ENABLE                                   1

/////////////////// DEEP SAVE FLG //////////////////////////////////
//#if (__PROJECT_2P4G_FEATURE_DEMOE__)
//    #define USED_DEEP_ANA_REG               PM_ANA_REG_WD_CLR_BUF1
//#else
//    #define USED_DEEP_ANA_REG               DEEP_ANA_REG0 //u8,can save 8 bit info when deep
//#endif
//#define LOW_BATT_FLG                        BIT(0) //if 1: low battery
//#define CONN_DEEP_FLG                       BIT(4) //if 1: conn deep, 0: ADV deep



///////////////////////// System Clock  Configuration /////////////////////////////////////////
//#define CLOCK_SYS_CLOCK_HZ                                  16000000


///////////////////////////////////////// PRINT DEBUG INFO ///////////////////////////////////////
//#if (UART_PRINT_DEBUG_ENABLE)
//    #define DEBUG_INFO_TX_PIN               GPIO_PB1
//    #define PULL_WAKEUP_SRC_PB1             PM_PIN_PULLUP_10K
//    #define PB1_OUTPUT_ENABLE               1
//    #define PB1_DATA_OUT                    1
//#endif

//#if TEST_CONN_CURRENT_ENABLE
//    #if DEBUG_GPIO_ENABLE || UART_PRINT_DEBUG_ENABLE || UI_KEYBOARD_ENABLE || UI_LED_ENABLE || UI_BUTTON_ENABLE
//        #error "If testing current, the above definitions must be disable!!!"
//    #endif
//#endif


#include "vendor/common/default_config.h"

#endif // FEATURE_TEST_MODE == PM
