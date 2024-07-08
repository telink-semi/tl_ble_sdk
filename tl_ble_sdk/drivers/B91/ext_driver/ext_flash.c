/********************************************************************************************************
 * @file    ext_flash.c
 *
 * @brief   This is the source file for BLE SDK
 *
 * @author  BLE GROUP
 * @date    06,2024
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
#include "ext_flash.h"


/**
 * @brief         This function is used to set the use of quad IO mode when reading and writing flash.
 * @param       None
 * @return         1: success, 0: error, 2: parameter error, 3: mid is not supported.
 */
unsigned char ble_flash_quad_mode_enable(void)
{
    unsigned int mid = ble_flash_read_mid();
    unsigned char status = flash_4line_en(mid);
    if(1 == status)
    {
        flash_read_page = flash_4read;
        flash_set_xip_config(XIP_X4READ_CMD);
        flash_write_page = flash_quad_page_program;
    }
    return status;
}