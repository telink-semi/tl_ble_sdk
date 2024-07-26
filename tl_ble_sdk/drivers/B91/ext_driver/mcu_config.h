/********************************************************************************************************
 * @file    mcu_config.h
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
#ifndef DRIVERS_B91_EXT_DRIVER_MCU_CONFIG_H_
#define DRIVERS_B91_EXT_DRIVER_MCU_CONFIG_H_


/*
 * attention: user can not change any code in this file,
 *            these are hardware feature !!!
 */



/*
    Flash Type  uid CMD     MID         Company
    P25Q80U     0x4b        0x146085    PUYA
    P25Q16SU    0x4b        0x156085    PUYA
    P25Q32SU    0x4b        0x166085    PUYA
    PY25Q128H   0x4b        0x182085    PUYA
 */
#define FLASH_P25Q80U_SUPPORT_EN                                1
#define FLASH_P25Q16SU_SUPPORT_EN                               1
#define FLASH_P25Q32SU_SUPPORT_EN                               1
#define FLASH_P25Q128H_SUPPORT_EN                               1




/**
 * @brief   Public Device Address Company ID for this chip
 */
#define PDA_COMPANY_ID                                          0xD85F77    //B91



/**
 * @brief   MAC address burning on EFUSE
 */
#define BUILT_IN_MAC_ON_EFUSE                                   0




/**
 * @brief   Hardware secure boot
 *          doing firmware signature verification when MCU boot
 */
#define HARDWARE_SECURE_BOOT_SUPPORT_EN                         0

/**
 * @brief   Hardware firmware encryption
 */
#define HARDWARE_FIRMWARE_ENCRYPTION_SUPPORT_EN                 0

/**
 * @brief   Hardware secure debug
 */
#define HARDWARE_SECURE_DEBUG_SUPPORT_EN                        0



/**
 * @brief   support multiple priority interrupt
 */
#define MCU_SUPPORT_MULTI_PRIORITY_IRQ                          1


#endif /* DRIVERS_B91_EXT_DRIVER_MCU_CONFIG_H_ */
