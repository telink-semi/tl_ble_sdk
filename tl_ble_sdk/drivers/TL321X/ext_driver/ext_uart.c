/********************************************************************************************************
 * @file    ext_uart.c
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
#include "ext_uart.h"

#include "../driver.h"

#include "common/config/user_config.h"





int lp_uart_init = 0;  //attention: can not be retention data !!!
void uart_debug_init(void)
{
    uart_reset(DEBUG_UART_CHANNEL);  //will reset uart digital registers from 0x90 ~ 0x9f, so uart setting must set after this reset
    uart_set_pin(DEBUG_UART_CHANNEL,TLKAPI_DEBUG_UART_TX_PIN, TLKAPI_DEBUG_UART_RX_PIN);
    unsigned short div;
    unsigned char bwpc;
    uart_cal_div_and_bwpc(TLKAPI_DEBUG_UART_BAUDRATE, sys_clk.pclk*1000*1000, &div, &bwpc);
    uart_init(DEBUG_UART_CHANNEL, div, bwpc, UART_PARITY_NONE, UART_STOP_BIT_ONE);


    uart_set_tx_dma_config(DEBUG_UART_CHANNEL, TLKAPI_DEBUG_UART_TX_DMA);
    uart_clr_irq_status(DEBUG_UART_CHANNEL,UART_TXDONE_IRQ_STATUS);
    dma_clr_irq_mask(TLKAPI_DEBUG_UART_TX_DMA, TC_MASK|ABT_MASK|ERR_MASK);

    //uart_set_rx_timeout(DEBUG_UART_CHANNEL, bwpc, 12, UART_BW_MUL1);

    uart_set_irq_mask(DEBUG_UART_CHANNEL, UART_TXDONE_MASK);
    if (DEBUG_UART_CHANNEL == UART0){
        plic_interrupt_enable(IRQ_UART0);
        plic_set_priority(IRQ_UART0, 2);
    }
    else if (DEBUG_UART_CHANNEL == UART1){
        plic_interrupt_enable(IRQ_UART1);
        plic_set_priority(IRQ_UART1, 2);
    }

    lp_uart_init = 1;
}


_attribute_ram_code_
void uart_debug_prepare_dma_data(unsigned char * addr, unsigned int len)
{
    uart_send_dma(DEBUG_UART_CHANNEL, addr, len);
}
