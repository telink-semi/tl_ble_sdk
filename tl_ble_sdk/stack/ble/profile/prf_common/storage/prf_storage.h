/********************************************************************************************************
 * @file    prf_storage.h
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


/* for LE Audio SDP: bonding audio GATT service information area (reuse FLASH_SDP_ATT_ADDRESS_1M_FLASH) */
#ifndef FLASH_AUD_ATT_ADDRESS_1M_FLASH
    #define FLASH_AUD_ATT_ADDRESS_1M_FLASH 0xF0000 //F0000 & F1000 & F2000 & F3000
#endif                                             //Reuse 'ACL Central simple SDP' storage area

/* for LE Audio SDP: bonding audio GATT service information area (reuse FLASH_SDP_ATT_ADDRESS_2M_FLASH) */
#ifndef FLASH_AUD_ATT_ADDRESS_2M_FLASH
    #define FLASH_AUD_ATT_ADDRESS_2M_FLASH 0x1E8000 //1E8000 & 1E9000 & 1EA000 & 1EB000
#endif                                              //Reuse 'ACL Central simple SDP' storage area

/* for LE Audio SDP: bonding audio GATT service information area (reuse FLASH_SDP_ATT_ADDRESS_4M_FLASH) */
#ifndef FLASH_AUD_ATT_ADDRESS_4M_FLASH
    #define FLASH_AUD_ATT_ADDRESS_4M_FLASH 0x3E8000 //3E8000 & 3E9000 & 3EA000 & 3EB000
#endif                                              //Reuse 'ACL Central simple SDP' storage area

/* for LE Audio SDP: bonding audio GATT service information area (reuse FLASH_SDP_ATT_ADDRESS_16M_FLASH) */
#ifndef FLASH_AUD_ATT_ADDRESS_16M_FLASH
    #define FLASH_AUD_ATT_ADDRESS_16M_FLASH 0xFE8000 //FE8000 & FE9000 & FEA000 & FEB000
#endif                                               //Reuse 'ACL Central simple SDP' storage area


/**
 * @brief      This function is used to initialize store profile pair information module.
 * @param[in]  none
 * @return     none
 */
void blc_prf_initPairingInfoStoreModule(void);


/**
 * @brief      This function is used to configure the audio information storage address and size.
 * @param[in]  address - AUD bonding storage start address.
 * @param[in]  size_byte - AUD bonding storage size(e.g.: 2*4096).
 * @Note:      address and size must be 4K aligned, total used flash size: 2 * size_byte
 * @return     none.
 * @note       If used, need to call before FUNC: blc_prf_initPairingInfoStoreModule
 */
void blc_prf_configPairingInfoStorageAddressAndSize(int address, int size_byte);

/**
 * @brief       This function is used for retrieving location of stored bonding information based on ACL handle
 * @param[in]   connHandle  - ACL Connect Handle..
 * @param[out]  valueLen    - length of the stored nv record, NULL if none found
 * @return      flash address of the record found.
 */
u32 blt_prf_searchBondingDeviceByAclHandle(u16 connHandle, u16* valueLen);

/**
 * @brief       This function is used for removing stored bonding information based on ACL handle
 * @param[in]   connHandle  - ACL Connect Handle..
 * @return      none.
 */
void blt_prf_deleteBondingInfoByFlashAddress(u32 flashAddr);
/**
 * @brief       This function is used to set max profile bond number.
 * @param[in]   none
 * @return      set status 0 means success.
 */
int blc_prf_setPairingDeviceMaxNumber (int maxBondNum);
