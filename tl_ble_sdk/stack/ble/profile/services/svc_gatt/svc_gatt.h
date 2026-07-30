/********************************************************************************************************
 * @file    svc_gatt.h
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

#ifdef __BOOT_SWITCH_APP1__
    //sdk fix service uuid handle
    #define SERVICE_GENERIC_ACCESS_HDL     SERVICE_GATT_START_HANDLE
    #define GAP_MAX_HDL_NUM                7  //0x0F
    #define SERVICE_GENERIC_ATTRIBUTE_HDL  SERVICE_GENERIC_ACCESS_HDL + GAP_MAX_HDL_NUM
    #define GATT_MAX_HDL_NUM               10 //0x10
    #define SERVICE_DEVICE_INFORMATION_HDL SERVICE_GENERIC_ATTRIBUTE_HDL + GATT_MAX_HDL_NUM
    #define DIS_MAX_HDL_NUM                0  //0x10
    #define SERVICE_BATTERY_HDL            SERVICE_DEVICE_INFORMATION_HDL + DIS_MAX_HDL_NUM
    #define BAS_MAX_HDL_NUM                7  //0x10
    #define SERVICE_SCAN_PARAMETERS_HDL    SERVICE_BATTERY_HDL + BAS_MAX_HDL_NUM
    #define SCPS_MAX_HDL_NUM               0x10
#else
    //sdk fix service uuid handle
    #define SERVICE_GENERIC_ACCESS_HDL     SERVICE_GATT_START_HANDLE
    #define GAP_MAX_HDL_NUM                0x0F
    #define SERVICE_GENERIC_ATTRIBUTE_HDL  SERVICE_GENERIC_ACCESS_HDL + GAP_MAX_HDL_NUM
    #define GATT_MAX_HDL_NUM               0x10
    #define SERVICE_DEVICE_INFORMATION_HDL SERVICE_GENERIC_ATTRIBUTE_HDL + GATT_MAX_HDL_NUM
    #define DIS_MAX_HDL_NUM                0x20
    #define SERVICE_BATTERY_HDL            SERVICE_DEVICE_INFORMATION_HDL + DIS_MAX_HDL_NUM
    #define BAS_MAX_HDL_NUM                0x30
    #define SERVICE_SCAN_PARAMETERS_HDL    SERVICE_BATTERY_HDL + BAS_MAX_HDL_NUM
    #define SCPS_MAX_HDL_NUM               0x10
    #define SERVICE_TX_POWER_HDL           SERVICE_SCAN_PARAMETERS_HDL + SCPS_MAX_HDL_NUM
    #define TPS_MAX_HDL_NUM                0x04
#endif

#include "bas/svc_battery.h"
#include "core/svc_core.h"
#include "dis/svc_dis.h"
#include "scps/svc_scps.h"

