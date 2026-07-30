/********************************************************************************************************
 * @file    TL721X_ML7218D1.h
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
#ifndef VENDOR_COMMON_BOARDS_TL721X_ML7218D1_H_
#define VENDOR_COMMON_BOARDS_TL721X_ML7218D1_H_

/**
 *  @brief  Antenna Switch Configuration
 */
#ifndef ANTENNA_SWITCHING_AUTO_EN
    #define ANTENNA_SWITCHING_AUTO_EN 0
#endif

#ifndef NUM_ANT_SUPPORT
    #define NUM_ANT_SUPPORT 0x01
#endif

#ifndef MAX_ANT_PATHS_SUPPORT
    #define MAX_ANT_PATHS_SUPPORT 0X02
#endif

/**
 *  @brief  Keyboard Configuration
 */
#if (UI_KEYBOARD_ENABLE)
#error "There is no Keyboard on AIOT_DK1"
#endif

/**
 *  @brief  Button Configuration
 */
#if (UI_BUTTON_ENABLE)
    //Schematic pin map
    //SW4 - TL_IO11 - PE3
    //SW5 - TL_IO21 - PF4
    //SW6 - TL_IO09 - PE5
    #define PE3_FUNC AS_GPIO        //SW4
    #define PE5_FUNC AS_GPIO        //SW6
    #define PF4_FUNC AS_GPIO        //SW5, not used in demo now.

    #define PULL_WAKEUP_SRC_PE3 GPIO_PIN_PULLUP_10K
    #define PULL_WAKEUP_SRC_PE5 GPIO_PIN_PULLUP_10K
    #define PULL_WAKEUP_SRC_PF4 GPIO_PIN_PULLUP_10K

    #define PE3_INPUT_ENABLE 1
    #define PE5_INPUT_ENABLE 1
    #define PF4_INPUT_ENABLE 1
#endif

/**
 *  @brief  LED Configuration
 */
#if UI_LED_ENABLE
    /**
     *  @brief  Definition gpio for led
     */

    #define GPIO_LED_GREEN    GPIO_PE2          //D2 - TL_IO12
    #define GPIO_LED_WHITE    GPIO_PE1          //D1 - TL_IO13
    #define GPIO_LED_RED      GPIO_LED_WHITE    //Reuse for no this LED on AIOT_DK1
    #define GPIO_LED_BLUE     GPIO_LED_GREEN    //Reuse for no this LED on AIOT_DK1

    #define PE2_FUNC          AS_GPIO
    #define PE1_FUNC          AS_GPIO

    #define PE2_OUTPUT_ENABLE 1
    #define PE1_OUTPUT_ENABLE 1

    #define LED_ON_LEVEL      1 //gpio output high voltage to turn on led

#endif


/**
 *  @brief  Battery_check Configuration
 */
#if (BATT_CHECK_ENABLE)
    #define VBAT_CHANNEL_EN 0

    #if VBAT_CHANNEL_EN
        /**     The battery voltage sample range is 1.8~3.5V    **/
    #else
        /**     if the battery voltage > 3.6V, should take some external voltage divider    **/
        #define ADC_INPUT_PIN_CHN_P ADC_GPIO_PB0
        #define ADC_INPUT_PIN_CHN_N 0
    #endif
#endif


/**
 *  @brief  GPIO definition for debug_io
 */
#if (DEBUG_GPIO_ENABLE)
    #define GPIO_CHN0         GPIO_PE0
    #define GPIO_CHN1         GPIO_PE1
    #define GPIO_CHN2         GPIO_PE2
    #define GPIO_CHN3         GPIO_PE3
    #define GPIO_CHN4         GPIO_PE4
    #define GPIO_CHN5         GPIO_PE5
    #define GPIO_CHN6         GPIO_PE6
    #define GPIO_CHN7         GPIO_PF3

    #define GPIO_CHN8         GPIO_PF4
    #define GPIO_CHN9         GPIO_PF6
    #define GPIO_CHN10        GPIO_PF7
    #define GPIO_CHN11        GPIO_PA0
    #define GPIO_CHN12        GPIO_PA1
    #define GPIO_CHN13        GPIO_PE7
    #define GPIO_CHN14        GPIO_PF0
    #define GPIO_CHN15        GPIO_PF1


    #define PE0_OUTPUT_ENABLE 1
    #define PE1_OUTPUT_ENABLE 1
    #define PE2_OUTPUT_ENABLE 1
    #define PE3_OUTPUT_ENABLE 1
    #define PE4_OUTPUT_ENABLE 1
    #define PE5_OUTPUT_ENABLE 1
    #define PE6_OUTPUT_ENABLE 1
    #define PF3_OUTPUT_ENABLE 1

    #define PF4_OUTPUT_ENABLE 1
    #define PF6_OUTPUT_ENABLE 1
    #define PF7_OUTPUT_ENABLE 1
    #define PA0_OUTPUT_ENABLE 1
    #define PA1_OUTPUT_ENABLE 1
    #define PE7_OUTPUT_ENABLE 1
    #define PF0_OUTPUT_ENABLE 1
    #define PF1_OUTPUT_ENABLE 1

#endif //end of DEBUG_GPIO_ENABLE

#define TLKAPI_DEBUG_GPIO_PIN GPIO_PB7

#endif /* VENDOR_COMMON_BOARDS_TL721X_ML7218D1_H_ */
