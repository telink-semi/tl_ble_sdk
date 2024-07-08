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

#if (FEATURE_TEST_MODE == TEST_LL_MD)


#define ACL_CENTRAL_MAX_NUM                            4 // ACL central maximum number
#define ACL_PERIPHR_MAX_NUM                            4 // ACL peripheral maximum number




///////////////////////// Feature Configuration////////////////////////////////////////////////
#define ACL_PERIPHR_SMP_ENABLE                        1   //1 for smp,  0 no security
#define ACL_CENTRAL_SMP_ENABLE                        1   //1 for smp,  0 no security
#define ACL_CENTRAL_SIMPLE_SDP_ENABLE                1   //simple service discovery for ACL central

#define BLE_APP_PM_ENABLE                            0



#define APP_DEFAULT_BUFFER_ACL_OCTETS_MTU_SIZE_MINIMUM        1
#define APP_DEFAULT_HID_BATTERY_OTA_ATTRIBUTE_TABLE            1


///////////////////////// UI Configuration ////////////////////////////////////////////////////
#define UI_LED_ENABLE                                1
#define    UI_KEYBOARD_ENABLE                            1

///////////////////////// DEBUG  Configuration ////////////////////////////////////////////////
#define DEBUG_GPIO_ENABLE                            0

#define TLKAPI_DEBUG_ENABLE                            1
#define TLKAPI_DEBUG_CHANNEL                          TLKAPI_DEBUG_CHANNEL_GSUART

#define APP_LOG_EN                                    1
#define APP_CONTR_EVT_LOG_EN                        1    //controller event
#define APP_HOST_EVT_LOG_EN                            1
#define APP_SMP_LOG_EN                                0
#define APP_SIMPLE_SDP_LOG_EN                        0
#define APP_PAIR_LOG_EN                                1
#define APP_KEY_LOG_EN                                1

#define JTAG_DEBUG_DISABLE                            1  //if use JTAG, change this



#include "../../common/default_config.h"

#endif //end of (FEATURE_TEST_MODE == ...)
