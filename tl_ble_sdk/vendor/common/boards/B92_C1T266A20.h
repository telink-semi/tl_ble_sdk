/********************************************************************************************************
 * @file    B92_C1T266A20.h
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

#ifndef B92_C1T266A20_H_
#define B92_C1T266A20_H_

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
    #define MATRIX_ROW_PULL    GPIO_PIN_PULLDOWN_100K
    #define MATRIX_COL_PULL    GPIO_PIN_PULLUP_10K

    #define KB_LINE_HIGH_VALID 0 //drive pin output 0 when scan key, scan pin read 0 is valid

    #define BTN_PAIR           0x01
    #define BTN_UNPAIR         0x02

    #define CR_VOL_UP          0xf0 ////
    #define CR_VOL_DN          0xf1

    /**
     *  @brief  Normal keyboard map
     */
    #define KB_MAP_NORMAL {      \
        {CR_VOL_DN, BTN_PAIR  },   \
        {CR_VOL_UP, BTN_UNPAIR}, \
}

    //////////////////// KEY CONFIG (EVK board) ///////////////////////////
    #define KB_DRIVE_PINS {GPIO_PD6, GPIO_PF6}
    #define KB_SCAN_PINS  {GPIO_PD7, GPIO_PD2}

    //scan pin as gpio
    #define PD6_FUNC AS_GPIO
    #define PF6_FUNC AS_GPIO

    //scan  pin need 10K pullup
    #define PULL_WAKEUP_SRC_PD6 MATRIX_ROW_PULL
    #define PULL_WAKEUP_SRC_PF6 MATRIX_ROW_PULL

    //scan pin open input to read gpio level
    #define PD6_INPUT_ENABLE 1
    #define PF6_INPUT_ENABLE 1

    //drive pin as gpio
    #define PD7_FUNC AS_GPIO
    #define PD2_FUNC AS_GPIO

    //drive pin need 100K pulldown
    #define PULL_WAKEUP_SRC_PD7 MATRIX_COL_PULL
    #define PULL_WAKEUP_SRC_PD2 MATRIX_COL_PULL

    //drive pin open input to read gpio wakeup level
    #define PD7_INPUT_ENABLE 1
    #define PD2_INPUT_ENABLE 1

#endif

/**
 *  @brief  LED Configuration
 */
#if UI_LED_ENABLE
    /**
     *  @brief  Definition gpio for led
     */
    #define GPIO_LED_BLUE     GPIO_PD0
    #define GPIO_LED_GREEN    GPIO_PD1
    #define GPIO_LED_WHITE    GPIO_PE6
    #define GPIO_LED_RED      GPIO_PE7

    #define PD0_FUNC          AS_GPIO
    #define PD1_FUNC          AS_GPIO
    #define PE6_FUNC          AS_GPIO
    #define PE7_FUNC          AS_GPIO

    #define PD0_OUTPUT_ENABLE 1
    #define PD1_OUTPUT_ENABLE 1
    #define PE6_OUTPUT_ENABLE 1
    #define PE7_OUTPUT_ENABLE 1

    #define LED_ON_LEVEL      1 //gpio output high voltage to turn on led

#endif



/**
 *  @brief  GPIO definition for debug_io
 */
#if (DEBUG_GPIO_ENABLE)
    #define GPIO_CHN0         GPIO_PA3
    #define GPIO_CHN1         GPIO_PA4
    #define GPIO_CHN2         GPIO_PB0
    #define GPIO_CHN3         GPIO_PB1
    #define GPIO_CHN4         GPIO_PB2
    #define GPIO_CHN5         GPIO_PB3
    #define GPIO_CHN6         GPIO_PB4
    #define GPIO_CHN7         GPIO_PB5
    #define GPIO_CHN8         GPIO_PB6
    #define GPIO_CHN9         GPIO_PB7
    #define GPIO_CHN10        GPIO_PC0
    #define GPIO_CHN11        GPIO_PC1
    #define GPIO_CHN12        GPIO_PC2



    #define PA3_OUTPUT_ENABLE 1
    #define PA4_OUTPUT_ENABLE 1
    #define PB0_OUTPUT_ENABLE 1
    #define PB1_OUTPUT_ENABLE 1
    #define PB3_OUTPUT_ENABLE 1
    #define PB4_OUTPUT_ENABLE 1
    #define PB5_OUTPUT_ENABLE 1
    #define PB6_OUTPUT_ENABLE 1
    #define PB7_OUTPUT_ENABLE 1
    #define PC0_OUTPUT_ENABLE 1
    #define PC1_OUTPUT_ENABLE 1
    #define PC2_OUTPUT_ENABLE 1

#endif //end of DEBUG_GPIO_ENABLE

/**
 *  @brief  Battery_check Configuration
 */
#if (BATT_CHECK_ENABLE)
    #define VBAT_CHANNEL_EN 0

    #if VBAT_CHANNEL_EN
        /**     The battery voltage sample range is 1.8~3.5V    **/
    #else
        /**     if the battery voltage > 3.6V, should take some external voltage divider    **/
        #define ADC_INPUT_PIN_CHN_P ADC_GPIO_PB1
        #define ADC_INPUT_PIN_CHN_N 0
    #endif
#endif

#endif /* B92_C1T266A20_H_ */
