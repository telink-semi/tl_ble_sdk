/********************************************************************************************************
 * @file    ext_hci_uart.c
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
#include "ext_hci_uart.h"
#include "clock.h"
#include "tl_common.h"


#ifndef HCI_UART_EXT_DRIVER_EN
#define HCI_UART_EXT_DRIVER_EN  0
#endif

#ifndef HCI_TR_EN
#define HCI_TR_EN               0
#endif

#if (HCI_UART_EXT_DRIVER_EN||HCI_TR_EN)
/**
 * @brief  UART Select interface
 */
#ifndef  EXT_HCI_UART_CHANNEL
#define  EXT_HCI_UART_CHANNEL               UART1
#endif

#ifndef  EXT_HCI_UART_IRQ
#define  EXT_HCI_UART_IRQ                   IRQ_UART1
#endif

#ifndef  EXT_HCI_UART_DMA_CHN_RX
#define  EXT_HCI_UART_DMA_CHN_RX            DMA2   //uart dma
#endif

#ifndef  EXT_HCI_UART_DMA_CHN_TX
#define  EXT_HCI_UART_DMA_CHN_TX            DMA3
#endif

/*** RTS ***/
#define RTS_INVERT    1 /*!< 0: RTS PIN will change from low to high. !! use for RTS auto mode(default auto mode)*/
#define RTS_THRESH    5 /*!< RTS trigger threshold. range: 1-16. */
/*** CTS ***/
#define STOP_VOLT     1         //0 :Low level stops TX.  1 :High level stops TX.



_attribute_data_retention_sec_ static volatile unsigned char  uart_dma_send_flag = 1;
_attribute_data_retention_sec_ static volatile CpltCallback   RxCpltCallback;
_attribute_data_retention_sec_ static volatile CpltCallback   TxCpltCallback;
_attribute_data_retention_sec_ static volatile unsigned char  *ReceAddr;

/**
 * @brief   hci uart initialization
 * @param   none
 * @return  none
 */
ext_hci_StatusTypeDef_e ext_hci_uartInit(ext_hci_InitTypeDef * uart)
{

    unsigned short div;
    unsigned char bwpc;
      /* Check the UART handle allocation */
   if (uart == NULL)
   {
        return EXT_UART_ERROR;
   }

    uart_reset(EXT_HCI_UART_CHANNEL);

    uart_set_pin(EXT_HCI_UART_CHANNEL,uart->tx_Pin,uart->rx_Pin);

    uart_cal_div_and_bwpc(uart->baudrate, sys_clk.pclk*1000*1000, &div, &bwpc);

    uart_set_rx_timeout(EXT_HCI_UART_CHANNEL, bwpc, 12, UART_BW_MUL2);

    uart_init(EXT_HCI_UART_CHANNEL, div, bwpc, UART_PARITY_NONE, UART_STOP_BIT_ONE);

    uart_set_tx_dma_config(EXT_HCI_UART_CHANNEL, EXT_HCI_UART_DMA_CHN_TX);

    uart_set_rx_dma_config(EXT_HCI_UART_CHANNEL, EXT_HCI_UART_DMA_CHN_RX);


    uart_set_irq_mask(EXT_HCI_UART_CHANNEL, UART_TXDONE_MASK);

    uart_set_irq_mask(EXT_HCI_UART_CHANNEL, UART_RXDONE_MASK);

    plic_interrupt_enable(EXT_HCI_UART_IRQ);

     //cts function
     if((uart->HwFlowCtl != 0) && (uart->cts_Pin != 0))
     {
        uart_cts_config(EXT_HCI_UART_CHANNEL,uart->cts_Pin,STOP_VOLT);
        uart_set_cts_en(EXT_HCI_UART_CHANNEL);

     }
     //rts function
     if((uart->HwFlowCtl != 0) && (uart->rts_Pin != 0))
     {
            uart_set_rts_en(EXT_HCI_UART_CHANNEL);
            uart_rts_config(EXT_HCI_UART_CHANNEL,uart->rts_Pin,RTS_INVERT,UART_RTS_MODE_AUTO);
            uart_rts_trig_level_auto_mode(EXT_HCI_UART_CHANNEL, RTS_THRESH);
     }
     uart_dma_send_flag = 1;
     TxCpltCallback = uart->TxCpltCallback;
     RxCpltCallback = uart->RxCpltCallback;
     return EXT_UART_OK;
}

/**
 * @brief  uart interrupt function
 */
_attribute_ram_code_
_attribute_ram_code_sec_ void ext_hci_irq_handler(void){
    //transmit
     if(uart_get_irq_status(EXT_HCI_UART_CHANNEL,UART_TXDONE_IRQ_STATUS))
     {
         uart_clr_irq_status(EXT_HCI_UART_CHANNEL,UART_TXDONE_IRQ_STATUS);
         uart_dma_send_flag = 1; //clean flag

         if(TxCpltCallback != NULL)
         {
             TxCpltCallback(NULL);
         }
     }

     //receive
     if(uart_get_irq_status(EXT_HCI_UART_CHANNEL,UART_RXDONE_IRQ_STATUS))
     {
         if((uart_get_irq_status(EXT_HCI_UART_CHANNEL,UART_RX_ERR)))
         {
            uart_clr_irq_status(EXT_HCI_UART_CHANNEL,UART_RXBUF_IRQ_STATUS);
         }
            /************************get the length of receive data****************************/
         unsigned int rev_data_len = uart_get_dma_rev_data_len(EXT_HCI_UART_CHANNEL,EXT_HCI_UART_DMA_CHN_RX);
            /************************clr rx_irq****************************/
         uart_clr_irq_status(EXT_HCI_UART_CHANNEL,UART_RXDONE_IRQ_STATUS);
         unsigned char  * addr = ( unsigned char  *)&ReceAddr[0]-4;
         addr[3] = (rev_data_len >> 24);
         addr[2] = rev_data_len >> 16;
         addr[1] = rev_data_len >> 8;
         addr[0] = rev_data_len ;
         if(RxCpltCallback != NULL)
         {
            RxCpltCallback(NULL);
         }
      }
}
PLIC_ISR_REGISTER(ext_hci_irq_handler, EXT_HCI_UART_IRQ )




/**
 * @brief  Check whether the transmission is complete
 * @param  none
 * @retval 0: busy  1:idle
 */
unsigned char ext_hci_getTxCompleteDone(void)
{
    return uart_dma_send_flag;
}

/**
 * @brief       This function serves to send data by DMA, this function tell the DMA to get data from the RAM and start.
 * @param[in]   addr     - pointer to the buffer containing data need to send.
 * @param[in]   len      - DMA transmission length.The maximum transmission length of DMA is 0xFFFFFC bytes, so dont'n over this length.
 * @return      1  dma start send.
 *              0  the length is error.
 * @note        addr: must be aligned by word (4 bytes), otherwise the program will enter an exception.
 */
_attribute_ram_code_sec_
unsigned char ext_hci_uartSendData(unsigned char *addr, unsigned int len)
{
    unsigned char ret_val;
    core_interrupt_disable();
    uart_dma_send_flag = 0;
    ret_val = uart_send_dma(EXT_HCI_UART_CHANNEL, addr, len);
    core_interrupt_enable();
    return ret_val;
}

_attribute_ram_code_sec_  //BLE SDK use:
void ext_hci_uartReceData(unsigned char *addr, unsigned int len)
{
    core_interrupt_disable();
    uart_receive_dma(EXT_HCI_UART_CHANNEL, addr, len);
    ReceAddr = addr;
    core_interrupt_enable();
}
#endif

