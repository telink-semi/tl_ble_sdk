/********************************************************************************************************
 * @file    adc.c
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
#include "adc.h"
#include "audio.h"
#include "compiler.h"
#include "lib/include/stimer.h"
/**********************************************************************************************************************
 *                                           Declare variables                                                             *
 *********************************************************************************************************************/
extern drv_api_status_e efuse_get_sar_drv_flag(unsigned int * flag);

/**********************************************************************************************************************
 *                                              local macro                                                        *
 *********************************************************************************************************************/
#define ADC_CHN_CNT                        3

/* The default value is the one obtained through testing under high driving capability conditions (0x1d cfg) and is provided by the ATE.
 * When temp_code is 0 (unconfigured or not programmed in eFuse), the system defaults to the high-drive configuration. 
 * Therefore, the median value derived from large-scale data under the high-drive mode can be directly used as the default value.
*/
#define SAR0_ADC_GPIO_VREF_DEFAULT_VALUE        1175
#define SAR0_ADC_GPIO_VREF_OFFSET_DEFAULT_VALUE 0

#define SAR0_ADC_VBAT_VREF_DEFAULT_VALUE        1175
#define SAR0_ADC_VBAT_VREF_OFFSET_DEFAULT_VALUE 0

#define SAR1_ADC_GPIO_VREF_DEFAULT_VALUE        1190
#define SAR1_ADC_GPIO_VREF_OFFSET_DEFAULT_VALUE 0


_attribute_data_retention_sec_ unsigned short g_adc_vref[ADC_SAR_CNT][ADC_CHN_CNT];                                              //ADC calibration value voltage table(unit:mV).
_attribute_data_retention_sec_ signed char     g_adc_vref_offset[ADC_SAR_CNT][ADC_CHN_CNT];                                      //ADC calibration value voltage offset table(unit:mV).
_attribute_data_retention_sec_ unsigned short g_adc_sar0_gpio_calib_vref        = SAR0_ADC_GPIO_VREF_DEFAULT_VALUE;              //sar0 gpio calibration value voltage (unit:mV)(used for gpio voltage sample).
_attribute_data_retention_sec_ signed char    g_adc_sar0_gpio_calib_vref_offset = SAR0_ADC_GPIO_VREF_OFFSET_DEFAULT_VALUE;       //sar0 gpio calibration value voltage offset (unit:mV)(used for gpio voltage sample).
_attribute_data_retention_sec_ unsigned short g_adc_sar0_vbat_calib_vref        = SAR0_ADC_VBAT_VREF_DEFAULT_VALUE;              //sar0 vbat calibration value voltage (unit:mV)(used for vbat voltage sample).
_attribute_data_retention_sec_ signed char    g_adc_sar0_vbat_calib_vref_offset = SAR0_ADC_VBAT_VREF_OFFSET_DEFAULT_VALUE;       //sar0 vbat calibration value voltage offset(unit:mV)(used for vbat voltage sample).
_attribute_data_retention_sec_ unsigned short g_adc_sar1_gpio_calib_vref        = SAR1_ADC_GPIO_VREF_DEFAULT_VALUE;              //sar1 gpio calibration value voltage (unit:mV)(used for gpio voltage sample).
_attribute_data_retention_sec_ signed char    g_adc_sar1_gpio_calib_vref_offset = SAR1_ADC_GPIO_VREF_OFFSET_DEFAULT_VALUE;       //sar1 gpio calibration value voltage offset (unit:mV)(used for vbat voltage sample).


volatile unsigned char g_adc_pre_scale[ADC_SAR_CNT][ADC_CHN_CNT];
unsigned char          g_adc_rx_fifo_index[ADC_SAR_CNT];
unsigned char          g_channel_cnt       = 0;

dma_chn_e    adc_dma_chn;
dma_config_t adc_rx_dma_config =
    {
        .dst_req_sel    = 0,
        .src_req_sel    = DMA_REQ_SAR_ADC_RX,
        .dst_addr_ctrl  = DMA_ADDR_INCREMENT,
        .src_addr_ctrl  = DMA_ADDR_FIX,
        .dstmode        = DMA_NORMAL_MODE,
        .srcmode        = DMA_HANDSHAKE_MODE,
        .dstwidth       = DMA_CTR_WORD_WIDTH, //must word
        .srcwidth       = DMA_CTR_WORD_WIDTH, //must word
        .src_burst_size = 0,                  //must 0
        .read_num_en    = 0,
        .priority       = 0,
        .write_num_en   = 0,
        .auto_en        = 0, //must 0
};
dma_config_t adc1_rx_dma_config=
{
    .dst_req_sel= 0,
    .src_req_sel=DMA_REQ_SAR_ADC1_RX,
    .dst_addr_ctrl=DMA_ADDR_INCREMENT,
    .src_addr_ctrl=DMA_ADDR_FIX,
    .dstmode=DMA_NORMAL_MODE,
    .srcmode=DMA_HANDSHAKE_MODE,
    .dstwidth=DMA_CTR_WORD_WIDTH,//must word
    .srcwidth=DMA_CTR_WORD_WIDTH,//must word
    .src_burst_size=0,//must 0
    .read_num_en=0,
    .priority=0,
    .write_num_en=0,
    .auto_en=0,//must 0
};
/**
 * @brief      This setting serves to set the configuration of stimer PEM event.
 */
pem_event_config_t adc_pem_event_config={
            .module         = PEM_EVENT_SAR_ADC,
            .sig_sel        = 0,
            .clk_sel        = PCLK,
            .lvl            = PULSE,
            .edge_detect    = RISING_EDGE,
            .inv            = 0,
};

/**
 * @brief      This setting serves to set the configuration of stimer PEM task.
 */
pem_task_config_t adc_pem_task_config={
            .module         = PEM_TASK_SAR_ADC,
            .sig_sel        = 0,
            .clk_sel        = PCLK,
            .lvl            = PULSE,
};

/**********************************************************************************************************************
 *                                         DMA and NDMA common interface                                              *
 **********************************************************************************************************************/
/**
 * @brief      This function is used to reset sar_adc module.
 * @param[in]  sar_adc_num - SAR0/SAR1.
 * @return     none
 */
static inline void adc_reset(adc_num_e sar_adc_num)
{
    if(sar_adc_num == 0){
        reg_rst3 &= (~FLD_RST3_SARADC );
        reg_rst3 |=FLD_RST3_SARADC;
    }
    else{
        reg_rst5 &= (~FLD_RST5_SAR1ADC );
        reg_rst5 |=FLD_RST5_SAR1ADC;
    }
    adc_clr_rx_index(sar_adc_num);
}

/**
 * @brief      This function enable adc source clock: xtal_24M
 * @param[in]  sar_adc_num - SAR0/SAR1.
 * @return     none
 */
static inline void adc_clk_en(adc_num_e sar_adc_num)
{
    if(sar_adc_num == 0){
        reg_clk_en3 |= FLD_CLK3_SARADC_EN;
    }
    else{
        reg_clk_en5 |= FLD_CLK5_SARADC1_EN;
    }
}

/**
 * @brief      This function disable adc source clock: xtal_24M
 * @param[in]  sar_adc_num - SAR0/SAR1.
 * @return     none
 */
static inline void adc_clk_dis(adc_num_e sar_adc_num)
{
    if(sar_adc_num == 0){
        reg_clk_en3 &= (~FLD_CLK3_SARADC_EN);
    }
    else{
        reg_clk_en5 &= (~FLD_CLK5_SARADC1_EN);
    }
}

/**
 * @brief      This function sets adc digital clock and analog clock.
 * @param[in]  sar_adc_num - SAR0/SAR1.
 * @param      div - Analog adc frequency division coefficient.
 * @return     none
 * @attention  The ADC digital clock is driven by a crystal (24MHz) or PLL, while the analog clock is driven by the digital clock.
 */
void adc_set_clk(adc_num_e sar_adc_num,char div)
{
    reg_adc_config1(sar_adc_num) = ((reg_adc_config1(sar_adc_num)  & FLD_ADC_CLK_DIV) | 1);//div=1, adc digital clk = pll or crystal /div.(crystal = 24MHz)
    analog_write_reg8(areg_adc_sample_clk_div(sar_adc_num), div & 0x0f);//adc analog clk = digital clock/(1+div).
}

/**
 * @brief      This function sets ADC input channel as M channel,L channel or R channel.
 * @param[in]  sar_adc_num - SAR0/SAR1.
 * @param[in]  chn - enum variable of ADC sample channel.
 * @return     none
 */
static inline void adc_set_chn_en(adc_num_e sar_adc_num,adc_sample_chn_e chn)
{
    reg_adc_config2(sar_adc_num) |= BIT(chn);
}
/**
 * @brief This function serves to set resolution.
 * @param[in]  sar_adc_num - SAR0/SAR1.
 * @param[in]  res - enum variable of ADC resolution.
 * @return none
 */
static inline void adc_set_resolution(adc_num_e sar_adc_num,adc_res_e res)
{
    analog_write_reg8(areg_adc_res_m(sar_adc_num), (analog_read_reg8(areg_adc_res_m(sar_adc_num) )&(~FLD_ADC_RES_M)) | res);
}
/**
 * @brief      This function serves to set ADC sample time(the number of adc clocks for sample cycles) for M channel,L channel or R channel.
 * @param[in]  sar_adc_num - SAR0/SAR1.
 * @param[in]  chn - enum variable of ADC sample channel.
 * @param[in]  sample_cycle - enum variable of adc sample cycles.
 * @return     none
 */
static void adc_set_tsample_cycle(adc_num_e sar_adc_num,adc_sample_chn_e chn,adc_sample_cycle_e sample_cycle)
{
    switch (chn) {
    case ADC_M_CHANNEL:
            reg_adc_tsamp(sar_adc_num) = ((reg_adc_tsamp(sar_adc_num) & (~FLD_M_TSAMP)) | sample_cycle);
        break;
#if INTERNAL_TEST_FUNC_EN
    case ADC_L_CHANNEL:
            reg_adc_tsamp(sar_adc_num) = ((reg_adc_tsamp(sar_adc_num) & (~FLD_L_TSAMP)) | (sample_cycle<<4));
        break;
    case ADC_R_CHANNEL:
            reg_adc_r_tsamp(sar_adc_num) = ((reg_adc_r_tsamp(sar_adc_num) & (~FLD_R_TSAMP)) | sample_cycle);
        break;
#endif
    default:
        break;
    }
}
/**
 * @brief This function serves to set state and capture_state length.
 * @param[in]   sar_adc_num - SAR0/SAR1.
 * @param[in]   chn - enum variable of ADC sample channel.
 * @param[in]   r_max_mc - Value of length of "capture" state for channel.
 * @param[in]   r_max_s - Value of length of "set" state for channel.
 * @return none
 */
static inline void adc_set_state_length(adc_num_e sar_adc_num,adc_sample_chn_e chn,unsigned short r_max_mc,unsigned char r_max_s)
{
    reg_adc_capture_state(sar_adc_num,chn) = r_max_mc;
    reg_adc_channel_set_state(sar_adc_num,chn) = ((reg_adc_channel_set_state(sar_adc_num,chn)&(~FLD_R_MAX_S)) | r_max_s);
}
/**
 * @brief   This function is used to enable the transmission of data from the adc's M channel, L channel, and R channel to the sar adc rxfifo.
 * @param[in]  sar_adc_num - SAR0/SAR1.
 * @return  none
 * @note    -# In DMA mode, must enable this function.
 *          -# In NDMA mode, if this function is not enabled, the adc code can only be read from the registers, but it will lead to the problem of repeatedly getting the same adc code when calling adc_get_raw_code() several times,
 *             if this function is enabled, the adc code can be read from the rx fifo, and this problem can be avoided, so it is also must enable it.
 */
static inline void adc_all_chn_data_to_fifo_en(adc_num_e sar_adc_num)
{
    analog_write_reg8(areg_adc_data_transfer_control(sar_adc_num) ,analog_read_reg8(areg_adc_data_transfer_control(sar_adc_num) ) & (~FLD_AUTO_NOT_EN));
}

/**
 * @brief   This function is used to disable the transmission of data from the adc's M channel, L channel, and R channel to the sar adc rxfifo.
 * @param[in]  sar_adc_num - SAR0/SAR1.
 * @return  none
 */
static inline void adc_all_chn_data_to_fifo_dis(adc_num_e sar_adc_num)
{
    analog_write_reg8(areg_adc_data_transfer_control(sar_adc_num),FLD_AUTO_NOT_EN);
}

/**
 * @brief       This function is used to set the scan channel cnt.
 * @param[in]   sar_adc_num - SAR0/SAR1.
 * @param[in]   chn_cnt - range:1-3
 * @return      none
 */
static inline void adc_set_scan_chn_cnt(adc_num_e sar_adc_num,unsigned char chn_cnt)
{
    reg_adc_config0(sar_adc_num) = ((reg_adc_config0(sar_adc_num) & (~FLD_SCANT_MAX)) | ((chn_cnt * 2)<< 4));//scan_cnt = chn_cnt*2

}

/**
 * @brief      This function disable adc digital clock.
 * @param[in]  sar_adc_num - SAR0/SAR1.
 * @return     none
 * @note       -# After disabling the digital clock, rxfifo will not be updated.init
 *             -# When you don't need to keep the adc sampling, you can disable the digital clock for power saving purposes.
 */
static inline void adc_dig_clk_dis(adc_num_e sar_adc_num)
{
    reg_adc_config2(sar_adc_num) &= (~FLD_CLK_EN);
}
/**
 * @brief      This function enable adc digital clock.
 * @param[in]  sar_adc_num - SAR0/SAR1.
 * @return     none
 */
static inline void adc_dig_clk_en(adc_num_e sar_adc_num)
{
    reg_adc_config2(sar_adc_num) |= FLD_CLK_EN;

}

/**
 * @brief      This function serves to stop the state machine at the beginning of the M channel and not start sampling.
 * @param[in]  sar_adc_num - SAR0/SAR1.
 * @return     none
 */
static inline void adc_set_scan_chn_dis(adc_num_e sar_adc_num)
{
    /**
     * FLD_SCANT_MAX cannot be configured to 0, because 0 is the RNG channel, if RNG is not enabled (default is not enabled), the state machine will not stop at the RNG channel after setting 0, it will not stop.
     */
    reg_adc_config0(sar_adc_num) = ((reg_adc_config0(sar_adc_num) & (~FLD_SCANT_MAX)) | (1 << 4));
}
/**
 * @brief    This function is used to power on sar_adc.
 * @param[in]  sar_adc_num - SAR0/SAR1.
 * @return   none.
 * @note     -# User need to wait >30us after adc_power_on() for ADC to be stable.
 *           -# If you calling adc_power_off(), because all analog circuits of ADC are turned off after adc_power_off(),
 *            it is necessary to wait >30us after re-adc_power_on() for ADC to be stable.
 */
void adc_power_on(adc_num_e sar_adc_num)
{
    adc_set_scan_chn_dis(sar_adc_num);
    adc_reset(sar_adc_num);
    adc_set_scan_chn_cnt(sar_adc_num, 1);
    analog_write_reg8(areg_adc_pga_ctrl(sar_adc_num), (analog_read_reg8(areg_adc_pga_ctrl(sar_adc_num))&(~FLD_SAR_ADC_POWER_DOWN)));
    adc_dig_clk_en(sar_adc_num);

}
/**
 * @brief      This function is used to power off sar_adc.
 * @param[in]  sar_adc_num - SAR0/SAR1.
 * @return     none
 */
void adc_power_off(adc_num_e sar_adc_num)
{
    adc_dig_clk_dis(sar_adc_num);
    analog_write_reg8(areg_adc_pga_ctrl(sar_adc_num), (analog_read_reg8(areg_adc_pga_ctrl(sar_adc_num))|FLD_SAR_ADC_POWER_DOWN));

}
/**
 * @brief This function is used to set IO port for ADC supply or ADC IO port voltage sampling.
 * @param[in]  pin - gpio_pin_e ADC input gpio pin
 * @return none
 */
void adc_pin_config(gpio_pin_e pin)
{
    unsigned short adc_input_pin = pin & 0xfff;
    gpio_function_en(adc_input_pin);
    gpio_input_dis(adc_input_pin);
    gpio_output_dis(adc_input_pin);
    gpio_set_low_level(adc_input_pin);
    gpio_set_up_down_res(adc_input_pin,GPIO_PIN_UP_DOWN_FLOAT);

}

/**
 * @brief This function serves to set the reference voltage of the channel.
 * @param[in]  sar_adc_num - SAR0/SAR1.
 * @param[in]  chn - enum variable of ADC sample channel.
 * @return none
 * @note       1. adc_set_ref_voltage does not take effect immediately after configuration, it needs to be delayed 100us after calling adc_dig_clk_en().
 *             2. If you call adc_set_ref_voltage() alone, please change the value of g_adc_vref of the corresponding channel, otherwise the voltage conversion will be wrong.
 *
 */
static void adc_set_ref_voltage(adc_num_e sar_adc_num,adc_sample_chn_e chn)
{
    unsigned int flag = 0;
    unsigned int ret = 0;
    reg_adc_channel_set_state(sar_adc_num,chn) = (reg_adc_channel_set_state(sar_adc_num,chn)&(~FLD_SEL_VREF)) | (ADC_VREF_1P2V<<6);
    //Vref buffer bias current trimming:        150%
    //Comparator preamp bias current trimming:  100%
    //if flag all bits are 0, use high drive capability.
    /*Background: ATE calibrated 8K chips with configuration 0x1d, which resulted in relatively high power consumption.
     * At that time, power consumption requirements were not very sensitive. Later, to reduce power consumption, the chips were produced using configuration 0x15. The error performance of configuration 0x15 on the original 8K chips was 50mV.
     * To balance the ADC performance of the old chips, the following compatible logic was used.*/
    ret = efuse_get_sar_drv_flag(&flag);
    if (!flag || ret) { // if read efuse fail unlikely, also use high drive capability .
        analog_write_reg8(areg_ain_scale(sar_adc_num), (analog_read_reg8(areg_ain_scale(sar_adc_num)  )&(0xC0)) | 0x1d );
    } else {
        analog_write_reg8(areg_ain_scale(sar_adc_num), (analog_read_reg8(areg_ain_scale(sar_adc_num)  )&(0xC0)) | 0x15 );

    }
}
/**
 * @brief This function serves to set the sample frequency.
 * @param[in]  sar_adc_num - SAR0/SAR1.
 * @param[in]  chn - enum variable of ADC sample channel.
 * @param[in]  sample_freq - enum variable of ADC sample frequency.
 * @return none
 */
static void adc_set_sample_rate(adc_num_e sar_adc_num,adc_sample_chn_e chn,adc_sample_freq_e sample_freq)
{
    unsigned char adc_set = sample_freq & 0xf;
    unsigned char adc_sample_cycle = (sample_freq>>4) & 0xf;
    unsigned short adc_capture = (sample_freq>>8) & 0xffff;
    adc_set_state_length(sar_adc_num,chn,adc_capture, adc_set);
    adc_set_tsample_cycle(sar_adc_num,chn,adc_sample_cycle);
}
/**
 * @brief This function serves to set pre_scaling factor.
 * @param[in]  sar_adc_num - SAR0/SAR1.
 * @param[in]  chn - enum variable of ADC sample channel.
 * @param[in]  pre_scale - enum variable of ADC pre_scaling factor.
 * @return none
 */
static inline void adc_set_scale_factor(adc_num_e sar_adc_num,adc_sample_chn_e chn,adc_pre_scale_e pre_scale)
{
    reg_adc_channel_set_state(sar_adc_num,chn) = ((reg_adc_channel_set_state(sar_adc_num,chn)&(~FLD_SEL_AI_SCALE)) | (pre_scale<<4));
    g_adc_pre_scale[sar_adc_num][chn] = 1<<pre_scale;
}
/**
 * @brief      This function serves to set Vbat voltage division factor.
 * @param[in]  sar_adc_num - SAR0/SAR1.
 * @param[in]  chn - enum variable of ADC sample channel
 * @return     none
 * @note       adc_set_vbat_divider() does not take effect immediately after configuration, it needs to be delayed 100us after calling adc_dig_clk_en().
 */
void adc_set_vbat_divider(adc_num_e sar_adc_num,adc_sample_chn_e chn)
{
    unsigned char offset = (chn == 0) ? 0 : (1 << chn);
    reg_adc_vabt_div(sar_adc_num) = (reg_adc_vabt_div(sar_adc_num) & (~BIT_RNG(offset,offset+1)))| ((ADC_VBAT_DIV_1F4) << offset);
}
#if INTERNAL_TEST_FUNC_EN
/**
 * @brief       This function is used to enable the status of the valid adc code for the m channel.
 * @param[in]  sar_adc_num - SAR0/SAR1.
 * @return      none
 * @attention   The adc_ana_read_en() API must be called before the adc_get_m_chn_valid_status()API.
 * @note        This function is used in NDMA mode where adc_get_m_chn_valid_status() needs to be called.
 */
static inline void adc_ana_read_en(adc_num_e sar_adc_num)
{
    analog_write_reg8(areg_adc_data_sample_control(sar_adc_num),  analog_read_reg8(areg_adc_data_sample_control(sar_adc_num)) | FLD_ANA_RD_EN);
}

/**
 * @brief       This function is used to get the status of the valid adc code for the m channel.
 * @param[in]   sar_adc_num - SAR0/SAR1.
 * @return      0:Before sampling one adc_code at each time,m_chn_valid_status is automatically set to 0.
 *              !0:After sampling one adc_code at each time,m_chn_valid_status is automatically set to 1.
 * @attention   Must call adc_ana_read_en()API first to get the status of the valid adc code for the m channel.
 */
static inline unsigned char adc_get_m_chn_valid_status(adc_num_e sar_adc_num)
{
    return (analog_read_reg8(areg_m_chn_data_valid_status(sar_adc_num)) & FLD_M_CHN_DATA_VALID_STATUS);
}

/**
 * @brief      This function open temperature sensor power.
 * @return     none
 */
static inline void adc_temp_sensor_power_on(void)
{
    analog_write_reg8(areg_temp_sensor_ctrl, (analog_read_reg8(areg_temp_sensor_ctrl) & (~FLD_TEMP_SENSOR_POWER_DOWN)));
}

/**
 * @brief      This function close temperature sensor power.
 * @return     none
 */
static inline void adc_temp_sensor_power_off(void)
{
    analog_write_reg8(areg_temp_sensor_ctrl, (analog_read_reg8(areg_temp_sensor_ctrl) | FLD_TEMP_SENSOR_POWER_DOWN));
}

/**
 * @brief This function serves to calculate temperature from temperature sensor adc sample code.
 * @param[in]   adc_code            - the temperature sensor adc sample code.
 * @return      adc_temp_value      - the temperature value.
 * @attention   Temperature and adc_code are linearly related. We test four chips between -40~130 (Celsius) and got an average relationship:
 *          Temp =  564 - ((adc_code * 819)>>11),when Vref = 1.2V, pre_scale = 1.
 */
short adc_calculate_temperature(short adc_code)
{
    //////////////// adc sample data convert to temperature(Celsius) ////////////////
    //adc_temp_value = 564 - ((adc_code * 819)>>11)
    return 564 - ((adc_code * 819) >> 11);
}
/**
 * @brief      This function disable oversample mode.
 * @param[in]  sar_adc_num - SAR0/SAR1.
 * @return     none
 * @note       Hardware averaging mode
 */
inline void adc_oversample_dis(adc_num_e sar_adc_num)
{
    reg_adc_oversample(sar_adc_num) &= (~FLD_OVERSAMPLE_ENABLE);
}

/**
 * @brief      This function enable oversample mode.
 * @param[in]  sar_adc_num - SAR0/SAR1.
 * @return     none
 * @note       Hardware averaging mode
 */
inline void adc_oversample_en(adc_num_e sar_adc_num)
{
    reg_adc_oversample(sar_adc_num) |= FLD_OVERSAMPLE_ENABLE;
}

/**
 * @brief      This function serves to set multiple acquisition data averaging
 * @param[in]  sar_adc_num - SAR0/SAR1.
 * @param[in]  cnt - Set cnt data to be averaged once.
 * @return     none
 */
void adc_set_oversample_cnt(adc_num_e sar_adc_num,char cnt)
{
    reg_adc_oversample(sar_adc_num) = ((reg_adc_oversample(sar_adc_num) & (~FLD_MAX_OVERSAMPLE_CNT)) | cnt);
}

/**
 * @brief      This function serves to configure the PEM task..
 * @param[in]  sar_adc_num - SAR0/SAR1.
 * @param[in]  chn - to select the PEM channel.
 * @return     none.
 */
void adc_set_pem_task(adc_num_e sar_adc_num,pem_chn_e chn)
{
    reg_pem_enable(sar_adc_num) |= FLD_PEM_TASK_ENABLE;
    adc_pem_task_config.module = PEM_TASK_SAR_ADC;
    adc_pem_task_config.sig_sel = 0;         //only single adc_trig task
    pem_task_config(chn,adc_pem_task_config);
}

/**
 * @brief      This function serves to configure the PEM event..
 * @param[in]  sar_adc_num - SAR0/SAR1.
 * @param[in]  chn - to select the PEM channel.
 * @param[in]  event_signal - to select the event signal.
 * @return     none.
 */
void adc_set_pem_event(adc_num_e sar_adc_num,pem_chn_e chn,unsigned char adc_sel)
{
    reg_pem_enable(sar_adc_num) |= FLD_PEM_EVENT1_ENABLE;
    adc_pem_event_config.module = PEM_EVENT_SAR_ADC;
    adc_pem_event_config.sig_sel = adc_sel;
    pem_event_config(chn, adc_pem_event_config);
}

#endif

/**
 * @brief This function is used to initialize the ADC.
 * @param[in]  sar_adc_num - SAR0/SAR1.
 * @param[in]  channel_cnt - transfer_mode and the number of channels used.
 * @return none
 * @attention Many features are configured in adc_init function. But some features
 *      such as adc digital clk, adc analog clk, resolution, we think better to set as default value,
 *      and user don't need to change them in most use cases.
 */
void adc_init(adc_num_e sar_adc_num,adc_chn_cnt_e channel_cnt)
{
    adc_power_off(sar_adc_num);//power off sar adc
    adc_reset(sar_adc_num);
    adc_clk_en(sar_adc_num);
    adc_set_resolution(sar_adc_num,ADC_RES12);//default adc_resolution set as 12bit ,BIT(11) is sign bit
    //Enabling dwa affects adc performance.(confirmed by liupeng)
    //adc_data_weighted_average_en();
    if (NDMA_M_CHN == channel_cnt) {
        adc_all_chn_data_to_fifo_dis(sar_adc_num);
        reg_adc_config2(sar_adc_num) &= ~FLD_RX_DMA_ENABLE;//In NDMA mode,RX DMA needs to be disabled.
        reg_adc_config2(sar_adc_num) |= FLD_SAR_RX_INTERRUPT_ENABLE;//SAR_RX_INTERRUPT must be enabled to call adc_get_irq_status() to get adc irq status.
    }
    /**
     * The set and capture of RNG channel are configured to 0 by default, and the actual state machine scanning time of RNG channel is the maximum time(about 25us),
     * and by configuring both of them to 1 (the minimum scanning time), the state machine scanning time of RNG channel is only (1+1)/24M=83ns,
     * which enables the state machine to enter into the set state of the M channel faster, and prevents the first code of M channel abnormality.
     */
    reg_adc_rng_set_state(sar_adc_num)     = 0x01;
    reg_adc_rng_capture_state(sar_adc_num) = 0x01;
    g_channel_cnt             = channel_cnt;
}

/**
 * @brief This function is used to configure the channel of the ADC.
 * @param[in]  sar_adc_num - SAR0/SAR1.
 * @param[in]  chn -the channel to be configured.
 * @param[in]  adc_cfg -structure pointer for configuring ADC channel.
 * @attention  When using a 2M or 1M sampling rate, the ADC analog clock must be 48M, and the PLL clock must be a multiple of 48M.
 * @return none
 */
void adc_chn_config(adc_num_e sar_adc_num, adc_sample_chn_e chn, adc_chn_cfg_t *adc_cfg)
{
    adc_set_diff_input(sar_adc_num,chn, adc_cfg->input_p, adc_cfg->input_n);
    adc_set_vbat_divider(sar_adc_num,chn);
    adc_set_ref_voltage(sar_adc_num,chn);
    adc_set_scale_factor(sar_adc_num, chn, adc_cfg->pre_scale);
    adc_set_sample_rate(sar_adc_num, chn, adc_cfg->sample_freq);
    adc_set_chn_en(sar_adc_num, chn);
    if(adc_cfg->sample_freq !=  ADC_SAMPLE_FREQ_2M && adc_cfg->sample_freq !=  ADC_SAMPLE_FREQ_1M ){
        adc_select_clk(sar_adc_num,FLD_CLOCK_XTL,0x01);
        adc_set_clk(sar_adc_num,5);//set adc digital clk to 24MHz and adc analog clk to 4MHz
    }
    else{
        adc_select_clk(sar_adc_num,FLD_CLOCK_PLL,sys_clk.pll_clk/48);
        adc_set_clk(sar_adc_num,0);//set adc digital clk to 48MHz and adc analog clk to 48MHz
    }

}
/**
 * @brief This function is used to initialize the ADC for sampling.
 * @param[in]  sar_adc_num - SAR0/SAR1.
 * @param[in]  mode - ADC_GPIO_MODE/ADC_VBAT_MODE/ADC_TEMP_MODE.
 * @param[in]  chn -the channel to be configured.
 * @param[in]  cfg -structure pointer for configuring ADC channel.
 * @return none
 */
void adc_channel_sample_init(adc_num_e sar_adc_num,adc_mode_e mode,adc_sample_chn_e chn , adc_chn_cfg_t *cfg)
{   
    if(mode == ADC_GPIO_MODE){
        adc_pin_config(cfg->input_p & 0xfff);
    }else if(mode == ADC_VBAT_MODE){
        cfg->input_n = ADC_GND_N;
        cfg->input_p = ADC_VBAT_P;
        cfg->pre_scale = ADC_PRESCALE_1;
    }
#if INTERNAL_TEST_FUNC_EN
    else if(mode == ADC_TEMP_MODE){
        cfg->input_n = ADC_GND_N;
        cfg->input_p = ADC_TEMPSENSORN_P;
        cfg->pre_scale   = ADC_PRESCALE_1;
        cfg->sample_freq = ADC_SAMPLE_FREQ_96K;
    }
#endif

    if (cfg->input_n != ADC_GND_N) {
        adc_pin_config(cfg->input_n & 0xfff);
    }

    adc_chn_config(sar_adc_num, chn, cfg);
#if INTERNAL_TEST_FUNC_EN
    if(mode == ADC_TEMP_MODE){
        adc_temp_sensor_power_on();
    }   
#endif

    //The Vref voltage switch should be pre-established. Once it is enabled, ensure that Vref is stabilized before the PD is turned off, in order to avoid fluctuations during the initial few code cycles.
    analog_write_reg8(areg_adc_vref_fast_startup_sampled_inuput(sar_adc_num),FLD_VREF_FAST_STARTUP|FLD_SAMPLED_INPUT_MODEBAR);

    /**
     * At present, the reference voltage is 1.2V by default, and the calibration is also based on 1.2V. If other chips have different gears in the future,
     * it is necessary to add a judgment here: only when the corresponding gears of the corresponding calibration conditions are selected,
     * the following calibration code can be invoked
     */
    if(sar_adc_num == ADC0 && mode == ADC_GPIO_MODE){
        g_adc_vref[sar_adc_num][chn]        = g_adc_sar0_gpio_calib_vref;        
        g_adc_vref_offset[sar_adc_num][chn] = g_adc_sar0_gpio_calib_vref_offset; 
    }else if(sar_adc_num == ADC0 && mode == ADC_VBAT_MODE){
        g_adc_vref[sar_adc_num][chn]        = g_adc_sar0_vbat_calib_vref;        
        g_adc_vref_offset[sar_adc_num][chn] = g_adc_sar0_vbat_calib_vref_offset; 
    } else if(sar_adc_num == ADC1 && mode == ADC_GPIO_MODE) {
        g_adc_vref[sar_adc_num][chn]        = g_adc_sar1_gpio_calib_vref;        
        g_adc_vref_offset[sar_adc_num][chn] = g_adc_sar1_gpio_calib_vref_offset; 
    }
}

/**
 * @brief This function is used to initialize the ADC for keyscan sampling.
 * @param[in]  sar_adc_num - SAR0/SAR1.
 * @note  The sampling rate for keyscan must be set to 2M.
 *        The PLL clock must be a multiple of 48M.    
 * @return none
 */
void adc_keyscan_sample_init(adc_num_e sar_adc_num)
{
    adc_chn_cfg_t chn_cfg =
    {
        .pre_scale = ADC_PRESCALE_1F4,
        .sample_freq = ADC_SAMPLE_FREQ_2M,  //When using a 2M sampling rate, the ADC analog clock must be 48M, and the PLL clock must be a multiple of 48.
        .input_p = ADC0_GPIO_PC0P,
        .input_n = ADC0_GPIO_PC0N,
    };

    adc_chn_config(sar_adc_num, ADC_M_CHANNEL, &chn_cfg);
    adc_set_trigger_cnt(sar_adc_num, 1);
    adc_trigger_en(sar_adc_num);
    adc_keyscan_auto_en(sar_adc_num);

    if(sar_adc_num == ADC0){
        adc_pin_config(GPIO_PC0);
        adc_pin_config(GPIO_PC1);
        adc_pin_config(GPIO_PC2);
        adc_pin_config(GPIO_PC3);
        adc_pin_config(GPIO_PC4);
        adc_pin_config(GPIO_PC5);
        adc_pin_config(GPIO_PC6);
        adc_pin_config(GPIO_PC7);

        reg_pad_auto_p(ADC0) = 0x87654321;
        reg_pad_auto_n(ADC0) = 0xbbbbbbbb;//GND
    }else{
        adc_pin_config(GPIO_PB0);
        adc_pin_config(GPIO_PB1);
        adc_pin_config(GPIO_PB2);
        adc_pin_config(GPIO_PB3);

        reg_pad_auto_p(ADC0) = 0x43214321;
        reg_pad_auto_n(ADC0) = 0xbbbbbbbb;//GND

        reg_pad_auto_p(ADC1) = 0x43214321;
        reg_pad_auto_n(ADC1) = 0xbbbbbbbb;//GND
    }
    analog_write_reg8(areg_adc_vref_fast_startup_sampled_inuput(sar_adc_num),FLD_VREF_FAST_STARTUP|FLD_SAMPLED_INPUT_MODEBAR); //Quickly start the ADC
}
/**
 * @brief This function serves to calculate voltage from adc sample code.
 * @param[in]   sar_adc_num - SAR0/SAR1.
 * @param[in]   mode - ADC sample mode
 * @param[in]   chn - enum variable of ADC sample channel.
 * @param[in]   adc_code    - the adc sample code.
 * @return      adc_vol_mv  - the average value of adc voltage value.
 */
short adc_calculate_voltage(adc_num_e sar_adc_num,adc_mode_e mode,adc_sample_chn_e chn,short adc_code)
{
    /**
     *  adc sample code convert to voltage(mv):
     *  (adc code BIT<10~0> is valid data)
     *  adc_voltage  =  (adc_code * Vref * adc_pre_scale / 0x800) + offset
     *               =  (adc_code * Vref * adc_pre_scale >>11) + offset
     */
    if(mode == ADC_VBAT_MODE){
        return (((adc_code * 4 * g_adc_pre_scale[sar_adc_num][chn] * g_adc_vref[sar_adc_num][chn]) >> 11) + g_adc_vref_offset[sar_adc_num][chn]); // divider = 4
    }else if(mode == ADC_GPIO_MODE){
        return (((adc_code * g_adc_pre_scale[sar_adc_num][chn] * g_adc_vref[sar_adc_num][chn]) >> 11) + g_adc_vref_offset[sar_adc_num][chn]);
    }
#if INTERNAL_TEST_FUNC_EN
    else if (mode == ADC_TEMP_MODE)
    {
        return 564 - ((adc_code * 819) >> 11);
    }
#endif
    return 0;
}

/**
 * @brief This function is used to calib ADC 1.2V vref for sar0 GPIO.
 * @param[in] vref - GPIO sampling calibration value.
 * @param[in] offset - GPIO sampling two-point calibration value offset.
 * @return none
 */
void adc_set_sar0_gpio_calib_vref(unsigned short vref, signed char offset)
{
    g_adc_sar0_gpio_calib_vref        = vref;
    g_adc_sar0_gpio_calib_vref_offset = offset;
}

/**
 * @brief This function is used to calib ADC 1.2V vref for Vbat.
 * @param[in] vref - Vbat channel sampling calibration value.
 * @param[in] offset - Vbat channel sampling two-point calibration value offset.
 * @return none
 */
void adc_set_sar0_vbat_calib_vref(unsigned short vref, signed char offset)
{
    g_adc_sar0_vbat_calib_vref        = vref;
    g_adc_sar0_vbat_calib_vref_offset = offset;
}
/**
 * @brief This function is used to calib ADC 1.2V vref for sar1 GPIO.
 * @param[in] vref - GPIO sampling calibration value.
 * @param[in] offset - GPIO sampling two-point calibration value offset.
 * @return none
 */
void adc_set_sar1_gpio_calib_vref(unsigned short vref, signed char offset)
{
    g_adc_sar1_gpio_calib_vref        = vref;
    g_adc_sar1_gpio_calib_vref_offset = offset;
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
void adc_set_dma_config(adc_num_e sar_adc_num,dma_chn_e chn)
{
    adc_dma_chn     = chn;
    reg_adc_config2(sar_adc_num) = FLD_RX_DMA_ENABLE;
    if(sar_adc_num == 0){
        dma_config(chn, &adc_rx_dma_config);
    }
    else{
        dma_config(chn, &adc1_rx_dma_config);
    }

    reg_dma_llp(adc_dma_chn) = 0;
    /*
     * Configuration differs from TL7518 for the following reasons:
     * The TL7518's RX FIFO is stored in WORD units.
     * The TL322X's RX FIFO is stored in HALF WORD units.
     *    it will be transferred away by the DMA by WORD, resulting in data misalignment when using multiple channels.
     */
    adc_set_rx_fifo_trig_cnt(sar_adc_num,1);//Default value is 1, users should not change.
    adc_clr_rx_fifo_cnt(sar_adc_num); //If the fifo is not cleared, there may be residual values in the fifo that affect the sampling results.
    adc_all_chn_data_to_fifo_en(sar_adc_num);
}

/**
 * @brief     The adc starts sampling in DMA mode.
 * @param[in] sar_adc_num - SAR0/SAR1.
 * @param[in] adc_data_buf  - Pointer to data buffer, it must be 4-bytes aligned address
 *                            and the actual buffer size defined by the user needs to be not smaller than the data_byte_len,
 *                            otherwise there may be an out-of-bounds problem.
 * @param[in] data_byte_len - Amount of data to be sampled in bytes, the maximum value can be up to 0xFFFFFC.
 * @note      The depth of SAR_ADC_FIFO is 8 words, and its characteristic is store first, fetch first.
 *            When all three channels are working at the same time, the order of storing adc channel data into SAR_ADC_FIFO is M: L: R,
 *            so the arrangement after dma handling is also M: L: R.
 * @return    none
 */
void adc_start_sample_dma(adc_num_e sar_adc_num,short *adc_data_buf,unsigned int data_byte_len)
{
    dma_set_address(adc_dma_chn,SAR_ADC_FIFO(sar_adc_num),(unsigned int)adc_data_buf);
    dma_set_size(adc_dma_chn, data_byte_len, DMA_WORD_WIDTH);
    /*
     * dma_chn_en() must be in front of adc_set_scan_chn_cnt() to prevent mis-ordering of multi-channel sampling data when using multiple channels.
     */
    dma_chn_en(adc_dma_chn);
    adc_set_scan_chn_cnt(sar_adc_num,g_channel_cnt & 0x0f);
    adc_dig_clk_en(sar_adc_num);
}

/**
 * @brief     This function serves to get adc DMA irq status.
 * @return    0: the sample is in progress.
 *            !0: the sample is finished.
 * @note      The code is placed in the ram code section, in order to shorten the time.
 */
_attribute_ram_code_sec_noinline_ unsigned char adc_get_irq_status_dma(void)
{
    return (dma_get_tc_irq_status(1 << adc_dma_chn));
}

/**
 * @brief     This function serves to clear adc DMA irq status.
 * @param[in] sar_adc_num - SAR0/SAR1.
 * @return    none
 * @note      The code is placed in the ram code section, in order to shorten the time.       
 */
_attribute_ram_code_sec_noinline_ void adc_clr_irq_status_dma(adc_num_e sar_adc_num)
{
    /*
     * adc_set_scan_chn_dis() must be called when DMA is finished to stop the state machine at the beginning of the M channel to prevent mis-ordering of multi-channel sampling data when using multiple channels.
     */
    adc_set_scan_chn_dis(sar_adc_num);
    dma_chn_dis(adc_dma_chn);
    dma_clr_tc_irq_status(1 << adc_dma_chn);
}

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
short adc_get_raw_code(adc_num_e sar_adc_num)
{
    short adc_code = 0;
    /**
     * Change the way to get adc code from read areg_adc_misc to read rx fifo, to solve the problem that the valid status is 1 for a long time
     * due to the slow change of valid status in adc_get_m_chn_valid_status(), which leads to fetching the same code repeatedly.(Confirmed by qiangkai.xu, added by xiaobin.huang at 20240903)
     */
    if(!g_adc_rx_fifo_index[sar_adc_num])
    {
        adc_code = (short)reg_adc_rxfifo_dat(sar_adc_num,g_adc_rx_fifo_index[sar_adc_num]);
        g_adc_rx_fifo_index[sar_adc_num] = 1;
    } else {
        adc_code = (short)reg_adc_rxfifo_dat(sar_adc_num,g_adc_rx_fifo_index[sar_adc_num]);
        g_adc_rx_fifo_index[sar_adc_num] = 0;
    }
    return adc_code;
}

/**
 * @brief   This function is used to enable the transmission of data from the adc's M channel, L channel, and R channel to the sar adc rxfifo.
 * @param[in]  sar_adc_num - SAR0/SAR1.
 * @return  none
 */
void adc_start_sample_nodma(adc_num_e sar_adc_num)
{
    adc_clr_rx_fifo_cnt(sar_adc_num); //If the fifo is not cleared, there may be residual values in the fifo that affect the sampling results.
    adc_all_chn_data_to_fifo_en(sar_adc_num);
    adc_set_scan_chn_cnt(sar_adc_num,1);
}

/**
 * @brief   This function is used to disable the transmission of data from the adc's M channel, L channel, and R channel to the sar adc rxfifo and clear rx fifo cnt and g_adc_rx_fifo_index.
 * @param[in]  sar_adc_num - SAR0/SAR1.
 * @return  none
 */
void adc_stop_sample_nodma(adc_num_e sar_adc_num)
{
    adc_all_chn_data_to_fifo_dis(sar_adc_num);
    adc_clr_rx_fifo_cnt(sar_adc_num);
}

/**
 * @brief      This function serves to sets the number of data collected by the trigger at a time.
 * @param[in]  sar_adc_num - SAR0/SAR1.
 * @param[in]  cnt - Set trigger to collect cnt data at a time
 * @return     none
 */
void adc_set_trigger_cnt(adc_num_e sar_adc_num,char cnt)
{
    reg_sample_times(sar_adc_num) =  cnt;
}

/**
 * @brief      This function disable trigger mode.
 * @param[in]  sar_adc_num - SAR0/SAR1.
 * @return     none
 * @note       Manual trigger required
 */
void adc_trigger_dis(adc_num_e sar_adc_num)
{
    reg_adc_config2(sar_adc_num) &= (~FLD_TRIG_MODE);
}

/**
 * @brief      This function enable trigger mode.
 * @param[in]  sar_adc_num - SAR0/SAR1.
 * @return     none
 * @note       Manual trigger required
 */
void adc_trigger_en(adc_num_e sar_adc_num)
{
    reg_adc_config2(sar_adc_num) |= FLD_TRIG_MODE;
}

