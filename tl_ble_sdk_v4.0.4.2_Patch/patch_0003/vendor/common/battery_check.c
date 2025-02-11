/********************************************************************************************************
 * @file    battery_check.c
 *
 * @brief   This is the source file for BLE SDK
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
#include "tl_common.h"
#include "drivers.h"
#include "stack/ble/ble.h"

#include "battery_check.h"


#if (BATT_CHECK_ENABLE)

_attribute_data_retention_  u8      lowBattDet_enable = 1;
                            u8      adc_hw_initialized = 0;   //note: can not be retention variable
_attribute_data_retention_  u16     batt_vol_mv;


/**
 * @brief      This function serves to enable battery detect.
 * @param[in]  en - enable battery detect
 * @return     none
 */
void battery_set_detect_enable (int en)
{
    lowBattDet_enable = en;

    if(!en){
        adc_hw_initialized = 0;   //need initialized again
    }

}

/**
 * @brief     This function serves to get battery detect status.
 * @param   none
 * @return    other: battery detect enable
 *            0: battery detect disable
 */
int battery_get_detect_enable (void)
{
    return lowBattDet_enable;
}

extern unsigned short g_adc_vref;
extern unsigned short g_adc_gpio_calib_vref;
extern unsigned char g_adc_pre_scale;
extern unsigned char g_adc_vbat_divider;
extern signed char g_adc_vref_offset;
extern signed char g_adc_gpio_calib_vref_offset;
extern unsigned short g_adc_vbat_calib_vref;
extern signed char g_adc_vbat_calib_vref_offset;

/**
 * @brief      this function is used for user to initialize battery detect.
 * @param      none
 * @return     none
 */
_attribute_ram_code_ void adc_bat_detect_init(void)
{
#if (MCU_CORE_TYPE == MCU_CORE_B91)
    g_adc_vref = g_adc_gpio_calib_vref;//set gpio sample calib vref
    #if VBAT_CHANNEL_EN//vbat mode, vbat channel
        g_adc_vref_offset = 0;//Vbat has no two-point calibration, offset must be set to 0.
    #else
        g_adc_vref_offset = g_adc_gpio_calib_vref_offset;//set adc_vref_offset as adc_gpio_calib_vref_offset
    #endif
    /******power off sar adc********/
    adc_power_off();

    //reset whole digital adc module
    adc_reset();

    /******set adc sample clk as 4MHz******/
    adc_clk_en();//enable signal of 24M clock to sar adc
    adc_set_clk(5);//default adc_clk 4M = 24M/(1+div),

    //set misc channel vref 1.2V
    analog_write_reg8(areg_adc_vref, ADC_VREF_1P2V);
    analog_write_reg8(areg_ain_scale  , (analog_read_reg8( areg_ain_scale  )&(0xC0)) | 0x3d );
    g_adc_vref = 1175;

    //set Analog input pre-scaling,ADC_PRESCALE_1F4
    analog_write_reg8( areg_ain_scale  , (analog_read_reg8( areg_ain_scale  )&(~FLD_SEL_AIN_SCALE)) | (ADC_PRESCALE_1F4<<6) );
    g_adc_pre_scale = 1<<(unsigned char)ADC_PRESCALE_1F4;

    //set sample frequency.96k
    adc_set_state_length(240, 10);

    //set misc t_sample 6 cycle of adc clock:  6 * 1/4M
    adc_set_tsample_cycle(ADC_SAMPLE_CYC_6);

    //default adc_resolution set as 14bit ,BIT(13) is sign bit
    adc_set_resolution(ADC_RES14);

    //enable adc channel.
    adc_set_m_chn_en();

    //set vbat divider : ADC_VBAT_DIV_OFF
    analog_write_reg8(areg_adc_vref_vbat_div, (analog_read_reg8(areg_adc_vref_vbat_div)&(~FLD_ADC_VREF_VBAT_DIV)) | (ADC_VBAT_DIV_OFF<<2) );
    g_adc_vbat_divider = 1;

    #if VBAT_CHANNEL_EN//vbat mode, vbat channel
        adc_set_diff_input(ADC_VBAT, GND);
    #else//base mode, gpio channel
        adc_set_diff_input(ADC_INPUT_PIN_CHN>>12, GND);
    #endif
#elif (MCU_CORE_TYPE == MCU_CORE_B92)
    #if VBAT_CHANNEL_EN//vbat mode, vbat channel
        g_adc_vref = g_adc_vbat_calib_vref;//set vbat sample calib vref
        g_adc_vref_offset = g_adc_vbat_calib_vref_offset;
        adc_init(ADC_VREF_1P2V, ADC_PRESCALE_1, ADC_SAMPLE_FREQ_96K);
        adc_set_vbat_divider(ADC_VBAT_DIV_1F4);
        adc_set_diff_input(ADC_VBAT, GND);
    #else//base mode, gpio channel
        g_adc_vref = g_adc_gpio_calib_vref;//set gpio sample calib vref
        g_adc_vref_offset = g_adc_gpio_calib_vref_offset;//set adc_vref_offset as adc_gpio_calib_vref_offset
        adc_init(ADC_VREF_1P2V, ADC_PRESCALE_1F4, ADC_SAMPLE_FREQ_96K);
        adc_set_vbat_divider(ADC_VBAT_DIV_OFF);
        adc_pin_config(ADC_GPIO_MODE, ADC_INPUT_PIN_CHN);
        adc_set_diff_input(ADC_INPUT_PIN_CHN >> 12, GND);
    #endif
#elif (MCU_CORE_TYPE == MCU_CORE_TL721X || MCU_CORE_TYPE == MCU_CORE_TL321X)
     adc_init(NDMA_M_CHN);
#if VBAT_CHANNEL_EN//vbat mode, vbat channel
     adc_vbat_sample_init(ADC_M_CHANNEL);
#else//base mode, gpio channel
     adc_gpio_cfg_t adc_gpio_cfg_m =
     {
            .v_ref = ADC_VREF_GPIO_1P2V,
        #if (MCU_CORE_TYPE == MCU_CORE_TL721X)
            .pre_scale = ADC_PRESCALE_1F4,
        #elif (MCU_CORE_TYPE == MCU_CORE_TL321X)
            .pre_scale = ADC_PRESCALE_1F4,
        #endif
            .sample_freq = ADC_SAMPLE_FREQ_96K,
            .pin = ADC_INPUT_PIN_CHN,
     };
     adc_gpio_sample_init(ADC_M_CHANNEL, adc_gpio_cfg_m);
#endif /*!< END OF VBAT_CHANNEL_EN */

#endif
    /******power on sar adc********/
    //note: this setting must be set after all other settings
    adc_power_on();

    /* wait at least 2 sample cycle(f = 96K, T = 10.4us),
     * Wait >30us after adc_power_on() for ADC to be stable.
     */
#if(MCU_CORE_TYPE == MCU_CORE_TL721X && VBAT_CHANNEL_EN)
    sleep_us(50);
#elif(MCU_CORE_TYPE == MCU_CORE_TL321X && VBAT_CHANNEL_EN)
    sleep_us(100);
#else
    sleep_us(30);
#endif
}

/**
 * @brief This function serves to sort adc sample code and get average value.
 * @param[in]   channel_sample_buffer - This parameter is the first address of the received data buffer, which must be 4 bytes aligned, otherwise the program will enter an exception.
 *              and the actual buffer size defined by the user needs to be not smaller than the sample_num, otherwise there may be an out-of-bounds problem.
 * @return      adc_code_average    - the average value of adc sample code.
 */
_attribute_ram_code_ static unsigned short adc_sort_and_get_average_code(unsigned short *channel_sample_buffer)
{
    int i, j;
    unsigned short adc_code_average = 0;
    unsigned short temp;

    /**** insert Sort and get average value ******/
    for(i = 1 ;i < 8; i++)
    {
        if(channel_sample_buffer[i] < channel_sample_buffer[i-1])
        {
            temp = channel_sample_buffer[i];
            channel_sample_buffer[i] = channel_sample_buffer[i-1];
            for(j=i-1; j>=0 && channel_sample_buffer[j] > temp;j--)
            {
                channel_sample_buffer[j+1] = channel_sample_buffer[j];
            }
            channel_sample_buffer[j+1] = temp;
        }
    }
    //get average value from raw data(abandon 1/4 small and 1/4 big data)
    for (i = 8>>2; i < (8 - (8>>2)); i++)
    {
        adc_code_average += channel_sample_buffer[i]/(8>>1);
    }
    return adc_code_average;
}

/**
 * @brief       This is battery check function
 * @param[in]   alarm_vol_mv - input battery calibration
 * @return      0: batt_vol_mv < alarm_vol_mv 1: batt_vol_mv > alarm_vol_mv
 */
_attribute_ram_code_ int app_battery_power_check(u16 alarm_vol_mv)
{


    //when MCU powered up or wakeup from deep/deep with retention, adc need be initialized
    if(!adc_hw_initialized){
        adc_hw_initialized = 1;
        adc_bat_detect_init();
    }
    //Note:25us should be reserved between each reading(wait at least 2 sample cycle(f = 96K, T = 10.4us)).
    //The sdk is only sampled once, and the user can redesign the filtering algorithm according to the actual application.
    unsigned short adc_misc_data;
    u32 adc_average=0;
#if ((MCU_CORE_TYPE == MCU_CORE_B91) || (MCU_CORE_TYPE == MCU_CORE_B92) )
#if DCDC_ADC_SOFTWARE_FILTER
    u8 adc_sample_num=6;
    for(int i=0;i<adc_sample_num;i++)
    {
        u8 ana_read_f3 = analog_read_reg8(areg_adc_data_sample_control);
        analog_write_reg8(areg_adc_data_sample_control, ana_read_f3 | FLD_NOT_SAMPLE_ADC_DATA);
        adc_misc_data = analog_read_reg16(areg_adc_misc_l);
        analog_write_reg8(areg_adc_data_sample_control, ana_read_f3 & (~FLD_NOT_SAMPLE_ADC_DATA));
        if(adc_misc_data & BIT(13)){
            adc_misc_data=0;
            return 1;
        }
        else
        {
            adc_misc_data &= 0x1FFF;
        }
        adc_average +=adc_misc_data;
    }
    adc_average = adc_average/adc_sample_num;
#else
    analog_write_reg8(areg_adc_data_sample_control, analog_read_reg8(areg_adc_data_sample_control) | FLD_NOT_SAMPLE_ADC_DATA);
    adc_misc_data = analog_read_reg16(areg_adc_misc_l);
    analog_write_reg8(areg_adc_data_sample_control, analog_read_reg8(areg_adc_data_sample_control) & (~FLD_NOT_SAMPLE_ADC_DATA));


    if(adc_misc_data & BIT(13)){
        adc_misc_data=0;
        return 1;
    }
    else{
        adc_misc_data &= 0x1FFF;
    }

    adc_average = adc_misc_data;
#endif  //#if DCDC_ADC_SOFTWARE_FILTER
////////////////// adc sample data convert to voltage(mv) ////////////////
#if (MCU_CORE_TYPE == MCU_CORE_B91)
    batt_vol_mv  = (((adc_average * g_adc_vbat_divider * g_adc_pre_scale * g_adc_vref)>>13) + g_adc_vref_offset);
#elif (MCU_CORE_TYPE == MCU_CORE_B92)
    batt_vol_mv  = (((adc_average * g_adc_vbat_divider * g_adc_pre_scale * g_adc_vref)>>13) + g_adc_vref_offset);
#endif


#elif((MCU_CORE_TYPE == MCU_CORE_TL721X) || (MCU_CORE_TYPE == MCU_CORE_TL321X))

    unsigned short code_average;
    unsigned int cnt = 0;
    __attribute__((aligned(4))) unsigned short channel_buffers[8] = {0};

    adc_start_sample_nodma();
    while(cnt < 8)
    {
        if(adc_get_rxfifo_cnt() <= 0)   continue;
        channel_buffers[cnt]= adc_get_code();
        if(channel_buffers[cnt] & BIT(11)){ //12 bit resolution, BIT(11) is sign bit, 1 means negative voltage in differential_mode
            channel_buffers[cnt]=0;
        }
        else{
            channel_buffers[cnt] &= 0x7FF;  //BIT(10..0) is valid adc code
        }
        cnt++;
    }

    code_average = adc_sort_and_get_average_code(channel_buffers);
    batt_vol_mv = adc_calculate_voltage(ADC_M_CHANNEL, code_average);
#endif

    if(batt_vol_mv < alarm_vol_mv){
        return 0;
    }
    return 1;
}


#endif //#if (BATT_CHECK_ENABLE)
