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

#define ACL_CENTRAL_MAX_NUM               0            // ACL central maximum number
#define ACL_PERIPHR_MAX_NUM               1            // ACL peripheral maximum number
#define LEGACY_ADV_SEND                   1
#define BLE_OTA_SERVER_ENABLE             1
#define BLT_SOFTWARE_TIMER_ENABLE         1
#define HW_EVK                            1
#define HW_C1T335A78                      2            // TL321X

#define HARDWARE_BOARD_SELECT             HW_EVK
#define BOARD_SELECT                      BOARD_321X_EVK_C1T335A20

#define APP_IMAGE_STORAGE_FLASH                     1
#if (APP_IMAGE_STORAGE_FLASH)
#define OTS_SERVER_MAX_OBJECTS_NUM                  0x21
#define APP_IMAGE_STORAGE_MAX_IMAGES                0x20
#else
#define OTS_SERVER_MAX_OBJECTS_NUM                  0x02
#define APP_IMAGE_STORAGE_MAX_IMAGES                0x01
#endif
#define OTS_SERVER_MAX_OBJECT_NAME_LENGTH           8
#define APP_IMAGE_STORAGE_MAX_IMAGE_SIZE            0x1280
#define APP_VENDOR_IMAGE                            1
#if (HARDWARE_BOARD_SELECT != HW_EVK)
    #define APP_EPD_DISPLAY 1
#else
    #define APP_EPD_DISPLAY 0
#endif

#define ESLS_DISPLAYS_SUPPORTED 1
#define ESLS_SENSORS_SUPPORTED  1
#define ESLS_LEDS_SUPPORTED     2

///////////////////////// Feature Configuration////////////////////////////////////////////////
#define ACL_PERIPHR_SMP_ENABLE        1 //1 for smp,  0 no security
#define ACL_CENTRAL_SMP_ENABLE        0 //1 for smp,  0 no security
#define ACL_CENTRAL_SIMPLE_SDP_ENABLE 1 //simple service discovery for ACL central

#define BLE_APP_PM_ENABLE             1
#define PM_DEEPSLEEP_RETENTION_ENABLE 1

#define BATT_CHECK_ENABLE             0

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

///////////////////////// UI Configuration ////////////////////////////////////////////////////
#define UI_LED_ENABLE 1
#if (HARDWARE_BOARD_SELECT != HW_EVK)
    #define UI_KEYBOARD_ENABLE 0
#else
    #define UI_KEYBOARD_ENABLE 0
#endif

///////////////////////// DEBUG  Configuration ////////////////////////////////////////////////
#define DEBUG_GPIO_ENABLE 0

#if (HARDWARE_BOARD_SELECT != HW_EVK)
    #define TLKAPI_DEBUG_ENABLE 0
#else
    #define TLKAPI_DEBUG_ENABLE 1
#endif
#define TLKAPI_DEBUG_CHANNEL  TLKAPI_DEBUG_CHANNEL_GSUART

#define APP_LOG_EN            1
#define APP_PAWR_EVT_LOG_EN   1 //controller event
#define APP_HOST_EVT_LOG_EN   1
#define APP_SMP_LOG_EN        0
#define APP_SIMPLE_SDP_LOG_EN 0
#define APP_PAIR_LOG_EN       1
#define APP_KEY_LOG_EN        1




#include "../common/default_config.h"
