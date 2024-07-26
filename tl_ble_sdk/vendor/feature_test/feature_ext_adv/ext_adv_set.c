/********************************************************************************************************
 * @file    ext_adv_set.c
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

#include "app_config.h"
#include "app.h"


#if (FEATURE_TEST_MODE == TEST_EXT_ADV)




/**
 * @brief   BLE Advertising data
 */
const u8    tbl_advData[] = {
     8,  DT_COMPLETE_LOCAL_NAME,                'e','x','t', '_', 'a', 'd', 'v',
     2,  DT_FLAGS,                              0x05,                   // BLE limited discoverable mode and BR/EDR not supported
     3,  DT_APPEARANCE,                             0x80, 0x01,             // 384, Generic Remote Control, Generic category
     5,  DT_INCOMPLETE_LIST_16BIT_SERVICE_UUID, 0x12, 0x18, 0x0F, 0x18, // incomplete list of service class UUIDs (0x1812, 0x180F)
};

/**
 * @brief   BLE Scan Response Packet data
 */
const u8    tbl_scanRsp [] = {
     8,  DT_COMPLETE_LOCAL_NAME,                'e','x','t', '_', 'a', 'd', 'v',
};




const u8    tbl_advData_0 [] = {
     17,    DT_COMPLETE_LOCAL_NAME,                 'A','A','A','A','A','A','A','A','A','A','A','A','A','A','A','A',
     2,     DT_FLAGS,                               0x05,
     3,     DT_APPEARANCE,                          0x80, 0x01,
};

const u8    tbl_scanRsp_0 [] = {
     17,    DT_COMPLETE_LOCAL_NAME,                 'A','A','A','A','A','A','A','A','A','A','A','A','A','A','A','A',
};


const u8    tbl_advData_1 [] = {
     17,    DT_COMPLETE_LOCAL_NAME,                 'B','B','B','B','B','B','B','B','B','B','B','B','B','B','B','B',
     2,     DT_FLAGS,                               0x05,
     3,     DT_APPEARANCE,                          0x80, 0x01,
};

const u8    tbl_scanRsp_1 [] = {
     17,    DT_COMPLETE_LOCAL_NAME,                 'B','B','B','B','B','B','B','B','B','B','B','B','B','B','B','B',
};



const u8    tbl_advData_2 [] = {
     17,    DT_COMPLETE_LOCAL_NAME,                 'C','C','C','C','C','C','C','C','C','C','C','C','C','C','C','C',
     2,     DT_FLAGS,                               0x05,
     3,     DT_APPEARANCE,                          0x80, 0x01,
};

const u8    tbl_scanRsp_2 [] = {
     17,    DT_COMPLETE_LOCAL_NAME,                 'C','C','C','C','C','C','C','C','C','C','C','C','C','C','C','C',
};



const u8    tbl_advData_3 [] = {
     17,    DT_COMPLETE_LOCAL_NAME,                 'D','D','D','D','D','D','D','D','D','D','D','D','D','D','D','D',
     2,     DT_FLAGS,                               0x05,
     3,     DT_APPEARANCE,                          0x80, 0x01,
};

const u8    tbl_scanRsp_3 [] = {
     17,    DT_COMPLETE_LOCAL_NAME,                 'D','D','D','D','D','D','D','D','D','D','D','D','D','D','D','D',
};


typedef enum{
//Legacy, Unconnectable Undirected
  APP_LEGACY_UNCONNECTABLE          = 1,
  APP_LEGACY_UNCONNECTABLE_COMPLETE = 2,
  APP_LEGACY_UNCONNECTABLE_END      = 3,
//Legacy, Connectable_Scannable, Undirected
  APP_LEGACY_CONN_SCAN,
  APP_LEGACY_CONN_SCAN_COMPLETE,
  APP_LEGACY_CONN_SCAN_END,
//Legacy, Connectable_Scannable, Undirected
  APP_LEGACY_SCAN_UNDIRECTED,
  APP_LEGACY_SCAN_UNDIRECTED_COMPLETE,
  APP_LEGACY_SCAN_UNDIRECTED_END,
// Extended, None_Connectable_None_Scannable undirected, without auxiliary packet
  APP_EXT_NONE_CONN_NONE_SCAN_UNDIRECTED__WITHOUT_AUX_PKT,
  APP_EXT_NONE_CONN_NONE_SCAN_UNDIRECTED__WITHOUT_AUX_PKT_COMPLETE,
  APP_EXT_NONE_CONN_NONE_SCAN_UNDIRECTED__WITHOUT_AUX_PKT_END,
// Extended, None_Connectable_None_Scannable directed, without auxiliary packet
  APP_EXT_NONE_CONN_NONE_SCAN_DIRECTED__WITHOUT_AUX_PKT,
  APP_EXT_NONE_CONN_NONE_SCAN_DIRECTED__WITHOUT_AUX_PKT_COMPLETE,
  APP_EXT_NONE_CONN_NONE_SCAN_DIRECTED__WITHOUT_AUX_PKT_END,
// Extended, None_Connectable_None_Scannable undirected, with auxiliary packet
  APP_EXT_NONE_CONN_NONE_SCAN_UNDIRECTED__WITH_AUX_PKT,
  APP_EXT_NONE_CONN_NONE_SCAN_UNDIRECTED__WITH_AUX_PKT_COMPLETE,
  APP_EXT_NONE_CONN_NONE_SCAN_UNDIRECTED__WITH_AUX_PKT_END,
// Extended, None_Connectable_None_Scannable Directed, with auxiliary packet
  APP_EXT_NONE_CONN_NONE_SCAN_DIRECTED__WITH_AUX_PKT,
  APP_EXT_NONE_CONN_NONE_SCAN_DIRECTED__WITH_AUX_PKT_COMPLETE,
  APP_EXT_NONE_CONN_NONE_SCAN_DIRECTED__WITH_AUX_PKT_END,
// Extended, Scannable, Undirected
  APP_EXT_SCAN_UNDIRECTED,
  APP_EXT_SCAN_UNDIRECTED_COMPLETE,
  APP_EXT_SCAN_UNDIRECTED_END,
// Extended, Connectable, Undirected
  APP_EXT_CONN_UNDIRECTED,
  APP_EXT_CONN_UNDIRECTED_COMPLETE,
  APP_EXT_CONN_UNDIRECTED_END,
//4*Legacy, Unconnectable Undirected
  APP_LEGACY_UNCONNECTABLE_4,
  APP_LEGACY_UNCONNECTABLE_4_COMPLETE,
  APP_LEGACY_UNCONNECTABLE_4_END,
//4*Legacy, Connectable_Scannable, Undirected
  APP_LEGACY_CONN_SCAN_4,
  APP_LEGACY_CONN_SCAN_4_COMPLETE,
  APP_LEGACY_CONN_SCAN_4_END,
//adv_set 1: Extended, Connectable_scannable
//adv_set 2: Extended, None_Connectable_None_Scannable undirected, with auxiliary packet
//adv_set 3: Extended, Scannable, Undirected
//adv_set 4: Extended, Connectable, Undirected
  APP_LEGACY_CON_EXT_NCON_EXT_SCAN_EXT_CON,
  APP_LEGACY_CON_EXT_NCON_EXT_SCAN_EXT_CON_COMPLETE,
  APP_LEGACY_CON_EXT_NCON_EXT_SCAN_EXT_CON_END,

  APP_ADV_EVENT_PROPERTIES_END
}app_Advertising_Event_Properties_t;


_attribute_ble_data_retention_  u32 my_adv_select_ = ADV_INTERVAL_50MS;


_attribute_ble_data_retention_  u8  testAdvData[APP_EXT_ADV_DATA_LENGTH];
_attribute_ble_data_retention_  u8  testScanRspData[APP_EXT_SCANRSP_DATA_LENGTH];

_attribute_ble_data_retention_ app_Advertising_Event_Properties_t app_Advertising_Event_Properties = APP_LEGACY_CONN_SCAN;

void app_ext_adv_set_change(void)
{
   if((APP_LEGACY_UNCONNECTABLE_COMPLETE %3)== 2)
   {
     app_Advertising_Event_Properties++;
   }
   tlkapi_printf(APP_LOG_EN,"app_Advertising_Event_Properties = %d",app_Advertising_Event_Properties);
}

void app_ext_adv_set_test(void)
{
    switch(app_Advertising_Event_Properties)
    {
        //Legacy, non_connectable_non_scannable
        case APP_LEGACY_UNCONNECTABLE:
        {
            tlkapi_printf(APP_LOG_EN,"Legacy, non_connectable_non_scannable");
            blc_ll_setExtAdvParam( ADV_HANDLE0,         ADV_EVT_PROP_LEGACY_NON_CONNECTABLE_NON_SCANNABLE_UNDIRECTED,  ADV_INTERVAL_50MS,           ADV_INTERVAL_100MS,
                                   BLT_ENABLE_ADV_ALL,  OWN_ADDRESS_PUBLIC,                                            BLE_ADDR_PUBLIC,                 NULL,
                                   ADV_FP_NONE,         TX_POWER_3dBm,                                                 BLE_PHY_1M,                      0,
                                   BLE_PHY_1M,          ADV_SID_0,                                                     0);

            blc_ll_setExtAdvData(ADV_HANDLE0, sizeof(tbl_advData), (const u8 *)tbl_advData);

            blc_ll_setExtAdvEnable(BLC_ADV_ENABLE, ADV_HANDLE0, 0, 0);
            app_Advertising_Event_Properties++;
        }
        break;

        //Legacy, Connectable_Scannable, Undirected
        case APP_LEGACY_CONN_SCAN:
        {
            tlkapi_printf(APP_LOG_EN,"Legacy, Connectable_Scannable, Undirected");
            blc_ll_setExtAdvParam( ADV_HANDLE0,         ADV_EVT_PROP_LEGACY_CONNECTABLE_SCANNABLE_UNDIRECTED,  ADV_INTERVAL_50MS,           ADV_INTERVAL_100MS,
                                   BLT_ENABLE_ADV_ALL,  OWN_ADDRESS_PUBLIC,                                    BLE_ADDR_PUBLIC,                 NULL,
                                   ADV_FP_NONE,         TX_POWER_3dBm,                                         BLE_PHY_1M,                      0,
                                   BLE_PHY_1M,          ADV_SID_0,                                             0);

            blc_ll_setExtAdvData(ADV_HANDLE0, sizeof(tbl_advData), (const u8 *)tbl_advData);
            blc_ll_setExtScanRspData(ADV_HANDLE0, sizeof(tbl_scanRsp), (const u8 *)tbl_scanRsp);
            blc_ll_setExtAdvEnable(BLC_ADV_ENABLE, ADV_HANDLE0, 0, 0);
            app_Advertising_Event_Properties++;
        }
        break;

        //Legacy, Scannable undirected
        case APP_LEGACY_SCAN_UNDIRECTED:
        {
            tlkapi_printf(APP_LOG_EN,"Legacy, Connectable_Scannable, Undirected");
            blc_ll_setExtAdvParam( ADV_HANDLE0,         ADV_EVT_PROP_LEGACY_SCANNABLE_UNDIRECTED,       ADV_INTERVAL_50MS,          ADV_INTERVAL_100MS,
                                   BLT_ENABLE_ADV_ALL,  OWN_ADDRESS_PUBLIC,                             BLE_ADDR_PUBLIC,                NULL,
                                   ADV_FP_NONE,         TX_POWER_3dBm,                                  BLE_PHY_1M,                     0,
                                   BLE_PHY_1M,          ADV_SID_0,                                      0);

            blc_ll_setExtScanRspData(ADV_HANDLE0,  sizeof(tbl_scanRsp), (const u8 *)tbl_scanRsp);

            blc_ll_setExtAdvEnable(BLC_ADV_ENABLE, ADV_HANDLE0, 0, 0);
            app_Advertising_Event_Properties++;
        }
        break;

      // Extended, None_Connectable_None_Scannable undirected, without auxiliary packet
        case APP_EXT_NONE_CONN_NONE_SCAN_UNDIRECTED__WITHOUT_AUX_PKT:
        {
            tlkapi_printf(APP_LOG_EN,"Extended, None_Connectable_None_Scannable undirected, without auxiliary packet");
            u16 event_prop = ADV_EVT_PROP_EXTENDED_NON_CONNECTABLE_NON_SCANNABLE_UNDIRECTED | ADV_EVT_PROP_EXTENDED_MASK_TX_POWER_INCLUDE;

            blc_ll_setExtAdvParam( ADV_HANDLE0,         event_prop,                                                    ADV_INTERVAL_50MS,               ADV_INTERVAL_100MS,
                                   BLT_ENABLE_ADV_ALL,  OWN_ADDRESS_PUBLIC,                                            BLE_ADDR_PUBLIC,                 NULL,
                                   ADV_FP_NONE,         TX_POWER_3dBm,                                                 BLE_PHY_1M,                      0,
                                   BLE_PHY_1M,          ADV_SID_0,                                                     0);
            blc_ll_setExtAdvData( ADV_HANDLE0, 0 , NULL);
            blc_ll_setExtAdvEnable(BLC_ADV_ENABLE, ADV_HANDLE0, 0, 0);
            app_Advertising_Event_Properties++;
        }
        break;

    // Extended, None_Connectable_None_Scannable directed, without auxiliary packet
        case APP_EXT_NONE_CONN_NONE_SCAN_DIRECTED__WITHOUT_AUX_PKT:
        {
            tlkapi_printf(APP_LOG_EN,"Extended, None_Connectable_None_Scannable directed, without auxiliary packet");
            u8 test_peer_type = BLE_ADDR_PUBLIC;  // BLE_ADDR_RANDOM
            u8 test_peer_mac[6] = {0x11,0x11,0x11,0x11,0x11,0x11};
            blc_ll_setExtAdvParam( ADV_HANDLE0,         ADV_EVT_PROP_EXTENDED_NON_CONNECTABLE_NON_SCANNABLE_DIRECTED,  ADV_INTERVAL_50MS,               ADV_INTERVAL_100MS,
                                   BLT_ENABLE_ADV_ALL,  OWN_ADDRESS_PUBLIC,                                            test_peer_type,                  test_peer_mac,
                                   ADV_FP_NONE,         TX_POWER_3dBm,                                                 BLE_PHY_1M,                      0,
                                   BLE_PHY_1M,          ADV_SID_0,                                                     0);
            blc_ll_setExtAdvData( ADV_HANDLE0, 0 , NULL);
            blc_ll_setExtAdvEnable(BLC_ADV_ENABLE, ADV_HANDLE0, 0, 0);
            app_Advertising_Event_Properties++;
        }
        break;

        // Extended, None_Connectable_None_Scannable undirected, with auxiliary packet
        case APP_EXT_NONE_CONN_NONE_SCAN_UNDIRECTED__WITH_AUX_PKT:
        {
            tlkapi_printf(APP_LOG_EN,"Extended, None_Connectable_None_Scannable undirected, with auxiliary packet");
            blc_ll_setExtAdvParam( ADV_HANDLE0,         ADV_EVT_PROP_EXTENDED_NON_CONNECTABLE_NON_SCANNABLE_UNDIRECTED, ADV_INTERVAL_50MS,          ADV_INTERVAL_100MS,
                                   BLT_ENABLE_ADV_ALL,  OWN_ADDRESS_PUBLIC,                                             BLE_ADDR_PUBLIC,                NULL,
                                   ADV_FP_NONE,         TX_POWER_3dBm,                                                  BLE_PHY_1M,                     0,
                                   BLE_PHY_1M,          ADV_SID_0,                                                      0);
            for(int i=0;i<1024;i++){
                testAdvData[i] = i;
            }
            blc_ll_setExtAdvData(ADV_HANDLE0,  1024, testAdvData);
            blc_ll_setExtAdvEnable(BLC_ADV_ENABLE, ADV_HANDLE0, 0 , 0);
            app_Advertising_Event_Properties++;
        }
        break;

        // Extended, None_Connectable_None_Scannable Directed, with auxiliary packet
        case APP_EXT_NONE_CONN_NONE_SCAN_DIRECTED__WITH_AUX_PKT:
        {
            tlkapi_printf(APP_LOG_EN,"Extended, None_Connectable_None_Scannable Directed, with auxiliary packet");
            u8 test_peer_type = BLE_ADDR_RANDOM;  // BLE_ADDR_RANDOM
            u8 test_peer_mac[6] = {0x11,0x11,0x11,0x11,0x11,0x11};

            blc_ll_setExtAdvParam( ADV_HANDLE0,         ADV_EVT_PROP_EXTENDED_NON_CONNECTABLE_NON_SCANNABLE_DIRECTED,   ADV_INTERVAL_50MS,          ADV_INTERVAL_100MS,
                                   BLT_ENABLE_ADV_ALL,  OWN_ADDRESS_PUBLIC,                                             test_peer_type,                 test_peer_mac,
                                   ADV_FP_NONE,         TX_POWER_3dBm,                                                  BLE_PHY_1M,                     0,
                                   BLE_PHY_1M,          ADV_SID_0,                                                      0);
            for(int i=0;i<1024;i++){
                testAdvData[i]=i;
            }
            blc_ll_setExtAdvData(ADV_HANDLE0, 1024, testAdvData);
            blc_ll_setExtAdvEnable(BLC_ADV_ENABLE, ADV_HANDLE0, 0, 0);
            app_Advertising_Event_Properties++;
        }
        break;

      // Extended, Scannable, Undirected
        case APP_EXT_SCAN_UNDIRECTED:
        {
            tlkapi_printf(APP_LOG_EN,"Extended, Scannable, Undirected");
            blc_ll_setExtAdvParam( ADV_HANDLE0,         ADV_EVT_PROP_EXTENDED_SCANNABLE_UNDIRECTED,                     ADV_INTERVAL_50MS,          ADV_INTERVAL_100MS,
                                   BLT_ENABLE_ADV_ALL,  OWN_ADDRESS_PUBLIC,                                             BLE_ADDR_PUBLIC,                NULL,
                                   ADV_FP_NONE,         TX_POWER_3dBm,                                                  BLE_PHY_1M,                     0,
                                   BLE_PHY_1M,          ADV_SID_0,                                                      0);

            //Extended Scannable Event do not have ADV data
            for(int i=0;i<1024;i++){
                testScanRspData[i]=i;
            }
            blc_ll_setExtScanRspData(ADV_HANDLE0, 1024, testScanRspData);
            blc_ll_setExtAdvEnable(BLC_ADV_ENABLE, ADV_HANDLE0, 0, 0);
            app_Advertising_Event_Properties++;
        }
        break;

        // Extended, Connectable, Undirected
        case APP_EXT_CONN_UNDIRECTED:
        {
            tlkapi_printf(APP_LOG_EN,"Extended, Connectable, Undirected");
            blc_ll_initChannelSelectionAlgorithm_2_feature();
            //  blc_ll_setDefaultConnCodingIndication(CODED_PHY_PREFER_S2);
            blc_ll_setExtAdvParam( ADV_HANDLE0,         ADV_EVT_PROP_EXTENDED_CONNECTABLE_UNDIRECTED,                   ADV_INTERVAL_50MS,          ADV_INTERVAL_100MS,
                                   BLT_ENABLE_ADV_ALL,  OWN_ADDRESS_PUBLIC,                                             BLE_ADDR_PUBLIC,                NULL,
                                   ADV_FP_NONE,         TX_POWER_3dBm,                                                  BLE_PHY_1M,                     0,
                                   BLE_PHY_2M,          ADV_SID_0,                                                      0);

            blc_ll_setExtAdvData(ADV_HANDLE0, sizeof(tbl_advData), (const u8 *)tbl_advData);
            //Extended Connectable Event do not have scan_rsp data
            blc_ll_setExtAdvEnable(BLC_ADV_ENABLE, ADV_HANDLE0, 0, 0);
            app_Advertising_Event_Properties++;
        }
        break;

        case APP_LEGACY_UNCONNECTABLE_4:
        {
            /* adv_set 0: Legacy, Unconnectable Undirected
               adv_set 1: Legacy, Unconnectable Undirected
               adv_set 2: Legacy, Unconnectable Undirected
               adv_set 3: Legacy, Unconnectable Undirected */
            tlkapi_printf(APP_LOG_EN,"adv_set 0: Legacy, Unconnectable Undirected");
            blc_ll_setExtAdvParam( ADV_HANDLE0,         ADV_EVT_PROP_LEGACY_NON_CONNECTABLE_NON_SCANNABLE_UNDIRECTED,  ADV_INTERVAL_50MS,           ADV_INTERVAL_100MS,
                                   BLT_ENABLE_ADV_ALL,  OWN_ADDRESS_PUBLIC,                                            BLE_ADDR_PUBLIC,                 NULL,
                                   ADV_FP_NONE,         TX_POWER_3dBm,                                                 BLE_PHY_2M,                      0,
                                   BLE_PHY_1M,          ADV_SID_0,                                                     0);

            blc_ll_setExtAdvData(ADV_HANDLE0,  sizeof(tbl_advData_0), (const u8 *)tbl_advData_0);

            tlkapi_printf(APP_LOG_EN,"adv_set 1: Legacy, Unconnectable Undirected");
            blc_ll_setExtAdvParam( ADV_HANDLE1,         ADV_EVT_PROP_LEGACY_NON_CONNECTABLE_NON_SCANNABLE_UNDIRECTED,  ADV_INTERVAL_50MS,           ADV_INTERVAL_100MS,
                                   BLT_ENABLE_ADV_ALL,  OWN_ADDRESS_PUBLIC,                                            BLE_ADDR_PUBLIC,                 NULL,
                                   ADV_FP_NONE,         TX_POWER_3dBm,                                                 BLE_PHY_1M,                      0,
                                   BLE_PHY_2M,          ADV_SID_1,                                                     0);

            blc_ll_setExtAdvData(ADV_HANDLE1,  sizeof(tbl_advData_1), (const u8 *)tbl_advData_1);

            tlkapi_printf(APP_LOG_EN,"adv_set 2: Legacy, Unconnectable Undirected");
            blc_ll_setExtAdvParam( ADV_HANDLE2,         ADV_EVT_PROP_LEGACY_NON_CONNECTABLE_NON_SCANNABLE_UNDIRECTED,  ADV_INTERVAL_50MS,           ADV_INTERVAL_100MS,
                                   BLT_ENABLE_ADV_ALL,  OWN_ADDRESS_PUBLIC,                                            BLE_ADDR_PUBLIC,                 NULL,
                                   ADV_FP_NONE,         TX_POWER_3dBm,                                                 BLE_PHY_1M,                      0,
                                   BLE_PHY_CODED,       ADV_SID_2,                                                     0);

            blc_ll_setExtAdvData(ADV_HANDLE2,  sizeof(tbl_advData_2), (const u8 *)tbl_advData_2);

            tlkapi_printf(APP_LOG_EN,"adv_set 3: Legacy, Unconnectable Undirected");
            blc_ll_setExtAdvParam( ADV_HANDLE3,         ADV_EVT_PROP_LEGACY_NON_CONNECTABLE_NON_SCANNABLE_UNDIRECTED,  ADV_INTERVAL_50MS,           ADV_INTERVAL_100MS,
                                   BLT_ENABLE_ADV_ALL,  OWN_ADDRESS_PUBLIC,                                            BLE_ADDR_PUBLIC,                 NULL,
                                   ADV_FP_NONE,         TX_POWER_3dBm,                                                 BLE_PHY_1M,                      0,
                                   BLE_PHY_CODED,       ADV_SID_3,                                                     0);

            blc_ll_setExtAdvData(ADV_HANDLE3,  sizeof(tbl_advData_3), (const u8 *)tbl_advData_3);
            blc_ll_setExtAdvEnable(BLC_ADV_ENABLE, ADV_HANDLE0, 0, 0);
            blc_ll_setExtAdvEnable(BLC_ADV_ENABLE, ADV_HANDLE1, 0, 0);
            blc_ll_setExtAdvEnable(BLC_ADV_ENABLE, ADV_HANDLE2, 0, 0);
            blc_ll_setExtAdvEnable(BLC_ADV_ENABLE, ADV_HANDLE3, 0, 0);
            app_Advertising_Event_Properties++;
        }
        break;


        case APP_LEGACY_CONN_SCAN_4:
        {
            /* adv_set 0: Legacy, Connectable_Scannable, Undirected
               adv_set 1: Legacy, Connectable_Scannable, Undirected
               adv_set 2: Legacy, Connectable_Scannable, Undirected
               adv_set 3: Legacy, Connectable_Scannable, Undirected */
            tlkapi_printf(APP_LOG_EN,"adv_set 0: Legacy, Connectable_Scannable, Undirected");
            blc_ll_setExtAdvParam( ADV_HANDLE0,         ADV_EVT_PROP_LEGACY_CONNECTABLE_SCANNABLE_UNDIRECTED,          ADV_INTERVAL_50MS,           ADV_INTERVAL_100MS,
                                   BLT_ENABLE_ADV_ALL,  OWN_ADDRESS_PUBLIC,                                            BLE_ADDR_PUBLIC,                 NULL,
                                   ADV_FP_NONE,         TX_POWER_3dBm,                                                 BLE_PHY_1M,                      0,
                                   BLE_PHY_1M,          ADV_SID_0,                                                     0);

            blc_ll_setExtAdvData    (ADV_HANDLE0,  sizeof(tbl_advData_0), (const u8 *)tbl_advData_0);
            blc_ll_setExtScanRspData(ADV_HANDLE0,  sizeof(tbl_scanRsp_0), (const u8 *)tbl_scanRsp_0);
            tlkapi_printf(APP_LOG_EN,"adv_set 1: Legacy, Connectable_Scannable, Undirected");
            blc_ll_setExtAdvParam( ADV_HANDLE1,         ADV_EVT_PROP_LEGACY_CONNECTABLE_SCANNABLE_UNDIRECTED,          ADV_INTERVAL_50MS,           ADV_INTERVAL_100MS,
                                   BLT_ENABLE_ADV_ALL,  OWN_ADDRESS_PUBLIC,                                            BLE_ADDR_PUBLIC,                 NULL,
                                   ADV_FP_NONE,         TX_POWER_3dBm,                                                 BLE_PHY_1M,                      0,
                                   BLE_PHY_2M,          ADV_SID_1,                                                     0);

            blc_ll_setExtAdvData    (ADV_HANDLE1,  sizeof(tbl_advData_1), (const u8 *)tbl_advData_1);
            blc_ll_setExtScanRspData(ADV_HANDLE1,  sizeof(tbl_scanRsp_1), (const u8 *)tbl_scanRsp_1);
            tlkapi_printf(APP_LOG_EN,"adv_set 2: Legacy, Connectable_Scannable, Undirected");
            blc_ll_setExtAdvParam( ADV_HANDLE2,         ADV_EVT_PROP_LEGACY_CONNECTABLE_SCANNABLE_UNDIRECTED,          ADV_INTERVAL_50MS,           ADV_INTERVAL_100MS,
                                   BLT_ENABLE_ADV_ALL,  OWN_ADDRESS_PUBLIC,                                            BLE_ADDR_PUBLIC,                 NULL,
                                   ADV_FP_NONE,         TX_POWER_3dBm,                                                 BLE_PHY_1M,                      0,
                                   BLE_PHY_CODED,           ADV_SID_2,                                                     0);

            blc_ll_setExtAdvData    (ADV_HANDLE2,  sizeof(tbl_advData_2), (const u8 *)tbl_advData_2);
            blc_ll_setExtScanRspData(ADV_HANDLE2,  sizeof(tbl_scanRsp_2), (const u8 *)tbl_scanRsp_2);
            tlkapi_printf(APP_LOG_EN,"adv_set 3: Legacy, Connectable_Scannable, Undirected");
            blc_ll_setExtAdvParam( ADV_HANDLE3,         ADV_EVT_PROP_LEGACY_CONNECTABLE_SCANNABLE_UNDIRECTED,          ADV_INTERVAL_50MS,           ADV_INTERVAL_100MS,
                                   BLT_ENABLE_ADV_ALL,  OWN_ADDRESS_PUBLIC,                                            BLE_ADDR_PUBLIC,                 NULL,
                                   ADV_FP_NONE,         TX_POWER_3dBm,                                                 BLE_PHY_1M,                      0,
                                   BLE_PHY_CODED,       ADV_SID_3,                                                     0);

            blc_ll_setExtAdvData    (ADV_HANDLE3,  sizeof(tbl_advData_3), (const u8 *)tbl_advData_3);
            blc_ll_setExtScanRspData(ADV_HANDLE3,  sizeof(tbl_scanRsp_3), (const u8 *)tbl_scanRsp_3);
            blc_ll_setExtAdvEnable(BLC_ADV_ENABLE, ADV_HANDLE0, 0, 0);
            blc_ll_setExtAdvEnable(BLC_ADV_ENABLE, ADV_HANDLE1, 0, 0);
            blc_ll_setExtAdvEnable(BLC_ADV_ENABLE, ADV_HANDLE2, 0, 0);
            blc_ll_setExtAdvEnable(BLC_ADV_ENABLE, ADV_HANDLE3, 0, 0);
            app_Advertising_Event_Properties++;
        }
        break;



        case APP_LEGACY_CON_EXT_NCON_EXT_SCAN_EXT_CON:
        {
            //adv_set 1: Extended, Connectable_scannable
            //adv_set 2: Extended, None_Connectable_None_Scannable undirected, with auxiliary packet
            //adv_set 3: Extended, Scannable, Undirected
            //adv_set 4: Extended, Connectable, Undirected
            for(int i=0;i<1024;i++){
                testAdvData[i]=i;
                testScanRspData[i]=i;
            }
            blc_ll_initChannelSelectionAlgorithm_2_feature();
            blc_ll_setDefaultConnCodingIndication(CODED_PHY_PREFER_S8);
            //adv_set 1: Extended, Connectable_scannable
            tlkapi_printf(APP_LOG_EN,"adv_set 1: Extended, Connectable_scannable");
            blc_ll_setExtAdvParam( ADV_HANDLE0,         ADV_EVT_PROP_EXTENDED_CONNECTABLE_UNDIRECTED,                   ADV_INTERVAL_50MS,          ADV_INTERVAL_100MS,
                                   BLT_ENABLE_ADV_ALL,  OWN_ADDRESS_PUBLIC,                                             BLE_ADDR_PUBLIC,                NULL,
                                   ADV_FP_NONE,         TX_POWER_3dBm,                                                  BLE_PHY_1M,                     0,
                                   BLE_PHY_1M,          ADV_SID_0,                                                      0);

            blc_ll_setExtAdvData( ADV_HANDLE0,  sizeof(tbl_advData_1) , (const u8 *)tbl_advData_1);
            //adv_set 2: Extended, None_Connectable_None_Scannable undirected, with auxiliary packet
            tlkapi_printf(APP_LOG_EN,"adv_set 2: Extended, None_Connectable_None_Scannable undirected, with auxiliary packet");
            blc_ll_setExtAdvParam( ADV_HANDLE1,         ADV_EVT_PROP_EXTENDED_NON_CONNECTABLE_NON_SCANNABLE_UNDIRECTED,  ADV_INTERVAL_50MS,             ADV_INTERVAL_100MS,
                                   BLT_ENABLE_ADV_ALL,  OWN_ADDRESS_PUBLIC,                                              BLE_ADDR_PUBLIC,               NULL,
                                   ADV_FP_NONE,         TX_POWER_3dBm,                                                   BLE_PHY_1M,                    0,
                                   BLE_PHY_2M,          ADV_SID_1,                                                       0);

            blc_ll_setExtAdvData( ADV_HANDLE1, 1024, testAdvData);
            // adv_set 3: Extended, Scannable, Undirected
            tlkapi_printf(APP_LOG_EN,"adv_set 3: Extended, Scannable, Undirected");
            blc_ll_setExtAdvParam( ADV_HANDLE2,         ADV_EVT_PROP_EXTENDED_SCANNABLE_UNDIRECTED,                      ADV_INTERVAL_50MS,             ADV_INTERVAL_100MS,
                                   BLT_ENABLE_ADV_ALL,  OWN_ADDRESS_PUBLIC,                                              BLE_ADDR_PUBLIC,               NULL,
                                   ADV_FP_NONE,         TX_POWER_3dBm,                                                   BLE_PHY_1M,                    0,
                                   BLE_PHY_1M,          ADV_SID_2,                                                       0);

            blc_ll_setExtAdvData    ( ADV_HANDLE2,  1024, testAdvData);
            blc_ll_setExtScanRspData( ADV_HANDLE2,  1024, testScanRspData);

            tlkapi_printf(APP_LOG_EN,"adv_set 4: Extended, Connectable, Undirected");
            blc_ll_setExtAdvParam( ADV_HANDLE3,         ADV_EVT_PROP_EXTENDED_CONNECTABLE_UNDIRECTED,                   ADV_INTERVAL_50MS,          ADV_INTERVAL_100MS,
                                   BLT_ENABLE_ADV_ALL,  OWN_ADDRESS_PUBLIC,                                             BLE_ADDR_PUBLIC,                NULL,
                                   ADV_FP_NONE,         TX_POWER_3dBm,                                                  BLE_PHY_1M,                     0,
                                   BLE_PHY_2M,          ADV_SID_3,                                                      0);

            blc_ll_setExtAdvData    ( ADV_HANDLE3,  1024, testAdvData);
            blc_ll_setExtScanRspData( ADV_HANDLE3,  1024, testScanRspData);
            blc_ll_setExtAdvEnable(BLC_ADV_ENABLE, ADV_HANDLE0, 0, 0);
            blc_ll_setExtAdvEnable(BLC_ADV_ENABLE, ADV_HANDLE1, 0, 0);
            blc_ll_setExtAdvEnable(BLC_ADV_ENABLE, ADV_HANDLE2, 0, 0);
            blc_ll_setExtAdvEnable(BLC_ADV_ENABLE, ADV_HANDLE3, 0, 0);
            app_Advertising_Event_Properties++;
        }
        break;

        case APP_LEGACY_UNCONNECTABLE_END:
        case APP_LEGACY_CONN_SCAN_END:
        case APP_LEGACY_SCAN_UNDIRECTED_END:
        case APP_EXT_NONE_CONN_NONE_SCAN_UNDIRECTED__WITHOUT_AUX_PKT_END:
        case APP_EXT_NONE_CONN_NONE_SCAN_DIRECTED__WITHOUT_AUX_PKT_END:
        case APP_EXT_NONE_CONN_NONE_SCAN_UNDIRECTED__WITH_AUX_PKT_END:
        case APP_EXT_NONE_CONN_NONE_SCAN_DIRECTED__WITH_AUX_PKT_END:
        case APP_EXT_SCAN_UNDIRECTED_END:
        case APP_EXT_CONN_UNDIRECTED_END:
            blc_ll_setExtAdvEnable(BLC_ADV_DISABLE, ADV_HANDLE0, 0, 0);
            app_Advertising_Event_Properties++;
            tlkapi_printf(APP_LOG_EN,"BLC_ADV_DISABLE");
        break;

        case APP_LEGACY_UNCONNECTABLE_4_END:
        case APP_LEGACY_CONN_SCAN_4_END:
        case APP_LEGACY_CON_EXT_NCON_EXT_SCAN_EXT_CON_END:
            blc_ll_setExtAdvEnable(BLC_ADV_DISABLE, ADV_HANDLE0, 0, 0);
            blc_ll_setExtAdvEnable(BLC_ADV_DISABLE, ADV_HANDLE1, 0, 0);
            blc_ll_setExtAdvEnable(BLC_ADV_DISABLE, ADV_HANDLE2, 0, 0);
            blc_ll_setExtAdvEnable(BLC_ADV_DISABLE, ADV_HANDLE3, 0, 0);
            app_Advertising_Event_Properties++;
            tlkapi_printf(APP_LOG_EN,"BLC_ADV_DISABLE");
        break;

        case APP_ADV_EVENT_PROPERTIES_END:
            app_Advertising_Event_Properties = 1; //goto start
            tlkapi_printf(APP_LOG_EN,"app_Advertising_Event_Properties = %d",app_Advertising_Event_Properties);
        break;

        default:
            //todo
            if((app_Advertising_Event_Properties%3)==0)
            {
              tlkapi_printf(APP_LOG_EN,"You need to manually add the closing function"); //You can refer to: APP_LEGACY_UNCONNECTABLE_4_END
            }
        break;
    }
    //  blc_ll_setMaxAdvDelay_for_AdvEvent(MAX_DELAY_0MS);  //debug
}

#endif //end of (FEATURE_TEST_MODE == ...)
