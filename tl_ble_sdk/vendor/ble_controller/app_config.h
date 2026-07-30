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
#include "bqb_config.h"

#define HCI_TR_EN 1
#if HCI_TR_EN
    /*! HCI UART transport pin define */
    #if (MCU_CORE_TYPE == MCU_CORE_B91)
        #define  EXT_HCI_UART_CHANNEL      UART0
        #define  EXT_HCI_UART_IRQ          IRQ_UART0
        #define HCI_TR_RX_PIN              UART0_RX_PD3 //UART0_RX_PB3 //--->EBQ TX
        #define HCI_TR_TX_PIN              UART0_TX_PD2 //UART0_TX_PB2 //--->EBQ RX
        #define HCI_TR_BAUDRATE            (1000000)
        #define HCI_UART_SoftwareRxDone_EN 0
        /*** RTS/CTS Pin ***/
        #if (HCI_UART_SoftwareRxDone_EN)
            #define HCI_TR_RTS_PIN UART0_RTS_PD1
            #define HCI_TR_CTS_PIN UART0_CTS_PD0
        #endif
    #elif (MCU_CORE_TYPE == MCU_CORE_B92)
        #define HCI_TR_RX_PIN   GPIO_FC_PC6
        #define HCI_TR_TX_PIN   GPIO_FC_PC7
        #define HCI_TR_BAUDRATE (1000000)
    #elif (MCU_CORE_TYPE == MCU_CORE_TL721X)
        #define HCI_TR_RX_PIN   GPIO_FC_PB4
        #define HCI_TR_TX_PIN   GPIO_FC_PB5
        #define HCI_TR_BAUDRATE (1000000)
    #elif (MCU_CORE_TYPE == MCU_CORE_TL321X)
        #define HCI_TR_RX_PIN   GPIO_FC_PC4
        #define HCI_TR_TX_PIN   GPIO_FC_PC5
        #define HCI_TR_BAUDRATE (1000000)
    #elif (MCU_CORE_TYPE == MCU_CORE_TL322X)
        #define HCI_TR_RX_PIN              GPIO_FC_PD6 //UART0_RX_PB3 //--->EBQ TX
        #define HCI_TR_TX_PIN              GPIO_FC_PD7 //UART0_TX_PB2 //--->EBQ RX
        #define HCI_TR_BAUDRATE (1000000)
    #elif (MCU_CORE_TYPE == MCU_CORE_TL521X)
        #define HCI_TR_RX_PIN              GPIO_FC_PC4
        #define HCI_TR_TX_PIN              GPIO_FC_PC5
        #define HCI_TR_BAUDRATE (1000000)
    #elif (MCU_CORE_TYPE == MCU_CORE_TL323X)
        #define HCI_TR_RX_PIN              GPIO_FC_PB0
        #define HCI_TR_TX_PIN              GPIO_FC_PB1
        #define HCI_TR_BAUDRATE (1000000)
    #endif

    #define DBG_HCI_TR      0

    /*! HCI transport buffer size define. */
    #define HCI_TR_RX_BUF_SIZE (300)
    #define HCI_TR_TX_BUF_SIZE (300)

    #define HCI_DFU_EN         0
#else
    #define HCI_DFU_EN 0
#endif


#define ACL_CENTRAL_MAX_NUM 1 // ACL central maximum number
#define ACL_PERIPHR_MAX_NUM 1 // ACL peripheral maximum number

#if MCU_CORE_TYPE == MCU_CORE_TL322X
    #define PLIC_ENABLE           1
    #define CLIC_ENABLE           0
#endif

#define APP_CS_LOG_EN        0
///////////////////////// UI Configuration ////////////////////////////////////////////////////
#define UI_LED_ENABLE 1


#if (MCU_CORE_TYPE == MCU_CORE_B91)
    #define BOARD_SELECT BOARD_951X_EVK_C1T213A20
#elif (MCU_CORE_TYPE == MCU_CORE_B92)
    #define BOARD_SELECT BOARD_952X_EVK_C1T266A20
#elif (MCU_CORE_TYPE == MCU_CORE_TL721X)
    #define BOARD_SELECT BOARD_721X_EVK_C1T315A20
#elif (MCU_CORE_TYPE == MCU_CORE_TL321X)
    #define BOARD_SELECT BOARD_321X_EVK_C1T331A20 //BOARD_321X_EVK_C1T335A20
#elif (MCU_CORE_TYPE == MCU_CORE_TL322X)
    #define BOARD_SELECT BOARD_322X_EVK_C1T382A20
#elif (MCU_CORE_TYPE == MCU_CORE_TL323X)
    #define BOARD_SELECT BOARD_323X_EVK_C1T388A20
#elif (MCU_CORE_TYPE == MCU_CORE_TL521X)
    #define BOARD_SELECT BOARD_521X_EVK_C1T416A20
#endif

///////////////////////// DEBUG  Configuration ////////////////////////////////////////////////
#define DEBUG_GPIO_ENABLE     0

#define TLKAPI_DEBUG_ENABLE   1
#define TLKAPI_DEBUG_CHANNEL  TLKAPI_DEBUG_CHANNEL_GSUART

#define APP_LOG_EN            1
#define APP_FLASH_INIT_LOG_EN 1
#define APP_CONTR_EVT_LOG_EN  1 //controller event
#define APP_HOST_EVT_LOG_EN   1
#define APP_SMP_LOG_EN        0
#define APP_SIMPLE_SDP_LOG_EN 0
#define APP_PAIR_LOG_EN       1
#define APP_KEY_LOG_EN        1


#include "../common/default_config.h"
#include "bqb_config.h"
