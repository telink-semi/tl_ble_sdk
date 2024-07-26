/********************************************************************************************************
 * @file    prd_adv.h
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
#ifndef STACK_BLE_CONTROLLER_LL_PRDADV_PRD_ADV_H_
#define STACK_BLE_CONTROLLER_LL_PRDADV_PRD_ADV_H_





/* maximum number of periodic advertising this SDK can support */
#define         PERIODIC_ADV_NUMBER_MAX                         2


#define         PERD_ADV_PARAM_LENGTH                           604  //user can't modify this value !!! sizeof(st_prd_adv_t)



/**
 * @brief      for user to initialize periodic advertising module, and to allocate periodic advertising parameters buffer
 * @param[in]  pBuff - global buffer allocated by application layer.
 * @param[in]  num_periodic_adv - number of application adv_sets
 * @return     Status - 0x00: command succeeded;
 *                      0x12: num_periodic_adv exceed maximum number of supported periodic advertising.
 */
ble_sts_t   blc_ll_initPeriodicAdvModule_initPeriodicdAdvSetParamBuffer(u8 *pBuff, int num_periodic_adv);


/**
 * @brief      This function is used by the Host to set the parameters for periodic advertising.
 * @param[in]  adv_handle - - Used to identify a periodic advertising train
 * @param[in]  advInter_min - Periodic_Advertising_Interval_Min(Range: 0x0006 to 0xFFFF, Time = N * 1.25 ms Time Range: 7.5 ms to 81.91875 s)
 * @param[in]  advInter_max - Periodic_Advertising_Interval_Max
 * @param[in]  property - Periodic_Advertising_Properties
 * @return     Status - 0x00: command succeeded; 0x01-0xFF: command failed
 */
ble_sts_t   blc_ll_setPeriodicAdvParam(adv_handle_t adv_handle, u16 advInter_min, u16 advInter_max, perd_adv_prop_t property);

/**
 * @brief      initialize Periodic Advertising Data buffer for all adv_set
 * @param[in]  perdAdvData -
 * @param[in]  max_len_perdAdvData -
 * @return     none
 */
void        blc_ll_initPeriodicAdvDataBuffer(u8 *perdAdvData, int max_len_perdAdvData);

/**
 * @brief      initialize Periodic Advertising Data buffer for specific adv_set.
 *             notice that:
 * @param[in]  adv_handle - equal to adv_set index here. So if using single adv_set, adv_handle can only be 0x00;
 *                          if using multiple adv_set, for example 3, adv_handle can be 0x00,0x01 and 0x02
 * @param[in]  perdAdvData -
 * @param[in]  max_len_perdAdvData -
 * @return     Status - 0x00: succeed.
 *                      0x12: adv_handle out of range.
 */
ble_sts_t   blc_ll_initPeriodicAdvDataBuffer_by_advHandle(u8 adv_handle, u8 *perdAdvData, int max_len_perdAdvData);

/**
 * @brief      This function is used to set the data used in periodic advertising PDUs.
 * @param[in]  adv_handle - - equal to adv_set index here.
 * @param[in]  advData_len -
 * @param[in]  *advdata -
 * @return     Status - 0x00: command succeeded; 0x01-0xFF: command failed
 */
ble_sts_t   blc_ll_setPeriodicAdvData(adv_handle_t adv_handle, u16 advData_len, const u8 *advdata);



/**
 * @brief      This function is used to enable or disable the periodic advertising for the advertising
 *             set specified by the Advertising_Handle parameter
 * @param[in]  per_adv_enable - Advertising_Enable
 * @param[in]  adv_handle     - Used to identify an advertising set.
 * @return     Status         - 0x00: command succeeded; 0x01-0xFF: command failed
 */
ble_sts_t   blc_ll_setPeriodicAdvEnable(u8 per_adv_enable, adv_handle_t adv_handle);








#endif /* STACK_BLE_CONTROLLER_LL_PRDADV_PRD_ADV_H_ */
