/********************************************************************************************************
 * @file    otas_client_buf.h
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

#define OTA_DATA_MAX_LENGTH    32

struct blc_otas_client{
    gattc_sub_ccc_msg_t ntfInput;

    u16 otaDataHdl;
    u16 otaDataCccHdl;

    u16 otaDataReadResultLength;
    u8 otaDataReadResult[OTA_DATA_MAX_LENGTH];
    u8 reserved[2];
}__attribute__((packed));


struct blc_otas_client_ctrl{
    blc_prf_proc_t process;
    struct blc_otas_client pOtasClient[STACK_PRF_ACL_CONN_MAX_NUM];
}__attribute__((packed));

