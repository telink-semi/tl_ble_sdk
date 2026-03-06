/********************************************************************************************************
 * @file    adc.h
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
/** @page ADC
 *
 *  Introduction
 *  ===============
 *  TL321X supports hardware ADC function.
 *
 *  API Reference
 *  ===============
 *  Header File: adc.h
 */
#pragma once

#include "dma.h"
#include "audio.h"
#include "compiler.h"
#include "gpio.h"
#include "reg_include/register.h"
#include "pem.h"

#ifndef INTERNAL_TEST_FUNC_EN
#define INTERNAL_TEST_FUNC_EN            0//only for internal test
#endif

extern unsigned char g_adc_rx_fifo_index[2];

#define ADC_SAR_CNT  2  //SAR0 and SAR1
/**
 *  @brief  Define ADC chn
 */
typedef enum
{
    ADC0 = 0,
    ADC1,
} adc_num_e;

/**
 * @brief pem event module.
 */
typedef enum{
    ADC_PEM_EVENT_RX_THRESHOLD,
    ADC_PEM_EVENT_RX_DATA_FIFO_WR,
}adc_pem_event_module_sel_e;

typedef enum{
    ADC_VREF_1P2V = 0x00,
} adc_ref_vol_e;

typedef enum{
    ADC_VBAT_DIV_1F4 = 0x01,
} adc_vbat_div_e;

/**
 * @brief Define ADC negative input.
 * |           |              |
 * | :-------- | :----------- |
 * |   <15:12> |    <11:0>    |
 * |adc channel|    gpio pin  |
 * @note SAR0 utilizes the PC port for input.
 *       SAR1 utilizes the PB port for input.
 */
typedef enum
{
    ADC0_GPIO_PC0N = GPIO_PC0 | (0x1 << 12),
    ADC0_GPIO_PC1N = GPIO_PC1 | (0x2 << 12),
    ADC0_GPIO_PC2N = GPIO_PC2 | (0x3 << 12),
    ADC0_GPIO_PC3N = GPIO_PC3 | (0x4 << 12),
    ADC0_GPIO_PC4N = GPIO_PC4 | (0x5 << 12),
    ADC0_GPIO_PC5N = GPIO_PC5 | (0x6 << 12),
    ADC0_GPIO_PC6N = GPIO_PC6 | (0x7 << 12),
    ADC0_GPIO_PC7N = GPIO_PC7 | (0x8 << 12),

    ADC1_GPIO_PB0N = GPIO_PB0 | (0x1 << 12),
    ADC1_GPIO_PB1N = GPIO_PB1 | (0x2 << 12),
    ADC1_GPIO_PB2N = GPIO_PB2 | (0x3 << 12),
    ADC1_GPIO_PB3N = GPIO_PB3 | (0x4 << 12),
    ADC1_GPIO_PB4N = GPIO_PB4 | (0x5 << 12),
    ADC1_GPIO_PB5N = GPIO_PB5 | (0x6 << 12),
    ADC1_GPIO_PB6N = GPIO_PB6 | (0x7 << 12),
    ADC1_GPIO_PB7N = GPIO_PB7 | (0x8 << 12),
#if INTERNAL_TEST_FUNC_EN
    ADC_TEMPSENSORN_N = 0x00 | (0xa << 12),
#endif
    ADC_GND_N      = 0x00     | (0xb << 12), // Only SAR0 is supported.
} adc_input_nch_e;
/**
 * @brief Define ADC positive input.
 * |           |              |
 * | :-------- | :----------- |
 * |   <15:12> |    <11:0>    |
 * |adc channel|    gpio pin  |
 * @note SAR0 utilizes the PC port for input.
 *       SAR1 utilizes the PB port for input.
 */
typedef enum
{
    ADC0_GPIO_PC0P = GPIO_PC0 | (0x1 << 12),
    ADC0_GPIO_PC1P = GPIO_PC1 | (0x2 << 12),
    ADC0_GPIO_PC2P = GPIO_PC2 | (0x3 << 12),
    ADC0_GPIO_PC3P = GPIO_PC3 | (0x4 << 12),
    ADC0_GPIO_PC4P = GPIO_PC4 | (0x5 << 12),
    ADC0_GPIO_PC5P = GPIO_PC5 | (0x6 << 12),
    ADC0_GPIO_PC6P = GPIO_PC6 | (0x7 << 12),
    ADC0_GPIO_PC7P = GPIO_PC7 | (0x8 << 12),

    ADC1_GPIO_PB0P = GPIO_PB0 | (0x1 << 12),
    ADC1_GPIO_PB1P = GPIO_PB1 | (0x2 << 12),
    ADC1_GPIO_PB2P = GPIO_PB2 | (0x3 << 12),
    ADC1_GPIO_PB3P = GPIO_PB3 | (0x4 << 12),
    ADC1_GPIO_PB4P = GPIO_PB4 | (0x5 << 12),
    ADC1_GPIO_PB5P = GPIO_PB5 | (0x6 << 12),
    ADC1_GPIO_PB6P = GPIO_PB6 | (0x7 << 12),
    ADC1_GPIO_PB7P = GPIO_PB7 | (0x8 << 12),
#if INTERNAL_TEST_FUNC_EN
    ADC_TEMPSENSORN_P = 0x00 | (0xa << 12),
#endif
    ADC_VBAT_P     = 0x00     | (0xb << 12), // Only SAR0 is supported.
} adc_input_pch_e;

/**
 *  @brief  Define ADC mode
 */
typedef enum
{
    ADC_GPIO_MODE = 0,
    ADC_VBAT_MODE = 1,// Only supports sar0
#if INTERNAL_TEST_FUNC_EN
    ADC_TEMP_MODE = 2,
#endif
} adc_mode_e;

/**
 * @brief adc sample channel type.
 * @attention
 *  #1 Only M channel can be used in NDMA mode, multiple channels can be used in DMA mode.
 *  #2 When using one channels in DMA mode, only M channel can be selected.
 *  #3 When using two channels in DMA mode, only M and L channels can be selected.
 */
typedef enum
{
    ADC_M_CHANNEL,
#if INTERNAL_TEST_FUNC_EN
    ADC_L_CHANNEL,
    ADC_R_CHANNEL,
#endif
} adc_sample_chn_e;

/**
 * @brief adc channel cnt type.
*|             |             |
*| :--------   | :-----------|
*|   <7:4>     |    <3:0>    |
*|transfer_mode| channel_cnt |
**/
typedef enum
{
    NDMA_M_CHN    = 1 | (0 << 4),
    DMA_M_CHN     = 1 | (1 << 4),
#if INTERNAL_TEST_FUNC_EN
    DMA_M_L_CHN   = 2 | (1 << 4),
    DMA_M_L_R_CHN = 3 | (1 << 4),
#endif
}adc_chn_cnt_e;

/**
 * @brief enumeration of adc resolutions.
 * @note  #1 The adc performs best when set to ADC_RES12 by default, and it is not recommended to change this configuration.
 *        #2 The sign and valid bits corresponding to adc code for setting different resolutions are as follows:
 *           If ADC_RES8 is set, the sign bit of adc code is bit11 and bit0-6 are valid bits.
 *           If ADC_RES10 is set, the sign bit of adc code is bit11 and bit0-8 are valid bits.
 *           If ADC_RES12 is set, the sign bit of adc code is bit11 and bit0-10 are valid bits (default)
 *        #3 Regardless of the set resolution, the final output code value is always 16bit.
 */
typedef enum
{
    ADC_RES8  = 0,
    ADC_RES10 = 0x01,
    ADC_RES12 = 0x02,
} adc_res_e;

typedef enum
{
    ADC_SAMPLE_CYC_4,
    ADC_SAMPLE_CYC_6,
    ADC_SAMPLE_CYC_8,
    ADC_SAMPLE_CYC_10,
    ADC_SAMPLE_CYC_12,
    ADC_SAMPLE_CYC_14,
    ADC_SAMPLE_CYC_16,
    ADC_SAMPLE_CYC_18,
    ADC_SAMPLE_CYC_20,
    ADC_SAMPLE_CYC_22,
    ADC_SAMPLE_CYC_24,
    ADC_SAMPLE_CYC_26,
    ADC_SAMPLE_CYC_28,
    ADC_SAMPLE_CYC_30,
    ADC_SAMPLE_CYC_32,
    ADC_SAMPLE_CYC_34,
} adc_sample_cycle_e;

/**
 * @brief adc sample frequency type.
 * |         |           |             |             |
 * | :-------|  :--------| :--------   | :-----------|
 * | <25:24> |  <23:8>   |   <7:4>     |    <3:0>    |
 * |  flag   |  capture  | sample_cycle|     set     |
 *
 */
typedef enum
{

    ADC_SAMPLE_FREQ_23K  = (0 << 24) | (1037 << 8) | (ADC_SAMPLE_CYC_34 << 4) | 1,
    ADC_SAMPLE_FREQ_48K  = (1 << 24) | (499 << 8)  | (ADC_SAMPLE_CYC_34 << 4) | 1,
    ADC_SAMPLE_FREQ_96K  = (2 << 24) | (249 << 8)  | (ADC_SAMPLE_CYC_26 << 4) | 1,
    ADC_SAMPLE_FREQ_192K = (3 << 24) | (124 << 8)  | (ADC_SAMPLE_CYC_6 << 4)  | 1,
    /**
     * When using a 2M or 1M sampling rate, the ADC analog clock must be 48M, and the PLL clock must be a multiple of 48M.
     * VBAT sampling does not support 2M or 1M sampling rates.
     */
    ADC_SAMPLE_FREQ_1M  =  (3<<24)   |(45<<8)      | (14<<4)                   | 2,
    ADC_SAMPLE_FREQ_2M  =  (3<<24)   |(21<<8)      | (1<<4)                    | 2,
} adc_sample_freq_e;

typedef enum
{
    ADC_PRESCALE_1 = 0x00,
    ADC_PRESCALE_1F2 = 0x01,//Only for internal testing
    ADC_PRESCALE_1F4 = 0x02,
    ADC_PRESCALE_1F8 = 0x03,//Only for internal testing
} adc_pre_scale_e;

typedef struct
{
    adc_pre_scale_e   pre_scale;
    adc_sample_freq_e sample_freq;
    adc_input_pch_e   input_p;
    adc_input_nch_e   input_n;
} adc_chn_cfg_t;

typedef enum
{
    DMA,
    NDMA,
} adc_transfer_mode_e;

typedef enum{
    FLD_CLOCK_RC,
    FLD_CLOCK_XTL,
    FLD_CLOCK_PLL,
}adc_dig_clk_src_e;
/**********************************************************************************************************************
 *                                         DMA and NDMA common interface                                              *
 **********************************************************************************************************************/
/**
 * @brief    This function is used to power on sar_adc.
 * @param[in]  sar_adc_num - SAR0/SAR1.
 * @return   none.
 * @note     -# User need to wait >30us after adc_power_on() for ADC to be stable.
 *           -# If you calling adc_power_off(), because all analog circuits of ADC are turned off after adc_power_off(),
 *            it is necessary to wait >30us after re-adc_power_on() for ADC to be stable.
 */
void adc_power_on(adc_num_e sar_adc_num);
/**
 * @brief      This function is used to power off sar_adc.
 * @param[in]  sar_adc_num - SAR0/SAR1.
 * @return     none
 */
void adc_power_off(adc_num_e sar_adc_num);
/**
 * @brief This function serves to set input channel in differential_mode.
 * @param[in]  sar_adc_num - SAR0/SAR1.
 * @param[in]  chn - enum variable of ADC sample channel.
 * @param[in]  p_ain - enum variable of ADC analog positive input pin.
 * @param[in]  n_ain - enum variable of ADC analog negative input pin.
 * @return none
 */
static inline void adc_set_diff_input(adc_num_e sar_adc_num,adc_sample_chn_e chn,adc_input_pch_e p_ain,adc_input_nch_e n_ain)
{
    analog_write_reg8(areg_adc_res_m(sar_adc_num) , analog_read_reg8(areg_adc_res_m(sar_adc_num)) | FLD_ADC_EN_DIFF_CHN_M);
    reg_adc_r_mux(sar_adc_num,chn) = ((p_ain>>12) | ((n_ain>>12)<<4)) ;

}
/**
 * @brief This function is used to set IO port for ADC supply or ADC IO port voltage sampling.
 * @param[in]  pin - gpio_pin_e ADC input gpio pin
 * @return none
 */
void adc_pin_config(gpio_pin_e pin);

/**
 * @brief      This function serves to set Vbat voltage division factor.
 * @param[in]  sar_adc_num - SAR0/SAR1.
 * @param[in]  chn - enum variable of ADC sample channel
 * @return     none
 */
void adc_set_vbat_divider(adc_num_e sar_adc_num,adc_sample_chn_e chn);

/**
 * @brief This function is used to initialize the ADC.
 * @param[in]  sar_adc_num - SAR0/SAR1.
 * @param[in]  channel_cnt - transfer_mode and the number of channels used.
 * @return none
 * @attention Many features are configured in adc_init function. But some features
 *      such as adc digital clk, adc analog clk, resolution, we think better to set as default value,
 *      and user don't need to change them in most use cases.
 */
void adc_init(adc_num_e sar_adc_num,adc_chn_cnt_e channel_cnt);
/**
 * @brief This function is used to initialize the ADC for sampling.
 * @param[in]  sar_adc_num - SAR0/SAR1.
 * @param[in]  mode - ADC_GPIO_MODE/ADC_VBAT_MODE/ADC_TEMP_MODE.
 * @param[in]  chn -the channel to be configured.
 * @param[in]  cfg -structure pointer for configuring ADC channel.
 * @return none
 */
void adc_channel_sample_init(adc_num_e sar_adc_num,adc_mode_e mode,adc_sample_chn_e chn,adc_chn_cfg_t *cfg);

/**
 * @brief This function is used to initialize the ADC for keyscan sampling.
 * @param[in]  sar_adc_num - SAR0/SAR1.
 * @note  The sampling rate for keyscan must be set to 2M.
 *        The PLL clock must be a multiple of 48M.    
 * @return none
 */
void adc_keyscan_sample_init(adc_num_e sar_adc_num);
#if INTERNAL_TEST_FUNC_EN

/**
 * @brief This function serves to calculate temperature from temperature sensor adc sample code.
 * @param[in]   adc_code            - the temperature sensor adc sample code.
 * @return      adc_temp_value      - the temperature value.
 * @attention   Temperature and adc_code are linearly related. We test four chips between -40~130 (Celsius) and got an average relationship:
 *          Temp =  564 - ((adc_code * 819)>>11),when Vref = 1.2V, pre_scale = 1.
 */
 short adc_calculate_temperature(short adc_code);
/**
 * @brief      This function serves to set multiple acquisition data averaging
 * @param[in]  sar_adc_num - SAR0/SAR1.
 * @param[in]  cnt - Set cnt data to be averaged once.
 * @return     none
 */
void adc_set_oversample_cnt(adc_num_e sar_adc_num,char cnt);

/**
 * @brief      This function disable oversample mode.
 * @param[in]  sar_adc_num - SAR0/SAR1.
 * @return     none
 * @note       Hardware averaging mode
 */

void adc_oversample_en(adc_num_e sar_adc_num);
/**
 * @brief      This function enable trigger mode.
 * @param[in]  sar_adc_num - SAR0/SAR1.
 * @return     none
 * @note       Manual trigger required
 */
void adc_oversample_dis(adc_num_e sar_adc_num);

#endif

/**
 * @brief This function serves to calculate voltage from adc sample code.
 * @param[in]   sar_adc_num - SAR0/SAR1.
 * @param[in]   mode - ADC sample mode
 * @param[in]   chn - enum variable of ADC sample channel.
 * @param[in]   adc_code    - the adc sample code.
 * @return      adc_vol_mv  - the average value of adc voltage value.
 */
short adc_calculate_voltage(adc_num_e sar_adc_num,adc_mode_e mode,adc_sample_chn_e chn,short adc_code);

/**
 * @brief       This function sets the threshold that triggers the DMA IRQ or SAR ADC RX IRQ.
 * @param[in]   sar_adc_num - SAR0/SAR1.
 * @param[in]   trig_num -trigger threshold.
 * @return      none
 * @note        -# The DMA IRQ or SAR ADC RX IRQ is triggered when fifo_cnt> trig_cnt.
 *              -# unit:half word, range:0-7(the actual fifo depth is 8 words, but the register is only 3 bits, so it is limited to 0~7).
 *              -# The data of rx fifo is stored according to HALF word.
 *              -# In DMA mode, DMA is configured to transmit by word, so the trigger cnt must be set to an odd number, otherwise there will be incorrect data to the buffer.
 *              -# In DMA mode, it has been configured by default to 1 (greater than 1 half word, i.e., 1 word triggers a DMA request to the DMA once ).
 *              -# In NDMA mode, the threshold can be configured according to the usage situation.
 *              -# The threshold is recalculated after the dma has finished moving.
 */
static inline void adc_set_rx_fifo_trig_cnt(adc_num_e sar_adc_num,unsigned char trig_num)
{
    reg_adc_rxfifo_trig_num(sar_adc_num) = ((reg_adc_rxfifo_trig_num(sar_adc_num) & (~FLD_RXFIFO_TRIG_NUM)) | trig_num);
}
/**********************************************************************************************************************
 *                                                DMA only interface                                                  *
 **********************************************************************************************************************/
/**
 * @brief      This function serves to configure adc_dma_chn channel.
 * @param[in]  sar_adc_num - SAR0/SAR1.
 * @param[in]  chn - the DMA channel
 * @return     none
 */
void adc_set_dma_config(adc_num_e sar_adc_num,dma_chn_e chn);

/**
 * @brief     The adc starts sampling in DMA mode.
 * @param[in]  sar_adc_num - SAR0/SAR1.
 * @param[in] adc_data_buf  - Pointer to data buffer, it must be 4-bytes aligned address
 *                            and the actual buffer size defined by the user needs to be not smaller than the data_byte_len,
 *                            otherwise there may be an out-of-bounds problem.
 * @param[in] data_byte_len - Amount of data to be sampled in bytes, the maximum value can be up to 0xFFFFFC.
 * @note      The depth of SAR_ADC_FIFO is 8 words, and its characteristic is store first, fetch first.
 *            When all three channels are working at the same time, the order of storing adc channel data into SAR_ADC_FIFO is M: L: R,
 *            so the arrangement after dma handling is also M: L: R.
 * @return    none
 */
void adc_start_sample_dma(adc_num_e sar_adc_num,short *adc_data_buf,unsigned int data_byte_len);

/**
 * @brief     This function serves to get adc DMA irq status.
 * @return    0: the sample is in progress.
 *            !0: the sample is finished.
 * @note      The code is placed in the ram code sectionunsignedunsigned, in order to shorten the time.
 */
_attribute_ram_code_sec_noinline_ unsigned char adc_get_irq_status_dma(void);

/**
 * @brief     This function serves to clear adc DMA irq status.
 * @param[in] sar_adc_num - SAR0/SAR1.
 * @return    none
 * @note      The code is placed in the ram code section, in order to shorten the time.
 */
_attribute_ram_code_sec_noinline_ void adc_clr_irq_status_dma(adc_num_e sar_adc_num);


/**********************************************************************************************************************
 *                                                NDMA only interface                                                 *
 **********************************************************************************************************************/
/**
 * @brief This function serves to directly get an adc sample code from fifo.
 * @param[in]  sar_adc_num - SAR0/SAR1.
 * @return  adc_code    - the adc sample code.
 *                      - Bit[11:15] of the adc code read from reg_adc_rxfifo_dat are sign bits,if the adc code is positive, bits [11:15] are all 1's,
 *                        if the adc code is negative, bits [11:15] are all 0's and valid data bits are Bit[0:10],the valid range is 0~0x7FF.
 */
short adc_get_raw_code(adc_num_e sar_adc_num);

/**
 * @brief     Get the irq status of ADC.
 * @param[in]  sar_adc_num - SAR0/SAR1.
 * @retval    non-zero      -  the interrupt occurred.
 * @retval    zero  -  the interrupt did not occur.
 */
static inline unsigned char adc_get_irq_status(adc_num_e sar_adc_num)
{
    return reg_soft_control(sar_adc_num) & FLD_SAR_IRQ_RX_STATUS;
}

/**
 * @brief     Clear the irq status of ADC.
 * @param[in]  sar_adc_num - SAR0/SAR1.
 * @return    none
 * @note      The irq status of ADC will not be cleared automatically after reading out the data from the rxfifo, you need to clear manually.
 */
static inline void adc_clr_irq_status(adc_num_e sar_adc_num)
{
    reg_soft_control(sar_adc_num) |= FLD_SAR_IRQ_RX_STATUS;
}

/**
 * @brief   This function is used to enable the transmission of data from the adc's M channel, L channel, and R channel to the sar adc rxfifo.
 * @param[in]  sar_adc_num - SAR0/SAR1.
 * @return  none
 */
void adc_start_sample_nodma(adc_num_e sar_adc_num);

/**
 * @brief   This function is used to disable the transmission of data from the adc's M channel, L channel, and R channel to the sar adc rxfifo and clear rx fifo cnt and g_adc_rx_fifo_index.
 * @param[in]  sar_adc_num - SAR0/SAR1.
 * @return  none
 */
void adc_stop_sample_nodma(adc_num_e sar_adc_num);

/**
 * @brief     Get the rxfifo cnt,when data enters rxfifo, the rxfifo cnt increases; when reading data from rx_fifo, rxfifo cnt decays.
 * @param[in]  sar_adc_num - SAR0/SAR1.
 * @return    none
 * @note      After waking up from suspend mode,rxfifo will not be cleared.(confirmed by qiangkai.xu, added by xiaobin.huang at 20240903)
 */
static inline unsigned char adc_get_rxfifo_cnt(adc_num_e sar_adc_num)
{
    return ((reg_adc_rxfifo_trig_num(sar_adc_num) & FLD_BUF_CNT)>>4);
}

/**
 * @brief     Set the 'g_adc_rx_fifo_index' to 0,'g_adc_rx_fifo_index' is used to synchronize the rxfifo hardware pointer in NDMA mode.
 * @param[in]  sar_adc_num - SAR0/SAR1.
 * @return    none.
 */
static inline void adc_clr_rx_index(adc_num_e sar_adc_num)
{
    g_adc_rx_fifo_index[sar_adc_num] = 0;
}

/**
 * @brief     Clear the ADC rx fifo.
 * @param[in]  sar_adc_num - SAR0/SAR1.
 * @return    none
 */
static inline void adc_clr_rx_fifo_cnt(adc_num_e sar_adc_num)
{
    reg_soft_control(sar_adc_num) |= FLD_FIFO_CLR;
    adc_clr_rx_index(sar_adc_num);
}
/**
* @brief      This function select adc source clock.
* @param[in]  sar_adc_num - SAR0/SAR1.
* @param      adc_clk - dig adc clock source.
* @param      div - dig adc clk division coefficient.
* @return     none
*/
static inline void adc_select_clk(adc_num_e sar_adc_num,adc_dig_clk_src_e adc_clk,char div)
{
    reg_adc_dig_clk_set(sar_adc_num)= (((reg_adc_dig_clk_set(sar_adc_num) & ~FLD_ADC_DIG_DIV_IN_SEL) | ((adc_clk & 0x03) << 4))  & ~FLD_ADC_DIG_CLK_MOD) | (div & 0x0F);
}
/**
 * @brief This function is used to configure the channel of the ADC.
 * @param[in]  sar_adc_num - SAR0/SAR1.
 * @param[in]  chn -the channel to be configured.
 * @param[in]  adc_cfg -structure pointer for configuring ADC channel.
 * @attention  When using a 2M or 1M sampling rate, the ADC analog clock must be 48M, and the PLL clock must be a multiple of 48M.
 * @return none
 */
void adc_chn_config(adc_num_e sar_adc_num,adc_sample_chn_e chn,adc_chn_cfg_t *adc_cfg);

/**
 * @brief      This function enable keyscan mode.
 * @param[in]  sar_adc_num - SAR0/SAR1.
 * @return     none
 * @note       auto keyscan required
 */
static inline void adc_keyscan_auto_en(adc_num_e sar_adc_num)
{
    reg_adc_oversample(sar_adc_num) |= FLD_PAD_AUTO_MUX_EN;
}

/**
 * @brief      This function disable keyscan mode.
 * @return     none
 * @note       auto keyscan required
 */
static inline void adc_keyscan_auto_dis(adc_num_e sar_adc_num)
{
    reg_adc_oversample(sar_adc_num) &= (~FLD_PAD_AUTO_MUX_EN);
}

/**
 * @brief      This function start trigger.
 * @param[in]  sar_adc_num - SAR0/SAR1.
 * @return     none
 * @note       Start collecting adc data
 */
static inline void adc_trigger_start(adc_num_e sar_adc_num)
{
    reg_soft_control(sar_adc_num) = FLD_TRIG_START;
}

/**
 * @brief      This function serves to configure the PEM event.
 * @param[in]  sar_adc_num - SAR0/SAR1.
 * @param[in]  chn - to select the PEM channel.
 * @param[in]  event_signal - to select the event signal.
 * @return     none.
 */
void adc_set_pem_event(adc_num_e sar_adc_num,pem_chn_e chn,unsigned char adc_sel);
/**
 * @brief      This function serves to configure the PEM task.
 * @param[in]  sar_adc_num - SAR0/SAR1.
 * @param[in]  chn - to select the PEM channel.
 * @return     none.
 */
void adc_set_pem_task(adc_num_e sar_adc_num,pem_chn_e chn);
/**
 * @brief      This function sets adc digital clock and analog clock.
 * @param[in]  sar_adc_num - SAR0/SAR1.
 * @param      div - Analog adc frequency division coefficient.
 * @return     none
 * @attention  The ADC digital clock is driven by a crystal (24MHz) or PLL, while the analog clock is driven by the digital clock.
 */
void adc_set_clk(adc_num_e sar_adc_num,char div);
/**
 * @brief      This function serves to sets the number of data collected by the trigger at a time.
 * @param[in]  sar_adc_num - SAR0/SAR1.
 * @param[in]  cnt - Set trigger to collect cnt data at a time.
 * @return     none
 */
void adc_set_trigger_cnt(adc_num_e sar_adc_num,char cnt);
/**
 * @brief      This function enable oversample mode.
 * @param[in]  sar_adc_num - SAR0/SAR1.
 * @return     none
 * @note       Hardware averaging mode
 */
void adc_trigger_en(adc_num_e sar_adc_num);

/**
 * @brief      This function disable trigger mode.
 * @param[in]  sar_adc_num - SAR0/SAR1.
 * @return     none
 * @note       Manual trigger required
 */
void adc_trigger_dis(adc_num_e sar_adc_num);

/**
 * @brief This function is used to calib ADC 1.2V vref for SAR0 GPIO.
 * @param[in] vref - GPIO sampling two-point calibration value.
 * @param[in] offset - GPIO sampling two-point calibration value offset.
 * @return none
 */
void adc_set_sar0_gpio_calib_vref(unsigned short vref, signed char offset);

/**
 * @brief This function is used to calib ADC 1.2V vref for SAR0 Vbat.
 * @param[in] vref - Vbat channel sampling two-point calibration value.
 * @param[in] offset - Vbat channel sampling two-point calibration value offset.
 * @return none
 */
void adc_set_sar0_vbat_calib_vref(unsigned short vref, signed char offset);

/**
 * @brief This function is used to calib ADC 1.2V vref for SAR1 GPIO.
 * @param[in] vref - GPIO sampling two-point calibration value.
 * @param[in] offset - GPIO sampling two-point calibration value offset.
 * @return none
 */
void adc_set_sar1_gpio_calib_vref(unsigned short vref, signed char offset);