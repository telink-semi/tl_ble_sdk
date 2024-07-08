/********************************************************************************************************
 * @file    blt_led.h
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
#ifndef BLT_LED_H_
#define BLT_LED_H_

#include "tl_common.h"



#ifndef BLT_APP_LED_ENABLE
#define BLT_APP_LED_ENABLE                0
#endif


/**
 * @brief    Configure the parameters for led event
 */
typedef struct{
    unsigned short onTime_ms;
    unsigned short offTime_ms;

    unsigned char  repeatCount;  //0xff special for long on(offTime_ms=0)/long off(onTime_ms=0)
    unsigned char  priority;     //0x00 < 0x01 < 0x02 < 0x04 < 0x08 < 0x10 < 0x20 < 0x40 < 0x80
} led_cfg_t;

/**
 * @brief    the status of led event
 */
typedef struct {
    unsigned char  isOn;
    unsigned char  polar;
    unsigned char  repeatCount;
    unsigned char  priority;


    unsigned short onTime_ms;
    unsigned short offTime_ms;

    unsigned int gpio_led;
    unsigned int startTick;
}device_led_t;

extern device_led_t device_led;

#define  DEVICE_LED_BUSY    (device_led.repeatCount)

/**
 * @brief        This function is used to manage led tasks
 * @param[in]    none
 * @return      none
 */
extern void led_proc(void);

/**
 * @brief        This function is used to initialize device led setting
 * @param[in]    gpio - the GPIO corresponding to device led
 * @param[in]    polarity - 1 for high led on, 0 for low led on
 * @return      none
 */
extern void device_led_init(u32 gpio,u8 polarity);

/**
 * @brief        This function is used to create new led task
 * @param[in]    led_cfg - Configure the parameters for led event
 * @return      0 - new led event priority not higher than the not ongoing one
 *                 1 - new led event created successfully
 */
int device_led_setup(led_cfg_t led_cfg);

/**
 * @brief        This function is used to manage led tasks
 * @param[in]    none
 * @return      none
 */
static inline void device_led_process(void)
{
#if (BLT_APP_LED_ENABLE)
    if(DEVICE_LED_BUSY){
        led_proc();
    }
#endif
}




#endif /* BLT_LED_H_ */
