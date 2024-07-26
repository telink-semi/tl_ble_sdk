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
#define ACL_PERIPHR_MAX_NUM                         0 // ACL peripheral maximum number


///////////////////////// Feature Configuration////////////////////////////////////////////////
#define ACL_PERIPHR_SMP_ENABLE                      0   //1 for smp,  0 no security
#define ACL_CENTRAL_SMP_ENABLE                      1   //1 for smp,  0 no security
#define ACL_CENTRAL_SIMPLE_SDP_ENABLE               1   //simple service discovery for ACL central

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

///////////////////////// UI Configuration ////////////////////////////////////////////////////
/// B91:
///     EVK: C1T213A20
///     Dongle: C1T213A3
/// B92:
///     EVK: C1T266A20
#define HW_EVK                                      1
#define HW_DONGLE                                   2

#define HARDWARE_BOARD_SELECT                       HW_EVK

#if(HARDWARE_BOARD_SELECT == HW_EVK)
    #define UI_KEYBOARD_ENABLE                      1
    #define UI_BUTTON_ENABLE                        0
#elif(HARDWARE_BOARD_SELECT == HW_DONGLE)
    #define UI_KEYBOARD_ENABLE                      0
    #define UI_BUTTON_ENABLE                        1
#endif
#define UI_LED_ENABLE                               1
#define APPLICATION_DONGLE                          0

///////////////////////// DEBUG  Configuration ////////////////////////////////////////////////
#define DEBUG_GPIO_ENABLE                       0

#define TLKAPI_DEBUG_ENABLE                         1
#define TLKAPI_DEBUG_CHANNEL                        TLKAPI_DEBUG_CHANNEL_GSUART

#define APP_LOG_EN                                  1
#define APP_CONTR_EVT_LOG_EN                        1   //controller event
#define APP_HOST_EVT_LOG_EN                         1
#define APP_SMP_LOG_EN                              0
#define APP_SIMPLE_SDP_LOG_EN                       0
#define APP_PAIR_LOG_EN                             1
#define APP_KEY_LOG_EN                              1

#define JTAG_DEBUG_DISABLE                          1  //if use JTAG, change this


/**
 *  @brief  GPIO definition for Button
 */
#if (UI_BUTTON_ENABLE)
    #define SW1_GPIO                GPIO_PB2
    #define SW2_GPIO                GPIO_PB3
    #define PB2_FUNC                AS_GPIO
    #define PB3_FUNC                AS_GPIO
    #define PB2_INPUT_ENABLE        1
    #define PB3_INPUT_ENABLE        1
    #define PULL_WAKEUP_SRC_PB2     PM_PIN_PULLUP_10K
    #define PULL_WAKEUP_SRC_PB3     PM_PIN_PULLUP_10K
#endif


/**
 *  @brief  GPIO definition for dongle USB
 */
#if(APPLICATION_DONGLE)
    #define ID_VENDOR               0x248a          // for report
    #define ID_PRODUCT_BASE         0x880c          //AUDIO_HOGP
    #define STRING_VENDOR           L"Telink"
    #define STRING_PRODUCT          L"BLE Master Dongle"
    #define STRING_SERIAL           L"TLSR9"

    //////////////////// Audio /////////////////////////////////////
    #define MIC_RESOLUTION_BIT      16
    #define MIC_SAMPLE_RATE         16000
    #define MIC_CHANNEL_COUNT       1
    #define MIC_ENCODER_ENABLE      0

    #define SPK_RESOLUTION_BIT      16
    #define SPK_SAMPLE_RATE         48000
    #define SPK_CHANNEL_COUNT       2

    #define PA5_FUNC                AS_USB_DM
    #define PA6_FUNC                AS_USB_DP
    #define PA5_INPUT_ENABLE        1
    #define PA6_INPUT_ENABLE        1

    #define USB_PRINTER_ENABLE      1
    #define USB_SPEAKER_ENABLE      0

    #define AUDIO_HOGP              0

    #define USB_MIC_ENABLE          1
    #define USB_MOUSE_ENABLE        1
    #define USB_KEYBOARD_ENABLE     1
    #define USB_SOMATIC_ENABLE      0   //  when USB_SOMATIC_ENABLE, USB_EDP_PRINTER_OUT disable
    #define USB_CUSTOM_HID_REPORT   1
#endif




/////////////////// DEEP SAVE FLG //////////////////////////////////
#define USED_DEEP_ANA_REG                   PM_ANA_REG_POWER_ON_CLR_BUF1 //u8,can save 8 bit info when deep
#define LOW_BATT_FLG                        BIT(0) //if 1: low battery
#define CONN_DEEP_FLG                       BIT(1) //if 1: conn deep, 0: adv deep




#include "../common/default_config.h"
