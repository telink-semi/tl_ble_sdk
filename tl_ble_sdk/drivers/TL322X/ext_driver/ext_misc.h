/********************************************************************************************************
 * @file    ext_misc.h
 *
 * @brief   This is the header file for BLE SDK
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
#ifndef DRIVERS_TL322X_EXT_MISC_H_
#define DRIVERS_TL322X_EXT_MISC_H_



#include "types.h"
#include "../compatibility_pack/cmpt.h"
#include "../lib/include/analog.h"
#include "../adc.h"
#include "../gpio.h"
#include "../lib/include/stimer.h"
#include "../lib/include/pm/pm.h"
#include "lib/include/rf/rf_common.h"
#include "../lib/include/trng/trng.h"
#include <stdbool.h>



/******************************* pke_start ******************************************************************/
#define ismemzero4(a, len)              uint32_BigNum_Check_Zero(a, len)    //For compatible with B91
/******************************* pke_end ********************************************************************/


/******************************* analog_start ******************************************************************/
#define analog_write                analog_write_reg8
#define analog_read                 analog_read_reg8
/******************************* analog_end ********************************************************************/



/******************************* core_start ******************************************************************/
#ifdef BLC_ZEPHYR_BLE_INTEGRATION
    #ifndef irq_enable
        #define irq_enable                  core_interrupt_enable
    #endif
    #ifndef irq_disable
        #define irq_disable                 core_interrupt_disable
    #endif
#else
    #define irq_disable                 core_interrupt_disable
    #define irq_enable                  core_interrupt_enable
#endif
#define irq_restore(en)             core_restore_interrupt(en)

#ifdef BLC_ZEPHYR_BLE_INTEGRATION
    #define start_reboot                protected_sys_reboot   //This function serves to reboot chip.
#else
    #define start_reboot                sys_reboot            //This function serves to reboot chip.
#endif
/******************************* core_end ********************************************************************/




/******************************* efuse start *****************************************************************/
#define get_device_mac_address            efuse_get_mac_address

bool efuse_get_mac_address(u8* mac_read, int length);
/******************************* efuse end *******************************************************************/




/******************************* gpio start ******************************************************************/

#define reg_gpio_out    reg_gpio_out_set_clear
/**
 * @brief     This function read a pin's cache from the buffer.
 * @param[in] pin - the pin needs to read.
 * @param[in] p - the buffer from which to read the pin's level.
 * @return    the state of the pin.
 */
static inline unsigned int gpio_read_cache(gpio_pin_e pin, unsigned char *p)
{
    return p[pin>>8] & (pin & 0xff);
}

/**
 * @brief      This function read all the pins' input level.
 * @param[out] p - the buffer used to store all the pins' input level
 * @return     none
 */
static inline void gpio_read_all(unsigned char *p)
{
    //TODO: gpio_get_level_all(p);
    p[0] = reg_gpio_pa_in;
    p[1] = reg_gpio_pb_in;
    p[2] = reg_gpio_pc_in;
    p[3] = reg_gpio_pd_in;
    p[4] = reg_gpio_pe_in;
    p[5] = reg_gpio_pf_in;
    p[6] = reg_gpio_pg_in;
    p[7] = reg_gpio_ph_in;
}

/******************************* gpio end ********************************************************************/




/******************************* rf tart **********************************************************************/
/**
 * @brief     This function serves to set BLE mode of RF.
 * @return    none.
 */
void rf_drv_ble_init(void);

#define RF_POWER_P3dBm   RF_POWER_INDEX_P3p00dBm
#define RF_POWER_P0dBm   RF_POWER_INDEX_P0p00dBm
#define RF_POWER_P9dBm   RF_POWER_INDEX_P9p00dBm

/******************************* rf end  **********************************************************************/




/******************************* trng_start ******************************************************************/

#define random_generator_init       trng_init



/**
 * @brief     This function performs to generate a series of random numbers
 * @param[in]  len - data length
 * @param[out] data - data pointer
 * @return    none
 **/
void generateRandomNum(int len, unsigned char *data);

/******************************* trng_end ********************************************************************/


/******************************* stimer start ******************************************************************/
#define sleep_us(x)                 delay_us(x)
#define sleep_ms(x)                 delay_ms(x)
/******************************* stimer end ********************************************************************/


/******************************* dma_start ***************************************************************/

//4(DMA_len) + 2(BLE header) + ISORxOct + 4(MIC) + 3(CRC) + 8(ExtraInfo)
#define     TLK_RF_RX_EXT_LEN       (21)

//10 = 4(DMA_len) + 2(BLE header) + 4(MIC)
#define     TLK_RF_TX_EXT_LEN       (10)

/**
 * @brief   RX Data buffer length = n + 21, then 16 Byte align
 *          n + 21 = 4(DMA_len) + 2(BLE header) + n + 4(MIC) + 3(CRC) + 8(ExtraInfo)
            RX buffer size must be be 16*n, due to MCU design
 */
#define     CAL_LL_COMMON_RX_FIFO_SIZE(n)       (((n + TLK_RF_RX_EXT_LEN) + 15) / 16 *16)
#define     CAL_LL_ACL_RX_FIFO_SIZE(maxRxOct)   CAL_LL_COMMON_RX_FIFO_SIZE(maxRxOct)
#define     CAL_LL_CIS_RX_FIFO_SIZE(cisRxPdu)   CAL_LL_COMMON_RX_FIFO_SIZE(cisRxPdu)

/**
 * @brief   ACL TX Data buffer length = maxTxOct + 10, then 16 Byte align
 *          maxTxOct + 10 = 4(DMA_len) + 2(BLE header) + maxTxOct + 4(MIC)
            TX buffer size must be be 16*n, due to MCU design
 */
#define     CAL_LL_ACL_TX_FIFO_SIZE(maxTxOct)   (((maxTxOct+TLK_RF_TX_EXT_LEN) + 15) / 16 *16)


/*HCI TX RX buffer len = uart_fifo+ dma 4byte */
#define     HCI_FIFO_SIZE(n)                    (((n+2+4) + 15) / 16 *16)





/*
 * @brief   ISO RX Data buffer length = ISORxOct + 21, then 16 Byte align
 *          ISORxOct + 21 = 4(DMA_len) + 2(BLE header) + ISORxOct + 4(MIC) + 3(CRC) + 8(ExtraInfo)
 *          RX buffer size must be be 16*n, due to MCU design
 */
#define     CAL_LL_ISO_RX_FIFO_SIZE(n)          (((n + TLK_RF_RX_EXT_LEN) + 15) / 16 * 16)


/*
 * @brief   ISO TX Data buffer length = ISOTxOct + 10, then 16 Byte align
 *          ISORxOct + 10 = 4(DMA_len) + 2(BLE header) + ISOTxOct + 4(MIC)
 *          TX buffer size must be be 16*n, due to MCU design
 */
#define     CAL_LL_ISO_TX_FIFO_SIZE(n)          (((n + TLK_RF_TX_EXT_LEN) + 15) / 16 * 16)


/*
* DMA_LEN(4B)+Hdr(2B)+PLD(251B)+MIC(4B)+CRC(3B)+TLK_PKT_INFO(12B)
*             **use 2B enough**
*/
#define     ISO_BIS_RX_PDU_SIZE_ALIGN16(n)          (((n + 25) + 15) / 16 * 16) //4+2+4+2+4+3+12

//12 = 4(struct bis_rx_pdu_tag  *next) + 4(u32 payloadNum) + 4(u32 idealPldAnchorTick) in bis_rx_pdu_t
#define     BIS_LL_RX_PDU_FIFO_SIZE(n)              (CAL_LL_ISO_RX_FIFO_SIZE(n) + 12)


/*
*  DMA_len(4us)
*
*  ExtraInfo(sizeof(cs_rx_para_t) = 62us)
*
*  Mode2_len: rx_early_us(5us) + (T_PM + T_SW)*(AntPath +1) - T_SW
*  Mode1_len: rx_early_us(5us) + AA_Only(44us) + Sequence(maximum 128us) + extend (15us) = 192us
*  Mode0_len: T_FM(80us)
*
*  Max_mode_len = max3(Mode2_len,Mode1_len,Mode0_len)
*
*  IQ sample_1us :  4[sample rate is 4Mhz] * 5[IQ_20_BIT] = 20 bytes
*
*  buffer_len = DMA_len(bytes) + Max_mode_len * IQ sample_1us + ExtraInfo(sizeof(cs_rx_para_t) 80bytes already contains redundancy) 
*
*  RX buffer size must be be 16*n, due to MCU design
*
*  RX buffer size : ((buffer_len + 15)/16) *16
*
*/
#define     CS_EXTRAINFO_LEN                                                 96
#define     CS_ALIGN_16(len)                                                 (((len + 15)>>4) <<4)
#define     CS_RX_MODE0_FIFO_SIZE_MAX                                        CS_ALIGN_16(80*20 + CS_EXTRAINFO_LEN + 4)
#define     CS_RX_MODE1_FIFO_SIZE_MAX                                        CS_ALIGN_16((5 + 44 + 128 + 15)*20 + CS_EXTRAINFO_LEN + 4)
#define     CS_RX_MODE2_FIFO_SIZE_MAX(AP,PM, SW)                             CS_ALIGN_16((5 + (AP+1)*(PM+SW) - SW)*20 + CS_EXTRAINFO_LEN + 4)
#define     CHANNEL_SOUNDING_RX_FIFO_SIZE_ALIGN16(N_AP, T_PM_US, T_SW_US)    max3(CS_RX_MODE0_FIFO_SIZE_MAX,CS_RX_MODE1_FIFO_SIZE_MAX, CS_RX_MODE2_FIFO_SIZE_MAX(N_AP, T_PM_US, T_SW_US))

/*
*  transport_len(4 bytes)
*
*  Mode0_len: (10us) * 20 bytes(IQ sample_1us = 20 bytes) + 4 bytes(transport_len) + extend
*  Mode1_len: rx_early_us(5us) + AA_Only(44us) + Sequence(maximum 128us) + extend
*  Mode2_len: 4 bytes(transport_len) + ExtraInfo(sizeof(cs_rx_para_t) 80bytes already contains redundancy) + (T_PM(8us valid))*(AntPath +1) * 20 bytes(IQ sample_1us = 20 bytes)
*
*  Max_mode_len = max3(Mode2_len,Mode1_len,Mode0_len)
*
*  RX buffer size must be be 16*n, due to MCU design
*
*  RX buffer size : ((buffer_len + 15)/16) *16
*
*/
#define CS_TRANSPORT_RX_MODE0_FIFO_SIZE_MAX                                        CS_ALIGN_16(10*20 + CS_EXTRAINFO_LEN + 4)
#define CS_TRANSPORT_RX_MODE1_FIFO_SIZE_MAX                                        CS_ALIGN_16((5 + 44 + 128 + 15)*20 + CS_EXTRAINFO_LEN + 4)
#define CS_TRANSPORT_RX_MODE2_FIFO_SIZE_MAX(AP,PM, SW)                             CS_ALIGN_16(((AP+1)*(8))*20 + CS_EXTRAINFO_LEN + 4)
#define CHANNEL_SOUNDING_TRANSPORT_RX_FIFO_SIZE_WITHOUT_MODE1_ALIGN16(N_AP, T_PM_US, T_SW_US)    max2(CS_TRANSPORT_RX_MODE0_FIFO_SIZE_MAX, CS_TRANSPORT_RX_MODE2_FIFO_SIZE_MAX(N_AP, T_PM_US, T_SW_US))
#define CHANNEL_SOUNDING_TRANSPORT_RX_FIFO_SIZE_ALIGN16(N_AP, T_PM_US, T_SW_US)                  max3(CS_TRANSPORT_RX_MODE0_FIFO_SIZE_MAX, CS_TRANSPORT_RX_MODE1_FIFO_SIZE_MAX, CS_TRANSPORT_RX_MODE2_FIFO_SIZE_MAX(N_AP, T_PM_US, T_SW_US))

/******************************* dma_end ********************************************************************/


/******************************* cs_ant_switch_start *********************************************************/
typedef struct{
    gpio_pin_e  pin;
    gpio_func_e  pin_fun;
}rf_cs_ant_switch_ctrl;

/**
 * @brief        * This function initializes the GPIO pins used for antenna switching according to the specified configuration.
 *                 It disables the input and output functions of the GPIO pins, sets the pull-down resistance, and configures
 *                 the multiplexing function of the pins.
 *
 * @param[in]   switch_ctrl    - Pointer to the antenna switch control structure array.
 * @param[in]   num            - Number of antenna switch pins
 * @return      none.
 */
void rf_cs_ant_switch_pin_init(rf_cs_ant_switch_ctrl *switch_ctrl, u8 num);

/******************************* cs_ant_switch_end **********************************************************/


#endif /* DRIVERS_TL721X_EXT_MISC_H_ */
