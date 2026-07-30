/********************************************************************************************************
 * @file    svc_core.h
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

#include "stack/ble/profile/services/svc_adv.h"


#ifndef DEFAULT_DEV_APPEARE
    #define DEFAULT_DEV_APPEARE GAP_APPEARANCE_UNKNOWN
#endif

#ifndef DEFAULT_DEV_NAME
    #define DEFAULT_DEV_NAME "B91m_ble_multi_conn"
#endif
#define MAX_DEV_NAME_LEN sizeof(DEFAULT_DEV_NAME) - 1

#ifndef EATT_SUPPORTED_FLAG
    #define EATT_SUPPORTED_FLAG 0
#endif

#ifndef TELINK_CS_TEST_WITH_PHONE
    #define TELINK_CS_TEST_WITH_PHONE 0
#endif

/**
 * @brief      for user add default GATT and GAP service in all GAP server.
 * @param[in]  none.
 * @return     none.
 */
void blc_svc_addCoreGroup(void);

/**
 * @brief      for user remove default GATT and GAP service in all GAP server.
 * @param[in]  none.
 * @return     none.
 */
void blc_svc_removeCoreGroup(void);

/**
 * @brief      for user calculate database hash value(core version >= 5.1).
 * @param[in]  none.
 * @return     none.
 */
void blc_svc_calculateDatabaseHash(void);

/**
 * @brief     for user set device name.
 * @param[in] name: user device name.
 * @return    none.
 */
void blc_svc_setDeviceName(const char *name);

typedef struct
{
    blc_adv_ltv_t ltv;
    u8            flags;
} blc_adv_flags_t;

typedef struct
{
    blc_adv_ltv_t ltv;
    u8            completeName[sizeof(DEFAULT_DEV_NAME)];
} blc_adv_completeName_t;

typedef struct
{
    blc_adv_ltv_t ltv;
    u16           appearance;
} blc_adv_appearance_t;

extern const blc_adv_flags_t        advDefFlags;
extern const blc_adv_completeName_t advDefCompleteName;
extern const blc_adv_appearance_t   advDefAppearance;
