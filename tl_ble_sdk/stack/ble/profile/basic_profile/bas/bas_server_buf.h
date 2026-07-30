/********************************************************************************************************
 * @file    bas_server_buf.h
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

struct blc_bas_server
{
    u16 batteryLevelHdl;      //Battery level attribute handle
    u16 batteryPowerStateHdl; //Battery power state attribute handle
};


struct blc_bas_server_ctrl
{
    blc_prf_proc_t        process;
    struct blc_bas_server basServer;
} __attribute__((packed));

