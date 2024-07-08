/********************************************************************************************************
 * @file    tlkapi_debug.h
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
#ifndef VENDOR_COMMON_TLKAPI_DEBUG_H_
#define VENDOR_COMMON_TLKAPI_DEBUG_H_



/**
 * @brief    Telink debug log enable or disable
 *             user can set it in app_config.h, default disable if user not set it
 */
#ifndef    TLKAPI_DEBUG_ENABLE
#define TLKAPI_DEBUG_ENABLE                           0
#endif



/**
 * @brief    Telink debug log channel select
 *             user can select one type in app_config.h
 */
#define TLKAPI_DEBUG_CHANNEL_UDB                      1    //USB Dump
#define TLKAPI_DEBUG_CHANNEL_GSUART                   2     //GPIO simulate UART
#define TLKAPI_DEBUG_CHANNEL_UART                     3   //hardware UART

/* if user enable debug log but not select log channel, will use "GPIO simulate UART" as default */
#ifndef    TLKAPI_DEBUG_CHANNEL
#define TLKAPI_DEBUG_CHANNEL                          TLKAPI_DEBUG_CHANNEL_GSUART
#endif



#if (TLKAPI_DEBUG_CHANNEL == TLKAPI_DEBUG_CHANNEL_GSUART)
    /**
     * @brief    some default setting for "GPIO simulate UART" log channel
     */
    /* default GPIO, user can change it in app_config.h */
    #ifndef TLKAPI_DEBUG_GPIO_PIN
    #define TLKAPI_DEBUG_GPIO_PIN                    GPIO_PD4
    #endif

    /* default UART baudrate, user can change it in app_config.h */
    #ifndef TLKAPI_DEBUG_GSUART_BAUDRATE
    #define TLKAPI_DEBUG_GSUART_BAUDRATE             1000000
    #endif
#elif (TLKAPI_DEBUG_CHANNEL == TLKAPI_DEBUG_CHANNEL_UART)
    /**
     * attention: default setting for UART log channel are written in ext_uart.h, because hardware UART maybe different for different IC
     */
#endif


/**
 * @brief    default log FIFO size, user can change it in app_config.h
 */
#ifndef    TLKAPI_DEBUG_FIFO_SIZE
#define TLKAPI_DEBUG_FIFO_SIZE                        288
#endif

/**
 * @brief    default log FIFO number, user can change it in app_config.h
 */
#ifndef    TLKAPI_DEBUG_FIFO_NUM
#define TLKAPI_DEBUG_FIFO_NUM                         16
#endif



#ifndef    APP_REAL_TIME_PRINTF
#define APP_REAL_TIME_PRINTF                           0
#endif


/* internal special UART tool with high efficiency, but not publicly release
 * so only for internal debug, customer never change this macro */
#ifndef    TLKAPI_USE_INTERNAL_SPECIAL_UART_TOOL
#define TLKAPI_USE_INTERNAL_SPECIAL_UART_TOOL           0
#endif



/**
 * @brief    Debug log control data structure, user do not need to pay attention to these
 */
typedef struct{
    u8    dbg_en;
    u8    dbg_chn;
    u8    uartSendIsBusy;
    u8    fifo_format_len;

    u16    usb_id;
    u16    fifo_data_len;
}tlk_dbg_t;
extern tlk_dbg_t tlkDbgCtl;





/**
 * @brief       Debug log initialization when MCU power on or wake_up from deepSleep mode
 * @param[in]    none
 * @return        none
 */
int  tlkapi_debug_init(void);


/**
 * @brief       Debug log initialization when MCU wake_up from deepSleep_retention mode
 * @param[in]    none
 * @return        none
 */
int  tlkapi_debug_deepRetn_init(void);


/**
 * @brief       Debug log process in mainloop, output log form log FIFO if FIFO not empty
 * @param[in]    none
 * @return        none
 */
void tlkapi_debug_handler(void);


/**
 * @brief          check if debug log busy
 * @param[in]    none
 * @return        1: debug log busy, some log pending in FIFO, not all finish;
 *                 0: debug log empty
 */
bool tlkapi_debug_isBusy(void);


/**
 * @brief          customize USB ID for UDB channel
 *                 user can open more than one USB debug tool on PC with different USB ID
 * @param[in]    cus_usb_id - customized USB ID
 * @return        none
 */
void tlkapi_debug_customize_usb_id(u16 cus_usb_id);







/**
 * @brief    user do not need to pay attention to three APIs below, and do not use them in application.
 */
void tlkapi_send_str_data (char *str, u8 *pData, u32 data_len);
void tlkapi_send_str_u8s(char *str, int size, ...);
void tlkapi_send_str_u32s(char *str, int size, ...);



/**
 * @brief       Send debug log to log FIFO, character string and data mixed mode.
 *                attention: here just send log to FIFO, can not output immediately, wait for "tlkapi debug_handler" to output log.
 * @param[in]    en - send log enable, 1: enable;  0: disable
 * @param[in]    str - character string
 * @param[in]    pData - pointer of data
 * @param[in]    len - length of data
 * @return        1: send to FIFO success; 0: send to FIFO fail
 */
#define tlkapi_send_string_data(en, str, pData, len)        if(en){tlkapi_send_str_data(str,(u8*)(u32)(pData), len);}


/**
 * @brief       Send debug log to log FIFO, character string and data mixed mode, with variable length data, data in "unsigned int" format
 *                attention: here just send log to FIFO, can not output immediately, wait for "tlkapi debug_handler" to output log.
 * @param[in]    en - send log enable, 1: enable;  0: disable
 * @param[in]    str - character string
 * @param[in]    ... - variable length data, maximum length is 8
 * @param[in]    data_len - length of data
 * @return        1: send to FIFO success; 0: send to FIFO fail
 */
#define tlkapi_send_string_u32s(en, str, ...)                if(en){tlkapi_send_str_u32s(str, COUNT_ARGS(__VA_ARGS__), ##__VA_ARGS__);}


/**
 * @brief       Send debug log to log FIFO, character string and data mixed mode, with variable length data, data in "unsigned char" format
 *                attention: here just send log to FIFO, can not output immediately, wait for "tlkapi debug_handler" to output log.
 * @param[in]    en - send log enable, 1: enable;  0: disable
 * @param[in]    str - character string
 * @param[in]    ... - variable length data, maximum length is 16
 * @param[in]    data_len - length of data
 * @return        1: send to FIFO success; 0: send to FIFO fail
 */
#define tlkapi_send_string_u8s(en, str, ...)                if(en){tlkapi_send_str_u8s(str, COUNT_ARGS(__VA_ARGS__), ##__VA_ARGS__);}





/**
 * @brief       Send debug log to log FIFO, string only mode
 *                attention: here just send log to FIFO, can not output immediately, wait for "tlkapi debug_handler" to output log.
 * @param[in]    fmt - please refer to standard C function "printf"
 * @return        none
 */
int  tlk_printf(const char *fmt, ...);

/**
 * @brief       Send debug log to log FIFO, string only mode
 *                attention: here just send log to FIFO, can not output immediately, wait for "tlkapi debug_handler" to output log.
 * @param[in]    en - send log enable, 1: enable;  0: disable
 * @param[in]    fmt - please refer to standard C function "printf"
 * @return        none
 */
#define tlkapi_printf(en, fmt, ...)                            if(en){tlk_printf(fmt, ##__VA_ARGS__);}









/* remove later */
#ifndef    APP_LOG_EN
#define APP_LOG_EN                                            0
#endif


#define BLT_APP_LOG(fmt, ...)                                 if(APP_LOG_EN) tlkapi_printf(1, "[APP]"fmt "\n", ##__VA_ARGS__);
#define BLT_APP_STR_LOG                                       if(APP_LOG_EN) tlkapi_send_str_data



#endif /* VENDOR_COMMON_TLKAPI_DEBUG_H_ */
