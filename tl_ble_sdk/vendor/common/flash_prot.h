/********************************************************************************************************
 * @file    flash_prot.h
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
#ifndef VENDOR_COMMON_FLASH_PROT_H_
#define VENDOR_COMMON_FLASH_PROT_H_


/**
 * @brief    Application Flash protection enable control. Default disable, use can enable it in app_conifg.h.
 *             User must enable Flash protection on mass production application !!!
 */
#ifndef APP_FLASH_PROTECTION_ENABLE
#define APP_FLASH_PROTECTION_ENABLE                    0   //enable or disable
#endif


/**
 * @brief    Application Flash protection log enable or not. Default disable, use can enable it in app_conifg.h.
 */
#ifndef APP_FLASH_PROT_LOG_EN
#define APP_FLASH_PROT_LOG_EN                        0
#endif


/**
 * @brief    application lock block definition
 *   Each kind of IC have multiple flash supported.
 *   Different flash, especially form different vendor, do not use all same lock block size.
 *   If user want protect certain range of flash, they must consider different kind of flash supported lock block size.
 *   Here, we just put a few of block size to show how to use flash protection.
 *   If user want use more specific flash protection area, they should add by themselves.
 */
typedef enum{
    /* lock low 256K for firmware, do not consider system data and user data.
     * Flash size is at least 1M in this SDK, system data is stored above 256K. User data must stored above 256K too.
     * If using Telink OTA with multiple hardware boot address, low 256K area is used for both raw firmware and new firmware storage.
     * If user data stored below 256K, please do not use reference code, but design and implement by themselves. */
    FLASH_LOCK_FW_LOW_256K        =    1,

    /* lock low 512K for firmware, do not consider system data and user data.
     * Flash size is at least 1M in this SDK, system data is stored above 512K. User data must stored above 512K too.
     * If using Telink OTA with multiple hardware boot address, low 512K area is used for both raw firmware and new firmware storage.
     * If user data stored below 512K, please do not use reference code, but design and implement by themselves. */
    FLASH_LOCK_FW_LOW_512K        =    2,


    /* lock low 1M for firmware, do not consider system data and user data.
     * 1. For 1M capacity flash
     *         can not lock 1M all, should leave some upper 64K for system data and user data.
     *        System data is stored above 960K(0x1F0000). User data must stored above 960K too. Then we can choose lock 960K.
     *        If user data stored below 960K, please do not use reference code, but design and implement by themselves.
     * 2. For Flash capacity bigger than 1M
     *         System data is stored above 1M. User data must stored above 1M too.
     *      If using Telink OTA with multiple hardware boot address, low 1M area is used for both raw firmware and new firmware storage.
     *      If user data stored below 1M, please do not use reference code, but design and implement by themselves. */
    FLASH_LOCK_FW_LOW_1M        =   3,

    /* User can not use "FLASH_LOCK_FW_LOW_2M" or "FLASH_LOCK_FW_LOW_4M"
     * unless they can guarantee that they have no IC with 1M Flash internal but only with 2M Flash */

    /* lock all Flash area, even system data and user data.
     * Attention: More conditions need to be considered, such as system data(SMP pairing information or OTA data if OTA used)*/
    FLASH_LOCK_ALL_AREA            =   4,
}flash_app_lock_e;



/**
 * @brief    The structure of some initialization error about flash protect.
 */
typedef struct{
    u8    init_err;

}flash_prot_t;
extern flash_prot_t    blc_flashProt;



/**
 * @brief        this function serves to  locks a specific region or address range in the flash memory.
 * @param[in]    none
 * @return      none
 */
typedef unsigned char  (*flash_lock_t)(unsigned int);
/**
 * @brief        this function serves to unlocks a specific region or address range in the flash memory.
 * @param[in]    none
 * @return      none
 */
typedef unsigned char  (*flash_unlock_t)(void);
/**
 * @brief        this function that retrieves the lock status of a specific region or address range in the flash memory.
 * @param[in]    none
 * @return      none
 */
typedef unsigned short  (*flash_get_lock_status_t)(void);
/**
 * @brief        this function as a callback function for flash protection operations.
 * @param[in]    none
 * @return      none
 */
typedef void  (*flash_prot_op_callback_t)(unsigned char, unsigned int, unsigned int);
extern    flash_prot_op_callback_t         flash_prot_op_cb;






/*
 * @brief flash operation event
 *    Here just list all type appeared in SDK sample, including APP initialization and OTA/SMP operating flash
 *    If you have more flash write or erase action for your application, should add more type and process them
 *
 */

/* application layer event, initialization, lock flash */
#define FLASH_OP_EVT_APP_INITIALIZATION                                 1



/*
 * @brief OTA operate Flash
 *
 * If using Telink OTA, 2 type of Flash operation must be processed as below:
 *     OTA operate Flash type 1: clear old firmware area(erase Flash), backup for storing new OTA data in future.
 *     OTA operate Flash type 2: write new firmware data(write Flash) for OTA ongoing
 *
 * If user design and implement OTA with proprietary mechanism, Please refer to Telink OTA.
 *        Sample code in BLE stack below, taking OTA writing new firmware for example:
 *                           if(flash_prot_op_cb){
 *                        flash_prot_op_cb(FLASH_OP_EVT_STACK_OTA_WRITE_NEW_FW_BEGIN, address_start, address_end);
 *                    }
 *
 *                           if(flash_prot_op_cb){
 *                        flash_prot_op_cb(FLASH_OP_EVT_STACK_OTA_WRITE_NEW_FW_END, address_start, address_end);
 *                    }
 */

/* stack layer event, OTA initialization, clear old firmware begin, may need unlock flash */
#define FLASH_OP_EVT_STACK_OTA_CLEAR_OLD_FW_BEGIN                    10
/* stack layer event, OTA initialization, clear old firmware end , may need restore locking flash */
#define FLASH_OP_EVT_STACK_OTA_CLEAR_OLD_FW_END                        11



/* stack layer event, OTA initialization, write new firmware begin, may need unlock flash */
#define FLASH_OP_EVT_STACK_OTA_WRITE_NEW_FW_BEGIN                    12
/* stack layer event, OTA initialization, write new firmware begin, may need restore locking flash */
#define FLASH_OP_EVT_STACK_OTA_WRITE_NEW_FW_END                        13



/*
 * @brief SMP operate Flash
 *
 * If user lock firmware area only, system data and user data not considered, then SMP operating Flash not involved.
 * If user lock all Flash area include system data and user data not considered, then SMP operating Flash must be processed correctly.
 *
 * SMP operate Flash type 1: select paring area when first power on, write a mark on Flash
 * SMP operate Flash type 2: save pairing information when paired with peer device success
 * SMP operate Flash type 3: delete pairing information of one paired peer device
 *                              maybe triggered by stack internal,
 *                              maybe triggered by user API "blc smp_eraseAllBondingInfo" or "blc smp_deleteBondingSlaveInfo_by_PeerMacAddress"
 * SMP operate Flash type 4: switch pairing information area between normal area and backup area,
 *                              need erase old area data and write valid pairing information to new area
 */

/* stack layer event, SMP select pairing information area begin(write a special mark), may need unlock flash */
#define FLASH_OP_EVT_STACK_SMP_SELECT_PAIRING_INFO_AREA_BEGIN        20
/* stack layer event, SMP select pairing information area begin , may need restore locking flash */
#define FLASH_OP_EVT_STACK_SMP_SELECT_PAIRING_INFO_AREA_END         21



/* stack layer event, SMP save pairing information begin, may need unlock flash */
#define FLASH_OP_EVT_STACK_SMP_SAVE_PAIRING_INFO_BEGIN                22
/* stack layer event, SMP save pairing information end , may need restore locking flash */
#define FLASH_OP_EVT_STACK_SMP_SAVE_PAIRING_INFO_END                23


/* stack layer event, SMP delete pairing information begin, may need unlock flash */
#define FLASH_OP_EVT_STACK_SMP_DELETE_DEVICE_BEGIN                    24
/* stack layer event, SMP delete pairing information end , may need restore locking flash */
#define FLASH_OP_EVT_STACK_SMP_DELETE_DEVICE_END                    25


/* stack layer event, SMP switch pairing information area begin, may need unlock flash */
#define FLASH_OP_EVT_STACK_SMP_SWITCH_PAIRING_INFO_BEGIN            26
/* stack layer event, SMP switch pairing information area end , may need restore locking flash */
#define FLASH_OP_EVT_STACK_SMP_SWITCH_PAIRING_INFO_END                27











/**
 * @brief      this function is used to initialize flash protection.
 * @param[in]  none
 * @return     none
 */
void flash_protection_init(void);


/**
 * @brief      this function is used to register callback for stack, when OTA write or erase flash, send event to application layer
 * @param[in]  cb - flash protection operation callback function
 * @return     none
 */
void blc_appRegisterStackFlashOperationCallback(flash_prot_op_callback_t cb);

/**
 * @brief         This function serves to set the protection of the flash.
 * @param[in]   flash_lock_cmd - flash lock block command for different flash type
 *                 e.g. for P25Q16SU, command is selected from "mid156085_lock_block_e"
 * @return         1: Flash lock succeed; 0: Flash lock fail
 */
bool flash_lock(unsigned int flash_lock_cmd);

/**
 * @brief         This function serves to release flash protection.
 * @param[in]   none.
 * @return         none.
 */
bool flash_unlock(void);

/**
 * @brief      this function is used to change application lock block definition to flash lock block definition according to flash type used
 *                attention: only processed a few lock block size for SDK demo, if you have more lock block size, please
 *                           add more code by yourself
 * @param[in]  app_lock_block - application lock block definition
 * @return     flash_lock_block_size - The size of the lock block size of flash.
 */
u16 flash_change_app_lock_block_to_flash_lock_block(flash_app_lock_e app_lock_block);

#endif /* VENDOR_COMMON_FLASH_PROT_H_ */
