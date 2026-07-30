/********************************************************************************************************
 * @file    user_config.h
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

#ifndef __PROJECT_ACL_CONN_DEMO__
    #define __PROJECT_ACL_CONN_DEMO__ 0
#endif

#ifndef __PROJECT_ACL_CEN_DEMO__
    #define __PROJECT_ACL_CEN_DEMO__ 0
#endif

#ifndef __PROJECT_ACL_PER_DEMO__
    #define __PROJECT_ACL_PER_DEMO__ 0
#endif

#ifndef __PROJECT_FEATURE_TEST__
    #define __PROJECT_FEATURE_TEST__ 0
#endif

#ifndef __PROJECT_BLE_REMOTE__
    #define __PROJECT_BLE_REMOTE__ 0
#endif

#ifndef __PROJECT_BLE_CONTROLLER__
    #define __PROJECT_BLE_CONTROLLER__ 0
#endif

#ifndef __PROJECT_TPLL_DEMOE__
    #define __PROJECT_TPLL_DEMOE__ 0
#endif

#ifndef __PROJECT_GEN_FSK_DEMOE__
    #define __PROJECT_GEN_FSK_DEMOE__ 0
#endif

#ifndef __PROJECT_2P4G_FEATURE_DEMOE__
    #define __PROJECT_2P4G_FEATURE_DEMOE__ 0
#endif

#ifndef __PROJECT_ESLP_ESL_DEMO__
    #define __PROJECT_ESLP_ESL_DEMO__ 0
#endif

#ifndef __PROJECT_ESLP_AP_DEMO__
    #define __PROJECT_ESLP_AP_DEMO__ 0
#endif


#if (__PROJECT_ACL_CONN_DEMO__)
    #include "vendor/acl_connection_demo/app_config.h"
#elif (__PROJECT_ACL_CEN_DEMO__)
    #include "vendor/acl_central_demo/app_config.h"
#elif (__PROJECT_ACL_PER_DEMO__)
    #include "vendor/acl_peripheral_demo/app_config.h"
#elif (__PROJECT_FEATURE_TEST__)
    #include "vendor/feature_test/app_config.h"
#elif (__PROJECT_BLE_REMOTE__)
    #include "vendor/ble_remote/app_config.h"
#elif (__PROJECT_BLE_CONTROLLER__)
    #include "vendor/ble_controller/app_config.h"
#elif (__PROJECT_ESLP_ESL_DEMO__)
    #include "vendor/eslp_esl_demo/app_config.h"
#elif (__PROJECT_ESLP_AP_DEMO__)
    #include "vendor/eslp_ap_demo/app_config.h"
#elif (__PROJECT_TPLL_DEMOE__)
    #include <vendor/2p4g_tpll/app_config.h>
#elif (__PROJECT_GEN_FSK_DEMOE__)
    #include "vendor/2p4g_gen_fsk/app_config.h"
#elif (__PROJECT_2P4G_FEATURE_DEMOE__)
    #include "vendor/2p4g_feature_test/app_config.h"
#else
    #include "vendor/common/default_config.h"
#endif
