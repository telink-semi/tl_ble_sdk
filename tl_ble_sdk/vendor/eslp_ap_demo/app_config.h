/********************************************************************************************************
 * @file    app_config.h
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

#define HW_EVK                                      1
#define HW_DONGLE                                   2

#define HARDWARE_BOARD_SELECT                       HW_EVK

#define ACL_CENTRAL_MAX_NUM                         1 // ACL central maximum number
#define ACL_PERIPHR_MAX_NUM                         0 // ACL peripheral maximum number

#define ESLP_AP_MAX_GROUPS                          4
#define ESLP_AP_ESL_RECORDS                         64

#define APP_PARSE_CHAR_UART                         1
#define APP_PARSE_CHAR_USB_CDC                      2

#if (MCU_CORE_TYPE == MCU_CORE_B91)
    #define BOARD_SELECT                                BOARD_951X_EVK_C1T213A20
    #define APP_PARSE_CHAR_IFACE                        APP_PARSE_CHAR_UART
#elif (MCU_CORE_TYPE == MCU_CORE_B92)
    #define BOARD_SELECT                                BOARD_952X_EVK_C1T266A20
    #define APP_PARSE_CHAR_IFACE                        APP_PARSE_CHAR_UART
#elif (MCU_CORE_TYPE == MCU_CORE_TL721X)
    #define BOARD_SELECT                                BOARD_721X_EVK_C1T315A20
    #define APP_PARSE_CHAR_IFACE                        APP_PARSE_CHAR_UART
#elif (MCU_CORE_TYPE == MCU_CORE_TL321X)
    #define BOARD_SELECT                                BOARD_321X_EVK_C1T331A20
    #define APP_PARSE_CHAR_IFACE                        APP_PARSE_CHAR_UART
#else
    #error "Please set HARDWARE_BOARD_SELECT in app_config.h"
#endif

#define BLE_OTA_CLIENT_ENABLE                       1
#if (BLE_OTA_CLIENT_ENABLE)
    #define OTA_LEGACY_PROTOCOL                             0  //0: OTA extended protocol; 1: OTA legacy protocol
    #define OTA_CLIENT_SUPPORT_BIG_PDU_ENABLE               0
    #define OTA_CLIENT_SEND_SECURE_BOOT_SIGNATURE_ENABLE    0
    #define OTA_SECURE_BOOT_DESCRIPTOR_SIZE                 0x2000
#endif

#if(APP_PARSE_CHAR_IFACE == APP_PARSE_CHAR_UART)
    #define HCI_UART_EXT_DRIVER_EN                      1
#endif  //#if(APP_PARSE_CHAR_IFACE == APP_PARSE_CHAR_UART)


///////////////////////// Feature Configuration////////////////////////////////////////////////
#define ACL_PERIPHR_SMP_ENABLE                      0   //1 for smp,  0 no security
#define ACL_CENTRAL_SMP_ENABLE                      1   //1 for smp,  0 no security
#define ACL_CENTRAL_SIMPLE_SDP_ENABLE               0   //simple service discovery for ACL central

#define BLE_APP_PM_ENABLE                           0

#define BATT_CHECK_ENABLE             0


/* Flash Protection:
 * 1. Flash protection is enabled by default in SDK. User must enable this function on their final mass production application.
 * 2. User should use "Unlock" command in Telink BDT tool for Flash access during development and debugging phase.
 * 3. Flash protection demonstration in SDK is a reference design based on sample code. Considering that user's final application may
 *    different from sample code, for example, user's final firmware size is bigger, or user have a different OTA design, or user need
 *    store more data in some other area of Flash, all these differences imply that Flash protection reference design in SDK can not
 *    be directly used on user's mass production application without any change. User should refer to sample code, understand the
 *    principles and methods, then change and implement a more appropriate mechanism according to their application if needed.
 */
#define APP_FLASH_PROTECTION_ENABLE 1
///////////////////////// ! OS settings////////////////////////////////////////////////
#define FREERTOS_ENABLE                             0
#define MODULE_USB_ENABLE                           1
#define USB_CDC_ENABLE                              1

///////////////////////// UI Configuration ////////////////////////////////////////////////////
#define UI_LED_ENABLE                               1
#define UI_KEYBOARD_ENABLE                          0

///////////////////////// DEBUG  Configuration ////////////////////////////////////////////////
#define DEBUG_GPIO_ENABLE                           0

#define TLKAPI_DEBUG_ENABLE                         1
#if (HARDWARE_BOARD_SELECT == HW_DONGLE)
#define TLKAPI_DEBUG_CHANNEL                        TLKAPI_DEBUG_CHANNEL_GSUART
#else
#define TLKAPI_DEBUG_CHANNEL                        TLKAPI_DEBUG_CHANNEL_GSUART
#endif

#define APP_LOG_EN                                  1
#define APP_CONTR_EVT_LOG_EN                        1   //controller event
#define APP_HOST_EVT_LOG_EN                         1
#define APP_SMP_LOG_EN                              0
#define APP_SIMPLE_SDP_LOG_EN                       0
#define APP_PAIR_LOG_EN                             1
#define APP_KEY_LOG_EN                              1



#if (MCU_CORE_TYPE == MCU_CORE_B91)
    #define PARSE_CHAR_UART_TX_PIN         UART1_TX_PC6
    #define PARSE_CHAR_UART_RX_PIN         UART1_RX_PC7
    #define PARSE_CHAR_UART_BAUDRATE       1000000
#elif (MCU_CORE_TYPE == MCU_CORE_B92)
    #define PARSE_CHAR_UART_TX_PIN         GPIO_PC6
    #define PARSE_CHAR_UART_RX_PIN         GPIO_PC7
    #define PARSE_CHAR_UART_BAUDRATE       1000000
#elif (MCU_CORE_TYPE == MCU_CORE_TL721X)
    #define PARSE_CHAR_UART_TX_PIN         GPIO_PB6
    #define PARSE_CHAR_UART_RX_PIN         GPIO_PB5
    #define PARSE_CHAR_UART_BAUDRATE       1000000
#elif (MCU_CORE_TYPE == MCU_CORE_TL321X)
    #define PARSE_CHAR_UART_TX_PIN         GPIO_PC6
    #define PARSE_CHAR_UART_RX_PIN         GPIO_PC7
    #define PARSE_CHAR_UART_BAUDRATE       1000000
#endif

///////////////////////// OS settings /////////////////////////////////////////////////////////
#define OS_SEPARATE_STACK_SPACE                     1   //Separate the task stack and interrupt stack space
#define configTOTAL_HEAP_SIZE                       (24*1024)
#define configISR_PLIC_STACK_SIZE                   2048

#if FREERTOS_ENABLE
    #define traceAPP_LED_Task_Toggle()  //gpio_toggle(GPIO_CH01);
    #define traceAPP_BLE_Task_BEGIN()   //gpio_write(GPIO_CH02,1);
    #define traceAPP_BLE_Task_END()     //gpio_write(GPIO_CH02,0);
    #define traceAPP_KEY_Task_BEGIN()   //gpio_write(GPIO_CH03,1);
    #define traceAPP_KEY_Task_END()     //gpio_write(GPIO_CH03,0);
    #define traceAPP_BAT_Task_BEGIN()   //gpio_write(GPIO_CH04,1);
    #define traceAPP_BAT_Task_END()     //gpio_write(GPIO_CH04,0);

    #define traceAPP_MUTEX_Task_BEGIN() //gpio_write(GPIO_CH05,1);
    #define traceAPP_MUTEX_Task_END()   //gpio_write(GPIO_CH05,0);

    #define tracePort_IrqHandler_BEGIN() //gpio_write(GPIO_CH06,1);
    #define tracePort_IrqHandler_END()   //gpio_write(GPIO_CH06,0);

#endif

#include "../common/default_config.h"
