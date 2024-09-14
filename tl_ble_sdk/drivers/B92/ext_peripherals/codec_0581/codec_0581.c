/********************************************************************************************************
 * @file    codec_0581.c
 *
 * @brief   This is the source file for B92
 *
 * @author  Driver Group
 * @date    2023
 *
 * @par     Copyright (c) 2019, Telink Semiconductor (Shanghai) Co., Ltd. ("TELINK")
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
#include "codec_0581.h"

/**
 * @brief      This function serves to get register length .
 * @param[in]  first_byte_addr    external codec register addr.
 * @param[out] reg_width          get register width.
 * @return                        0 - get register width success  -1 - err parameter.
 */
char codec_0581_get_addr_length(unsigned int first_byte_addr, unsigned char *reg_width)
{
    unsigned int last_byte_addr;
    unsigned char width = 0;

    last_byte_addr = first_byte_addr;

    if (first_byte_addr >= CODEC_8BIT_REG_ADDR_START && first_byte_addr <= CODEC_8BIT_REG_ADDR_END)
    {
        if (last_byte_addr >= CODEC_8BIT_REG_ADDR_START && last_byte_addr <= CODEC_8BIT_REG_ADDR_END)
        {
            width = 1;
        }
    }
    else
    {
        if (last_byte_addr < CODEC_8BIT_REG_ADDR_START || last_byte_addr > CODEC_8BIT_REG_ADDR_END)
        {
            width = 4;
        }
    }

    if (width > 0 && first_byte_addr % width == 0)
    {
        *reg_width = width;
        return 0;
    }

    return -1;
}

/**
 * @brief      This function serves to read bit field.
 * @param[in]  reg_addr    external codec register address.
 * @param[in]  bf_info     bit field information, byte0 - start bit, byte1 - bits length.
 * @param[out] bf_val      pointer to bit field read value.
 * @return     none.
 */
static void codec_0581_bf_read(unsigned int reg_addr, unsigned int bf_info, unsigned int *bf_val)
{
    unsigned int reg_value, reg_temp, reg_mask;
    int reg_count, i;
    unsigned char bit_start = bf_info;
    unsigned char bit_count = bf_info >> 8;

    if (reg_addr >= CODEC_8BIT_REG_ADDR_START && reg_addr <= CODEC_8BIT_REG_ADDR_END)
    {
        reg_value = 0;
        reg_count = ((bit_start + bit_count) >> 3) + ((((bit_start + bit_count) & 7) > 0) ? 1 : 0);
        for (i = 0; i < reg_count; i++)
        {
            codec_0581_reg_read(reg_addr + i, &reg_temp);
            reg_value += reg_temp << (i * 8);
        }

        reg_mask = (bit_count == 32) ? (unsigned int)0xffffffff : (unsigned int)((1 << bit_count) - 1);
        *bf_val = (reg_value >> bit_start) & reg_mask;
    }
    else
    {
        codec_0581_reg_read(reg_addr, &reg_value);
        if ((bit_count + bit_start) <= 32)
        {
            reg_mask = (bit_count == 32) ? (unsigned int)0xffffffff : (unsigned int)((1 << bit_count) - 1);
            *bf_val = (reg_value >> bit_start) & reg_mask;
        }
        else
        {
            *bf_val = reg_value >> bit_start;
            codec_0581_reg_read(reg_addr + 4, &reg_value);
            reg_mask = (1 << (bit_count - (32 - bit_start))) - 1;
            *bf_val += (reg_value & reg_mask) << (32 - bit_start);
        }
    }
}

/**
 * @brief      This function serves to write bit field.
 * @param[in]  reg_addr    external codec register address.
 * @param[in]  bf_info     bit field information, byte0 - start bit, byte1 - bits length.
 * @param[in]  bf_val      bit field to write value.
 * @return     none.
 */
static void codec_0581_bf_write(unsigned int reg_addr, unsigned int bf_info, unsigned int bf_val)
{
    unsigned int reg_value, reg_temp, reg_mask;
    int reg_count, i;
    unsigned char bit_start = bf_info;
    unsigned char bit_count = bf_info >> 8;

    if (reg_addr >= CODEC_8BIT_REG_ADDR_START && reg_addr <= CODEC_8BIT_REG_ADDR_END)
    {
        if ((bit_count == 8) && (bit_start == 0))
        {
            codec_0581_reg_write(reg_addr, bf_val);
        }
        else
        {
            reg_value = 0;
            reg_count = ((bit_start + bit_count) >> 3) + ((((bit_start + bit_count) & 7) > 0) ? 1 : 0);
            for (i = 0; i < reg_count; i++)
            {
                codec_0581_reg_read(reg_addr + i, &reg_temp);
                reg_value += reg_temp << (i * 8);
            }
            reg_mask = (bit_count == 32) ? (unsigned int)0xffffffff : (unsigned int)((1 << bit_count) - 1);
            reg_value &= ~(reg_mask << bit_start);
            reg_value |= (bf_val << bit_start);
            for (i = 0; i < reg_count; i++)
            {
                codec_0581_reg_write(reg_addr + i, (reg_value >> (i * 8)) & 0xff);
            }
        }
    }
    else
    {
        if ((bit_count == 32) && (bit_start == 0))
        {
            codec_0581_reg_write(reg_addr, bf_val);
        }
        else
        {
            codec_0581_reg_read(reg_addr, &reg_value);
            if ((bit_count + bit_start) <= 32)
            {
                reg_mask = (bit_count == 32) ? (unsigned int)0xffffffff : (unsigned int)((1 << bit_count) - 1);
                reg_value &= ~(reg_mask << bit_start);
                reg_value |= (bf_val << bit_start);
                codec_0581_reg_write(reg_addr, reg_value);
            }
            else
            {
                reg_mask = 0xffffffff;
                reg_value &= ~(reg_mask << bit_start);
                reg_value |= (bf_val << bit_start);
                codec_0581_reg_write(reg_addr, reg_value);
                codec_0581_reg_read(reg_addr + 4, &reg_value);
                reg_mask = (1 << (bit_count - (32 - bit_start))) - 1;
                reg_value &= ~reg_mask;
                reg_value |= (bf_val >> (32 - bit_start));
                codec_0581_reg_write(reg_addr, reg_value);
            }
        }
    }
}

/**
 * @brief       This function servers to power on/off codec_0581.
 * @param[in]   power_pin        the power pin selected.
 * @param[in]   enable           CODEC_BOOL_TRUE  - pin output enable and power on
 *                               CODEC_BOOL_FALSE - power off
 * @return      none
 * @attention   active Low Power-Down. All digital and analog circuits are powered down. There is an internal pull-down resistor on
 *              this pin; therefore, the codec is held in power-down mode if its input signal is floating while power is applied to
 *              the supply pins.
 */
void codec_0581_power_on(gpio_pin_e power_pin, codec_0581_bool_e enable)
{
    if (CODEC_BOOL_TRUE == enable)
    {
        gpio_function_en(power_pin);
        gpio_output_en(power_pin);
        delay_ms(35); /* there need at least 35ms delay for the CM voltage setting before any further operation */
    }
    gpio_set_level(power_pin, enable);
}

/**
 * @brief       This function servers to get codec_0581 vendor id, device id and device revision
 * @param[out]  vid        pointer to vendor id value
 * @param[out]  did        pointer to device id value
 * @param[out]  drev       pointer to device revision value
 * @return      none
 */
void codec_0581_get_id(unsigned char *vid, unsigned short *did, unsigned char *drev)
{
    unsigned int id;

    codec_0581_bf_read(BF_VENDOR_INFO, &id);
    *vid = (unsigned char)id;

    codec_0581_bf_read(BF_DEVICE1_INFO, &id);
    *did = (unsigned short)id;
    codec_0581_bf_read(BF_DEVICE2_INFO, &id);
    *did = (unsigned short)((id << 8) + (*did));

    codec_0581_bf_read(BF_REV_INFO, &id);
    *drev = (unsigned char)id;
}

/**
 * @brief       This function servers to reset this codec_0581 in soft way
 * @param[in]   full_reset  CODEC_BOOL_TRUE  for full soft reset, software reset of entire IC
 *                          CODEC_BOOL_FALSE for soft reset,      software reset not including register settings
 * @return      none
 */
void codec_0581_reset(codec_0581_bool_e full_reset)
{
    if (CODEC_BOOL_TRUE == full_reset)
    {
        codec_0581_bf_write(BF_SOFT_FULL_RESET_INFO, 1);
    }
    else
    {
        codec_0581_bf_write(BF_SOFT_RESET_INFO, 1);
    }
}

/**
 * @brief       This function servers to power on/off ADC.
 * @param[in]   adc_channel      ADC channel codec_0581_adc_chnl_e
 * @param[in]   enable           CODEC_BOOL_TRUE - power on, CODEC_BOOL_FALSE - power off
 * @return      none
 */
void codec_0581_adc_enable_power_on(codec_0581_adc_chnl_e adc_channel, codec_0581_bool_e enable)
{
    codec_0581_bf_write(BF_ADC0_EN_INFO + adc_channel, enable ? 1 : 0);
}

/**
 * @brief       This function servers to get ADC power on/off status.
 * @param[in]   adc_channel      ADC channel codec_0581_adc_chnl_e
 * @param[out]  power_state      to store the power state which is read back
 * @return      none
 */
void codec_0581_adc_get_power_state(codec_0581_adc_chnl_e adc_channel, unsigned int *power_state)
{
    codec_0581_bf_read(BF_ADC0_EN_INFO + adc_channel, power_state);
}

/**
 * @brief       This function servers to set ADC sample rate
 * @param[in]   adc_channel      ADC channel codec_0581_adc_chnl_e
 * @param[in]   rate             ADC sample rate, codec_0581_adc_sample_rate_e
 * @return      none
 */
void codec_0581_adc_set_sample_rate(codec_0581_adc_chnl_e adc_channel, codec_0581_adc_sample_rate_e rate)
{
    if (adc_channel < 2)
    {
        codec_0581_bf_write(BF_ADC01_FS_INFO, rate);
    }
    else
    {
        codec_0581_bf_write(BF_ADC2_FS_INFO, rate);
    }
}

/**
 * @brief       This function servers to get ADC sample rate
 * @param[in]   adc_channel      ADC channel codec_0581_adc_chnl_e
 * @param[out]  rate             to store ADC sample rate which is read back, codec_0581_adc_sample_rate_e
 * @return      none
 */
void codec_0581_adc_get_sample_rate(codec_0581_adc_chnl_e adc_channel, codec_0581_adc_sample_rate_e *rate)
{
    unsigned int bfvalue;

    if (adc_channel < 2)
    {
        codec_0581_bf_read(BF_ADC01_FS_INFO, &bfvalue);
    }
    else
    {
        codec_0581_bf_read(BF_ADC2_FS_INFO, &bfvalue);
    }

    *rate = (unsigned char)bfvalue;
}

/**
 * @brief       This function servers to set ADC filter
 * @param[in]   adc_channel           ADC channel codec_0581_adc_chnl_e
 * @param[in]   filter                high pass filter cut off frequence, codec_0581_adc_hpf_e
 * @param[in]   higher_order_enable   CODEC_BOOL_TRUE - higher order, CODEC_BOOL_FALSE - lower order
 * @param[in]   high_freq_comp_enable CODEC_BOOL_TRUE - high frequency response compensated, CODEC_BOOL_FALSE - high frequency response not compensated
 * @return      none
 */
void codec_0581_adc_set_filter(codec_0581_adc_chnl_e adc_channel, codec_0581_adc_hpf_e filter, codec_0581_bool_e higher_order_enable, codec_0581_bool_e high_freq_comp_enable)
{
    codec_0581_bf_write(BF_ADC0_HPF_EN_INFO + 2 * adc_channel, filter);

    if (adc_channel < 2)
    {
        codec_0581_bf_write(BF_ADC01_DEC_ORDER_INFO, higher_order_enable ? 1 : 0);
        codec_0581_bf_write(BF_ADC01_FCOMP_INFO, high_freq_comp_enable ? 1 : 0);
    }
    else
    {
        codec_0581_bf_write(BF_ADC2_DEC_ORDER_INFO, higher_order_enable ? 1 : 0);
        codec_0581_bf_write(BF_ADC2_FCOMP_INFO, high_freq_comp_enable ? 1 : 0);
    }
}

/**
 * @brief       This function servers to enable or disable ADC chop
 * @param[in]   enable_flag      Enable flag: CODEC_BOOL_TRUE - enable, CODEC_BOOL_FALSE - disable
 * @return      none
 */
void codec_0581_adc_set_chop_enable(codec_0581_bool_e enable_flag)
{
    codec_0581_bf_write(BF_ADC_CHOP_EN_INFO, enable_flag ? 1 : 0);
}

/**
 * @brief       This function servers to set ADC voice wakeup mode.
 * @param[in]   voicewakeup_enable   CODEC_BOOL_TRUE: enable voice wakeup; CODEC_BOOL_FALSE: disable voice wakeup
 *                                   the ADCx can work in voice wake up mode to further reduce power consumption.
 *                                   during the voice wake up mode, the ADCx sample rate is 8kHz or 16kHz and
 *                                   the ADCx data width is 16 bits (16 MSBs of the ADC output).
 * @return      none
 */
void codec_0581_adc_set_voice_wakeup_mode(codec_0581_bool_e voicewakeup_enable)
{
    codec_0581_bf_write(BF_ADC_VOC_WKUP_INFO, voicewakeup_enable ? 1 : 0);
}

/**
 * @brief       This function servers to set adc input mode.
 * @param[in]   adc_channel      ADC channel codec_0581_adc_chnl_e
 * @param[in]   single_enable    ADC input mode, CODEC_BOOL_TRUE - single input, CODEC_BOOL_FALSE - diff input
 * @return      none
 */
void codec_0581_adc_set_input_mode(codec_0581_adc_chnl_e adc_channel, codec_0581_bool_e single_enable)
{
    codec_0581_bf_write(BF_AIN0_INPUT_MODE_INFO + adc_channel, single_enable ? 1 : 0);
}

/**
 * @brief       This function servers to enable/disable ADC mute.
 * @param[in]   adc_channel      ADC channel codec_0581_adc_chnl_e
 * @param[in]   enable           CODEC_BOOL_TRUE - mute, CODEC_BOOL_FALSE - unmute
 * @return      none
 */
void codec_0581_adc_enable_mute(codec_0581_adc_chnl_e adc_channel, codec_0581_bool_e enable)
{
    codec_0581_bf_write(BF_ADC0_MUTE_INFO + adc_channel, enable ? 1 : 0);
}

/**
 * @brief       This function servers to set ADC volume.
 * @param[in]   adc_channel      ADC channel codec_0581_adc_chnl_e
 * @param[in]   volume           volume, output dB = 24 - 0.375 * volume, if volume is 0xff, mute ADC
 * @return      none
 */
static void codec_0581_adc_set_volume(codec_0581_adc_chnl_e adc_channel, unsigned char volume)
{
    codec_0581_bf_write(adc_channel + BF_ADC0_VOL_INFO, volume);
}

/**
 * @brief       This function servers to set ADC volume.
 * @param[in]   adc_channel      ADC channel codec_0581_adc_chnl_e
 * @param[in]   vol_db           volume, unit: dB
 * @return      none
 */
void codec_0581_adc_set_volume_db(codec_0581_adc_chnl_e adc_channel, float vol_db)
{
    float volume;
    unsigned char vol_code;

    volume = (24 - vol_db) / 0.375;
    volume = (volume < 0) ? 0 : volume;
    volume = (volume > 255) ? 255 : volume;
    vol_code = (unsigned char)(volume + 0.5);

    codec_0581_adc_set_volume(adc_channel, vol_code);
}

/**
 * @brief       This function servers to enable dither and set dither offset size.
 * @param[in]   dither           dither level,  codec_0581_adc_dither_level_e
 * @return      none
 */
void codec_0581_adc_set_dither_level(codec_0581_adc_dither_level_e dither)
{
    codec_0581_bf_write(BF_ADC_DITHER_LEV_INFO, dither);
}

/**
 * @brief       This function servers to enable or disable ASRC channel.
 * @param[in]   chnl_no          ASRCI/O channel to be configured, codec_0581_asrc_chnl_e
 * @param[in]   enable_flag      enable flag: CODEC_BOOL_TRUE - enable, CODEC_BOOL_FALSE - disable
 * @return      none
 */
void codec_0581_asrc_enable_chnl_power_on(codec_0581_asrc_chnl_e chnl_no, codec_0581_bool_e enable_flag)
{
    codec_0581_bf_write(BF_ASRCI0_EN_INFO + chnl_no, enable_flag ? 1 : 0);
}

/**
 * @brief       This function servers to enable or disable multiple ASRC channels.
 * @param[in]   chnl_bits_set    if the bit is set, the channel will be affected; if the bit is clear, the channel will not be affected
 *                               bit[0]:     ASRCI
 *                               bit[1:3]:   reserved
 *                               bit[4:6]:   ASRCO channel 0-2
 *                               bit[7]:     reserved
 * @param[in]   enable_flag      enable or disable flag:
 *                               CODEC_BOOL_TRUE - enable the channels whose bit is set, CODEC_BOOL_FALSE - disable the channels whose bit is clear
 * @return      none
 */
void codec_0581_asrc_enable_multi_chnls_power_on(unsigned char chnl_bits_set, codec_0581_bool_e enable_flag)
{
    unsigned int reg_data;

    codec_0581_reg_read(REG_ASRC_PWR_ADDR, &reg_data);
    codec_0581_reg_write(REG_ASRC_PWR_ADDR, enable_flag ? (reg_data | chnl_bits_set) : (reg_data & (~chnl_bits_set)));
}

/**
 * @brief       This function servers to select ASRCI input channel.
 * @param[in]   route_from       ASRCI input source channel codec_0581_sap_slot_e
 * @return      none
 */
void codec_0581_asrc_select_asrci_route(codec_0581_sap_slot_e route_from)
{
    codec_0581_bf_write(BF_ASRCI0_ROUTE_INFO, route_from);
}

/**
 * @brief       This function servers to select ASRCO input channel.
 * @param[in]   asrco_chnl       ASRCO channel to be configured codec_0581_asrco_chnl_e
 * @param[in]   route_from       ASRCO input source channel
 * @return      none
 */
void codec_0581_asrc_select_asrco_route(codec_0581_asrco_chnl_e asrco_chnl, codec_0581_asrco_route_chnl_e route_from)
{
    codec_0581_reg_write(REG_ASRCO_ROUTE0_ADDR + asrco_chnl, route_from);
}

/**
 * @brief       This function servers to select ASRCI output frequency.
 * @param[in]   out_fs           ASRCI output frequency, codec_0581_asrc_fs_e
 * @return      none
 */
void codec_0581_asrc_select_asrci_fs(codec_0581_asrc_fs_e out_fs)
{
    codec_0581_bf_write(BF_ASRCI_OUT_FS_INFO, out_fs);
}

/**
 * @brief       This function servers to select ASRCO input frequency.
 * @param[in]   in_fs            ASRCO input frequency, codec_0581_asrc_fs_e
 * @return      none
 */
void codec_0581_asrc_select_asrco_fs(codec_0581_asrc_fs_e in_fs)
{
    codec_0581_bf_write(BF_ASRCO_IN_FS_INFO, in_fs);
}

/**
 * @brief       This function servers to power on/off PLL.
 * @param[in]   enable           CODEC_BOOL_TRUE - power on, CODEC_BOOL_FALSE - power off
 * @return      none
 */
void codec_0581_clk_enable_pll_power_on(codec_0581_bool_e enable)
{
    codec_0581_bf_write(BF_PLL_EN_INFO, enable ? 1 : 0);
}

/**
 * @brief       This function servers to configure PLL, PLL output = (input_clk/prescaler)*multiplier for integer mode, = (input_clk/prescaler)*(multiplier + numerator/denominator) for fractional mode
 * @param[in]   pll_src      codec_0581_clk_pll_source_e
 * @param[in]   type         codec_0581_clk_pll_type_e
 * @param[in]   sync_src     codec_0581_clk_sync_source_e
 * @param[in]   prescaler    pll divider
 * @param[in]   multiplier   pll multiplier
 * @param[in]   numerator    pll numerator
 * @param[in]   denominator  pll denominator
 * @return      none
 */
void codec_0581_clk_config_pll(codec_0581_clk_pll_source_e pll_src, codec_0581_clk_pll_type_e type,
                              codec_0581_clk_sync_source_e sync_src, unsigned char prescaler, unsigned short multiplier, unsigned short numerator, unsigned short denominator)
{
    codec_0581_bf_write(BF_PLL_SOURCE_INFO, pll_src);
    codec_0581_clk_update_pll();
    codec_0581_bf_write(BF_PLL_TYPE_INFO, type);
    codec_0581_clk_update_pll();
    codec_0581_bf_write(BF_SYNC_SOURCE_INFO, sync_src);
    codec_0581_clk_update_pll();
    codec_0581_bf_write(BF_PLL_INPUT_PRESCALER_INFO, prescaler);
    codec_0581_bf_write(BF_PLL_INTEGER_DIVIDER_INFO, multiplier);
    codec_0581_bf_write(BF_PLL_NUMERATOR_INFO, numerator);
    codec_0581_bf_write(BF_PLL_DENOMINATOR_INFO, denominator);
    codec_0581_clk_update_pll();
}

/**
 * @brief       This function servers to update all PLL configuration settings.
 * @return      none
 */
void codec_0581_clk_update_pll(void)
{
    codec_0581_bf_write(BF_PLL_UPDATE_INFO, 1);
    codec_0581_bf_write(BF_PLL_UPDATE_INFO, 0);
}

/**
 * @brief       This function servers to get pll locked status.
 * @param[out]  locked           0 - pll unlocked
 *                               1 - pll locked
 * @return      none
 */
void codec_0581_clk_get_pll_locked_status(unsigned char *locked)
{
    unsigned int pll_locked;

    codec_0581_bf_read(BF_PLL_LOCK_INFO, &pll_locked);
    *locked = pll_locked & 0x01;
}

/**
 * @brief       This function servers to get input and output ASRCI lock status.
 * @param[out]  asrci_locked     0 - ASRC currently unlocked  1 - ASRC currently locked
 * @param[out]  asrco_locked     0 - ASRC currently unlocked  1 - ASRC currently locked
 * @return      none
 */
void codec_0581_clk_get_asrc_locked_status(unsigned char *asrci_locked, unsigned char *asrco_locked)
{
    unsigned int pll_locked;

    codec_0581_bf_read(BF_ASRCI_LOCK_INFO, &pll_locked);
    *asrci_locked = pll_locked & 0x01;

    codec_0581_bf_read(BF_ASRCO_LOCK_INFO, &pll_locked);
    *asrco_locked = pll_locked & 0x01;
}

/**
 * @brief       This function servers to enable or disable pll bypass.
 * @param[in]   bypass_pll   enable flag: CODEC_BOOL_TRUE - bypass pll, CODEC_BOOL_FALSE - use pll
 * @return      none
 */
void codec_0581_clk_enable_pll_bypass(codec_0581_bool_e bypass_pll)
{
    codec_0581_bf_write(BF_PLL_FM_BYPASS_INFO, bypass_pll ? 1 : 0);
}

/**
 * @brief       This function servers to select SAP MCLK output rate
 * @param[in]   rate             codec_0581_sap_mclk_rate_e
 * @return      none
 */
void codec_0581_clk_set_sap_mclk_out_rate(codec_0581_sap_mclk_rate_e rate)
{
    codec_0581_bf_write(BF_MCLKO_RATE_INFO, rate);
}

/**
 * @brief       This function servers to power on/off DAC.
 * @param[in]   enable           CODEC_BOOL_TRUE - power on, CODEC_BOOL_FALSE - power off
 * @return      none
 */
void codec_0581_dac_enable_power_on(codec_0581_bool_e enable)
{
    codec_0581_bf_write(BF_PB0_EN_INFO, enable ? 1 : 0);
}

/**
 * @brief       This function servers to set DAC normal/enhance mode
 * @param[in]   enable           CODEC_BOOL_TRUE - enhance mode
 *                               CODEC_BOOL_FALSE - normal mode
 * @return      none
 */
void codec_0581_dac_enhanced_mode_enable(codec_0581_bool_e enable)
{
    if (enable)
    {
        codec_0581_bf_write(BF_PB_POWER_CTRL_INFO, 2);     /* enhance mode */
        codec_0581_bf_write(BF_DAC_LOW_NOISE_EN2_INFO, 1); /* set the DAC_LOW_NOISE_EN1 bit and DAC_LOW_NOISE_EN2 Bit for Better Performance when configured as ENHANCE_MODE. */
        codec_0581_bf_write(BF_DAC_LOW_NOISE_EN1_INFO, 1);
    }
    else
    {
        codec_0581_bf_write(BF_PB_POWER_CTRL_INFO, 0); /* normal mode */
        codec_0581_bf_write(BF_DAC_LOW_NOISE_EN2_INFO, 0);
        codec_0581_bf_write(BF_DAC_LOW_NOISE_EN1_INFO, 0);
    }
}

/**
 * @brief       This function servers to set DAC input sample rate.
 * @param[in]   rate             codec_0581_dac_sample_rate_e
 * @return      none
 */
void codec_0581_dac_set_sample_rate(codec_0581_dac_sample_rate_e rate)
{
    codec_0581_bf_write(BF_DAC_FS_INFO, rate);
}

/**
 * @brief       This function servers to enable/disable DAC high frequency response compensated.
 * @param[in]   hpf_enable               CODEC_BOOL_TRUE - ADC high pass filter On, CODEC_BOOL_FALSE - ADC high pass filter off
 * @param[in]   high_freq_comp_enable    CODEC_BOOL_TRUE - high frequency response compensated, CODEC_BOOL_FALSE - high frequency response not compensated
 * @param[in]   interp_enable            CODEC_BOOL_TRUE -  more interpolation filtering : higher delay, CODEC_BOOL_FALSE - Less interpolation filtering : lower delay
 * @return      none
 */
void codec_0581_dac_set_filter(codec_0581_bool_e hpf_enable, codec_0581_bool_e high_freq_comp_enable, codec_0581_bool_e interp_enable)
{
    codec_0581_bf_write(BF_DAC0_HPF_EN_INFO, hpf_enable ? 1 : 0);
    codec_0581_bf_write(BF_DAC_FCOMP_INFO, high_freq_comp_enable ? 1 : 0);
    codec_0581_bf_write(BF_DAC_MORE_FILT_INFO, interp_enable ? 1 : 0);
}

/**
 * @brief       This function servers to enable/disable low power mode.
 * @param[in]   enable           CODEC_BOOL_TRUE -  enable low power mode, output sample rate 3.072MHz
 *                               CODEC_BOOL_FALSE - disable low power mode, output sample rate 6.144MHz
 * @return      none
 */
void codec_0581_dac_enable_low_power_mode(codec_0581_bool_e enable)
{
    codec_0581_bf_write(BF_DAC_LPM_INFO, enable ? 1 : 0);
}

/**
 * @brief       This function servers to enable/disable DAC mute.
 * @param[in]   enable           CODEC_BOOL_TRUE - mute, CODEC_BOOL_FALSE - unmute
 * @return      none
 */
void codec_0581_dac_enable_mute(codec_0581_bool_e enable)
{
    codec_0581_bf_write(BF_DAC0_MUTE_INFO, enable ? 1 : 0);
    codec_0581_bf_write(BF_DAC0_FORCE_MUTE_INFO, enable ? 1 : 0);
}

/**
 * @brief       This function servers to set DAC volume.
 * @param[in]   volume           volume, output dB = 24 - 0.375 * volume, if volume is 0xff, mute DAC
 * @return      none
 */
static void codec_0581_dac_set_volume(unsigned char volume)
{
    codec_0581_bf_write(BF_DAC0_VOL_INFO, volume);
}

/**
 * @brief       This function servers to set DAC volume.
 * @param[in]   vol_db           volume, unit: dB
 * @return      none
 */
void codec_0581_dac_set_volume_db(float vol_db)
{
    float volume;
    unsigned char vol_code;

    volume = (24 - vol_db) / 0.375;
    volume = (volume < 0) ? 0 : volume;
    volume = (volume > 255) ? 255 : volume;
    vol_code = (unsigned char)(volume + 0.5);

    codec_0581_dac_set_volume(vol_code);
}

/**
 * @brief       This function servers to set DAC input route.
 * @param[in]   route            codec_0581_dac_input_route_e
 * @return      none
 */
void codec_0581_dac_select_input_route(codec_0581_dac_input_route_e route)
{
    codec_0581_bf_write(BF_DAC0_ROUTE_INFO, route);
}

/**
 * @brief       This function servers to set EQ input data source route
 * @param[in]   route            codec_0581_eq_route_e
 * @return      none
 */
void codec_0581_eq_select_route(codec_0581_eq_route_e route)
{
    codec_0581_bf_write(BF_EQ_ROUTE_INFO, route);
}

/**
 * @brief       This function servers to enable/disable EQ
 * @param[in]   enable           CODEC_BOOL_TRUE to run EQ, CODEC_BOOL_FALSE to stop EQ
 * @return      none
 */
void codec_0581_eq_enable_run(codec_0581_bool_e enable)
{
    codec_0581_bf_write(BF_EQ_RUN_INFO, enable ? 1 : 0);
}

/**
 * @brief       This function servers to clear param ram
 * @return      none
 */
void codec_0581_eq_clear_param_ram(void)
{
    codec_0581_bf_write(BF_EQ_CLEAR_INFO, 1);
}

/**
 * @brief       This function servers to get clear status when clearing param ram
 * @param[out]  clear_done       1 to mean clearing is done
 * @return      none
 */
void codec_0581_eq_get_param_ram_clear_status(unsigned char *clear_done)
{
    unsigned int done;

    codec_0581_bf_read(BF_IRQ1_EQ_CLEAR_DONE_INFO, &done);
    *clear_done = done & 0x01;
}

/**
 * @brief       This function servers to select param ram
 * @param[in]   bank_id          param bank id codec_0581_eq_bank_e
 * @return      none
 */
void codec_0581_eq_sel_param_ram(codec_0581_eq_bank_e bank_id)
{
    codec_0581_bf_write(BF_EQ_BANK_SEL_INFO, bank_id);
}

/**
 * @brief       This function servers to enable or disable specific decimator channel.
 * @param[in]   chnl_no          channel to be enabled/disabled codec_0581_fdec_chnl_e
 * @param[in]   enable_flag      enable or disable flag: CODEC_BOOL_TRUE - enable, CODEC_BOOL_FALSE - disable
 * @return      none
 */
void codec_0581_fdec_enable_chnl_power_on(codec_0581_fdec_chnl_e chnl_no, codec_0581_bool_e enable_flag)
{
    codec_0581_bf_write(REG_FDEC_PWR_ADDR, (1 << 8) + chnl_no, enable_flag ? 1 : 0);
}

/**
 * @brief       This function servers to enable or disable multiple decimator channels
 * @param[in]   chnl_bits_set    if the bit is set, the channel will be affected; if the bit is clear, the channel will not be affected
 *                               bit[0:3]:      fdec channel 0-3
 *                               bit[4:7]:      reserved
 * @param[in]   enable_flag      enable or disable flag:
 *                               CODEC_BOOL_TRUE - enable the channels whose bit is set, CODEC_BOOL_FALSE - disable the channels whose bit is clear
 * @return      none
 */
void codec_0581_fdec_enable_multi_chnls_power_on(unsigned char chnl_bits_set, codec_0581_bool_e enable_flag)
{
    unsigned int reg_data;

    codec_0581_reg_read(REG_FDEC_PWR_ADDR, &reg_data);
    codec_0581_reg_write(REG_FDEC_PWR_ADDR, enable_flag ? (reg_data | chnl_bits_set) : (reg_data & (~chnl_bits_set)));
}

/**
 * @brief       This function servers to select sample frequency of a pair of decimator channels.
 * @param[in]   pair_chnls       channel pair to be set
 * @param[in]   in_fs            input sample rate
 * @param[in]   out_fs           output sample rate
 * @return      none
 */
void codec_0581_fdec_select_pair_chnls_fs(codec_0581_fdec_pair_chnl_e pair_chnls, codec_0581_fdec_in_fs_e in_fs, codec_0581_fdec_out_fs_e out_fs)
{
    codec_0581_reg_write(REG_FDEC_CTRL1_ADDR + pair_chnls, (out_fs << 4) + in_fs);
}

/**
 * @brief       This function servers to select the route source of a decimator channel.
 * @param[in]   chnl_no          decimator channel to be configured codec_0581_fdec_chnl_e
 * @param[in]   route_chnl       input/output channel for the decimator channel
 * @return      none
 */
void codec_0581_fdec_select_chnl_route(codec_0581_fdec_chnl_e chnl_no, codec_0581_fdec_route_chnl_e route_chnl)
{
    codec_0581_reg_write(REG_FDEC_ROUTE0_ADDR + chnl_no, route_chnl);
}

/**
 * @brief       This function servers to power on/off fast DSP.
 * @param[in]   enable           enable: CODEC_BOOL_TRUE - power on, CODEC_BOOL_FALSE - power off
 * @return      none
 */
void codec_0581_fdsp_enable_power_on(codec_0581_bool_e enable)
{
    codec_0581_bf_write(BF_FDSP_EN_INFO, enable ? 1 : 0);
}

/**
 * @brief       This function servers to run/stop fast DSP.
 * @param[in]   enable           enable: CODEC_BOOL_TRUE - run, CODEC_BOOL_FALSE - stop
 * @return      none
 */
void codec_0581_fdsp_enable_run(codec_0581_bool_e enable)
{
    codec_0581_bf_write(BF_FDSP_RUN_INFO, enable ? 1 : 0);
}

/**
 * @brief       This function servers to select parameter bank.
 * @param[in]   sel              parameter bank index, codec_0581_fdsp_param_bank_e
 * @return      none
 */
void codec_0581_fdsp_select_param_bank(codec_0581_fdsp_param_bank_e sel)
{
    codec_0581_bf_write(BF_FDSP_BANK_SEL_INFO, sel);
}

/**
 * @brief       This function servers to set parameter bank switch settings.
 * @param[in]   mode             ramp or instant, codec_0581_fdsp_param_switch_mode_e
 * @param[in]   rate             ramp or instant, codec_0581_fdsp_ramp_rate_e
 * @param[in]   zero             enable: CODEC_BOOL_TRUE - zero state during bank switch, CODEC_BOOL_FALSE - do not zero state during bank switch
 * @param[in]   stop_point       parameter bank switch ramp will stop at lambda/64 of full ramp
 * @return      none
 */
void codec_0581_fdsp_config_bank_switch_settings(codec_0581_fdsp_param_switch_mode_e mode, codec_0581_fdsp_ramp_rate_e rate, codec_0581_bool_e zero, unsigned char stop_point)
{
    codec_0581_bf_write(BF_FDSP_RAMP_MODE_INFO, mode);
    codec_0581_bf_write(BF_FDSP_RAMP_RATE_INFO, rate);
    codec_0581_bf_write(BF_FDSP_ZERO_STATE_INFO, zero ? 1 : 0);
    codec_0581_bf_write(BF_FDSP_LAMBDA_INFO, stop_point);
}

/**
 * @brief       This function servers to copy parameter bank.
 * @param[in]   mask             copy mask codec_0581_fdsp_copy_param_bank_e
 * @return                       0 - successful  -1 failed
 */
char codec_0581_fdsp_copy_param_bank(codec_0581_fdsp_copy_param_bank_e mask)
{
    unsigned int sel, loop_count = 0;
    unsigned int copy_done;
    codec_0581_bf_read(BF_FDSP_BANK_SEL_INFO, &sel);

    /* Cannot copy to active bank */
    if (((sel == CODEC_FDSP_PARAM_BANK_A) && ((mask == CODEC_FDSP_COPY_PARAM_BANK_B2A) || (mask == CODEC_FDSP_COPY_PARAM_BANK_C2A))) ||
        ((sel == CODEC_FDSP_PARAM_BANK_B) && ((mask == CODEC_FDSP_COPY_PARAM_BANK_A2B) || (mask == CODEC_FDSP_COPY_PARAM_BANK_C2B))) ||
        ((sel == CODEC_FDSP_PARAM_BANK_C) && ((mask == CODEC_FDSP_COPY_PARAM_BANK_A2C) || (mask == CODEC_FDSP_COPY_PARAM_BANK_B2C))))
    {
        return -1;
    }

    codec_0581_reg_write(REG_FDSP_CTRL3_ADDR, 1 << mask);

    do
    {
        codec_0581_bf_read(BF_FDSP_BANK_COPY_DONE_INFO, &copy_done);
    } while ((copy_done == 0) && (loop_count++ < 1000));

    codec_0581_bf_write(BF_FDSP_COPY_DONE_CLEAR_INFO, 1);

    return 0;
}

/**
 * @brief       This function servers to set rate source and divider.
 * @param[in]   source           rate source codec_0581_fdsp_rate_src_e
 * @param[in]   div              divider, 16 bits
 * @return      none
 */
void codec_0581_fdsp_set_rate(codec_0581_fdsp_rate_src_e source, unsigned short div)
{
    codec_0581_bf_write(BF_FDSP_RATE_SOURCE_INFO, source);
    codec_0581_bf_write(BF_FDSP_RATE_DIV_INFO, div);
}

/**
 * @brief       This function servers to set modulo N.
 * @param[in]   n                N, 6 bits
 * @return      none
 */
void codec_0581_fdsp_set_modulo_n(unsigned char n)
{
    codec_0581_bf_write(BF_FDSP_MOD_N_INFO, n);
}

/**
 * @brief       This function servers to load active parameter bank.
 * @param[in]   inst_index       instruction index, 0 ~ 95
 * @param[in]   param            parameter memory 0 ~ 4
 * @return      none
 */
void codec_0581_fdsp_safeload(unsigned char inst_index, unsigned int param[5])
{
    codec_0581_bf_write(BF_FDSP_SL_ADDR_INFO, inst_index);
    codec_0581_bf_write(BF_FDSP_SL_P0_INFO, param[0]);
    codec_0581_bf_write(BF_FDSP_SL_P1_INFO, param[1]);
    codec_0581_bf_write(BF_FDSP_SL_P2_INFO, param[2]);
    codec_0581_bf_write(BF_FDSP_SL_P3_INFO, param[3]);
    codec_0581_bf_write(BF_FDSP_SL_P4_INFO, param[4]);
    codec_0581_bf_write(BF_FDSP_SL_UPDATE_INFO, 0);
    codec_0581_bf_write(BF_FDSP_SL_UPDATE_INFO, 1);
}

/**
 * @brief       This function servers to enable or disable specific interpolator channel
 * @param[in]   chnl_no          channel to be enabled/disabled codec_0581_fint_chnl_e
 * @param[in]   enable_flag      enable or disable flag: CODEC_BOOL_TRUE - enable, CODEC_BOOL_FALSE - disable
 * @return      none
 */
void codec_0581_fint_enable_chnl_power_on(codec_0581_fint_chnl_e chnl_no, codec_0581_bool_e enable_flag)
{
    codec_0581_bf_write(BF_FINT0_EN_INFO + chnl_no, enable_flag ? 1 : 0);
}

/**
 * @brief       This function servers to enable or disable multiple interpolator channels
 * @param[in]   chnl_bits_set    if the bit is set, the channel will be affected; if the bit is clear, the channel will not be affected
 *                               bit[0:1]:    fint channel 0-1
 *                               bit[2:7]:    reserved
 * @param[in]   enable_flag      enable or disable flag:
 *                               CODEC_BOOL_TRUE - enable the channels whose bit is set, CODEC_BOOL_FALSE - disable the channels whose bit is clear
 * @return      none
 */
void codec_0581_fint_enable_multi_chnls_power_on(unsigned char chnl_bits_set, codec_0581_bool_e enable_flag)
{
    unsigned int reg_data;

    codec_0581_reg_read(REG_FINT_PWR_ADDR, &reg_data);
    codec_0581_reg_write(REG_FINT_PWR_ADDR, enable_flag ? (reg_data | chnl_bits_set) : (reg_data & (~chnl_bits_set)));
}

/**
 * @brief       This function servers to select sample frequency of a pair of interpolator channels.
 * @param[in]   pair_chnls       channel pair to be set
 * @param[in]   in_fs            input sample rate
 * @param[in]   out_fs           output sample rate
 * @return      none
 */
void codec_0581_fint_select_pair_chnls_fs(codec_0581_fint_pair_chnl_e pair_chnls, codec_0581_fint_in_fs_e in_fs, codec_0581_fint_out_fs_e out_fs)
{
    codec_0581_reg_write(REG_FINT_CTRL1_ADDR + pair_chnls, (out_fs << 4) + in_fs);
}

/**
 * @brief       This function servers to select the route source of a interpolator channel.
 * @param[in]   chnl_no          interpolator channel to be configured codec_0581_fint_chnl_e
 * @param[in]   route_chnl       input/output channel for the decimator channel
 * @return      none
 */
void codec_0581_fint_select_chnl_route(codec_0581_fint_chnl_e chnl_no, codec_0581_fint_route_chnl_e route_chnl)
{
    codec_0581_reg_write(REG_FINT_ROUTE0_ADDR + chnl_no, route_chnl);
}

/**
 * @brief       This function servers to set headphone output mode.
 * @param[in]   mode             headphone or line-out, codec_0581_hpamp_mode_e
 * @return      none
 */
void codec_0581_hpamp_set_output_mode(codec_0581_hpamp_mode_e mode)
{
    codec_0581_bf_write(BF_HP0_MODE_INFO, mode);
}

/**
 * @brief       This function servers to clear interrupt request
 * @param[in]   index      codec_0581_irq_index_e
 * @return      none
 */
void codec_0581_int_clr_irq(codec_0581_irq_index_e index)
{
    switch (index)
    {
    case 1:
        codec_0581_bf_write(BF_IRQ1_CLEAR_INFO, 1);
        break;
    case 2:
        codec_0581_bf_write(BF_IRQ2_CLEAR_INFO, 1);
        break;
    default:
        break;
    }
}

/**
 * @brief       This function servers to set interrupt request polarity
 * @param[in]   index      codec_0581_irq_index_e
 * @param[in]   pol        0: active low, 1: active high
 * @return      none
 */
void codec_0581_int_set_irq_pol(codec_0581_irq_index_e index, unsigned char pol)
{
    switch (index)
    {
    case 1:
        codec_0581_bf_write(BF_IRQ1_PINOUT_POL_INFO, pol);
        break;
    case 2:
        codec_0581_bf_write(BF_IRQ2_PINOUT_POL_INFO, pol);
        break;
    default:
        break;
    }
}

/**
 * @brief       This function servers to enable interrupt request
 * @param[in]   index      codec_0581_irq_index_e
 * @param[in]   irq_src    codec_0581_irq_src_e
 * @param[in]   enable     CODEC_BOOL_TRUE to enable, CODEC_BOOL_FALSE to disable
 * @return      none
 */
void codec_0581_int_enable_irq(codec_0581_irq_index_e index, codec_0581_irq_src_e irq_src, codec_0581_bool_e enable)
{
    unsigned int mask = enable > 0 ? 0 : 1;
    unsigned int masks = 0;

    if ((irq_src & 0x0000000f) > 0)
    {
        codec_0581_reg_read(REG_IRQ1_MASK1_ADDR + (index - 1) * 3, &masks);
        masks = ((irq_src & CODEC_IRQ_DAC0_CLIP) > 0)        ? ((masks & 0xfe) | (mask << 0)) : masks;
        masks = ((irq_src & CODEC_IRQ_ADC0_CLIP) > 0)        ? ((masks & 0xef) | (mask << 4)) : masks;
        masks = ((irq_src & CODEC_IRQ_ADC1_CLIP) > 0)        ? ((masks & 0xdf) | (mask << 5)) : masks;
        masks = ((irq_src & CODEC_IRQ_ADC2_CLIP) > 0)        ? ((masks & 0xbf) | (mask << 6)) : masks;
        codec_0581_reg_write(REG_IRQ1_MASK1_ADDR + (index - 1) * 3, masks);
    }
    if ((irq_src & 0x00003cf0) > 0)
    {
        codec_0581_reg_read(REG_IRQ1_MASK2_ADDR + (index - 1) * 3, &masks);
        masks = ((irq_src & CODEC_IRQ_PLL_LOCKED) > 0)       ? ((masks & 0xfe) | (mask << 0)) : masks;
        masks = ((irq_src & CODEC_IRQ_PLL_UNLOCKED) > 0)     ? ((masks & 0xfd) | (mask << 1)) : masks;
        masks = ((irq_src & CODEC_IRQ_AVDD_UVW) > 0)         ? ((masks & 0xfb) | (mask << 2)) : masks;
        masks = ((irq_src & CODEC_IRQ_PRAMP) > 0)            ? ((masks & 0xf7) | (mask << 3)) : masks;
        masks = ((irq_src & CODEC_IRQ_ASRCI_LOCKED) > 0)     ? ((masks & 0xef) | (mask << 4)) : masks;
        masks = ((irq_src & CODEC_IRQ_ASRCI_UNLOCKED) > 0)   ? ((masks & 0xdf) | (mask << 5)) : masks;
        masks = ((irq_src & CODEC_IRQ_ASRCO_LOCKED) > 0)     ? ((masks & 0xbf) | (mask << 6)) : masks;
        masks = ((irq_src & CODEC_IRQ_ASRCO_UNLOCKED) > 0)   ? ((masks & 0x7f) | (mask << 7)) : masks;
        codec_0581_reg_write(REG_IRQ1_MASK2_ADDR + (index - 1) * 3, masks);
    }
    if ((irq_src & 0x0005c300) > 0)
    {
        codec_0581_reg_read(REG_IRQ1_MASK3_ADDR + (index - 1) * 3, &masks);
        masks = ((irq_src & CODEC_IRQ_SPT0_UNLOCKED) > 0)    ? ((masks & 0xfe) | (mask << 0)) : masks;
        masks = ((irq_src & CODEC_IRQ_SYNC_LOCKED) > 0)      ? ((masks & 0xfd) | (mask << 1)) : masks;
        masks = ((irq_src & CODEC_IRQ_SYNC_UNLOCKED) > 0)    ? ((masks & 0xfb) | (mask << 2)) : masks;
        masks = ((irq_src & CODEC_IRQ_POWERUP_COMPLETE) > 0) ? ((masks & 0xef) | (mask << 4)) : masks;
        masks = ((irq_src & CODEC_IRQ_EQ_CLR_DONE) > 0)      ? ((masks & 0xdf) | (mask << 5)) : masks;
        masks = ((irq_src & CODEC_IRQ_FDSP) > 0)             ? ((masks & 0xbf) | (mask << 6)) : masks;
        codec_0581_reg_write(REG_IRQ1_MASK3_ADDR + (index - 1) * 3, masks);
    }
}

/**
 * @brief       This function servers to get interrupt status
 * @param[in]   index      codec_0581_irq_index_e
 * @param[in]   irq_src    codec_0581_irq_src_e
 * @param[out]  status     0 - interrupt was not triggered, 1 - interrupt was triggered
 * @return      none
 */
void codec_0581_int_get_irq_status(codec_0581_irq_index_e index, codec_0581_irq_src_e irq_src, unsigned char *status)
{
    unsigned int irq_status = 0;

    switch (irq_src)
    {
    case CODEC_IRQ_DAC0_CLIP:
        codec_0581_bf_read((index - 1) * 3 + BF_IRQ1_DAC0_CLIP_INFO, &irq_status);
        break;
    case CODEC_IRQ_ADC0_CLIP:
        codec_0581_bf_read((index - 1) * 3 + BF_IRQ1_ADC0_CLIP_INFO, &irq_status);
        break;
    case CODEC_IRQ_ADC1_CLIP:
        codec_0581_bf_read((index - 1) * 3 + BF_IRQ1_ADC1_CLIP_INFO, &irq_status);
        break;
    case CODEC_IRQ_ADC2_CLIP:
        codec_0581_bf_read((index - 1) * 3 + BF_IRQ1_ADC2_CLIP_INFO, &irq_status);
        break;
    case CODEC_IRQ_PLL_LOCKED:
        codec_0581_bf_read((index - 1) * 3 + BF_IRQ1_PLL_LOCKED_INFO, &irq_status);
        break;
    case CODEC_IRQ_PLL_UNLOCKED:
        codec_0581_bf_read((index - 1) * 3 + BF_IRQ1_PLL_UNLOCKED_INFO, &irq_status);
        break;
    case CODEC_IRQ_AVDD_UVW:
        codec_0581_bf_read((index - 1) * 3 + BF_IRQ1_AVDD_UVW_INFO, &irq_status);
        break;
    case CODEC_IRQ_PRAMP:
        codec_0581_bf_read((index - 1) * 3 + BF_IRQ1_PRAMP_INFO, &irq_status);
        break;
    case CODEC_IRQ_FDSP:
        codec_0581_bf_read((index - 1) * 3 + BF_IRQ1_FDSP_MASK_ONZ_INFO, &irq_status);
        break;
    case CODEC_IRQ_EQ_CLR_DONE:
        codec_0581_bf_read((index - 1) * 3 + BF_IRQ1_EQ_CLEAR_DONE_INFO, &irq_status);
        break;
    case CODEC_IRQ_ASRCI_LOCKED:
        codec_0581_bf_read((index - 1) * 3 + BF_IRQ1_ASRCI_LOCKED_INFO, &irq_status);
        break;
    case CODEC_IRQ_ASRCI_UNLOCKED:
        codec_0581_bf_read((index - 1) * 3 + BF_IRQ1_ASRCI_UNLOCKED_INFO, &irq_status);
        break;
    case CODEC_IRQ_ASRCO_LOCKED:
        codec_0581_bf_read((index - 1) * 3 + BF_IRQ1_ASRCO_LOCKED_INFO, &irq_status);
        break;
    case CODEC_IRQ_ASRCO_UNLOCKED:
        codec_0581_bf_read((index - 1) * 3 + BF_IRQ1_ASRCO_UNLOCKED_INFO, &irq_status);
        break;
    case CODEC_IRQ_SYNC_LOCKED:
        codec_0581_bf_read((index - 1) * 3 + BF_IRQ1_SYNC_LOCKED_INFO, &irq_status);
        break;
    case CODEC_IRQ_SYNC_UNLOCKED:
        codec_0581_bf_read((index - 1) * 3 + BF_IRQ1_SYNC_UNLOCKED_INFO, &irq_status);
        break;
    case CODEC_IRQ_SPT0_UNLOCKED:
        codec_0581_bf_read((index - 1) * 3 + BF_IRQ1_SPT0_UNLOCKED_INFO, &irq_status);
        break;
    case CODEC_IRQ_POWERUP_COMPLETE:
        codec_0581_bf_read((index - 1) * 3 + BF_IRQ1_POWER_UP_COMPLETE_INFO, &irq_status);
        break;
    default:
        break;
    }

    *status = irq_status & 0xff;
}

/**
 * @brief       This function servers to power on/off PGA.
 * @param[in]   pga_channel      PGA channel codec_0581_pga_chnl_e
 * @param[in]   enable           CODEC_BOOL_TRUE - power on, CODEC_BOOL_FALSE - power off
 * @return      none
 */
void codec_0581_pga_enable_power_on(codec_0581_pga_chnl_e pga_channel, codec_0581_bool_e enable)
{
    codec_0581_bf_write(BF_PGA0_EN_INFO + pga_channel, enable ? 1 : 0);
}

/**
 * @brief       This function servers to set PGA course gain.
 * @param[in]   pga_channel      PGA channel codec_0581_pga_chnl_e
 * @param[in]   gain             gain, output in dB is 0.75 * gain
 * @return      none
 */
void codec_0581_pga_set_gain(codec_0581_pga_chnl_e pga_channel, unsigned char gain)
{
    codec_0581_reg_write(REG_PGA0_CTRL1_ADDR + 1 + 2 * pga_channel, gain);
}

/**
 * @brief       This function servers to enable/disable gain link.
 * @param[in]   enable           CODEC_BOOL_TRUE - all PGA channels use channel 0 gain value
 *                               CODEC_BOOL_FALSE - each PGA channel uses its respective gain value
 * @return      none
 */
void codec_0581_pga_enable_gain_link(codec_0581_bool_e enable)
{
    codec_0581_bf_write(BF_PGA_GAIN_LINK_INFO, enable ? 1 : 0);
}

/**
 * @brief       This function servers to set input resistance.
 * @param[in]   pga_channel           PGA channel index codec_0581_pga_chnl_e
 * @param[in]   low_resistance_enable CODEC_BOOL_TRUE - PGA channel has low input resistance
 *                                    CODEC_BOOL_FALSE - PGA channel has high input resistance
 * @return      none
 */
void codec_0581_pga_set_input_resistance(codec_0581_pga_chnl_e pga_channel, codec_0581_bool_e low_resistance_enable)
{
    codec_0581_bf_write(BF_PGA0_RIN_INFO + pga_channel, low_resistance_enable ? 1 : 0);
}

/**
 * @brief       This function servers to set PGA operation mode
 * @param[in]   pga_channel      PGA channel index codec_0581_pga_chnl_e
 * @param[in]   enhanced_enable  CODEC_BOOL_TRUE - PGA channel is in enhanced operation mode
 *                               CODEC_BOOL_FALSE - PGA channel is in normal  performance mode.
 * @return      none
 */
void codec_0581_pga_set_power_mode(codec_0581_pga_chnl_e pga_channel, codec_0581_bool_e enhanced_enable)
{
    codec_0581_bf_write(BF_PGA0_POWER_MODE_INFO + pga_channel, enhanced_enable ? 1 : 0);
}

/**
 * @brief  This function servers enable/disable high power CM startup boost mode
 * @param  enable_flag           CODEC_BOOL_TRUE - CM pin fast charge disabled
 *                               CODEC_BOOL_FALSE - CM pin fast charge enabled setting to false can save some power
 * @return      none
 * @attention  recommended to set CM_STARTUP_OVER = 1 to keep lower power consumption
 */
void codec_0581_pmu_enable_cm_pin_fast_charge(codec_0581_bool_e enable_flag)
{
    codec_0581_bf_write(BF_CM_STARTUP_OVER_INFO, enable_flag ? 0 : 1);
}

/**
 * @brief       This function servers to set codec_0581 power mode.
 * @param[in]   mode             CODEC_PWR_MODE_HIBERNATE - set chip into it's lowest power state
 *                               CODEC_PWR_MODE_ACTIVE    - allows block enabling of PLL, FDSP, and EQ etc
 * @return      none
 */
void codec_0581_pmu_set_power_mode(codec_0581_pwr_mode_e mode)
{
    codec_0581_bf_write(BF_POWER_EN_INFO, mode);
}

/**
 * @brief       This function servers to enable or disable master block power.
 * @param[in]   enable_flag      enable flag: CODEC_BOOL_TRUE  - all blocks that have their respective block enable set
 *                                            CODEC_BOOL_FALSE - all blocks are disabled
 * @return      none
 */
void codec_0581_pmu_enable_master_block(codec_0581_bool_e enable_flag)
{
    codec_0581_bf_write(BF_MASTER_BLOCK_EN_INFO, enable_flag ? 1 : 0);
}

/**
 * @brief       This function servers to configure all the control features of SAP
 * @param[in]   config_ptr       pointer to codec_0581_sap_config_t to be used.
 * @return      none
 * @attention   b92 is not support TDM mode
 */
void codec_0581_sap_config_all(codec_0581_sap_config_t *config_ptr)
{
    codec_0581_sap_set_sai_mode(config_ptr->sai_mode);
    codec_0581_sap_set_data_format(config_ptr->data_format);
    codec_0581_sap_set_tri_state(config_ptr->tri_state);
    codec_0581_sap_set_bclk_src(config_ptr->bclk_src);
    codec_0581_sap_set_bclk_pol(config_ptr->bclk_pol);
    codec_0581_sap_set_lrclk_src(config_ptr->lrclk_src);
    codec_0581_sap_set_lrclk_pol(config_ptr->lrclk_pol);
}

/**
 * @brief       This function servers to set serial audio interface mode
 * @param[in]   sai_mode         sai mode codec_0581_sap_sai_mode_e.
 * @return      none
 * @attention   b92 is not support TDM mode
 */
void codec_0581_sap_set_sai_mode(codec_0581_sap_sai_mode_e sai_mode)
{
    codec_0581_bf_write(BF_SPT0_SAI_MODE_INFO, sai_mode);
}

/**
 * @brief       This function servers to set SAP data format
 * @param[in]   data_format      data format codec_0581_sap_data_format_e.
 * @return      none
 */
void codec_0581_sap_set_data_format(codec_0581_sap_data_format_e data_format)
{
    codec_0581_bf_write(BF_SPT0_DATA_FORMAT_INFO, data_format);
}

/**
 * @brief       This function servers to set serial port output tri-state enable or disable
 * @param[in]   tri_state        tri-state codec_0581_sap_tristate_e.
 * @return      none
 */
void codec_0581_sap_set_tri_state(codec_0581_sap_tristate_e tri_state)
{
    codec_0581_bf_write(BF_SPT0_TRI_STATE_INFO, tri_state);
}

/**
 * @brief       This function servers to set SAP bit clock source
 * @param[in]   bclk_src         bit clock source codec_0581_sap_bclk_src_e.
 * @return      none
 */
void codec_0581_sap_set_bclk_src(codec_0581_sap_bclk_src_e bclk_src)
{
    codec_0581_bf_write(BF_SPT0_BCLK_SRC_INFO, bclk_src);
}

/**
 * @brief       This function servers to set SAP bit clock polarity
 * @param[in]   bclk_pol         bit clock polarity codec_0581_sap_bclk_polarity_e.
 * @return      none
 */
void codec_0581_sap_set_bclk_pol(codec_0581_sap_bclk_polarity_e bclk_pol)
{
    codec_0581_bf_write(BF_SPT0_BCLK_POL_INFO, bclk_pol);
}

/**
 * @brief       This function servers to set SAP frame (LR) clock source
 * @param[in]   lrclk_src        bit clock source codec_0581_sap_lrclk_src_e.
 * @return      none
 */
void codec_0581_sap_set_lrclk_src(codec_0581_sap_lrclk_src_e lrclk_src)
{
    codec_0581_bf_write(BF_SPT0_LRCLK_SRC_INFO, lrclk_src);
}

/**
 * @brief       This function servers to set SAP frame (LR) clock polarity
 * @param[in]   lrclk_pol        frame clock polarity codec_0581_sap_lrclk_polarity_e.
 * @return      none
 */
void codec_0581_sap_set_lrclk_pol(codec_0581_sap_lrclk_polarity_e lrclk_pol)
{
    codec_0581_bf_write(BF_SPT0_LRCLK_POL_INFO, lrclk_pol);
}

/**
 * @brief       This function servers to select SAP SPT output slot data source
 * @param[in]   slot_id          the slot id to be configured codec_0581_sap_slot_e
 * @param[in]   route_from       where the output data from. codec_0581_sap_output_route_from_e
 * @return      none
 */
void codec_0581_sap_select_out_route(codec_0581_sap_slot_e slot_id, codec_0581_sap_output_route_from_e route_from)
{
    codec_0581_reg_write(REG_SPT0_ROUTE0_ADDR + slot_id, route_from);
}

/**
 * @brief       This function servers to turn on/off SAP input
 * @param[in]   enable_flag      CODEC_BOOL_TRUE - turn on input, CODEC_BOOL_FALSE - turn off input
 * @return      none
 */
void codec_0581_sap_enable_chnl_input(codec_0581_bool_e enable_flag)
{
    codec_0581_bf_write(BF_SPT0_IN_EN_INFO, enable_flag ? 1 : 0);
}

/**
 * @brief       This function servers to turn on/off SAP output
 * @param[in]   enable_flag      CODEC_BOOL_TRUE - turn on output, CODEC_BOOL_FALSE - turn off output
 * @return      none
 */
void codec_0581_sap_enable_chnl_output(codec_0581_bool_e enable_flag)
{
    codec_0581_bf_write(BF_SPT0_OUT_EN_INFO, enable_flag ? 1 : 0);
}

/**
 * @brief This function serves to initialize input route
 * @param[in]  input_config - the relevant configuration codec_0581_input_t struct pointer
 * @return    none
 */
void codec_0581_input_init(codec_0581_input_t *input_config)
{
    /* adc */
    if (input_config->adc_config)
    {
        codec_0581_adc_set_sample_rate(input_config->adc_config->adc_chnl, input_config->adc_config->adc_rate);
        if (CODEC_ADC_CHNL_2 == input_config->adc_config->adc_chnl)
            codec_0581_adc_set_input_mode(CODEC_ADC_CHNL_2, CODEC_BOOL_FALSE); /* ADC2 default single */
        codec_0581_adc_enable_power_on(input_config->adc_config->adc_chnl, CODEC_BOOL_TRUE);
    }

    /* asrco */
    if (input_config->asrco_config)
    {
        codec_0581_asrc_select_asrco_fs(input_config->asrco_config->asrco_in_fs);
        codec_0581_asrc_select_asrco_route(input_config->asrco_config->asrco_chnl, input_config->asrco_config->asrco_route_from);
        codec_0581_asrc_enable_chnl_power_on(input_config->asrco_config->asrco_chnl + 4, CODEC_BOOL_TRUE);
    }

    /* fdec */
    if (input_config->fdec_config)
    {
        if (input_config->fdec_config->fdec_chnl <= CODEC_FDEC_CHNL_1)
            codec_0581_fdec_select_pair_chnls_fs(CODEC_FDEC_PAIR_CHNL_0_1, input_config->fdec_config->fdec_in_fs, input_config->fdec_config->fdec_out_fs);
        else
            codec_0581_fdec_select_pair_chnls_fs(CODEC_FDEC_PAIR_CHNL_2_3, input_config->fdec_config->fdec_in_fs, input_config->fdec_config->fdec_out_fs);
        codec_0581_fdec_select_chnl_route(input_config->fdec_config->fdec_chnl, input_config->fdec_config->fdec_route_from);
        codec_0581_fdec_enable_chnl_power_on(input_config->fdec_config->fdec_chnl, CODEC_BOOL_TRUE);
    }

    /* sap */
    if (input_config->sap_config)
    {
        codec_0581_sap_select_out_route(input_config->sap_config->slot_id, input_config->sap_config->sap_route_from);
        codec_0581_sap_enable_chnl_output(CODEC_BOOL_TRUE); /* sap output enable(mcu input means codec output) */
    }
}

/**
 * @brief This function serves to initialize output route
 * @param[in]  output_config - the relevant configuration codec_0581_output_t struct pointer
 * @return    none
 */
void codec_0581_output_init(codec_0581_output_t *output_config)
{
    /* sap */
    codec_0581_sap_enable_chnl_input(CODEC_BOOL_TRUE); /* sap input enable(mcu output means codec input) */

    /* asrci */
    if (output_config->asrci_config)
    {
        codec_0581_asrc_select_asrci_fs(output_config->asrci_config->asrci_out_fs);
        codec_0581_asrc_select_asrci_route(CODEC_SAP_SLOT1_RIGHT);
        codec_0581_asrc_enable_chnl_power_on(CODEC_ASRC_CHNL_ASRCI0, CODEC_BOOL_TRUE);
    }

    /* fint */
    if (output_config->fint_config)
    {
        codec_0581_fint_select_pair_chnls_fs(CODEC_FINT_PAIR_CHNL_0_1, output_config->fint_config->fint_in_fs, output_config->fint_config->fint_out_fs);
        codec_0581_fint_select_chnl_route(output_config->fint_config->fint_chnl, output_config->fint_config->fint_route_from);
        codec_0581_fint_enable_chnl_power_on(output_config->fint_config->fint_chnl, CODEC_BOOL_TRUE);
    }

    /* dac */
    if (output_config->dac_config)
    {
        codec_0581_dac_set_sample_rate(output_config->dac_config->dac_rate);
        codec_0581_dac_select_input_route(output_config->dac_config->dac_input);
        codec_0581_dac_enable_power_on(CODEC_BOOL_TRUE);
        codec_0581_dac_enable_mute(CODEC_BOOL_FALSE);
    }
}

/**
 * @brief This function serves to initialize eq route and param
 * @param[in]  eq_config - the relevant configuration codec_0581_eq_config_t struct pointer
 * @return    none
 */
void codec_0581_eq_init(codec_0581_eq_config_t *eq_config)
{
    codec_0581_eq_select_route(eq_config->eq_route_from);
    codec_0581_eq_enable_run(CODEC_BOOL_FALSE);
    codec_0581_eq_sel_param_ram(eq_config->bank_id);
    codec_0581_eq_clear_param_ram();

    unsigned int bank_addr = eq_config->bank_id * (CODEC_EQ_PARAM1_MEM_BASE - CODEC_EQ_PARAM0_MEM_BASE) + CODEC_EQ_PARAM_MEM_BASE;
    unsigned char reg_length = 0;

    /* write program data */
    codec_0581_get_addr_length(CODEC_EQ_PROGRAM_MEM_BASE, &reg_length);
    for (unsigned int i = 0; i < eq_config->program_len; i++)
    {
        codec_0581_reg_write(CODEC_EQ_PROGRAM_MEM_BASE + i * reg_length, *(eq_config->program_addr)++);
    }

    /* write param data */
    codec_0581_get_addr_length(bank_addr, &reg_length);
    for (unsigned int j = 0; j < eq_config->param_len; j++)
    {
        codec_0581_reg_write(bank_addr + j * reg_length, *(eq_config->param_addr)++);
    }
    codec_0581_eq_enable_run(CODEC_BOOL_TRUE);
}

/**
 * @brief This function serves to initialize fdsp route and param
 * @param[in]  fdsp_config - the relevant configuration codec_0581_fdsp_config_t struct pointer
 * @return    none
 */
void codec_0581_fdsp_init(codec_0581_fdsp_config_t *fdsp_config)
{
    codec_0581_fdsp_enable_power_on(CODEC_BOOL_TRUE);
    codec_0581_fdsp_enable_run(CODEC_BOOL_FALSE);
    codec_0581_fdsp_select_param_bank(fdsp_config->bank_id);

    unsigned int bank_addr = fdsp_config->bank_id * (CODEC_FDSP_PARAM_BANKB_BASE - CODEC_FDSP_PARAM_BANKA_BASE) + CODEC_FDSP_PARAM_MEM_BASE;
    unsigned char reg_length = 0;

    /* write program data */
    codec_0581_get_addr_length(CODEC_FDSP_PROGRAM_MEM_BASE, &reg_length);
    for (unsigned int i = 0; i < fdsp_config->program_len; i++)
    {
        codec_0581_reg_write(CODEC_FDSP_PROGRAM_MEM_BASE + i * reg_length, *(fdsp_config->program_addr)++);
    }

    /* write param data */
    codec_0581_get_addr_length(bank_addr, &reg_length);
    for (int row = 0; row < (CODEC_FDSP_PARAM_BANKB_BASE - CODEC_FDSP_PARAM_BANKA_BASE) / CODEC_FDSP_PARAM_SPAN; row++)
    {
        for (unsigned int column = 0; column < fdsp_config->program_len; column++)
        {
            codec_0581_reg_write(bank_addr + CODEC_FDSP_PARAM_SPAN * row + column * reg_length, *(fdsp_config->param_addr)++);
        }
    }

    codec_0581_fdsp_enable_run(CODEC_BOOL_TRUE);
}


/************************** end of file *****************************/
