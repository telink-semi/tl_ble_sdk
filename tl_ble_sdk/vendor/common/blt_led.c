/********************************************************************************************************
 * @file    blt_led.c
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
#include "blt_led.h"



device_led_t device_led;

/**
 * @brief        This function is used to control device led on or off
 * @param[in]    on - the status of led
 * @return      none
 */
void device_led_on_off(u8 on)
{
    gpio_write( device_led.gpio_led, on^device_led.polar );
    gpio_set_output_en(device_led.gpio_led, on);
    device_led.isOn = on;
}


/**
 * @brief        This function is used to initialize device led setting
 * @param[in]    gpio - the GPIO corresponding to device led
 * @param[in]    polarity - 1 for high led on, 0 for low led on
 * @return      none
 */
void device_led_init(u32 gpio,u8 polarity){  //polarity: 1 for high led on, 0 for low led on
    (void)gpio; (void)polarity;
#if (BLT_APP_LED_ENABLE)
    device_led.gpio_led = gpio;
    device_led.polar = !polarity;
    gpio_write( gpio, !polarity );
#endif
}

/**
 * @brief        This function is used to create new led task
 * @param[in]    led_cfg - Configure the parameters for led event
 * @return      0 - new led event priority not higher than the not ongoing one
 *                 1 - new led event created successfully
 */
int device_led_setup(led_cfg_t led_cfg)
{
    (void)led_cfg;
#if (BLT_APP_LED_ENABLE)
    if( device_led.repeatCount &&  device_led.priority >= led_cfg.priority){
        return 0; //new led event priority not higher than the not ongoing one
    }
    else{
        device_led.onTime_ms = led_cfg.onTime_ms;
        device_led.offTime_ms = led_cfg.offTime_ms;
        device_led.repeatCount = led_cfg.repeatCount;
        device_led.priority = led_cfg.priority;

        if(led_cfg.repeatCount == 0xff){ //for long on/long off
            device_led.repeatCount = 0;
        }
        else{ //process one of on/off Time is zero situation
            if(!device_led.onTime_ms){  //onTime is zero
                device_led.offTime_ms *= device_led.repeatCount;
                device_led.repeatCount = 1;
            }
            else if(!device_led.offTime_ms){
                device_led.onTime_ms *= device_led.repeatCount;
                device_led.repeatCount = 1;
            }
        }

        device_led.startTick = clock_time();
        device_led_on_off(device_led.onTime_ms ? 1 : 0);

        return 1;
    }
#else
    return 0;
#endif
}

/**
 * @brief        This function is used to manage led tasks
 * @param[in]    none
 * @return      none
 */
void led_proc(void)
{
#if (BLT_APP_LED_ENABLE)
    if(device_led.isOn){
        if(clock_time_exceed(device_led.startTick,device_led.onTime_ms*1000)){
            device_led_on_off(0);
            if(device_led.offTime_ms){ //offTime not zero
                device_led.startTick += device_led.onTime_ms*SYSTEM_TIMER_TICK_1MS;
            }
            else{
                device_led.repeatCount = 0;
            }
        }
    }
    else{
        if(clock_time_exceed(device_led.startTick,device_led.offTime_ms*1000)){
            if(--device_led.repeatCount){
                device_led_on_off(1);
                device_led.startTick += device_led.offTime_ms*SYSTEM_TIMER_TICK_1MS;
            }
        }
    }
#endif
}



