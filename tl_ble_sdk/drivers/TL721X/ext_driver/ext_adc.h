/********************************************************************************************************
 * @file    ext_adc.h
 *
 * @brief   This is the header file for BLE SDK
 *
 * @author  BLE GROUP
 * @date    06,2024
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

#ifndef DRIVERS_EXT_DRIVER_EXT_ADC_H_
#define DRIVERS_EXT_DRIVER_EXT_ADC_H_

#include "../driver.h"

/**********************************************************************************************************************
 *                                         Users can modify macros                                                    *
 *********************************************************************************************************************/


#define EXT_ADC_DMA_FUNC_EN                 0

#define EXT_ADC_GPIO_MODE_EN                1

#define EXT_ADC_VBAT_MODE_EN                1

#define EXT_ADC_TEMP_MODE_EN                0  //Customers can choose to enable or disable this module depending on the usage context
/**
 * @brief ADC working modes
 */
typedef enum {
    HAL_ADC_MODE_GPIO = 0,  // GPIO input mode (requires GPIO parameters in hw_priv)
    HAL_ADC_MODE_VBAT,      // Battery voltage sampling mode (no GPIO required)
    HAL_ADC_MODE_TEMP,      // Temperature sampling mode (no GPIO required)
    HAL_ADC_MODE_MAX        // Maximum mode value (for boundary checking)
} hal_adc_mode_e;

typedef struct {
    adc_input_pin_def_e input_p;
    unsigned short input_n;  //not used , for compatibility with the ADC general framework
} ext_adc_gpio_pin_cfg_t;

/**
 * @brief Common configuration for ADC (includes GPIO, sampling, and output parameters)
 * @note Passed to HAL layer via the `hw_priv` pointer in hal_adc_cfg_t
 */
typedef struct {

    adc_sample_freq_e sample_freq;

} hal_adc_common_cfg_t;


/**
 * @brief Main configuration struct for ADC HAL layer initialization
 */
typedef struct {
    hal_adc_mode_e                 adc_mode;    // ADC working mode (GPIO/VBAT/TEMP)
    dma_chn_e                      dma_chn;     // DMA channel (set to 0 for non-DMA mode)
    ext_adc_gpio_pin_cfg_t         gpio_cfg;    // GPIO configuration (set to 0 for VBAT/TEMP)
    void                           *hw_priv;    // Pointer to hardware extension parameters (points to hal_adc_common_cfg_t)
} hal_adc_cfg_t;


/**
 * @brief Return status for ADC HAL layer functions
 */
typedef enum {
    HAL_ADC_OK = 0,         // Operation successful
    HAL_ADC_ERR_PARAM,      // Parameter error (null pointer/invalid mode/missing required config)
    HAL_ADC_ERR_HW_INIT,    // Hardware initialization failed (returned by chip adaptation layer)
    HAL_ADC_ERR_MODE        // Mode not supported (e.g., hw_priv not configured for GPIO mode)
} hal_adc_status_e;


/**
 * @brief Initialize ADC HAL layer (external)
 * Initializes hardware and configs. Call first before other ADC functions.
 * @param adc_cfg [in] Pointer to ADC config struct
 * @return hal_adc_status_e Result (HAL_ADC_OK on success)
 */
hal_adc_status_e ext_adc_init(const hal_adc_cfg_t *adc_cfg);

/**
 * @brief Power on ADC (macro)
 * Activates ADC. Call after init. Uses SD_ADC_SAMPLE_MODE.
 */
#define ext_adc_power_on()     {adc_power_on(); }

/**
 * @brief Start ADC sampling (macro)
 * Triggers sampling. Ensure init and power-on done first.
 */
#define ext_adc_start()   (void)0

/**
 * @brief Stop ADC sampling (macro)
 * Stops sampling. ADC remains powered.
 */
#define ext_adc_stop()    (void)0


/**
 * @brief Read ADC data (external)
 * Returns processed sampling result. Call after sampling starts.
 * @return signed int Processed ADC value
 */
signed int ext_adc_read_data(void);


#endif /* DRIVERS_EXT_DRIVER_EXT_ADC_H_ */
