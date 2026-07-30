/********************************************************************************************************
 * @file    app_buffer.h
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
#ifndef APP_BUFFER_H_
#define APP_BUFFER_H_

#include "tl_common.h"
#include "drivers.h"
#include "stack/ble/ble.h"

#include "app_config.h"

#if APP_LE_EXTENDED_ADV_EN
/*********************************** Extended ADV data buffer allocation, Begin ************************************/
/**
 * @brief   Extended Advertising Set number and data buffer length
 *
 *  APP_EXT_ADV_SETS_NUMBER:
 *  Number of extended ADV Sets, user set value, should be in range of 1 ~ "ADV_SETS_NUMBER_MAX"
 *
 *  APP_EXT_ADV_DATA_LENGTH:
 *  Extended ADV Data Length, user set value, should be in range of 31 ~ 1650
 *  if ADV Set number bigger than 1, this length should be maximum value of ADV data length for all ADV Sets
 *  if all ADV Set use legacy ADV mode, 31 byte is enough
 *
 *
 *  APP_EXT_SCANRSP_DATA_LENGTH:
 *  Extended ADV Scan Response data Length, user set value, should be in range of 31 ~ 1650
 *  if ADV set number bigger than 1, this length should be maximum value of scan response data length for all ADV Sets
 *  if all ADV Set use legacy ADV mode, 31 byte is enough
 */
#define APP_EXT_ADV_SETS_NUMBER     4   //user set value
#define APP_EXT_ADV_DATA_LENGTH     255 //2048//1664//1024   //user set value
#define APP_EXT_SCANRSP_DATA_LENGTH 255 //2048//1664//1024   //user set value


extern u8 app_extAdvSetParam_buf[];
extern u8 app_extAdvData_buf[];
extern u8 app_extScanRspData_buf[];
/*********************************** Extended ADV data buffer allocation, End** ************************************/
#endif

#if (APP_LE_CHANNEL_SOUNDING)
/**************************************** CS rx Buffer allocation, Begin *******************************************/

/**
 * @brief   CS RX Data buffer length
 */
#if ANTENNA_SWITCHING_AUTO_EN
#define     CS_RX_FIFO_SIZE            CHANNEL_SOUNDING_RX_FIFO_SIZE_ALIGN16(MAX_ANT_PATHS_SUPPORT, 40, 10)
#else
#define     CS_RX_FIFO_SIZE            CHANNEL_SOUNDING_RX_FIFO_SIZE_ALIGN16(2, 40, 10)
#endif

#define     CS_RX_FIFO_NUM              4

#define     APP_CS_CONFIG_NUM           4

extern  u8 app_cs_rx_buf[];
extern u8 app_CsConfigParam[];
/**************************************** CS Buffer allocation, End ***********************************************/
#endif


/********************* ACL connection LinkLayer TX & RX data FIFO allocation, Begin ************************************************/
/**
 * @brief   connMaxRxOctets
 * refer to BLE SPEC "4.5.10 Data PDU length management" & "2.4.2.21 LL_LENGTH_REQ and LL_LENGTH_RSP"
 * usage limitation:
 * 1. should be in range of 27 ~ 251
 * 2. for CIS peripheral, receive ll_cis_req(36Byte), must be equal to or greater than 36
 */
#define ACL_CONN_MAX_RX_OCTETS 251 //user set value


/**
 * @brief   connMaxTxOctets
 * refer to BLE SPEC: Vol 6, Part B, "4.5.10 Data PDU length management"
 *                    Vol 6, Part B, "2.4.2.21 LL_LENGTH_REQ and LL_LENGTH_RSP"
 *  in this SDK, we separate this value into 2 parts: peripheralMaxTxOctets and centralMaxTxOctets,
 *  for purpose to save some SRAM costed by when peripheral and central use different connMaxTxOctets.
 *
 * usage limitation for ACL_xxx_MAX_TX_OCTETS
 * 1. should be in range of 27 ~ 251
 * 2. for CIS central, send ll_cis_req(36Byte), ACL_CENTRAL_MAX_TX_OCTETS must be equal to or greater than 36
 */
#define ACL_CENTRAL_MAX_TX_OCTETS 251 //user set value
#define ACL_PERIPHR_MAX_TX_OCTETS 251 //user set value

#define ACL_CONN_MAX_TX_OCTETS    251 // !!! max(acl_centarl_max_tx, acl_peripheral_max_tx)

/**
 * @brief   ACL RX buffer size & number
 *          ACL RX buffer is shared by all connections to hold LinkLayer RF RX data.
 * usage limitation for ACL_RX_FIFO_SIZE:
 * 1. must use CAL_LL_ACL_RX_FIFO_SIZE to calculate, user can not change !!!
 *
 * usage limitation for ACL_RX_FIFO_NUM:
 * 1. must be: 2^n, (power of 2)
 * 2. at least 4; recommended value: 4, 8, 16
 */
#define ACL_RX_FIFO_SIZE CAL_LL_ACL_RX_FIFO_SIZE(ACL_CONN_MAX_RX_OCTETS) //user can not change !!!
#define ACL_RX_FIFO_NUM  8                                               //user set value


/**
 * @brief   ACL TX buffer size & number
 *          ACL Central TX buffer is shared by all central connections to hold LinkLayer RF TX data.
*           ACL Peripheral TX buffer is shared by all peripheral connections to hold LinkLayer RF TX data.
 * usage limitation for ACL_xxx_TX_FIFO_SIZE:
 * 1. must use CAL_LL_ACL_TX_FIFO_SIZE to calculate, user can not change !!!
 *
 * usage limitation for ACL_xxx_TX_FIFO_NUM:
 * 1. must be: (2^n) + 1, (power of 2, then add 1)
 * 2. for B91m IC: at least 9; recommended value: 9, 17, 33; other value not allowed.
 * 3. for B85m IC: at least 8; recommended value: 8, 16, 32; other value not allowed.
 *
 * only for B91: usage limitation for size * (number - 1)
 * 1. (ACL_xxx_TX_FIFO_SIZE * (ACL_xxx_TX_FIFO_NUM - 1)) must be less than 4096 (4K)
 *    so when ACL TX FIFO size equal to or bigger than 256, ACL TX FIFO number can only be 9(can not use 17 or 33), cause 256*(17-1)=4096
 */
#define ACL_CENTRAL_TX_FIFO_SIZE CAL_LL_ACL_TX_FIFO_SIZE(ACL_CENTRAL_MAX_TX_OCTETS) //user can not change !!!
#define ACL_CENTRAL_TX_FIFO_NUM  9                                                  //user set value

#define ACL_PERIPHR_TX_FIFO_SIZE CAL_LL_ACL_TX_FIFO_SIZE(ACL_PERIPHR_MAX_TX_OCTETS) //user can not change !!!
#define ACL_PERIPHR_TX_FIFO_NUM  9                                                  //user set value


extern u8 app_acl_rx_fifo[];
extern u8 app_acl_cen_tx_fifo[];
extern u8 app_acl_per_tx_fifo[];
extern u8 app_acl_cache_Txfifo[];
/******************** ACL connection LinkLayer TX & RX data FIFO allocation, End ***************************************************/


/***************************** HCI TX & RX data FIFO allocation, Begin *********************************************/
/**
 * @brief   LE_ACL_Data_Packet_Length, refer to BLE SPEC "7.8.2 LE Read Buffer Size command"
 * usage limitation:
 * 1. only used for BLE controller project
 * 2. must greater than maximum of ACL_PERIPHR_MAX_TX_OCTETS, ACL_CENTRAL_MAX_TX_OCTETS
 * 4. can not exceed 252(for maximum tx_octets when DLE used)
 */
#define LE_ACL_DATA_PACKET_LENGTH ACL_CONN_MAX_TX_OCTETS

#define HCI_RX_ACL_FIFO_SIZE      CALCULATE_HCI_ACL_DATA_FIFO_SIZE(LE_ACL_DATA_PACKET_LENGTH)
#define HCI_RX_ACL_FIFO_NUM       8


#define HCI_TX_FIFO_SIZE          HCI_ACL_IN_FIFO_SIZE(0xFF)
#define HCI_TX_FIFO_NUM           8

#define HCI_RX_FIFO_SIZE          HCI_ACL_IN_FIFO_SIZE(0xFF)
#define HCI_RX_FIFO_NUM           8


extern u8 app_hci_rxfifo[];
extern u8 app_hci_txfifo[];
extern u8 app_hci_rxAclfifo[];


/****************************** HCI TX & RX data FIFO allocation, End *********************************************/


#endif /* APP_BUFFER_H_ */
