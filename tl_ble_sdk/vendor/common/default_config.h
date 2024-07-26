/********************************************************************************************************
 * @file    default_config.h
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
#pragma once


#include "config.h"
#include "ext_driver/mcu_config.h"
#include "boards/boards_config.h"

//////////// product  Information  //////////////////////////////
#ifndef ID_VENDOR
#define ID_VENDOR           0x248a
#endif
#ifndef ID_PRODUCT_BASE
#define ID_PRODUCT_BASE     0x8800
#endif
#ifndef STRING_VENDOR
#define STRING_VENDOR       L"Telink"
#endif
#ifndef STRING_PRODUCT
#define STRING_PRODUCT      L"BLE 5.3"
#endif

#ifndef STRING_SERIAL
#define STRING_SERIAL       L"TLSR95XX"
#endif


#ifndef PM_DEEPSLEEP_RETENTION_ENABLE
#define PM_DEEPSLEEP_RETENTION_ENABLE               0
#endif


#ifndef BLC_PM_EN
#define BLC_PM_EN                                   1
#endif

#ifndef BLC_PM_DEEP_RETENTION_MODE_EN
#define BLC_PM_DEEP_RETENTION_MODE_EN               1
#endif



#ifndef BLE_APP_PM_ENABLE
#define BLE_APP_PM_ENABLE                           0
#endif


#if(!BLE_APP_PM_ENABLE && PM_DEEPSLEEP_RETENTION_ENABLE)
    #error "can not use deep retention when PM disable !!!"
#endif


#ifndef OS_SEPARATE_STACK_SPACE
#define OS_SEPARATE_STACK_SPACE                     0
#endif

#ifndef FREERTOS_ENABLE
#define FREERTOS_ENABLE                             0
#endif




#ifndef APP_HW_FIRMWARE_ENCRYPTION_ENABLE
#define APP_HW_FIRMWARE_ENCRYPTION_ENABLE           0   //firmware Encryption
#endif

#ifndef APP_HW_SECURE_BOOT_ENABLE
#define APP_HW_SECURE_BOOT_ENABLE                   0   //secure boot: firmware signature verification
#endif

#ifndef BLE_OTA_SERVER_ENABLE
#define BLE_OTA_SERVER_ENABLE                       0
#endif

#ifndef ACL_PERIPHR_SMP_ENABLE
#define ACL_PERIPHR_SMP_ENABLE                      0
#endif

#ifndef ACL_CENTRAL_SMP_ENABLE
#define ACL_CENTRAL_SMP_ENABLE                      0
#endif

#ifndef BLE_MASTER_SIMPLE_SDP_ENABLE
#define BLE_MASTER_SIMPLE_SDP_ENABLE                0
#endif

#ifndef CENTRAL_CONNECT_PERIPHR_MAC_FILTER_EN
#define CENTRAL_CONNECT_PERIPHR_MAC_FILTER_EN       0
#endif

#ifndef PERIPHR_CONNECT_CENTRAL_MAC_FILTER_EN
#define PERIPHR_CONNECT_CENTRAL_MAC_FILTER_EN       0
#endif

#if (APP_HW_FIRMWARE_ENCRYPTION_ENABLE && !HARDWARE_FIRMWARE_ENCRYPTION_SUPPORT_EN)
    #error "MCU do not support HW FIRMWARE ENCRYPTION"
#endif

#if (APP_HW_SECURE_BOOT_ENABLE && !HARDWARE_SECURE_BOOT_SUPPORT_EN)
    #error "MCU do not support HW SECURE BOOT"
#endif

#ifndef APPLICATION_DONGLE
#define APPLICATION_DONGLE                          0
#endif

#ifndef UART_PRINT_DEBUG_ENABLE
#define UART_PRINT_DEBUG_ENABLE                     0
#endif

#if (TLKAPI_DEBUG_ENABLE && UART_PRINT_DEBUG_ENABLE)
    #error "TLKAPI_DEBUG_ENABLE and UART_PRINT_DEBUG_ENABLE can not be enable at same time !!!"
#endif





#if(APPLICATION_DONGLE)
    #ifndef MODULE_MOUSE_ENABLE
    #define MODULE_MOUSE_ENABLE                     0
    #endif
    #ifndef MODULE_KEYBOARD_ENABLE
    #define MODULE_KEYBOARD_ENABLE                  0
    #endif
    #ifndef MODULE_MIC_ENABLE
    #define MODULE_MIC_ENABLE                       0
    #endif
    #ifndef MODULE_SPEAKER_ENABLE
    #define MODULE_SPEAKER_ENABLE                   0           // device , not dongle
    #endif
    #ifndef MODULE_USB_ENABLE
    #define MODULE_USB_ENABLE                       1
    #endif
#else
    #ifndef MODULE_MOUSE_ENABLE
    #define MODULE_MOUSE_ENABLE                     1
    #endif
    #ifndef MODULE_KEYBOARD_ENABLE
    #define MODULE_KEYBOARD_ENABLE                  1
    #endif

    #ifndef MODULE_MIC_ENABLE
    #define MODULE_MIC_ENABLE                       0
    #endif
    #ifndef MODULE_SPEAKER_ENABLE
    #define MODULE_SPEAKER_ENABLE                   0       // device , not dongle
    #endif

    #ifndef MODULE_USB_ENABLE
    #define MODULE_USB_ENABLE                       0
    #endif
#endif








///////////////////  USB   /////////////////////////////////
#ifndef IRQ_USB_PWDN_ENABLE
#define IRQ_USB_PWDN_ENABLE                         0
#endif

#ifndef MS_OS_DESCRIPTOR_ENABLE
#define MS_OS_DESCRIPTOR_ENABLE                     0
#endif

#ifndef AUDIO_HOGP
#define AUDIO_HOGP                                  0
#endif

#ifndef USB_CDC_ENABLE
#define USB_CDC_ENABLE                              0
#endif

#ifndef USB_PRINTER_ENABLE
#define USB_PRINTER_ENABLE                          0
#endif

#ifndef USB_SPEAKER_ENABLE
#define USB_SPEAKER_ENABLE                          0
#endif

#ifndef USB_MIC_ENABLE
#define USB_MIC_ENABLE                              0
#endif

#ifndef USB_MOUSE_ENABLE
#define USB_MOUSE_ENABLE                            0
#endif

#ifndef USB_KEYBOARD_ENABLE
#define USB_KEYBOARD_ENABLE                         0
#endif

#ifndef USB_SOMATIC_ENABLE
#define USB_SOMATIC_ENABLE                          0
#endif

#ifndef USB_CRC_ENABLE
#define USB_CRC_ENABLE                              0
#endif

#ifndef USB_CUSTOM_HID_REPORT
#define USB_CUSTOM_HID_REPORT                       0
#endif

#ifndef USB_AUDIO_441K_ENABLE
#define USB_AUDIO_441K_ENABLE                       0
#endif

#ifndef DESC_IAD_ENABLE
#define DESC_IAD_ENABLE                             0
#endif

#ifndef USB_MASS_STORAGE_ENABLE
#define USB_MASS_STORAGE_ENABLE                                 0
#endif

#ifndef MIC_CHANNEL_COUNT
#define MIC_CHANNEL_COUNT                                       2
#endif

#ifndef USB_DESCRIPTOR_CONFIGURATION_FOR_KM_DONGLE
#define USB_DESCRIPTOR_CONFIGURATION_FOR_KM_DONGLE              0
#endif

#ifndef USB_ID_AND_STRING_CUSTOM
#define USB_ID_AND_STRING_CUSTOM                                0
#endif

#define KEYBOARD_RESENT_MAX_CNT         3
#define KEYBOARD_REPEAT_CHECK_TIME      300000  // in us    
#define KEYBOARD_REPEAT_INTERVAL        100000  // in us    
#define KEYBOARD_SCAN_INTERVAL          16000   // in us
#define MOUSE_SCAN_INTERVAL             8000    // in us    
#define SOMATIC_SCAN_INTERVAL           8000

#define USB_KEYBOARD_POLL_INTERVAL      10      // in ms    USB_KEYBOARD_POLL_INTERVAL < KEYBOARD_SCAN_INTERVAL to ensure PC no missing key

#ifndef USB_MOUSE_POLL_INTERVAL
#define USB_MOUSE_POLL_INTERVAL         4       // in ms
#endif

#define USB_SOMATIC_POLL_INTERVAL       8       // in ms

#define USB_KEYBOARD_RELEASE_TIMEOUT    (450000) // in us
#define USB_MOUSE_RELEASE_TIMEOUT       (200000) // in us

#ifndef BATT_CHECK_ENABLE
#define BATT_CHECK_ENABLE                               0
#endif


///////////////////  FLASH   /////////////////////////////////
#ifndef FLASH_4LINE_MODE_ENABLE
#define FLASH_4LINE_MODE_ENABLE                    0
#endif

#ifndef APP_FLASH_PROTECTION_ENABLE
#define APP_FLASH_PROTECTION_ENABLE                     0
#endif


///////////////////  Board configuration   /////////////////////////////////

#ifndef BOARD_SELECT

    #if (MCU_CORE_TYPE == MCU_CORE_B91)
        #define BOARD_SELECT                                BOARD_951X_EVK_C1T213A20
    #elif (MCU_CORE_TYPE == MCU_CORE_TL721X)
        #define BOARD_SELECT                                BOARD_721X_EVK_C1T315A20
    #elif (MCU_CORE_TYPE == MCU_CORE_TL321X)
        #define BOARD_SELECT                                BOARD_321X_EVK_C1T331A20  //BOARD_321X_EVK_C1T335A20
    #else
        #error "SDK do not support this MCU!"
    #endif
#endif

#if ( BOARD_SELECT == BOARD_951X_EVK_C1T213A20)
    #include "boards/B91_C1T213A20.h"
#elif ( BOARD_SELECT == BOARD_721X_EVK_C1T315A20)
    #include "boards/TL721X_C1T315A20.h"
#elif ( BOARD_SELECT == BOARD_321X_EVK_C1T331A20)
    #include "boards/TL321X_C1T331A20.h"
#elif ( BOARD_SELECT == BOARD_321X_EVK_C1T335A20)
    #include "boards/TL321X_C1T335A20.h"
#endif


#ifndef UI_KEYBOARD_ENABLE
#define UI_KEYBOARD_ENABLE                              0
#endif

#ifndef LED_ON_LEVEL
#define LED_ON_LEVEL                                    1
#endif






#ifndef DEBUG_GPIO_ENABLE
#define DEBUG_GPIO_ENABLE                               0
#endif





#if (DEBUG_GPIO_ENABLE)
    #ifdef GPIO_CHN0
        #define DBG_CHN0_LOW        gpio_write(GPIO_CHN0, 0)
        #define DBG_CHN0_HIGH       gpio_write(GPIO_CHN0, 1)
        #define DBG_CHN0_TOGGLE     gpio_toggle(GPIO_CHN0)
    #else
        #define DBG_CHN0_LOW
        #define DBG_CHN0_HIGH
        #define DBG_CHN0_TOGGLE
    #endif

    #ifdef  GPIO_CHN1
        #define DBG_CHN1_LOW        gpio_write(GPIO_CHN1, 0)
        #define DBG_CHN1_HIGH       gpio_write(GPIO_CHN1, 1)
        #define DBG_CHN1_TOGGLE     gpio_toggle(GPIO_CHN1)
    #else
        #define DBG_CHN1_LOW
        #define DBG_CHN1_HIGH
        #define DBG_CHN1_TOGGLE
    #endif

    #ifdef  GPIO_CHN2
        #define DBG_CHN2_LOW        gpio_write(GPIO_CHN2, 0)
        #define DBG_CHN2_HIGH       gpio_write(GPIO_CHN2, 1)
        #define DBG_CHN2_TOGGLE     gpio_toggle(GPIO_CHN2)
    #else
        #define DBG_CHN2_LOW
        #define DBG_CHN2_HIGH
        #define DBG_CHN2_TOGGLE
    #endif

    #ifdef  GPIO_CHN3
        #define DBG_CHN3_LOW        gpio_write(GPIO_CHN3, 0)
        #define DBG_CHN3_HIGH       gpio_write(GPIO_CHN3, 1)
        #define DBG_CHN3_TOGGLE     gpio_toggle(GPIO_CHN3)
    #else
        #define DBG_CHN3_LOW
        #define DBG_CHN3_HIGH
        #define DBG_CHN3_TOGGLE
    #endif

    #ifdef GPIO_CHN4
        #define DBG_CHN4_LOW        gpio_write(GPIO_CHN4, 0)
        #define DBG_CHN4_HIGH       gpio_write(GPIO_CHN4, 1)
        #define DBG_CHN4_TOGGLE     gpio_toggle(GPIO_CHN4)
    #else
        #define DBG_CHN4_LOW
        #define DBG_CHN4_HIGH
        #define DBG_CHN4_TOGGLE
    #endif

    #ifdef  GPIO_CHN5
        #define DBG_CHN5_LOW        gpio_write(GPIO_CHN5, 0)
        #define DBG_CHN5_HIGH       gpio_write(GPIO_CHN5, 1)
        #define DBG_CHN5_TOGGLE     gpio_toggle(GPIO_CHN5)
    #else
        #define DBG_CHN5_LOW
        #define DBG_CHN5_HIGH
        #define DBG_CHN5_TOGGLE
    #endif

    #ifdef  GPIO_CHN6
        #define DBG_CHN6_LOW        gpio_write(GPIO_CHN6, 0)
        #define DBG_CHN6_HIGH       gpio_write(GPIO_CHN6, 1)
        #define DBG_CHN6_TOGGLE     gpio_toggle(GPIO_CHN6)
    #else
        #define DBG_CHN6_LOW
        #define DBG_CHN6_HIGH
        #define DBG_CHN6_TOGGLE
    #endif

    #ifdef  GPIO_CHN7
        #define DBG_CHN7_LOW        gpio_write(GPIO_CHN7, 0)
        #define DBG_CHN7_HIGH       gpio_write(GPIO_CHN7, 1)
        #define DBG_CHN7_TOGGLE     gpio_toggle(GPIO_CHN7)
    #else
        #define DBG_CHN7_LOW
        #define DBG_CHN7_HIGH
        #define DBG_CHN7_TOGGLE
    #endif

    #ifdef GPIO_CHN8
        #define DBG_CHN8_LOW        gpio_write(GPIO_CHN8, 0)
        #define DBG_CHN8_HIGH       gpio_write(GPIO_CHN8, 1)
        #define DBG_CHN8_TOGGLE     gpio_toggle(GPIO_CHN8)
    #else
        #define DBG_CHN8_LOW
        #define DBG_CHN8_HIGH
        #define DBG_CHN8_TOGGLE
    #endif

    #ifdef  GPIO_CHN9
        #define DBG_CHN9_LOW        gpio_write(GPIO_CHN9, 0)
        #define DBG_CHN9_HIGH       gpio_write(GPIO_CHN9, 1)
        #define DBG_CHN9_TOGGLE     gpio_toggle(GPIO_CHN9)
    #else
        #define DBG_CHN9_LOW
        #define DBG_CHN9_HIGH
        #define DBG_CHN9_TOGGLE
    #endif

    #ifdef  GPIO_CHN10
        #define DBG_CHN10_LOW       gpio_write(GPIO_CHN10, 0)
        #define DBG_CHN10_HIGH      gpio_write(GPIO_CHN10, 1)
        #define DBG_CHN10_TOGGLE    gpio_toggle(GPIO_CHN10)
    #else
        #define DBG_CHN10_LOW
        #define DBG_CHN10_HIGH
        #define DBG_CHN10_TOGGLE
    #endif

    #ifdef  GPIO_CHN11
        #define DBG_CHN11_LOW       gpio_write(GPIO_CHN11, 0)
        #define DBG_CHN11_HIGH      gpio_write(GPIO_CHN11, 1)
        #define DBG_CHN11_TOGGLE    gpio_toggle(GPIO_CHN11)
    #else
        #define DBG_CHN11_LOW
        #define DBG_CHN11_HIGH
        #define DBG_CHN11_TOGGLE
    #endif

    #ifdef GPIO_CHN12
        #define DBG_CHN12_LOW       gpio_write(GPIO_CHN12, 0)
        #define DBG_CHN12_HIGH      gpio_write(GPIO_CHN12, 1)
        #define DBG_CHN12_TOGGLE    gpio_toggle(GPIO_CHN12)
    #else
        #define DBG_CHN12_LOW
        #define DBG_CHN12_HIGH
        #define DBG_CHN12_TOGGLE
    #endif

    #ifdef  GPIO_CHN13
        #define DBG_CHN13_LOW       gpio_write(GPIO_CHN13, 0)
        #define DBG_CHN13_HIGH      gpio_write(GPIO_CHN13, 1)
        #define DBG_CHN13_TOGGLE    gpio_toggle(GPIO_CHN13)
    #else
        #define DBG_CHN13_LOW
        #define DBG_CHN13_HIGH
        #define DBG_CHN13_TOGGLE
    #endif

    #ifdef  GPIO_CHN14
        #define DBG_CHN14_LOW       gpio_write(GPIO_CHN14, 0)
        #define DBG_CHN14_HIGH      gpio_write(GPIO_CHN14, 1)
        #define DBG_CHN14_TOGGLE    gpio_toggle(GPIO_CHN14)
    #else
        #define DBG_CHN14_LOW
        #define DBG_CHN14_HIGH
        #define DBG_CHN14_TOGGLE
    #endif

    #ifdef  GPIO_CHN15
        #define DBG_CHN15_LOW       gpio_write(GPIO_CHN15, 0)
        #define DBG_CHN15_HIGH      gpio_write(GPIO_CHN15, 1)
        #define DBG_CHN15_TOGGLE    gpio_toggle(GPIO_CHN15)
    #else
        #define DBG_CHN15_LOW
        #define DBG_CHN15_HIGH
        #define DBG_CHN15_TOGGLE
    #endif
#endif


#ifndef DBG_CHN0_LOW
#define DBG_CHN0_LOW
#endif

#ifndef DBG_CHN0_HIGH
#define DBG_CHN0_HIGH
#endif

#ifndef DBG_CHN0_TOGGLE
#define DBG_CHN0_TOGGLE
#endif


#ifndef DBG_CHN1_LOW
#define DBG_CHN1_LOW
#endif

#ifndef DBG_CHN1_HIGH
#define DBG_CHN1_HIGH
#endif

#ifndef DBG_CHN1_TOGGLE
#define DBG_CHN1_TOGGLE
#endif


#ifndef DBG_CHN2_LOW
#define DBG_CHN2_LOW
#endif

#ifndef DBG_CHN2_HIGH
#define DBG_CHN2_HIGH
#endif

#ifndef DBG_CHN2_TOGGLE
#define DBG_CHN2_TOGGLE
#endif


#ifndef DBG_CHN3_LOW
#define DBG_CHN3_LOW
#endif

#ifndef DBG_CHN3_HIGH
#define DBG_CHN3_HIGH
#endif

#ifndef DBG_CHN3_TOGGLE
#define DBG_CHN3_TOGGLE
#endif


#ifndef DBG_CHN4_LOW
#define DBG_CHN4_LOW
#endif

#ifndef DBG_CHN4_HIGH
#define DBG_CHN4_HIGH
#endif

#ifndef DBG_CHN4_TOGGLE
#define DBG_CHN4_TOGGLE
#endif


#ifndef DBG_CHN5_LOW
#define DBG_CHN5_LOW
#endif

#ifndef DBG_CHN5_HIGH
#define DBG_CHN5_HIGH
#endif

#ifndef DBG_CHN5_TOGGLE
#define DBG_CHN5_TOGGLE
#endif


#ifndef DBG_CHN6_LOW
#define DBG_CHN6_LOW
#endif

#ifndef DBG_CHN6_HIGH
#define DBG_CHN6_HIGH
#endif

#ifndef DBG_CHN6_TOGGLE
#define DBG_CHN6_TOGGLE
#endif


#ifndef DBG_CHN7_LOW
#define DBG_CHN7_LOW
#endif

#ifndef DBG_CHN7_HIGH
#define DBG_CHN7_HIGH
#endif

#ifndef DBG_CHN7_TOGGLE
#define DBG_CHN7_TOGGLE
#endif


#ifndef DBG_CHN8_LOW
#define DBG_CHN8_LOW
#endif

#ifndef DBG_CHN8_HIGH
#define DBG_CHN8_HIGH
#endif

#ifndef DBG_CHN8_TOGGLE
#define DBG_CHN8_TOGGLE
#endif


#ifndef DBG_CHN9_LOW
#define DBG_CHN9_LOW
#endif

#ifndef DBG_CHN9_HIGH
#define DBG_CHN9_HIGH
#endif

#ifndef DBG_CHN9_TOGGLE
#define DBG_CHN9_TOGGLE
#endif


#ifndef DBG_CHN10_LOW
#define DBG_CHN10_LOW
#endif

#ifndef DBG_CHN10_HIGH
#define DBG_CHN10_HIGH
#endif

#ifndef DBG_CHN10_TOGGLE
#define DBG_CHN10_TOGGLE
#endif



#ifndef DBG_CHN11_LOW
#define DBG_CHN11_LOW
#endif

#ifndef DBG_CHN11_HIGH
#define DBG_CHN11_HIGH
#endif

#ifndef DBG_CHN11_TOGGLE
#define DBG_CHN11_TOGGLE
#endif


#ifndef DBG_CHN12_LOW
#define DBG_CHN12_LOW
#endif

#ifndef DBG_CHN12_HIGH
#define DBG_CHN12_HIGH
#endif

#ifndef DBG_CHN12_TOGGLE
#define DBG_CHN12_TOGGLE
#endif


#ifndef DBG_CHN13_LOW
#define DBG_CHN13_LOW
#endif

#ifndef DBG_CHN13_HIGH
#define DBG_CHN13_HIGH
#endif

#ifndef DBG_CHN13_TOGGLE
#define DBG_CHN13_TOGGLE
#endif


#ifndef DBG_CHN14_LOW
#define DBG_CHN14_LOW
#endif

#ifndef DBG_CHN14_HIGH
#define DBG_CHN14_HIGH
#endif

#ifndef DBG_CHN14_TOGGLE
#define DBG_CHN14_TOGGLE
#endif


#ifndef DBG_CHN15_LOW
#define DBG_CHN15_LOW
#endif

#ifndef DBG_CHN15_HIGH
#define DBG_CHN15_HIGH
#endif

#ifndef DBG_CHN15_TOGGLE
#define DBG_CHN15_TOGGLE
#endif




#ifndef APP_DBG_CHN_0_LOW
#define APP_DBG_CHN_0_LOW
#endif
#ifndef APP_DBG_CHN_0_HIGH
#define APP_DBG_CHN_0_HIGH
#endif

#ifndef APP_DBG_CHN_1_LOW
#define APP_DBG_CHN_1_LOW
#endif
#ifndef APP_DBG_CHN_1_HIGH
#define APP_DBG_CHN_1_HIGH
#endif


#ifndef APP_DBG_CHN_2_LOW
#define APP_DBG_CHN_2_LOW
#endif
#ifndef APP_DBG_CHN_2_HIGH
#define APP_DBG_CHN_2_HIGH
#endif

#ifndef APP_DBG_CHN_3_LOW
#define APP_DBG_CHN_3_LOW
#endif
#ifndef APP_DBG_CHN_3_HIGH
#define APP_DBG_CHN_3_HIGH
#endif

#ifndef APP_DBG_CHN_4_LOW
#define APP_DBG_CHN_4_LOW
#endif
#ifndef APP_DBG_CHN_4_HIGH
#define APP_DBG_CHN_4_HIGH
#endif

#ifndef APP_DBG_CHN_5_LOW
#define APP_DBG_CHN_5_LOW
#endif
#ifndef APP_DBG_CHN_5_HIGH
#define APP_DBG_CHN_5_HIGH
#endif

#ifndef APP_DBG_CHN_6_LOW
#define APP_DBG_CHN_6_LOW
#endif
#ifndef APP_DBG_CHN_6_HIGH
#define APP_DBG_CHN_6_HIGH
#endif

#ifndef APP_DBG_CHN_7_LOW
#define APP_DBG_CHN_7_LOW
#endif
#ifndef APP_DBG_CHN_7_HIGH
#define APP_DBG_CHN_7_HIGH
#endif

#ifndef APP_DBG_CHN_8_LOW
#define APP_DBG_CHN_8_LOW
#endif
#ifndef APP_DBG_CHN_8_HIGH
#define APP_DBG_CHN_8_HIGH
#endif

#ifndef APP_DBG_CHN_9_LOW
#define APP_DBG_CHN_9_LOW
#endif
#ifndef APP_DBG_CHN_9_HIGH
#define APP_DBG_CHN_9_HIGH
#endif

#ifndef APP_DBG_CHN_10_LOW
#define APP_DBG_CHN_10_LOW
#endif
#ifndef APP_DBG_CHN_10_HIGH
#define APP_DBG_CHN_10_HIGH
#endif

#ifndef APP_DBG_CHN_11_LOW
#define APP_DBG_CHN_11_LOW
#endif
#ifndef APP_DBG_CHN_11_HIGH
#define APP_DBG_CHN_11_HIGH
#endif

#ifndef APP_DBG_CHN_12_LOW
#define APP_DBG_CHN_12_LOW
#endif
#ifndef APP_DBG_CHN_12_HIGH
#define APP_DBG_CHN_12_HIGH
#endif

#ifndef APP_DBG_CHN_13_LOW
#define APP_DBG_CHN_13_LOW
#endif
#ifndef APP_DBG_CHN_13_HIGH
#define APP_DBG_CHN_13_HIGH
#endif

#ifndef APP_DBG_CHN_14_LOW
#define APP_DBG_CHN_14_LOW
#endif
#ifndef APP_DBG_CHN_14_HIGH
#define APP_DBG_CHN_14_HIGH
#endif

#ifndef APP_DBG_CHN_15_LOW
#define APP_DBG_CHN_15_LOW
#endif
#ifndef APP_DBG_CHN_15_HIGH
#define APP_DBG_CHN_15_HIGH
#endif
