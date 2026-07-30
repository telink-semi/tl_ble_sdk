/********************************************************************************************************
 * @file    svc.h
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

#define SERVICE_GATT_START_HANDLE          0x0001

#define SERVICE_HID_START_HDL              0x00C0

#define SERVICE_LE_AUDIO_START_HDL         0x0200

#define SERVICE_CHANNEL_SOUNDING_START_HDL 0x0800

#define SERVICE_ELECTRONIC_SHELF_LABEL_HDL 0x0820

#define SERVICE_SPORT_FITNESS_HDL          0x0860

#define SERVICE_BINARY_SENSOR_HDL          0x08A0

#define SERVICE_CURRENT_TIME_HDL           0x08B0

#define SERVICE_DEVICE_TIME_HDL            0x08C0

#define SERVICE_HEALTH_HDL                 0x08D0

#define SERVICE_IMMEDIATE_ALERT_HDL        0x0900

#define SERVICE_IP_SUPPORT_HDL             0x0904

#define SERVICE_LINK_LOSS_HDL              0x0906

#define SERVICE_REFERENCE_TIME_UPDATE_HDL  0x090A

#define SERVICE_POSITIONING_HDL            0x0910

//Telink private Service all 128 uuid
#define SERVICE_TELINK_PRIVATE_START_HDL 0x8000


#include "svc_gatt/svc_gatt.h"
#include "svc_telink/svc_telink.h"
#include "svc_esl/svc_esl.h"

#include "svc_uuid.h"
