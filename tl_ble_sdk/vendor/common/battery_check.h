/********************************************************************************************************
 * @file    battery_check.h
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
#ifndef BATTERY_CHECK_H_
#define BATTERY_CHECK_H_


#ifndef BATT_CHECK_ENABLE
#define BATT_CHECK_ENABLE                    0
#endif


#define BAT_DEEP_THRESHOLD_MV                2000   // 2000 mV low battery alarm

#define DCDC_ADC_SOFTWARE_FILTER            0  // Filter ADC data in DCDC mode

extern u8      adc_hw_initialized;   //note: can not be retention variable

/**
 * @brief      This function serves to enable battery detect.
 * @param[in]  en - enable battery detect
 * @return     none
 */
void battery_set_detect_enable (int en);

/**
 * @brief     This function serves to get battery detect status.
 * @param    none
 * @return    other: battery detect enable
 *               0: battery detect disable
 */
int  battery_get_detect_enable (void);

/**
 * @brief        This is battery check function
 * @param[in]    alarm_vol_mv - input battery calibration
 * @return      0: batt_vol_mv < alarm_vol_mv 1: batt_vol_mv > alarm_vol_mv
 */
_attribute_ram_code_
int app_battery_power_check(u16 alarm_vol_mv);

#endif /* APP_BATTDET_H_ */
