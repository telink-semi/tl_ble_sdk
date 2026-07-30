/********************************************************************************************************
 * @file    dis.h
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

// DIS: Device Information Service
// DISC: Device Information Service Client.
// DISS: Device Information Service

/******************************* DIS Common Start **********************************************************************/

/******************************* DIS Common End **********************************************************************/

/******************************* DIS Client Start **********************************************************************/
//DIS Client Event ID
enum
{
    BASIC_EVT_DISC_START = BASIC_EVT_TYPE_DIS_CLIENT,
};

struct blc_disc_regParam
{
};

/**
 * @brief       for user to register Device Information service control client module.
 * @param[in]   param - currently not used, fixed NULL.
 * @return      none.
 */
void blc_basic_registerDISControlClient(const struct blc_disc_regParam *param);

//DIS Client Read Characteristic Value Operation API
int blc_disc_readManufacturerName(u16 connHandle, prf_read_cb_t readCb);
int blc_disc_readModelNumber(u16 connHandle, prf_read_cb_t readCb);
int blc_disc_readSerialNumber(u16 connHandle, prf_read_cb_t readCb);
int blc_disc_readHardwareRevision(u16 connHandle, prf_read_cb_t readCb);
int blc_disc_readFirmwareRevision(u16 connHandle, prf_read_cb_t readCb);
int blc_disc_readSoftwareRevision(u16 connHandle, prf_read_cb_t readCb);
int blc_disc_readSystemId(u16 connHandle, prf_read_cb_t readCb);
int blc_disc_readIEEEDataList(u16 connHandle, prf_read_cb_t readCb);
int blc_disc_readPnPID(u16 connHandle, prf_read_cb_t readCb);
int blc_disc_readUdiForMedicalDevices(u16 connHandle, prf_read_cb_t readCb);

//DIS Client Get Characteristic Value Operation API
int blc_disc_get(u16 connHandle, u8 *batteryLevel);
int blc_disc_getManufacturerName(u16 connHandle, u8 *manufacturerName, u16 *manufacturerNameLen);
int blc_disc_getModelNumber(u16 connHandle, u8 *modelNumber, u16 *modelNumberLen);
int blc_disc_getSerialNumber(u16 connHandle, u8 *serialNumber, u16 *serialNumberLen);
int blc_disc_getHardwareRevision(u16 connHandle, u8 *hardwareRevision, u16 *hardwareRevisionLen);
int blc_disc_getFirmwareRevision(u16 connHandle, u8 *firmwareRevision, u16 *firmwareRevisionLen);
int blc_disc_getSoftwareRevision(u16 connHandle, u8 *softwareRevision, u16 *softwareRevisionLen);
int blc_disc_getSystemId(u16 connHandle, dis_system_id_t *systemId);
int blc_disc_getPnPID(u16 connHandle, dis_pnp_t *PnPID);

int blc_disc_getIEEEDataList(u16 connHandle, u8 *IEEEDataList, u16 *IEEEDataListLen);
int blc_disc_getUdiForMedicalDevices(u16 connHandle, u8 *udiForMedicalDevices, u16 *udiForMedicalDevicesLen);

/******************************* DIS Client End **********************************************************************/


/******************************* DIS Server Start **********************************************************************/
//DIS Server Event ID
enum
{
    BASIC_EVT_DISS_START = BASIC_EVT_TYPE_DIS_SERVER,
};

struct blc_diss_regParam
{
};

/**
 * @brief       for user to register Device Information service control server module.
 * @param[in]   param - currently not used, fixed NULL.
 * @return      none.
 * note: DIS service all characteristic properties is read only, attribute value initial static const value,
 * if you want to modify the value, directly modify the svc_dis.c file.
 */
void blc_basic_registerDISControlServer(const struct blc_diss_regParam *param);
/******************************* DIS Server End **********************************************************************/
