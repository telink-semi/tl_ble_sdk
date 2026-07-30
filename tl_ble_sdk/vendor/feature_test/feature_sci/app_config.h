/********************************************************************************************************
 * @file    app_config.h
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

#include "../feature_config.h"

#if (FEATURE_TEST_MODE == TEST_FEATURE_SCI)


    /////////////////////// Role Selection //////////////////////////////////////////////////////
    /* Select role via IDE compile macro:
     *   - Default (no macro): SCI_PERIPHERAL, compiles as peripheral (advertising, wait for central)
     *   - Define SCI_CENTRAL in IDE "Defined symbols (-D)": compiles as central (scanning, auto-connect)
     * Two devices are needed for bidirectional test: one flashed as peripheral (default),
     * the other flashed as central (add SCI_CENTRAL macro in IDE).
     */

     #define SCI_PERIPHERAL 0
     #define SCI_CENTRAL 1

    /////////////////////// ACL Connection Number ////////////////////////////////////////////////
    /* Single connection per device: only the active role keeps 1 link. */
    #if (SCI_PERIPHERAL)
        #define ACL_CENTRAL_MAX_NUM 0
        #define ACL_PERIPHR_MAX_NUM 1
    #elif (SCI_CENTRAL)
        #define ACL_CENTRAL_MAX_NUM 1
        #define ACL_PERIPHR_MAX_NUM 0
    #endif


    /////////////////////// Feature Configuration////////////////////////////////////////////////
    #define ACL_PERIPHR_SMP_ENABLE                         0 //no security, simplified for test
    #define ACL_CENTRAL_SMP_ENABLE                         0 //no security, simplified for test
    #define ACL_CENTRAL_SIMPLE_SDP_ENABLE                  0 //use fixed ATT handle, no SDP


    #define BLE_APP_PM_ENABLE                              0


    #define APP_DEFAULT_BUFFER_ACL_OCTETS_MTU_SIZE_MINIMUM 1
    #define APP_DEFAULT_HID_BATTERY_OTA_ATTRIBUTE_TABLE    1


    /////////////////////// SCI Test Configuration //////////////////////////////////////////////
    /* Device name used in advertising and scan filter, both roles must use the same name. */
    #define SCI_DEVICE_NAME                "sci_test"
    #define SCI_DEVICE_NAME_LEN            8

    /* Test packet payload: numeric pattern fill, polling through 4 patterns. */
    #define SCI_TX_PAYLOAD_LEN             3
    #define SCI_TX_PATTERN_NUM             4
    #define SCI_TX_PATTERN_TBL             {0x11, 0x22, 0x33}

    /* SCI connection rate parameters applied in blc_ll_handle_HostConnRateReq.
     * NOTE: SCI connIntvl unit is 125us (NOT 1.25ms), supervisionTimeout unit is 10ms. */

    #define SCI_CONN_LATENCY               0
    #define SCI_CONN_TIMEOUT               200 //unit 10ms, 200 = 2s

    /* Connection setup state machine parameters (key 3 trigger).
     * Sequence: getRemoteSupportedFeatures -> setPhy -> frameSpaceUpdate. */

    /* PHY update: prefer 2M for both TX/RX to reduce air time. */
    #define SCI_SETUP_PHY_ALL_PHYS         PHY_TRX_PREFER       //has preference for TX & RX PHY
    #define SCI_SETUP_PHY_TX               PHY_PREFER_2M        //TX prefer 2M
    #define SCI_SETUP_PHY_RX               PHY_PREFER_2M        //RX prefer 2M
    #define SCI_SETUP_PHY_OPTIONS          CODEDPHY_PREFER_NONE //no coded PHY preference

    /* Frame space update: min frame spacing for ACL CP & PC directions.
     * Unit: us. Range: 80 ~ 6800. Smaller value = lower latency. */
    #define SCI_SETUP_FS_MIN               60   //us, minimum supported frame space
    #define SCI_SETUP_FS_MAX               70   //us, maximum supported frame space
    #define SCI_SETUP_FS_PHY_MASK          (PHY_PREFER_1M | PHY_PREFER_2M) //apply to 1M & 2M PHY
    #define SCI_SETUP_FS_SPACING_TYPE      (FSU_ST_ACL_CP | FSU_ST_ACL_PC|FSU_ST_ACL_MCES)  //ACL both directions

    /* Auto reconnect after disconnect. */
    #define SCI_AUTO_RECONNECT_EN          1
    #define SCI_RECONNECT_DELAY_MS         500 //delay before re-enabling adv/scan


    /////////////////////// UI Configuration ////////////////////////////////////////////////////
    #define UI_LED_ENABLE      1
    #define UI_KEYBOARD_ENABLE 1

    /////////////////////// DEBUG  Configuration ////////////////////////////////////////////////
    #define DEBUG_GPIO_ENABLE     0

    #define TLKAPI_DEBUG_ENABLE   1
    #define TLKAPI_DEBUG_CHANNEL  TLKAPI_DEBUG_CHANNEL_GSUART

    #define APP_LOG_EN            1
    #define APP_CONTR_EVT_LOG_EN  1 //controller event
    #define APP_HOST_EVT_LOG_EN   0
    #define APP_SMP_LOG_EN        0
    #define APP_SIMPLE_SDP_LOG_EN 0
    #define APP_PAIR_LOG_EN       0
    #define APP_KEY_LOG_EN        1


    #include "../../common/default_config.h"

#endif //end of (FEATURE_TEST_MODE == ...)
