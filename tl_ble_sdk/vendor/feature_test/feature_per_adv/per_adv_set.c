/********************************************************************************************************
 * @file    per_adv_set.c
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
#include "stack/ble/ble.h"


#if (FEATURE_TEST_MODE == TEST_PER_ADV)

#if defined(TLK_ONLY_BLE_HOST)
#define ADV_SET_PARAM_LENGTH  0
#define PERD_ADV_PARAM_LENGTH  0
#endif

    /*********************************** Extended ADV data buffer allocation, Begin ************************************/
    #define APP_EXT_ADV_SETS_NUMBER     1    //user set value
    #define APP_EXT_ADV_DATA_LENGTH     1024 //2048//1664//1024   //user set value
    #define APP_EXT_SCANRSP_DATA_LENGTH 1024 //2048//1664//1024   //user set value
_attribute_ble_data_retention_ u8 app_extAdvSetParam_buf[ADV_SET_PARAM_LENGTH * APP_EXT_ADV_SETS_NUMBER];
_attribute_iram_noinit_data_ u8   app_extAdvData_buf[APP_EXT_ADV_DATA_LENGTH * APP_EXT_ADV_SETS_NUMBER];
_attribute_iram_noinit_data_ u8   app_extScanRspData_buf[APP_EXT_SCANRSP_DATA_LENGTH * APP_EXT_ADV_SETS_NUMBER];

    /*********************************** Extended ADV data buffer allocation, End** ************************************/

    /*********************************** Periodic ADV data buffer allocation, Begin ************************************/
    #define APP_PERID_ADV_SETS_NUMBER 1   //1//EBQ test need to change it to the supported value
    #define APP_PERID_ADV_DATA_LENGTH 100 //1024

_attribute_ble_data_retention_ u8 app_peridAdvSet_buffer[PERD_ADV_PARAM_LENGTH * APP_PERID_ADV_SETS_NUMBER];
_attribute_iram_noinit_data_ u8   app_peridAdvData_buffer[APP_PERID_ADV_DATA_LENGTH * APP_PERID_ADV_SETS_NUMBER];
/*********************************** Periodic ADV data buffer allocation, End** ************************************/

    #if (APP_PAWR_ADV_TEST_EN)
        /*********************************** Periodic ADV with Response data buffer allocation, Begin ************************************/
        #define APP_PAWR_SUBEVT_DATA_LENGTH 100
        #define APP_PAWR_SUBEVT_DATA_COUNT  4
_attribute_iram_noinit_data_ u8 app_peridAdvWrData_buffer[APP_PERID_ADV_SETS_NUMBER * APP_PAWR_SUBEVT_DATA_LENGTH * APP_PAWR_SUBEVT_DATA_COUNT];
    /*********************************** Periodic ADV with Response data buffer allocation, End** ************************************/
    #endif

_attribute_ble_data_retention_ ble_sts_t ble_status = BLE_SUCCESS;
_attribute_ble_data_retention_ u8        testAdvData[APP_EXT_ADV_DATA_LENGTH];
_attribute_ble_data_retention_ u8        testScanRspData[APP_EXT_SCANRSP_DATA_LENGTH];

void app_periodic_adv_test(void)
{
    //////////// Extended ADV Initialization  Begin ///////////////////////
    /* Extended ADV module and ADV Set Parameters buffer initialization */
    blc_ll_initExtendedAdvModule_initExtendedAdvSetParamBuffer(app_extAdvSetParam_buf, APP_EXT_ADV_SETS_NUMBER);
    blc_ll_initExtendedAdvDataBuffer(app_extAdvData_buf, APP_EXT_ADV_DATA_LENGTH);
    blc_ll_initExtendedScanRspDataBuffer(app_extScanRspData_buf, APP_EXT_SCANRSP_DATA_LENGTH);
    u32 my_adv_interval_min = ADV_INTERVAL_200MS;
    u32 my_adv_interval_max = ADV_INTERVAL_200MS;
    // Extended, None_Connectable_None_Scannable undirected, with auxiliary packet
    blc_ll_setExtAdvParam(ADV_HANDLE0, ADV_EVT_PROP_EXTENDED_NON_CONNECTABLE_NON_SCANNABLE_UNDIRECTED, my_adv_interval_min, my_adv_interval_max, BLT_ENABLE_ADV_ALL, OWN_ADDRESS_PUBLIC, BLE_ADDR_PUBLIC, NULL, ADV_FP_NONE, TX_POWER_3dBm, BLE_PHY_1M, 0, BLE_PHY_1M, ADV_SID_0, 0);
    for (int i = 0; i < 1024; i++) {
        testAdvData[i] = i;
    }

    blc_ll_setExtAdvData(ADV_HANDLE0, 31, (u8 *)testAdvData);
    //  blc_ll_setExtAdvData( ADV_HANDLE0,   200, testAdvData);
    //  blc_ll_setExtAdvData( ADV_HANDLE0,   400, testAdvData);
    //  blc_ll_setExtAdvData( ADV_HANDLE0,   600, testAdvData);
    //  blc_ll_setExtAdvData( ADV_HANDLE0,  1024, testAdvData);
    blc_ll_setExtAdvEnable(BLC_ADV_ENABLE, ADV_HANDLE0, 0, 0);
    //////////////// Extended ADV Initialization End ///////////////////////


    //////////// Periodic Adv Initialization  Begin ////////////////////////
    blc_ll_initPeriodicAdvModule_initPeriodicdAdvSetParamBuffer(app_peridAdvSet_buffer, APP_PERID_ADV_SETS_NUMBER);
    blc_ll_initPeriodicAdvDataBuffer(app_peridAdvData_buffer, APP_PERID_ADV_DATA_LENGTH);
    u32 my_per_adv_itvl_min = PERADV_INTERVAL_200MS;
    u32 my_per_adv_itvl_max = PERADV_INTERVAL_200MS;

    blc_ll_setPeriodicAdvParam(ADV_HANDLE0, my_per_adv_itvl_min, my_per_adv_itvl_max, PERD_ADV_PROP_MASK_TX_POWER_INCLUDE);
    blc_ll_setPeriodicAdvData(ADV_HANDLE0, 100, testAdvData);
    blc_ll_setPeriodicAdvEnable(BLC_ADV_ENABLE, ADV_HANDLE0);
    //////////////// Periodic Adv Initialization End ///////////////////////


    //  blc_ll_setAuxAdvChnIdxByCustomers(1); //debug chn_2406
    //  blc_ll_setMaxAdvDelay_for_AdvEvent(MAX_DELAY_0MS);
}


#endif // end of (FEATURE_TEST_MODE == TEST_PER_ADV)
