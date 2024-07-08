/********************************************************************************************************
 * @file    ble_host.h
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
#ifndef STACK_BLE_HOST_BLE_HOST_H_
#define STACK_BLE_HOST_BLE_HOST_H_


#include "stack/ble/ble_common.h"
#include "stack/ble/ble_format.h"


/* ATT/GATT old version */
#include "stack/ble/host/att/att_v0/att_v0.h"
#include "stack/ble/host/att/att_v0/gatt_v0.h"

/* ATT/GATT new version */
#include "stack/ble/host/att/att.h"
#include "stack/ble/host/att/att.h"
#include "stack/ble/host/att/atts.h"
#include "stack/ble/host/att/attc.h"
#include "stack/ble/host/att/att_uuid.h"
#include "stack/ble/host/att/att_cmd.h"
#include "stack/ble/host/gatt/gatt.h"
#include "stack/ble/host/gatt/gatts.h"
#include "stack/ble/host/gatt/gattc.h"


#include "stack/ble/host/smp/smp.h"
#include "stack/ble/host/smp/smp_storage.h"

#include "stack/ble/host/gap/gap.h"
#include "stack/ble/host/gap/gapc.h"
#include "stack/ble/host/gap/gap_event.h"

#include "stack/ble/host/host_comp.h"

#include "stack/ble/host/l2cap/l2cap.h"
#include "stack/ble/host/signaling/signaling.h"



#endif /* STACK_BLE_HOST_BLE_HOST_H_ */
