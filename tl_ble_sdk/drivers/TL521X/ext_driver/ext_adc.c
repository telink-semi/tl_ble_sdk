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

sd_adc_gpio_cfg_t sd_adc_gpio_cfg =
{
    .clk_freq           = SD_ADC_SAPMPLE_CLK_2M,
    .downsample_rate    = SD_ADC_DOWNSAMPLE_RATE_128,
    .gpio_div           = SD_ADC_GPIO_CHN_DIV_1F4,
    .input_p            = SD_ADC_GPIO_PB6P,
    .input_n            = SD_ADC_GNDN,
};


#define SD_ADC_SAMPLE_CNT  16

#define NDMA_POLLING_MODE        1
#define DMA_INTERRUPT_MODE       2

#define SAMPLE_MODE        NDMA_POLLING_MODE

/*
 *  The length of sd_adc_sample_buffer must be >= SD_ADC_FIFO_DEPTH, otherwise there is a risk of array overflow.
 */
signed int sd_adc_sample_buffer[SD_ADC_SAMPLE_CNT] __attribute__((aligned(4))) = {0};

/**
 * @brief ADC HAL layer initialization (external interface)
 * @param adc_cfg [in] Pointer to ADC main configuration struct
 * @return hal_adc_status_e Initialization result (HAL_ADC_OK if successful)
 */
hal_adc_status_e ext_adc_init(const hal_adc_cfg_t *adc_cfg) {

    // 1. Validate input parameters first
    // 1. Basic parameter check (null pointer/invalid mode)
      if (adc_cfg == NULL) {
          return HAL_SD_ADC_ERR_PARAM;
      }

    // 2. Parse hw_priv (including GPIO config): Use defaults if not provided
    if(adc_cfg->adc_mode == HAL_ADC_MODE_GPIO)
    {
        sd_adc_gpio_cfg.input_p  = adc_cfg->gpio_cfg.input_p;
    }

//    if (adc_cfg->hw_priv != NULL) {
//        sd_adc_gpio_cfg_t *user_cfg = ( sd_adc_gpio_cfg_t *)adc_cfg->hw_priv;
//        adc_gpio_cfg_m.pre_scale = user_cfg->pre_scale;
//
//        adc_gpio_cfg_m.v_ref = user_cfg->v_ref;
//
//        adc_gpio_cfg_m.sample_freq = user_cfg->sample_freq;
//
//
//    }

    //dma
    if(adc_cfg->dma_chn != 0xFF)
    {
        return HAL_SD_ADC_ERR_MODE;
    }
    else
    {
        sd_adc_init(SD_ADC_SINGLE_DC_MODE);
    }

    //  Initialize ADC based on working mode
    switch (adc_cfg->adc_mode) {
        case HAL_ADC_MODE_GPIO: {
            // GPIO mode: Reuse legacy GPIO config (replaceable with s_adc_common_cfg for consistency)
            sd_adc_gpio_sample_init(&sd_adc_gpio_cfg);
            break;
        }
        case HAL_ADC_MODE_VBAT: {
            // VBAT mode: Use config from s_adc_common_cfg
            sd_adc_vbat_sample_init(SD_ADC_SAPMPLE_CLK_2M, SD_ADC_VBAT_DIV_1F4, SD_ADC_DOWNSAMPLE_RATE_128);
            break;
        }
        #if EXT_ADC_TEMP_MODE_EN
        case HAL_SD_ADC_TEMP_MODE: {
            // Temperature mode: Use default clock/downsample (replaceable with s_adc_common_cfg)
            adc_temp_init(ADC_M_CHANNEL);
        }
        #endif
        default:
            return HAL_SD_ADC_ERR_MODE; // Unsupported mode
    }



#if EXT_ADC_DMA_FUNC_EN
   //todo
#endif



    return HAL_SD_ADC_OK;
}



/**
 * @brief       This function serves to sort and get average code.
 * @param[in]   sd_adc_data_buf -Pointer to sd_adc_data_buf
 * @return      average code
 * @note        If sd_adc_sort_and_get_average_code() interface is called, SD_ADC_SAMPLE_CNT must be a multiple of 4.
 */
signed int sd_adc_sort_and_get_average_code(signed int *sample_buffer)
{
    int i, j;
    signed int sd_adc_code_average = 0;
    signed int temp;

    /**** insert Sort and get average value ******/
    for(i = 1 ;i < SD_ADC_SAMPLE_CNT; i++)
    {
        if(sample_buffer[i] < sample_buffer[i-1])
        {
            temp = sample_buffer[i];
            sample_buffer[i] = sample_buffer[i-1];
            for(j=i-1; j>=0 && sample_buffer[j] > temp;j--)
            {
                sample_buffer[j+1] = sample_buffer[j];
            }
            sample_buffer[j+1] = temp;
        }
    }
    //get average value from raw data(abandon 1/4 small and 1/4 big data)
    for (i = SD_ADC_SAMPLE_CNT>>2; i < (SD_ADC_SAMPLE_CNT - (SD_ADC_SAMPLE_CNT>>2)); i++)
    {
        sd_adc_code_average += sample_buffer[i]/(SD_ADC_SAMPLE_CNT>>1);
    }
    return sd_adc_code_average;
}

/**
 * @brief       This function serves to get voltage or temperature value.
 * @param[in]   result_type -sd_adc_result_type_e
 * @return      voltage or temperature value
 */
signed int sd_adc_get_result(sd_adc_result_type_e result_type)
{
    signed int sd_adc_result;
    signed int code_average;
    while(1)
    {
#if(SAMPLE_MODE == NDMA_POLLING_MODE)
        int cnt = 0;
        while (cnt < SD_ADC_SAMPLE_CNT) {
            int sample_cnt = sd_adc_get_rxfifo_cnt();
            if (sample_cnt > 0) {
                sd_adc_sample_buffer[cnt] = sd_adc_get_raw_code();
                cnt++;
            }
        }
#elif(SAMPLE_MODE == DMA_INTERRUPT_MODE)
        /* Wait for DMA to finish if it was restarted by rescaling */
        while(sd_adc_rx_done_flag == 0);
#endif

        code_average = sd_adc_sort_and_get_average_code(sd_adc_sample_buffer);

        if(result_type == SD_ADC_VOLTAGE_10X_MV || result_type == SD_ADC_VOLTAGE_MV)
        {
            sd_adc_result = sd_adc_calculate_voltage(code_average, result_type);
            return sd_adc_result;
        }
    }
}
/**
 * @brief Read ADC sampling data (external interface)
 * @return signed int Processed ADC result
 */
signed int ext_adc_read_data(void) {
    signed int adc_vol_mv_average = sd_adc_get_result(SD_ADC_VOLTAGE_MV);
    return adc_vol_mv_average;
}



