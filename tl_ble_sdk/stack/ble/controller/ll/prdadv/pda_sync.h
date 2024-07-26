/********************************************************************************************************
 * @file    pda_sync.h
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
#ifndef STACK_BLE_CONTROLLER_LL_PRDADV_PDA_SYNC_H_
#define STACK_BLE_CONTROLLER_LL_PRDADV_PDA_SYNC_H_


/**
 * @brief      for user to initialize periodic advertising synchronization module
 * @param      none
 * @return     none
 */
void        blc_ll_initPeriodicAdvertisingSynchronization_module(void);


/**
 * @brief      this function is used to synchronize with a periodic advertising train from an advertiser and begin
               receiving periodic advertising packets.
 * @param[in]  options - used to determine whether the Periodic Advertiser List is used and whether HCI_Periodic_Advertising_Report events
 *                       for this periodic advertising train are initially enabled or disabled.
 * @param[in]  adv_sid - Advertising SID subfield in the ADI field used to identify the Periodic Advertising
 * @param[in]  adv_adrType - Advertiser Address Type
 * @param[in]  *adv_addr - Advertiser Address
 * @param[in]  skip - The maximum number of periodic advertising events that can be skipped after a successful receive
 * @param[in]  sync_timeout - Synchronization timeout for the periodic advertising train
 * @param[in]  sync_cte_type -
 * @return     Status - 0x00: command succeeded; 0x01-0xFF: command failed
 */
ble_sts_t   blc_ll_periodicAdvertisingCreateSync ( option_msk_t options, u8 adv_sid, u8 adv_adrType, u8 *adv_addr, u16 skip, sync_tm_t sync_timeout, u8 sync_cte_type);

/**
 * @brief      This function is used to cancel the periodic advertising create sync command while it is pending.
 * @param      none
 * @return     Status - 0x00: command succeeded; 0x01-0xFF: command failed
 */
ble_sts_t   blc_ll_periodicAdvertisingCreateSyncCancel (void);



/**
 * @brief      This function is used to stop reception of the periodic advertising train identified
 *             by the Sync_Handle parameter.
 * @param[in]  sync_handle - Sync_Handle identifying the periodic advertising train
 * @return     Status - 0x00: command succeeded; 0x01-0xFF: command failed
 */
ble_sts_t   blc_ll_periodicAdvertisingTerminateSync (u16 sync_handle);

/**
 * @brief      This function is used to add an entry, consisting of a single device address and SID,
 *             to the Periodic Advertiser list stored in the Controller.
 * @param[in]  adv_adrType - Advertiser_Address_Type
 * @param[in]  *adv_addr - Advertiser_Address
 * @param[in]  adv_sid - Advertising_SID
 * @return     Status - 0x00: command succeeded; 0x01-0xFF: command failed
 */
ble_sts_t   blc_ll_addDeviceToPeriodicAdvertiserList (u8 adv_adrType, u8 *adv_addr, u8 adv_sid);

/**
 * @brief      This function is used to remove one entry from the list of Periodic Advertisers
 *             stored in the Controller.
 * @param[in]  adv_adrType - Advertiser_Address_Type
 * @param[in]  *adv_addr - Advertiser_Address
 * @param[in]  adv_sid - Advertising_SID
 * @return     Status - 0x00: command succeeded; 0x01-0xFF: command failed
 */
ble_sts_t   blc_ll_removeDeviceFromPeriodicAdvertiserList (u8 adv_adrType, u8 *adv_addr, u8 adv_sid);

/**
 * @brief      This function is used to remove all entries from the list of Periodic Advertisers.
 * @param      none
 * @return     Status - 0x00: command succeeded; 0x01-0xFF: command failed
 */
ble_sts_t   blc_ll_clearPeriodicAdvertiserList (void);

/**
 * @brief      This function is used to read the total number of Periodic Advertiser list entries.
 * @param[in]  none
 * @param[out]  *perdAdvListSize - point to Periodic_Advertiser_List_Size
 * @return     Status - 0x00: command succeeded; 0x01-0xFF: command failed
 */
ble_sts_t   blc_ll_readPeriodicAdvertiserListSize (u8 *perdAdvListSize);

/**
 * @brief      This function is used to enable or disable reports for the periodic advertising train
 *             identified by the Sync_Handle parameter
 * @param[in]  sync_handle - Sync_Handle identifying the periodic advertising train
 * @param[in]  enable - refer to 'sync_adv_rcv_en_msk'
 * @return     Status - 0x00: command succeeded; 0x01-0xFF: command failed
 */
ble_sts_t   blc_ll_periodicAdvertisingReceiveEnable (u16 sync_handle, sync_adv_rcv_en_msk enable);




#endif /* STACK_BLE_CONTROLLER_LL_PRDADV_PDA_SYNC_H_ */
