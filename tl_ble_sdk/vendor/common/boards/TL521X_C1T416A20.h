/********************************************************************************************************
 * @file    TL521X_C1T416A20.h
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
#ifndef VENDOR_COMMON_BOARDS_TL521X_C1T416A20_H_
#define VENDOR_COMMON_BOARDS_TL521X_C1T416A20_H_

/**
 *  @brief  Antenna Switch Configuration
 */
#ifndef ANTENNA_SWITCHING_AUTO_EN
    #define ANTENNA_SWITCHING_AUTO_EN 1
#endif

#ifndef NUM_ANT_SUPPORT
    #define NUM_ANT_SUPPORT 0x02
#endif

#ifndef MAX_ANT_PATHS_SUPPORT
    #define MAX_ANT_PATHS_SUPPORT 0X04
#endif

#if (ANTENNA_SWITCHING_AUTO_EN)
    #define ANTENNA_SWITCHING_SEL_0_PIN GPIO_PD3
    #define ANTENNA_SWITCHING_CTRL_BASE     0
#endif

#ifndef BLE_AUDIO_ENABLE
    #define BLE_AUDIO_ENABLE        0
#endif

/**
 *  @brief  Keyboard Configuration
 */
#if (UI_KEYBOARD_ENABLE)
    #define MATRIX_ROW_PULL    PM_PIN_PULLDOWN_100K
    #define MATRIX_COL_PULL    PM_PIN_PULLUP_20K

    #define KB_LINE_HIGH_VALID 0 //drive pin output 0 when scan key, scan pin read 0 is valid

    #if (BLE_AUDIO_ENABLE)
        #define VOICE              0xc0
        #define BTN_UNPAIR         0x02

        #define CR_VOL_UP          0xf0 ////
        #define CR_VOL_DN          0xf1

        /**
         *  @brief  Normal keyboard map
         */
        #define KB_MAP_NORMAL {     \
            {BTN_UNPAIR, VOICE }, \
            {CR_VOL_UP,  CR_VOL_DN}, \
        } 
    #else
        #define BTN_PAIR           0x01
        #define BTN_UNPAIR         0x02

        #define CR_VOL_UP          0xf0 ////
        #define CR_VOL_DN          0xf1

        /**
         *  @brief  Normal keyboard map
         */
        #define KB_MAP_NORMAL {     \
            {BTN_UNPAIR, BTN_PAIR }, \
            {CR_VOL_UP,  CR_VOL_DN}, \
        }
    #endif

    //////////////////// KEY CONFIG (EVK board) ///////////////////////////
    #define KB_DRIVE_PINS {GPIO_PC2, GPIO_PC3}
    #define KB_SCAN_PINS  {GPIO_PA1, GPIO_PB2}

    //scan pin as gpio
    #define PC2_FUNC AS_GPIO
    #define PC3_FUNC AS_GPIO

    //scan  pin need 10K pullup
    #define PULL_WAKEUP_SRC_PC2 MATRIX_ROW_PULL
    #define PULL_WAKEUP_SRC_PC3 MATRIX_ROW_PULL

    //scan pin open input to read gpio level
    #define PC2_INPUT_ENABLE 1
    #define PC3_INPUT_ENABLE 1

    //drive pin as gpio
    #define PA1_FUNC AS_GPIO
    #define PB2_FUNC AS_GPIO

    //drive pin need 100K pulldown
    #define PULL_WAKEUP_SRC_PA1 MATRIX_COL_PULL
    #define PULL_WAKEUP_SRC_PB2 MATRIX_COL_PULL

    //drive pin open input to read gpio wakeup level
    #define PA1_INPUT_ENABLE 1
    #define PB2_INPUT_ENABLE 1

#endif

/**
 *  @brief  LED Configuration
 */
#if UI_LED_ENABLE
    /**
     *  @brief  Definition gpio for led
     */
    #define GPIO_LED_BLUE     GPIO_PA4
    #define GPIO_LED_GREEN    GPIO_PA2
    #define GPIO_LED_WHITE    GPIO_PA0
    #define GPIO_LED_RED      GPIO_PA3

    #define PA4_FUNC          AS_GPIO
    #define PA2_FUNC          AS_GPIO
    #define PA0_FUNC          AS_GPIO
    #define PA3_FUNC          AS_GPIO

    #define PA4_OUTPUT_ENABLE 1
    #define PA2_OUTPUT_ENABLE 1
    #define PA0_OUTPUT_ENABLE 1
    #define PA3_OUTPUT_ENABLE 1

    #define LED_ON_LEVEL      1 //gpio output high voltage to turn on led

#endif

#ifndef JTAG_DEBUG_DISABLE
    #define JTAG_DEBUG_DISABLE 1
#endif
/**
 *  @brief  GPIO definition for JTAG
 */
#if (JTAG_DEBUG_DISABLE)
    //JTAG will cost some power
    #define PC4_FUNC            AS_GPIO
    #define PC5_FUNC            AS_GPIO
    #define PC6_FUNC            AS_GPIO
    #define PC7_FUNC            AS_GPIO

    #define PC4_INPUT_ENABLE    0
    #define PC5_INPUT_ENABLE    0
    #define PC6_INPUT_ENABLE    0
    #define PC7_INPUT_ENABLE    0

    #define PULL_WAKEUP_SRC_PC4 0
    #define PULL_WAKEUP_SRC_PC5 0
    #define PULL_WAKEUP_SRC_PC6 0
    #define PULL_WAKEUP_SRC_PC7 0

#endif

/**
 *  @brief  Battery_check Configuration
 */
#if (BATT_CHECK_ENABLE)
    #define VBAT_CHANNEL_EN 1

    #if VBAT_CHANNEL_EN
        /**     The battery voltage sample range is 1.8~3.5V    **/
    #else
        /**     if the battery voltage > 3.6V, should take some external voltage divider    **/
        #define ADC_INPUT_PIN_CHN_P     SD_ADC_GPIO_PB6P
        #define ADC_INPUT_PIN_CHN_N     0
    #endif
#endif

/**
 *  @brief  GPIO definition for debug_io
 */
#if (DEBUG_GPIO_ENABLE)
    #define GPIO_CHN0         GPIO_PA6
    #define GPIO_CHN1         GPIO_PA5
    #define GPIO_CHN2         GPIO_PD1
    #define GPIO_CHN3         GPIO_PD2
    #define GPIO_CHN4         GPIO_PD3
    #define GPIO_CHN5         GPIO_PD4
    #define GPIO_CHN6         GPIO_PD5
    #define GPIO_CHN7         GPIO_PD6

    #define GPIO_CHN8         GPIO_PD7
    #define GPIO_CHN9         GPIO_PB0
    #define GPIO_CHN10        GPIO_PD0
    #define GPIO_CHN11        GPIO_PE0
    #define GPIO_CHN12        GPIO_PE1
    #define GPIO_CHN13        GPIO_PB5
    #define GPIO_CHN14        GPIO_PB6
    #define GPIO_CHN15        GPIO_PB7

    #define PA6_OUTPUT_ENABLE 1
    #define PA5_OUTPUT_ENABLE 1
    #define PD1_OUTPUT_ENABLE 1
    #define PD2_OUTPUT_ENABLE 1
    #define PD3_OUTPUT_ENABLE 1
    #define PD4_OUTPUT_ENABLE 1
    #define PD5_OUTPUT_ENABLE 1
    #define PD6_OUTPUT_ENABLE 1

    #define PD7_OUTPUT_ENABLE 1
    #define PB0_OUTPUT_ENABLE 1
    #define PD0_OUTPUT_ENABLE 1
    #define PE0_OUTPUT_ENABLE 1
    #define PE1_OUTPUT_ENABLE 1
    #define PB5_OUTPUT_ENABLE 1
    #define PB6_OUTPUT_ENABLE 1
    #define PB7_OUTPUT_ENABLE 1
#endif //end of DEBUG_GPIO_ENABLE

#define TLKAPI_DEBUG_GPIO_PIN GPIO_PC7

/**
 *  @brief  AUDIO Configuration
 */
#if (BLE_AUDIO_ENABLE)
    #define GPIO_DMIC_DI                    GPIO_PD3
    #define GPIO_DMIC_CK                    GPIO_PD1
    #define GPIO_DMIC_CK2                   GPIO_PD2

    #define GPIO_AMIC_BIAS                  GPIO_PB1
    #define GPIO_AMIC_SP                    GPIO_PC0
    #define GPIO_AMIC_SN                    GPIO_PC1

    #define PB1_FUNC                        AS_GPIO
    #define PC0_FUNC                        AS_GPIO
    #define PC1_FUNC                        AS_GPIO
#endif

#endif /* VENDOR_COMMON_BOARDS_TL323X_C1T315115_H_ */
