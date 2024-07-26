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

#include "../feature_config.h"

#if (FEATURE_TEST_MODE == TEST_OTA)


#define ACL_CENTRAL_MAX_NUM                         1 // ACL central maximum number
#define ACL_PERIPHR_MAX_NUM                         1 // ACL peripheral maximum number


///////////////////////// Feature Configuration////////////////////////////////////////////////
/// Peripheral definition
#define ACL_PERIPHR_SMP_ENABLE                      0   //1 for smp,  0 no security
#define BLE_OTA_SERVER_ENABLE                       1

/// Central definition
#define ACL_CENTRAL_SMP_ENABLE                      0   //1 for smp,  0 no security
#define ACL_CENTRAL_SIMPLE_SDP_ENABLE               0   //simple service discovery for ACL central
#define BLE_OTA_CLIENT_ENABLE                       0
#if (BLE_OTA_CLIENT_ENABLE)
    #define OTA_LEGACY_PROTOCOL                     0  //0: OTA extended protocol; 1: OTA legacy protocol
#endif

#define BLE_APP_PM_ENABLE                           0

//Secure Boot / Firmware Encryption function definition
#define APP_HW_FIRMWARE_ENCRYPTION_ENABLE           0
#define APP_HW_SECURE_BOOT_ENABLE                   0
    #define OTA_CLIENT_SEND_SECURE_BOOT_SIGNATURE_ENABLE     0  //peer device hardware secure boot enable, so OTA client should send signature
    #define OTA_SECURE_BOOT_DESCRIPTOR_SIZE                  0x2000  //8K for B92; if other IC, please change this size

//Speed up OTA function
#define OTA_SERVER_SUPPORT_BIG_PDU_ENABLE           0  //receive big PDU from Central device
#define OTA_CLIENT_SUPPORT_BIG_PDU_ENABLE           0  //send big PDU to Peripheral device

#define APP_DEFAULT_HID_BATTERY_OTA_ATTRIBUTE_TABLE         1

///////////////////////// UI Configuration ////////////////////////////////////////////////////
#define UI_LED_ENABLE                               1
#define UI_KEYBOARD_ENABLE                          1

///////////////////////// DEBUG  Configuration ////////////////////////////////////////////////
#define DEBUG_GPIO_ENABLE                           0

#define TLKAPI_DEBUG_ENABLE                         1
#define TLKAPI_DEBUG_CHANNEL                        TLKAPI_DEBUG_CHANNEL_GSUART

#define APP_LOG_EN                                  1
#define APP_CONTR_EVT_LOG_EN                        1   //controller event
#define APP_HOST_EVT_LOG_EN                         1
#define APP_SMP_LOG_EN                              0
#define APP_SIMPLE_SDP_LOG_EN                       0
#define APP_PAIR_LOG_EN                             1
#define APP_KEY_LOG_EN                              1
#define APP_OTA_CLIENT_LOG_EN                       1

#define JTAG_DEBUG_DISABLE                          1  //if use JTAG, change this



#include "../../common/default_config.h"

#endif
