/********************************************************************************************************
 * @file    tlkapi_debug.c
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

#include "tlkapi_debug.h"

#include <stdarg.h>

#include "user_config.h"
_attribute_ble_data_retention_ tlk_dbg_t tlkDbgCtl = {
    .usb_id = 0x120,
};

_attribute_ble_data_retention_ my_fifo_t   *tlkapi_print_fifo = NULL;
_attribute_ble_data_retention_ u16 g_debug_serial = 0;


#if (TLKAPI_DEBUG_ENABLE)




//MYFIFO_INIT_IRAM(print_fifo, TLKAPI_DEBUG_FIFO_SIZE, TLKAPI_DEBUG_FIFO_NUM);
_attribute_iram_noinit_data_ u8   print_fifo_b[TLKAPI_DEBUG_FIFO_SIZE * TLKAPI_DEBUG_FIFO_NUM];

_attribute_ble_data_retention_    my_fifo_t   print_fifo = {
                                                TLKAPI_DEBUG_FIFO_SIZE,
                                                TLKAPI_DEBUG_FIFO_NUM,
                                                0,
                                                0,
                                                print_fifo_b};









/**
 * @brief       customize USB ID for UDB channel
 *              user can open more than one USB debug tool on PC with different USB ID
 * @param[in]   cus_usb_id - customized USB ID
 * @return      none
 */
void tlkapi_debug_customize_usb_id(u16 cus_usb_id)
{
    tlkDbgCtl.usb_id = cus_usb_id;
}




#if (TLKAPI_DEBUG_CHANNEL == TLKAPI_DEBUG_CHANNEL_UDB)






#elif (TLKAPI_DEBUG_CHANNEL == TLKAPI_DEBUG_CHANNEL_GSUART)

_attribute_ble_data_retention_ static uint32 sTlkApiDebugBitIntv;


_attribute_ram_code_sec_noinline_
void tlkapi_debug_putchar(uint08 byte)
{
    uint32 r = irq_disable();

#if((MCU_CORE_TYPE == MCU_CORE_B91) || (MCU_CORE_TYPE == MCU_CORE_B92))
    uint08 bits[14] = {0};
    uint08 out_level = reg_gpio_out(TLKAPI_DEBUG_GPIO_PIN);
    uint08 bit0 = out_level & ~(TLKAPI_DEBUG_GPIO_PIN);
    uint08 bit1 = out_level | TLKAPI_DEBUG_GPIO_PIN;
#elif (MCU_CORE_TYPE == MCU_CORE_TL721X || MCU_CORE_TYPE == MCU_CORE_TL321X)
    uint16 bits[14] = {0};
    uint16 bit0 = (TLKAPI_DEBUG_GPIO_PIN & 0xff)<<8;;
    uint16 bit1 = TLKAPI_DEBUG_GPIO_PIN & 0xff;
#else
    #error "Confirm api interface (reg_gpio)"
#endif

    bits[4] = bit0;
    for(int i=0; i<8; i++){
        if(byte & BIT(i)){
            bits[i+5] = bit1;
        }
        else{
            bits[i+5] = bit0;
        }
    }
    bits[13] = bit1;
    if(sys_clk.cclk <=32 && TLKAPI_DEBUG_GSUART_BAUDRATE>115200)
    {
        bits[0] = bit1;
        bits[1] = bit1;
        bits[2] = bit1;
        bits[3] = bit1;
        unsigned char i,j = 0;
        /*
         * bit_nop is num of clock to transmit a bit use PRINT_BAUD_RATE.
         * 1 / sys_clk.cclk * (8 + 4 * bit_nop) = 1000000 / PRINT_BAUD_RATE.
         * 8 is clock of nop when bit_nop is 0.
         * 4 is clock of for(i) when bit_nop is not 0.
         */
        unsigned char bit_nop = sys_clk.cclk * 250000/ TLKAPI_DEBUG_GSUART_BAUDRATE - 2;
        for(j = 0;j<14;j++)
        {
            for(i = 0;i<bit_nop;i++)//for:4 nop
            {
                __asm__("nop");
            }
            __asm__("nop");
            reg_gpio_out(TLKAPI_DEBUG_GPIO_PIN) = bits[j];
        }
    }
    else
    {
        uint32 time1 = clock_time();
        uint32 time2;
        for(int i=4; i<14; i++){
            time2 = time1;
            while(time1 - time2 < sTlkApiDebugBitIntv){
                time1 = clock_time();
            }
            reg_gpio_out(TLKAPI_DEBUG_GPIO_PIN) = bits[i];
        }
    }
    irq_restore(r);
}

#elif (TLKAPI_DEBUG_CHANNEL == TLKAPI_DEBUG_CHANNEL_UART)
        _attribute_ram_code_sec_ void tlkapi_uart_irq_handler(void)
        {
            if(UART_DEBUG_CHECK_TX_DONE_IRQ_STATUS){
                UART_DEBUG_CLEAR_TX_DONE;

                if(tlkDbgCtl.uartSendIsBusy){
                    tlkDbgCtl.uartSendIsBusy = 0;

                    if (tlkapi_print_fifo->wptr != tlkapi_print_fifo->rptr){
                        u8 *pData =  tlkapi_print_fifo->p + (tlkapi_print_fifo->rptr++ & (tlkapi_print_fifo->num - 1)) * tlkapi_print_fifo->size;
                        uart_debug_prepare_dma_data(pData + 4, pData[0]);
                        tlkDbgCtl.uartSendIsBusy = 1;
                    }
                }
            }
        }
#if (TLKAPI_DEBUG_UART_PORT == DBG_UART_PORT0)
        PLIC_ISR_REGISTER(tlkapi_uart_irq_handler, IRQ_UART0)
#elif (TLKAPI_DEBUG_UART_PORT == DBG_UART_PORT1)
        PLIC_ISR_REGISTER(tlkapi_uart_irq_handler, IRQ_UART1)
#endif

#endif


/**
 * @brief       Debug log initialization when MCU power on or wake_up from deepSleep mode
 * @param[in]   none
 * @return      none
 */
int tlkapi_debug_init(void)
{
#if (APP_REAL_TIME_PRINTF && TLKAPI_DEBUG_CHANNEL != TLKAPI_DEBUG_CHANNEL_GSUART)
    #error "APP_REAL_TIME_PRINTF only support TLKAPI_DEBUG_CHANNEL_GSUART!!"
#endif
    tlkDbgCtl.dbg_en = 1;

    if (!tlkapi_print_fifo){
        tlkapi_print_fifo = &print_fifo;
    }
    tlkapi_print_fifo->wptr = tlkapi_print_fifo->rptr = 0;


    #if (TLKAPI_DEBUG_CHANNEL == TLKAPI_DEBUG_CHANNEL_UDB)
        #if (BLE_APP_PM_ENABLE)
            #error "can not use USB debug when PM enable !!!"
        #endif
        #if (MCU_CORE_TYPE == MCU_CORE_TL721X)
            #error "TL721X Unable to use USB debugging  !!!"  //todo
        #endif
        extern my_fifo_t   *myudb_print_fifo;
        myudb_print_fifo = tlkapi_print_fifo;
        tlkDbgCtl.dbg_chn = TLKAPI_DEBUG_CHANNEL_UDB;
        tlkDbgCtl.fifo_format_len = 12;
        myudb_usb_init(tlkDbgCtl.usb_id, NULL);
        usb_set_pin_en();
    #else

        #if (TLKAPI_DEBUG_CHANNEL == TLKAPI_DEBUG_CHANNEL_GSUART)
            tlkDbgCtl.dbg_chn = TLKAPI_DEBUG_CHANNEL_GSUART;
            sTlkApiDebugBitIntv = SYSTEM_TIMER_TICK_1S/TLKAPI_DEBUG_GSUART_BAUDRATE;
            gpio_set_gpio_en(TLKAPI_DEBUG_GPIO_PIN);
            gpio_set_up_down_res(TLKAPI_DEBUG_GPIO_PIN, GPIO_PIN_PULLUP_1M);
            gpio_set_output_en(TLKAPI_DEBUG_GPIO_PIN, 1);
            gpio_write(TLKAPI_DEBUG_GPIO_PIN, 1);
        #elif (TLKAPI_DEBUG_CHANNEL == TLKAPI_DEBUG_CHANNEL_UART)
            tlkDbgCtl.dbg_chn = TLKAPI_DEBUG_CHANNEL_UART;

            tlkDbgCtl.uartSendIsBusy = 0;
            uart_debug_init();
        #endif

        #if (TLKAPI_USE_INTERNAL_SPECIAL_UART_TOOL)
            tlkDbgCtl.fifo_format_len = 12;
        #else
            tlkDbgCtl.fifo_format_len = 0;
        #endif
    #endif


    tlkDbgCtl.fifo_data_len = tlkapi_print_fifo->size - tlkDbgCtl.fifo_format_len;

    return 0;
}


/**
 * @brief       Debug log initialization when MCU wake_up from deepSleep_retention mode
 * @param[in]   none
 * @return      none
 */
int tlkapi_debug_deepRetn_init(void)
{

    #if (TLKAPI_DEBUG_CHANNEL == TLKAPI_DEBUG_CHANNEL_GSUART)
        gpio_set_gpio_en(TLKAPI_DEBUG_GPIO_PIN);
        gpio_write(TLKAPI_DEBUG_GPIO_PIN, 1);
        gpio_set_output_en(TLKAPI_DEBUG_GPIO_PIN, 1);
    #elif (TLKAPI_DEBUG_CHANNEL == TLKAPI_DEBUG_CHANNEL_UART)
        uart_debug_init();
    #endif

    return 0;
}



/**
 * @brief       Debug log process in mainloop, output log form log FIFO if FIFO not empty
 * @param[in]   none
 * @return      none
 */
_attribute_ram_code_sec_noinline_
void tlkapi_debug_handler(void)
{

#if (TLKAPI_DEBUG_CHANNEL == TLKAPI_DEBUG_CHANNEL_UDB)
    udb_usb_handle_irq();
#elif(TLKAPI_DEBUG_CHANNEL == TLKAPI_DEBUG_CHANNEL_GSUART)
    uint08 *pData;
    if (tlkapi_print_fifo->wptr != tlkapi_print_fifo->rptr){
        pData =  tlkapi_print_fifo->p + (tlkapi_print_fifo->rptr++ & (tlkapi_print_fifo->num - 1)) * tlkapi_print_fifo->size;
    }
    else{
        return;
    }
    uint16 dataLen = ((uint16)pData[1] << 8) | pData[0];
    for(int i=0; i<dataLen; i++){
        tlkapi_debug_putchar(pData[4+i]);
    }
#elif(TLKAPI_DEBUG_CHANNEL == TLKAPI_DEBUG_CHANNEL_UART)
    if (!tlkDbgCtl.uartSendIsBusy && tlkapi_print_fifo->wptr != tlkapi_print_fifo->rptr){
        u32 r = irq_disable();
        if(!tlkDbgCtl.uartSendIsBusy && tlkapi_print_fifo->wptr != tlkapi_print_fifo->rptr){
            u8 *pData =  tlkapi_print_fifo->p + (tlkapi_print_fifo->rptr & (tlkapi_print_fifo->num - 1)) * tlkapi_print_fifo->size;
            uart_debug_prepare_dma_data(pData + 4, pData[0]);
            tlkDbgCtl.uartSendIsBusy = 1;
            tlkapi_print_fifo->rptr++;
        }
        irq_restore(r);
    }
#endif
}


#endif
/**
 * @brief       check if debug log busy
 * @param[in]   none
 * @return      1: debug log busy, some log pending in FIFO, not all finish;
 *              0: debug log empty
 */
bool tlkapi_debug_isBusy(void)
{
    if(tlkapi_print_fifo){
        #if (TLKAPI_DEBUG_CHANNEL == TLKAPI_DEBUG_CHANNEL_UART)
            if(tlkDbgCtl.uartSendIsBusy){
                 return TRUE;
            }
        #endif

        return (tlkapi_print_fifo->wptr != tlkapi_print_fifo->rptr);
    }
    else{
        return 0;
    }
}


__attribute__((section(".data"))) unsigned char hex_table[] = "0123456789abcdef"; //improve: can not optimized to rodata
/**
 * @brief   Send debug log to log FIFO, character string and data mixed mode.
 *          attention: here just send log to FIFO, can not output immediately, wait for "tlkapi debug_handler" to output log.
*/
_attribute_ram_code_sec_noinline_
void tlkapi_send_str_data (char *str, u8 *pData, u32 data_len)
{
    if(!tlkapi_print_fifo){
        return;
    }

#if (APP_REAL_TIME_PRINTF && TLKAPI_DEBUG_CHANNEL == TLKAPI_DEBUG_CHANNEL_GSUART)
#define TLKAPI_DEBUG_DATA_MAX_LEN    64

        unsigned char temp_str[TLKAPI_DEBUG_DATA_MAX_LEN * 3 + 4];
        u8 i;

        u8 len = min(data_len, TLKAPI_DEBUG_DATA_MAX_LEN);
        int ind = 0;
        temp_str[ind++] = ':';
        for (i = 0; i < len; i++) {
            temp_str[ind++] = ' ';
            temp_str[ind++] = hex_table[pData[i] >> 4];
            temp_str[ind++] = hex_table[pData[i] & 0xf];
        }
        temp_str[ind++] = '\r';
        temp_str[ind++] = '\n';
        temp_str[ind] = '\0';
        printf("%s%s", str, temp_str);

#else
    if(data_len > 500){
        return;
    }

    extern int tlk_strlen(const char *str);
    int ns = str ? tlk_strlen (str) : 0;
    if (ns > tlkDbgCtl.fifo_data_len)
    {
        ns = tlkDbgCtl.fifo_data_len;
        data_len = 0;
    }
    else if (data_len + ns > tlkDbgCtl.fifo_data_len)
    {
        data_len = tlkDbgCtl.fifo_data_len - ns;
    }



    u32 r = irq_disable();

    u8 *pd =  tlkapi_print_fifo->p + (tlkapi_print_fifo->wptr & (tlkapi_print_fifo->num - 1)) * tlkapi_print_fifo->size;

    if(tlkDbgCtl.dbg_chn == TLKAPI_DEBUG_CHANNEL_UDB)
    {
        int len = data_len + ns + 5 + 6;
        *pd++ = len;
        *pd++ = len >> 8;
        *pd++ = 0;
        *pd++ = 0;
        *pd++ = 0x82;
        *pd++ = 8;

        *pd++ = 0x22;
        *pd++ = data_len;
        *pd++ = data_len >> 8;

        while (data_len--)
        {
            *pd++ = *pData++;
        }

        g_debug_serial++;
        *pd++ = '[';
        *pd++ = hex_table[g_debug_serial >> 12]; //0x0000 ~ 0xFFFF, high 4 bit no need "& 0x0F"
        *pd++ = hex_table[(g_debug_serial >> 8) & 0x0F];
        *pd++ = hex_table[(g_debug_serial >> 4) & 0x0F];
        *pd++ = hex_table[g_debug_serial & 0x0F];
        *pd++ = ']';

        while (ns--)
        {
            *pd++ = *str++;
        }
    }
    else // TLKAPI_DEBUG_CHANNEL_GSUART or TLKAPI_DEBUG_CHANNEL_UART
    {

        #if (TLKAPI_USE_INTERNAL_SPECIAL_UART_TOOL)
            int len = ns + data_len + 5;
            *pd++ = len;
            *pd++ = len >> 8;
            *pd++ = 0;
            *pd++ = 0;

            *pd++ = 0x95;   //special mark: 0xA695
            *pd++ = 0xA6;
            *pd++ = ns;     // string length, 1byte
            *pd++ = data_len;       // data length, 2 byte
            *pd++ = data_len >> 8;

            while (ns--)
            {
                *pd++ = *str++;
            }
            while (data_len--)
            {
                *pd++ = *pData++;
            }
            //add a '\n' by UART tool
        #else
            u16 max_len = ((tlkDbgCtl.fifo_data_len - ns) - 3 - 4) / 3;
            if(data_len > max_len){
                data_len = max_len;
            }
            int len = ns + data_len * 3 + 3 + 6;
            *pd++ = len;
            *pd++ = len >> 8;
            *pd++ = 0;
            *pd++ = 0;

            g_debug_serial++;
            *pd++ = '[';
            *pd++ = hex_table[g_debug_serial >> 12]; //0x0000 ~ 0xFFFF, high 4 bit no need "& 0x0F"
            *pd++ = hex_table[(g_debug_serial >> 8) & 0x0F];
            *pd++ = hex_table[(g_debug_serial >> 4) & 0x0F];
            *pd++ = hex_table[g_debug_serial & 0x0F];
            *pd++ = ']';

            while (ns--)
            {
                *pd++ = *str++;
            }

            *pd++ = ':';
            for (unsigned int i = 0; i < data_len; i++) {
                *pd++ = ' ';
                *pd++ = hex_table[*pData >> 4];
                *pd++ = hex_table[*pData & 0xf];
                pData++;
            }
            *pd++ = '\r';
            *pd = '\n';
        #endif
    }

    tlkapi_print_fifo->wptr++;

    irq_restore(r);
#endif
}

#define SEND_U8S_MAX_NUM                        16

_attribute_ram_code_sec_noinline_
void tlkapi_send_str_u8s(char *str, int size, ...)
{
    u8 d[SEND_U8S_MAX_NUM];
    size = min(SEND_U8S_MAX_NUM, size);
    va_list args;
    va_start( args, size);
    for(int i=0; i<size; i++)
    {
        d[i] = va_arg(args, unsigned int);
    }
    tlkapi_send_str_data(str, d, size);
    va_end( args );
}


#define SEND_U32S_MAX_NUM                       8

_attribute_ram_code_sec_noinline_
void tlkapi_send_str_u32s(char *str, int size, ...)
{
    u32 d[SEND_U32S_MAX_NUM];
    size = min(SEND_U32S_MAX_NUM, size);
    va_list args;
    va_start( args, size);
    for(int i=0; i<size; i++)
    {
        d[i] = va_arg(args, unsigned int);
    }
    tlkapi_send_str_data(str, (u8*)d, size*4);
    va_end( args );
}


//keep safe, if user call printf func, smp will be wrong, if no printf is used, below can be removed.

__attribute__((used)) int _write(int fd, const unsigned char *buf, int size)
{
    (void)fd;
#if (APP_REAL_TIME_PRINTF && (TLKAPI_DEBUG_CHANNEL == TLKAPI_DEBUG_CHANNEL_GSUART) && TLKAPI_DEBUG_ENABLE)
    int i;
    for(i = 0; i < size; i++){
        tlkapi_debug_putchar(buf[i]);
    }
    return i;
#else
    if(!tlkapi_print_fifo){
        return 0;
    }
    u8 *pd =  tlkapi_print_fifo->p + (tlkapi_print_fifo->wptr & (tlkapi_print_fifo->num - 1)) * tlkapi_print_fifo->size;
    if(tlkDbgCtl.dbg_chn == TLKAPI_DEBUG_CHANNEL_UDB){
        memcpy((char*)(pd + 9),buf,size);
        int len = size + 5;

        *pd++ = len;
        *pd++ = len >> 8;
        *pd++ = 0;
        *pd++ = 0;

        *pd++ = 0x82;
        *pd++ = 8;
        *pd++ = 0x22;
        *pd++ = 0;
        *pd++ = 0;
    }
    else{
        memcpy((char*)(pd + 4),buf,size);
        int len = size;

        *pd++ = len;
        *pd++ = len >> 8;
        *pd++ = 0;
        *pd++ = 0;
    }
    tlkapi_print_fifo->wptr ++;
    return size;
#endif
}

/**
 * @brief       Send debug log to log FIFO, printf mode
 *              attention: here just send log to FIFO, can not output immediately, wait for "tlkapi debug_handler" to output log.
 * @param[in]   format -
 * @return
 */
int tlk_printf(const char *format, ...)
{
#if (APP_REAL_TIME_PRINTF && TLKAPI_DEBUG_CHANNEL == TLKAPI_DEBUG_CHANNEL_GSUART)
    va_list args;
    va_start(args, format);
    int ret = vprintf(format, args);
    va_end(args);
    return ret;

#else
    if(!tlkapi_print_fifo){
        return 0;
    }

    u8 *pd =  tlkapi_print_fifo->p + (tlkapi_print_fifo->wptr & (tlkapi_print_fifo->num - 1)) * tlkapi_print_fifo->size;
    int ret;

#if ((MCU_CORE_TYPE == MCU_CORE_B91) || (MCU_CORE_TYPE == MCU_CORE_B92) || (MCU_CORE_TYPE == MCU_CORE_TL751X)||(MCU_CORE_TYPE == MCU_CORE_TL721X)||(MCU_CORE_TYPE == MCU_CORE_TL321X))
    va_list args;
    va_start( args, format );

    if(tlkDbgCtl.dbg_chn == TLKAPI_DEBUG_CHANNEL_UDB){
        ret = vsnprintf((char*)(pd + 9 + 6), tlkDbgCtl.fifo_data_len, format, args);
    }
    else{
        ret = vsnprintf((char*)(pd + 4 + 6), tlkDbgCtl.fifo_data_len, format, args);
    }

    va_end( args );
#else
    #error "print_f process for other MCU !!!"
#endif


    if(tlkDbgCtl.dbg_chn == TLKAPI_DEBUG_CHANNEL_UDB){
        int len = ret + 5 + 6;
        *pd++ = len;
        *pd++ = len >> 8;
        *pd++ = 0;
        *pd++ = 0;

        *pd++ = 0x82;
        *pd++ = 8;
        *pd++ = 0x22;
        *pd++ = 0;
        *pd++ = 0;

        g_debug_serial++;
        *pd++ = '[';
        *pd++ = hex_table[g_debug_serial >> 12]; //0x0000 ~ 0xFFFF, high 4 bit no need "& 0x0F"
        *pd++ = hex_table[(g_debug_serial >> 8) & 0x0F];
        *pd++ = hex_table[(g_debug_serial >> 4) & 0x0F];
        *pd++ = hex_table[g_debug_serial & 0x0F];
        *pd++ = ']';
    }
    else{
        int len = ret + 6;
        *pd++ = len;
        *pd++ = len >> 8;
        *pd++ = 0;
        *pd++ = 0;

        g_debug_serial++;
        *pd++ = '[';
        *pd++ = hex_table[g_debug_serial >> 12]; //0x0000 ~ 0xFFFF, high 4 bit no need "& 0x0F"
        *pd++ = hex_table[(g_debug_serial >> 8) & 0x0F];
        *pd++ = hex_table[(g_debug_serial >> 4) & 0x0F];
        *pd++ = hex_table[g_debug_serial & 0x0F];
        *pd++ = ']';
        //pd += ret;
        //*pd++ = '\n';
    }

    tlkapi_print_fifo->wptr ++;
    return ret;
#endif
}
