/********************************************************************************************************
 * @file    app_freertos.h
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
#ifndef VENDOR_APP_FREERTOS_H_
#define VENDOR_APP_FREERTOS_H_


/**
 * @brief        This function creates the os task
 * @param[in]    none.
 * @return        none.
 */
void app_TaskCreate(void);


/**
 * @brief        This function is used to send a semaphore and can be called inside an interrupt.
 * @param[in]    none.
 * @return        none.
 */
void os_give_sem_from_isr(void);


/**
 * @brief        This function is to send a semaphore
 * @param[in]    none.
 * @return        none.
 */
void os_give_sem(void);


#endif /* VENDOR_APP_FREERTOS_H_ */
