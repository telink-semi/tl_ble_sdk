/********************************************************************************************************
 * @file    svc_core.c
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

#define GAP_START_HDL SERVICE_GENERIC_ACCESS_HDL

_attribute_ble_data_retention_ static u8 defaultDevName[32] = DEFAULT_DEV_NAME;
static u16                               defaultDevNameLen  = MAX_DEV_NAME_LEN;

static const u16 defaultAppearance    = DEFAULT_DEV_APPEARE;
static const u16 defaultAppearanceLen = sizeof(defaultAppearance);
#if (TELINK_CS_TEST_WITH_PHONE)
static const u16 defaultPeriConnParameters[]  = {48, 48, 0, 100}; //gap_periConnectParams_t
#else
static const u16 defaultPeriConnParameters[]  = {20, 40, 0, 100}; //gap_periConnectParams_t
#endif
static const u16 defaultPeriConnParametersLen = sizeof(defaultPeriConnParameters);

/*
 * @brief the structure for default GAP service List.
 */
static const atts_attribute_t gapList[] =
    {
        ATTS_PRIMARY_SERVICE(serviceGenericAccessUuid),

        //device name
        ATTS_CHAR_UUID_READ_POINT_NOCB(charPropRead, characteristicDeviceNameUuid, defaultDevName),

        //Appearance
        ATTS_CHAR_UUID_READ_ENTITY_NOCB(charPropRead, characteristicAppearanceUuid, defaultAppearance),

        //period connect parameter
        ATTS_CHAR_UUID_READ_ENTITY_NOCB(charPropRead, characteristicPeripheralPreferredConnParamUuid, defaultPeriConnParameters),
};


/*
 * @brief the structure for default GAP service group.
 */
_attribute_ble_data_retention_ static atts_group_t svcGapGroup =
    {
        NULL,
        gapList,
        NULL,
        NULL,
        GAP_START_HDL,
        0};

#define GATT_START_HDL SERVICE_GENERIC_ATTRIBUTE_HDL

#ifndef SERVER_SUPPORT_FEATURES
    #if EATT_SUPPORTED_FLAG
        #define GATT_SERVER_EATT_SUPPORTED SERVER_SUPP_FEAT_EATT_BEARER
    #else
        #define GATT_SERVER_EATT_SUPPORTED SERVER_NOT_SUPP_FEAT_EATT_BEARER
    #endif

    #define SERVER_SUPPORT_FEATURES GATT_SERVER_EATT_SUPPORTED
#endif

_attribute_ble_data_retention_
    u16          gattServiceChangeVal[2] = {0x0000, 0x0000};
static const u16 gattServiceChangeValLen = sizeof(gattServiceChangeVal);

static const u8  serverSuppFeatVal[1] = {SERVER_SUPPORT_FEATURES}; //EATT not support
static const u16 serverSuppFeatValLen = sizeof(serverSuppFeatVal);

static const u8  clientSuppFeatVal    = 0;
static const u16 clientSuppFeatValLen = sizeof(clientSuppFeatVal);

_attribute_ble_data_retention_
    u8           databaseHashVal[16]; //database hash calculation
static const u16 databaseHashValLen = 0x10;

/*
 * @brief the structure for default GATT service List.
 */
static const atts_attribute_t gattList[] =
    {
        ATTS_PRIMARY_SERVICE(serviceGenericAttributeUuid),

        //service change
        ATTS_CHAR_UUID_NO_RDWR_POINT_NOCB(charPropIndicate, characteristicServiceChangedUuid, gattServiceChangeVal),
        ATTS_COMMON_CCC_DEFINE,

        //serverSuppFeat
        ATTS_CHAR_UUID_READ_POINT_NOCB(charPropRead, characteristicServerSupportedFeaturesUuid, serverSuppFeatVal),

        //clientSuppFeat
        ATTS_CHAR_UUID_RDWR_ENTITY_RWCB(charPropReadWriteWriteWithout, characteristicClientSupportedFeaturesUuid, clientSuppFeatVal),

        //database Hash
        ATTS_CHAR_UUID_READ_ENTITY_NOCB(charPropRead, characteristicDatabaseHashUuid, databaseHashVal),
};

/*
 * @brief the structure for default GATT service group.
 */
_attribute_ble_data_retention_ static atts_group_t svcGattGroup =
    {
        NULL,
        gattList,
        NULL,
        NULL,
        GATT_START_HDL,
        0,
};

/**
 * @brief      for user add default GATT and GAP service in all GAP server.
 * @param[in]  none.
 * @return     none.
 */
void blc_svc_addCoreGroup(void)
{
    svcGapGroup.endHandle  = svcGapGroup.startHandle + ARRAY_SIZE(gapList) - 1;
    svcGattGroup.endHandle = svcGattGroup.startHandle + ARRAY_SIZE(gattList) - 1;
    blc_gatts_addAttributeServiceGroup(&svcGapGroup);
    blc_gatts_addAttributeServiceGroup(&svcGattGroup);
}

/**
 * @brief      for user remove default GATT and GAP service in all GAP server.
 * @param[in]  none.
 * @return     none.
 */
void blc_svc_removeCoreGroup(void)
{
    blc_gatts_removeAttributeServiceGroup(GAP_START_HDL);
    blc_gatts_removeAttributeServiceGroup(GATT_START_HDL);
}

/**
 * @brief      for user calculate database hash value(core version >= 5.1).
 * @param[in]  none.
 * @return     none.
 */
void blc_svc_calculateDatabaseHash(void)
{
    blc_gatts_calculateDatabaseHash(0xFFFF, databaseHashVal);
}

/**
 * @brief     for user set device name.
 * @param[in] name: user device name.
 * @return    none.
 */
void blc_svc_setDeviceName(const char *name)
{
    strncpy((char*)defaultDevName, name, sizeof(defaultDevName));
    defaultDevNameLen = strlen(name);
}

/********************adv Core info**********************/

const blc_adv_flags_t advDefFlags = {
    .ltv.len  = 0x02,
    .ltv.type = DT_FLAGS,
    .flags    = 0x05};

const blc_adv_completeName_t advDefCompleteName = {
    .ltv.len      = sizeof(DEFAULT_DEV_NAME),
    .ltv.type     = DT_COMPLETE_LOCAL_NAME,
    .completeName = DEFAULT_DEV_NAME,
};

const blc_adv_appearance_t advDefAppearance = {
    .ltv.len    = 0x03,
    .ltv.type   = DT_APPEARANCE,
    .appearance = DEFAULT_DEV_APPEARE,
};
