/********************************************************************************************************
 * @file    ext_uart.h
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
#ifndef DRIVERS_TL321X_EXT_DRIVER_EXT_UART_H_
#define DRIVERS_TL321X_EXT_DRIVER_EXT_UART_H_


#include "common/config/user_config.h"


/**
 * @brief   debug UART port definition, don not change this
 */
#define DBG_UART_PORT0                              0x01
#define DBG_UART_PORT1                              0x02

/**
 * @brief   debug UART port selection, user can change in app_config.h
 */
#ifndef TLKAPI_DEBUG_UART_PORT
#define TLKAPI_DEBUG_UART_PORT                      DBG_UART_PORT0
#endif


/**
 * @brief   define debug UART channel according to UART port selection. don not change
 */
#if (TLKAPI_DEBUG_UART_PORT == DBG_UART_PORT0)
    #define DEBUG_UART_CHANNEL                      UART0
#elif (TLKAPI_DEBUG_UART_PORT == DBG_UART_PORT1)
    #define DEBUG_UART_CHANNEL                      UART1
#else
    #error "unsupported UART channel"
#endif


/**
 * @brief   debug UART TX DMA selection, user can change in app_config.h
 */
#ifndef TLKAPI_DEBUG_UART_TX_DMA
#define TLKAPI_DEBUG_UART_TX_DMA                    DMA4
#endif


/**
 * @brief   debug UART TX pin selection, user can change in app_config.h
 */
#ifndef TLKAPI_DEBUG_UART_TX_PIN
#define TLKAPI_DEBUG_UART_TX_PIN                    GPIO_FC_PD4
#endif

/**
 * @brief   debug UART RX pin selection, user can change in app_config.h
 */
#ifndef TLKAPI_DEBUG_UART_RX_PIN
#define TLKAPI_DEBUG_UART_RX_PIN                    GPIO_FC_PD3
#endif


/**
 * @brief   debug UART baudrate definition, user can change in app_config.h
 */
#ifndef TLKAPI_DEBUG_UART_BAUDRATE
#define TLKAPI_DEBUG_UART_BAUDRATE                  1000000
#endif



/**
 * @brief   UART IRQ status check, do not change this
 */
#define UART_DEBUG_CHECK_TX_DONE_IRQ_STATUS     uart_get_irq_status(DEBUG_UART_CHANNEL,UART_TXDONE_IRQ_STATUS)
#define UART_DEBUG_CLEAR_TX_DONE                uart_clr_irq_status(DEBUG_UART_CHANNEL, UART_TXDONE_IRQ_STATUS)





/**
 * @brief       Initialize UART debug interface, configuration see TLKAPI_DEBUG_UART_xxx.
 * @return      none
 */
void uart_debug_init(void);

/**
 * @brief       Send UART debug data to output interface.
 * @param[in]   addr - FIFO data pointer.
 * @param[in]   len - FIFO data length.
 * @return      none
 */
void uart_debug_prepare_dma_data(unsigned char * addr, unsigned int len);

#endif /* DRIVERS_TL321X_EXT_DRIVER_EXT_UART_H_ */
