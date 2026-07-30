/********************************************************************************************************
 * @file    TL322X_C1T371A20.h
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
#ifndef VENDOR_COMMON_BOARDS_TL322X_C1T371A20_H_
#define VENDOR_COMMON_BOARDS_TL322X_C1T371A20_H_

#define N22_FW_DOWNLOAD_FLASH_ADDR  0x20080000
#define N22_FW_DOWNLOAD_NVM_ADDR    0x00500000
#define N22_IRAM_STARTUP_ADDR       0x50000000

/**
 * @brief  Default to D25F core if no MCU core is explicitly specified
 * (only takes effect when neither MCU_CORE_D25F_ENABLE nor MCU_CORE_N22_ENABLE is defined)
 */
#if  (!defined(MCU_CORE_D25F_ENABLE) && !defined(MCU_CORE_N22_ENABLE))
#define MCU_CORE_D25F_ENABLE        1
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
    #define KB_MAP_NORMAL {     \
        {CR_VOL_UP, CR_VOL_DN }, \
        {BTN_PAIR,  BTN_UNPAIR}, \
    }

    //////////////////// KEY CONFIG (EVK board) ///////////////////////////
    /* Description
     +--------------------------------------------------+
     |  Button  |   SW3   |   SW6   |   SW4   |   SW7   |
     +----------+---------+---------+---------+---------+
     | Function |   Vol+  |   Vol-  |   Pair  |  Unpair |
     +----------+---------+---------+---------+---------+
     */
    #define KB_DRIVE_PINS {GPIO_PG1, GPIO_PG0}
    #define KB_SCAN_PINS  {GPIO_PG3, GPIO_PG2}

    //drive pin as gpio
    #define PG1_FUNC AS_GPIO
    #define PG0_FUNC AS_GPIO

    //drive pin need 100K pulldown
    #define PULL_WAKEUP_SRC_PG1 MATRIX_ROW_PULL
    #define PULL_WAKEUP_SRC_PG0 MATRIX_ROW_PULL

    //drive pin open input to read gpio wakeup level
    #define PG1_INPUT_ENABLE 1
    #define PG0_INPUT_ENABLE 1

    //scan pin as gpio
    #define PG3_FUNC AS_GPIO
    #define PG2_FUNC AS_GPIO

    //scan  pin need 10K pullup
    #define PULL_WAKEUP_SRC_PG3 MATRIX_COL_PULL
    #define PULL_WAKEUP_SRC_PG2 MATRIX_COL_PULL

    //scan pin open input to read gpio level
    #define PG3_INPUT_ENABLE 1
    #define PG2_INPUT_ENABLE 1
#endif

/**
 *  @brief  LED Configuration
 */
#if UI_LED_ENABLE
    /**
     *  @brief  Definition gpio for led
     */
    #define GPIO_LED_RED      GPIO_PC4
    #define GPIO_LED_GREEN    GPIO_PC5
    #define GPIO_LED_BLUE     GPIO_PC7
    #define GPIO_LED_WHITE    GPIO_PC6


    #define PC4_FUNC          AS_GPIO
    #define PC5_FUNC          AS_GPIO
    #define PC6_FUNC          AS_GPIO
    #define PC7_FUNC          AS_GPIO

    #define PC4_OUTPUT_ENABLE 1
    #define PC5_OUTPUT_ENABLE 1
    #define PC6_OUTPUT_ENABLE 1
    #define PC7_OUTPUT_ENABLE 1

    #define LED_ON_LEVEL      1 //gpio output high voltage to turn on led
#endif

#ifndef JTAG_DEBUG_DISABLE
    #define JTAG_DEBUG_DISABLE 1
#endif
/**
 *  @brief  GPIO definition for JTAG
 */
#if (0) //(JTAG_DEBUG_DISABLE)
    //JTAG will cost some power
    #define PD4_FUNC            AS_GPIO
    #define PD5_FUNC            AS_GPIO
    #define PD6_FUNC            AS_GPIO
    #define PD7_FUNC            AS_GPIO

    #define PD4_INPUT_ENABLE    0
    #define PD5_INPUT_ENABLE    0
    #define PD6_INPUT_ENABLE    0
    #define PD7_INPUT_ENABLE    0

    #define PULL_WAKEUP_SRC_PD4 0
    #define PULL_WAKEUP_SRC_PD5 0
    #define PULL_WAKEUP_SRC_PD6 0
    #define PULL_WAKEUP_SRC_PD7 0
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
        #define ADC_INPUT_PIN_CHN_P SD_ADC_GPIO_PC1P
        #define ADC_INPUT_PIN_CHN_N SD_ADC_GNDN
    #endif
#endif


/**
 *  @brief  GPIO definition for debug_io
 */
#if (DEBUG_GPIO_ENABLE)
    #define GPIO_CHN0              GPIO_PE2
    #define GPIO_CHN1              GPIO_PE0
    #define GPIO_CHN2              GPIO_PH6
    #define GPIO_CHN3              GPIO_PH4
    #define GPIO_CHN4              GPIO_PH2
    #define GPIO_CHN5              GPIO_PH0
    #define GPIO_CHN6              GPIO_PG6
    #define GPIO_CHN7              GPIO_PG4

    #define GPIO_CHN8              GPIO_PE3
    #define GPIO_CHN9              GPIO_PE1
    #define GPIO_CHN10             GPIO_PH7
    #define GPIO_CHN11             GPIO_PH5
    #define GPIO_CHN12             GPIO_PH3
    #define GPIO_CHN13             GPIO_PH1
    #define GPIO_CHN14             GPIO_PG7
    #define GPIO_CHN15             GPIO_PG5

    #define PE2_OUTPUT_ENABLE      1
    #define PE0_OUTPUT_ENABLE      1
    #define PH6_OUTPUT_ENABLE      1
    #define PH4_OUTPUT_ENABLE      1
    #define PH2_OUTPUT_ENABLE      1
    #define PH0_OUTPUT_ENABLE      1
    #define PG6_OUTPUT_ENABLE      1
    #define PG4_OUTPUT_ENABLE      1

    #define PE3_OUTPUT_ENABLE      1
    #define PE1_OUTPUT_ENABLE      1
    #define PH7_OUTPUT_ENABLE      1
    #define PH5_OUTPUT_ENABLE      1
    #define PH3_OUTPUT_ENABLE      1
    #define PH1_OUTPUT_ENABLE      1
    #define PG7_OUTPUT_ENABLE      1
    #define PG5_OUTPUT_ENABLE      1
#endif //end of DEBUG_GPIO_ENABLE


#define TLKAPI_DEBUG_GPIO_PIN        GPIO_PA6
#define TLKAPI_DEBUG_GSUART_BAUDRATE 1000000


#ifndef DEBUG_GPIO_CHAN_ENABLE
    #define DEBUG_GPIO_CHAN_ENABLE 0
#endif


#if DEBUG_CS_GPIO_ENABLE
    #define GPIO_CHN0         GPIO_PD2
    #define GPIO_CHN1         GPIO_PD3
    #define GPIO_CHN2         GPIO_PF0
    #define GPIO_CHN3         GPIO_PF1
//    #define GPIO_CHN4         GPIO_PE2
//    #define GPIO_CHN5         GPIO_PF3
//    #define GPIO_CHN6         GPIO_PF0
//    #define GPIO_CHN7         GPIO_PF4
//
//    #define GPIO_CHN8         GPIO_PF5
//    #define GPIO_CHN9         GPIO_PE3
//    #define GPIO_CHN10        GPIO_PF6
//    #define GPIO_CHN11        GPIO_PE4
//    #define GPIO_CHN12        GPIO_PF7
//    #define GPIO_CHN13        GPIO_PE5
    #define GPIO_CHN14        GPIO_PD0
    #define GPIO_CHN15        GPIO_PD1


    #define PD2_OUTPUT_ENABLE 1
    #define PD3_OUTPUT_ENABLE 1
    #define PF0_OUTPUT_ENABLE 1
    #define PF1_OUTPUT_ENABLE 1
//    #define PE4_OUTPUT_ENABLE 1
//    #define PE5_OUTPUT_ENABLE 1
//    #define PE6_OUTPUT_ENABLE 1
//    #define PF0_OUTPUT_ENABLE 1
//
//    #define PF1_OUTPUT_ENABLE 1
//    #define PF2_OUTPUT_ENABLE 1
//    #define PF3_OUTPUT_ENABLE 1
//    #define PF4_OUTPUT_ENABLE 1
//    #define PF5_OUTPUT_ENABLE 1
//    #define PF6_OUTPUT_ENABLE 1
    #define PD0_OUTPUT_ENABLE 1
    #define PD1_OUTPUT_ENABLE 1

#endif

#if DEBUG_GPIO_CHAN_ENABLE
    #define gpio_write(pin, value) gpio_set_level(pin, value)

    #define GPIO_CHN0              GPIO_PC0
    #define GPIO_CHN1              GPIO_PC1
    #define GPIO_CHN2              GPIO_PC2
    #define GPIO_CHN3              GPIO_PC3
    #define GPIO_CHN4              GPIO_PD4
    #define GPIO_CHN5              GPIO_PD5
    #define GPIO_CHN6              GPIO_PD6
    #define GPIO_CHN7              GPIO_PD7

    #define GPIO_CHN8              GPIO_PH0
    #define GPIO_CHN9              GPIO_PH1
    #define GPIO_CHN10             GPIO_PH2
    #define GPIO_CHN11             GPIO_PH3
    #define GPIO_CHN12             GPIO_PH4
    #define GPIO_CHN13             GPIO_PH5
    #define GPIO_CHN14             GPIO_PH6
    #define GPIO_CHN15             GPIO_PH7

    #define PC0_OUTPUT_ENABLE      1
    #define PC1_OUTPUT_ENABLE      1
    #define PC2_OUTPUT_ENABLE      1
    #define PC3_OUTPUT_ENABLE      1
    #define PD4_OUTPUT_ENABLE      1
    #define PD5_OUTPUT_ENABLE      1
    #define PD6_OUTPUT_ENABLE      1
    #define PD7_OUTPUT_ENABLE      1

    #define PH0_OUTPUT_ENABLE      1
    #define PH1_OUTPUT_ENABLE      1
    #define PH2_OUTPUT_ENABLE      1
    #define PH3_OUTPUT_ENABLE      1
    #define PH4_OUTPUT_ENABLE      1
    #define PH5_OUTPUT_ENABLE      1
    #define PH6_OUTPUT_ENABLE      1
    #define PH7_OUTPUT_ENABLE      1

    #define DBG_CHN0_LOW           gpio_write(GPIO_CHN0, 0)
    #define DBG_CHN0_HIGH          gpio_write(GPIO_CHN0, 1)
    #define DBG_CHN0_TOGGLE        gpio_toggle(GPIO_CHN0)

    #define DBG_CHN1_LOW           gpio_write(GPIO_CHN1, 0)
    #define DBG_CHN1_HIGH          gpio_write(GPIO_CHN1, 1)
    #define DBG_CHN1_TOGGLE        gpio_toggle(GPIO_CHN1)

    #define DBG_CHN2_LOW           gpio_write(GPIO_CHN2, 0)
    #define DBG_CHN2_HIGH          gpio_write(GPIO_CHN2, 1)
    #define DBG_CHN2_TOGGLE        gpio_toggle(GPIO_CHN2)

    #define DBG_CHN3_LOW           gpio_write(GPIO_CHN3, 0)
    #define DBG_CHN3_HIGH          gpio_write(GPIO_CHN3, 1)
    #define DBG_CHN3_TOGGLE        gpio_toggle(GPIO_CHN3)

    #define DBG_CHN4_LOW           gpio_write(GPIO_CHN4, 0)
    #define DBG_CHN4_HIGH          gpio_write(GPIO_CHN4, 1)
    #define DBG_CHN4_TOGGLE        gpio_toggle(GPIO_CHN4)

    #define DBG_CHN5_LOW           gpio_write(GPIO_CHN5, 0)
    #define DBG_CHN5_HIGH          gpio_write(GPIO_CHN5, 1)
    #define DBG_CHN5_TOGGLE        gpio_toggle(GPIO_CHN5)

    #define DBG_CHN6_LOW           gpio_write(GPIO_CHN6, 0)
    #define DBG_CHN6_HIGH          gpio_write(GPIO_CHN6, 1)
    #define DBG_CHN6_TOGGLE        gpio_toggle(GPIO_CHN6)

    #define DBG_CHN7_LOW           gpio_write(GPIO_CHN7, 0)
    #define DBG_CHN7_HIGH          gpio_write(GPIO_CHN7, 1)
    #define DBG_CHN7_TOGGLE        gpio_toggle(GPIO_CHN7)

    #define DBG_CHN8_LOW           gpio_write(GPIO_CHN8, 0)
    #define DBG_CHN8_HIGH          gpio_write(GPIO_CHN8, 1)
    #define DBG_CHN8_TOGGLE        gpio_toggle(GPIO_CHN8)

    #define DBG_CHN9_LOW           gpio_write(GPIO_CHN9, 0)
    #define DBG_CHN9_HIGH          gpio_write(GPIO_CHN9, 1)
    #define DBG_CHN9_TOGGLE        gpio_toggle(GPIO_CHN9)

    #define DBG_CHN10_LOW          gpio_write(GPIO_CHN10, 0)
    #define DBG_CHN10_HIGH         gpio_write(GPIO_CHN10, 1)
    #define DBG_CHN10_TOGGLE       gpio_toggle(GPIO_CHN10)

    #define DBG_CHN11_LOW          gpio_write(GPIO_CHN11, 0)
    #define DBG_CHN11_HIGH         gpio_write(GPIO_CHN11, 1)
    #define DBG_CHN11_TOGGLE       gpio_toggle(GPIO_CHN11)

    #define DBG_CHN12_LOW          gpio_write(GPIO_CHN12, 0)
    #define DBG_CHN12_HIGH         gpio_write(GPIO_CHN12, 1)
    #define DBG_CHN12_TOGGLE       gpio_toggle(GPIO_CHN12)

    #define DBG_CHN13_LOW          gpio_write(GPIO_CHN13, 0)
    #define DBG_CHN13_HIGH         gpio_write(GPIO_CHN13, 1)
    #define DBG_CHN13_TOGGLE       gpio_toggle(GPIO_CHN13)

    #define DBG_CHN14_LOW          gpio_write(GPIO_CHN14, 0)
    #define DBG_CHN14_HIGH         gpio_write(GPIO_CHN14, 1)
    #define DBG_CHN14_TOGGLE       gpio_toggle(GPIO_CHN14)

    #define DBG_CHN15_LOW          gpio_write(GPIO_CHN15, 0)
    #define DBG_CHN15_HIGH         gpio_write(GPIO_CHN15, 1)
    #define DBG_CHN15_TOGGLE       gpio_toggle(GPIO_CHN15)
#endif /* DEBUG_GPIO_CHAN_ENABLE */

#endif /* VENDOR_COMMON_BOARDS_TL322X_C1T371A20_H_ */
