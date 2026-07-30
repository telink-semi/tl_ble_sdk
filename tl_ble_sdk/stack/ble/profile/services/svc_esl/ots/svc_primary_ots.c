/********************************************************************************************************
 * @file    svc_primary_ots.c
 *
 * @brief   This is the source file for BLE SDK
 *
 * @author  BLE GROUP
 * @date    04,2024
 *
 * @par     Copyright (c) 2024, Telink Semiconductor (Shanghai) Co., Ltd. ("TELINK")
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

#define PRIMARY_OTS_START_HDL SERVICE_PRIMARY_OTS_HDL

#define OTS_OBJECT_NAME_SIZE  64

const u16 primaryOtsObjectNameMaxSize = OTS_OBJECT_NAME_SIZE;

_attribute_ble_data_retention_ u8 primaryOtsFeatureValue[sizeof(u32) * 2];
static const u16                  primaryOtsFeatureValueLen = sizeof(primaryOtsFeatureValue);

static const atts_attribute_t primaryOtsList[] =
    {
        ATTS_PRIMARY_SERVICE(serviceObjectTransferUuid),

        ATTS_CHAR_UUID_ENCR_READ_POINT_NOCB(charPropRead, characteristicOtsFeatureUuid, primaryOtsFeatureValue),

// Write Object Name is mandatory if OACP Create procedure supported, excluded otherwise
#if OTS_CREATE_OBJECT_SUPPORTED
        ATTS_CHAR_UUID_ENCR_RDWR_NULL(charPropReadWrite, characteristicObjectNameUuid),
#else
        ATTS_CHAR_UUID_ENCR_READ_NULL(charPropRead, characteristicObjectNameUuid),
#endif

        ATTS_CHAR_UUID_ENCR_READ_NULL(charPropRead, characteristicObjectTypeUuid),

        ATTS_CHAR_UUID_ENCR_READ_NULL(charPropRead, characteristicObjectSizeUuid),

#if OTS_OBJECT_FIRST_CREATED_SUPPORTED
    #if OTS_CREATE_OBJECT_SUPPORTED
        ATTS_CHAR_UUID_ENCR_RDWR_NULL(charPropReadWrite, characteristicObjectFirstCreatedUuid),
    #else
        ATTS_CHAR_UUID_ENCR_READ_NULL(charPropRead, characteristicObjectFirstCreatedUuid),
    #endif
#endif

#if OTS_OBJECT_LAST_MODIFIED_SUPPORTED
    #if OTS_SERVER_RTC_SUPPORTED
        ATTS_CHAR_UUID_ENCR_READ_NULL(charPropRead, characteristicObjectLastModifiedUuid),
    #else
        ATTS_CHAR_UUID_ENCR_RDWR_NULL(charPropReadWrite, characteristicObjectLastModifiedUuid),
    #endif
#endif

#if OTS_MULTIPLE_OBJECTS_SUPPORTED
        ATTS_CHAR_UUID_ENCR_READ_NULL(charPropRead, characteristicObjectIdUuid),
#endif

#if OTS_OBJECT_PROPERTIES_WRITE_SUPPORTED
        ATTS_CHAR_UUID_ENCR_RDWR_NULL(charPropReadWrite, characteristicObjectPropertiesUuid),
#else
        ATTS_CHAR_UUID_ENCR_READ_NULL(charPropRead, characteristicObjectPropertiesUuid),
#endif

        ATTS_CHAR_UUID_ENCR_WRITE_NULL(charPropWriteIndicate, characteristicObjectActionControlPointUuid),
        ATTS_COMMON_CCC_DEFINE_CB,

#if (OTS_MULTIPLE_OBJECTS_SUPPORTED)
        ATTS_CHAR_UUID_ENCR_WRITE_NULL(charPropWriteIndicate, characteristicObjectListControlPointUuid),
        ATTS_COMMON_CCC_DEFINE_CB,
#endif

#if (OTS_MULTIPLE_OBJECTS_SUPPORTED && OTS_OBJECT_LIST_FILTER_SUPPORTED)
        ATTS_CHAR_UUID_ENCR_RDWR_NULL(charPropReadWrite, characteristicObjectListFilterUuid),
        ATTS_CHAR_UUID_ENCR_RDWR_NULL(charPropReadWrite, characteristicObjectListFilterUuid),
        ATTS_CHAR_UUID_ENCR_RDWR_NULL(charPropReadWrite, characteristicObjectListFilterUuid),
#endif

#if OTS_OBJECT_CHANGED_SUPPORTED
        ATTS_CHAR_UUID_INDIC_ONLY(characteristicObjectChangedUuid),
        ATTS_COMMON_CCC_DEFINE_CB,
#endif
};

/* Primary OTS group structure */
_attribute_ble_data_retention_ static atts_group_t svcPrimaryOtsGroup =
    {
        NULL,
        primaryOtsList,
        NULL,
        NULL,
        PRIMARY_OTS_START_HDL,
        0,
};

void blc_svc_addPrimaryOtsGroup(void)
{
    svcPrimaryOtsGroup.endHandle = svcPrimaryOtsGroup.startHandle + ARRAY_SIZE(primaryOtsList) - 1;
    blc_gatts_addAttributeServiceGroup(&svcPrimaryOtsGroup);
}

void blc_svc_removePrimaryOtsGroup(void)
{
    blc_gatts_removeAttributeServiceGroup(PRIMARY_OTS_START_HDL);
}

void blc_svc_primaryOtsCbackRegister(atts_r_cb_t readCback, atts_w_cb_t writeCback)
{
    svcPrimaryOtsGroup.readCback  = readCback;
    svcPrimaryOtsGroup.writeCback = writeCback;
}
