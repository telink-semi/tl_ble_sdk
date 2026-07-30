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

// Battery detection related global variables
_attribute_data_retention_ u8  lowBattDet_enable  = 1; // Battery detection enable flag
u8                             adc_hw_initialized = 0; //note: can not be retention variable
_attribute_data_retention_ signed int batt_vol_mv;     // Battery voltage value

/**
 * @brief      This function serves to enable battery detect.
 * @param[in]  en - enable battery detect
 * @return     none
 */
void battery_set_detect_enable(int en)
{
    lowBattDet_enable = en;

    if (!en) {
        adc_hw_initialized = 0; //// ADC needs re-initialization when disabled
    }
}

/**
 * @brief     This function serves to get battery detect status.
 * @param   none
 * @return    other: battery detect enable
 *            0: battery detect disable
 */
int battery_get_detect_enable(void)
{
    return lowBattDet_enable;
}

/**
 * @brief      this function is used for user to initialize battery detect.
 * @param      none
 * @return     none
 */
_attribute_ram_code_ void adc_bat_detect_init(void)
{
    hal_adc_cfg_t adc_cfg = {0};
#if VBAT_CHANNEL_EN
    adc_cfg.adc_mode = HAL_ADC_MODE_VBAT;  // Configure ADC to VBAT mode
    #if (MCU_CORE_TYPE == MCU_CORE_TL521X)
        adc_cfg.dma_chn  = 0xFF;
    #endif
#else
    adc_cfg.adc_mode = HAL_ADC_MODE_GPIO;  // Configure ADC to GPIO mode
    adc_cfg.dma_chn  = 0xFF;      // no used
    adc_cfg.gpio_cfg.input_p  = ADC_INPUT_PIN_CHN_P;
    adc_cfg.gpio_cfg.input_n  = ADC_INPUT_PIN_CHN_N;
    adc_cfg.hw_priv  = NULL;               // No hardware private data
#endif
    // Initialize external ADC
    ext_adc_init(&adc_cfg);

    // Power on ADC
    ext_adc_power_on();

    // Start ADC sampling
    ext_adc_start();

    /* wait at least 2 sample cycle(f = 96K, T = 10.4us),
     * Wait >30us after adc_power_on() for ADC to be stable.
     */
    #if (MCU_CORE_TYPE == MCU_CORE_TL721X && VBAT_CHANNEL_EN)
    sleep_us(50);
    #elif (MCU_CORE_TYPE == MCU_CORE_TL321X && VBAT_CHANNEL_EN)
    sleep_us(100);
    #else
    sleep_us(30); // Default minimum 30us delay
    #endif
}

/**
 * @brief       This is battery check function
 * @param[in]   alarm_vol_mv - input battery calibration
 * @return      0: batt_vol_mv < alarm_vol_mv 1: batt_vol_mv > alarm_vol_mv
 */
_attribute_ram_code_ int app_battery_power_check(u16 alarm_vol_mv)
{
    //when MCU powered up or wakeup from deep/deep with retention, adc need be initialized
    if (!adc_hw_initialized) {
        adc_hw_initialized = 1;   // Mark ADC as initialized
        adc_bat_detect_init();
        adc_hw_initialized = 1;  // Mark ADC as initialized
    }

    // Read current battery voltage
    batt_vol_mv = ext_adc_read_data();

    // Compare with alarm threshold and return result
    if (batt_vol_mv < alarm_vol_mv) {
        return 0;
    }
    return 1;
}


#endif //#if (BATT_CHECK_ENABLE)
