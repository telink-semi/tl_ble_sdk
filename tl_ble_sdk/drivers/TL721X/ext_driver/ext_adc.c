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


adc_gpio_cfg_t adc_gpio_cfg_m =
{
    .v_ref       = ADC_VREF_1P2V,
    .pre_scale   = ADC_PRESCALE_1F4,
    .sample_freq = ADC_SAMPLE_FREQ_96K,
    .pin         = ADC_GPIO_PB0,
};


typedef enum
{
    ADC_VOLTAGE,
#if INTERNAL_TEST_FUNC_EN
    TEMP_VALUE,
#endif
} adc_result_type_e;


#define ADC_SAMPLE_GROUP_CNT 8 //Number of adc sample codes per channel.

#define ADC_SAMPLE_GROUP_CNT 8 //Number of adc sample codes per channel.

#define ADC_DMA_MODE         1
#define ADC_NDMA_MODE        2

#define ADC_MODE             ADC_NDMA_MODE

//In NDMA mode, only M channel can be used.
#define NDMA_M_1_CHN_EN 1
#define ADC_SAMPLE_CHN_CNT NDMA_M_1_CHN_EN


unsigned short adc_sort_and_get_average_code(unsigned short *channel_sample_buffer);
unsigned short adc_get_result(adc_transfer_mode_e transfer_mode, adc_sample_chn_e chn, adc_result_type_e result_type);
#if (ADC_MODE == ADC_DMA_MODE)
void adc_code_split_dma(unsigned short *sample_buffer, unsigned int sample_num, unsigned char chn_cnt, unsigned short buffers[chn_cnt][sample_num]);
#endif


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
        adc_gpio_cfg_m.pin  = adc_cfg->gpio_cfg.input_p;
        //input_n
    }

    if (adc_cfg->hw_priv != NULL) {
        adc_gpio_cfg_t *user_cfg = ( adc_gpio_cfg_t *)adc_cfg->hw_priv;
        adc_gpio_cfg_m.pre_scale = user_cfg->pre_scale;

        adc_gpio_cfg_m.v_ref = user_cfg->v_ref;

        adc_gpio_cfg_m.sample_freq = user_cfg->sample_freq;


    }

    //dma
    if(adc_cfg->dma_chn != 0xFF)
    {
        return HAL_ADC_ERR_MODE;
    }
    else
    {
        adc_init(NDMA_M_CHN);
    }

    //  Initialize ADC based on working mode
    switch (adc_cfg->adc_mode) {
        case HAL_ADC_MODE_GPIO: {
            // GPIO mode: Reuse legacy GPIO config (replaceable with s_adc_common_cfg for consistency)
            adc_gpio_sample_init(ADC_M_CHANNEL, adc_gpio_cfg_m);
            break;
        }
        case HAL_ADC_MODE_VBAT: {
            // VBAT mode: Use config from s_adc_common_cfg
            adc_vbat_sample_init(ADC_M_CHANNEL);
            break;
        }
        #if EXT_ADC_TEMP_MODE_EN
        case HAL_ADC_MODE_TEMP: {
            // Temperature mode: Use default clock/downsample (replaceable with s_adc_common_cfg)
            adc_temp_init(ADC_M_CHANNEL);
        }
        #endif
        default:
            return HAL_ADC_ERR_MODE; // Unsupported mode
    }



#if EXT_ADC_DMA_FUNC_EN
   //todo
#endif



    return HAL_ADC_OK;
}



/**
 * @brief This function serves to sort adc sample code and get average value.
 * @param[in]   channel_sample_buffer - This parameter is the first address of the received data buffer, which must be 4 bytes aligned, otherwise the program will enter an exception.
 *              and the actual buffer size defined by the user needs to be not smaller than the sample_num, otherwise there may be an out-of-bounds problem.
 * @return      adc_code_average    - the average value of adc sample code.
 */
unsigned short adc_sort_and_get_average_code(unsigned short *channel_sample_buffer)
{
    int            i, j;
    unsigned short adc_code_average = 0;
    unsigned short temp;

    /**** insert Sort and get average value ******/
    for (i = 1; i < ADC_SAMPLE_GROUP_CNT; i++) {
        if (channel_sample_buffer[i] < channel_sample_buffer[i - 1]) {
            temp                     = channel_sample_buffer[i];
            channel_sample_buffer[i] = channel_sample_buffer[i - 1];
            for (j = i - 1; j >= 0 && channel_sample_buffer[j] > temp; j--) {
                channel_sample_buffer[j + 1] = channel_sample_buffer[j];
            }
            channel_sample_buffer[j + 1] = temp;
        }
    }
    //get average value from raw data(abandon 1/4 small and 1/4 big data)
    for (i = ADC_SAMPLE_GROUP_CNT >> 2; i < (ADC_SAMPLE_GROUP_CNT - (ADC_SAMPLE_GROUP_CNT >> 2)); i++) {
        adc_code_average += channel_sample_buffer[i] / (ADC_SAMPLE_GROUP_CNT >> 1);
    }
    return adc_code_average;
}

/**
 * @brief      This function serves to convert to voltage value and temperature value.
 * @param[in]  transfer_mode -enum variable of adc code transfer mode.
 * @param[in]  chn - enum variable of ADC sample channel.
 * @param[in]  result_type - enum variable of result value
 * @return     adc_result   - adc voltage value or temperature value.
 */
unsigned short adc_get_result(adc_transfer_mode_e transfer_mode, adc_sample_chn_e chn, adc_result_type_e result_type)
{
    unsigned short code_average;
    unsigned short adc_result;
    unsigned int   cnt = 0;

    unsigned short channel_buffers[3][ADC_SAMPLE_GROUP_CNT] __attribute__((aligned(4)))                     = {0};

    if (transfer_mode == NDMA) {
        adc_start_sample_nodma();

        while (cnt < ADC_SAMPLE_GROUP_CNT) {
            int sample_cnt = adc_get_rxfifo_cnt();
            if (sample_cnt > 0) {

                channel_buffers[chn][cnt] = adc_get_raw_code();

                if (channel_buffers[chn][cnt] & BIT(11)) { //12 bit resolution, BIT(11) is sign bit, 1 means negative voltage in differential_mode
                    channel_buffers[chn][cnt] = 0;
                } else {
                    channel_buffers[chn][cnt] &= 0x7FF;    //BIT(10..0) is valid adc code
                }

                cnt++;

            }
        }
    }

    code_average = adc_sort_and_get_average_code(channel_buffers[chn]);

    if (result_type == ADC_VOLTAGE) {
        return adc_result = adc_calculate_voltage(chn, code_average);
    }
#if INTERNAL_TEST_FUNC_EN
    else if ((result_type == TEMP_VALUE)) {
        return adc_result = adc_calculate_temperature(code_average);
    }
#endif
    else {
        return 0;
    }
}
/**
 * @brief Read ADC sampling data (external interface)
 * @return signed int Processed ADC result
 */
signed int ext_adc_read_data(void) {
    signed int adc_vol_mv_average = adc_get_result(NDMA, ADC_M_CHANNEL, ADC_VOLTAGE);
    return adc_vol_mv_average;
}



