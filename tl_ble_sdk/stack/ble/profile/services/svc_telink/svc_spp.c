/********************************************************************************************************
 * @file    svc_spp.c
 *
 * @brief   This is the source file for BLE SDK
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
#include "stack/ble/ble.h"
#include "svc_spp.h"

#define SPP_START_HDL SERVICE_TELINK_SPP_HDL

#define SPP_CCC_PROPERTIES_DISABLE      0x00
#define SPP_CCC_NOTIFY_MASK             0x01
#define SPP_CCC_INDICATE_MASK           0x02
#define SPP_CCC_INDICATE_NOTIFY_MASK    0x03

#define SPPS_LOG_EN                     1

static const u8 serviceSppUuid[16] = {TELINK_SPP_UUID_SERVICE};

static const u8 sppInUuid[16] = {TELINK_SPP_DATA_CLIENT2SERVER};
_attribute_ble_data_retention_ u8  sppInData[256];
_attribute_ble_data_retention_ u16 sppInDataLen = 0;

static const u8 sppOutUuid[16] = {TELINK_SPP_DATA_SERVER2CLIENT};
_attribute_ble_data_retention_ u16 sppOutProperties = SPP_CCC_PROPERTIES_DISABLE;

static const atts_attribute_t sppList[] =
{
        ATTS_PRIMARY_SERVICE_128(serviceSppUuid),

        ATTS_CHARACTERISTIC_DECLARATIONS(charPropReadWriteWriteWithout),
        {ATT_PERMISSIONS_RDWR, ATT_128_UUID_LEN, (u8 *)(size_t)&sppInUuid[0], (u16 *)(size_t)&sppInData, sizeof(sppInData), (u8 *)(size_t)&sppInData, ATTS_SET_WRITE_CBACK},

        ATTS_CHARACTERISTIC_DECLARATIONS(charPropNotifyIndicate),
        {0, ATT_128_UUID_LEN, (u8 *)(size_t)sppOutUuid, NULL, 0, NULL, 0 },
        {ATT_PERMISSIONS_WRITE, ATT_16_UUID_LEN, (u8 *)(size_t)descriptorClientCharacteristicConfigurationUuid, (u16 *)(size_t) & clientCharacteristicConfigurationLen, 0, (u8 *)NULL, ATTS_SET_WRITE_CBACK},
};

ble_sts_t blc_spp_pushHandleValue(u16 connHandle, u8 *p, int len){
    if (sppOutProperties & SPP_CCC_NOTIFY_MASK) {
        return blc_gatt_pushHandleValueNotify(connHandle, SPP_START_HDL + 4, p, len);
    } else if (sppOutProperties & SPP_CCC_INDICATE_MASK) {
        return blc_gatt_pushHandleValueIndicate(connHandle, SPP_START_HDL + 4, p, len);
    } else {
        tlkapi_printf(SPPS_LOG_EN, "Notify and indicate disabled!");
        return PRF_ERR_INVALID_PARAMETER;
    }
}

int telinkSppWrite(u16 connHandle, u8 opcode, u16 attrHandle, u8 *writeValue, u16 valueLen)
{
    (void)opcode;
    if (attrHandle == SPP_START_HDL + 2) {
        // sppInData
        sppInDataLen = valueLen > sizeof(sppInData) ? sizeof(sppInData) : valueLen;
        memcpy(sppInData, writeValue, sppInDataLen);
        tlkapi_send_string_data(SPPS_LOG_EN, "[PRF][SPPS]Receive from client", writeValue, valueLen);
        // send back
        blc_spp_pushHandleValue(connHandle, writeValue, valueLen);
    } else if (attrHandle == SPP_START_HDL + 5) {
        // sppOut CCC
        sppOutProperties = (*(u16 *) writeValue) & SPP_CCC_INDICATE_NOTIFY_MASK;
        if (sppOutProperties == SPP_CCC_PROPERTIES_DISABLE) {
            tlkapi_printf(SPPS_LOG_EN, "Client disabled notify and indicate\r\n");
        } else if (sppOutProperties & SPP_CCC_NOTIFY_MASK) {
            tlkapi_printf(SPPS_LOG_EN, "Client enabled notify\r\n");
        } else if (sppOutProperties & SPP_CCC_INDICATE_MASK) {
            tlkapi_printf(SPPS_LOG_EN, "Client enabled indicate\r\n");
        }
    }

    return 0;
}

/* GAP group structure */
_attribute_ble_data_retention_ static atts_group_t svcSppGroup =
    {
        NULL,
        sppList,
        NULL,
        NULL,
        SPP_START_HDL,
        0};

void blc_svc_addSppGroup(void)
{
    svcSppGroup.endHandle  = svcSppGroup.startHandle + ARRAY_SIZE(sppList) - 1;
    svcSppGroup.writeCback = telinkSppWrite;
    blc_gatts_addAttributeServiceGroup(&svcSppGroup);
}

void blc_svc_removeSppGroup(void)
{
    blc_gatts_removeAttributeServiceGroup(SPP_START_HDL);
}
