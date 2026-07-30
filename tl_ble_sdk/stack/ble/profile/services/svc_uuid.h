/********************************************************************************************************
 * @file    svc_uuid.h
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

#define CHARACTERISTIC_PROPERTIES_LENGTH 1

extern const unsigned short attr16UuidLen;
extern const unsigned short attr128UuidLen;
extern const unsigned short gattIncludeValueLen;


extern const unsigned short characteristicPropertiesLen;

extern const unsigned char charPropRead;
extern const unsigned char charPropReadWrite;
extern const unsigned char charPropReadWriteExtended;
extern const unsigned char charPropReadWriteWithout;
extern const unsigned char charPropReadWriteWriteWithout;
extern const unsigned char charPropReadWriteWriteWithoutExtended;
extern const unsigned char charPropReadWriteWriteWithoutNotify;
extern const unsigned char charPropReadWriteNotify;
extern const unsigned char charPropReadWriteIndicate;
extern const unsigned char charPropReadWriteWithoutNotify;
extern const unsigned char charPropReadNotfiy;
extern const unsigned char charPropReadIndicate;

extern const unsigned char charPropWrite;
extern const unsigned char charPropWriteWithout;
extern const unsigned char charPropWriteWriteWithout;
extern const unsigned char charPropWriteIndicate;
extern const unsigned char charPropWriteNotifyIndicate;
extern const unsigned char charPropWriteWriteWithoutNotify;
extern const unsigned char charPropWriteWithoutIndicate;

extern const unsigned char charPropNotify;
extern const unsigned char charPropIndicate;
extern const unsigned char charPropNotifyIndicate;
extern const unsigned char charPropNotifyBroadcast;

extern const unsigned char  clientCharacteristicConfiguration[2];
extern const unsigned short clientCharacteristicConfigurationLen;

extern const unsigned char  serverCharacteristicConfiguration[2];
extern const unsigned short serverCharacteristicConfigurationLen;

#define ATTS_CHARACTERISTIC_DECLARATIONS(properties) \
    {ATT_PERMISSIONS_READ, ATT_16_UUID_LEN, (u8 *)(size_t)declarationsCharacteristicUuid, (u16 *)(size_t) & characteristicPropertiesLen, CHARACTERISTIC_PROPERTIES_LENGTH, (u8 *)(size_t) & properties, ATTS_SET_ATTR_VALUE_PROPERTIES}

#define SERVICE_CHAR_DEFINE(properties, handle, uuid)             {properties, U16_TO_BYTES(handle), U16_TO_BYTES(uuid)}
#define SERVICE_CHAR_READ(handle, uuid)                           SERVICE_CHAR_DEFINE(CHAR_PROP_READ, handle, uuid)
#define SERVICE_CHAR_READ_WRITE(handle, uuid)                     SERVICE_CHAR_DEFINE(CHAR_PROP_READ | CHAR_PROP_WRITE, handle, uuid)

#define SERVICE_CHAR_WRITE(handle, uuid)                          SERVICE_CHAR_DEFINE(CHAR_PROP_WRITE, handle, uuid)
#define SERVICE_CHAR_WRITE_WRITEWITHOUT(handle, uuid)             SERVICE_CHAR_DEFINE(CHAR_PROP_WRITE_WITHOUT_RSP | CHAR_PROP_WRITE, handle, uuid)
#define SERVICE_CHAR_NOTIFY(handle, uuid)                         SERVICE_CHAR_DEFINE(CHAR_PROP_NOTIFY, handle, uuid)
#define SERVICE_CHAR_READ_NOTIFY(handle, uuid)                    SERVICE_CHAR_DEFINE(CHAR_PROP_READ | CHAR_PROP_NOTIFY, handle, uuid)
#define SERVICE_CHAR_INDICATE(handle, uuid)                       SERVICE_CHAR_DEFINE(CHAR_PROP_INDICATE, handle, uuid)
#define SERVICE_CHAR_WRITE_INDICATE(handle, uuid)                 SERVICE_CHAR_DEFINE(CHAR_PROP_WRITE | CHAR_PROP_INDICATE, handle, uuid)
#define SERVICE_CHAR_WRITE_INDICATE_NOTIFY(handle, uuid)          SERVICE_CHAR_DEFINE(CHAR_PROP_WRITE | CHAR_PROP_NOTIFY | CHAR_PROP_INDICATE, handle, uuid)

#define SERVICE_CHAR_READ_WRITE_WRITEWITHOUT(handle, uuid)        SERVICE_CHAR_DEFINE(CHAR_PROP_READ | CHAR_PROP_WRITE | CHAR_PROP_WRITE_WITHOUT_RSP, handle, uuid)
#define SERVICE_CHAR_WRITE_NOTIFY_INDICATE(handle, uuid)          SERVICE_CHAR_DEFINE(CHAR_PROP_NOTIFY | CHAR_PROP_INDICATE, handle, uuid)
#define SERVICE_CHAR_READ_WRITE_NOTIFY(handle, uuid)              SERVICE_CHAR_DEFINE(CHAR_PROP_READ | CHAR_PROP_WRITE | CHAR_PROP_NOTIFY, handle, uuid)
#define SERVICE_CHAR_READ_WRITEWITHOUT_NOTIFY(handle, uuid)       SERVICE_CHAR_DEFINE(CHAR_PROP_READ | CHAR_PROP_WRITE_WITHOUT_RSP | CHAR_PROP_NOTIFY, handle, uuid)
#define SERVICE_CHAR_WRITE_WRITEWITHOUT_NOTIFY(handle, uuid)      SERVICE_CHAR_DEFINE(CHAR_PROP_WRITE_WITHOUT_RSP | CHAR_PROP_WRITE | CHAR_PROP_NOTIFY, handle, uuid)
#define SERVICE_CHAR_READ_WRITE_WRITEWITHOUT_NOTIFY(handle, uuid) SERVICE_CHAR_DEFINE(CHAR_PROP_READ | CHAR_PROP_WRITE_WITHOUT_RSP | CHAR_PROP_WRITE | CHAR_PROP_NOTIFY, handle, uuid)

//{ATT_PERMISSIONS_READ, ATT_16_UUID_LEN, (u8*)declarationsPrimaryServiceUuid,(u16*)&attr16UuidLen,ATT_16_UUID_LEN,(u8*)serviceUuid,0}
#define ATTS_PRIMARY_SERVICE(serviceUuid) {ATT_PERMISSIONS_READ, ATT_16_UUID_LEN, (u8 *)(size_t)declarationsPrimaryServiceUuid, (u16 *)(size_t) & attr16UuidLen, ATT_16_UUID_LEN, (u8 *)(size_t)serviceUuid, 0}
//{ATT_PERMISSIONS_READ, ATT_16_UUID_LEN, (u8*)declarationsSecondaryServiceUuid,(u16*)&attr16UuidLen,ATT_16_UUID_LEN,(u8*)serviceUuid,0}
#define ATTS_SECONDARY_SERVICE(serviceUuid) {ATT_PERMISSIONS_READ, ATT_16_UUID_LEN, (u8 *)(size_t)declarationsSecondaryServiceUuid, (u16 *)(size_t) & attr16UuidLen, ATT_16_UUID_LEN, (u8 *)(size_t)serviceUuid, 0}

//{ATT_PERMISSIONS_READ, ATT_16_UUID_LEN, (u8*)declarationsPrimaryServiceUuid,(u16*)&attr128UuidLen,ATT_128_UUID_LEN,(u8*)serviceUuid,0}
#define ATTS_PRIMARY_SERVICE_128(serviceUuid) {ATT_PERMISSIONS_READ, ATT_16_UUID_LEN, (u8 *)(size_t)declarationsPrimaryServiceUuid, (u16 *)(size_t) & attr128UuidLen, ATT_128_UUID_LEN, (u8 *)(size_t)serviceUuid, 0}
//{ATT_PERMISSIONS_READ, ATT_16_UUID_LEN, (u8*)declarationsSecondaryServiceUuid,(u16*)&attr128UuidLen,ATT_128_UUID_LEN,(u8*)serviceUuid,0}
#define ATTS_SECONDARY_SERVICE_128(serviceUuid) {ATT_PERMISSIONS_READ, ATT_16_UUID_LEN, (u8 *)(size_t)declarationsSecondaryServiceUuid, (u16 *)(size_t) & attr128UuidLen, ATT_128_UUID_LEN, (u8 *)(size_t)serviceUuid, 0}


//{ATT_PERMISSIONS_RDWR,ATT_16_UUID_LEN,  (u8*) descriptorClientCharacteristicConfigurationUuid,    (u16*) &pacsSinkPACCCCLen,  sizeof(pacsSinkPACCCC), (u8*)pacsSinkPACCCC,0}
#define ATTS_CCC_DEFINE_COMMON(ccc, cccLen) {ATT_PERMISSIONS_RDWR, ATT_16_UUID_LEN, (u8 *)(size_t)descriptorClientCharacteristicConfigurationUuid, (u16 *)(size_t) & cccLen, sizeof(ccc), (u8 *)(size_t)ccc, 0}
#define ATTS_CCC_DEFINE(ccc)                ATTS_CCC_DEFINE_COMMON(ccc, ccc##Len)
#define ATTS_COMMON_CCC_DEFINE              ATTS_CCC_DEFINE(clientCharacteristicConfiguration)
#define ATTS_COMMON_CCC_DEFINE_CB           {ATT_PERMISSIONS_RDWR, ATT_16_UUID_LEN, (u8 *)(size_t)descriptorClientCharacteristicConfigurationUuid, (u16 *)NULL, 0, (u8 *)NULL, ATTS_SET_WRITE_CBACK | ATTS_SET_READ_CBACK}

#define ATTS_SCC_DEFINE_COMMON(scc, sccLen) {ATT_PERMISSIONS_RDWR, ATT_16_UUID_LEN, (u8 *)(size_t)descriptorServerCharacteristicConfigurationUuid, (u16 *)(size_t) & sccLen, sizeof(scc), (u8 *)(size_t)scc, 0}
#define ATTS_SCC_DEFINE(scc)                ATTS_SCC_DEFINE_COMMON(scc, scc##Len)
#define ATTS_COMMON_SCC_DEFINE              ATTS_SCC_DEFINE(serverCharacteristicConfiguration)
#define ATTS_COMMON_SCC_DEFINE_CB           {ATT_PERMISSIONS_RDWR, ATT_16_UUID_LEN, (u8 *)(size_t)descriptorServerCharacteristicConfigurationUuid, (u16 *)NULL, 0, (u8 *)NULL, ATTS_SET_WRITE_CBACK | ATTS_SET_READ_CBACK}

#define ATTS_DESCR_DEFINE_COMMON(perm, uuid, val_ptr, val_len_ptr, val_len_max, settings) \
    {perm, ATT_16_UUID_LEN, (u8 *)(size_t)uuid, (u16 *)(size_t)val_len_ptr, val_len_max, (u8 *)(size_t)val_ptr, settings}

#define ATTS_DESCR_DEFINE_READ(uuid, val_ptr, val_len_ptr, val_len_max, settings)  ATTS_DESCR_DEFINE_COMMON(ATT_PERMISSIONS_READ, uuid, val_ptr, val_len_ptr, val_len_max, settings)
#define ATTS_DESCR_DEFINE_WRITE(uuid, val_ptr, val_len_ptr, val_len_max, settings) ATTS_DESCR_DEFINE_COMMON(ATT_PERMISSIONS_WRITE, uuid, val_ptr, val_len_ptr, val_len_max, settings)
#define ATTS_DESCR_DEFINE_RDWR(uuid, val_ptr, val_len_ptr, val_len_max, settings)  ATTS_DESCR_DEFINE_COMMON(ATT_PERMISSIONS_RDWR, uuid, val_ptr, val_len_ptr, val_len_max, settings)

#define ATTS_DESCR_DEFINE_NOCB_READ(uuid, val_ptr, val_len_ptr, val_len_max)       ATTS_DESCR_DEFINE_READ(uuid, val_ptr, val_len_ptr, val_len_max, 0)
#define ATTS_DESCR_DEFINE_NOCB_WRITE(uuid, val_ptr, val_len_ptr, val_len_max)      ATTS_DESCR_DEFINE_WRITE(uuid, val_ptr, val_len_ptr, val_len_max, 0)
#define ATTS_DESCR_DEFINE_NOCB_RDWR(uuid, val_ptr, val_len_ptr, val_len_max)       ATTS_DESCR_DEFINE_RDWR(uuid, val_ptr, val_len_ptr, val_len_max, 0)

#define ATTS_DESCR_DEFINE_PTR_NOCB_READ(uuid, val)                                 ATTS_DESCR_DEFINE_NOCB_READ(uuid, val, &val##Len, sizeof(val))
#define ATTS_DESCR_DEFINE_PTR_NOCB_WRITE(uuid, val)                                ATTS_DESCR_DEFINE_NOCB_READ(uuid, val, &val##Len, sizeof(val))
#define ATTS_DESCR_DEFINE_PTR_NOCB_RDWR(uuid, val)                                 ATTS_DESCR_DEFINE_NOCB_READ(uuid, val, &val##Len, sizeof(val))

#define ATTS_INCLUDE_DEFINE(value)                                                 {ATT_PERMISSIONS_READ, ATT_16_UUID_LEN, (u8 *)(size_t)declarationsIncludeUuid, (u16 *)(size_t) & gattIncludeValueLen, 6, (u8 *)(size_t)value, 0}

#define ATTS_CHAR_UUID_ENCR_WRITE_NULL(properties, uuid)                                                        \
    ATTS_CHARACTERISTIC_DECLARATIONS(properties),                                                               \
    {                                                                                                           \
        ATT_PERMISSIONS_ENCRYPT_WRITE, ATT_16_UUID_LEN, (u8 *)(size_t)uuid, NULL, 0, NULL, ATTS_SET_WRITE_CBACK \
    }

#define ATTS_CHAR_UUID_ENCR_READ_NULL(properties, uuid)                                                            \
    ATTS_CHARACTERISTIC_DECLARATIONS(properties),                                                                  \
    {                                                                                                              \
        ATT_PERMISSIONS_ENCRYPT_READ, ATT_16_UUID_LEN, (u8 *)(size_t)uuid, (u16 *)NULL, 0, (u8 *)NULL, ATTS_SET_READ_CBACK \
    }

#define ATTS_CHAR_UUID_ENCR_RDWR_NULL(properties, uuid)                                                                                   \
    ATTS_CHARACTERISTIC_DECLARATIONS(properties),                                                                                         \
    {                                                                                                                                     \
        ATT_PERMISSIONS_ENCRYPT_RDWR, ATT_16_UUID_LEN, (u8 *)(size_t)uuid, (u16 *)NULL, 0, (u8 *)NULL, ATTS_SET_WRITE_CBACK | ATTS_SET_READ_CBACK \
    }

#define ATTS_CHAR_UUID_WRITE_NULL(properties, uuid)                                                     \
    ATTS_CHARACTERISTIC_DECLARATIONS(properties),                                                       \
    {                                                                                                   \
        ATT_PERMISSIONS_WRITE, ATT_16_UUID_LEN, (u8 *)(size_t)uuid, NULL, 0, NULL, ATTS_SET_WRITE_CBACK \
    }


#define ATTS_CHAR_UUID_NOTIF_ONLY(uuid)                          \
    ATTS_CHARACTERISTIC_DECLARATIONS(charPropNotify),            \
    {                                                            \
        0, ATT_16_UUID_LEN, (u8 *)(size_t)uuid, NULL, 0, NULL, 0 \
    }

#define ATTS_CHAR_UUID_NOTIF_BROADCAST_ONLY(uuid)                \
    ATTS_CHARACTERISTIC_DECLARATIONS(charPropNotifyBroadcast),   \
    {                                                            \
        0, ATT_16_UUID_LEN, (u8 *)(size_t)uuid, NULL, 0, NULL, 0 \
    }

#define ATTS_CHAR_UUID_INDICATE_ONLY(uuid) ATTS_CHAR_UUID_NOTIF_ONLY(uuid)

#define ATTS_CHAR_UUID_INDIC_ONLY(uuid)                          \
    ATTS_CHARACTERISTIC_DECLARATIONS(charPropIndicate),          \
    {                                                            \
        0, ATT_16_UUID_LEN, (u8 *)(size_t)uuid, NULL, 0, NULL, 0 \
    }

#define ATTS_CHAR_UUID_NOTIF_INDICATE_ONLY(uuid)                 \
    ATTS_CHARACTERISTIC_DECLARATIONS(charPropNotifyIndicate),    \
    {                                                            \
        0, ATT_16_UUID_LEN, (u8 *)(size_t)uuid, NULL, 0, NULL, 0 \
    }

#define ATTS_CHAR_UUID_DEFINE(perm, uuid, valueLen, maxValueLen, value, settings) {perm, ATT_16_UUID_LEN, (u8 *)(size_t)uuid, (u16 *)(size_t) & valueLen, maxValueLen, (u8 *)(size_t)value, settings}


#define ATTS_CHAR_UUID_DEFINE_VALUE_POINTER(properties, perm, uuid, maxValueLen, value, settings) \
    ATTS_CHARACTERISTIC_DECLARATIONS(properties),                                                 \
        ATTS_CHAR_UUID_DEFINE(perm, uuid, value##Len, maxValueLen, value, settings)

#define ATTS_CHAR_UUID_DEFINE_VALUE_ENTITY(properties, perm, uuid, value, settings) \
    ATTS_CHARACTERISTIC_DECLARATIONS(properties),                                   \
        ATTS_CHAR_UUID_DEFINE(perm, uuid, value##Len, sizeof(value), &value, settings)

#define ATTS_CHAR_UUID_ENCR_RDWR_NULL_GL(properties, uuid)                                                                                   \
    ATTS_CHARACTERISTIC_DECLARATIONS(properties),                                                                                         \
    {                                                                                                                                     \
        ATT_PERMISSIONS_ENCRYPT_RDWR, ATT_128_UUID_LEN, (u8 *)uuid, (u16 *)NULL, 0, (u8 *)NULL, ATTS_SET_WRITE_CBACK | ATTS_SET_READ_CBACK \
    }

#define ATTS_CHAR_UUID_ENCR_READ_POINTER(properties, uuid, value, settings)     ATTS_CHAR_UUID_DEFINE_VALUE_POINTER(properties, ATT_PERMISSIONS_ENCRYPT_READ, uuid, 0, value, settings)
#define ATTS_CHAR_UUID_ENCR_WRITE_POINT(properties, uuid, value, settings)      ATTS_CHAR_UUID_DEFINE_VALUE_POINTER(properties, ATT_PERMISSIONS_ENCRYPT_WRITE, uuid, sizeof(value), value, settings)
#define ATTS_CHAR_UUID_ENCR_RDWR_POINT(properties, uuid, value, settings)       ATTS_CHAR_UUID_DEFINE_VALUE_POINTER(properties, ATT_PERMISSIONS_ENCRYPT_RDWR, uuid, sizeof(value), value, settings)
#define ATTS_CHAR_UUID_ENCR_READ_POINT_NOCB(properties, uuid, value)            ATTS_CHAR_UUID_ENCR_READ_POINTER(properties, uuid, value, 0)
#define ATTS_CHAR_UUID_ENCR_WRITE_POINT_NOCB(properties, uuid, value)           ATTS_CHAR_UUID_ENCR_WRITE_POINT(properties, uuid, value, 0)
#define ATTS_CHAR_UUID_ENCR_RDWR_POINT_NOCB(properties, uuid, value)            ATTS_CHAR_UUID_ENCR_RDWR_POINT(properties, uuid, value, 0)
#define ATTS_CHAR_UUID_ENCR_READ_POINT_CB(properties, uuid, value)              ATTS_CHAR_UUID_ENCR_READ_POINTER(properties, uuid, value, ATTS_SET_READ_CBACK)
#define ATTS_CHAR_UUID_ENCR_WRITE_POINT_CB(properties, uuid, value)             ATTS_CHAR_UUID_ENCR_WRITE_POINT(properties, uuid, value, ATTS_SET_WRITE_CBACK)
#define ATTS_CHAR_UUID_ENCR_RDWR_POINT_RCB(properties, uuid, value)             ATTS_CHAR_UUID_ENCR_RDWR_POINT(properties, uuid, value, ATTS_SET_READ_CBACK)
#define ATTS_CHAR_UUID_ENCR_RDWR_POINT_WCB(properties, uuid, value)             ATTS_CHAR_UUID_ENCR_RDWR_POINT(properties, uuid, value, ATTS_SET_WRITE_CBACK)
#define ATTS_CHAR_UUID_ENCR_RDWR_POINT_RWCB(properties, uuid, value)            ATTS_CHAR_UUID_ENCR_RDWR_POINT(properties, uuid, value, ATTS_SET_WRITE_CBACK | ATTS_SET_READ_CBACK)

#define ATTS_CHAR_UUID_ENCR_READ_ENTITY(properties, uuid, value, settings)      ATTS_CHAR_UUID_DEFINE_VALUE_ENTITY(properties, ATT_PERMISSIONS_ENCRYPT_READ, uuid, value, settings)
#define ATTS_CHAR_UUID_ENCR_WRITE_ENTITY(properties, uuid, value, settings)     ATTS_CHAR_UUID_DEFINE_VALUE_ENTITY(properties, ATT_PERMISSIONS_ENCRYPT_WRITE, uuid, value, settings)
#define ATTS_CHAR_UUID_ENCR_RDWR_ENTITY(properties, uuid, value, settings)      ATTS_CHAR_UUID_DEFINE_VALUE_ENTITY(properties, ATT_PERMISSIONS_ENCRYPT_RDWR, uuid, value, settings)
#define ATTS_CHAR_UUID_ENCR_READ_ENTITY_NOCB(properties, uuid, value)           ATTS_CHAR_UUID_ENCR_READ_ENTITY(properties, uuid, value, 0)
#define ATTS_CHAR_UUID_ENCR_WRITE_ENTITY_NOCB(properties, uuid, value)          ATTS_CHAR_UUID_ENCR_WRITE_ENTITY(properties, uuid, value, 0)
#define ATTS_CHAR_UUID_ENCR_RDWR_ENTITY_NOCB(properties, uuid, value)           ATTS_CHAR_UUID_ENCR_RDWR_ENTITY(properties, uuid, value, 0)
#define ATTS_CHAR_UUID_ENCR_READ_ENTITY_CB(properties, uuid, value)             ATTS_CHAR_UUID_ENCR_READ_ENTITY(properties, uuid, value, ATTS_SET_READ_CBACK)
#define ATTS_CHAR_UUID_ENCR_WRITE_ENTITY_CB(properties, uuid, value)            ATTS_CHAR_UUID_ENCR_WRITE_ENTITY(properties, uuid, value, ATTS_SET_WRITE_CBACK)
#define ATTS_CHAR_UUID_ENCR_RDWR_ENTITY_RCB(properties, uuid, value)            ATTS_CHAR_UUID_ENCR_RDWR_ENTITY(properties, uuid, value, ATTS_SET_READ_CBACK)
#define ATTS_CHAR_UUID_ENCR_RDWR_ENTITY_WCB(properties, uuid, value)            ATTS_CHAR_UUID_ENCR_RDWR_ENTITY(properties, uuid, value, ATTS_SET_WRITE_CBACK)
#define ATTS_CHAR_UUID_ENCR_RDWR_ENTITY_RWCB(properties, uuid, value)           ATTS_CHAR_UUID_ENCR_RDWR_ENTITY(properties, uuid, value, ATTS_SET_WRITE_CBACK | ATTS_SET_READ_CBACK)


#define ATTS_CHAR_UUIU_NO_READ_WRITE_POINTER(properties, uuid, value, settings) ATTS_CHAR_UUID_DEFINE_VALUE_POINTER(properties, 0, uuid, sizeof(value), value, settings)
#define ATTS_CHAR_UUID_NO_RDWR_POINT_NOCB(properties, uuid, value)              ATTS_CHAR_UUIU_NO_READ_WRITE_POINTER(properties, uuid, value, 0)
#define ATTS_CHAR_UUID_READ_POINTER(properties, uuid, value, settings)          ATTS_CHAR_UUID_DEFINE_VALUE_POINTER(properties, ATT_PERMISSIONS_READ, uuid, 0, value, settings)
#define ATTS_CHAR_UUID_WRITE_POINT(properties, uuid, value, settings)           ATTS_CHAR_UUID_DEFINE_VALUE_POINTER(properties, ATT_PERMISSIONS_WRITE, uuid, sizeof(value), value, settings)
#define ATTS_CHAR_UUID_RDWR_POINT(properties, uuid, value, settings)            ATTS_CHAR_UUID_DEFINE_VALUE_POINTER(properties, ATT_PERMISSIONS_RDWR, uuid, sizeof(value), value, settings)
#define ATTS_CHAR_UUID_READ_POINT_NOCB(properties, uuid, value)                 ATTS_CHAR_UUID_READ_POINTER(properties, uuid, value, 0)
#define ATTS_CHAR_UUID_WRITE_POINT_NOCB(properties, uuid, value)                ATTS_CHAR_UUID_WRITE_POINT(properties, uuid, value, 0)
#define ATTS_CHAR_UUID_RDWR_POINT_NOCB(properties, uuid, value)                 ATTS_CHAR_UUID_RDWR_POINT(properties, uuid, value, ATTS_SET_ALLOW_WRITE)
#define ATTS_CHAR_UUID_READ_POINT_CB(properties, uuid, value)                   ATTS_CHAR_UUID_READ_POINTER(properties, uuid, value, ATTS_SET_READ_CBACK)
#define ATTS_CHAR_UUID_WRITE_POINT_CB(properties, uuid, value)                  ATTS_CHAR_UUID_WRITE_POINT(properties, uuid, value, ATTS_SET_WRITE_CBACK)
#define ATTS_CHAR_UUID_RDWR_POINT_RCB(properties, uuid, value)                  ATTS_CHAR_UUID_RDWR_POINT(properties, uuid, value, ATTS_SET_READ_CBACK)
#define ATTS_CHAR_UUID_RDWR_POINT_WCB(properties, uuid, value)                  ATTS_CHAR_UUID_RDWR_POINT(properties, uuid, value, ATTS_SET_WRITE_CBACK)
#define ATTS_CHAR_UUID_RDWR_POINT_RWCB(properties, uuid, value)                 ATTS_CHAR_UUID_RDWR_POINT(properties, uuid, value, ATTS_SET_WRITE_CBACK | ATTS_SET_READ_CBACK)

#define ATTS_CHAR_UUID_READ_ENTITY(properties, uuid, value, settings)           ATTS_CHAR_UUID_DEFINE_VALUE_ENTITY(properties, ATT_PERMISSIONS_READ, uuid, value, settings)
#define ATTS_CHAR_UUID_WRITE_ENTITY(properties, uuid, value, settings)          ATTS_CHAR_UUID_DEFINE_VALUE_ENTITY(properties, ATT_PERMISSIONS_WRITE, uuid, value, settings)
#define ATTS_CHAR_UUID_RDWR_ENTITY(properties, uuid, value, settings)           ATTS_CHAR_UUID_DEFINE_VALUE_ENTITY(properties, ATT_PERMISSIONS_RDWR, uuid, value, settings)
#define ATTS_CHAR_UUID_READ_ENTITY_NOCB(properties, uuid, value)                ATTS_CHAR_UUID_READ_ENTITY(properties, uuid, value, 0)
#define ATTS_CHAR_UUID_WRITE_ENTITY_NOCB(properties, uuid, value)               ATTS_CHAR_UUID_WRITE_ENTITY(properties, uuid, value, 0)
#define ATTS_CHAR_UUID_RDWR_ENTITY_NOCB(properties, uuid, value)                ATTS_CHAR_UUID_RDWR_ENTITY(properties, uuid, value, 0)
#define ATTS_CHAR_UUID_READ_ENTITY_CB(properties, uuid, value)                  ATTS_CHAR_UUID_READ_ENTITY(properties, uuid, value, ATTS_SET_READ_CBACK)
#define ATTS_CHAR_UUID_WRITE_ENTITY_CB(properties, uuid, value)                 ATTS_CHAR_UUID_WRITE_ENTITY(properties, uuid, value, ATTS_SET_WRITE_CBACK)
#define ATTS_CHAR_UUID_RDWR_ENTITY_RCB(properties, uuid, value)                 ATTS_CHAR_UUID_RDWR_ENTITY(properties, uuid, value, ATTS_SET_READ_CBACK)
#define ATTS_CHAR_UUID_RDWR_ENTITY_WCB(properties, uuid, value)                 ATTS_CHAR_UUID_RDWR_ENTITY(properties, uuid, value, ATTS_SET_WRITE_CBACK)
#define ATTS_CHAR_UUID_RDWR_ENTITY_RWCB(properties, uuid, value)                ATTS_CHAR_UUID_RDWR_ENTITY(properties, uuid, value, ATTS_SET_WRITE_CBACK | ATTS_SET_READ_CBACK)
