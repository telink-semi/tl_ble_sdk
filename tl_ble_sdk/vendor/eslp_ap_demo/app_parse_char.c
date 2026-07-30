/********************************************************************************************************
 * @file    app_parse_char.c
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

#include "tl_common.h"
#include "drivers.h"
#include "application/usbstd/usbdesc.h"
#include "application/usbstd/usb.h"
#include "application/app/usbcdc.h"
#include "app_parse_char.h"
#include <strings.h>
#include <stdarg.h>

typedef struct {
    u16 write_index;
    u16 read_index;
    u16 size;
    u8 *buffer;
} ring_buf_t;

u8 shellRecvCmdBuf[PARSE_CHAR_UART_BUFF_SIZE + 1];      //str + '\0'
u16 shellRecvCmdBufIdx = 0;
u8 uartRecvBuf[PARSE_CHAR_UART_BUFF_SIZE];
const parse_fun_list_t* gParseList = NULL;
int gParseSize = 0;
static ring_buf_t appParseRingBuf;
static u8 ringBuf[PARSE_CHAR_UART_BUFF_SIZE * 2];
#if (APP_PARSE_CHAR_IFACE == APP_PARSE_CHAR_UART)
static app_parse_notify_cb_t notify_cb;
#endif

/**
 * @brief       ring buffer initial function.
 * @param[in]   ring_buf: ring buffer structure pointer.
 * @param[in]   size: ring buffer size.
 * @param[in]   buffer: ring buffer store data pointer.
 * @return      none.
 */
static void ring_buf_init(ring_buf_t *ring_buf, u16 size, u8 *buffer)
{
    ring_buf->size = size;
    ring_buf->buffer = buffer;
    ring_buf->write_index = 0;
    ring_buf->read_index = 0;
}

/**
 * @brief       free ring buffer space.
 * @param[in]   ring_buf: ring buffer structure pointer.
 * @return      none.
 */
static u16 ring_buf_free_space(ring_buf_t *ring_buf)
{
    if (ring_buf->read_index > ring_buf->write_index) {
        return ring_buf->read_index - ring_buf->write_index - 1;
    } else if (ring_buf->write_index > ring_buf->read_index) {
        return (ring_buf->read_index + ring_buf->size - ring_buf->write_index - 1);
    } else {
        // (ring_buf->write_index == ring_buf->read_index) means that buffer is empty
        return ring_buf->size - 1;
    }
}

/**
 * @brief       write data into ring buffer.
 * @param[in]   ring_buf: ring buffer structure pointer.
 * @param[in]   length: write buffer length.
 * @param[in]   buffer: write buffer pointer.
 * @return      number of bytes written.
 */
_attribute_ram_code_
static u16 ring_buf_write(ring_buf_t *ring_buf, u16 length, u8 *buffer)
{
    u16 free_space = ring_buf_free_space(ring_buf);
    u16 remaining = length;
    u16 len2;

    if (ring_buf->write_index >= ring_buf->read_index) {
        u16 len1 = free_space < (ring_buf->size - ring_buf->write_index) ? free_space : (ring_buf->size - ring_buf->write_index);
        if (len1 > remaining) {
            len1 = remaining;
        }

//      memcpy(&ring_buf->buffer[ring_buf->write_index], buffer, len1);
        //Interrupt code, must be placed in ramcode
        for(int i=0; i< len1; i++)
        {
            ring_buf->buffer[ring_buf->write_index + i] = buffer[i];
        }

        ring_buf->write_index = (ring_buf->write_index + len1) % ring_buf->size;
        remaining -= len1;
        buffer += len1;
        free_space -= len1;
    }

    len2 = remaining < free_space ? remaining : free_space;
//  memcpy(&ring_buf->buffer[ring_buf->write_index], buffer, len2);
    //Interrupt code, must be placed in ramcode
    for(int i=0; i< len2; i++)
    {
        ring_buf->buffer[ring_buf->write_index + i] = buffer[i];
    }
    remaining -= len2;
    ring_buf->write_index = (ring_buf->write_index + len2) % ring_buf->size;

    return length - remaining;
}

/**
 * @brief       read data into ring buffer.
 * @param[in]   ring_buf: ring buffer structure pointer.
 * @param[in]   length: want read buffer length.
 * @param[in]   buffer: read buffer pointer.
 * @return      number of bytes read.
 */
static u16 ring_buf_read(ring_buf_t *ring_buf, u16 length, u8 *buffer)
{
    u16 remaining = length;
    u16 available;

    if (ring_buf->read_index > ring_buf->write_index) {
        available = ring_buf->size - ring_buf->read_index;
        if (available > remaining) {
            available = remaining;
        }
        
        memcpy(buffer, &ring_buf->buffer[ring_buf->read_index], available);
        ring_buf->read_index = (ring_buf->read_index + available) % ring_buf->size;
        remaining -= available;
        buffer += available;
    }
    
    if (ring_buf->read_index < ring_buf->write_index) {
        available = ring_buf->write_index - ring_buf->read_index;
        if (available > remaining) {
            available = remaining;
        }
        
        memcpy(buffer, &ring_buf->buffer[ring_buf->read_index], available);
        ring_buf->read_index = (ring_buf->read_index + available) % ring_buf->size;
        remaining -= available;
        buffer += available;
    }
    
    return length - remaining;
}

#if (APP_PARSE_CHAR_IFACE == APP_PARSE_CHAR_UART)

_attribute_ram_code_
void app_UartTxIRQHandler(unsigned int * param)
{
    (void)* param;
}

_attribute_ram_code_
void app_UartRxIRQHandler(unsigned int * param)
{
    (void)* param;
     u32 rxLen;
    u8 *p = uartRecvBuf;
    ext_hci_uartReceData((p+4), PARSE_CHAR_UART_BUFF_SIZE - 4);//[!!important - must]

    STREAM_TO_U32(rxLen,p);

    ring_buf_write(&appParseRingBuf, rxLen, uartRecvBuf + 4);

    if (notify_cb) {
        notify_cb();
    }
}


#else

/**
 * @brief       usb-cdc receive data callback.
 * @param[in]   data: usb receive data pointer.
 * @param[in]   length: data length.
 * @return      none.
 */
//static void usb_cdc_read_cb(unsigned char * data, unsigned short length)
//{
//    ring_buf_write(&appParseRingBuf, length, data);
//    usb_cdc_read(usb_cdc_read_cb);
//}
#endif

/**
 * @brief       parse initial interface(uart/usb-cdc) function.
 * @param[in]   none.
 * @return      none.
 */
static void init_interface(void)
{
#if (APP_PARSE_CHAR_IFACE == APP_PARSE_CHAR_UART)

    ext_hci_InitTypeDef  hci_h4_uart;
    memset(&hci_h4_uart, 0, sizeof(ext_hci_InitTypeDef));
    hci_h4_uart.baudrate  = PARSE_CHAR_UART_BAUDRATE;
    hci_h4_uart.tx_Pin = PARSE_CHAR_UART_TX_PIN;      //
    hci_h4_uart.rx_Pin = PARSE_CHAR_UART_RX_PIN;      //
    hci_h4_uart.HwFlowCtl = 0;
    hci_h4_uart.cts_Pin = 0;
    hci_h4_uart.rts_Pin = 0;
    hci_h4_uart.RxCpltCallback = app_UartRxIRQHandler;
    hci_h4_uart.TxCpltCallback = app_UartTxIRQHandler;
    ext_hci_uartInit(&hci_h4_uart);
    u8 *p = uartRecvBuf;
    ext_hci_uartReceData((p + 4), PARSE_CHAR_UART_BUFF_SIZE - 4);


#elif (APP_PARSE_CHAR_IFACE == APP_PARSE_CHAR_USB_CDC)
    usb_init();

    /* set data endpoint buffer size and addr */
    usbhw_set_eps_max_size(CDC_TXRX_EPSIZE); /* max 64 */
    usbhw_set_ep_addr(CDC_NOTIFICATION_EPNUM, 0x00);
    usbhw_set_ep_addr(USB_PHYSICAL_EDP_CDC_IN, CDC_NOTIFICATION_EPSIZE);
    usbhw_set_ep_addr(USB_PHYSICAL_EDP_CDC_OUT, CDC_NOTIFICATION_EPSIZE + CDC_TXRX_EPSIZE);

    /* enable data endpoint CDC_NOTIFICATION_EPNUM, USB_PHYSICAL_EDP_CDC_IN and USB_PHYSICAL_EDP_CDC_OUT. */
    usbhw_set_eps_en(BIT(CDC_NOTIFICATION_EPNUM) | BIT(USB_PHYSICAL_EDP_CDC_IN) | BIT(USB_PHYSICAL_EDP_CDC_OUT));

    // enable USB DP pull up 1.5k
    usb_set_pin(1);

//    usb_cdc_read(usb_cdc_read_cb);
#endif
}


/**
 * @brief       parse initial function.
 * @param[in]   parseList: parse command list.
 * @param[in]   size: list size.
 * @return      none.
 */
void app_parse_init(const parse_fun_list_t *parseList, int size)
{
    gParseList = parseList;
    gParseSize = size;

    ring_buf_init(&appParseRingBuf, sizeof(ringBuf), ringBuf);

    init_interface();
}

/**
 * @brief       parse print log function.
 * @param[in]   Refer to printf parameter description.
 * @return      none.
 */
void app_parse_printf(const char *format, ...)
{
    static u8 aclBuf[PARSE_CHAR_UART_BUFF_SIZE];
    va_list args;
    va_start( args, format );

#if (APP_PARSE_CHAR_IFACE == APP_PARSE_CHAR_UART)
    // Before modifying the aclBuf, make sure that the previous transaction is complete
    while(ext_hci_getTxCompleteDone() == 0);
#endif

    int ret = vsnprintf((char*)(aclBuf), PARSE_CHAR_UART_BUFF_SIZE, format, args);
    va_end( args );

#if (APP_PARSE_CHAR_IFACE == APP_PARSE_CHAR_UART)
    ext_hci_uartSendData(aclBuf, ret);
#elif (APP_PARSE_CHAR_IFACE == APP_PARSE_CHAR_USB_CDC)
    usb_cdc_tx_data_to_host(aclBuf, ret);
#endif
}

/**
 * @brief       Query the separator location.
 * @param[in]   str: input string, '\0' ending, ' ' or '\t' separator.
 * @return      next input parameter pointer.
 */
static char* tlk_strchr(char *str)
{
    bool stringFlag = false;

    while(*str == ' ' || *str == '\t')
    {
        str++;
    }

    if(*str == '"')
    {
        stringFlag = true;
        str++;
    }

    while(*str != '\0')
    {
        if(*str == '\\')    //Escape character
        {
            str++;
            if(*str == '"')
                str ++;
        }

        if(stringFlag)
        {
            if(*str == '"')
            {
                *str++ = '\0';
                break;
            }
        }
        else
        {
            if(*str == ' ' || *str == '\t')
            {
                *str++ = '\0';
                break;
            }
        }
        str++;
    }

    return str;
}

/**
 * @brief       parse string split input parameter.
 * @param[in]   str: input string, '\0' ending, ' ' or '\t' separator.
 * @param[out]  argv: split input parameter pointer.
 * @return      parameter size.
 */
static int tlk_split_argv(char *str, char *argv[])
{
    int argc = 0;

    if (!strlen(str))
    {
        return 0;
    }

    for(int i = strlen(str)-1; i>0; i--)
    {
        if(str[i] == '\r' || str[i] == '\n')
            str[i] = '\0';
        else
            break;
    }

    while (*str && (*str == ' ' || *str == '\t'))
    {
        str++;  //skip empty or tab
    }

    if (!*str)
    {
        return 0;
    }

    argv[argc++] = str;

    while ((str = tlk_strchr(str)))
    {
        while (*str && (*str == ' ' || *str == '\t'))
        {
            str++;
        }

        if (!*str)
        {
            break;
        }

        argv[argc++] = *str == '"'? str+1: str;

        if (argc == PARSE_CHAR_MAX_ARGV_SIZE)
        {
            app_parse_printf("Too many parameters (max %zu)\r\n", PARSE_CHAR_MAX_ARGV_SIZE);
            return 0;
        }
    }

    /* keep it POSIX style where argv[argc] is required to be NULL */
    argv[argc] = NULL;

    return argc;
}

static void app_parse_complete(void)
{
    char *argv[PARSE_CHAR_MAX_ARGV_SIZE];
    int argc = tlk_split_argv((char *)shellRecvCmdBuf, argv);

    if (!argc) {
        return;
    }

    for(int i = 0; i<gParseSize; i++)
    {
        if(strcasecmp(argv[0], gParseList[i].fun_name) == 0)
        {
            gParseList[i].fun(&argv[1], argc-1, gParseList[i].user_data);
        }
    }
}

void app_parse_set_uart_rx_notify(app_parse_notify_cb_t cb)
{
#if (APP_PARSE_CHAR_IFACE == APP_PARSE_CHAR_UART)
    notify_cb = cb;
#else
    (void) cb;
#endif
}

/**
 * @brief       parse string loop.
 * @param[in]   none.
 * @return      none.
 */
void app_parse_loop(void)
{
#if (APP_PARSE_CHAR_IFACE == APP_PARSE_CHAR_USB_CDC)
    usb_handle_irq();
#endif
    if(usb_cdc_data_len) {
        ring_buf_write(&appParseRingBuf, usb_cdc_data_len, usb_cdc_data);
        usb_cdc_data_len = 0;
    }

    while (true) {
        if (!ring_buf_read(&appParseRingBuf, 1, &shellRecvCmdBuf[shellRecvCmdBufIdx])) {
            return;
        }

        if (shellRecvCmdBuf[shellRecvCmdBufIdx] == '\n' || shellRecvCmdBuf[shellRecvCmdBufIdx] == '\r' ||
                shellRecvCmdBuf[shellRecvCmdBufIdx] == '\0' || shellRecvCmdBufIdx == (sizeof(shellRecvCmdBuf) - 1)) {
            shellRecvCmdBuf[shellRecvCmdBufIdx] = '\0';
            app_parse_complete();
            shellRecvCmdBufIdx = 0;
        } else {
            // Continue reading characters
            shellRecvCmdBufIdx += 1;
        }
    }
}

/**
 * @brief       parse string to immediate value.
 * @param[in]   ps: value string, '\0' ending, supported -1, -0xAB, 1.
 * @return      immediate value.
 */
int app_parse_str2n (char * ps)
{
    int n = 0;
    int s = 1;
    int i = 0;
    int b = 10;

    while (ps[i]) {
        int c = ps[i];
        if (i==0 && c == '-') {
            s = -1;
        }
        else if (c == 'x' || c == 'X') {
            b = 16;
        }
        else {  //
            if (c>='A' && c<='F') {
                c = c - 'A' + 10;
            }
            else if (c>='a' && c<='f') {
                c = c - 'a' + 10;
            }
            else if (c>='0' && c<='9' ) {
                c -= '0';
            }
            else {
                c = 0;
            }
            n = n * b + c;

        }
        i++;
    }
    return s * n;
}
