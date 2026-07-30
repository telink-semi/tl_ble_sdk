/********************************************************************************************************
 * @file    svc_uuid.c
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
#include "common/bit.h"
#include "common/types.h"
#include "svc_uuid.h"
#include "stack/ble/host/att/att_uuid.h"
#include "stack/ble/host/gatt/gatt.h"

const unsigned short attr16UuidLen       = 2;
const unsigned short attr128UuidLen      = 16;
const unsigned short gattIncludeValueLen = 6;

const unsigned short characteristicPropertiesLen = CHARACTERISTIC_PROPERTIES_LENGTH;

const unsigned char charPropRead                          = CHAR_PROP_READ;
const unsigned char charPropReadWrite                     = CHAR_PROP_READ | CHAR_PROP_WRITE;
const unsigned char charPropReadWriteExtended             = CHAR_PROP_READ | CHAR_PROP_WRITE | CHAR_PROP_EXTENDED;
const unsigned char charPropReadWriteWithout              = CHAR_PROP_READ | CHAR_PROP_WRITE_WITHOUT_RSP;
const unsigned char charPropReadWriteWriteWithout         = CHAR_PROP_READ | CHAR_PROP_WRITE | CHAR_PROP_WRITE_WITHOUT_RSP;
const unsigned char charPropReadWriteWriteWithoutNotify   = CHAR_PROP_READ | CHAR_PROP_WRITE | CHAR_PROP_WRITE_WITHOUT_RSP | CHAR_PROP_NOTIFY;
const unsigned char charPropReadWriteWriteWithoutExtended = CHAR_PROP_READ | CHAR_PROP_WRITE | CHAR_PROP_WRITE_WITHOUT_RSP | CHAR_PROP_EXTENDED;
const unsigned char charPropReadWriteNotify               = CHAR_PROP_READ | CHAR_PROP_WRITE | CHAR_PROP_NOTIFY;
const unsigned char charPropReadWriteIndicate             = CHAR_PROP_READ | CHAR_PROP_WRITE | CHAR_PROP_INDICATE;
const unsigned char charPropReadWriteWithoutNotify        = CHAR_PROP_READ | CHAR_PROP_WRITE_WITHOUT_RSP | CHAR_PROP_NOTIFY;
const unsigned char charPropReadNotfiy                    = CHAR_PROP_READ | CHAR_PROP_NOTIFY;
const unsigned char charPropReadIndicate                  = CHAR_PROP_READ | CHAR_PROP_INDICATE;


const unsigned char charPropWrite                   = CHAR_PROP_WRITE;
const unsigned char charPropWriteWithout            = CHAR_PROP_WRITE_WITHOUT_RSP;
const unsigned char charPropWriteWriteWithout       = CHAR_PROP_WRITE | CHAR_PROP_WRITE_WITHOUT_RSP;
const unsigned char charPropWriteIndicate           = CHAR_PROP_WRITE | CHAR_PROP_INDICATE;
const unsigned char charPropWriteNotifyIndicate     = CHAR_PROP_WRITE | CHAR_PROP_NOTIFY | CHAR_PROP_INDICATE;
const unsigned char charPropWriteWriteWithoutNotify = CHAR_PROP_WRITE | CHAR_PROP_WRITE_WITHOUT_RSP | CHAR_PROP_NOTIFY;
const unsigned char charPropWriteWithoutIndicate    = CHAR_PROP_WRITE_WITHOUT_RSP | CHAR_PROP_INDICATE;

const unsigned char charPropNotify          = CHAR_PROP_NOTIFY;
const unsigned char charPropNotifyBroadcast = CHAR_PROP_BROADCAST | CHAR_PROP_NOTIFY;
const unsigned char charPropIndicate        = CHAR_PROP_INDICATE;
const unsigned char charPropNotifyIndicate  = CHAR_PROP_NOTIFY | CHAR_PROP_INDICATE;

const unsigned char  clientCharacteristicConfiguration[2] = {0x00, 0x00};
const unsigned short clientCharacteristicConfigurationLen = sizeof(clientCharacteristicConfiguration);

const unsigned char  serverCharacteristicConfiguration[2] = {0x00, 0x00};
const unsigned short serverCharacteristicConfigurationLen = sizeof(serverCharacteristicConfiguration);
