/********************************************************************************************************
 * @file    gpio.h
 *
 * @brief   This is the header file for tl322x
 *
 * @author  Driver Group
 * @date    2024
 *
 * @par     Copyright (c) 2024, Telink Semiconductor (Shanghai) Co., Ltd. ("TELINK")
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
/** @page GPIO
 *
 *  Introduction
 *  ===============
 *  B92 contain two six group gpio(A~F), total 44 gpio pin.
 *
 *  API Reference
 *  ===============
 *  Header File: gpio.h
 */
#ifndef DRIVERS_GPIO_H_
#define DRIVERS_GPIO_H_


#include "lib/include/plic.h"
#include "lib/include/analog.h"
#include "reg_include/gpio_reg.h"
#include "pem.h"

/**********************************************************************************************************************
 *                                         global constants                                                           *
 *********************************************************************************************************************/

/**********************************************************************************************************************
 *                                           global macro                                                             *
 *********************************************************************************************************************/


/**********************************************************************************************************************
 *                                         global data type                                                           *
 *********************************************************************************************************************/
/**
 *  @brief  Define GPIO group types
 */
typedef enum
{
    GPIO_GROUP_A = 0,
    GPIO_GROUP_B = 1,
    GPIO_GROUP_C = 2,
    GPIO_GROUP_D = 3,
    GPIO_GROUP_E = 4,
    GPIO_GROUP_F = 5,
    GPIO_GROUP_G = 6,
    GPIO_GROUP_H = 7,
    GPIO_GROUP_I = 8,
} gpio_group_e;

/**
 *  @brief  Define GPIO types
 *  @note  the following two points need to noticed when using GPIOD and GPIOI groups:
 *          1. GPIOD groups pin is not recommend to use as wake-up source;
 *          2. Since the GPIOI groups are used for connecting the flash and have been occupied, they cannot be used as wake-up pins.
 */
typedef enum
{
    GPIO_GROUPA = 0x000,
    GPIO_GROUPB = 0x100,
    GPIO_GROUPC = 0x200,
    GPIO_GROUPD = 0x300,
    GPIO_GROUPE = 0x400,
    GPIO_GROUPF = 0x500,
    GPIO_GROUPG = 0x600,
    GPIO_GROUPH = 0x700,
    GPIO_GROUPI = 0x800,
    GPIO_ALL    = 0X900,

    GPIO_PA0  = GPIO_GROUPA | BIT(0),
    GPIO_PA1  = GPIO_GROUPA | BIT(1),
    GPIO_PA2  = GPIO_GROUPA | BIT(2),
    GPIO_PA3  = GPIO_GROUPA | BIT(3),//Only supports USB function
    GPIO_PA4  = GPIO_GROUPA | BIT(4),//Only supports USB function
    GPIO_PA5  = GPIO_GROUPA | BIT(5),
    GPIO_DM   = GPIO_PA5,
    GPIO_PA6  = GPIO_GROUPA | BIT(6),
    GPIO_DP   = GPIO_PA6,
    GPIO_PA7  = GPIO_GROUPA | BIT(7),
    GPIO_SWS  = GPIO_PA7,
    GPIOA_ALL = GPIO_GROUPA | 0x00ff,

    GPIO_PB0  = GPIO_GROUPB | BIT(0),
    GPIO_PB1  = GPIO_GROUPB | BIT(1),
    GPIO_PB2  = GPIO_GROUPB | BIT(2),
    GPIO_PB3  = GPIO_GROUPB | BIT(3),
    GPIO_PB4  = GPIO_GROUPB | BIT(4),
    GPIO_PB5  = GPIO_GROUPB | BIT(5),
    GPIO_PB6  = GPIO_GROUPB | BIT(6),
    GPIO_PB7  = GPIO_GROUPB | BIT(7),
    GPIOB_ALL = GPIO_GROUPB | 0x00ff,

    GPIO_PC0  = GPIO_GROUPC | BIT(0),
    GPIO_PC1  = GPIO_GROUPC | BIT(1),
    GPIO_PC2  = GPIO_GROUPC | BIT(2),
    GPIO_PC3  = GPIO_GROUPC | BIT(3),
    GPIO_PC4  = GPIO_GROUPC | BIT(4),
    GPIO_PC5  = GPIO_GROUPC | BIT(5),
    GPIO_PC6  = GPIO_GROUPC | BIT(6),
    GPIO_PC7  = GPIO_GROUPC | BIT(7),
    GPIOC_ALL = GPIO_GROUPC | 0x00ff,

    GPIO_PD0  = GPIO_GROUPD | BIT(0),
    GPIO_PD1  = GPIO_GROUPD | BIT(1),
    GPIO_PD2  = GPIO_GROUPD | BIT(2),
    GPIO_PD3  = GPIO_GROUPD | BIT(3),
    GPIO_PD4  = GPIO_GROUPD | BIT(4),
    GPIO_PD5  = GPIO_GROUPD | BIT(5),
    GPIO_PD6  = GPIO_GROUPD | BIT(6),
    GPIO_PD7  = GPIO_GROUPD | BIT(7),
    GPIOD_ALL = GPIO_GROUPD | 0x00ff,

    GPIO_PE0  = GPIO_GROUPE | BIT(0),
    GPIO_PE1  = GPIO_GROUPE | BIT(1),
    GPIO_PE2  = GPIO_GROUPE | BIT(2),
    GPIO_PE3  = GPIO_GROUPE | BIT(3),
    GPIO_PE4  = GPIO_GROUPE | BIT(4),
    GPIO_PE5  = GPIO_GROUPE | BIT(5),
    GPIO_PE6  = GPIO_GROUPE | BIT(6),
    GPIO_PE7  = GPIO_GROUPE | BIT(7),
    GPIOE_ALL = GPIO_GROUPE | 0x00ff,

    GPIO_PF0  = GPIO_GROUPF | BIT(0),
    GPIO_PF1  = GPIO_GROUPF | BIT(1),
    GPIO_PF2  = GPIO_GROUPF | BIT(2),
    GPIO_PF3  = GPIO_GROUPF | BIT(3),
    GPIO_PF4  = GPIO_GROUPF | BIT(4),
    GPIO_PF5  = GPIO_GROUPF | BIT(5),
    GPIO_PF6  = GPIO_GROUPF | BIT(6),
    GPIO_PF7  = GPIO_GROUPF | BIT(7),
    GPIOF_ALL = GPIO_GROUPF | 0x00ff,

    GPIO_PG0  = GPIO_GROUPG | BIT(0),
    GPIO_PG1  = GPIO_GROUPG | BIT(1),
    GPIO_PG2  = GPIO_GROUPG | BIT(2),
    GPIO_PG3  = GPIO_GROUPG | BIT(3),
    GPIO_PG4  = GPIO_GROUPG | BIT(4),
    GPIO_PG5  = GPIO_GROUPG | BIT(5),
    GPIO_PG6  = GPIO_GROUPG | BIT(6),
    GPIO_PG7  = GPIO_GROUPG | BIT(7),
    GPIOG_ALL = GPIO_GROUPG | 0x00ff,

    GPIO_PH0  = GPIO_GROUPH | BIT(0),
    GPIO_PH1  = GPIO_GROUPH | BIT(1),
    GPIO_PH2  = GPIO_GROUPH | BIT(2),
    GPIO_PH3  = GPIO_GROUPH | BIT(3),
    GPIO_PH4  = GPIO_GROUPH | BIT(4),
    GPIO_PH5  = GPIO_GROUPH | BIT(5),
    GPIO_PH6  = GPIO_GROUPH | BIT(6),
    GPIO_PH7  = GPIO_GROUPH | BIT(7),
    GPIOH_ALL = GPIO_GROUPH | 0x00ff,

    GPIO_PI0  = GPIO_GROUPI | BIT(0),
    GPIO_PI1  = GPIO_GROUPI | BIT(1),
    GPIO_PI2  = GPIO_GROUPI | BIT(2),
    GPIO_PI3  = GPIO_GROUPI | BIT(3),
    GPIO_PI4  = GPIO_GROUPI | BIT(4),
    GPIO_PI5  = GPIO_GROUPI | BIT(5),
    GPIOI_ALL = GPIO_GROUPI | 0x00ff,
} gpio_pin_e;

/**
 *  @brief  Define GPIO function pin types.
 *  @note   Each GPIO can only output the PWM listed; those not listed cannot be output.PA3 to PA7 cannot be used as PWM outputs.
 PA[0] : PWM20_N PWM16_N PWM12_N PWM8_N PWM4_N PWM0_N PWM20 PWM16 PWM12 PWM8  PWM4 PWM0
 PA[1] : PWM21_N PWM17_N PWM13_N PWM9_N PWM5_N PWM1_N PWM21 PWM17 PWM13 PWM9  PWM5 PWM1
 PA[2] : PWM22_N PWM18_N PWM14_N PWM10_N PWM6_N PWM2_N PWM22 PWM18 PWM14 PWM10 PWM6 PWM2
 PB[0] : PWM21_N PWM17_N PWM13_N PWM9_N PWM5_N PWM1_N PWM21 PWM17 PWM13 PWM9  PWM5 PWM1
 PB[1] : PWM22_N PWM18_N PWM14_N PWM10_N PWM6_N PWM2_N PWM22 PWM18 PWM14 PWM10 PWM6 PWM2
 PB[2] : PWM23_N PWM19_N PWM15_N PWM11_N PWM7_N PWM3_N PWM23 PWM19 PWM15 PWM11 PWM7 PWM3
 PB[3] : PWM20_N PWM16_N PWM12_N PWM8_N PWM4_N PWM0_N PWM20 PWM16 PWM12 PWM8  PWM4 PWM0
 PB[4] : PWM21_N PWM17_N PWM13_N PWM9_N PWM5_N PWM1_N PWM21 PWM17 PWM13 PWM9  PWM5 PWM1
 PB[5] : PWM22_N PWM18_N PWM14_N PWM10_N PWM6_N PWM2_N PWM22 PWM18 PWM14 PWM10 PWM6 PWM2
 PB[6] : PWM23_N PWM19_N PWM15_N PWM11_N PWM7_N PWM3_N PWM23 PWM19 PWM15 PWM11 PWM7 PWM3
 PB[7] : PWM20_N PWM16_N PWM12_N PWM8_N PWM4_N PWM0_N PWM20 PWM16 PWM12 PWM8  PWM4 PWM0
 PC[0] : PWM21_N PWM17_N PWM13_N PWM9_N PWM5_N PWM1_N PWM21 PWM17 PWM13 PWM9  PWM5 PWM1
 PC[1] : PWM22_N PWM18_N PWM14_N PWM10_N PWM6_N PWM2_N PWM22 PWM18 PWM14 PWM10 PWM6 PWM2
 PC[2] : PWM23_N PWM19_N PWM15_N PWM11_N PWM7_N PWM3_N PWM23 PWM19 PWM15 PWM11 PWM7 PWM3
 PC[3] : PWM20_N PWM16_N PWM12_N PWM8_N PWM4_N PWM0_N PWM20 PWM16 PWM12 PWM8  PWM4 PWM0
 PC[4] : PWM21_N PWM17_N PWM13_N PWM9_N PWM5_N PWM1_N PWM21 PWM17 PWM13 PWM9  PWM5 PWM1
 PC[5] : PWM22_N PWM18_N PWM14_N PWM10_N PWM6_N PWM2_N PWM22 PWM18 PWM14 PWM10 PWM6 PWM2
 PC[6] : PWM23_N PWM19_N PWM15_N PWM11_N PWM7_N PWM3_N PWM23 PWM19 PWM15 PWM11 PWM7 PWM3
 PC[7] : PWM20_N PWM16_N PWM12_N PWM8_N PWM4_N PWM0_N PWM20 PWM16 PWM12 PWM8  PWM4 PWM0
 PD[0] : PWM21_N PWM17_N PWM13_N PWM9_N PWM5_N PWM1_N PWM21 PWM17 PWM13 PWM9  PWM5 PWM1
 PD[1] : PWM22_N PWM18_N PWM14_N PWM10_N PWM6_N PWM2_N PWM22 PWM18 PWM14 PWM10 PWM6 PWM2
 PD[2] : PWM23_N PWM19_N PWM15_N PWM11_N PWM7_N PWM3_N PWM23 PWM19 PWM15 PWM11 PWM7 PWM3
 PD[3] : PWM20_N PWM16_N PWM12_N PWM8_N PWM4_N PWM0_N PWM20 PWM16 PWM12 PWM8  PWM4 PWM0
 PD[4] : PWM21_N PWM17_N PWM13_N PWM9_N PWM5_N PWM1_N PWM21 PWM17 PWM13 PWM9  PWM5 PWM1
 PD[5] : PWM22_N PWM18_N PWM14_N PWM10_N PWM6_N PWM2_N PWM22 PWM18 PWM14 PWM10 PWM6 PWM2
 PD[6] : PWM23_N PWM19_N PWM15_N PWM11_N PWM7_N PWM3_N PWM23 PWM19 PWM15 PWM11 PWM7 PWM3
 PD[7] : PWM20_N PWM16_N PWM12_N PWM8_N PWM4_N PWM0_N PWM20 PWM16 PWM12 PWM8  PWM4 PWM0
 PE[0] : PWM21_N PWM17_N PWM13_N PWM9_N PWM5_N PWM1_N PWM21 PWM17 PWM13 PWM9  PWM5 PWM1
 PE[1] : PWM22_N PWM18_N PWM14_N PWM10_N PWM6_N PWM2_N PWM22 PWM18 PWM14 PWM10 PWM6 PWM2
 PE[2] : PWM23_N PWM19_N PWM15_N PWM11_N PWM7_N PWM3_N PWM23 PWM19 PWM15 PWM11 PWM7 PWM3
 PE[3] : PWM20_N PWM16_N PWM12_N PWM8_N PWM4_N PWM0_N PWM20 PWM16 PWM12 PWM8  PWM4 PWM0
 PE[4] : PWM21_N PWM17_N PWM13_N PWM9_N PWM5_N PWM1_N PWM21 PWM17 PWM13 PWM9  PWM5 PWM1
 PE[5] : PWM22_N PWM18_N PWM14_N PWM10_N PWM6_N PWM2_N PWM22 PWM18 PWM14 PWM10 PWM6 PWM2
 PE[6] : PWM23_N PWM19_N PWM15_N PWM11_N PWM7_N PWM3_N PWM23 PWM19 PWM15 PWM11 PWM7 PWM3
 PE[7] : PWM20_N PWM16_N PWM12_N PWM8_N PWM4_N PWM0_N PWM20 PWM16 PWM12 PWM8  PWM4 PWM0
 PF[0] : PWM21_N PWM17_N PWM13_N PWM9_N PWM5_N PWM1_N PWM21 PWM17 PWM13 PWM9  PWM5 PWM1
 PF[1] : PWM22_N PWM18_N PWM14_N PWM10_N PWM6_N PWM2_N PWM22 PWM18 PWM14 PWM10 PWM6 PWM2
 PF[2] : PWM23_N PWM19_N PWM15_N PWM11_N PWM7_N PWM3_N PWM23 PWM19 PWM15 PWM11 PWM7 PWM3
 PF[3] : PWM20_N PWM16_N PWM12_N PWM8_N PWM4_N PWM0_N PWM20 PWM16 PWM12 PWM8  PWM4 PWM0
 PF[4] : PWM21_N PWM17_N PWM13_N PWM9_N PWM5_N PWM1_N PWM21 PWM17 PWM13 PWM9  PWM5 PWM1
 PF[5] : PWM22_N PWM18_N PWM14_N PWM10_N PWM6_N PWM2_N PWM22 PWM18 PWM14 PWM10 PWM6 PWM2
 PF[6] : PWM23_N PWM19_N PWM15_N PWM11_N PWM7_N PWM3_N PWM23 PWM19 PWM15 PWM11 PWM7 PWM3
 PF[7] : PWM20_N PWM16_N PWM12_N PWM8_N PWM4_N PWM0_N PWM20 PWM16 PWM12 PWM8  PWM4 PWM0
 PG[0] : PWM21_N PWM17_N PWM13_N PWM9_N PWM5_N PWM1_N PWM21 PWM17 PWM13 PWM9  PWM5 PWM1
 PG[1] : PWM22_N PWM18_N PWM14_N PWM10_N PWM6_N PWM2_N PWM22 PWM18 PWM14 PWM10 PWM6 PWM2
 PG[2] : PWM23_N PWM19_N PWM15_N PWM11_N PWM7_N PWM3_N PWM23 PWM19 PWM15 PWM11 PWM7 PWM3
 PG[3] : PWM20_N PWM16_N PWM12_N PWM8_N PWM4_N PWM0_N PWM20 PWM16 PWM12 PWM8  PWM4 PWM0
 PG[4] : PWM21_N PWM17_N PWM13_N PWM9_N PWM5_N PWM1_N PWM21 PWM17 PWM13 PWM9  PWM5 PWM1
 PG[5] : PWM22_N PWM18_N PWM14_N PWM10_N PWM6_N PWM2_N PWM22 PWM18 PWM14 PWM10 PWM6 PWM2
 PG[6] : PWM23_N PWM19_N PWM15_N PWM11_N PWM7_N PWM3_N PWM23 PWM19 PWM15 PWM11 PWM7 PWM3
 PG[7] : PWM20_N PWM16_N PWM12_N PWM8_N PWM4_N PWM0_N PWM20 PWM16 PWM12 PWM8  PWM4 PWM0
 PH[0] : PWM21_N PWM17_N PWM13_N PWM9_N PWM5_N PWM1_N PWM21 PWM17 PWM13 PWM9  PWM5 PWM1
 PH[1] : PWM22_N PWM18_N PWM14_N PWM10_N PWM6_N PWM2_N PWM22 PWM18 PWM14 PWM10 PWM6 PWM2
 PH[2] : PWM23_N PWM19_N PWM15_N PWM11_N PWM7_N PWM3_N PWM23 PWM19 PWM15 PWM11 PWM7 PWM3
 PH[3] : PWM20_N PWM16_N PWM12_N PWM8_N PWM4_N PWM0_N PWM20 PWM16 PWM12 PWM8  PWM4 PWM0
 PH[4] : PWM21_N PWM17_N PWM13_N PWM9_N PWM5_N PWM1_N PWM21 PWM17 PWM13 PWM9  PWM5 PWM1
 PH[5] : PWM22_N PWM18_N PWM14_N PWM10_N PWM6_N PWM2_N PWM22 PWM18 PWM14 PWM10 PWM6 PWM2
 PH[6] : PWM23_N PWM19_N PWM15_N PWM11_N PWM7_N PWM3_N PWM23 PWM19 PWM15 PWM11 PWM7 PWM3
 PH[7] : PWM20_N PWM16_N PWM12_N PWM8_N PWM4_N PWM0_N PWM20 PWM16 PWM12 PWM8  PWM4 PWM0

 */
typedef enum
{
    GPIO_FC_PA0 = GPIO_PA0,
    GPIO_FC_PA1 = GPIO_PA1,
    GPIO_FC_PA2 = GPIO_PA2,
    GPIO_FC_PA3 = GPIO_PA3,//Only supports USB function
    GPIO_FC_PA4 = GPIO_PA4,//Only supports USB function

    GPIO_FC_PB0 = GPIO_PB0,
    GPIO_FC_PB1 = GPIO_PB1,
    GPIO_FC_PB2 = GPIO_PB2,
    GPIO_FC_PB3 = GPIO_PB3,
    GPIO_FC_PB4 = GPIO_PB4,
    GPIO_FC_PB5 = GPIO_PB5,
    GPIO_FC_PB6 = GPIO_PB6,
    GPIO_FC_PB7 = GPIO_PB7,

    GPIO_FC_PC0 = GPIO_PC0,
    GPIO_FC_PC1 = GPIO_PC1,
    GPIO_FC_PC2 = GPIO_PC2,
    GPIO_FC_PC3 = GPIO_PC3,
    GPIO_FC_PC4 = GPIO_PC4,
    GPIO_FC_PC5 = GPIO_PC5,
    GPIO_FC_PC6 = GPIO_PC6,
    GPIO_FC_PC7 = GPIO_PC7,

    GPIO_FC_PD0 = GPIO_PD0,
    GPIO_FC_PD1 = GPIO_PD1,
    GPIO_FC_PD2 = GPIO_PD2,
    GPIO_FC_PD3 = GPIO_PD3,
    GPIO_FC_PD4 = GPIO_PD4,
    GPIO_FC_PD5 = GPIO_PD5,
    GPIO_FC_PD6 = GPIO_PD6,
    GPIO_FC_PD7 = GPIO_PD7,

    GPIO_FC_PE0 = GPIO_PE0,
    GPIO_FC_PE1 = GPIO_PE1,
    GPIO_FC_PE2 = GPIO_PE2,
    GPIO_FC_PE3 = GPIO_PE3,
    GPIO_FC_PE4 = GPIO_PE4,
    GPIO_FC_PE5 = GPIO_PE5,
    GPIO_FC_PE6 = GPIO_PE6,
    GPIO_FC_PE7 = GPIO_PE7,

    GPIO_FC_PF0 = GPIO_PF0,
    GPIO_FC_PF1 = GPIO_PF1,
    GPIO_FC_PF2 = GPIO_PF2,
    GPIO_FC_PF3 = GPIO_PF3,
    GPIO_FC_PF4 = GPIO_PF4,
    GPIO_FC_PF5 = GPIO_PF5,
    GPIO_FC_PF6 = GPIO_PF6,
    GPIO_FC_PF7 = GPIO_PF7,

    GPIO_FC_PG0 = GPIO_PG0,
    GPIO_FC_PG1 = GPIO_PG1,
    GPIO_FC_PG2 = GPIO_PG2,
    GPIO_FC_PG3 = GPIO_PG3,
    GPIO_FC_PG4 = GPIO_PG4,
    GPIO_FC_PG5 = GPIO_PG5,
    GPIO_FC_PG6 = GPIO_PG6,
    GPIO_FC_PG7 = GPIO_PG7,

    GPIO_FC_PH0 = GPIO_PH0,
    GPIO_FC_PH1 = GPIO_PH1,
    GPIO_FC_PH2 = GPIO_PH2,
    GPIO_FC_PH3 = GPIO_PH3,
    GPIO_FC_PH4 = GPIO_PH4,
    GPIO_FC_PH5 = GPIO_PH5,
    GPIO_FC_PH6 = GPIO_PH6,
    GPIO_FC_PH7 = GPIO_PH7,

    GPIO_FC_PI0 = GPIO_PI0,
    GPIO_FC_PI1 = GPIO_PI1,
    GPIO_FC_PI2 = GPIO_PI2,
    GPIO_FC_PI3 = GPIO_PI3,
    GPIO_FC_PI4 = GPIO_PI4,
    GPIO_FC_PI5 = GPIO_PI5,

    GPIO_NONE_PIN = 0x00,
} gpio_func_pin_e;

/**
 *  @brief  Define GPIO function mux types
 */
typedef enum
{
 PWM0    =  1, PWM1    =  1, PWM2    =  1, PWM3    =  1,
 PWM4    =  2, PWM5    =  2, PWM6    =  2, PWM7    =  2,
 PWM8    =  3, PWM9    =  3, PWM10   =  3, PWM11   =  3,
 PWM12   =  4, PWM13   =  4, PWM14   =  4, PWM15   =  4,
 PWM16   =  5, PWM17   =  5, PWM18   =  5, PWM19   =  5,
 PWM20   =  6, PWM21   =  6, PWM22   =  6, PWM23   =  6,
 PWM0_N  =  7, PWM1_N  =  7, PWM2_N  =  7, PWM3_N  =  7,
 PWM4_N  =  8, PWM5_N  =  8, PWM6_N  =  8, PWM7_N  =  8,
 PWM8_N  =  9, PWM9_N  =  9, PWM10_N =  9, PWM11_N =  9,
 PWM12_N = 10, PWM13_N = 10, PWM14_N = 10, PWM15_N = 10,
 PWM16_N = 11, PWM17_N = 11, PWM18_N = 11, PWM19_N = 11,
 PWM20_N = 12, PWM21_N = 12, PWM22_N = 12, PWM23_N = 12,
    MSPI_CN2           = 13,
    MSPI_CN3           = 14,
    MSPI_CN1           = 15,
    I2C_SCL_IO         = 16,
    I2C_SDA_IO         = 17,
    I2C1_SDA_IO        = 18,
    I2C1_SCL_IO        = 19,
    UART0_CTS_I        = 20,
    UART0_RTS          = 21,
    UART0_TX           = 22,
    UART0_RTX_IO       = 23,
    UART1_CTS_I        = 24,
    UART1_RTS          = 25,
    UART1_TX           = 26,
    UART1_RTX_IO       = 27,
    UART2_CTS_I        = 28,
    UART2_RTS          = 29,
    UART2_TX           = 30,
    UART2_RTX_IO       = 31,
    UART3_CTS_I        = 32,
    UART3_RTS          = 33,
    UART3_TX           = 34,
    UART3_RTX_IO       = 35,
    UART4_CTS_I        = 36,
    UART4_RTS          = 37,
    UART4_TX           = 38,
    UART4_RTX_IO       = 39,
    CLK_7816           = 40,
    TDI_I              = 41,
    I2S2_BCK_IO        = 42,
    I2S2_LR0_IO        = 43,
    I2S2_DAT0_IO       = 44,
    I2S2_LR1_IO        = 45,
    I2S2_DAT1_IO       = 46,
    I2S2_CLK           = 47,
    DMIC0_CLK          = 48,
    DMIC0_DAT_I        = 49,
    SDM0_P             = 50,
    SDM0_N             = 51,
    SDM1_P             = 52,
    SDM1_N             = 53,
    IR_LEARN_I         = 54,
    SSPI_CN_I          = 55,
    SSPI_CK_I          = 56,
    SSPI_SI_IO         = 57,
    SSPI_SO_IO         = 58,
    KEYS0_IO           = 59,
    PWM_SYNC_I         = 60,
    RZ_TX              = 61,
    SWM_IO             = 62,
    TX_CYC2PA          = 63,
    WIFI_DENY_I        = 64,
    BT_ACTIVITY        = 65,
    BT_STATUS          = 66,
    ATSEL_0            = 67,
    ATSEL_1            = 68,
    ATSEL_2            = 69,
    ATSEL_3            = 70,
    ATSEL_4            = 71,
    ATSEL_5            = 72,
    RX_CYC2LNA         = 73,
    DBG_PROBE_CLK      = 74,
    DBG_BB0            = 75, DBG_AUDIO_DAC = 75,
    DBG_ADC_I_DAT0     = 76,
    LIN0_RX_I          = 77,
    LIN0_TX            = 78,
    LIN1_RX_I          = 79,
    LIN1_TX            = 80,
    CAN0_RX_I          = 81,
    CAN0_TX            = 82,
    CAN1_RX_I          = 83,
    CAN1_TX            = 84,
    I3C0_SDA_PULLUP_EN = 85,
    I3C0_SDA_IO        = 86,
    I3C0_SCL_IO        = 87,
    I3C1_SDA_PULLUP_EN = 88,
    I3C1_SDA_IO        = 89,
    I3C1_SCL_IO        = 90,
    GSPI_CN_IO         = 91,
    GSPI_IO3_IO        = 92,
    GSPI_IO2_IO        = 93,
    GSPI_MISO_IO       = 94,
    GSPI_MOSI_IO       = 95,
    GSPI_CK_IO         = 96,
    GSPI1_CN_IO        = 97,
    GSPI1_IO3_IO       = 98,
    GSPI1_IO2_IO       = 99,
    GSPI1_MISO_IO      = 100,
    GSPI1_MOSI_IO      = 101,
    GSPI1_CK_IO        = 102,
    GSPI2_CN_IO        = 103,
    GSPI2_IO3_IO       = 104,
    GSPI2_IO2_IO       = 105,
    GSPI2_MISO_IO      = 106,
    GSPI2_MOSI_IO      = 107,
    GSPI2_CK_IO        = 108,
    GSPI3_CN_IO        = 109,
    GSPI3_IO3_IO       = 110,
    GSPI3_IO2_IO       = 111,
    GSPI3_MISO_IO      = 112,
    GSPI3_MOSI_IO      = 113,
    GSPI3_CK_IO        = 114,
    GSPI4_CN_IO        = 115,
    GSPI4_IO3_IO       = 116,
    GSPI4_IO2_IO       = 117,
    GSPI4_MISO_IO      = 118,
    GSPI4_MOSI_IO      = 119,
    GSPI4_CK_IO        = 120,
    LSPI_CN_IO         = 121,
    LSPI_IO3_IO        = 122,
    LSPI_IO2_IO        = 123,
    LSPI_MISO_IO       = 124,
    LSPI_MOSI_IO       = 125,
    LSPI_CK_IO         = 126,
    PA5_USB0_DM_IO     = 0,
    PA6_USB0_DP_IO     = 0,
    PA7_SWS_IO         = 0,
    PC4_TDI_I          = 0,
    PC5_TDO_IO         = 0,
    PC6_TMS_IO         = 0,
    PC7_TCK_I          = 0,
    PI0_MSPI_MOSI_IO   = 0,
    PI1_MSPI_CK_IO     = 0,
    PI2_MSPI_IO3_IO    = 0,
    PI3_MSPI_CN_IO     = 0,
    PI4_MSPI_MISO_IO   = 0,
    PI5_MSPI_IO2_IO    = 0,

} gpio_func_e;

/**
 *  @brief  Define GPIO mux func
 */
typedef enum
{
    AS_GPIO,

    AS_SWS,

    AS_TDI,
    AS_TDO,
    AS_TMS,
    AS_TCK,

    AS_MSPI_MOSI,
    AS_MSPI_CK,
    AS_MSPI_IO3,
    AS_MSPI_CN,
    AS_MSPI_MISO,
    AS_MSPI_IO2,
} gpio_fuc_e;

/*
 * @brief define gpio irq status types
 */
typedef enum
{
    GPIO_IRQ_IRQ0 = BIT(0),
    GPIO_IRQ_IRQ1 = BIT(1),
    GPIO_IRQ_IRQ2 = BIT(2),
    GPIO_IRQ_IRQ3 = BIT(3),
    GPIO_IRQ_IRQ4 = BIT(4),
    GPIO_IRQ_IRQ5 = BIT(5),
    GPIO_IRQ_IRQ6 = BIT(6),
    GPIO_IRQ_IRQ7 = BIT(7),
} gpio_irq_e;

/**
 *  @brief  Define rising/falling types
 */
typedef enum
{
    POL_RISING  = 0,
    POL_FALLING = 1,
} gpio_pol_e;

/**
 *  @brief  Define interrupt types
 */
typedef enum
{
    INTR_RISING_EDGE = 0,
    INTR_FALLING_EDGE,
    INTR_HIGH_LEVEL,
    INTR_LOW_LEVEL,
} gpio_irq_trigger_type_e;

/**
 *  @brief  Define IRQ types
 */
typedef enum
{
    GPIO_IRQ0 = 0,
    GPIO_IRQ1 = 1,
    GPIO_IRQ2 = 2,
    GPIO_IRQ3 = 3,
    GPIO_IRQ4 = 4,
    GPIO_IRQ5 = 5,
    GPIO_IRQ6 = 6,
    GPIO_IRQ7 = 7,
} gpio_irq_num_e;

/**
 *  @brief  Define pull up or down types
 *  @note   The GPIO pull-up/pull-down resistance is a simulation result by the internal
 *          MOSFET and affected by the IO voltage VDDO3. The lower the IO voltage of GPIO,
 *          the higher the pull-up/pull-down resistance of GPIO.
 */
typedef enum
{
    GPIO_PIN_UP_DOWN_FLOAT = 0,
    GPIO_PIN_PULLUP_1M     = 1,
    GPIO_PIN_PULLDOWN_100K = 2,
    GPIO_PIN_PULLUP_10K    = 3,
} gpio_pull_type_e;

typedef enum
{
    PROBE_CLK32K     = 0,
    PROBE_RC24M      = 1,
    PROBE_PLL0       = 2,
    PROBE_XTL24M     = 3,
    PROBE_CCLK       = 4,
    PROBE_HCLK       = 5,
    PROBE_PCLK       = 6,
    PROBE_NVM        = 7,
    PROBE_CLK_MSPI   = 8,
    PROBE_CLK_LSPI   = 9,
    PROBE_CLK_GSPI   = 0x0a,
    PROBE_CLK_GSPI1  = 0x0b,
    PROBE_CLK_GSPI2  = 0x0c,
    PROBE_CLK_GSPI3  = 0x0d,
    PROBE_CLK_GSPI4  = 0x0e,
    PROBE_CLK_I3C0   = 0x0f,
    PROBE_CLK_I3C1   = 0x10,
    PROBE_CLK_STIMER = 0x11,
    PROBE_CLK_SARDIG = 0x12,
    PROBE_CLK_I2S0   = 0x13,
    PROBE_CLK_I2S2   = 0x14,
    PROBE_CLK_DMIC   = 0x15,
    PROBE_CLK_AUDIO  = 0x16,
    PROBE_CLK_7816   = 0x19,
    PROBE_DBG_CLK    = 0x1a,
} probe_clk_sel_e;

/**
 *  @brief  Define pem task signal types
 */
typedef enum
{
    TASK_SIGNAL_SEL0 = 0,
    TASK_SIGNAL_SEL1 = 1,
    TASK_SIGNAL_SEL2 = 2,
    TASK_SIGNAL_SEL3 = 3,
    TASK_SIGNAL_SEL4 = 4,
    TASK_SIGNAL_SEL5 = 5,
    TASK_SIGNAL_SEL6 = 6,
    TASK_SIGNAL_SEL7 = 7,
} pem_task_signal_type_e;

/**
 *  @brief  Define pem gpio group task types, choose which gpio group  as task 
 */
typedef enum
{
    TASK_GROUP_GPIO_PA = 0,
    TASK_GROUP_GPIO_PB = 1,
    TASK_GROUP_GPIO_PC = 2,
    TASK_GROUP_GPIO_PD = 3,
    TASK_GROUP_GPIO_PE = 4,
    TASK_GROUP_GPIO_PF = 5,
} pem_gpio_group_task_type_e;

/**
 *  @brief  Define driving strength types
 */
typedef enum
{
    DRV_STRENGTH_2MA  = 0,
    DRV_STRENGTH_4MA  = 1,
    DRV_STRENGTH_8MA  = 2,
    DRV_STRENGTH_12MA = 3,
} gpio_drv_strength_e;

typedef enum
{
    DRV_Slew_Rate_Fast = 0,
    DRV_Slew_Rate_Slow = 1,
} gpio_drv_slew_rate_e;

/**********************************************************************************************************************
 *                                     global variable declaration                                                    *
 *********************************************************************************************************************/

/**********************************************************************************************************************
 *                                      global function prototype                                                     *
 *********************************************************************************************************************/

/**
 * @brief      This function servers to enable gpio function.
 * @param[in]  pin - the selected pin.
 * @return     none.
 */
static inline void gpio_function_en(gpio_pin_e pin)
{
    unsigned char bit = pin & 0xff;
    BM_SET(reg_gpio_func(pin), bit);
}

/**
 * @brief      This function servers to disable gpio function.
 * @param[in]  pin - the selected pin.
 * @return     none.
 */
static inline void gpio_function_dis(gpio_pin_e pin)
{
    unsigned char bit = pin & 0xff;
    BM_CLR(reg_gpio_func(pin), bit);
}

/**
 * @brief     This function set the pin's output high level.
 * @param[in] pin - the pin needs to set its output level.
 * @return    none.
 */
static inline void gpio_set_high_level(gpio_pin_e pin)
{
    unsigned char bit     = pin & 0xff;
    reg_gpio_out_set(pin) = bit;
}

/**
 * @brief     This function set the pin's output low level.
 * @param[in] pin - the pin needs to set its output level.
 * @return    none.
 */
static inline void gpio_set_low_level(gpio_pin_e pin)
{
    unsigned char bit       = pin & 0xff;
    reg_gpio_out_clear(pin) = bit;
}

/**
 * @brief     This function set the pin's output level.
 * @param[in] pin - the pin needs to set its output level
 * @param[in] value - value of the output level(1: high 0: low)
 * @return    none
 */
static inline void gpio_set_level(gpio_pin_e pin, unsigned char value)
{
    if (value) {
        gpio_set_high_level(pin);
    } else {
        gpio_set_low_level(pin);
    }
}

/**
 * @brief     This function read the pin's input level.
 * @param[in] pin - the pin needs to read its input level.
 * @return    1: the pin's input level is high.
 *            0: the pin's input level is low.
 */
static inline _Bool gpio_get_level(gpio_pin_e pin)
{
    return BM_IS_SET(reg_gpio_in(pin), pin & 0xff);
}

/**
 * @brief      This function read all the pins' input level.
 * @param[out] p - the buffer used to store all the pins' input level
 * @return     none
 */
static inline void gpio_get_level_all(unsigned char *p)
{
    p[0] = reg_gpio_pa_in;
    p[1] = reg_gpio_pb_in;
    p[2] = reg_gpio_pc_in;
    p[3] = reg_gpio_pd_in;
    p[4] = reg_gpio_pe_in;
    p[5] = reg_gpio_pf_in;
    p[6] = reg_gpio_pg_in;
    p[7] = reg_gpio_ph_in;
}

/**
 * @brief     This function set the pin toggle.
 * @param[in] pin - the pin needs to toggle.
 * @return    none.
 */
static inline void gpio_toggle(gpio_pin_e pin)
{
    unsigned char bit        = pin & 0xff;
    reg_gpio_out_toggle(pin) = bit;
}

/**
 * @brief      This function enable the output function of a pin.
 * @param[in]  pin - the pin needs to set the output function.
 * @return     none.
 */
static inline void gpio_output_en(gpio_pin_e pin)
{
    unsigned char bit = pin & 0xff;
    BM_CLR(reg_gpio_oen(pin), bit);
}

/**
 * @brief      This function disable the output function of a pin.
 * @param[in]  pin - the pin needs to set the output function.
 * @return     none.
 */
static inline void gpio_output_dis(gpio_pin_e pin)
{
    unsigned char bit = pin & 0xff;
    BM_SET(reg_gpio_oen(pin), bit);
}

/**
 * @brief      This function enable set output function of a pin.
 * @param[in]  pin - the pin needs to set the output function (1: enable,0: disable)
 * @return     none
 */
static inline void gpio_set_output(gpio_pin_e pin, unsigned char value)
{
    if (value) {
        gpio_output_en(pin);
    } else {
        gpio_output_dis(pin);
    }
}

/**
 * @brief      This function determines whether the output function of a pin is enabled.
 * @param[in]  pin - the pin needs to determine whether its output function is enabled.
 * @return     1: the pin's output function is enabled.
 *             0: the pin's output function is disabled.
 */
static inline _Bool gpio_is_output_en(gpio_pin_e pin)
{
    return !BM_IS_SET(reg_gpio_oen(pin), pin & 0xff);
}

/**
 * @brief     This function determines whether the input function of a pin is enabled.
 * @param[in] pin - the pin needs to determine whether its input function is enabled(not include group_pc).
 * @return    1: the pin's input function is enabled.
 *            0: the pin's input function is disabled.
 */
static inline _Bool gpio_is_input_en(gpio_pin_e pin)
{
    return BM_IS_SET(reg_gpio_ie(pin), pin & 0xff);
}

/**
 * @brief       This function is used to enable the GPIO pin of mspi.
 * @param[in]   none.
 * @return      none.
 * @note        This interface is for internal use only.
 */
static _always_inline void gpio_set_mspi_pin_ie_en(void)
{
    reg_gpio_pi_ie = 0x3f;
}

/**
 * @brief       This function is used to disable the GPIO pin of mspi.
 * @param[in]   none.
 * @return      none.
 * @note        This interface is for internal use only.
 */
static _always_inline void gpio_set_mspi_pin_ie_dis(void)
{
    reg_gpio_pi_ie = 0x00;
}

/**
 * @brief      This function serves to enable gpio irq0~7 function.
 * @param[in]  pin  - the pin needs to enable its IRQ.
 * @param[in]  irq  - there are 8 types of irq to choose.(irq0/irq1/irq2/irq3/irq4/irq5/irq6/irq7)
 * @return     none.
 */
static inline void gpio_irq_en(gpio_pin_e pin, gpio_irq_num_e irq)
{
    BM_SET(reg_gpio_irq_en(pin, irq), pin & 0xff);
}

/**
 * @brief      This function serves to disable gpio irq0 function.
 * @param[in]  pin  - the pin needs to disable its IRQ.
 * @param[in]  irq  - there are 8 types of irq to choose.(irq0/irq1/irq2/irq3/irq4/irq5/irq6/irq7)
 * @return     none.
 */
static inline void gpio_irq_dis(gpio_pin_e pin, gpio_irq_num_e irq)
{
    BM_CLR(reg_gpio_irq_en(pin, irq), pin & 0xff);
}

/**
 * @brief      This function serves to enable gpio irq mask function.
 * @param[in]  mask  - to select interrupt type.
 * @return     none.
 */
static inline void gpio_set_irq_mask(gpio_irq_e mask)
{
    BM_SET(reg_gpio_irq_src_mask, mask);
}

/**
 * @brief      This function serves to clr gpio irq status.
 * @param[in]  status  - the irq need to clear.
 * @return     none.
 */
static inline void gpio_clr_irq_status(gpio_irq_e status)
{
    reg_gpio_irq_clr = status;
}

/**
 * @brief      This function serves to disable gpio irq mask function.
 *             if disable gpio interrupt,choose disable gpio mask , use interface gpio_clr_irq_mask instead of gpio_irq_dis/gpio_gpio2risc0_irq_dis/gpio_gpio2risc1_irq_dis.
 * @return     none.
 */
static inline void gpio_clr_irq_mask(gpio_irq_e mask)
{
    BM_CLR(reg_gpio_irq_ctrl, mask);
}

/**
 * @brief     This function set a pin's IRQ , here you can choose from 8 interrupts for flexible configuration, each interrupt is independent and equal to each other.
 * @param[in] irq           - there are 8 types of irq to choose.(irq0/irq1/irq2/irq3/irq4/irq5/irq6/irq7)
 * @param[in] pin           - the pin needs to enable its IRQ.
 * @param[in] trigger_type  - gpio interrupt type.
 *                            0: rising edge.
 *                            1: falling edge.
 *                            2: high level.
 *                            3: low level.
 * @return    none.
 */
void gpio_set_irq(gpio_irq_num_e irq, gpio_pin_e pin, gpio_irq_trigger_type_e trigger_type);

/**
 * @brief      This function serves to set the gpio-mux function.
 * @param[in]  pin      - the pin needs to set.
 * @param[in]  function - the function need to set.
 * @return     none.
 */
void gpio_set_mux_function(gpio_func_pin_e pin, gpio_func_e function);

/**
 * @brief      This function set the input function of a pin.
 * @param[in]  pin - the pin needs to set the input function.
 * @return     none.
 */
void gpio_input_en(gpio_pin_e pin);

/**
 * @brief      This function disable the input function of a pin.
 * @param[in]  pin - the pin needs to set the input function.
 * @return     none.
 */
void gpio_input_dis(gpio_pin_e pin);

/**
 * @brief      This function set the input function of a pin.
 * @param[in]  pin - the pin needs to set the input function
 * @param[in]  value - enable or disable the pin's input function(1: enable,0: disable )
 * @return     none
 */
void gpio_set_input(gpio_pin_e pin, unsigned char value);

/**
 * @brief      This function servers to set the specified GPIO into High-impedance state and also enable the pull-down resistor.
 *             To prevent power leakage, you need to call gpio_shutdown(GPIO_ALL) (set all gpio to high resistance, except SWS and MSPI)
 *             as front as possible in the program, and then initialize the corresponding GPIO according to the actual using situation.
 * @param[in]  pin  - select the specified GPIO.Only support GPIO_GROUPA ~GPIO_GROUPI,GPIO_GPOUPANA and GPIO_ALL.
 * @return     none.
 */
void gpio_shutdown(gpio_pin_e pin);

/**
 * @brief     This function set a pin's pull-up/down resistor.
 * @param[in] pin - the pin needs to set its pull-up/down resistor.
 * @param[in] up_down_res - the type of the pull-up/down resistor.
 * @return    none.
 */
void gpio_set_up_down_res(gpio_pin_e pin, gpio_pull_type_e up_down_res);

/**
 * @brief      This function set the pin's slew rate.
 * @param[in]  pin - the pin needs to set the driving strength
 * @param[in]  value - the level of slew rate
 * @return     none
 * @note       Logic HIGH selects slow slew rate, logic LOW selects fast slew rate.
 */
void gpio_set_slew_rate(gpio_pin_e pin, gpio_drv_slew_rate_e value);
/**
 * @brief     This function set probe clk output.
 * @param[in] pin
 * @param[in] sel_clk
 * @return    none.
 */
void gpio_set_probe_clk_function(gpio_func_pin_e pin, probe_clk_sel_e sel_clk);
/**
 * @brief      This function serves to configure the PEM task.
 * @param[in]  chn - to select the PEM channel.
 * @param[in]  pin - the pin needs to set the output function.
 * @return     none.
 */
void gpio_set_pem_task(pem_chn_e chn, gpio_pin_e pin);
/**
 * @brief      This function serves to configure the PEM event.
 * @param[in]  chn - to select the PEM channel.
 * @param[in]  pin - the pin needs to set the input function.
 * @return     none.
 */
void gpio_set_pem_event(pem_chn_e chn, gpio_pin_e pin);
/**
 * @brief      This function set the pin's driving strength.
 * @param[in]  pin - the pin needs to set the driving strength
 * @param[in]  value - the level of driving strength
 * @return     none
 * @note    | DS0 | DS1 | Drv Strength |
            | --- | --- | ------------ |
            | 0   | 0   | 2mA          |
            | 0   | 1   | 4mA          |
            | 1   | 0   | 8mA          |
            | 1   | 1   | 12mA         |
 */
void gpio_set_data_strength(gpio_pin_e pin, gpio_drv_strength_e value);

/**
 * @brief     This function set jtag or sdp function.
 * @param[in] pin
 * @return    none.
 */
void jtag_sdp_set_pin(gpio_pin_e pin);

/**
 * @brief     This function serves to set jtag(4 wires) pin . Where, PD[4]; PD[5]; PD[6]; PD[7] correspond to TDI; TDO; TMS; TCK functions mux respectively.
 * @param[in] none
 * @return    none.
 */
void jtag_set_pin_en(void);

#endif
