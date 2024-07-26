/********************************************************************************************************
 * @file    software_pa.c
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
#include "compiler.h"
#include "software_pa.h"
#include "../gpio.h"


_attribute_data_retention_sec_  rf_pa_callback_t  blc_rf_pa_cb = 0;

#if(PA_ENABLE)
_attribute_ram_code_
void app_rf_pa_handler(int type)
{
    if(type == PA_TYPE_TX_ON){
        gpio_set_low_level(PA_RXEN_PIN);
        gpio_set_high_level(PA_TXEN_PIN);
    }
    else if(type == PA_TYPE_RX_ON){
        gpio_set_low_level(PA_TXEN_PIN);
        gpio_set_high_level(PA_RXEN_PIN);
    }
    else{
        gpio_set_low_level(PA_RXEN_PIN);
        gpio_set_low_level(PA_TXEN_PIN);
    }
}
#endif


/**
 * @brief   RF software PA initialization
 * @param   none
 * @return  none
 */
void rf_pa_init(void)
{
#if(PA_ENABLE)
    gpio_function_en(PA_TXEN_PIN);
    gpio_input_dis(PA_TXEN_PIN);        //disable input
    gpio_output_en(PA_TXEN_PIN);         //enable output
    gpio_set_level(PA_TXEN_PIN, 0);

    gpio_function_en(PA_RXEN_PIN);
    gpio_input_dis(PA_RXEN_PIN);        //disable input
    gpio_output_en(PA_RXEN_PIN);         //enable output
    gpio_set_level(PA_RXEN_PIN, 0);
    blc_rf_pa_cb = app_rf_pa_handler;
#endif
}

