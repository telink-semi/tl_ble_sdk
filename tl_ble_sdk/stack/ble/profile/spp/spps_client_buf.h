/********************************************************************************************************
 * @file    spps_client_buf.h
 *
 * @brief   This is the header file for BLE SDK
 *
 * @author  BLE GROUP
 * @date    03,2025
 *
 * @par     Copyright (c) 2025, Telink Semiconductor (Shanghai) Co., Ltd. ("TELINK")
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

typedef struct
{
    gattc_sub_ccc_msg_t ntfInput;
    u16 sppDataHdl;
    u16 sppNotifyHdl;
    u16 sppDataCccHdl;
}blc_spps_client_t ;

typedef struct
{
    blc_prf_proc_t process;
    blc_spps_client_t pSppsClient[STACK_PRF_ACL_CONN_MAX_NUM];
} blc_spps_client_ctrl_t;

