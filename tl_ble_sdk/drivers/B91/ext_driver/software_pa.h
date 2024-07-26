/********************************************************************************************************
 * @file    software_pa.h
 *
 * @brief   This is the header file for BLE SDK
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
#ifndef BLT_PA_H_
#define BLT_PA_H_

#include "../gpio.h"


/**
 * @brief   software PA enable
 */
#ifndef PA_ENABLE
#define PA_ENABLE                           0
#endif


/**
 * @brief   GPIO to control PA TX EN, user should set it in app_config.h
 */
#ifndef PA_TXEN_PIN
#define PA_TXEN_PIN                         GPIO_PB2
#endif

/**
 * @brief   GPIO to control PA RX EN, user should set it in app_config.h
 */
#ifndef PA_RXEN_PIN
#define PA_RXEN_PIN                         GPIO_PB3
#endif


/**
 * @brief   PA control type
 */
#define PA_TYPE_OFF                         0
#define PA_TYPE_TX_ON                       1
#define PA_TYPE_RX_ON                       2


/**
 * @brief   software PA control Callback
 */
typedef void (*rf_pa_callback_t)(int type);
extern rf_pa_callback_t  blc_rf_pa_cb;



/**
 * @brief   RF software PA initialization
 * @param   none
 * @return  none
 */
void rf_pa_init(void);


#endif /* BLT_PA_H_ */
