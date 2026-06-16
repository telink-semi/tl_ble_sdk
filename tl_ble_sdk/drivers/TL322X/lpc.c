/********************************************************************************************************
 * @file    lpc.c
 *
 * @brief   This is the source file for tl322x
 *
 * @author  Driver Group
 * @date    2024
 *
 * @par     Copyright (c) 2024, Telink Semiconductor (Shanghai) Co., Ltd. ("TELINK")
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
#include "lpc.h"
#include "lib/include/plic.h"
#include "gpio.h"
#include "lib/include/pm/pm.h"
#include "flash.h"
#include "lib/include/stimer.h"
#include "lib/include/analog.h"

unsigned char g_lpc_pem_chn = 0xff;

pem_event_config_t misc_pem_event_config={
            .module         = PEM_EVENT_MISC,
            .sig_sel        = 0,
            .clk_sel        = ASYNC_CLK,
            .lvl            = LEVEL,
            .edge_detect    = RISING_EDGE,
            .inv            = 0,
};

pem_task_config_t dma_pem_task_config={
            .module         = PEM_TASK_DMA,
            .sig_sel        = 0,
            .clk_sel        = HCLK,
            .lvl            = PULSE,
};

dma_config_t register_rx_dma_config=
{
    .dst_req_sel= 0,//do not care
    .src_req_sel=0,//do not care
    .dst_addr_ctrl=DMA_ADDR_INCREMENT,
    .src_addr_ctrl=DMA_ADDR_FIX,
    .dstmode=DMA_NORMAL_MODE,
    .srcmode=DMA_NORMAL_MODE,
    .dstwidth=DMA_CTR_WORD_WIDTH,//must word
    .srcwidth=DMA_CTR_WORD_WIDTH,//must word
    .src_burst_size=0,//must 0
    .read_num_en=0,
    .priority=0,
    .write_num_en=0,
    .auto_en=0,//must 0
};

/**
 * @brief       This function selects input reference voltage for low power comparator.
 * @param[in]   mode    - lower power comparator working mode includes normal mode and low power mode.
 * @param[in]   ref     - selected input reference voltage.
 * @return      none.
 */
void lpc_set_input_ref(lpc_mode_e mode, lpc_reference_e ref)
{
    if (mode == LPC_LOWPOWER) {
        //switch uvlo vref
        analog_write_reg8(0x0b, analog_read_reg8(0x0b) & 0xf7);//0x0b[3] = 0, ref by uvlo
        analog_write_reg8(0x0d, analog_read_reg8(0x0d) | 0x80);
    } else if (mode == LPC_NORMAL) {
        //switch BG vref
        analog_write_reg8(0x0b, analog_read_reg8(0x0b) | 0x08);//0x0b[3] = 1, ref by BG
        analog_write_reg8(0x0d, analog_read_reg8(0x0d) & 0x7f);
    }
    analog_write_reg8(0x0d, (analog_read_reg8(0x0d) & 0x8f) | (ref << 4));
}

/**
 * @brief     This function servers to configure the PEM event in MISC module.
 * @param[in] chn - to select the PEM channel.
 * @param[in] misc_sel - to setting serves to set the configuration of MISC PEM event.
 * @return    none
 */
void lpc_set_misc_pem_event(pem_chn_e chn,unsigned char misc_sel)
{
    misc_pem_event_config.sig_sel = misc_sel;
    pem_event_config(chn, misc_pem_event_config);
    //misc module No pem_event_en signal, no need to configure.(confirmed by jianzhi)
}

/**
 * @brief     This function servers to configure the pem task in DMA module.
 * @param[in] chn - to select the PEM channel.
 * @param[in] dma_chn - to setting serves to set the configuration of DMA PEM task.
 * @return    none
 * @note      only DMA0-7 support PEM feature
 */
void lpc_set_dma_pem_task(pem_chn_e chn,dma_chn_e dma_chn)
{
    dma_pem_task_config.sig_sel = dma_chn;
    pem_task_config(chn,dma_pem_task_config);
    reg_dma_pem_task_en |= BIT(dma_chn);
}

/**
 * @brief     This function servers to configure the DMA to write values to registers.
 * @param[in] dma_chn - to select the DMA channel.
 * @param[in] src_data_buf  - Pointer to data buffer, it must be 4-bytes aligned address
 * @param[in] dst_reg_addr - the register address of destination.
 * @return    none
 * @note      only DMA0-7 support PEM feature
 */
void lpc_pem_dma_task_set_config(dma_chn_e dma_chn,unsigned int *src_data_buf,unsigned int dst_reg_addr)
{
    dma_config(dma_chn, &register_rx_dma_config);
    dma_set_address(dma_chn,(unsigned int)src_data_buf,dst_reg_addr);
    dma_set_size(dma_chn,4,DMA_WORD_WIDTH);
    dma_chn_dis(dma_chn);//dma must be disable(confirmed by jianzhi)
}

/**
 * @brief       This function serves to use LPC to trigger DMA via PEM to write the value to reg_rst to hold all reset signals, thereby protecting the flash during the chip power-down process.
 * @param[in]   lpc_chn - selected input channel.Input derived from external PortB(PB<1>~PB<7>).
 * @param[in]   pem_chn - to select the PEM channel.
 * @param[in]   dma_chn - to select the DMA channel.
 * @return      none.
 * @note       -# In order to improve the robustness of the chip during high-speed operation, the low power comparator (LPC) is used to
 *              protect the flash during power-down of the chip.
 *              When this feature is enabled, there are the following limitations:
 *               -# The chip power supply voltage is limited to 2.1V to 4.2V.
 *               -# One of PB[1:7] must be reserved for this feature.
 *               -# One of PEM must be reserved for this feature.
 *               -# One of DMA0-7 must be reserved for this feature.
 *               -# Before enabling this feature, activate the 32k watchdog or timer watchdog to prevent the LPC from triggering due to power supply voltage fluctuations. 
 *                  This prevents the MCU from stalling and the chip from being unable to reset for an extended period.
 */
void lpc_pem_flash_prot_config(lpc_input_channel_e lpc_chn, pem_chn_e pem_chn, dma_chn_e dma_chn)
{
    unsigned int register_data= 0x00010000;//write 0 to register reg_rst to hold all reset signal(especially the MSPI and MCU),but exclude timer watchdog
    unsigned int *ptr= &register_data;

    gpio_function_en(GPIO_GROUPB | (1<<lpc_chn));
    gpio_set_up_down_res(GPIO_GROUPB | (1<<lpc_chn), GPIO_PIN_UP_DOWN_FLOAT);//must be configured as float to prevent being pulled down in gpio_shutdown
    gpio_output_en(GPIO_GROUPB | (1<<lpc_chn));    //enable output
    gpio_input_dis(GPIO_GROUPB | (1<<lpc_chn));    //disable input
    gpio_set_level(GPIO_GROUPB | (1<<lpc_chn), 1);

    lpc_set_input_chn(lpc_chn);
    lpc_set_input_ref(LPC_NORMAL, LPC_REF_974MV);
    lpc_set_scaling_coeff(LPC_SCALING_PER50);
    lpc_power_on();
    delay_us(64);

    pem_init();
    lpc_set_misc_pem_event(pem_chn,6);//misc_sel=6: select lpc module as event to trigger task module
    lpc_set_dma_pem_task(pem_chn,dma_chn);
    lpc_pem_dma_task_set_config(dma_chn,(unsigned int *)ptr, (unsigned int)reg_rst_addr);//Use DMA to write src_data_buf to register reg_rst
    pem_chn_en(pem_chn);
    g_lpc_pem_chn = pem_chn;
}

/**
 * @brief       This function is used to initialize GPIO voltage detection.
 * @param[in]   mode    - lower power comparator working mode includes normal mode and low power mode.
 * @param[in]   pin     - selected input channel.Input derived from external PortB(PB<1>~PB<7>).
 * @param[in]   ref     - selected input reference voltage.
 * @param[in]   divider - selected scaling coefficient.(%25,%50,%75,%100)
 * @return      none.
 */
void lpc_gpio_vol_detect_init(lpc_mode_e mode, lpc_input_channel_e pin, lpc_reference_e ref,lpc_scaling_e divider)
{
    lpc_set_input_chn(pin);
    lpc_set_input_ref(mode,ref);
    lpc_set_scaling_coeff(divider);
}
