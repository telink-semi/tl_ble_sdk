/********************************************************************************************************
 * @file    stimer.c
 *
 * @brief   This is the source file for TL321X
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
#include "gpio.h"
#include "stimer.h"
#include "clock.h"
#include "pwm.h"
#include "dma.h"

unsigned int g_track_32kcnt=16;

/**
 * @brief      This setting serves to set the configuration of stimer input capture DMA.
 */
dma_config_t stimer_rx_dma_config={
    .dst_req_sel        = 0,
    .src_req_sel        = DMA_REQ_STIMER_RX,
    .dst_addr_ctrl      = DMA_ADDR_INCREMENT,
    .src_addr_ctrl      = DMA_ADDR_FIX,
    .dstmode            = DMA_NORMAL_MODE,
    .srcmode            = DMA_HANDSHAKE_MODE,
    .dstwidth           = DMA_CTR_WORD_WIDTH,
    .srcwidth           = DMA_CTR_WORD_WIDTH,
    .src_burst_size     = DMA_BURST_1_WORD,
    .vacant_bit         = 0,
    .read_num_en        = 0,
    .priority           = 0,
    .write_num_en       = 0,
    .auto_en            = 0,
};

/**
 * @brief     This function performs to set delay time by us.
 * @param[in] microsec - need to delay.
 * @return    none
 */
_attribute_ram_code_sec_noinline_ void delay_us(unsigned int microsec)
{
    unsigned long t = stimer_get_tick();
    while(!clock_time_exceed(t, microsec)){
    }
}

/**
 * @brief     This function performs to set delay time by ms.
 * @param[in] millisec - need to delay.
 * @return    none
 */
_attribute_ram_code_sec_noinline_ void delay_ms(unsigned int millisec)
{
    unsigned long t = stimer_get_tick();
    while(!clock_time_exceed(t, millisec*1000)){
    }
}

/**
 * @brief       This function is used to start the system timer.
 * @param[in]   mode    - starting mode.
 * @param[in]   tick    - The initial value of the tick at startup.
 * @return      none.
 */
_attribute_ram_code_sec_noinline_ void stimer_enable(stimer_enable_mode_e mode, unsigned int tick)
{
    if(STIMER_MANUAL_MODE == mode)
    {
        stimer_set_manual_enable_mode();
        stimer_set_tick(tick);
        stimer_enable_in_manual_mode();
    }
    else if(STIMER_AUTO_MODE_W_TRIG == mode)
    {
        stimer_set_auto_enable_mode();
        stimer_set_tick(tick);
    }
    else if(STIMER_AUTO_MODE_W_AND_NXT_32K_START == mode)
    {
        stimer_set_run_upon_nxt_32k_enable();   //system tick set upon next 32k posedge.
        stimer_set_auto_enable_mode();
    }
    else if(STIMER_AUTO_MODE_W_AND_NXT_32K_DONE == mode)
    {
        stimer_set_tick(tick);
        //wait command set delay done upon next 32k posedge.
        //if not using status bit, wait at least 1 32k cycle to set register r_run_upon_next_32k back to 0, or before next normal set
        stimer_wait_write_done();               //system timer set done status upon next 32k posedge
        stimer_set_run_upon_nxt_32k_disable();  //normal system tick update
    }
}

/**
 * @brief       This function is used to stop the system timer.
 * @return      none.
 */
_attribute_ram_code_sec_noinline_ void stimer_disable(void)
{
    if(stimer_get_enable_mode())    //auto mode
    {
        if(reg_system_ctrl & FLD_SYSTEM_TIMER_EN)   //abnormal condition
        {
            stimer_set_manual_enable_mode();
            reg_system_ctrl &= ~(FLD_SYSTEM_TIMER_EN);
        }
        else
        {
            reg_system_st = FLD_SYSTEM_CMD_STOP;    //Note: Write the corresponding bit directly here.
            stimer_set_manual_enable_mode();
        }
    }
    else    //manual mode enable
    {
        reg_system_ctrl &= ~(FLD_SYSTEM_TIMER_EN);
    }
}

/**
 * @brief       This function serves to set the input capture function.
 *              If this feature is used, it will take up the gpio_irq1 interrupt and cannot be used elsewhere.
 * @param[in]   pin         - the pin needs to set.
 * @param[in]   capt_mode   - input capture mode.
 * @return      none.
 */
void stimer_set_input_capt_pin(gpio_pin_e pin, stimer_capt_mode_e capt_mode)
{
    gpio_set_irq(GPIO_IRQ1,pin,INTR_HIGH_LEVEL);  //Only gpio_irq1 supports capture mode.
    gpio_set_irq_mask(GPIO_IRQ_IRQ1);
    stimer_set_input_capt_mode(capt_mode);
}

/**
 * @brief       This  function serves to set stimer dma.
 * @param[in]   stimer_dma_chn - dma channel.
 * @return      none
 */
void stimer_set_input_capt_dma_config(dma_chn_e chn)
{
    dma_config(chn, &stimer_rx_dma_config);
}

/**
 * @brief       This  function serves to set stimer dma.
 * @param[in]   stimer_dma_chn - dma channel.
 * @param[in]   dst_addr    - the address of destination.
 * @param[in]   len         - the length of data.
 * @return      none
 */
void stimer_set_input_capt_value_dma(dma_chn_e chn, unsigned int *dst_addr, unsigned int len)
{
    dma_set_address(chn, reg_stimer_capt_data_addr, (unsigned int)dst_addr);
    dma_set_size(chn, len, DMA_WORD_WIDTH);
    dma_chn_en(chn);
}

/**
 * @brief     This function servers to configure DMA head node.
 * @param[in] chn          - to select the DMA channel.
 * @param[in] dst_addr     - the dma address of destination
 * @param[in] data_len     - to configure DMA length.
 * @param[in] head_of_list - the head address of dma llp.
 * @return    none
 */
void stimer_set_input_capt_dma_chain_llp(dma_chn_e chn,unsigned int* dst_addr,unsigned int len,dma_chain_config_t *head_of_list)
{
    dma_config(chn, &stimer_rx_dma_config);
    dma_set_address(chn, reg_stimer_capt_data_addr, (unsigned int)(dst_addr));
    dma_set_size(chn, len, DMA_WORD_WIDTH);
    reg_dma_llp(chn) = (unsigned int)(head_of_list);
    dma_chn_en(chn);
}

/**
 * @brief     This function servers to configure DMA cycle chain node.
 * @param[in] chn         - to select the DMA channel.
 * @param[in] config_addr - to servers to configure the address of the current node.
 * @param[in] llpointer   - to configure the address of the next node configure.
 * @param[in] dst_addr    - the dma address of destination.
 * @param[in] data_len    - to configure DMA length.
 * @return    none
 */
 void stimer_input_capt_dma_add_list_element(dma_chn_e chn,unsigned int * dst_addr,unsigned int data_len,dma_chain_config_t *config_addr,dma_chain_config_t *llpointer)
 {
    config_addr->dma_chain_ctl = reg_dma_ctrl(chn)|BIT(0);
    config_addr->dma_chain_src_addr = reg_stimer_capt_data_addr;
    config_addr->dma_chain_dst_addr = (unsigned int)(dst_addr);
    config_addr->dma_chain_data_len = dma_cal_size(data_len, DMA_WORD_WIDTH);
    config_addr->dma_chain_llp_ptr = (unsigned int)(llpointer);
 }

