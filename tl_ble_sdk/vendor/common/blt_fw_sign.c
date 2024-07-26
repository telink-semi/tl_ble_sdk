/********************************************************************************************************
 * @file    blt_fw_sign.c
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
#include "blt_fw_sign.h"
#include "proj_lib/firmware_encrypt.h"
#include "ble_flash.h"

/**
 * @brief       This function is used to check digital signature of firmware
 * @param[in]   none
 * @return      none
 */
void blt_firmware_signature_check(void)
{
        unsigned int flash_mid;
        unsigned char flash_uid[16];
        unsigned char signature_enc_key[16];
        int flag = flash_read_mid_uid_with_check(&flash_mid, flash_uid);


        if(flag==0){  //reading flash UID error
            while(1);
        }

        firmware_encrypt_based_on_uid (flash_uid, signature_enc_key);

        /* must using "0x20000000 | address" when reading flash data by address pointer */
        if(memcmp(signature_enc_key, (u8*)(FLASH_R_BASE_ADDR | (flash_sector_calibration + CALIB_OFFSET_FIRMWARE_SIGNKEY)), 16)){  //signature not match
            while(1);   //user can change the code here to stop firmware running
        }
}
