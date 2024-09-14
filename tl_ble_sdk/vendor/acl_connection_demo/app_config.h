/********************************************************************************************************
 * @file    app_config.h
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
#pragma once

#include "config.h"


#define ACL_CENTRAL_MAX_NUM                         4 // ACL central maximum number
#define ACL_PERIPHR_MAX_NUM                         4 // ACL peripheral maximum number

///////////////////////// Feature Configuration////////////////////////////////////////////////
#define ACL_PERIPHR_SMP_ENABLE                      1  //1 for smp,  0 no security
#define ACL_CENTRAL_SMP_ENABLE                      1  //1 for smp,  0 no security
#define ACL_CENTRAL_SIMPLE_SDP_ENABLE               1  //simple service discovery for ACL central
#define BLE_OTA_SERVER_ENABLE                       1

#define BLE_APP_PM_ENABLE                           1

#define BATT_CHECK_ENABLE                           0


/* Flash Protection:
 * 1. Flash protection is enabled by default in SDK. User must enable this function on their final mass production application.
 * 2. User should use "Unlock" command in Telink BDT tool for Flash access during development and debugging phase.
 * 3. Flash protection demonstration in SDK is a reference design based on sample code. Considering that user's final application may
 *    different from sample code, for example, user's final firmware size is bigger, or user have a different OTA design, or user need
 *    store more data in some other area of Flash, all these differences imply that Flash protection reference design in SDK can not
 *    be directly used on user's mass production application without any change. User should refer to sample code, understand the
 *    principles and methods, then change and implement a more appropriate mechanism according to their application if needed.
 */
#define APP_FLASH_PROTECTION_ENABLE                 1

/* Flash 4line mode:
 *  enable the 4 line mode of flash, read and write.
 */
#define FLASH_4LINE_MODE_ENABLE                     0


///////////////////////// OS settings /////////////////////////////////////////////////////////
#define FREERTOS_ENABLE                             0
#define OS_SEPARATE_STACK_SPACE                     1   //Separate the task stack and interrupt stack space


/////////////////////// Board Select Configuration ///////////////////////////////
#if (MCU_CORE_TYPE == MCU_CORE_B91)
    #define BOARD_SELECT                                BOARD_951X_EVK_C1T213A20
#elif (MCU_CORE_TYPE == MCU_CORE_B92)
    #define BOARD_SELECT                                BOARD_952X_EVK_C1T266A20
#elif (MCU_CORE_TYPE == MCU_CORE_TL721X)
    #define BOARD_SELECT                                BOARD_721X_EVK_C1T315A20
#elif (MCU_CORE_TYPE == MCU_CORE_TL321X)
    #define BOARD_SELECT                                BOARD_321X_EVK_C1T331A20 //BOARD_321X_EVK_C1T335A20
#endif

///////////////////////// UI Configuration ////////////////////////////////////////////////////
#define UI_LED_ENABLE                               1
#define UI_KEYBOARD_ENABLE                          1

///////////////////////// DEBUG  Configuration ////////////////////////////////////////////////
#define DEBUG_GPIO_ENABLE                           0

#define TLKAPI_DEBUG_ENABLE                         1
#define TLKAPI_DEBUG_CHANNEL                        TLKAPI_DEBUG_CHANNEL_GSUART

#define APP_LOG_EN                                  1
#define APP_FLASH_INIT_LOG_EN                       1
#define APP_CONTR_EVT_LOG_EN                        1   //controller event
#define APP_HOST_EVT_LOG_EN                         1
#define APP_SMP_LOG_EN                              0
#define APP_SIMPLE_SDP_LOG_EN                       0
#define APP_PAIR_LOG_EN                             1
#define APP_KEY_LOG_EN                              1

#define JTAG_DEBUG_DISABLE                          1  //if use JTAG, change this




/////////////////// DEEP SAVE FLG //////////////////////////////////
#define USED_DEEP_ANA_REG                   PM_ANA_REG_POWER_ON_CLR_BUF1 //u8,can save 8 bit info when deep
#define LOW_BATT_FLG                        BIT(0) //if 1: low battery
#define CONN_DEEP_FLG                       BIT(1) //if 1: conn deep, 0: adv deep




#if FREERTOS_ENABLE
    #define traceAPP_LED_Task_Toggle()  //gpio_toggle(GPIO_CH01);
    #define traceAPP_BLE_Task_BEGIN()   //gpio_write(GPIO_CH02,1);
    #define traceAPP_BLE_Task_END()     //gpio_write(GPIO_CH02,0);
    #define traceAPP_KEY_Task_BEGIN()   //gpio_write(GPIO_CH03,1);
    #define traceAPP_KEY_Task_END()     //gpio_write(GPIO_CH03,0);
    #define traceAPP_BAT_Task_BEGIN()   //gpio_write(GPIO_CH04,1);
    #define traceAPP_BAT_Task_END()     //gpio_write(GPIO_CH04,0);

    #define traceAPP_MUTEX_Task_BEGIN() //gpio_write(GPIO_CH05,1);
    #define traceAPP_MUTEX_Task_END()   //gpio_write(GPIO_CH05,0);

    #define tracePort_IrqHandler_BEGIN() //gpio_write(GPIO_CH06,1);
    #define tracePort_IrqHandler_END()   //gpio_write(GPIO_CH06,0);

#endif



#include "../common/default_config.h"
