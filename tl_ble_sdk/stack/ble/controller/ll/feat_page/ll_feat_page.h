/********************************************************************************************************
 * @file    ll_feat_page.h
 *
 * @brief   This is the header file for BLE SDK
 *
 * @author  BLE GROUP
 * @date    03,2025
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
#ifndef LL_FEAT_PAGE_H_
#define LL_FEAT_PAGE_H_


/*
 * The set of features supported by a Link Layer is represented by a bit mask called
 * FeatureSet. This mask consists of 1984 bits divided into page 0 containing bits 0 to 63
 * (octets 0 to 7) and 10 pages of 192 bits (24 octets) each, numbered starting from 1 (i.e.,
 * page 1 is octets 8 to 31, page 2 is octets 32 to 55, etc.)
 */
#define MAX_FEATURE_PAGES           10 //page 0 [64 bits] resvered for FeatureSet bit0 to 63, page 1 to 10 [192 bits] for extended feature set
#define MAX_FEATURE_PAGE_SIZE       24
#define FEATURE_PAGE_SIZE_BITS      (MAX_FEATURE_PAGE_SIZE * 8)

enum {
    EXT_FEAT_PROCEDURE_WAIT_TX_FEAT_REQ = BIT(0),
    EXT_FEAT_PROCEDURE_START            = BIT(1),
    EXT_FEAT_PROCEDURE_REV_EXT_RSP      = BIT(2),
    EXT_FEAT_PROCEDURE_COMPLETE         = BIT(3),
};


enum{
    LL_EXTENDED_FEATURE_SET = BIT(7),
};


typedef struct {
    u8 extFeatSet[MAX_FEATURE_PAGES][MAX_FEATURE_PAGE_SIZE];
} blt_ll_ext_feature_set_t;

extern blt_ll_ext_feature_set_t LL_LOCAL_EXT_FEATURE_SET;
extern blt_ll_ext_feature_set_t LL_PEER_EXT_FEATURE_SET;

/**
 * @brief      This function is used to initialize the extended feature set features.
 * @param[in]  none
 * @return     none
 */
void blc_ll_initExtFeatureSet_feature(void);

/**
 * @brief      This function is used to request a change to one or more frame space values [LL format].    
 * @param[out]  max_page - The number of the highest-numbered page of the supported LE features that
                          contains at least one bit set to 1.
 * @param[out]  le_features - Bit Mask List of the supported LE features. See [Vol 6] Part B, Section 4.6.
 * @return     Status - 0x00: command succeeded; 0x01-0xFF: command failed
 */
ble_sts_t blc_ll_readAllLocalSupportedFeatures(u8 *max_page, u8 le_features[248]);

/**
 * @brief      This function is used to request a change to one or more frame space values [LL format].    
 * @param[in]  connHandle - connection handle    
 * @param[in]  pages_requested - The number of the highest-numbered page of features that the Host requires 
 *                              and the Controller shall obtain. : 0x00 to 0x0A
 * @return     Status - 0x00: command succeeded; 0x01-0xFF: command failed
 */
ble_sts_t blc_ll_readAllRemoteFeatures(u16 conn_handle, u8 pages_requested);

/**
 * @brief      This function is used to requests the supported LE features for the Controller [HCI command format].    
 * @param[out]  retPara - refer to 'hci_le_readAllLocalSupportedFeatures_retParam_t'
 * @return     Status - 0x00: command succeeded; 0x01-0xFF: command failed
 */
ble_sts_t blc_hci_le_readAllLocalSupportedFeatures(hci_le_readAllLocalSupportedFeatures_retParam_t *retPara);


/**
 * @brief      This function is used to requests, from the remote device identified by the Connection_Handle,
 *             the features used on the connection and the features supported by the remote device. [HCI command format].    
 * @param[in]  cmdPara - refer to 'hci_le_readAllRemoteFeatures_cmdParam_t'
 * @return     Status - 0x00: command succeeded; 0x01-0xFF: command failed
 */
ble_sts_t blc_hci_le_readAllRemoteFeatures(hci_le_readAllRemoteFeatures_cmdParam_t *cmdPara);

#endif //LL_FEAT_PAGE_H_

