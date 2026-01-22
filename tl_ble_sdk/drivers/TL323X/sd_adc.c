/********************************************************************************************************
 * @file    sd_adc.c
 *
 * @brief   This is the source file for tl323x
 *
 * @author  Driver Group
 * @date    2025
 *
 * @par     Copyright (c) 2025, Telink Semiconductor (Shanghai) Co., Ltd. ("TELINK")
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
#include "sd_adc.h"
#include "compiler.h"
#include <stdio.h>
#include "lpc.h"
volatile unsigned char g_sd_adc_divider=1;
volatile unsigned short g_sd_adc_downsample_rate=128;

/*The following calibration defaults are provided by ATE, and both gpio and vbat sampling are calibrated to within 50mv of errors. */
_attribute_data_retention_sec_ volatile unsigned short g_sd_adc_vref = 10000;
_attribute_data_retention_sec_ volatile signed short g_sd_adc_vref_offset = 0;

_attribute_data_retention_sec_ volatile unsigned short g_single_sd_adc_vref = 9073;
_attribute_data_retention_sec_ volatile signed short g_single_sd_adc_vref_offset = -125;

_attribute_data_retention_sec_ volatile unsigned short g_sd_adc_vbat_calib_vref = 9065;
_attribute_data_retention_sec_ volatile signed short g_sd_adc_vbat_calib_vref_offset = -249;

_attribute_data_retention_sec_ volatile unsigned short g_diff_sd_adc_vref = 9073;
_attribute_data_retention_sec_ volatile signed short g_diff_sd_adc_vref_offset = -125;

_attribute_data_retention_sec_ volatile double vbat_gain_a = 1;
_attribute_data_retention_sec_ volatile double vbat_gain_b = 1;
_attribute_data_retention_sec_ volatile double vbat_offset_c = 0;

extern unsigned char vbat_2v2_calib_flag;

dma_chn_e sd_adc_dma_chn;
dma_config_t sd_adc_rx_dma_config=
{
    .dst_req_sel= 0,
    .src_req_sel=DMA_REQ_AUDIO2_RX,
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
 * @brief This function is used to calibrate sd adc sample voltage for single GPIO.
 * @param[in] vref - GPIO sampling calibration value.
 * @param[in] offset - GPIO sampling two-point calibration value offset.
 * @return none
 */
void sd_adc_set_single_gpio_calib_vref(unsigned short vref, signed short offset)
{
    g_single_sd_adc_vref = vref;
    g_single_sd_adc_vref_offset = offset;
}

/**
 * @brief This function is used to calibrate sd adc sample voltage for VABT.
 * @param[in] vref - GPIO sampling calibration value.
 * @param[in] offset - GPIO sampling two-point calibration value offset.
 * @return none
 */
void sd_adc_set_vbat_calib_vref(unsigned short vref, signed short offset)
{
    g_sd_adc_vbat_calib_vref        = vref;
    g_sd_adc_vbat_calib_vref_offset = offset;
}

/**
 * @brief This function is used to calibrate sd adc sample voltage for GPIO differential.
 * @param[in] vref - GPIO sampling calibration value.
 * @param[in] offset - GPIO sampling two-point calibration value offset.
 * @return none
 */
void sd_adc_set_diff_gpio_calib_vref(unsigned short vref, signed short offset)
{
    g_diff_sd_adc_vref = vref;
    g_diff_sd_adc_vref_offset = offset;
}

/**
 * @brief This function is used to calibrate sd adc sample voltage for VBAT < 2.2V.
 * @param[in] gain_a - VBAT sampling calibration coefficient.
 * @param[in] gain_b - VBAT sampling calibration coefficient.
 * @param[in] offset_c - VBAT sampling calibration offset.
 * @return none
 */
void sd_adc_set_vbat_2v2_calib_vref(signed int gain_a, signed int gain_b, signed int offset_c)
{
    vbat_gain_a = (double)gain_a/10000;
    vbat_gain_b = (double)gain_b/10000;
    vbat_offset_c = (double)offset_c/10000;
}

/**
 * @brief     This function servers to set the sample mode.
 * @param[in] mode -sd_adc_mux_mode_e
 * @return    none
 */
void sd_adc_set_mux_control(sd_adc_mux_mode_e mode)
{
    switch (mode) {
    case SD_ADC_GPIO_MODE:
        analog_write_reg8(areg_sel_ana_input_div,(analog_read_reg8(areg_sel_ana_input_div) &(~FLD_L_SEL_DIVOUT_P))|(FLD_DIVOUT_P_GPIO<<6));
        analog_write_reg8(areg_0x10c, analog_read_reg8(areg_0x10c) & (~FLD_EN_VBAT));
        break;
    case SD_ADC_VBAT_MODE:
        analog_write_reg8(areg_sel_ana_input_div,(analog_read_reg8(areg_sel_ana_input_div) &(~FLD_L_SEL_DIVOUT_P))|(FLD_DIVOUT_P_VBAT<<6));
        analog_write_reg8(areg_0x10c, analog_read_reg8(areg_0x10c) | FLD_EN_VBAT);
        break;
    case SD_ADC_TEMP_MODE:
        analog_write_reg8(areg_sel_ana_input_div,(analog_read_reg8(areg_sel_ana_input_div) &(~FLD_L_SEL_DIVOUT_P))|(FLD_DIVOUT_P_TEMP<<6));
        analog_write_reg8(areg_0x10c, analog_read_reg8(areg_0x10c) & (~FLD_EN_VBAT));
        break;
    default:
        break;
    }
}

/**
 * @brief      This function serves to set adc vbg reference voltage.
 * @param[in]  en - SD_ADC_VBG_POWER_DOWN or SD_ADC_VBG_POWER_ON.
 * @return     none.
 */
void sd_adc_set_vbg(sd_adc_vmid_power_switch_e en)
{
    if (en)
    {
        /***enable vbg reference voltage***/
        analog_write_reg8(areg_0x10f, (analog_read_reg8(areg_0x10f) | FLD_AUDIO_PD_VMID));
    }
    else
    {
        /***disable vbg reference voltage***/
        analog_write_reg8(areg_0x10f, (analog_read_reg8(areg_0x10f) & (~FLD_AUDIO_PD_VMID)));
    }
}

/**
 * @brief     This function servers to power on SD_ADC.
 * @param[in] mode -sd_adc_mode_e
 * @return    none
 * @note      -# After each call to sd_adc_power_on(SD_ADC_SAMPLE_MODE), the first 4 codes sampled by sd_adc may be abnormal and need to be discarded.
 */
void sd_adc_power_on(sd_adc_mode_e mode)
{
    (void)mode;
    sd_adc_set_vbg(SD_ADC_VBG_POWER_ON);
    BM_SET(reg_clk_en4, FLD_CLK4_DC_EN);//dc clk signal enable
    analog_write_reg8(areg_0x10e, analog_read_reg8(areg_0x10e) & (~FLD_L_PD_BUFFER));//power on two sd_adc buffer at the positive and negative.
    analog_write_reg8(areg_0x10f, (analog_read_reg8(areg_0x10f) & (~(FLD_AUDIO_PD_BIAS|FLD_AUDIO_PD_ADC))));

}

/**
 * @brief     This function servers to power off SD_ADC.
 * @param[in] mode -sd_adc_mode_e
 * @return    none
 */
void sd_adc_power_off(sd_adc_mode_e mode)
{
    (void)mode;
    sd_adc_set_vbg(SD_ADC_VBG_POWER_DOWN);
    BM_CLR(reg_clk_en4, FLD_CLK4_DC_EN);//dc clk signal disable
    analog_write_reg8(areg_0x10e, analog_read_reg8(areg_0x10e) | FLD_L_PD_BUFFER);//power down two sd_adc buffer at the positive and negative.
    analog_write_reg8(areg_0x10f, (analog_read_reg8(areg_0x10f) | (FLD_AUDIO_PD_BIAS|FLD_AUDIO_PD_ADC)));
}

/**
 * @brief      This function is used to initialize the positive and negative channels for gpio and temp sensor sampling.
 * @param[in]  chn   - sd_adc_dc_chn_e
 * @param[in]  p_pin - sd_adc_p_input_pin_def_e
 * @param[in]  n_pin - sd_adc_n_input_pin_def_e
 * @return     none
 */
void sd_adc_gpio_pin_init(sd_adc_dc_chn_e  chn, sd_adc_p_input_pin_def_e p_pin,sd_adc_n_input_pin_def_e n_pin)
{
    unsigned short p_pin_gpio = p_pin & 0xfff;
    unsigned short n_pin_gpio = n_pin & 0xfff;

    //SD_ADC GPIO Init
    gpio_input_dis(p_pin_gpio);
    gpio_output_dis(p_pin_gpio);

    //PB5-7,PD0-1 enable buffer
    if(p_pin_gpio == GPIO_PB5){
        analog_write_reg8(areg_0x10f, analog_read_reg8(areg_0x10f)&(~BIT(0)));
    }else if (p_pin_gpio == GPIO_PB6){
        analog_write_reg8(areg_0x10f, analog_read_reg8(areg_0x10f)&(~BIT(1)));
    }else if(p_pin_gpio == GPIO_PB7){
        analog_write_reg8(areg_0x10f, analog_read_reg8(areg_0x10f)&(~BIT(2)));
    }else if(p_pin_gpio == GPIO_PD0){
        analog_write_reg8(areg_0x10f, analog_read_reg8(areg_0x10f)&(~BIT(3)));
    }else if(p_pin_gpio == GPIO_PD1){
        analog_write_reg8(areg_0x10f, analog_read_reg8(areg_0x10f)&(~BIT(4)));
    }

    //PB5-7,PD0-1 enable buffer
    if(n_pin != SD_ADC_GNDN)
    {
        gpio_input_dis(n_pin_gpio);
        gpio_output_dis(n_pin_gpio);
        if(n_pin_gpio == GPIO_PB5){
            analog_write_reg8(areg_0x10f, analog_read_reg8(areg_0x10f)&(~BIT(0)));
        }else if (n_pin_gpio == GPIO_PB6){
            analog_write_reg8(areg_0x10f, analog_read_reg8(areg_0x10f)&(~BIT(1)));
        }else if(n_pin_gpio == GPIO_PB7){
            analog_write_reg8(areg_0x10f, analog_read_reg8(areg_0x10f)&(~BIT(2)));
        }else if(n_pin_gpio == GPIO_PD0){
            analog_write_reg8(areg_0x10f, analog_read_reg8(areg_0x10f)&(~BIT(3)));
        }else if(n_pin_gpio == GPIO_PD1){
            analog_write_reg8(areg_0x10f, analog_read_reg8(areg_0x10f)&(~BIT(4)));
        }

        g_sd_adc_vref = g_diff_sd_adc_vref;
        g_sd_adc_vref_offset = g_diff_sd_adc_vref_offset;

    }else{
        g_sd_adc_vref = g_single_sd_adc_vref;
        g_sd_adc_vref_offset = g_single_sd_adc_vref_offset;
    }


    sd_adc_set_diff_input(chn, p_pin, n_pin);
}

/**
 * @brief      This function serves to select Vbat voltage division.
 * @param[in]  div - sd_adc_vbat_div_e.
 * @return     none
*/
void sd_adc_set_vbat_divider(sd_adc_vbat_div_e div)
{
    analog_write_reg8(areg_sel_ana_input_div, (analog_read_reg8(areg_sel_ana_input_div) & (~FLD_SEL_VBAT_DIV)) | div<<4 );
    g_sd_adc_divider = 8>>div;
}

/**
 * @brief      This function serves to select the positive and negative voltage divider of the gpio.
 * @param[in]  gpio_div - sd_adc_gpio_chn_div_e.
 * @return     none
 * @note       the positive and negative voltage divider must be configured to be the same,otherwise the output voltage value or temperature values will be incorrect.
 */
void sd_adc_set_gpio_divider(sd_adc_gpio_chn_div_e gpio_div)
{
    analog_write_reg8(areg_sel_ana_input_div, (analog_read_reg8(areg_sel_ana_input_div)&(~(FLD_SEL_ANA_INPUT_P_DIV | FLD_SEL_ANA_INPUT_N_DIV))) | (gpio_div | (gpio_div<<2)) );
    g_sd_adc_divider = 8>>gpio_div;
}

/**
 * @brief      This function is used to initialize the SD_ADC.
 * @param[in]  mode - sd_dc_op_mode_e
 * @return     none
 */
void sd_adc_init(sd_dc_op_mode_e mode)
{
    BM_SET(reg_rst4, FLD_RST4_DC);//dc rst signal enable
    sd_adc_data_weighted_average_en();
    sd_adc_set_op_mode(mode);
}

/**
 * @brief      This function is used to initialize the SD_ADC for gpio sampling.
 * @param[in]  cfg -Pointer to configure the GPIO channel structure.
 * @return     none
 * @note       If you switch to gpio mode from other modes, the first 4 codes of the sample are abnormal
 *             due to the internal filter reset, so you need to discard the first 4 codes.
 */
void sd_adc_gpio_sample_init(sd_adc_gpio_cfg_t *cfg)
{
    if (cfg == NULL){
        return;
    }
    sd_adc_temp_sensor_power_off();//Turn off to reduce power
    sd_adc_set_mux_control(SD_ADC_GPIO_MODE);
    sd_adc_set_clk_freq(cfg->clk_freq);
    sd_adc_set_gpio_divider(cfg->gpio_div);
    sd_adc_set_downsample_rate(cfg->downsample_rate);
    sd_adc_gpio_pin_init(SD_ADC_DC1,cfg->input_p,cfg->input_n);//must be DC1

}

#if SD_ADC_INTERNAL_TEST_FUNC_EN
/**
 * @brief      This function is used to drop the data before entering the filter in 2dc mode and audio+dc mode, preventing the data of two channels from interfering with each other.
 * @param[in]  drop_num -drop the data before entering the filter(drop_num <= 15).
 * @return     none
 * @note       -# drop means to drop the data before entering the filter to prevent the data of two channels from interfering with each other.
 *             -# When using 2dc mode, it will drop both dc0 and dc1 data at the same time.
 *             -# When using audio+dc mode, it will only drop dc data, not audio data.
 *             -# Recommended drop 4 codes, you can set according to the actual situation,  drop num must be <= 15.
 */
void sd_adc_set_drop_num(unsigned char drop_num)
{
    reg_dc_mode_config = (reg_dc_mode_config & FLD_DC_DROP_NUM) | (drop_num << 4);
}

/**
 * @brief      This function is used to set the total sample number of audio and dc in dudio+dc mode
 * @param[in]  total_sample_num -the total sample number of audio and dc in dudio+dc mode.
 * @return     none
 * @note       This API needs to be used together with sd_adc_set_dc_sample_num_with_audio(), the recommended ratio of audio:dc samples is 8:2 for best performance.
 */
void sd_adc_set_audio_dc_total_sample_num(unsigned char total_sample_num)
{
    reg_cnt_num_all = total_sample_num;//total sample num of audio and dc
}

/**
 * @brief      This function is used to set the sample number of dc in dudio+dc mode
 * @param[in]  dc_sample_num -the sample number of dc in dudio+dc mode.
 * @return     none
 * @note       This API needs to be used together with sd_adc_set_audio_dc_total_sample_num(), the recommended ratio of audio:dc samples is 8:2 for best performance.
 */
void sd_adc_set_dc_sample_num_with_audio(unsigned char dc_sample_num)
{
    reg_cnt_num_switch = dc_sample_num;
}
#endif
/**
 * @brief      This function is used to initialize the SD_ADC for vbat sampling.
 * @param[in]  clk_freq  - sample clock frequency: SD_ADC_SAPMPLE_CLK_1M / SD_ADC_SAPMPLE_CLK_2M.
 * @param[in]  div             - sd_adc_vbat_div_e.
 * @param[in]  downsample_rate - sd_adc_downsample_rate_e
 * @return     none
 * @note       If you switch to vbat mode from other modes, the first 4 codes of the sample are abnormal
 *             due to the internal filter reset, so you need to discard the first 4 codes.
 */
void sd_adc_vbat_sample_init(unsigned char clk_freq, sd_adc_vbat_div_e div, sd_adc_downsample_rate_e downsample_rate)
{
#if VBAT_MODE_BELOW_2V2_DETECT_EN
    lpc_vbat_vol_detect_init(LPC_VBAT_FALLING_2P04V_RISING_2P15V);
    lpc_power_on();
#endif
    sd_adc_temp_sensor_power_off();//Turn off to reduce power
    sd_adc_set_mux_control(SD_ADC_VBAT_MODE);
    sd_adc_set_clk_freq(clk_freq);
    sd_adc_set_vbat_divider(div);
    sd_adc_set_downsample_rate(downsample_rate);

    g_sd_adc_vref = g_sd_adc_vbat_calib_vref;
    g_sd_adc_vref_offset = g_sd_adc_vbat_calib_vref_offset;
}

/**
 * @brief      This function is used to initialize the SD_ADC for Temperature Sensor sampling.
 * @param[in]  clk_freq  - sample clock frequency: SD_ADC_SAPMPLE_CLK_1M / SD_ADC_SAPMPLE_CLK_2M.
 * @param[in]  downsample_rate - sd_adc_downsample_rate_e
 * @return     none
 * @note       If you switch to temp mode from other modes, the first 4 codes of the sample are abnormal
 *             due to the internal filter reset, so you need to discard the first 4 codes.
 */
void sd_adc_temp_init(unsigned char clk_freq, sd_adc_downsample_rate_e downsample_rate)
{
    sd_adc_set_mux_control(SD_ADC_TEMP_MODE);
    sd_adc_set_clk_freq(clk_freq);
    sd_adc_set_diff_input(SD_ADC_DC0,SD_ADC_TEMP_SENSOR_P, SD_ADC_GNDN);//DC0 only supply for temperature sensor sampling
    sd_adc_set_downsample_rate(downsample_rate);
    sd_adc_temp_sensor_power_on();
}

/**
 * @brief       This function is used to convert SD_ADC sample code to a voltage.
 * @param[in]   sd_adc_code    - the sd_adc sample code.
 * @param[in]   type       - sd_adc_result_type_e.
 * @return      the average value of adc voltage value.
 */
signed int sd_adc_calculate_voltage(signed int sd_adc_code,sd_adc_result_type_e type)
{
    //When the code value is 0, the returned voltage value should be 0.
    if(sd_adc_code == 0)
    {
        return 0;
    }
    else
    {
        unsigned char vbat_low_vol_detect = lpc_get_result();
        if(vbat_low_vol_detect && vbat_2v2_calib_flag)
        {
            if(type == SD_ADC_VOLTAGE_10X_MV)
            {
                int adc_vol = (signed long long)((float)sd_adc_code/g_sd_adc_downsample_rate/g_sd_adc_downsample_rate/g_sd_adc_downsample_rate/2*g_sd_adc_divider*10000);
                double vbat_vol = (double)adc_vol/10000;
                return (double)((double)vbat_gain_a * vbat_vol * vbat_vol + vbat_gain_b * vbat_vol + vbat_offset_c)*10000;

            }else if(type == SD_ADC_VOLTAGE_MV)
            {
                int adc_vol = (signed long long)((float)sd_adc_code/g_sd_adc_downsample_rate/g_sd_adc_downsample_rate/g_sd_adc_downsample_rate/2*g_sd_adc_divider*1000);
                double vbat_vol = (double)adc_vol/1000;
                return (double)((double)vbat_gain_a * vbat_vol * vbat_vol + vbat_gain_b * vbat_vol + vbat_offset_c)*1000;
            }else
            {
                return 0;
            }
        }else{
            //code to vol: vol = (code*1000/(OSR^3)/2)*diviver (unit:mv)
            if(type == SD_ADC_VOLTAGE_10X_MV)
            {
                return  (signed long long)((float)sd_adc_code/g_sd_adc_downsample_rate/g_sd_adc_downsample_rate/g_sd_adc_downsample_rate/2*g_sd_adc_divider*g_sd_adc_vref)+g_sd_adc_vref_offset;
            }else if(type == SD_ADC_VOLTAGE_MV)
            {
                return  (signed long long)((float)sd_adc_code/g_sd_adc_downsample_rate/g_sd_adc_downsample_rate/g_sd_adc_downsample_rate/2*g_sd_adc_divider*g_sd_adc_vref/10) + (g_sd_adc_vref_offset/10);
            }else
            {
                return 0;
            }
        }

    }
}

/**
 * @brief       This function is used to convert SD_ADC sample code to celsius value.
 * @param[in]   sd_adc_code    - the adc sample code.
 * @return      Celsius value.
 */
signed short sd_adc_calculate_temperature(signed int sd_adc_code)
{
//  sd_adc sample voltage convert to temperature: T = ((884 - V) / 1.4286) - 40 (unit:Celsius)
     return ((signed long long)(((float)884 - ((float)sd_adc_code*(float)10/(float)g_sd_adc_downsample_rate*(float)10/(float)g_sd_adc_downsample_rate*(float)10/(float)g_sd_adc_downsample_rate/(float)2)) * (float)10000/(float)14286 ) - 40 );
}

/**
 * @brief      This function serves to configure sd_adc_dma_chn channel.
 * @param[in]  chn - the DMA channel
 * @return     none
 */
void sd_adc_set_dma_config(dma_chn_e chn)
{
    sd_adc_dma_chn = chn;
    dma_config(chn, &sd_adc_rx_dma_config);
    reg_dma_llp(sd_adc_dma_chn) = 0;
    sd_adc_set_rx_fifo_trig_cnt(0);
}

/**
 * @brief     The adc starts sampling in DMA mode.
 * @param[in] sd_adc_data_buf - Pointer to data buffer, it must be 4-bytes aligned address
 *                            and the actual buffer size defined by the user needs to be not smaller than the data_byte_len,
 *                            otherwise there may be an out-of-bounds problem.
 * @param[in] data_byte_len - Amount of data to be sampled in bytes, the maximum value can be up to 0xFFFFFC.
 * @return    none
 */
void sd_adc_start_sample_dma(signed int *sd_adc_data_buf, unsigned int data_byte_len)
{
    dma_set_address(sd_adc_dma_chn, SD_ADC_FIFO, (unsigned int)sd_adc_data_buf);
    dma_set_size(sd_adc_dma_chn, data_byte_len, DMA_WORD_WIDTH);
    dma_chn_en(sd_adc_dma_chn);
}

/**
 * @brief     This function serves to get adc DMA sample status.
 * @return    0: the sample is in progress.
 *            !0: the sample is finished.
 */
unsigned char sd_adc_get_sample_status_dma(void)
{
    return (dma_get_tc_irq_status(1 << sd_adc_dma_chn));
}

/**
 * @brief     This function serves to clear adc DMA irq status.
 * @return    none
 * @note      The code is placed in the ram code section, in order to shorten the time.
 */
_attribute_ram_code_sec_noinline_ void sd_adc_clr_irq_status_dma(void)
{
    dma_clr_tc_irq_status(1<<sd_adc_dma_chn);
}
