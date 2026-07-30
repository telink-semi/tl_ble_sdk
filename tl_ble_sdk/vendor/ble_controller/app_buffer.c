/********************************************************************************************************
 * @file    app_buffer.c
 *
 * @brief   This is the source file for BLE SDK
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
#include "tl_common.h"
#include "drivers.h"
#include "stack/ble/ble.h"


#include "app.h"
#include "app_buffer.h"


/********************* USB_DEBUG_LOG FIFO allocation, Begin *******************************/


/******************** USB_DEBUG_LOG FIFO allocation, End ***********************************/


/*********************************** Extended ADV data buffer allocation, Begin ************************************/

#if APP_LE_EXTENDED_ADV_EN
/**
 * @brief   Extended ADV parameters buffer, used to hold extended ADV parameters which used in SDK.
 *          ADV_SET_PARAM_LENGTH is determined by SDK, user can not change !!!
 */
u8 app_extAdvSetParam_buf[ADV_SET_PARAM_LENGTH * APP_EXT_ADV_SETS_NUMBER];


/**
 * @brief   Extended ADV data buffer.
 *          if any ADV Set use ADV data(e.g. Extended, Connectable), this buffer should be defined and initialized.
 *          if no  ADV Set use ADV data(e.g. Extended, Scannable), this buffer is not necessary.
 */
u8 app_extAdvData_buf[APP_EXT_ADV_DATA_LENGTH * APP_EXT_ADV_SETS_NUMBER];


/**
 * @brief   Extended Scan Response data buffer.
 *          if any ADV Set use scan response data(e.g. Extended, Scannable), this buffer should be defined and initialized.
 *          if no  ADV Set use scan response data(e.g. Extended, Connectable), this buffer is not necessary.
 */
u8 app_extScanRspData_buf[APP_EXT_SCANRSP_DATA_LENGTH * APP_EXT_ADV_SETS_NUMBER];

#endif

/*********************************** Extended ADV data buffer allocation, End** ************************************/


/********************* ACL connection LinkLayer TX & RX data FIFO allocation, Begin *******************************/

/**
 * @brief   ACL RX buffer, shared by all connections to hold LinkLayer RF RX data.
 *          user should define and initialize this buffer if either ACL Central or ACL Peripheral is used.
 */
u8 app_acl_rx_fifo[ACL_RX_FIFO_SIZE * ACL_RX_FIFO_NUM] = {0};


/**
 * @brief   ACL Central TX buffer, shared by all central connections to hold LinkLayer RF TX data.
 *          ACL Central TX buffer should be defined only when ACl connection central role is used.
 */
u8 app_acl_cen_tx_fifo[ACL_CENTRAL_TX_FIFO_SIZE * ACL_CENTRAL_TX_FIFO_NUM * ACL_CENTRAL_MAX_NUM] = {0};


/**
 * @brief   ACL Peripheral TX buffer, shared by all peripheral connections to hold LinkLayer RF TX data.
 *          ACL Peripheral TX buffer should be defined only when ACl connection peripheral role is used.
 */
u8 app_acl_per_tx_fifo[ACL_PERIPHR_TX_FIFO_SIZE * ACL_PERIPHR_TX_FIFO_NUM * ACL_PERIPHR_MAX_NUM] = {0};

/******************** ACL connection LinkLayer TX & RX data FIFO allocation, End ***********************************/


#if (APP_WORKAROUND_TX_FIFO_4K_LIMITATION_EN && (ACL_CENTRAL_MAX_TX_OCTETS > 230 || ACL_PERIPHR_MAX_TX_OCTETS > 230))
u8 app_acl_cache_Txfifo[260 * 32] = {0};
#endif


#if (APP_LE_CHANNEL_SOUNDING)
/*************************************** CSrx Buffer allocation, Begin *********************************************/

u8 app_cs_rx_buf[CS_RX_FIFO_NUM * CS_RX_FIFO_SIZE];

_attribute_iram_bss_ u8 app_CsConfigParam[CS_PARAM_LENGTH*APP_CS_CONFIG_NUM] = {0};
/*************************************** CS Rx Buffer allocation, End **********************************************/
#endif


/***************************** HCI TX & RX data FIFO allocation, Begin *********************************************/
//_attribute_iram_noinit_data_
u8 app_hci_rxfifo[HCI_RX_FIFO_SIZE * HCI_RX_FIFO_NUM] = {0};
//_attribute_iram_noinit_data_
u8 app_hci_txfifo[HCI_TX_FIFO_SIZE * HCI_TX_FIFO_NUM] = {0};
//_attribute_iram_noinit_data_
u8 app_hci_rxAclfifo[HCI_RX_ACL_FIFO_SIZE * HCI_RX_ACL_FIFO_NUM] = {0};


/****************************** HCI TX & RX data FIFO allocation, ENd *********************************************/
