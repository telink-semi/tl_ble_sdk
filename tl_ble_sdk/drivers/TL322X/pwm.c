/********************************************************************************************************
 * @file    pwm.c
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
#include "pwm.h"
#define PWM_MAX_NUM 23


dma_config_t pwm_tx_dma_config = {
    .dst_req_sel    = DMA_REQ_PWM_TX,     //tx req
    .src_req_sel    = 0,
    .dst_addr_ctrl  = DMA_ADDR_FIX,
    .src_addr_ctrl  = DMA_ADDR_INCREMENT, //increment
    .dstmode        = DMA_HANDSHAKE_MODE, //handshake
    .srcmode        = DMA_NORMAL_MODE,
    .dstwidth       = DMA_CTR_WORD_WIDTH, //must word
    .srcwidth       = DMA_CTR_WORD_WIDTH, //must word
    .src_burst_size = 0,                  //must 0
    .read_num_en    = 0,
    .priority       = 0,
    .write_num_en   = 0,
    .auto_en        = 0,
};

/**
 * @brief      This function serves to set the pwm function.
 * @param[in]  pin      - the pin needs to set.
 * @param[in]  function - the function need to set.
 * @return     none.
 * @note       reference gpio.h GPIO function mux types
 */
void pwm_set_pin(gpio_func_pin_e pin, gpio_func_e func)
{
    gpio_set_mux_function(pin, func);
    gpio_function_dis((gpio_pin_e)pin);
}

/**
 * @brief     This function servers to configure DMA channel and some configures.
 * @param[in] chn - to select the DMA channel.
 * @return    none
 */
void pwm_set_dma_config(dma_chn_e chn)
{
    dma_config(chn, &pwm_tx_dma_config);
}

/**
 * @brief     This function servers to configure DMA channel address and length.
 * @param[in] chn - to select the DMA channel.
 * @param[in] buf_addr - the address where DMA need to get data from SRAM.
 * @param[in] len - the length of data in SRAM.
 * @return    none
 * @note      buf_addr: must be aligned by word (4 bytes), otherwise the program will enter an exception.
 */
void pwm_set_dma_buf(dma_chn_e chn, unsigned int buf_addr, unsigned int len)
{
    dma_set_address(chn, (unsigned int)(buf_addr), reg_pwm_data_buf_adr);
    dma_set_size(chn, len, DMA_WORD_WIDTH);
}

/**
 * @brief     This function servers to enable DMA channel.
 * @param[in] chn - to select the DMA channel.
 * @return    none
 */
void pwm_ir_dma_mode_start(dma_chn_e chn)
{
    dma_chn_en(chn);
}

/**
 * @brief     This function servers to configure DMA head node.
 * @param[in] chn - to select the DMA channel.
 * @param[in] src_addr - to configure DMA source address.
 * @param[in] data_len - to configure DMA length.
 * @param[in] head_of_list - to configure the address of the next node configure.
 * @return    none
 * @note      src_addr: must be aligned by word (4 bytes), otherwise the program will enter an exception.
 */
void pwm_set_dma_chain_llp(dma_chn_e chn, unsigned short *src_addr, unsigned int data_len, dma_chain_config_t *head_of_list)
{
    dma_config(chn, &pwm_tx_dma_config);
    dma_set_address(chn, (unsigned int)(src_addr), reg_pwm_data_buf_adr);
    dma_set_size(chn, data_len, DMA_WORD_WIDTH);
    reg_dma_llp(chn) = (unsigned int)(head_of_list);
}

/**
 * @brief     This function servers to configure DMA cycle chain node.
 * @param[in] chn - to select the DMA channel.
 * @param[in] config_addr  - to servers to configure the address of the current node.
 * @param[in] llpoint - to configure the address of the next node configure.
 * @param[in] src_addr - to configure DMA source address.
 * @param[in] data_len - to configure DMA length.
 * @return    none
 * @note      src_addr: must be aligned by word (4 bytes), otherwise the program will enter an exception.
 */
void pwm_set_tx_dma_add_list_element(dma_chn_e chn, dma_chain_config_t *config_addr, dma_chain_config_t *llpoint, unsigned short *src_addr, unsigned int data_len)
{
    config_addr->dma_chain_ctl      = reg_dma_ctrl(chn) | BIT(0);
    config_addr->dma_chain_src_addr = (unsigned int)(src_addr);
    config_addr->dma_chain_dst_addr = reg_pwm_data_buf_adr;
    config_addr->dma_chain_data_len = dma_cal_size(data_len, DMA_WORD_WIDTH);
    config_addr->dma_chain_llp_ptr  = (unsigned int)(llpoint);
}
/**
 * @brief     This function checks whether the interrupt type belongs to PWM0 special interrupt group.
 * @param[in] type - interrupt type.
 * @return    true if it is PWM0 special interrupt, otherwise false.
 * @note      PWM0 has additional interrupt sources (PNUM/DMA FIFO/FIFO) not shared by other PWM channels.
 */
static bool is_pwm0_special_irq(pwm_irq_type_e type)
{
    return (type == FLD_PWM0_PNUM_IRQ || type == FLD_PWM0_IR_DMA_FIFO_IRQ || type == FLD_PWM0_IR_FIFO_IRQ);
}

/**
 * @brief     This function enables PWM interrupt mask.
 * @param[in] id   - PWM channel id.
 * @param[in] type - interrupt type.
 * @return    none
 */
void pwm_set_irq_mask(pwm_id_e id, pwm_irq_type_e type)
{
    if (id == PWM0_ID && is_pwm0_special_irq(type)) {
        unsigned char reg_val = reg_pwm_ctrl_b2;

        switch (type) {
            case FLD_PWM0_PNUM_IRQ:
                reg_val |= FLD_PWM0_PNUM_IRQ;
                break;
            case FLD_PWM0_IR_DMA_FIFO_IRQ:
                reg_val |= FLD_PWM0_IR_DMA_FIFO_IRQ;
                break;
            case FLD_PWM0_IR_FIFO_IRQ:
                reg_val |= FLD_PWM0_IR_FIFO_IRQ;
                break;
            default:
                return;
        }

        reg_pwm_ctrl_b2 = reg_val;
        return;
    }

    if (type == FLD_PWM_FRAME_DONE_IRQ) {
        reg_pwm_cfg_b6(id) |= FLD_PWM_IRQ_CYCDONE_MASK;
    }
}

/**
 * @brief     This function disables PWM interrupt mask.
 * @param[in] id   - PWM channel id.
 * @param[in] type - interrupt type.
 * @return    none
 */
void pwm_clr_irq_mask(pwm_id_e id, pwm_irq_type_e type)
{
    if (id == PWM0_ID && is_pwm0_special_irq(type)) {
        unsigned char reg_val = reg_pwm_ctrl_b2;

        switch (type) {
            case FLD_PWM0_PNUM_IRQ:
                reg_val &= ~FLD_PWM0_PNUM_IRQ;
                break;
            case FLD_PWM0_IR_DMA_FIFO_IRQ:
                reg_val &= ~FLD_PWM0_IR_DMA_FIFO_IRQ;
                break;
            case FLD_PWM0_IR_FIFO_IRQ:
                reg_val &= ~FLD_PWM0_IR_FIFO_IRQ;
                break;
            default:
                return;
        }

        reg_pwm_ctrl_b2 = reg_val;
        return;
    }

    if (type == FLD_PWM_FRAME_DONE_IRQ) {
        reg_pwm_cfg_b6(id) &= ~FLD_PWM_IRQ_CYCDONE_MASK;
    }
}

/**
 * @brief     This function gets PWM interrupt status.
 * @param[in] id   - PWM channel id.
 * @param[in] type - interrupt type.
 * @return    interrupt status (0 or non-zero).
 */
unsigned int pwm_get_irq_status(pwm_id_e id, pwm_irq_type_e type)
{
    if (id == PWM0_ID && is_pwm0_special_irq(type)) {
        unsigned char reg_val = reg_pwm_int;

        switch (type) {
            case FLD_PWM0_PNUM_IRQ:
                return (reg_val & FLD_PWM0_PNUM_IRQ);
            case FLD_PWM0_IR_DMA_FIFO_IRQ:
                return (reg_val & FLD_PWM0_IR_DMA_FIFO_IRQ);
            case FLD_PWM0_IR_FIFO_IRQ:
                return (reg_val & FLD_PWM0_IR_FIFO_IRQ);
            default:
                return 0;
        }
    }

    if (type == FLD_PWM_FRAME_DONE_IRQ) {
        unsigned int reg_val = reg_pwm_irq_sta;
        unsigned int valid_mask = BIT(id) & ((1 << PWM_MAX_NUM) - 1);
        return reg_val & valid_mask;
    }

    return 0;
}

/**
 * @brief     This function clears PWM interrupt status.
 * @param[in] id   - PWM channel id.
 * @param[in] type - interrupt type.
 * @return    none
 * @note      Write 1 to clear interrupt flag (W1C).
 */
void pwm_clr_irq_status(pwm_id_e id, pwm_irq_type_e type)
{
    if (id == PWM0_ID && is_pwm0_special_irq(type)) {
        unsigned char clear_val = 0;

        switch (type) {
            case FLD_PWM0_PNUM_IRQ:
                clear_val = FLD_PWM0_PNUM_IRQ;
                break;
            case FLD_PWM0_IR_DMA_FIFO_IRQ:
                clear_val = FLD_PWM0_IR_DMA_FIFO_IRQ;
                break;
            case FLD_PWM0_IR_FIFO_IRQ:
                clear_val = FLD_PWM0_IR_FIFO_IRQ;
                break;
            default:
                return;
        }

        reg_pwm_int = clear_val;
        return;
    }

    if (type == FLD_PWM_FRAME_DONE_IRQ) {
        unsigned int valid_mask = BIT(id) & ((1 << PWM_MAX_NUM) - 1);
        if (valid_mask) {
            reg_pwm_irq_sta = valid_mask;
        }
    }
}

