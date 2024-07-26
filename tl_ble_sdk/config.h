/********************************************************************************************************
 * @file    config.h
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
#pragma once


#define CHIP_TYPE_B91       1
#define CHIP_TYPE_TL721X    5
#define CHIP_TYPE_TL321X    6

#ifndef CHIP_TYPE
#define CHIP_TYPE           CHIP_TYPE_B91
#endif






#define MCU_CORE_B91        1
#define MCU_CORE_TL721X     5
#define MCU_CORE_TL321X     6


#if(CHIP_TYPE == CHIP_TYPE_B91)
    #define MCU_CORE_TYPE   MCU_CORE_B91
#elif(CHIP_TYPE == CHIP_TYPE_TL721X)
    #define MCU_CORE_TYPE   MCU_CORE_TL721X
#elif(CHIP_TYPE == CHIP_TYPE_TL321X)
    #define MCU_CORE_TYPE   MCU_CORE_TL321X
#endif



