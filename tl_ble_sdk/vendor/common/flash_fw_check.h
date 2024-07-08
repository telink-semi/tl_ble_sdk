/********************************************************************************************************
 * @file    flash_fw_check.h
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
#ifndef _FLASH_FW_CHECK_H_
#define _FLASH_FW_CHECK_H_


#include "../../common/types.h"


/**
 * @brief        This function is used to check the firmware is ok or not
 * @param[in]    crc_init_value - the initial value of CRC
 * @return        0 - CRC is check success
 *                 1 - CRC is check fail
 */
bool flash_fw_check( u32 crc_init_value );



#endif
