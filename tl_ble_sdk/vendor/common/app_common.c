/********************************************************************************************************
 * @file    app_common.c
 *
 * @brief   This is the source file for BLE SDK
 *
 * @author  BLE GROUP
 * @date    06,2020
 *
 * @par     Copyright (c) 2020, Telink Semiconductor (Shanghai) Co., Ltd. ("TELINK")
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



/**
 * @brief      This function is check If IRK(local IRK or peer IRK) valid
 * @param[in]  pIrk - pointer to IRK(local IRK or peer IRK)
 * @return     1: IRK valid
 *             0: IRK invalid
 */
int blc_app_isIrkValid(u8* pIrk)
{

    u8 byte_all_0xFF[16] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    u8 byte_all_0x00[16] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

    /* IRK valid: not 16 0x00, not 16 0xFF */
    if(memcmp(pIrk, byte_all_0xFF, 16) && memcmp(pIrk, byte_all_0x00, 16)){
        return TRUE;
    }
    else{
        return FALSE;
    }
}
