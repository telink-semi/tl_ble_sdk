/********************************************************************************************************
 * @file    usbhw.c
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
#include <stddef.h>
#include <wchar.h>
#include "usbhw.h"

/**
 * @brief      This function serves to enable USB power and clock.
 * @return     none.
 */
void usbhw_init(void)
{
    pm_set_dig_module_power_switch(FLD_PD_USB_EN, PM_POWER_UP);

    BM_SET(reg_rst0, FLD_RST0_USB);
    BM_SET(reg_clk_en0, FLD_CLK0_USB_EN);

    write_reg8(SC_BASE_ADDR + 0x3b, sys_clk.pll_clk / 48); // Split the PLL clock to the USB clock
}

/**
 * @brief      This function serves to disable the manual interrupt(endpoint8 is the alias of endpoint0).
 * @param[in]  m - the irq mode needs to set.
 * @return     none.
 */
void usbhw_disable_manual_interrupt(int m)
{
    BM_SET(reg_ctrl_ep_irq_mode, m);
}

/**
 * @brief      This function serves to enable the manual interrupt.
 * @param[in]  m - the irq mode needs to set.
 * @return     none.
 */
void usbhw_enable_manual_interrupt(int m)
{
    BM_CLR(reg_ctrl_ep_irq_mode, m);
}

/**
 * @brief      This function serves to send a bulk of data to host via the specified endpoint.
 * @param[in]  ep - the number of the endpoint.
 * @param[in]  data - pointer to the data need to send.
 * @param[in]  len - length in byte of the data need to send.
 * @return     none.
 */
void usbhw_write_ep(unsigned int ep, unsigned char *data, int len)
{
    usbhw_reset_ep_ptr(ep);
    for (int i = 0; i < (len); ++i)
    {
        reg_usb_ep_dat(ep) = data[i];
    }
    usbhw_data_ep_ack(ep); // ACK
}

/**
 * @brief      This function serves to send two bytes data to host via the control endpoint.
 * @param[in]  v - the two bytes data need to send.
 * @return     none.
 */
void usbhw_write_ctrl_ep_u16(unsigned short v)
{
    usbhw_write_ctrl_ep_data(v & 0xff);
    usbhw_write_ctrl_ep_data(v >> 8);
}

/**
 * @brief   This function serves to read two bytes data from host via the control endpoint.
 * @return  The two bytes data read from the control endpoint.
 */
unsigned short usbhw_read_ctrl_ep_u16(void)
{
    unsigned short v = usbhw_read_ctrl_ep_data();
    return (usbhw_read_ctrl_ep_data() << 8) | v;
}

/**
 * @brief   This function serves to set data endpoint mapping.
 * @param   source_ep - The source endpoint of the mapping.
 * @param   target_ep - The target endpoint of the mapping.
 * @return    none.
 */
void usbhw_set_ep_map(usb_ep_index source_ep, usb_ep_index target_ep)
{
    reg_usb_rdps_map(source_ep) = (source_ep & 1) == 0 ? (reg_usb_rdps_map(source_ep) & (~BIT_RNG(0, 3))) | (target_ep) : (reg_usb_rdps_map(source_ep) & (~BIT_RNG(4, 7))) | ((target_ep) << 4);
}

/**
 * @brief   This function serves to set data endpoint mapping.
 * @param   map_en - mapping enable or disable.
 * @return    none.
 */
void usbhw_ep_map_en(usb_ep_map_sel_e map_en)
{
    if (map_en == EP_MAP_AUTO_EN)
    {
        reg_usb_min1 |= FLD_USB_EDP_MAP_AUTO_EN | FLD_USB_EDPS_SM_MAP_EN | FLD_USB_EDPS_MAP_TGL_EN | FLD_USB_GET_STA_MAP_EN;
    }
    else
    {
        reg_usb_min1 &= ~(FLD_USB_EDP_MAP_AUTO_EN | FLD_USB_EDP_MAP_MANUAL_EN);
    }
}

