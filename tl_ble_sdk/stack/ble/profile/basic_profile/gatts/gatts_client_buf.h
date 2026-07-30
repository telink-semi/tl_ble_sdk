/********************************************************************************************************
 * @file    gatts_client_buf.h
 *
 * @brief   This is the header file for BLE SDK
 *
 * @author  BLE GROUP
 * @date    04,2024
 *
 * @par     Copyright (c) 2024, Telink Semiconductor (Shanghai) Co., Ltd. ("TELINK")
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

typedef struct __attribute__((packed))  {
    gattc_sub_ccc_msg_t ntfInput;   //For code alignment, not notify attribute handle
    /* Characteristic value handle */
    u16 serviceChangedHdl;                  /* Service Changed */
    u16 serviceChangedCCCHdl;               /* Service Changed CCC */
    u16 clientSupportedFeaturesHdl;         /* Client Supported Features */
    u16 databaseHashHdl;                    /* Database Hash */

    u8 clientSupportedFeaturesVal[1];
    u16 clientSupportedFeaturesValLen;

    u8 databaseHash[16];
    u16 databaseHashLen;

    gattc_read_cfg_t databaseHashReadCfg;
    bool databaseHashReadInProgress;
    prf_read_cb_t databaseHashReadCb;
} blc_gatts_client_t;


typedef struct __attribute__((packed))  blc_gatts_client_ctrl{
    blc_prf_proc_t process;

    blc_gatts_client_t* pGattsClient[STACK_PRF_ACL_CONN_MAX_NUM];
} blc_gatts_client_ctrl_t;

typedef struct __attribute__((packed)) {

} blc_gattsc_regParam_t;

/**
 * @brief      Get the client buffer for a given index in the GATT server client structure.
 * @param[in]  index - The index of the client buffer to retrieve.
 * @return     blc_gatts_client_t* - Pointer to the client buffer at the specified index, or NULL if the index is invalid.
 */
blc_gatts_client_t* blc_gattsc_getClientBuf(u8 index);
