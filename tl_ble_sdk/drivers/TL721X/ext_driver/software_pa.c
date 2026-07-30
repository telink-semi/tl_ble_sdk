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
#include "common/types.h"
#include <stdint.h>

_attribute_data_retention_sec_  rf_pa_callback_t  blc_rf_pa_cb = 0;


typedef struct {
    u16 pin;
    u16 level[3];   // 3 different status  TX / RX / OFF
} pa_ctrl_t;

#if(PA_CHIP_KCT8207L)
static const pa_ctrl_t pa_table[] = {
    /* pin        TX RX OFF */
    {PA_CSD_PIN, {1, 1, 0}},
    {PA_CPS_PIN, {0, 0, 0}},
    {PA_CRX_PIN, {0, 1, 0}},
    {PA_CTX_PIN, {1, 0, 0}},
    {PA_CHL_PIN, {1, 0, 0}},
};
#else
#if(PA_ENABLE)
static const pa_ctrl_t pa_table[] = {
    /* pin         TX RX OFF */
    {PA_TXEN_PIN, {1, 0, 0}},
    {PA_RXEN_PIN, {0, 1, 0}},
};
#endif /* PA_ENABLE */
#endif

#if(PA_ENABLE)
#define PA_TABLE_SIZE (sizeof(pa_table)/sizeof(pa_table[0]))


static inline void pa_gpio_init(uint32_t pin)
{
    gpio_function_en(pin);
    gpio_input_dis(pin);
    gpio_output_en(pin);
    gpio_set_level(pin, 0);
}

_attribute_ram_code_sec_noinline_
static void app_rf_pa_handler(int type)
{
    u16 index;

    if(type == PA_TYPE_TX_ON)  
        index = 0;
    else if(type == PA_TYPE_RX_ON)
        index = 1;
    else
        index = 2;

    for(u16 i = 0; i < PA_TABLE_SIZE; i++){
        gpio_set_level(pa_table[i].pin, pa_table[i].level[index]);
    }
}

/**
 * @brief   RF software PA initialization
 * @param   none
 * @return  none
 */
void rf_pa_init(void)
{
    for(u16 i = 0; i < PA_TABLE_SIZE; i++){
        pa_gpio_init(pa_table[i].pin);
    }

    blc_rf_pa_cb = app_rf_pa_handler;
}
#endif /* PA_ENABLE */