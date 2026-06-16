/********************************************************************************************************
 * @file    usb0hw.h
 *
 * @brief   This is the header file for tl322x
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
#ifndef __USB0HW_H__
#define __USB0HW_H__
#include "reg_include/register.h"
#include "lib/include/stimer.h"

/**********************************************************************************************************************
 *                                                   USB0 enum/struct                                                  *
 *********************************************************************************************************************/
/*!
 * @name USB0 Endpoint enum/struct.
 * @{
 */

/**
 * @brief Define USB0 endpoint index.
 * @note  All endpoints are bi-directional.
 * 
 */
typedef enum
{
    USB0_EP0,
    USB0_EP1,
    USB0_EP2,
    USB0_EP3,
    USB0_EP4,
    USB0_EP5,
    USB0_EP6,
    USB0_EP7,
    USB0_EP8,
} usb0_ep_e;

/**
 * @brief Define USB0 TX FIFO index.
 * 
 */
typedef enum
{
    USB0_TX_FIFO0,
    USB0_TX_FIFO1,
    USB0_TX_FIFO2,
    USB0_TX_FIFO3,
    USB0_TX_FIFO4,
    USB0_TX_FIFO5,
    USB0_TX_FIFO6,
    USB0_TX_FIFO7,
    USB0_TX_FIFO8,
} usb0_tx_fifo_e;

/**
 * @brief Define USB0 speed.
 * 
 */
typedef enum
{
    USB0_SPEED_HIGH,
    USB0_SPEED_FULL,
} usb0_speed_e;

/**
 * @brief Define USB0 endpoint type.
 * 
 */
typedef enum
{
    USB0_EP_TYPE_CONTROL,
    USB0_EP_TYPE_ISOCHRONOUS,
    USB0_EP_TYPE_BULK,
    USB0_EP_TYPE_INTERRUPT,
} usb0_ep_type_e;

/**
 * @brief Define USB0 endpoint direction.
 *
 */
typedef enum
{
    USB0_DIR_OUT = 0,
    USB0_DIR_IN  = 1,

    USB0_DIR_IN_MASK = 0x80
} usb0_dir_e;

/**
 * @brief Define USB0 test mode.
 *
 */
typedef enum
{
    USB0_TEST_MODE_DISABLE,
    USB0_TEST_J_MODE,
    USB0_TEST_K,
    USB0_TEST_SE0_NAK_MODE,
    USB0_TEST_PACKET_MODE,
    USB0_TEST_FORCE_ENABLE_MODE,
} usb0_test_mode_t;

/**
 * @}
 */

/*!
 * @name USB0 DMA enum/struct.
 * @{
 */

/**
 * @brief Structure for the DMA descriptor of non-isochronous OUT transfers in USB communication.
 * 
 */
typedef struct
{
    unsigned int rx_bytes : 16; /**< Received number of bytes : 15:0 */
    unsigned int r : 7;         /**< Reserved                 : 22:16 */
    unsigned int mtrf : 1;      /**< Multiple Transfer        : 23. Reserved for ISO OUT and Control OUT endpoints.*/
    unsigned int sr : 1;        /**< Setup Packet Received    : 24 */
    unsigned int ioc : 1;       /**< Interrupt on complete    : 25 */
    unsigned int sp : 1;        /**< Short packet             : 26 */
    unsigned int l : 1;         /**< Last                     : 27 */
    unsigned int rx_sts : 2;    /**< Receive status           : 29:28 */
    unsigned int bs : 2;        /**< Buffer status            : 31:30 */
} usb_non_isochronous_out_t;

/**
 * @brief Structure for the DMA descriptor of isochronous OUT transfers in USB communication.
 * 
 */
typedef struct
{
    unsigned int rx_bytes : 11;  /**< Received number of bytes        : 10:0 */
    unsigned int r : 1;          /**< Reserved                        : 11 */
    unsigned int frame_num : 11; /**< Frame number                    : 22:12 */
    unsigned int pid : 2;        /**< ISO Number of packets per frame : 24:23 */
    unsigned int ioc : 1;        /**< Interrupt on complete           : 25 */
    unsigned int sp : 1;         /**< Short packet                    : 26 */
    unsigned int l : 1;          /**< Last                            : 27 */
    unsigned int rx_sts : 2;     /**< Receive status                  : 29:28 */
    unsigned int bs : 2;         /**< Buffer status                   : 31:30 */
} usb_isochronous_out_t;

/**
 * @brief Union for representing the DMA descriptor of the USB OUT endpoint.
 * 
 * This union provides two different types of DMA descriptor structures for non-isochronous and isochronous transfers respectively.
 * Due to the nature of the union, only one member can be used at a time.
 */
typedef union
{
    usb_non_isochronous_out_t non_iso_out; /**< DMA descriptor for non-isochronous OUT transfers. */
    usb_isochronous_out_t iso_out;         /**< DMA descriptor for isochronous OUT transfers. */
} usb_out_dma_desc_u;

/**
 * @brief Structure for the DMA descriptor of non-isochronous IN transfers in USB communication.
 * 
 */
typedef struct
{
    unsigned int tx_bytes : 16; /**< Transmit number of bytes : 15:0 */
    unsigned int r : 7;         /**< Reserved                 : 22:16 */
    unsigned int pid : 2;       /**< Multiple Transfer        : 24:23 */
    unsigned int ioc : 1;       /**< Interrupt on complete    : 25 */
    unsigned int sp : 1;        /**< Short packet             : 26 */
    unsigned int l : 1;         /**< Last                     : 27 */
    unsigned int tx_sts : 2;    /**< Transmit status          : 29:28 */
    unsigned int bs : 2;        /**< Buffer status            : 31:30 */
} usb_non_isochronous_in_t;

/**
 * @brief Structure for the DMA descriptor of isochronous IN transfers in USB communication.
 * 
 */
typedef struct
{
    unsigned int tx_bytes : 12;  /**< Transmit number of bytes : 11:0 */
    unsigned int frame_num : 11; /**< Frame number             : 22:12 */
    unsigned int pid : 2;        /**< ISO Received Data PID    : 24:23 */
    unsigned int ioc : 1;        /**< Interrupt on complete    : 25 */
    unsigned int sp : 1;         /**< Short packet             : 26 */
    unsigned int l : 1;          /**< Last                     : 27 */
    unsigned int tx_sts : 2;     /**< Transmit status          : 29:28 */
    unsigned int bs : 2;         /**< Buffer status            : 31:30 */
} usb_isochronous_in_t;

/**
 * @brief Union for representing the DMA descriptor of the USB IN endpoint.
 * 
 * This union provides two different types of DMA descriptor structures for non-isochronous and isochronous transfers respectively.
 * Due to the nature of the union, only one member can be used at a time.
 * 
 */
typedef union
{
    usb_non_isochronous_in_t non_iso_in; /**< DMA descriptor for non-isochronous IN transfers. */
    usb_isochronous_in_t iso_in;         /**< DMA descriptor for isochronous IN transfers. */
} usb_in_dma_desc_u;

/**
 * @}
 */

/*!
 * @name USB0 Interface.
 * @{
 */

 /**
 * @brief      This function servers to initialize the USB0.
 * 
 * @param[in]  speed_sel @see usb0_speed_e.
 * @return     None.
 * 
 */
void usb0hw_init(usb0_speed_e speed_sel);

/**
 * @brief      This function servers to power down the USB0.
 * 
 * @param      None.
 * @return     None.
 * 
 */
void usb0hw_power_down(void);

/**
 * @brief      This function servers to get the global interrupt status.
 * 
 * @param[in]  None.
 * @return     The global interrupt status.
 *
 */
static inline unsigned int usb0hw_get_gintsts(void)
{
    return reg_usb_gintsts;
}

/**
 * @brief      This function servers to clear the global interrupt status.
 * 
 * @param[in]  status - the status to be cleared.
 * @return     None.
 * 
 */
static inline void usb0hw_clear_gintsts(usb_gintsts_e status)
{
    reg_usb_gintsts = status;
}

/**
 * @brief      This function servers to disable usb pcgc clk
 * @return     None.
 *
 */
static inline void usb0hw_pcgc_clk_dis(void)
{
    reg_usb_pcgcctl |= FLD_USB_PCGCCTL_STOPPCLK | FLD_USB_PCGCCTL_PWRCLMP;
}

/**
 * @brief      This function servers to enable usb pcgc clk
 * @return     None.
 *
 */
static inline void usb0hw_pcgc_clk_en(void)
{
    reg_usb_pcgcctl &= ~(FLD_USB_PCGCCTL_STOPPCLK | FLD_USB_PCGCCTL_PWRCLMP);
}

/**
 * @brief      This function servers to disable usb phy pll clk
 * @return     None.
 *
 */
static inline void usb0hw_phy_pll_dis(void)
{
    reg_usb_usbphy_ct &= ~FLD_USB_USBPHYCT_RELEASE_AUDO;
}

/**
 * @brief      This function servers to enable usb phy pll clk
 * @return     None.
 *
 */
static inline void usb0hw_phy_pll_en(void)
{
    reg_usb_usbphy_ct |= FLD_USB_USBPHYCT_RELEASE_AUDO;
}

/**
 * @brief      This function servers to flush the TX FIFO.
 * 
 * @param[in]  ep_num - endpoint number.
 * @return     None.
 * @note
 *             - 0x10: Flush all the transmit FIFOs
 */
unsigned int usb0hw_flush_tx_fifo(usb0_ep_e ep_num);

/**
 * @brief      This function servers to flush the RX FIFO.
 * 
 * @param[in]  None.
 * @return
 *             - 0: successful.
 *             - 1: failed.
 * 
 */
unsigned int usb0hw_flush_rx_fifo(void);

/**
 * @brief      This function servers to get time stamp.
 * 
 * @param[in]  None.
 * @return     The time stamp.
 * 
 */
static inline unsigned int usb0hw_get_timer_stamp(void)
{
    return reg_usb_tstamp;
}

/**
 * @brief      This function servers to trigger USB0 remote wakeup.
 * 
 * @param[in]  None.
 * @return     None.
 * 
 */
void usb0hw_remote_wakeup(void);

/**
 * @brief      This function servers to set the device address.
 * 
 * @param[in]  dev_addr - device address.
 * @return     None.
 * 
 */
static inline void usb0hw_set_address(const unsigned char dev_addr)
{
    reg_usb_dcfg = (reg_usb_dcfg & (~FLD_USB_DCFG_DEVADDR)) | MASK_VAL(FLD_USB_DCFG_DEVADDR, dev_addr);
}

/**
 * @brief      This function servers to get the device speed.
 * 
 * @param[in]  None.
 * @return     device speed @ref usb0_speed_e. 
 * 
 */
static inline usb0_speed_e usb0hw_get_speed(void)
{
    return (reg_usb_dsts & FLD_USB_DSTS_ENUMSPD) >> 1;
}

/**
 * @brief      This function servers to get the SOF frame number.
 * 
 * @param[in]  None.
 * @return     SOF frame number.
 * 
 */
static inline unsigned int usb0hw_get_sof_fn(void)
{
    return (reg_usb_dsts & FLD_USB_DSTS_SOFFN) >> 8;
}

/**
 * @brief      This function servers to set USB0 software disconnect.
 *
 * @param[in]  None.
 * @return     None.
 * 
 */
static inline void usb0hw_soft_disconnect(void)
{
    BM_SET(reg_usb_dctl, FLD_USB_DCTL_SFTDISCON);
}

/**
 * @brief      This function servers to set USB0 software connect.
 *
 * @param[in]  None.
 * @return     None.
 * 
 */
static inline void usb0hw_soft_connect(void)
{
    BM_CLR(reg_usb_dctl, FLD_USB_DCTL_SFTDISCON);
}

/**
 * @brief      This function servers to set USB0 test mode.
 *
 * @param[in]  mode - test mode @ref usb0_test_mode_t.
 * @return     None.
 * 
 */
static inline void usb0hw_test_mode(usb0_test_mode_t mode)
{
    reg_usb_dctl = (reg_usb_dctl & (~FLD_USB_DCTL_TSTCTL)) | MASK_VAL(FLD_USB_DCTL_TSTCTL, mode);
}

/**
 * @brief      This function servers to set the power on program done.
 * 
 * @param[in]  None.
 * @return     None.
 * 
 */
static inline void usb0hw_set_pwronprgdone(void)
{
    BM_SET(reg_usb_dctl, FLD_USB_DCTL_PWRONPRGDONE);
}

/**
 * @brief      This function servers to enable the device endpoint interrupt mask.
 * 
 * @param[in]  mask - The mask to be enabled.
 * @return     None.
 *
 */
static inline void usb0hw_daintmsk_en(usb_daintmsk_e mask)
{
    BM_SET(reg_usb_daintmsk, mask);
}

/**
 * @brief      This function servers to disable the device endpoint interrupt mask.
 * 
 * @param[in]  mask - The mask to be disabled.
 * @return     None.
 *
 */
static inline void usb0hw_daintmsk_dis(usb_daintmsk_e mask)
{
    BM_CLR(reg_usb_daintmsk, mask);
}

/**
 * @brief      This function servers to get the device endpoint interrupt status.
 * 
 * @param[in]  None.
 * @return     The device endpoint interrupt status.
 *
 */
static inline unsigned int usb0hw_get_daint(void)
{
    return reg_usb_daint;
}

/**
 * @brief      This function servers to set the stall status of the specified in endpoint.
 * 
 * @param[in]  ep_num - endpoint number.
 * @return     None. 
 * 
 */
static inline void usb0hw_set_inep_stall(unsigned char ep_num)
{
    BM_SET(reg_usb_diepctl(ep_num), FLD_USB_DIEPCTL_STALL);
}

/**
 * @brief      This function servers to set the stall status of the specified out endpoint.
 * 
 * @param[in]  ep_num - endpoint number.
 * @return     None. 
 * 
 */
static inline void usb0hw_set_outep_stall(unsigned char ep_num)
{
    BM_SET(reg_usb_doepctl(ep_num), FLD_USB_DOEPCTL_STALL);
}

/**
 * @brief      This function servers to clear the stall status of the specified in endpoint.
 * 
 * @param[in]  ep_num - endpoint number.
 * @return     None.
 * 
 */
static inline void usb0hw_clear_epin_stall(unsigned char ep_num)
{
    BM_CLR(reg_usb_diepctl(ep_num), FLD_USB_DIEPCTL_STALL);
}

/**
 * @brief      This function servers to clear the stall status of the specified out endpoint.
 * 
 * @param[in]  ep_num - endpoint number.
 * @return     None.
 * 
 */
static inline void usb0hw_clear_epout_stall(unsigned char ep_num)
{
    BM_CLR(reg_usb_doepctl(ep_num), FLD_USB_DOEPCTL_STALL);
}

/**
 * @brief      This function servers to get the device output endpoint interrupt status.
 * 
 * @param[in]  ep_num - endpoint number.
 * @return     The device output endpoint interrupt status.
 * 
 */
static inline unsigned int usb0hw_get_doepint(usb0_ep_e ep_num)
{
    return reg_usb_doepint(ep_num);
}

/**
 * @brief      This function servers to clear the device output endpoint interrupt status.
 * 
 * @param[in]  ep_num - endpoint number.
 * @param[in]  status - the status to be cleared.
 * @return     None.
 *
 */
static inline void usb0hw_clear_doepint(usb0_ep_e ep_num, usb_doepint_e status)
{
    reg_usb_doepint(ep_num) = status;
}

/**
 * @brief      This function servers to get the device input endpoint interrupt status.
 * 
 * @param[in]  ep_num - endpoint number.
 * @return     The device input endpoint interrupt status.
 *
 */
static inline unsigned int usb0hw_get_diepint(usb0_ep_e ep_num)
{
    return reg_usb_diepint(ep_num);
}

/**
 * @brief      This function servers to clear the device input endpoint interrupt status.
 * 
 * @param[in]  ep_num - endpoint number.
 * @param[in]  status - the status to be cleared.
 * @return     None.
 *
 */
static inline void usb0hw_clear_diepint(usb0_ep_e ep_num, usb_diepint_e status)
{
    reg_usb_diepint(ep_num) = status;
}

/**
 * @brief      This function servers to set the global RX FIFO size.
 * 
 * @param[in]  size - the size to be set.
 * @return     None.
 * @note
 *             - This value is in terms of 32-bit words.
 * 
 */
static inline void usb0hw_set_grxfsiz(unsigned short size)
{
    reg_usb_grxfsiz = size;
}

/**
 * @brief      This function servers to set the device input endpoint fifo.
 * 
 * @param[in]  ep_num   - endpoint number.
 * @param[in]  fifo_num - fifo number.
 * @return     None.
 *
 */
static inline void usb0hw_set_epin_fifo(usb0_ep_e ep_num, usb0_tx_fifo_e fifo_num)
{
    reg_usb_diepctl(ep_num) = (reg_usb_diepctl(ep_num) & ~(FLD_USB_DIEPCTL_TXFNUM)) | MASK_VAL(FLD_USB_DIEPCTL_TXFNUM, fifo_num);
}

/**
 * @brief      This function servers to set the size of the specified IN endpoint.
 * 
 * @param[in]  ep_num - endpoint number.
 * @param[in]  addr   - start address of the endpoint.
 * @param[in]  size   - size of the endpoint.
 * @return     None.
 * @note
 *             - This value is in terms of 32-bit words.
 */
static inline void usb0hw_set_epin_size(usb0_ep_e ep_num, unsigned int addr, unsigned int size)
{
    reg_usb_dieptxf(ep_num) = MASK_VAL(FLD_USB_DIEPTXF_INEPNTXFSTADDR, addr, FLD_USB_DIEPTXF_INEPNTXFDEP, size);
}

/**
 * @brief      This function servers to reset the USB0.
 *
 * @param[in]  None.
 * @return     None.
 * 
 */
void usb0hw_reset(void);

/**
 * @brief      This function servers to get the length of the data received by the specified OUT endpoint.
 * 
 * @param[in]  ep_num - endpoint number.
 * @return     The length of the data received by the specified OUT endpoint, unit is byte.
 * 
 */
unsigned short usb0hw_get_epout_len(usb0_ep_e ep_num);

/**
 * @brief      This function servers to get the length of the data transmitted by the specified IN endpoint.
 * 
 * @param[in]  ep_num - endpoint number.
 * @return     The length of the data transmitted by the specified IN endpoint, unit is byte.
 * 
 */
unsigned short usb0hw_get_epin_len(usb0_ep_e ep_num);

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
void usb0hw_ep_open(usb0_ep_e ep_num, usb0_dir_e ep_dir, usb0_ep_type_e ep_type, unsigned short ep_mps);

/**
 * @brief      This function servers to close the endpoint.
 * 
 * @param[in]  ep_num - endpoint number.
 * @param[in]  ep_dir - endpoint direction.
 * @return     None
 * 
 */
void usb0hw_ep_close(usb0_ep_e ep_num, usb0_dir_e ep_dir);

/**
 * @brief      This function servers to write the endpoint data.
 * 
 * @param[in]  ep_num - endpoint number.
 * @param[in]  buf    - pointer to the buffer.
 * @param[in]  len    - bytes to be written.
 * @return     None
 * 
 */
void usb0hw_write_ep_data(usb0_ep_e ep_num, unsigned char *buf, unsigned int len);

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
void usb0hw_read_ep_data(usb0_ep_e ep_num, unsigned char *buf, unsigned int len);

/**
 * @}
 */

#endif
