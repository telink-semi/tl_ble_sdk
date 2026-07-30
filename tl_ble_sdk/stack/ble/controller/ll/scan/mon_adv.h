/********************************************************************************************************
 * @file    mon_adv.h
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

#ifndef STACK_BLE_CONTROLLER_LL_SCAN_MON_ADV_H_
#define STACK_BLE_CONTROLLER_LL_SCAN_MON_ADV_H_
//Note: This feature is supported from Spec 6.0.

#include "stack/ble/hci/hci_cmd.h"

/**
 * @brief      This function is used to add a device into monitored advertisers list.
 * @param      device information, ref to HCI cmd "7.8.146 LE Add Device To Monitored Advertisers List command".
 * @return     Status - 0x00: command succeeded; 0x01-0xFF: command failed
 */
ble_sts_t blc_hci_le_addDeviceToMonitoredAdvertisersList(hci_le_addDeviceToMonitoredAdvertisersListcmdParam_t *pCmdParam);

/**
 * @brief      This function enable monitoring advertisers.
 * @param      1: start; 0: stop.
 * @return     Status - 0x00: command succeeded; 0x01-0xFF: command failed
 */
ble_sts_t blc_ll_monitoringAdvertisersEnable(u8 enable);

/**
 * @brief      This function read the size of monitored advertisers list out.
 * @param      the size will be put into this param, ref to HCI cmd "7.8.150 LE Read Monitored Advertisers List Size command".
 * @return     Status - 0x00: command succeeded; 0x01-0xFF: command failed
 */
ble_sts_t blc_ll_readMonitoredAdvertisersListSize(hci_le_readMonitoredAdvertisersListSizeStatusParam_t *pRetParam);

/**
 * @brief      This function is used to remove a specific device from monitored advertisers list.
 * @param      device address type
 * @param      device address
 * @return     Status - 0x00: command succeeded; 0x01-0xFF: command failed
 */
ble_sts_t blc_ll_removeDeviceFromMonitoredAdvertisersList(u8 adr_type, u8 *addr);

/**
 * @brief      This function is used to clear monitored advertisers list.
 * @param      none
 * @return     Status - 0x00: command succeeded; 0x01-0xFF: command failed
 */
ble_sts_t blc_ll_clearMonitoredAdvertisersList(void);

/**
 * @brief      This function is used to initialize monitoring advertisers function.
 * @param      the size of monitored advertisers list (TODO: not used yet)
 * @return     Status - 0x00: command succeeded; 0x01-0xFF: command failed
 */
ble_sts_t blc_ll_initMonitoringAdvertisers(int mon_adv_max_num);

#endif /* STACK_BLE_CONTROLLER_LL_SCAN_MON_ADV_H_ */
