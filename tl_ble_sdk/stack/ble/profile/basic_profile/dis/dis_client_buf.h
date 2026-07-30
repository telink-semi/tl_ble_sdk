/********************************************************************************************************
 * @file    dis_client_buf.h
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

struct blc_dis_client
{
    gattc_sub_ccc_msg_t ntfInput; //For code alignment, not notify attribute handle
    /* Characteristic value handle */
    u16 manufacturerNameHdl;     /* Manufacturer Name String */
    u16 modelNumberHdl;          /* Model Number String */
    u16 serialNumberHdl;         /* Serial Number String */
    u16 hardwareRevisionHdl;     /* Hardware Revision String */
    u16 firmwareRevisionHdl;     /* firmware Revision String */
    u16 softwareRevisionHdl;     /* Software Revision String */
    u16 systemIdHdl;             /* System ID */
    u16 IEEEDataListHdl;         /* IEEE 11073-20601 Regulatory Certification Data List */
    u16 PnPIDHdl;                /* PnP ID */
    u16 udiForMedicalDevicesHdl; /* UDI For Medical Devices */

    u16             manufacturerNameLen;
    char            manufacturerName[30];
    u16             modelNumberLen;
    char            modelNumber[14];
    u16             serialNumberLen;
    char            serialNumber[14];
    u16             hardwareRevisionLen;
    char            hardwareRevision[14];
    u16             firmwareRevisionLen;
    char            firmwareRevision[14];
    u16             softwareRevisionLen;
    char            softwareRevision[14];
    dis_system_id_t systemId;
    dis_pnp_t       PnPID;
    u16             IEEEDataListLen;
    u8              IEEEDataList[11];
    u16             udiForMedicalDevicesLen;
    u8              udiForMedicalDevices[14];

} __attribute__((packed));


struct blc_dis_client_ctrl
{
    blc_prf_proc_t         process;
    struct blc_dis_client *pDisClient[STACK_PRF_ACL_CONN_MAX_NUM];
} __attribute__((packed));

