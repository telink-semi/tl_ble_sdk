/********************************************************************************************************
 * @file    bqb_config.h
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
#ifndef VENDOR_B91_CONTROLLER_BQB_CONFIG_H_
#define VENDOR_B91_CONTROLLER_BQB_CONFIG_H_


/**
 * @brief         enable the extended advertisement module.
 * @note          Run #222 - /HCI/DDI/BI-08-C.
 */
#define APP_LE_EXTENDED_ADV_EN 1


/**
 * @brief         enable the 2M and coded phy.
 * @note          default to enable.
 */
#define APP_LE_2M_CODED_PHY_EN 1


/**
 * @brief         enable the channel selection algorithm 2.
 * @note          default to enable.
 */
#define APP_LE_CHANNEL_SELECTION_ALGORITHM_2_EN 1


/**
 * @brief         enable the periodic advertisement module.
 */
#define APP_LE_PERIODIC_ADV_EN 0


/**
 * @brief         enable the periodic advertisement with response module.
 */
#define APP_LE_PAWR_ADV_EN 0


/**
 * @brief         enable the extended scan module.
 */
#define APP_LE_EXTENDED_SCAN_EN 1


/**
 * @brief         enable the extended initiating module.
 */
#define APP_LE_EXTENDED_INIT_EN 1


/**
 * @brief         enable the periodic advertising synchronization module.
 */
#define APP_SYNCHRONIZED_RECEIVER_EN 0


/**
 * @brief         enable the advertising coding selection module.
 */
#define APP_ADVERTISING_CODING_SELECT_EN 0


/**
 * @brief         enable the PHY test module.
 */
#define APP_LE_PHY_TEST_EN 0


/**
 * @brief         enable the ChnClassification feature.
 */
#define APP_CHN_CLASS_EN 0

/**
 * @brief         enable the Periodic sync feature.
 */
#define APP_SYNCHRONIZED_RECEIVER_EN 0

/**
 * @brief         enable the power control feature.
 */
#define APP_POWER_CONTROL 0


/**
 * @brief         enable the PAST feature.
 */
#define APP_PAST_EN 0


/**
 * @brief         enable the channel sounding feature.
 */
#define APP_LE_CHANNEL_SOUNDING 0


/**
 * @brief         enable the subrate feature.
 * @note          config by ACL_TXFIFO_4K_LIMITATION_WORKAROUND.
 */
#define APP_WORKAROUND_TX_FIFO_4K_LIMITATION_EN 0
#if APP_WORKAROUND_TX_FIFO_4K_LIMITATION_EN == 1
    #define ACL_TXFIFO_4K_LIMITATION_WORKAROUND 1
#endif


#endif /* VENDOR_B91_CONTROLLER_BQB_CONFIG_H_ */
