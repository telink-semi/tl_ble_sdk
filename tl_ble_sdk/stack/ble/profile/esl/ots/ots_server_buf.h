/********************************************************************************************************
 * @file    ots_server_buf.h
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

#define OLCP_IND_SIZE (7)
#define OACP_IND_SIZE (7)

typedef struct
{
    u16 otsFeatureHdl;
    u16 otsObjectNameHdl;
    u16 otsObjectTypeHdl;
    u16 otsObjectSizeHdl;
    u16 otsObjectFirstCreatedHdl;
    u16 otsObjectLastModifiedHdl;
    u16 otsObjectIdHdl;
    u16 otsObjectPropertiesHdl;
    u16 otsOacpHdl;
    u16 otsOacpCccHdl;
    u16 otsOlcpHdl;
    u16 otsOlcpCccHdl;
    u16 otsObjectListFilterHdl[3];
    u16 otsObjectChangedHdl;
    u16 otsObjectChangedCccHdl;
} blc_otp_server_t;

typedef void (*blt_otss_timer_cb_t)(void *data);

typedef struct
{
    u32                 tick;
    u8                  secRemaining;
    bool                active;
    void               *data;
    blt_otss_timer_cb_t cb;
} blt_otss_timer_t;

typedef struct
{
    void            *current_object;
    u8               pendingOacpInd        : 1;
    u8               pendingOacpCfm        : 1;
    u8               pendingOacpTransfer   : 1;
    u8               pendingOlcpCfm        : 1;
    u8               pendingOlcpInd        : 1;
    u8               transferChannelOpened : 1;
    u8               oacpIndLen;
    u8               olcpIndLen;
    u8               olcpIndBuf[OLCP_IND_SIZE];
    u8               oacpIndBuf[OACP_IND_SIZE];
    u8               olcpCccVal[sizeof(u16)];
    u8               oacpCccVal[sizeof(u16)];
    u8               objectChangeCccVal[sizeof(u16)];
    u8               objectIdVal[6];
    u8               objectPropertiesVal[sizeof(u32)];
    u8               objectTypeVal[16];
    u8               objectSizeVal[2 * sizeof(u32)];
    u16              transferChannelScid;
    u16              transferChannelDcid;
    u16              transferChannelMtu;
    blt_otss_timer_t oacpTimer;

    struct
    {
        u8  pendingOacpOpcode;
        u64 id;

        union
        {
            struct
            {
                u32 offset;
                u32 remaining;
            } read_op;

            struct
            {
                u32 offset;
                u32 remaining;
                u8  mode;
            } write_op;
        };
    } pendingTransferOp;
} blc_otp_server_client_t;

typedef struct blc_otp_server_ctrl
{
    blc_prf_proc_t          process;
    blc_otp_server_t        otpServer;
    blc_otp_server_client_t otsServerClients[STACK_PRF_ACL_CONN_MAX_NUM];
} blc_otp_server_ctrl_t;

typedef struct
{
    u32 oacp_features;
    u32 olcp_features;
} blc_otss_regParam_t;

extern const blc_otss_regParam_t defaultOtpsParam;
