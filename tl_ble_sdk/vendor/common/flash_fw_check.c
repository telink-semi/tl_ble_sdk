/********************************************************************************************************
 * @file    flash_fw_check.c
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
#if 1
#include "tl_common.h"
#include "drivers.h"
#include "stack/ble/ble.h"
#include "flash_fw_check.h"




static const unsigned long fw_crc32_half_tbl[16] = {
    0x00000000, 0x1db71064, 0x3b6e20c8, 0x26d930ac,
    0x76dc4190, 0x6b6b51f4, 0x4db26158, 0x5005713c,
    0xedb88320, 0xf00f9344, 0xd6d6a3e8, 0xcb61b38c,
    0x9b64c2b0, 0x86d3d2d4, 0xa00ae278, 0xbdbdf21c
};

#define FW_READ_SIZE      256 ///16 ///256 require more stack space

u32 fw_crc_init = 0xFFFFFFFF;

/***********************************
 * this function must be called after the function sys_init.
 * sys_init will set the ota_program_offset value.
 */
/**
 * @brief       This function is used to check the firmware is ok or not
 * @param[in]   crc_init_value - the initial value of CRC
 * @return      0 - CRC is check success
 *              1 - CRC is check fail
 */
bool flash_fw_check( u32 crc_init_value ){

    if(!crc_init_value){
        fw_crc_init = 0xFFFFFFFF;
    }else{
        fw_crc_init = crc_init_value;
    }

    /////find the real FW flash address
    u32 fw_flashAddr = blc_ota_getCurrentFirmwareStartAddress();
    u32 fw_size;
    flash_read_page( (fw_flashAddr+0x18), 4, (u8*)&fw_size); ///0x18 store bin size value

    u16 fw_Block;
    u16 fw_remainSizeByte;
    fw_Block = fw_size/FW_READ_SIZE;
    fw_remainSizeByte = fw_size%FW_READ_SIZE;

    int i=0;
    u8 fw_tmpdata[FW_READ_SIZE]; ///
    u8 ota_dat[FW_READ_SIZE<<1];
    for(i=0; i < fw_Block; i++){ ///Telink bin must align 16 bytes.

        flash_read_page( (fw_flashAddr+i*FW_READ_SIZE), FW_READ_SIZE, fw_tmpdata);

        //FW_READ_SIZE byte OTA data32  half byteCRC
        for(int j=0;j<FW_READ_SIZE;j++){
            ota_dat[j*2] = fw_tmpdata[j]&0x0f;
            ota_dat[j*2+1] = fw_tmpdata[j]>>4;
        }
        fw_crc_init = crc32_half_cal(fw_crc_init, ota_dat, (unsigned long* )(unsigned int)fw_crc32_half_tbl, (FW_READ_SIZE<<1));
    }

    //////////////////////////////
    if(fw_remainSizeByte != 4){
        flash_read_page( (fw_flashAddr+fw_size -fw_remainSizeByte), (fw_remainSizeByte-4), fw_tmpdata);
        for(i=0;i<(fw_remainSizeByte-4);i++){
            ota_dat[i*2] = fw_tmpdata[i]&0x0f;
            ota_dat[i*2+1] = fw_tmpdata[i]>>4;
        }
        fw_crc_init = crc32_half_cal(fw_crc_init, ota_dat, (unsigned long* )(unsigned int)fw_crc32_half_tbl, ((fw_remainSizeByte-4)<<1));
    }

    ////////read crc value and compare
    u32 fw_check_value;
    flash_read_page( (fw_flashAddr+fw_size-4), 4, (u8*)&fw_check_value);

    if(fw_check_value != fw_crc_init){
        return 1;  ///CRC check fail
    }

    return 0; ///CRC check ok
}

#endif



















