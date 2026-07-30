/********************************************************************************************************
 * @file    app_interrupt.c
 *
 * @brief   This is the source file for 2.4G SDK
 *
 * @author  2.4G Group
 * @date    2023
 *
 * @par     Copyright (c) 2023, Telink Semiconductor (Shanghai) Co., Ltd. ("TELINK")
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
#include "app_config.h"
#include "tl_common.h"
#include "drivers.h"
#include "../stack/2p4g/tpll/tpll.h"
#if (MCU_CORE_TYPE == MCU_CORE_TL323X)

#if ((TPLL_MODE == TPLL_PTX) || (TPLL_MODE == TPLL_PTX_3BIT))

volatile unsigned int tx_irq_cnt_tx          = 0;
volatile unsigned int tx_irq_cnt_invalid_pid = 0;
volatile unsigned int tx_irq_cnt_max_retry   = 0;
volatile unsigned int tx_irq_cnt_tx_ds       = 0;
volatile unsigned int tx_irq_cnt_rx_dr       = 0;
volatile unsigned int math_cnt               = 0;
volatile unsigned int unmath_cnt             = 0;

volatile unsigned char rx_flag       = 0;
volatile unsigned char ds_flag       = 0;
volatile unsigned char tx_done_flag  = 0;
volatile unsigned char maxretry_flag = 0;
volatile unsigned char rx_dr_flag    = 0;
__attribute__((section(".ram_code"))) __attribute__((optimize("-Os"))) void rf_irq_handler(void)
{
    unsigned char pipe = TPLL_GetTXPipe();

    if (rf_get_irq_status(FLD_RF_IRQ_TX)) {
        reg_rf_irq_status = FLD_RF_IRQ_TX;
        tx_irq_cnt_tx++;
        tx_done_flag = 1;
    }
    if (rf_get_irq_status(FLD_RF_IRQ_INVALID_PID)) {
        reg_rf_irq_status = FLD_RF_IRQ_INVALID_PID;
        tx_irq_cnt_invalid_pid++;
    }
    if (rf_get_irq_status(FLD_RF_IRQ_TX_RETRYCNT)) {
        //tx_retrycnt_irq:Maximum number of TX retransmits interrupt.
        //Asserted when retry counter reaches the max numbers.
        reg_rf_irq_status = FLD_RF_IRQ_TX_RETRYCNT;
        tx_irq_cnt_max_retry++;
        maxretry_flag = 1;
        //adjust rptr
        TPLL_UpdateTXFifoRptr(pipe);
    }
    if (rf_get_irq_status(FLD_RF_IRQ_TX_DS)) {
        //Data Sent TX FIFO interrupt. Asserted when packet transmitted on TX.
        //If AUTO_ACK is activated, this bit is set high only when ACK is received.
        reg_rf_irq_status = FLD_RF_IRQ_TX_DS;
        tx_irq_cnt_tx_ds++;
        ds_flag = 1;
    }
    if (rf_get_irq_status(FLD_RF_IRQ_RX_DR)) {
        //rx_dr_irq:Data Ready RX FIFO interrupt. Asserted when new data arrives RX FIFO.
        //The RX_DR IRQ is asserted by a new packet arrival even
        reg_rf_irq_status = FLD_RF_IRQ_RX_DR;
        tx_irq_cnt_rx_dr++;
        rx_dr_flag = 1;
    }
    if (rf_get_irq_status(FLD_RF_IRQ_PKT_MATCH)) {
        math_cnt++;
        rf_clr_irq_status(FLD_RF_IRQ_PKT_MATCH);
    }
    if (rf_get_irq_status(FLD_RF_IRQ_PKT_UNMATCH)) {
        unmath_cnt++;
        rf_clr_irq_status(FLD_RF_IRQ_PKT_UNMATCH);
    } else {
        rf_clr_irq_status(FLD_RF_IRQ_ALL);
    }
}
PLIC_ISR_REGISTER(rf_irq_handler, IRQ_ZB_RT)
#elif ((TPLL_MODE == TPLL_PRX) || (TPLL_MODE == TPLL_PRX_3BIT))
volatile unsigned int rx_irq_cnt_rx_dr       = 0;
volatile unsigned int rx_irq_cnt_invalid_pid = 0;
volatile unsigned int rx_irq_cnt_rx          = 0;
volatile unsigned int rx_irq_cnt_tx          = 0;
volatile unsigned int rx_irq_cnt_tx_ds       = 0;
volatile unsigned int math_cnt               = 0;
volatile unsigned int unmath_cnt             = 0;

volatile unsigned char rx_flag       = 0;
volatile unsigned char ds_flag       = 0;
volatile unsigned char tx_done_flag  = 0;
volatile unsigned char maxretry_flag = 0;
volatile unsigned char rx_dr_flag    = 0;

__attribute__((section(".ram_code"))) __attribute__((optimize("-Os"))) void rf_irq_handler(void)
{
    if (rf_get_irq_status(FLD_RF_IRQ_RX_DR)) {
        //rx_dr_irq:Data Ready RX FIFO interrupt. Asserted when new data arrives RX FIFO.
        //The RX_DR IRQ is asserted by a new packet arrival even
        reg_rf_irq_status = FLD_RF_IRQ_RX_DR;
        rx_irq_cnt_rx_dr++;
    }
    if (rf_get_irq_status(FLD_RF_IRQ_INVALID_PID)) {
        //rx_invld_pid_irq:received PID invalid interrupt.
        //Asserted when received PID !== expect_pid
        reg_rf_irq_status = FLD_RF_IRQ_INVALID_PID;
        rx_irq_cnt_invalid_pid++;
    }
    if (rf_get_irq_status(FLD_RF_IRQ_TX)) {
        reg_rf_irq_status = FLD_RF_IRQ_TX;
        rx_irq_cnt_tx++;
    }
    if (rf_get_irq_status(FLD_RF_IRQ_RX)) {
        reg_rf_irq_status = FLD_RF_IRQ_RX;
        rx_irq_cnt_rx++;
        rx_flag = 1;
    }
    if (rf_get_irq_status(FLD_RF_IRQ_TX_DS)) {
        //Data Sent TX FIFO interrupt. Asserted when packet transmitted on TX.
        //If AUTO_ACK is activated, this bit is set high only when ACK is received.
        reg_rf_irq_status = FLD_RF_IRQ_TX_DS;
        rx_irq_cnt_tx_ds++;
    }
    if (rf_get_irq_status(FLD_RF_IRQ_PKT_MATCH)) {
        math_cnt++;
        rf_clr_irq_status(FLD_RF_IRQ_PKT_MATCH);
    }
    if (rf_get_irq_status(FLD_RF_IRQ_PKT_UNMATCH)) {
        unmath_cnt++;
        rf_clr_irq_status(FLD_RF_IRQ_PKT_UNMATCH);
    } else {
        rf_clr_irq_status(FLD_RF_IRQ_ALL);
    }
}
PLIC_ISR_REGISTER(rf_irq_handler, IRQ_ZB_RT)
#endif



#else //(MCU_CORE_TYPE != MCU_CORE_TL323X)

#if (TPLL_MODE == TPLL_PTX)

volatile unsigned int tx_irq_cnt_tx          = 0;
volatile unsigned int tx_irq_cnt_invalid_pid = 0;
volatile unsigned int tx_irq_cnt_max_retry   = 0;
volatile unsigned int tx_irq_cnt_tx_ds       = 0;
volatile unsigned int tx_irq_cnt_rx_dr       = 0;
volatile unsigned int math_cnt               = 0;
volatile unsigned int unmath_cnt             = 0;

volatile unsigned char rx_flag       = 0;
volatile unsigned char ds_flag       = 0;
volatile unsigned char tx_done_flag  = 0;
volatile unsigned char maxretry_flag = 0;
volatile unsigned char rx_dr_flag    = 0;
__attribute__((section(".ram_code"))) __attribute__((optimize("-Os"))) void rf_irq_handler(void)
{
    unsigned char pipe = TPLL_GetTXPipe();

    if (rf_get_irq_status(FLD_RF_IRQ_TX)) {
        reg_rf_irq_status = FLD_RF_IRQ_TX;
        tx_irq_cnt_tx++;
        tx_done_flag = 1;
    }
    if (rf_get_irq_status(FLD_RF_IRQ_INVALID_PID)) {
        reg_rf_irq_status = FLD_RF_IRQ_INVALID_PID;
        tx_irq_cnt_invalid_pid++;
    }
    if (rf_get_irq_status(FLD_RF_IRQ_TX_RETRYCNT)) {
        //tx_retrycnt_irq:Maximum number of TX retransmits interrupt.
        //Asserted when retry counter reaches the max numbers.
        reg_rf_irq_status = FLD_RF_IRQ_TX_RETRYCNT;
        tx_irq_cnt_max_retry++;
        maxretry_flag = 1;
        //adjust rptr
        TPLL_UpdateTXFifoRptr(pipe);
    }
    if (rf_get_irq_status(FLD_RF_IRQ_TX_DS)) {
        //Data Sent TX FIFO interrupt. Asserted when packet transmitted on TX.
        //If AUTO_ACK is activated, this bit is set high only when ACK is received.
        reg_rf_irq_status = FLD_RF_IRQ_TX_DS;
        tx_irq_cnt_tx_ds++;
        ds_flag = 1;
    }
    if (rf_get_irq_status(FLD_RF_IRQ_RX_DR)) {
        //rx_dr_irq:Data Ready RX FIFO interrupt. Asserted when new data arrives RX FIFO.
        //The RX_DR IRQ is asserted by a new packet arrival even
        reg_rf_irq_status = FLD_RF_IRQ_RX_DR;
        tx_irq_cnt_rx_dr++;
        rx_dr_flag = 1;
    }
    if (rf_get_irq_status(FLD_RF_IRQ_PKT_MATCH)) {
        math_cnt++;
        rf_clr_irq_status(FLD_RF_IRQ_PKT_MATCH);
    }
    if (rf_get_irq_status(FLD_RF_IRQ_PKT_UNMATCH)) {
        unmath_cnt++;
        rf_clr_irq_status(FLD_RF_IRQ_PKT_UNMATCH);
    } else {
        rf_clr_irq_status(FLD_RF_IRQ_ALL);
    }
}
PLIC_ISR_REGISTER(rf_irq_handler, IRQ_ZB_RT)
#elif (TPLL_MODE == TPLL_PRX)
volatile unsigned int rx_irq_cnt_rx_dr       = 0;
volatile unsigned int rx_irq_cnt_invalid_pid = 0;
volatile unsigned int rx_irq_cnt_rx          = 0;
volatile unsigned int rx_irq_cnt_tx          = 0;
volatile unsigned int rx_irq_cnt_tx_ds       = 0;
volatile unsigned int math_cnt               = 0;
volatile unsigned int unmath_cnt             = 0;

volatile unsigned char rx_flag       = 0;
volatile unsigned char ds_flag       = 0;
volatile unsigned char tx_done_flag  = 0;
volatile unsigned char maxretry_flag = 0;
volatile unsigned char rx_dr_flag    = 0;

__attribute__((section(".ram_code"))) __attribute__((optimize("-Os"))) void rf_irq_handler(void)
{
    if (rf_get_irq_status(FLD_RF_IRQ_RX_DR)) {
        //rx_dr_irq:Data Ready RX FIFO interrupt. Asserted when new data arrives RX FIFO.
        //The RX_DR IRQ is asserted by a new packet arrival even
        reg_rf_irq_status = FLD_RF_IRQ_RX_DR;
        rx_irq_cnt_rx_dr++;
    }
    if (rf_get_irq_status(FLD_RF_IRQ_INVALID_PID)) {
        //rx_invld_pid_irq:received PID invalid interrupt.
        //Asserted when received PID !== expect_pid
        reg_rf_irq_status = FLD_RF_IRQ_INVALID_PID;
        rx_irq_cnt_invalid_pid++;
    }
    if (rf_get_irq_status(FLD_RF_IRQ_TX)) {
        reg_rf_irq_status = FLD_RF_IRQ_TX;
        rx_irq_cnt_tx++;
    }
    if (rf_get_irq_status(FLD_RF_IRQ_RX)) {
        reg_rf_irq_status = FLD_RF_IRQ_RX;
        rx_irq_cnt_rx++;
        rx_flag = 1;
    }
    if (rf_get_irq_status(FLD_RF_IRQ_TX_DS)) {
        //Data Sent TX FIFO interrupt. Asserted when packet transmitted on TX.
        //If AUTO_ACK is activated, this bit is set high only when ACK is received.
        reg_rf_irq_status = FLD_RF_IRQ_TX_DS;
        rx_irq_cnt_tx_ds++;
    }
    if (rf_get_irq_status(FLD_RF_IRQ_PKT_MATCH)) {
        math_cnt++;
        rf_clr_irq_status(FLD_RF_IRQ_PKT_MATCH);
    }
    if (rf_get_irq_status(FLD_RF_IRQ_PKT_UNMATCH)) {
        unmath_cnt++;
        rf_clr_irq_status(FLD_RF_IRQ_PKT_UNMATCH);
    } else {
        rf_clr_irq_status(FLD_RF_IRQ_ALL);
    }
}
PLIC_ISR_REGISTER(rf_irq_handler, IRQ_ZB_RT)
#endif

#endif // (MCU_CORE_TYPE == MCU_CORE_TL323X)
