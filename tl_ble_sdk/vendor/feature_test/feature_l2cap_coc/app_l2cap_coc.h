/********************************************************************************************************
 * @file    app_l2cap_coc.h
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
#ifndef APP_L2CAP_COC_H_
#define APP_L2CAP_COC_H_

#include "tl_common.h"
#include "app_config.h"

#if (FEATURE_TEST_MODE == TEST_L2CAP_COC)

#define CREATE_COC_CONNECT_ACL_COUNT            2
#define COC_CID_COUNT                            15
#define COC_MTU_SIZE                            1000

void app_l2cap_coc_init(void);

int app_host_coc_event_callback (u32 h, u8 *para, int n);
void app_createLeCreditBasedConnect(void);
void app_createCreditBasedConnect(void);
void app_sendCocData(void);
void app_disconnCocConnect(void);

#endif

#endif
