/********************************************************************************************************
 * @file    svc_adv.h
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

typedef struct
{
    u8 len;
    u8 type;
    u8 value[0];
} blc_adv_ltv_t;

u16 blc_adv_buildAdvData(blc_adv_ltv_t **ltvs, u32 numLtvs, u8 *advBuf);

u8 *blc_adv_getAdvTypeInformation(u8 *advData, u16 len, u8 advType, u8 *outLen);

u8 *blc_adv_getCompleteNameInformation(u8 *advData, u16 len, u8 *outLen);

u8 *blc_adv_getBroadcastNameInformation(u8 *advData, u16 len, u8 *outLen);

u8 *blc_adv_get16BitServiceDataInformation(u8 *advData, u16 len, u16 serviceUuid, u8 *outLen);

bool blc_advGetBroadcastID(u8 *advData, u16 len, u8 broadcastID[3]);

bool blc_adv_getCsipRSI(u8 *advData, u16 len, u8 rsi[6]);

u8 *blc_adv_getManufacturerDataInformationByCompanyId(u8 *advData, u16 len, u16 companyId, u8 *outLen);

bool blc_adv_get16BitServiceUuid(u8 *advData, u16 len, u16 uuid);

bool blc_adv_get16BitIncompleteServiceUuid(u8 *advData, u16 len, u16 uuid);
