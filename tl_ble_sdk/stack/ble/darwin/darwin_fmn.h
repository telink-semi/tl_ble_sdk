/********************************************************************************************************
 * @file    darwin_fmn.h
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

#ifndef STACK_BLE_DARWIN_DARWIN_FMN_H_
#define STACK_BLE_DARWIN_DARWIN_FMN_H_

typedef int (*blc_smp_paringreq_cb_t)(u16 connHandle);
typedef void (*blc_smp_sec_info_cb_t)(u16 connHandle);

/**
 * @brief      This func. used to enable custom fmn function.
 * @param      en, 0x01: enable.
 *                 0x00: disable, default value.
 *             pr_cb: pair request callback.
 *             sir_cb: security info request callback.
 * @return     none
 */
void blc_ll_setCustomFMNEnable(u8 en, blc_smp_paringreq_cb_t pr_cb, blc_smp_sec_info_cb_t sir_cb);


#endif /* STACK_BLE_DARWIN_DARWIN_FMN_H_ */
