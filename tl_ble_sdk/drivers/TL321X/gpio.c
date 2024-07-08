/********************************************************************************************************
 * @file    gpio.c
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

/**********************************************************************************************************************
 *                                            local constants                                                       *
 *********************************************************************************************************************/


/**********************************************************************************************************************
 *                                              local macro                                                        *
 *********************************************************************************************************************/


/**********************************************************************************************************************
 *                                             local data type                                                     *
 *********************************************************************************************************************/


/**********************************************************************************************************************
 *                                              global variable                                                       *
 *********************************************************************************************************************/

/**********************************************************************************************************************
 *                                              local variable                                                     *
 *********************************************************************************************************************/

/**********************************************************************************************************************
 *                                          local function prototype                                               *
 *********************************************************************************************************************/

/**********************************************************************************************************************
 *                                         global function implementation                                             *
 *********************************************************************************************************************/

/**
 * @brief      This function serves to set the gpio-mux function.
 * @param[in]  pin      - the pin needs to set.
 * @param[in]  function - the function need to set.
 * @return     none.
 */
void gpio_set_mux_function(gpio_func_pin_e pin,gpio_func_e function)
{
    reg_gpio_func_mux(pin)=function;
}
/**
 * @brief      This function enable the input function of a pin.
 * @param[in]  pin - the pin needs to set the input function.
 * @return     none.
 */
void gpio_input_en(gpio_pin_e pin)
{
    unsigned char   bit = pin & 0xff;
    unsigned short group = pin & 0xf00;
    if((pin == GPIO_PB4) || (pin == GPIO_PB5) || (pin == GPIO_PB6) || (pin == GPIO_PB7))
    {
        analog_write_reg8(areg_gpio_pb_ie, analog_read_reg8(areg_gpio_pb_ie)|bit);
    }else if(group == GPIO_GROUPC)
    {
        analog_write_reg8(areg_gpio_pc_ie, analog_read_reg8(areg_gpio_pc_ie)|bit);
    }else
    {
        BM_SET(reg_gpio_ie(pin), bit);
    }
}

/**
 * @brief      This function disable the input function of a pin.
 * @param[in]  pin - the pin needs to set the input function.
 * @return     none.
 */
void gpio_input_dis(gpio_pin_e pin)
{
    unsigned char   bit = pin & 0xff;
    unsigned short group = pin & 0xf00;
    if((pin == GPIO_PB4) || (pin == GPIO_PB5) || (pin == GPIO_PB6) || (pin == GPIO_PB7))
    {
        analog_write_reg8(areg_gpio_pb_ie, analog_read_reg8(areg_gpio_pb_ie)&(~bit));
    }else if(group == GPIO_GROUPC)
    {
        analog_write_reg8(areg_gpio_pc_ie, analog_read_reg8(areg_gpio_pc_ie)&(~bit));
    }else
    {
        BM_CLR(reg_gpio_ie(pin), bit);
    }
}

/**
 * @brief      This function set the input function of a pin.
 * @param[in]  pin - the pin needs to set the input function
 * @param[in]  value - enable or disable the pin's input function(1: enable,0: disable )
 * @return     none
 */
void gpio_set_input(gpio_pin_e pin, unsigned char value)
{
    if(value)
    {
        gpio_input_en(pin);
    }
    else
    {
        gpio_input_dis(pin);
    }
}

/**
 * @brief      This function servers to set the specified GPIO as high resistor.
 * @param[in]  pin  - select the specified GPIO, GPIOI GPIOJ group is not included in GPIO_ALL.
 * @return     none.
 * @note       -# gpio_shutdown(GPIO_ALL) is a debugging method only and is not recommended for use in applications.
 *             -# gpio_shutdown(GPIO_ALL) set all GPIOs to high impedance except SWS and MSPI.
 *             -# If you want to use JTAG/USB in active state, or wake up the MCU with a specific pin,
 *                you can enable the corresponding pin after calling gpio_shutdown(GPIO_ALL).
 */
void gpio_shutdown(gpio_pin_e pin)
{
    unsigned short group = pin & 0xf00;
    unsigned char bit = pin & 0xff;
    switch(group)
    {
        case GPIO_GROUPA:
            reg_gpio_pa_oen |= bit;//disable output
            reg_gpio_pa_gpio |= (bit&0x7f);
            reg_gpio_pa_ie &= ((~bit)|0x80);//disable input
            break;
        case GPIO_GROUPB:
            reg_gpio_pb_oen |= bit;
            reg_gpio_pb_gpio |= bit;
            if((pin == GPIO_PB4) || (pin == GPIO_PB5) || (pin == GPIO_PB6) || (pin == GPIO_PB7))
            {
                analog_write_reg8(areg_gpio_pb_ie, analog_read_reg8(areg_gpio_pb_ie)&(~bit));
            }
            else
            {
                reg_gpio_pb_ie &= (~bit);
            }
            break;
        case GPIO_GROUPC:
            reg_gpio_pc_oen |= bit;
            reg_gpio_pc_gpio |= bit;
            analog_write_reg8(areg_gpio_pc_ie, analog_read_reg8(areg_gpio_pc_ie) & (~bit));
            break;
        case GPIO_GROUPD:
            reg_gpio_pd_oen |= bit;
            reg_gpio_pd_gpio |= bit;
            reg_gpio_pd_ie &= (~bit);
            break;

        case GPIO_GROUPE:
            reg_gpio_pe_oen |= bit;
            reg_gpio_pe_gpio |= bit;
            reg_gpio_pe_ie &= (~bit);
            break;
        case GPIO_GROUPF:
            reg_gpio_pf_oen |= bit;
            reg_gpio_pf_gpio |= bit;
            reg_gpio_pf_ie &= (~bit);
            break;

        case GPIO_ALL:
        {
            //as gpio
            reg_gpio_pa_gpio = 0x7f;
            reg_gpio_pb_gpio = 0xff;
            reg_gpio_pc_gpio = 0xff;
            reg_gpio_pd_gpio = 0xff;
            reg_gpio_pe_gpio = 0xff;
            // reg_gpio_pf_gpio = 0xff;

            //output disable
            reg_gpio_pa_oen = 0xff;
            reg_gpio_pb_oen = 0xff;
            reg_gpio_pc_oen = 0xff;
            reg_gpio_pd_oen = 0xff;
            reg_gpio_pe_oen = 0xff;
            // reg_gpio_pf_oen = 0xff;

            //disable input
            reg_gpio_pa_ie = 0x80;  //SWS input is enabled
            reg_gpio_pb_ie &= 0xf0;
            analog_write_reg8(areg_gpio_pb_ie, (analog_read_reg8(areg_gpio_pb_ie) & 0x0f));
            analog_write_reg8(areg_gpio_pc_ie, 0);
            reg_gpio_pd_ie = 0x00;
            reg_gpio_pe_ie = 0x00;
            // reg_gpio_pf_ie = 0x00;
        }
    }
}

/**
 * @brief     This function set a pin's IRQ , here you can choose from 8 interrupts for flexible configuration, each interrupt is independent.
 * @param[in] irq           - there are 8 types of irq to choose.(irq0/irq1/irq2/irq3/irq4/irq5/irq6/irq7)
 * @param[in] pin           - the pin needs to enable its IRQ.
 * @param[in] trigger_type  - gpio interrupt type.
 *                            0: rising edge.
 *                            1: falling edge.
 *                            2: high level.
 *                            3: low level.
 * @return    none.
 */
void gpio_set_irq(gpio_irq_num_e irq, gpio_pin_e pin, gpio_irq_trigger_type_e trigger_type)
{
     /*
        When selecting pull-up resistance and rising edge to trigger gpio interrupt, gpio_irq_en should be placed before setting gpio_set_irq,
        otherwise an interrupt will be triggered by mistake.
     */
    gpio_irq_en(pin, irq);
    gpio_clr_irq_status((gpio_irq_e)BIT(irq));//must clear cause to unexpected interrupt.
    switch(trigger_type)
    {
    case INTR_RISING_EDGE:
        BM_CLR(reg_gpio_pol(pin), pin & 0xff);
        BM_CLR(reg_gpio_irq_level, BIT(irq));
    break;
    case INTR_FALLING_EDGE:
        BM_SET(reg_gpio_pol(pin), pin & 0xff);
        BM_CLR(reg_gpio_irq_level, BIT(irq));
    break;
    case INTR_HIGH_LEVEL:
        BM_CLR(reg_gpio_pol(pin), pin & 0xff);
        BM_SET(reg_gpio_irq_level, BIT(irq));
    break;
    case INTR_LOW_LEVEL:
        BM_SET(reg_gpio_pol(pin), pin & 0xff);
        BM_SET(reg_gpio_irq_level, BIT(irq));
     break;
    }
    if(irq == GPIO_IRQ0){
        reg_gpio_irq_ctrl |= FLD_GPIO_CORE_INTERRUPT_EN;//Only GPIO_IRQ0 needs to enable FLD_GPIO_CORE_INTERRUPT_EN.
    }
}

/**
 * @brief     This function set a pin's pull-up/down resistor.
 * @param[in] pin - the pin needs to set its pull-up/down resistor.
 * @param[in] up_down_res - the type of the pull-up/down resistor.
 * @return    none.
 */
void gpio_set_up_down_res(gpio_pin_e pin, gpio_pull_type_e up_down_res)
{
    unsigned char r_val = up_down_res & 0x03;

    unsigned char base_ana_reg = 0;
    if((pin>>8)<5)//A-E
    {
         base_ana_reg = 0x17 + ((pin >> 8) << 1) + ((pin & 0xf0) ? 1 : 0 );
    }
    else{
        return;
    }
    unsigned char shift_num, mask_not;

    if(pin & 0x11){
            shift_num = 0;
            mask_not = 0xfc;
        }
        else if(pin & 0x22){
            shift_num = 2;
            mask_not = 0xf3;
        }
        else if(pin & 0x44){
            shift_num = 4;
            mask_not = 0xcf;
        }
        else if(pin & 0x88){
            shift_num = 6;
            mask_not = 0x3f;
        }
        else{
            return;
        }
    analog_write_reg8(base_ana_reg, (analog_read_reg8(base_ana_reg) & mask_not) | (r_val << shift_num));
}

/**
 * @brief     This function set probe clk output.
 * @param[in] pin
 * @param[in] sel_clk
 * @return    none.
 */
void gpio_set_probe_clk_function(gpio_func_pin_e pin, probe_clk_sel_e sel_clk)
{
    reg_probe_clk_sel= (reg_probe_clk_sel & 0xe0) | sel_clk;    //probe_clk_sel_e
    gpio_set_mux_function(pin, DBG_PROBE_CLK);              //sel probe_clk function
    gpio_function_dis((gpio_pin_e)pin);
}
/**********************************************************************************************************************
  *                                         local function implementation                                             *
  *********************************************************************************************************************/

