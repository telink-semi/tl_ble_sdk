/********************************************************************************************************
 * @file    app_parse_char.h
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


#ifndef PARSE_CHAR_UART_PORT
#define PARSE_CHAR_UART_PORT                    UART0
#endif

#ifndef PARSE_CHAR_UART_TX_DMA
#define PARSE_CHAR_UART_TX_DMA                  DMA2
#endif

#ifndef PARSE_CHAR_UART_RX_DMA
#define PARSE_CHAR_UART_RX_DMA                  DMA3
#endif

#ifndef PARSE_CHAR_UART_TX_PIN
#define PARSE_CHAR_UART_TX_PIN                  UART1_TX_PC6
#endif

#ifndef PARSE_CHAR_UART_RX_PIN
#define PARSE_CHAR_UART_RX_PIN                  UART1_RX_PC7
#endif


#ifndef PARSE_CHAR_UART_BAUDRATE
#define PARSE_CHAR_UART_BAUDRATE                1000000
#endif

#ifndef PARSE_CHAR_UART_BUFF_SIZE
#define PARSE_CHAR_UART_BUFF_SIZE               256
#endif

#ifndef PARSE_CHAR_MAX_ARGV_SIZE
#define PARSE_CHAR_MAX_ARGV_SIZE                24
#endif

typedef struct{
    char fun_name[16];
    void (*fun)(char *argv[], int argc, void *user_data);
    void *user_data;
} parse_fun_list_t;

typedef struct{
    char param_name[16];
    void *param_ptr;
    char type;      //0:string, other: Numerical size
    char maxSize;   //
} set_param_list_t;

typedef void (* app_parse_notify_cb_t)(void);

/**
 * @brief       parse initial function.
 * @param[in]   parseList: parse command list.
 * @param[in]   size: list size.
 * @return      none.
 */
void app_parse_init(const parse_fun_list_t *parseList, int size);

/**
 * @brief       parse string loop.
 * @param[in]   none.
 * @return      none.
 */
void app_parse_loop(void);

/**
 * @brief       parse string to immediate value.
 * @param[in]   ps: value string, '\0' ending, supported -1, -0xAB, 1.
 * @return      immediate value.
 */
int app_parse_str2n (char * ps);

/**
 * @brief       parse print log function.
 * @param[in]   Refer to printf parameter description.
 * @return      none.
 */
void app_parse_printf(const char *format, ...);

/**
 * @brief       set UART RX ISR callback
 * @param[in]   cb: UART RX ISR notification callback
 * @return      none.
 */
void app_parse_set_uart_rx_notify(app_parse_notify_cb_t cb);
