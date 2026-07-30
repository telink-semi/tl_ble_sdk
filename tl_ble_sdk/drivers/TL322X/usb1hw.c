/********************************************************************************************************
 * @file    usb1hw.c
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
#include "usb1hw.h"
#include "lib/include/stimer.h"

/**
 * @brief      This function serves to enable USB1 power and clock.
 * @return     none.
 */
void usb1hw_init(void)
{
//    pm_set_dig_module_power_switch(FLD_PD_USB_EN, PM_POWER_UP); /* usb0 and usb1 use the same bit. */

    BM_SET(reg_rst7, FLD_RST7_USB1);
    BM_SET(reg_clk_en7, FLD_CLK7_USB1_EN);

    write_reg8(SC_BASE_ADDR + 0x3b, sys_clk.pll_clk / 48); /* Split the PLL clock to the USB1 clock. */
}

/**
 * @brief      This function servers to disable the manual interrupt.
 * @param[in]  m - the irq mode needs to set
 * @return     none
 */
void usb1hw_disable_manual_interrupt(int m)
{
    BM_SET(reg_usb1_ctrl_ep_irq_mode, m);
}

/**
 * @brief      This function servers to enable the manual interrupt
 * @param[in]  m - the irq mode needs to set
 * @return     none
 */
void usb1hw_enable_manual_interrupt(int m)
{
    BM_CLR(reg_usb1_ctrl_ep_irq_mode, m);
}

/**
 * @brief      This function sends two bytes data to host via the control endpoint.
 * @param[in]  v - the two bytes data need to send.
 * @return     none
 */
void usb1hw_write_ctrl_ep_u16(unsigned short v)
{
    usb1hw_write_ctrl_ep_data(v & 0xff);
    usb1hw_write_ctrl_ep_data(v >> 8);
}

/**
 * @brief   This function reads two bytes data from host via the control endpoint.
 * @return  the two bytes data read from the control endpoint.
 */
unsigned short usb1hw_read_ctrl_ep_u16(void)
{
    unsigned short v = usb1hw_read_ctrl_ep_data();
    return (usb1hw_read_ctrl_ep_data() << 8) | v;
}

static unsigned char data_ptr = 0;

/**
 * @brief     This function servers to write the data of endpoint.
 * @param[in] ep   - selected the endpoint
 * @param[in] data - the pointer to the data for write.
 * @param[in] len  - write length.
 * @return    none
 */
void usb1hw_write_ep_data(usb1_ep_index ep, unsigned char *data, unsigned short len)
{
    /* ep8 fifo mode. */
    if ((USB1_EDP8_IN == ep) && (BM_IS_SET(reg_usb1_ep8_fifo_mode, FLD_USB1_EDP8_FIFO_MODE))) {
        data_ptr = reg_usb1_ep8_ptr & 3; /* ep8 ptr remain byte. */
        if (data_ptr) {
            /* send remain byte. */
            for (unsigned char i = 0; (i < 4 - data_ptr) && (i < len); i++) {
                unsigned int ref_tick = stimer_get_tick();
                while (reg_usb1_ep8_fifo_mode & FLD_USB1_EDP8_FULL_FLAG) {
                    if (clock_time_exceed(ref_tick, 5000)) /* some exceptions occur, such as the usb disconnecting */
                    {
                        return;
                    }
                }
                reg_usb1_ep_dat_byte(USB1_EDP8_IN, reg_usb1_ep8_ptr & 3) = data[i];
            }
            len -= (4 - data_ptr) > len ? len : (4 - data_ptr);  /* Length of data already written. */
            data += (4 - data_ptr) > len ? len : (4 - data_ptr); /* Pointer to data already written. */
        }
    }

    unsigned char word_len   = len >> 2;
    unsigned char single_len = len & 3;

    /* When the remaining size is not less than 4 bytes, the MCU moves the data according to the word length. */
    for (unsigned int i = 0; i < word_len; i++) {
        if ((USB1_EDP8_IN == ep) && (BM_IS_SET(reg_usb1_ep8_fifo_mode, FLD_USB1_EDP8_FIFO_MODE))) {
            unsigned int ref_tick = stimer_get_tick();
            while (reg_usb1_ep8_fifo_mode & FLD_USB1_EDP8_FULL_FLAG) {
                if (clock_time_exceed(ref_tick, 5000)) /* some exceptions occur, such as the usb disconnecting */
                {
                    return;
                }
            }
        }
        reg_usb1_ep_dat_word(ep) = ((unsigned int *)data)[i];
    }

    /* When the remaining size is less than 4 bytes, the MCU moves the data according to the byte length. */
    for (unsigned int i = 0; i < single_len; i++) {
        if ((USB1_EDP8_IN == ep) && (BM_IS_SET(reg_usb1_ep8_fifo_mode, FLD_USB1_EDP8_FIFO_MODE))) {
            unsigned int ref_tick = stimer_get_tick();
            while (reg_usb1_ep8_fifo_mode & FLD_USB1_EDP8_FULL_FLAG) {
                if (clock_time_exceed(ref_tick, 5000)) /* some exceptions occur, such as the usb disconnecting */
                {
                    return;
                }
            }
        }
        reg_usb1_ep_dat_byte(ep, i % 4) = data[(word_len * 4) + i];
    }
}

/**
 * @brief     This function servers to write the data of endpoint.
 * @param[in] ep   - selected the endpoint
 * @param[in] data - the pointer to the data for write.
 * @param[in] len  - write length.
 * @return    none
 */
void usb1hw_read_ep_data(usb1_ep_index ep, unsigned char *data, unsigned short len)
{
    unsigned char word_len   = len >> 2;
    unsigned char single_len = len & 3;

    /* When the remaining size  is not less than 4 bytes, the MCU moves the data according to the word length. */
    for (unsigned int i = 0; i < word_len; i++) {
        ((unsigned int *)data)[i] = reg_usb1_ep_dat_word(ep);
    }

    /* When the remaining size is less than 4 bytes, the MCU moves the data according to the byte length. */
    for (unsigned int i = 0; i < single_len; i++) {
        data[(word_len * 4) + i] = reg_usb1_ep_dat_byte(ep, i % 4);
    }
}
