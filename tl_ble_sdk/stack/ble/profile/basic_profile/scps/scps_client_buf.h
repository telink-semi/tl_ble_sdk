/********************************************************************************************************
 * @file    scps_client_buf.h
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

struct blc_scps_client
{
    gattc_sub_ccc_msg_t ntfInput; //For code alignment, not notify attribute handle
    /* Characteristic value handle */
    u16 scanIntervalWindowHdl; /* Scan Interval Window */
    u16 scanRefreshHdl;        /* Scan Refresh */

} __attribute__((packed));

struct blc_scps_client_ctrl
{
    blc_prf_proc_t          process;
    struct blc_scps_client *pScpsClient[STACK_PRF_ACL_CENTRAL_MAX_NUM];
} __attribute__((packed));

