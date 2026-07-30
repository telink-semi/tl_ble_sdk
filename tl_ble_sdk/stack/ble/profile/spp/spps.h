/********************************************************************************************************
 * @file    spps.h
 *
 * @brief   This is the header file for BLE SDK
 *
 * @author  BLE GROUP
 * @date    03,2025
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

#include "spps_client_buf.h"

/******************************* SPPS Common Start **********************************************************************/

/******************************* SPPS Common End **********************************************************************/

/******************************* SPPS Client Start **********************************************************************/
//SPPS Client Event ID
enum
{
    SPPSC_EVT_BASC_START = SPP_EVT_TYPE_SPPSC,
    SPPSC_EVT_NOTIF, //refer to 'struct blc_sppsc_notifEvt'
};

typedef struct
{
}blc_sppsc_regParam_t;

typedef struct
{ //Event ID: SPPSC_EVT_NOTIF
    u8 data[0];
}blc_sppsc_notifEvt_t;

void      blc_spps_registerSPPSControlClient(const blc_sppsc_regParam_t *param);
ble_sts_t blc_sppsc_writeSppData(u16 connHandle, u16 length, u8 *data);

/******************************* SPPS Client End **********************************************************************/

/******************************* SPPS Server Start **********************************************************************/

/******************************* SPPS Server End **********************************************************************/
