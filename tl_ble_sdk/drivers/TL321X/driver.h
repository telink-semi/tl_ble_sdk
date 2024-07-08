/********************************************************************************************************
 * @file    driver.h
 *
 * @brief   This is the header file for TL321X
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
#pragma once




#include "lib/include/plic.h"
#include "lib/include/pm.h"
#include "lib/include/rf/rf_common.h"
#include "lib/include/sys.h"
#include "lib/include/trng/trng_algorithm.h"



#include "lib/include/ske/ske.h"
#include "lib/include/ske/ske_algorithm.h"

#include "analog.h"
#include "dma.h"
#include "gpio.h"
#include "i2c.h"
#include "spi.h"
#include "pwm.h"
#include "timer.h"
#include "flash.h"
#include "usbhw.h"
#include "watchdog.h"
#include "core.h"

#include "uart.h"
#include "stimer.h"

#include "clock.h"
#include "mspi.h"
#include "adc.h"
#include "qdec.h"
#include "plic_sw.h"
#include "plmt.h"
#include "flash/flash_type.h"
#include "flash/flash_common.h"
