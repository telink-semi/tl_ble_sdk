/********************************************************************************************************
 * @file    flash_prot.c
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

#include "flash_prot.h"


_attribute_ble_data_retention_  _attribute_aligned_(4)  flash_prot_t    blc_flashProt;

/* this pointer is used only by stack, when OTA write or erase flash, will give a callback to application layer */
_attribute_ble_data_retention_  _attribute_aligned_(4)  flash_prot_op_callback_t        flash_prot_op_cb = NULL;


#if (APP_FLASH_PROTECTION_ENABLE)

_attribute_ble_data_retention_  _attribute_aligned_(4)  flash_lock_t                flash_lock_mid = NULL;
_attribute_ble_data_retention_  _attribute_aligned_(4)  flash_unlock_t              flash_unlock_mid = NULL;
_attribute_ble_data_retention_  _attribute_aligned_(4)  flash_get_lock_status_t     flash_get_lock_status_mid = NULL;


_attribute_ble_data_retention_  u16 flash_unlock_status; //record flash unlock status for different flash type


/**
 * @brief      this function is used to register callback for stack, when OTA write or erase flash, send event to application layer
 * @param[in]  cb - flash protection operation callback function
 * @return     none
 */
void blc_appRegisterStackFlashOperationCallback(flash_prot_op_callback_t cb)
{
    flash_prot_op_cb = cb;
}



/**
 * @brief      this function is used to initialize flash protection.Block size of lock is a sample, user can change it according to bin size.
 *               This function only shows the flash supported by TELINK testing. If user use other flash, please follow the code below.
 * @param[in]  none
 * @return     none
 */
void flash_protection_init(void)
{
    if(!blc_flash_mid){
        blc_flash_mid = ble_flash_read_mid();
    }

    /* According to the flash mid, execute the corresponding lock flash API. */
    switch(blc_flash_mid)
    {
        #if (FLASH_P25Q80SU_SUPPORT_EN)
            case MID146085:
                flash_lock_mid = flash_lock_mid146085;
                flash_unlock_mid = flash_unlock_mid146085;
                flash_get_lock_status_mid = (flash_get_lock_status_t)(size_t)flash_get_lock_block_mid146085;
                flash_unlock_status = FLASH_LOCK_NONE_MID146085;
                break;
        #endif

        #if (FLASH_P25Q80U_SUPPORT_EN)
            case MID146085:
                flash_lock_mid = flash_lock_mid146085;
                flash_unlock_mid = flash_unlock_mid146085;
                flash_get_lock_status_mid = (flash_get_lock_status_t)(size_t)flash_get_lock_block_mid146085;
                flash_unlock_status = FLASH_LOCK_NONE_MID146085;
                break;
        #endif

        #if (FLASH_P25Q16SU_SUPPORT_EN)
            case MID156085:
                flash_lock_mid = flash_lock_mid156085;
                flash_unlock_mid = flash_unlock_mid156085;
                flash_get_lock_status_mid = (flash_get_lock_status_t)(size_t)flash_get_lock_block_mid156085;
                flash_unlock_status = FLASH_LOCK_NONE_MID156085;
                break;
        #endif

        #if (FLASH_GD25LE80E_SUPPORT_EN)
            case MID1460C8:
                flash_lock_mid = flash_lock_mid1460c8;
                flash_unlock_mid = flash_unlock_mid1460c8;
                flash_get_lock_status_mid = (flash_get_lock_status_t)(size_t)flash_get_lock_block_mid1460c8;
                flash_unlock_status = FLASH_LOCK_NONE_MID1460C8;
                break;
        #endif

        #if (FLASH_GD25LE16E_SUPPORT_EN)
            case MID1560C8:
                flash_lock_mid = flash_lock_mid1560c8;
                flash_unlock_mid = flash_unlock_mid1560c8;
                flash_get_lock_status_mid = (flash_get_lock_status_t)(size_t)flash_get_lock_block_mid1560c8;
                flash_unlock_status = FLASH_LOCK_NONE_MID1560C8;
                break;
        #endif

        #if (FLASH_P25Q32SU_SUPPORT_EN)
            case MID166085:
                flash_lock_mid = flash_lock_mid166085;
                flash_unlock_mid = flash_unlock_mid166085;
                flash_get_lock_status_mid = (flash_get_lock_status_t)(size_t)flash_get_lock_block_mid166085;
                flash_unlock_status = FLASH_LOCK_NONE_MID166085;
                break;
        #endif

        #if (FLASH_P25Q128L_SUPPORT_EN)
            case MID186085:
                flash_lock_mid = flash_lock_mid186085;
                flash_unlock_mid = flash_unlock_mid186085;
                flash_get_lock_status_mid = (flash_get_lock_status_t)(size_t)flash_get_lock_block_mid186085;
                flash_unlock_status = FLASH_LOCK_NONE_MID186085;
                break;
        #endif

        #if (FLASH_P25Q128H_SUPPORT_EN)
            case MID182085:
                flash_lock_mid = flash_lock_mid182085;
                flash_unlock_mid = flash_unlock_mid182085;
                flash_get_lock_status_mid = (flash_get_lock_status_t)(size_t)flash_get_lock_block_mid182085;
                flash_unlock_status = FLASH_LOCK_NONE_MID182085;
                break;
        #endif

        #if (FLASH_GD25LQ16E_SUPPORT_EN)
            case MID1560c8:
                flash_lock_mid = flash_lock_mid1560c8;
                flash_unlock_mid = flash_unlock_mid1560c8;
                flash_get_lock_status_mid = (flash_get_lock_status_t)(size_t)flash_get_lock_block_mid1560c8;
                flash_unlock_status = FLASH_LOCK_NONE_MID1560c8;
                break;
        #endif

        default:
            /*This SDK do not support other flash type except what listed above. If code stop here, please check current Flash */
            tlkapi_send_string_u32s(APP_FLASH_PROT_LOG_EN, "[FLASH][INI] unsupported flash type !!!", blc_flash_mid);
            blc_flashProt.init_err = 1;
            break;
    }
}





/**
 * @brief      this function is used to change application lock block definition to flash lock block definition according to flash type used
 *             attention: only processed a few lock block size for SDK demo, if you have more lock block size, please
 *                        add more code by yourself
 * @param[in]  app_lock_block - application lock block definition
 * @return     flash_lock_block_size - The size of the lock block size of flash.
 */
u16 flash_change_app_lock_block_to_flash_lock_block(flash_app_lock_e app_lock_block)
{
    u32 flash_lock_block_size = 0;

    switch(blc_flash_mid)
    {
        #if (FLASH_P25Q80U_SUPPORT_EN || FLASH_P25Q80SU_SUPPORT_EN) //1M capacity
            case MID146085:
                if(app_lock_block == FLASH_LOCK_FW_LOW_256K){
                    flash_lock_block_size = FLASH_LOCK_LOW_256K_MID146085;
                    tlkapi_printf(APP_FLASH_PROT_LOG_EN, "[FLASH][PROT] flash lock low 256K block!\n");
                }
                else if(app_lock_block == FLASH_LOCK_FW_LOW_512K){
                    flash_lock_block_size = FLASH_LOCK_LOW_512K_MID146085;
                    tlkapi_printf(APP_FLASH_PROT_LOG_EN, "[FLASH][PROT] flash lock low 512K block!\n");
                }
                else if(app_lock_block == FLASH_LOCK_FW_LOW_1M){
                    /* attention 1: use can change this value according to application
                     * attention 2: can not lock stack SMP data storage area
                     * attention 3: firmware size under protection is not 1M, user should calculate
                     * demo code: choose 960K, leave at 64K for system data(SMP storage data & calibration data & MAC address) and user data,
                     * if this demo can not meet your requirement, you should change !!!*/
                    flash_lock_block_size = FLASH_LOCK_LOW_960K_MID146085;
                    tlkapi_printf(APP_FLASH_PROT_LOG_EN, "[FLASH][PROT] flash lock low 960K block!\n");
                }
                else if(app_lock_block == FLASH_LOCK_ALL_AREA){
                    flash_lock_block_size = FLASH_LOCK_ALL_1M_MID146085;
                    tlkapi_printf(APP_FLASH_PROT_LOG_EN, "[FLASH][PROT] flash lock all area!\n");
                }
                else{
                    blc_flashProt.init_err = 1;
                }

                break;
        #endif

        #if (FLASH_GD25LE80E_SUPPORT_EN) //1M capacity
            case MID1460C8:
                if(app_lock_block == FLASH_LOCK_FW_LOW_256K){
                    flash_lock_block_size = FLASH_LOCK_LOW_256K_MID1460C8;
                    tlkapi_printf(APP_FLASH_PROT_LOG_EN, "[FLASH][PROT] flash lock low 256K block!\n");
                }
                else if(app_lock_block == FLASH_LOCK_FW_LOW_512K){
                    flash_lock_block_size = FLASH_LOCK_LOW_512K_MID1460C8;
                    tlkapi_printf(APP_FLASH_PROT_LOG_EN, "[FLASH][PROT] flash lock low 512K block!\n");
                }
                else if(app_lock_block == FLASH_LOCK_FW_LOW_1M){
                    /* attention 1: use can change this value according to application
                     * attention 2: can not lock stack SMP data storage area
                     * attention 3: firmware size under protection is not 1M, user should calculate
                     * demo code: choose 960K, leave at 64K for system data(SMP storage data & calibration data & MAC address) and user data,
                     * if this demo can not meet your requirement, you should change !!!*/
                    flash_lock_block_size = FLASH_LOCK_LOW_960K_MID1460C8;
                    tlkapi_printf(APP_FLASH_PROT_LOG_EN, "[FLASH][PROT] flash lock low 960K block!\n");
                }
                else if(app_lock_block == FLASH_LOCK_ALL_AREA){
                    flash_lock_block_size = FLASH_LOCK_ALL_1M_MID1460C8;
                    tlkapi_printf(APP_FLASH_PROT_LOG_EN, "[FLASH][PROT] flash lock all area!\n");
                }
                else{
                    blc_flashProt.init_err = 1;
                }

                break;
        #endif

        #if (FLASH_P25Q16SU_SUPPORT_EN) //2M capacity
            case MID156085:
                if(app_lock_block == FLASH_LOCK_FW_LOW_256K){
                    flash_lock_block_size = FLASH_LOCK_LOW_256K_MID156085;
                    tlkapi_printf(APP_FLASH_PROT_LOG_EN, "[FLASH][PROT] flash lock low 256K block!\n");
                }
                else if(app_lock_block == FLASH_LOCK_FW_LOW_512K){
                    flash_lock_block_size = FLASH_LOCK_LOW_512K_MID156085;
                    tlkapi_printf(APP_FLASH_PROT_LOG_EN, "[FLASH][PROT] flash lock low 512K block!\n");
                }
                else if(app_lock_block == FLASH_LOCK_FW_LOW_1M){
                    flash_lock_block_size = FLASH_LOCK_LOW_1M_MID156085;
                    tlkapi_printf(APP_FLASH_PROT_LOG_EN, "[FLASH][PROT] flash lock low 1M block!\n");
                }
                else if(app_lock_block == FLASH_LOCK_ALL_AREA){
                    flash_lock_block_size = FLASH_LOCK_ALL_2M_MID156085;
                    tlkapi_printf(APP_FLASH_PROT_LOG_EN, "[FLASH][PROT] flash lock all area!\n");
                }
                else{
                    blc_flashProt.init_err = 1;
                }

                break;
        #endif

        #if (FLASH_GD25LE16E_SUPPORT_EN) //2M capacity
            case MID1560C8:
                if(app_lock_block == FLASH_LOCK_FW_LOW_256K){
                    flash_lock_block_size = FLASH_LOCK_LOW_256K_MID1560C8;
                    tlkapi_printf(APP_FLASH_PROT_LOG_EN, "[FLASH][PROT] flash lock low 256K block!\n");
                }
                else if(app_lock_block == FLASH_LOCK_FW_LOW_512K){
                    flash_lock_block_size = FLASH_LOCK_LOW_512K_MID1560C8;
                    tlkapi_printf(APP_FLASH_PROT_LOG_EN, "[FLASH][PROT] flash lock low 512K block!\n");
                }
                else if(app_lock_block == FLASH_LOCK_FW_LOW_1M){
                    flash_lock_block_size = FLASH_LOCK_LOW_1M_MID1560C8;
                    tlkapi_printf(APP_FLASH_PROT_LOG_EN, "[FLASH][PROT] flash lock low 1M block!\n");
                }
                else if(app_lock_block == FLASH_LOCK_ALL_AREA){
                    flash_lock_block_size = FLASH_LOCK_ALL_2M_MID1560C8;
                    tlkapi_printf(APP_FLASH_PROT_LOG_EN, "[FLASH][PROT] flash lock all area!\n");
                }
                else{
                    blc_flashProt.init_err = 1;
                }

                break;
        #endif

        #if (FLASH_P25Q32SU_SUPPORT_EN) //4M capacity
            case MID166085:
                if(app_lock_block == FLASH_LOCK_FW_LOW_256K){
                    flash_lock_block_size = FLASH_LOCK_LOW_256K_MID166085;
                    tlkapi_printf(APP_FLASH_PROT_LOG_EN, "[FLASH][PROT] flash lock low 256K block!\n");
                }
                else if(app_lock_block == FLASH_LOCK_FW_LOW_512K){
                    flash_lock_block_size = FLASH_LOCK_LOW_512K_MID166085;
                    tlkapi_printf(APP_FLASH_PROT_LOG_EN, "[FLASH][PROT] flash lock low 512K block!\n");
                }
                else if(app_lock_block == FLASH_LOCK_FW_LOW_1M){
                    flash_lock_block_size = FLASH_LOCK_LOW_1M_MID166085;
                    tlkapi_printf(APP_FLASH_PROT_LOG_EN, "[FLASH][PROT] flash lock low 1M block!\n");
                }
                else if(app_lock_block == FLASH_LOCK_ALL_AREA){
                    flash_lock_block_size = FLASH_LOCK_ALL_4M_MID166085;
                    tlkapi_printf(APP_FLASH_PROT_LOG_EN, "[FLASH][PROT] flash lock all area!\n");
                }
                else{
                    blc_flashProt.init_err = 1;
                }

                break;
        #endif

        #if (FLASH_P25Q128L_SUPPORT_EN) //16M capacity
            case MID186085:
                if(app_lock_block == FLASH_LOCK_FW_LOW_256K){
                    flash_lock_block_size = FLASH_LOCK_LOW_256K_MID186085;
                    tlkapi_printf(APP_FLASH_PROT_LOG_EN, "[FLASH][PROT] flash lock low 256K block!\n");
                }
                else if(app_lock_block == FLASH_LOCK_FW_LOW_512K){
                    flash_lock_block_size = FLASH_LOCK_LOW_512K_MID186085;
                    tlkapi_printf(APP_FLASH_PROT_LOG_EN, "[FLASH][PROT] flash lock low 512K block!\n");
                }
                else if(app_lock_block == FLASH_LOCK_FW_LOW_1M){
                    flash_lock_block_size = FLASH_LOCK_LOW_1M_MID186085;
                    tlkapi_printf(APP_FLASH_PROT_LOG_EN, "[FLASH][PROT] flash lock low 1M block!\n");
                }
                else if(app_lock_block == FLASH_LOCK_ALL_AREA){
                    flash_lock_block_size = FLASH_LOCK_ALL_16M_MID186085;
                    tlkapi_printf(APP_FLASH_PROT_LOG_EN, "[FLASH][PROT] flash lock all area!\n");
                }
                else{
                    blc_flashProt.init_err = 1;
                }


                break;
        #endif

        #if (FLASH_P25Q128H_SUPPORT_EN) //16M capacity
            case MID182085:
                if(app_lock_block == FLASH_LOCK_FW_LOW_256K){
                    flash_lock_block_size = FLASH_LOCK_LOW_256K_MID182085;
                    tlkapi_printf(APP_FLASH_PROT_LOG_EN, "[FLASH][PROT] flash lock low 256K block!\n");
                }
                else if(app_lock_block == FLASH_LOCK_FW_LOW_512K){
                    flash_lock_block_size = FLASH_LOCK_LOW_512K_MID182085;
                    tlkapi_printf(APP_FLASH_PROT_LOG_EN, "[FLASH][PROT] flash lock low 512K block!\n");
                }
                else if(app_lock_block == FLASH_LOCK_FW_LOW_1M){
                    flash_lock_block_size = FLASH_LOCK_LOW_1M_MID182085;
                    tlkapi_printf(APP_FLASH_PROT_LOG_EN, "[FLASH][PROT] flash lock low 1M block!\n");
                }
                else if(app_lock_block == FLASH_LOCK_ALL_AREA){
                    flash_lock_block_size = FLASH_LOCK_ALL_16M_MID182085;
                    tlkapi_printf(APP_FLASH_PROT_LOG_EN, "[FLASH][PROT] flash lock all area!\n");
                }
                else{
                    blc_flashProt.init_err = 1;
                }


                break;
        #endif

        #if (FLASH_GD25LQ16E_SUPPORT_EN) //2M capacity
            case MID1560c8:
                if(app_lock_block == FLASH_LOCK_FW_LOW_256K){
                    flash_lock_block_size = FLASH_LOCK_LOW_256K_MID1560c8;
                    tlkapi_printf(APP_FLASH_PROT_LOG_EN, "[FLASH][PROT] flash lock low 256K block!\n");
                }
                else if(app_lock_block == FLASH_LOCK_FW_LOW_512K){
                    flash_lock_block_size = FLASH_LOCK_LOW_512K_MID1560c8;
                    tlkapi_printf(APP_FLASH_PROT_LOG_EN, "[FLASH][PROT] flash lock low 512K block!\n");
                }
                else if(app_lock_block == FLASH_LOCK_FW_LOW_1M){
                    flash_lock_block_size = FLASH_LOCK_LOW_1M_MID1560c8;
                    tlkapi_printf(APP_FLASH_PROT_LOG_EN, "[FLASH][PROT] flash lock low 1M block!\n");
                }
                else if(app_lock_block == FLASH_LOCK_ALL_AREA){
                    flash_lock_block_size = FLASH_LOCK_ALL_2M_MID1560c8;
                    tlkapi_printf(APP_FLASH_PROT_LOG_EN, "[FLASH][PROT] flash lock all area!\n");
                }
                else{
                    blc_flashProt.init_err = 1;
                }

                break;
        #endif

        default:
            break;
    }

    return flash_lock_block_size;
}




/**
 * @brief       This function serves to set the protection of the flash.
 * @param[in]   flash_lock_cmd - flash lock block command for different flash type
 *              e.g. for P25Q16SU, command is selected from "mid156085_lock_block_e"
 * @return      1: Flash lock succeed; 0: Flash lock fail
 */
bool flash_lock(unsigned int flash_lock_cmd)
{
    if(blc_flashProt.init_err){
        return FALSE;
    }

    u16 cur_lock_status = flash_get_lock_status_mid();

    if(cur_lock_status == flash_lock_cmd){ //lock status is what we want, no need lock again
        return TRUE;
    }
    else{ //unlocked or locked block size is not what we want

        if(cur_lock_status != flash_unlock_status){ //locked block size is not what we want, need unlock first
            for(int i = 0; i < 3; i++){ //Unlock flash up to 3 times to prevent failure.
                flash_unlock_mid();
                cur_lock_status = flash_get_lock_status_mid();

                if(cur_lock_status == flash_unlock_status){ //unlock success
                    break;
                }
            }
        }

        for(int i = 0; i < 3; i++) //Lock flash up to 3 times to prevent failure.
        {
            flash_lock_mid(flash_lock_cmd);
            cur_lock_status = flash_get_lock_status_mid();
            if(cur_lock_status == flash_lock_cmd){  //lock OK
                return TRUE;
            }
        }
    }

    return FALSE;
}

/**
 * @brief       This function serves to release flash protection.
 * @param[in]   none.
 * @return      1: Flash unlock succeed; 0: Flash unlock fail
 */
bool flash_unlock(void)
{
    if(blc_flashProt.init_err){
        return FALSE;
    }

    u16 cur_lock_status = flash_get_lock_status_mid();

    if(cur_lock_status == flash_unlock_status){ //Flash already unlocked
        return TRUE;
    }
    else{ //Flash locked
        for(int i = 0; i < 3; i++){ //Unlock flash up to 3 times to prevent failure.
            flash_unlock_mid();
            cur_lock_status = flash_get_lock_status_mid();

            if(cur_lock_status == flash_unlock_status){ //unlock success
                return TRUE;
            }
        }
    }

    return FALSE;
}


#endif


