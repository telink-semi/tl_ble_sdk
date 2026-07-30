/********************************************************************************************************
 * @file    svc_adv.c
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
#include "common/utility.h"


#include "svc_adv.h"
#include <string.h>

#include "stack/ble/ble_format.h"

#include "stack/ble/service/uuid.h"

u16 blc_adv_buildAdvData(blc_adv_ltv_t **ltvs, u32 numLtvs, u8 *advBuf)
{
    u8 *ptr = advBuf;

    for (u8 i = 0; i < numLtvs; i++) {
        U8_TO_STREAM(ptr, ltvs[i]->len);
        U8_TO_STREAM(ptr, ltvs[i]->type);
        memcpy(ptr, ltvs[i]->value, ltvs[i]->len - 1);
        ptr += ltvs[i]->len - 1;
    }

    return ptr - advBuf;
}

u8 *blc_adv_getAdvTypeInformation(u8 *advData, u16 len, u8 advType, u8 *outLen)
{
    u8 *p = advData;

    while (len) {
        if (p[1] == advType) {
            if (outLen) {
                *outLen = p[0] - 1;
            }
            return p + 2;
        }

        if (len > (p[0] + 1)) {
            len -= (p[0] + 1);
            p += (p[0] + 1);
        } else {
            len = 0;
        }
    }
    if (outLen) {
        *outLen = 0;
    }
    return NULL;
}

u8 *blc_adv_getAdvTypeInformationWithCmpValue(u8 *advData, u16 len, u8 advType, u8 *cmpValue, u8 cmpValueLen, u8 *outLen)
{
    u8 *p = advData;

    while (len) {
        if (p[1] == advType) {
            if (memcmp(p + 2, cmpValue, cmpValueLen) == 0) {
                if (outLen) {
                    *outLen = p[0] - cmpValueLen - 1;
                }
                return p + cmpValueLen + 2;
            }
        }

        if (len > (p[0] + 1)) {
            len -= (p[0] + 1);
            p += (p[0] + 1);
        } else {
            len = 0;
        }
    }
    if (outLen) {
        *outLen = 0;
    }
    return NULL;
}

u8 *blc_adv_getCompleteNameInformation(u8 *advData, u16 len, u8 *outLen)
{
    return blc_adv_getAdvTypeInformation(advData, len, DT_COMPLETE_LOCAL_NAME, outLen);
}

u8 *blc_adv_getBroadcastNameInformation(u8 *advData, u16 len, u8 *outLen)
{
    return blc_adv_getAdvTypeInformation(advData, len, DT_BROADCAST_NAME, outLen);
}

u8 *blc_adv_getManufacturerDataInformation(u8 *advData, u16 len, u8 *outLen)
{
    return blc_adv_getAdvTypeInformation(advData, len, DT_MANUFACTURER_SPECIFIC_DATA, outLen);
}

u8 *blc_adv_get16BitServiceDataInformation(u8 *advData, u16 len, u16 serviceUuid, u8 *outLen)
{
    return blc_adv_getAdvTypeInformationWithCmpValue(advData, len, DT_SERVICE_DATA_16BIT_UUID, (u8 *)&serviceUuid, 2, outLen);
}

bool blc_advGetBroadcastID(u8 *advData, u16 len, u8 broadcastID[3])
{
    u8 broadcastIdLen = 0;

    u8 *buff = blc_adv_get16BitServiceDataInformation(advData, len, SERVICE_UUID_BROADCAST_AUDIO_ANNOUNCEMENT, &broadcastIdLen);

    if (buff && broadcastIdLen == 3) {
        memcpy(broadcastID, buff, 3);
        return true;
    }
    return false;
}

u8 *blc_adv_getManufacturerDataInformationByCompanyId(u8 *advData, u16 len, u16 companyId, u8 *outLen)
{
    return blc_adv_getAdvTypeInformationWithCmpValue(advData, len, DT_MANUFACTURER_SPECIFIC_DATA, (u8 *)&companyId, 2, outLen);
}

bool blc_adv_getCsipRSI(u8 *advData, u16 len, u8 rsi[6])
{
    u8 rsiLen = 0;

    u8 *buff = blc_adv_getAdvTypeInformation(advData, len, DT_CSIP_RSI, &rsiLen);

    if (buff && rsiLen == 6) {
        memcpy(rsi, buff, 6);
        return true;
    }
    return false;
}

static bool blt_get16BitServiceUuid(u8 *advData, u16 len, u16 uuid, data_type_t type)
{
    u8   uuidLen = 0;
    u16 *pUuid   = (u16 *)blc_adv_getAdvTypeInformation(advData, len, type, &uuidLen);

    for (int i = 0; i < (uuidLen >> 1); i++) {
        //tlkapi_printf(1, "pUuid is %x, uuid is %x", *pUuid, uuid);
        if (*pUuid == uuid) {
            return true;
        }
        pUuid++;
    }
    return false;
}

bool blc_adv_get16BitServiceUuid(u8 *advData, u16 len, u16 uuid)
{
    return blt_get16BitServiceUuid(advData, len, uuid, DT_COMPLETE_LIST_16BIT_SERVICE_UUID);
}

bool blc_adv_get16BitIncompleteServiceUuid(u8 *advData, u16 len, u16 uuid)
{
    return blt_get16BitServiceUuid(advData, len, uuid, DT_INCOMPLETE_LIST_16BIT_SERVICE_UUID);
}
