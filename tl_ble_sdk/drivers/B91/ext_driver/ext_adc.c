/********************************************************************************************************
 * @file    ext_adc.c
 *
 * @brief   This is the source file for BLE SDK
 *
 * @author  BLE GROUP
 * @date    06,2024
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
#include "../driver.h"
#include "ext_adc.h"

#ifndef ADC_DMA_CHN
#define ADC_DMA_CHN                DMA7
#endif

#define ADC_SAMPLE_NUM             8
#define ADC_SAMPLE_FREQ            ADC_SAMPLE_FREQ_96K
#define ADC_SAMPLE_NDMA_DELAY_TIME ((1000 / (6 * (2 << (ADC_SAMPLE_FREQ)))) + 1) //delay 2 sample cycle


unsigned short          adc_sample_buffer[ADC_SAMPLE_NUM] __attribute__((aligned(4))) = {0};

adc_sample_freq_e sample_freq = ADC_GPIO_PD1;
adc_input_pin_def_e input_p;
/**
 * @brief Global buffer for ADC sampling data (defined in the project)
 */


// -----------------------------------------------------------------------------
// Public HAL Layer Functions (External Interface)
// -----------------------------------------------------------------------------
/**
 * @brief ADC HAL layer initialization (external interface)
 * @param adc_cfg [in] Pointer to ADC main configuration struct
 * @return hal_adc_status_e Initialization result (HAL_ADC_OK if successful)
 */
hal_adc_status_e ext_adc_init(const hal_adc_cfg_t *adc_cfg) {

    // 1. Validate input parameters first
    // 1. Basic parameter check (null pointer/invalid mode)
      if (adc_cfg == NULL) {
          return HAL_ADC_ERR_PARAM;
      }

    // 2. Parse hw_priv (including GPIO config): Use defaults if not provided
    if(adc_cfg->adc_mode == HAL_ADC_MODE_GPIO)
    {
        input_p  = adc_cfg->gpio_cfg.input_p;
        //input_n
    }

    if (adc_cfg->hw_priv != NULL) {
         hal_adc_common_cfg_t *user_cfg = ( hal_adc_common_cfg_t *)adc_cfg->hw_priv;
         sample_freq = user_cfg->sample_freq;

    }


    //  Initialize ADC based on working mode
    switch (adc_cfg->adc_mode) {
        case HAL_ADC_MODE_GPIO: {
            // GPIO mode: Reuse legacy GPIO config (replaceable with s_adc_common_cfg for consistency)
            adc_gpio_sample_init(adc_cfg->gpio_cfg.input_p, ADC_VREF_1P2V, ADC_PRESCALE_1F4, sample_freq);
            break;
        }
        case HAL_ADC_MODE_VBAT: {
            // VBAT mode: Use config from s_adc_common_cfg
            adc_battery_voltage_sample_init();
            break;
        }
        #if EXT_ADC_TEMP_MODE_EN
        case HAL_ADC_MODE_TEMP: {
            // Temperature mode: Use default clock/downsample (replaceable with s_adc_common_cfg)
            adc_temperature_sample_init();
        }
        #endif
        default:
            return HAL_ADC_ERR_MODE; // Unsupported mode
    }



#if EXT_ADC_DMA_FUNC_EN
    adc_set_dma_config(ADC_DMA_CHN);
#endif



    return HAL_ADC_OK;
}


/**
 * @brief This function serves to sort adc sample code and get average value.
 * @return      adc_code_average    - the average value of adc sample code.
 */
unsigned short adc_sort_and_get_average_code(void)
{
    unsigned short adc_code_average = 0;
    int            i, j;
    unsigned short temp;
    /**** insert Sort and get average value ******/
    for (i = 1; i < ADC_SAMPLE_NUM; i++) {
        if (adc_sample_buffer[i] < adc_sample_buffer[i - 1]) {
            temp                 = adc_sample_buffer[i];
            adc_sample_buffer[i] = adc_sample_buffer[i - 1];
            /**
         * add judgment condition "j>=0" in for loop,
         * otherwise may have array out of bounds.
         * changed by chaofan.20201230.
     */
            for (j = i - 1; j >= 0 && adc_sample_buffer[j] > temp; j--) {
                adc_sample_buffer[j + 1] = adc_sample_buffer[j];
            }
            adc_sample_buffer[j + 1] = temp;
        }
    }

    //get average value from raw data(abandon 1/4 small and 1/4 big data)
    for (i = ADC_SAMPLE_NUM >> 2; i < (ADC_SAMPLE_NUM - (ADC_SAMPLE_NUM >> 2)); i++) {
        adc_code_average += adc_sample_buffer[i] / (ADC_SAMPLE_NUM >> 1);
    }
    return adc_code_average;
}

/**
 * @brief Read ADC sampling data (external interface)
 * @return signed int Processed ADC result
 */
signed int ext_adc_read_data(void) {
    unsigned short adc_vol_mv_average = 0;
    unsigned short adc_code_average   = 0;
    for (int i = 0; i < ADC_SAMPLE_NUM; i++) {
        /**
     * move the "2 sample cycle" wait operation before adc_get_code(),
     * otherwise may have data lose due to no waiting when adc_power_on.
     * changed by chaofan.20201230.
     */
        delay_us(ADC_SAMPLE_NDMA_DELAY_TIME); //wait at least 2 sample cycle(f = 96K, T = 10.4us)
        adc_sample_buffer[i] = adc_get_code();
    }
#if EXT_ADC_TEMP_MODE_EN
    adc_code_average = adc_sort_and_get_average_code();
    adc_temp_average = adc_calculate_temperature(adc_code_average);
#else
    adc_code_average   = adc_sort_and_get_average_code();
    adc_vol_mv_average = adc_calculate_voltage(adc_code_average);
#endif
    return adc_vol_mv_average;
}



