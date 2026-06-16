/********************************************************************************************************
 * @file    usb0hw.c
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
#include "usb0hw.h"
#include "lib/include/pm/pm.h"
#include <string.h>

#ifndef USB_TIMEOUT_US
#define USB_TIMEOUT_US 1000U
#endif

static usb_in_dma_desc_u in_dma_desc[9];
static usb_out_dma_desc_u out_dma_desc[9];

static unsigned int diepdma_buf[9][2];
static unsigned int doepdma_buf[9][2];

typedef struct
{
    unsigned short max_size;
} xfer_ctl_t;

static xfer_ctl_t xfer_status[9][2];

/**
 * @brief      This function servers to initialize the USB0.
 * 
 * @param[in]  speed_sel @see usb0_speed_e.
 * @return     None.
 * 
 */
void usb0hw_init(usb0_speed_e speed_sel)
{
    /* gpio_init pull down PA3, PA4, when using usb0 function, set pin floating. */
    gpio_set_up_down_res(GPIO_PA3, GPIO_PIN_UP_DOWN_FLOAT); /* USB0_DM */
    gpio_set_up_down_res(GPIO_PA4, GPIO_PIN_UP_DOWN_FLOAT); /* USB0_DP */

    pm_set_dig_module_power_switch(FLD_PD_USB_EN, PM_POWER_UP);
    BM_SET(reg_rst0, FLD_RST0_USB);
    BM_SET(reg_clk_en0, FLD_RST0_USB);

    BM_SET(reg_usb_gahbcfg, FLD_USB_GAHBCFG_GLBLINTRMSK | FLD_USB_GAHBCFG_DMAEN);
    BM_SET(reg_usb_diepmsk, FLD_USB_DIEPMSK_XFERCOMPLMSK);
    BM_SET(reg_usb_doepmsk, FLD_USB_DOEPMSK_XFERCOMPLMSK | FLD_USB_DOEPMSK_SETUPMSK | FLD_USB_DOEPMSK_STSPHSERCVDMSK);
    BM_SET(reg_usb_daintmsk, FLD_USB_DAINTMSK_OUTEPMSK0 | FLD_USB_DAINTMSK_INEPMSK0);
    BM_SET(reg_usb_dctl, FLD_USB_DCTL_IGNRFRMNUM);
    BM_SET(reg_usb_gintmsk, FLD_USB_GINTMSK_USBRSTMSK | FLD_USB_GINTMSK_ENUMDONEMSK | FLD_USB_GINTMSK_IEPINTMSK | FLD_USB_GINTMSK_OEPINTMSK | FLD_USB_GINTMSK_SOFMSK |
                      FLD_USB_GINTMSK_RESETDETMSK | FLD_USB_GINTMSK_WKUPINTMSK);
    reg_usb_dcfg = (reg_usb_dcfg & (~FLD_USB_DCFG_DEVSPD)) | MASK_VAL(FLD_USB_DCFG_DEVSPD, speed_sel, FLD_USB_DCFG_DESCDMA, 1);
    reg_usb_gusbcfg = (reg_usb_gusbcfg & ~FLD_USB_GUSBCFG_USBTRDTIM) | MASK_VAL(FLD_USB_GUSBCFG_USBTRDTIM, 9);

    usb0hw_soft_connect();
}

/**
 * @brief      This function servers to power down the USB0.
 * 
 * @param      None.
 * @return     None.
 * 
 */
void usb0hw_power_down(void)
{
    usb0hw_pcgc_clk_dis();
    usb0hw_phy_pll_dis();
    BM_CLR(reg_clk_en0, FLD_RST0_USB);
    BM_CLR(reg_rst0, FLD_RST0_USB);
    pm_set_dig_module_power_switch(FLD_PD_USB_EN, PM_POWER_DOWN);
}

/**
 * @brief      This function servers to flush the TX FIFO.
 * 
 * @param[in]  ep_num - endpoint number.
 * @return     None.
 * @note
 *             - 0x10: Flush all the transmit FIFOs
 */
unsigned int usb0hw_flush_tx_fifo(usb0_ep_e ep_num)
{
    reg_usb_grstctl = (reg_usb_grstctl & ~((FLD_USB_GRSTCTL_TXFFLSH | FLD_USB_GRSTCTL_TXFNUM))) |
                      MASK_VAL(FLD_USB_GRSTCTL_TXFFLSH, 1, FLD_USB_GRSTCTL_TXFNUM, ep_num);

    unsigned int ref_tick = stimer_get_tick();
    while (BM_IS_SET(reg_usb_grstctl, FLD_USB_GRSTCTL_TXFFLSH))
    {
        if (clock_time_exceed(ref_tick, USB_TIMEOUT_US))
        {
            return 1;
        }
    }

    return 0;
}

/**
 * @brief      This function servers to flush the RX FIFO.
 * 
 * @param[in]  None.
 * @return
 *             - 0: successful.
 *             - 1: failed.
 */
unsigned int usb0hw_flush_rx_fifo(void)
{
    BM_SET(reg_usb_grstctl, FLD_USB_GRSTCTL_RXFFLSH);
    unsigned int ref_tick = stimer_get_tick();
    while (BM_IS_SET(reg_usb_grstctl, FLD_USB_GRSTCTL_RXFFLSH))
    {
        if (clock_time_exceed(ref_tick, USB_TIMEOUT_US))
        {
            return 1;
        }
    }

    return 0;
}

/**
 * @brief      This function servers to trigger USB0 remote wakeup.
 * 
 * @param[in]  None.
 * @return     None.
 */
void usb0hw_remote_wakeup(void)
{
    if (!(reg_usb_dsts & FLD_USB_DSTS_SUSPSTS))
    {
        return;
    }

    usb0hw_pcgc_clk_en();
    usb0hw_phy_pll_en();

    BM_SET(reg_usb_dctl, FLD_USB_DCTL_RMTWKUPSIG);
    delay_ms(10);
    BM_CLR(reg_usb_dctl, FLD_USB_DCTL_RMTWKUPSIG);
}

/**
 * @brief      This function servers to reset the USB0.
 *
 * @param[in]  None.
 * @return     None.
 * 
 */
void usb0hw_reset(void)
{
    memset(&xfer_status[0][0], 0, sizeof(xfer_status));

    usb0hw_set_address(0x00);

    for (unsigned char i = 0; i < 9; i++)
    {
        reg_usb_doepctl(i) |= FLD_USB_DOEPCTL_SNAK;
    }

    for (unsigned char i = 0; i < 9; i++)
    {
        reg_usb_diepctl(i) |= FLD_USB_DIEPCTL_SNAK | FLD_USB_DIEPCTL_EPDIS;
    }

    xfer_status[0][USB0_DIR_OUT].max_size = 64;
    xfer_status[0][USB0_DIR_IN].max_size  = 64;

    usb0hw_flush_tx_fifo(0x10);
    usb0hw_flush_rx_fifo();

    usb0hw_daintmsk_en(FLD_USB_DAINTMSK_OUTEPMSK0 | FLD_USB_DAINTMSK_INEPMSK0);

    usb0hw_clear_gintsts(FLD_USB_GINTSTS_USBSUSP);  /*Enable usb_suspend irq when connected to the host computer*/
    BM_SET(reg_usb_gintmsk,  FLD_USB_GINTMSK_USBSUSPMSK);
}

/**
 * @brief      This function servers to get the device input endpoint type.
 *
 * @param[in]  ep_num - endpoint number.
 * @return     The device input endpoint type @ref usb0_ep_type_e.
 *
 */
static inline usb0_ep_type_e usb0hw_get_epin_type(usb0_ep_e ep_num)
{
    return (((reg_usb_diepctl(ep_num) & FLD_USB_DIEPCTL_EPTYPE) >> 18) & 0b11);
}

/**
 * @brief      This function servers to get the device output endpoint type.
 *
 * @param[in]  ep_num - endpoint number.
 * @return     The device output endpoint type @ref usb0_ep_type_e.
 *
 */
static inline usb0_ep_type_e usb0hw_get_epout_type(usb0_ep_e ep_num)
{
    return (((reg_usb_doepctl(ep_num) & FLD_USB_DOEPCTL_EPTYPE) >> 18) & 0b11);
}

/**
 * @brief      This function servers to get the length of the data received by the specified OUT endpoint.
 * 
 * @param[in]  ep_num - endpoint number.
 * @return     The length of the data received by the specified OUT endpoint, unit is byte.
 * 
 */
unsigned short usb0hw_get_epout_len(usb0_ep_e ep_num)
{
    unsigned short xfered_len;

    if (usb0hw_get_epout_type(ep_num) == USB0_EP_TYPE_ISOCHRONOUS)
    {
        xfered_len = out_dma_desc[ep_num].non_iso_out.rx_bytes - (doepdma_buf[ep_num][USB0_DIR_OUT] & 0x7ff);
    }
    else
    {
        xfered_len = out_dma_desc[ep_num].non_iso_out.rx_bytes - (doepdma_buf[ep_num][USB0_DIR_OUT] & 0xffff);
    }

    return xfered_len;
}

/**
 * @brief      This function servers to get the length of the data transmitted by the specified IN endpoint.
 * 
 * @param[in]  ep_num - endpoint number.
 * @return     The length of the data transmitted by the specified IN endpoint, unit is byte.
 * 
 */
unsigned short usb0hw_get_epin_len(usb0_ep_e ep_num)
{
    unsigned short xfered_len;

    if (usb0hw_get_epin_type(ep_num) == USB0_EP_TYPE_ISOCHRONOUS)
    {
        xfered_len = in_dma_desc[ep_num].iso_in.tx_bytes;
    }
    else
    {
        xfered_len = in_dma_desc[ep_num].non_iso_in.tx_bytes;
    }

    return xfered_len;
}

/**
 * @brief      This function servers to open the endpoint.
 * 
 * @param[in]  ep_num  - endpoint number.
 * @param[in]  ep_dir  - endpoint direction.
 * @param[in]  ep_type - endpoint type.
 * @param[in]  ep_mps  - endpoint max packet size, unit is byte.
 * @return     None.
 * 
 */
void usb0hw_ep_open(usb0_ep_e ep_num, usb0_dir_e ep_dir, usb0_ep_type_e ep_type, unsigned short ep_mps)
{
    if (ep_dir == USB0_DIR_IN)
    {
        xfer_status[ep_num][USB0_DIR_IN].max_size = ep_mps;
        reg_usb_diepctl(ep_num) =
                (reg_usb_diepctl(ep_num) & ~(FLD_USB_DIEPCTL_TXFNUM | FLD_USB_DIEPCTL_EPTYPE | FLD_USB_DIEPCTL_MPS | FLD_USB_DIEPCTL_USBACTEP)) |
                MASK_VAL(FLD_USB_DIEPCTL_TXFNUM, ep_num, FLD_USB_DIEPCTL_EPTYPE, ep_type, FLD_USB_DIEPCTL_MPS, ep_mps, FLD_USB_DIEPCTL_USBACTEP, 1);

        usb0hw_set_epin_fifo(ep_num, (usb0_tx_fifo_e)ep_num);
        usb0hw_daintmsk_en(BIT(ep_num));
    }
    else
    {
        xfer_status[ep_num][USB0_DIR_OUT].max_size = ep_mps;
        reg_usb_doepctl(ep_num) = (reg_usb_doepctl(ep_num) & ~(FLD_USB_DOEPCTL_EPTYPE | FLD_USB_DOEPCTL_MPS | FLD_USB_DOEPCTL_USBACTEP)) |
                                  MASK_VAL(FLD_USB_DOEPCTL_EPTYPE, ep_type, FLD_USB_DOEPCTL_MPS, ep_mps, FLD_USB_DOEPCTL_USBACTEP, 1);

        usb0hw_daintmsk_en(BIT(ep_num) << 16);
    }
}

/**
 * @brief      This function servers to close the endpoint.
 * 
 * @param[in]  ep_num - endpoint number.
 * @param[in]  ep_dir - endpoint direction.
 * @return     None
 * 
 */
void usb0hw_ep_close(usb0_ep_e ep_num, usb0_dir_e ep_dir)
{
    unsigned int timeout = 0U;

    if (ep_dir == USB0_DIR_IN)
    {
        if (reg_usb_diepctl(ep_num) & FLD_USB_DIEPCTL_EPENA)
        {
            BM_SET(reg_usb_diepctl(ep_num), FLD_USB_DIEPCTL_SNAK);
            BM_SET(reg_usb_diepctl(ep_num), FLD_USB_DIEPCTL_EPDIS);

            timeout = 0;
            do
            {
                if (++timeout > 50000)
                {
                    break;
                }
            } while ((reg_usb_diepint(ep_num) & FLD_USB_DIEPINT_EPDISBLD) != FLD_USB_DIEPINT_EPDISBLD);
            reg_usb_diepint(ep_num) = FLD_USB_DIEPINT_EPDISBLD;
        }
        usb0hw_daintmsk_dis(BIT(ep_num));
        reg_usb_diepctl(ep_num) = 0;
    }
    else
    {
        if (reg_usb_doepctl(ep_num) & FLD_USB_DOEPCTL_EPENA)
        {
            BM_SET(reg_usb_doepctl(ep_num), FLD_USB_DOEPCTL_SNAK);
            BM_SET(reg_usb_doepctl(ep_num), FLD_USB_DOEPCTL_EPDIS);

            timeout = 0;
            do
            {
                if (++timeout > 50000)
                {
                    break;
                }
            } while ((reg_usb_doepint(ep_num) & FLD_USB_DOEPINT_EPDISBLD) != FLD_USB_DOEPINT_EPDISBLD);
            reg_usb_doepint(ep_num) = FLD_USB_DOEPINT_EPDISBLD;
        }
        usb0hw_daintmsk_dis(BIT(ep_num) << 16);
        reg_usb_doepctl(ep_num) = 0;
    }
}

/**
 * @brief      This function servers to write the endpoint data.
 * 
 * @param[in]  ep_num - endpoint number.
 * @param[in]  buf    - pointer to the buffer.
 * @param[in]  len    - bytes to be written.
 * @return     None
 * 
 */
void usb0hw_write_ep_data(usb0_ep_e ep_num, unsigned char *buf, unsigned int len)
{
    if (usb0hw_get_epin_type(ep_num) == USB0_EP_TYPE_ISOCHRONOUS)
    {
        /* iso endpoint */
        in_dma_desc[ep_num].iso_in.bs       = 0;
        in_dma_desc[ep_num].iso_in.tx_sts   = 0;
        in_dma_desc[ep_num].iso_in.l        = 1;
        in_dma_desc[ep_num].iso_in.sp       = 0;
        in_dma_desc[ep_num].iso_in.ioc      = 1;
        in_dma_desc[ep_num].iso_in.tx_bytes = len;
    }
    else
    {
        in_dma_desc[ep_num].non_iso_in.bs     = 0;
        in_dma_desc[ep_num].non_iso_in.tx_sts = 0;
        in_dma_desc[ep_num].non_iso_in.l      = 1;
        if ((len % xfer_status[ep_num][USB0_DIR_IN].max_size == 0) && (usb0hw_get_epin_type(ep_num) != USB0_EP_TYPE_INTERRUPT))
        {
            in_dma_desc[ep_num].non_iso_in.sp = 1;
        }
        else
        {
            in_dma_desc[ep_num].non_iso_in.sp = 0;
        }

        in_dma_desc[ep_num].non_iso_in.ioc      = 1;
        in_dma_desc[ep_num].non_iso_in.tx_bytes = len;
    }
    diepdma_buf[ep_num][0] = *(unsigned int *)&in_dma_desc[ep_num];
    diepdma_buf[ep_num][1] = (unsigned int)buf;
    reg_usb_diepdma(ep_num) = (unsigned int)&diepdma_buf[ep_num][0];
    BM_SET(reg_usb_diepctl(ep_num), FLD_USB_DIEPCTL_CNAK | FLD_USB_DIEPCTL_EPENA);
}

/**
 * @brief      This function servers to read the endpoint data.
 * 
 * @param[in]  ep_num - endpoint number.
 * @param[out] buf    - pointer to the buffer.
 * @param[in]  len    - bytes to be read.
 * @return     None
 * @note
 *             - This function is used to configure the receiving DMA, and the actual data reception is completed in the interrupt.
 *             - The actual data length received can be obtained by calling the usb0hw_get_epout_len function.
 *             - The len must be in terms of multiple of MPS for the corresponding endpoint.
 * 
 */
void usb0hw_read_ep_data(usb0_ep_e ep_num, unsigned char *buf, unsigned int len)
{
    if (usb0hw_get_epin_type(ep_num) == USB0_EP_TYPE_ISOCHRONOUS)
    {
        /* iso endpoint */
        out_dma_desc[ep_num].iso_out.bs       = 0;
        out_dma_desc[ep_num].iso_out.rx_sts   = 0;
        out_dma_desc[ep_num].iso_out.l        = 1;
        out_dma_desc[ep_num].iso_out.sp       = 0;
        out_dma_desc[ep_num].iso_out.ioc      = 1;
        out_dma_desc[ep_num].iso_out.rx_bytes = len;
    }
    else
    {
        out_dma_desc[ep_num].non_iso_out.bs       = 0;
        out_dma_desc[ep_num].non_iso_out.rx_sts   = 0;
        out_dma_desc[ep_num].non_iso_out.l        = 1;
        out_dma_desc[ep_num].non_iso_out.sp       = 0;
        out_dma_desc[ep_num].non_iso_out.ioc      = 1;
        out_dma_desc[ep_num].non_iso_out.sr       = 0;
        out_dma_desc[ep_num].non_iso_out.mtrf     = 0;
        out_dma_desc[ep_num].non_iso_out.rx_bytes = len;
    }

    doepdma_buf[ep_num][0] = *(unsigned int *)&out_dma_desc[ep_num];
    doepdma_buf[ep_num][1] = (unsigned int)buf;
    reg_usb_doepdma(ep_num) = (unsigned int)&doepdma_buf[ep_num][0];
    BM_SET(reg_usb_doepctl(ep_num), FLD_USB_DOEPCTL_CNAK | FLD_USB_DOEPCTL_EPENA);
}
