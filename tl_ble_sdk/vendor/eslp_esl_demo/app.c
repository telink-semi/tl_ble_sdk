/********************************************************************************************************
 * @file    app.c
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
#include "app_esl.h"
#include "app_buffer.h"

#define APP_PAWR_SYNC_RSP_DATA_LENGTH 100
#define APP_PAWR_SYNC_SETS_NUMBER     1

_attribute_ble_data_retention_ int central_smp_pending = 0; // SMP: security & encryption;
_attribute_ble_data_retention_ u8 ota_is_working = 0;
/***************** Periodic Advertising Response Data Buffer allocation, start *************************************/

_attribute_ble_data_retention_ u8 app_pawr_sync_rsp_buf[APP_PAWR_SYNC_RSP_DATA_LENGTH * APP_PAWR_SYNC_SETS_NUMBER] = {0};
/***************** Periodic Advertising Response Data Buffer allocation, end  **************************************/
#if (!LEGACY_ADV_SEND)
    #define APP_EXT_ADV_SETS_NUMBER     1   //user set value
    #define APP_EXT_ADV_DATA_LENGTH     100 //2048//1664//1024   //user set value
    #define APP_EXT_SCANRSP_DATA_LENGTH 100 //2048//1664//1024   //user set value

_attribute_data_retention_ u8 app_extAdvSetParam_buf[ADV_SET_PARAM_LENGTH * APP_EXT_ADV_SETS_NUMBER];
_attribute_data_retention_ u8 app_extAdvData_buf[APP_EXT_ADV_DATA_LENGTH * APP_EXT_ADV_SETS_NUMBER];
_attribute_data_retention_ u8 app_extScanRspData_buf[APP_EXT_SCANRSP_DATA_LENGTH * APP_EXT_ADV_SETS_NUMBER];
#endif //(!LEGACY_ADV_SEND)

#define APP_ESL_NAME_ADDR               (0x7f000)
#define APP_ESL_MAX_DEVICE_NAME_LEN     18
#define APP_ESL_MAX_DEVICE_NAME_LTV_LEN (APP_ESL_MAX_DEVICE_NAME_LEN + 2)
const u8 app_esl_default_device_name[] = {'t', 'e', 'l', 'i', 'n', 'k', ' ', 'e', 's', 'l'};

/**
 * @brief   BLE Advertising data
 */
const u8 tbl_advData[] = {
    2,
    DT_FLAGS,
    0x05, // BLE limited discoverable mode and BR/EDR not supported
    3,
    DT_APPEARANCE,
    0xC2,
    0x0A, // 0x0AC2, Electronic Label
    3,
    DT_INCOMPLETE_LIST_16BIT_SERVICE_UUID,
    0x57,
    0x18, // incomplete list of service class UUIDs (0x1857)
};

//////////////////////////////////////////////////////////
// event call back
//////////////////////////////////////////////////////////
/**
 * @brief      BLE controller event handler call-back.
 * @param[in]  h       event type
 * @param[in]  p       Pointer point to event parameter buffer.
 * @param[in]  n       the length of event parameter.
 * @return
 */
int app_controller_event_callback(u32 h, u8 *p, int n)
{
    app_esl_handle_controller_event(h, p, n);

    return 0;
}

/**
 * @brief      BLE host event handler call-back.
 * @param[in]  h       event type
 * @param[in]  para    Pointer point to event parameter buffer.
 * @param[in]  n       the length of event parameter.
 * @return
 */
int app_host_event_callback(u32 h, u8 *para, int n)
{
    app_esl_handle_host_event(h, para, n);

    return 0;
}

static u8 app_read_device_name_ltv(int flash_addr, u8 *adv_data)
{
    advData_completeLocalName_t *ltv = (advData_completeLocalName_t *)adv_data;

    if (!flash_addr) {
        return 0;
    }

    flash_read_page(flash_addr, sizeof(*ltv), (u8 *)ltv);
    if ((ltv->header.type == DT_COMPLETE_LOCAL_NAME) &&
        (ltv->header.length <= APP_ESL_MAX_DEVICE_NAME_LEN) &&
        (ltv->header.length > 1)) {
        flash_read_page(flash_addr + sizeof(*ltv), ltv->header.length - 1, ltv->completeLocalName);
    } else {
        ltv->header.type   = DT_COMPLETE_LOCAL_NAME;
        ltv->header.length = sizeof(app_esl_default_device_name) + 1;
        memcpy(ltv->completeLocalName, app_esl_default_device_name, sizeof(app_esl_default_device_name));
        flash_write_page(flash_addr, ltv->header.length + 1, (u8 *)ltv);
    }

    return ltv->header.length + 1;
}

/**
 * @brief      BLE GATT data handler call-back.
 * @param[in]  connHandle     connection handle.
 * @param[in]  pkt             Pointer point to data packet buffer.
 * @return
 */
int app_gatt_data_handler(u16 connHandle, u8 *pkt)
{
    (void)pkt;
    (void)connHandle;

    return 0;
}

///////////////////////////////////////////
_attribute_data_retention_ u32 g_scan_startTick = 0;

static void set_adv_data(void)
{
    u8 adv_data[sizeof(tbl_advData) + APP_ESL_MAX_DEVICE_NAME_LTV_LEN];
    u8 adv_data_len;
    u8 scan_rsp_data[APP_ESL_MAX_DEVICE_NAME_LTV_LEN];
    u8 scan_rsp_data_len;

    // Scan response holds only device name
    scan_rsp_data_len = app_read_device_name_ltv(0x7f000, scan_rsp_data);

    memcpy(adv_data, tbl_advData, sizeof(tbl_advData));
    // Append content of scan response to adv data
    memcpy(adv_data + sizeof(tbl_advData), scan_rsp_data, scan_rsp_data_len);
    adv_data_len = sizeof(tbl_advData) + scan_rsp_data_len;

#if (LEGACY_ADV_SEND)
    blc_ll_setAdvData(adv_data, adv_data_len);
    blc_ll_setScanRspData(scan_rsp_data, scan_rsp_data_len);
#else
    blc_ll_setExtAdvData(ADV_HANDLE0, adv_data_len, adv_data);
    blc_ll_setExtScanRspData(ADV_HANDLE0, scan_rsp_data_len, scan_rsp_data);
#endif
}

/**
 * @brief       user initialization when MCU power on or wake_up from deepSleep mode
 * @param[in]   none
 * @return      none
 */
_attribute_no_inline_ void user_init_normal(void) //app_controller_event_callback
{
    g_scan_startTick = clock_time() | 0x01;
    //////////////////////////// basic hardware Initialization  Begin //////////////////////////////////
    /* random number generator must be initiated here( in the beginning of user_init_normal).
     * When deepSleep retention wakeUp, no need initialize again */
    random_generator_init();

#if (TLKAPI_DEBUG_ENABLE)
    tlkapi_debug_init();
    blc_debug_enableStackLog(STK_LOG_NONE);
#endif

    blc_readFlashSize_autoConfigCustomFlashSector();

    /* attention that this function must be called after "blc readFlashSize_autoConfigCustomFlashSector" !!!*/
    blc_app_loadCustomizedParameters_normal();
    //////////////////////////// basic hardware Initialization  End /////////////////////////////////


    //////////////////////////// BLE stack Initialization  Begin //////////////////////////////////

    u8 mac_public[6];
    u8 mac_random_static[6];
    /* Note: If change IC type, need to confirm the FLASH_SIZE_CONFIG */
    blc_initMacAddress(flash_sector_mac_address, mac_public, mac_random_static);


    //////////// LinkLayer Initialization  Begin /////////////////////////
    blc_ll_initBasicMCU();
    blc_ll_initStandby_module(mac_public);
    blc_ll_setRandomAddr(mac_random_static);

    blc_ll_initLegacyAdvertising_module(); //maybe PAST

    blc_ll_initAclConnection_module();
    blc_ll_initAclPeriphrRole_module();

    //when not use PAST need to enable extended scan.
    blc_ll_initExtendedScanning_module();
    blc_ll_configScanEnableStrategy(SCAN_STRATEGY_1);


    blc_ll_initPeriodicAdvertisingSynchronization_module(); //PADVB sync


    blc_ll_setMaxConnectionNumber(ACL_CENTRAL_MAX_NUM, ACL_PERIPHR_MAX_NUM);

    blc_ll_setAclConnMaxOctetsNumber(ACL_CONN_MAX_RX_OCTETS, ACL_CENTRAL_MAX_TX_OCTETS, ACL_PERIPHR_MAX_TX_OCTETS);

    /* all ACL connection share same RX FIFO */
    blc_ll_initAclConnRxFifo(app_acl_rx_fifo, ACL_RX_FIFO_SIZE, ACL_RX_FIFO_NUM);
    /* ACL Peripheral TX FIFO */
    blc_ll_initAclPeriphrTxFifo(app_acl_per_tx_fifo, ACL_PERIPHR_TX_FIFO_SIZE, ACL_PERIPHR_TX_FIFO_NUM, ACL_PERIPHR_MAX_NUM);

    blc_ll_setAclCentralBaseConnectionInterval(CONN_INTERVAL_7P5MS);


    /////////////////// PAwR Initial Start/////////////////////////
    blc_ll_initPAwRsync_module(1); //PAwR sync
    blc_ll_initPAwRsync_rspDataBuffer(app_pawr_sync_rsp_buf, APP_PAWR_SYNC_RSP_DATA_LENGTH);
    /////////////////// PAwR Initial End/////////////////////////

    /////////////////PAST module Start/////////////////////
    blc_ll_initPAST_module();
    hci_le_dftPastParamsCmdParams_t pastParam;

    pastParam.mode        = PAST_MODE_RPT_ENABLED_DUP_FILTER_DIS;
    pastParam.skip        = 0;
    pastParam.syncTimeout = 200;
    pastParam.cteType     = PAST_CTE_TYPE_SYNC_TO_WITHOUT_CTE;

    blc_hci_le_setDftPeriodicAdvSyncTransferParams(&pastParam);
    /////////////////PAST module End/////////////////////

    //////////// LinkLayer Initialization  End /////////////////////////


    //////////// HCI Initialization  Begin /////////////////////////
    blc_hci_registerControllerDataHandler(blc_l2cap_pktHandler_5_3);

    blc_hci_registerControllerEventHandler(app_controller_event_callback); //controller hci event to host all processed in this func

    //bluetooth event
    blc_hci_setEventMask_cmd(HCI_EVT_MASK_DISCONNECTION_COMPLETE);

    //bluetooth low energy(LE) event
    blc_hci_le_setEventMask_cmd(HCI_LE_EVT_MASK_CONNECTION_COMPLETE | HCI_LE_EVT_MASK_ADVERTISING_REPORT | HCI_LE_EVT_MASK_CONNECTION_UPDATE_COMPLETE | HCI_LE_EVT_MASK_ENHANCED_CONNECTION_COMPLETE | HCI_LE_EVT_MASK_EXTENDED_ADVERTISING_REPORT | HCI_LE_EVT_MASK_PERIODIC_ADVERTISING_SYNC_ESTABLISHED | HCI_LE_EVT_MASK_PERIODIC_ADVERTISING_REPORT | HCI_LE_EVT_MASK_PERIODIC_ADVERTISING_SYNC_LOST | HCI_LE_EVT_MASK_PERIODIC_ADVERTISING_SYNC_TRANSFER_RECEIVED);


    u8 error_code = blc_contr_checkControllerInitialization();
    if (error_code != INIT_SUCCESS) {
        /* It's recommended that user set some UI alarm to know the exact error, e.g. LED shine, print log */
        write_log32(0x88880000 | error_code);
#if (TLKAPI_DEBUG_ENABLE)
        tlkapi_send_string_data(APP_LOG_EN, "[APP][INI] Controller INIT ERROR", &error_code, 1);
        while (1) {
            tlkapi_debug_handler();
        }
#else
        while (1)
            ;
#endif
    }
    //////////// HCI Initialization  End /////////////////////////


    //////////// Host Initialization  Begin /////////////////////////
    /* Host Initialization */
    /* GAP initialization must be done before any other host feature initialization !!! */
    blc_gap_init();

    /* L2CAP data buffer Initialization */
    blc_l2cap_initAclPeripheralBuffer(app_per_l2cap_rx_buf, PERIPHR_L2CAP_BUFF_SIZE, app_per_l2cap_tx_buf, PERIPHR_L2CAP_BUFF_SIZE);

    blc_att_setCentralRxMtuSize(CENTRAL_ATT_RX_MTU);    ///must be placed after "blc_gap_init"
    blc_att_setPeripheralRxMtuSize(PERIPHR_ATT_RX_MTU); ///must be placed after "blc_gap_init"

/* GATT Initialization */
#if (ACL_CENTRAL_SIMPLE_SDP_ENABLE)
    host_att_register_idle_func(main_idle_loop);
#endif
    blc_gatt_register_data_handler(app_gatt_data_handler);

/* SMP Initialization */
#if (ACL_PERIPHR_SMP_ENABLE || ACL_CENTRAL_SMP_ENABLE)
    /* Note: If change IC type, need to confirm the FLASH_SIZE_CONFIG */
    blc_smp_configPairingSecurityInfoStorageAddressAndSize(flash_sector_smp_storage, FLASH_SMP_PAIRING_MAX_SIZE);
#endif

#if (ACL_PERIPHR_SMP_ENABLE)                                                   //Peripheral SMP Enable
    blc_smp_setSecurityLevel_periphr(Unauthenticated_Pairing_with_Encryption); //LE_Security_Mode_1_Level_2
#else
    blc_smp_setSecurityLevel_periphr(No_Security);
#endif

#if (ACL_CENTRAL_SMP_ENABLE)
    blc_smp_setSecurityLevel_central(Unauthenticated_Pairing_with_Encryption); //LE_Security_Mode_1_Level_2
#else
    blc_smp_setSecurityLevel_central(No_Security);
#endif

    blc_smp_smpParamInit();
    blc_smp_eraseAllBondingInfo();
    blc_smp_setBondingDeviceMaxNumber(0, 1);
    blc_smp_setDevExceedMaxStrategy(NEW_DEVICE_REJECT_WHEN_PER_MAX_BONDING_NUM);


    //host(GAP/SMP/GATT/ATT) event process: register host event callback and set event mask
    blc_gap_registerHostEventHandler(app_host_event_callback);
    blc_gap_setEventMask(GAP_EVT_MASK_SMP_PAIRING_BEGIN |
                         GAP_EVT_MASK_SMP_PAIRING_SUCCESS |
                         GAP_EVT_MASK_SMP_PAIRING_FAIL |
                         GAP_EVT_MASK_L2CAP_COC_CONNECT |
                         GAP_EVT_MASK_L2CAP_COC_DISCONNECT |
                         GAP_EVT_MASK_L2CAP_COC_RECONFIGURE |
                         GAP_EVT_MASK_L2CAP_COC_RECV_DATA |
                         GAP_EVT_MASK_L2CAP_COC_SEND_DATA_FINISH |
                         GAP_EVT_MASK_L2CAP_COC_CREATE_CONNECT_FINISH |
                         GAP_EVT_MASK_SMP_SECURITY_PROCESS_DONE);

    blc_prf_initPairingInfoStoreModule();
    //////////// Host Initialization  End /////////////////////////

//////////////////////////// BLE stack Initialization  End //////////////////////////////////


//////////////////////////// User Configuration for BLE application ////////////////////////////
#if (LEGACY_ADV_SEND)
    set_adv_data();
    blc_ll_setAdvParam(ADV_INTERVAL_200MS, ADV_INTERVAL_200MS, ADV_TYPE_CONNECTABLE_UNDIRECTED, OWN_ADDRESS_PUBLIC, 0, NULL, BLT_ENABLE_ADV_ALL, ADV_FP_NONE);
    blc_ll_setAdvEnable(BLC_ADV_ENABLE); //ADV enable
#else
    blc_ll_initExtendedAdvModule_initExtendedAdvSetParamBuffer(app_extAdvSetParam_buf, APP_EXT_ADV_SETS_NUMBER);
    blc_ll_initExtendedAdvDataBuffer(app_extAdvData_buf, APP_EXT_ADV_DATA_LENGTH);
    blc_ll_initExtendedScanRspDataBuffer(app_extScanRspData_buf, APP_EXT_SCANRSP_DATA_LENGTH);

    blc_ll_setExtAdvParam(ADV_HANDLE0, ADV_EVT_PROP_EXTENDED_CONNECTABLE_UNDIRECTED, ADV_INTERVAL_50MS, ADV_INTERVAL_100MS, BLT_ENABLE_ADV_ALL, OWN_ADDRESS_PUBLIC, BLE_ADDR_PUBLIC, NULL, ADV_FP_NONE, TX_POWER_3dBm, BLE_PHY_1M, 0, BLE_PHY_1M, ADV_SID_0, 0);
    set_adv_data();
    blc_ll_setExtAdvEnable(BLC_ADV_ENABLE, ADV_HANDLE0, 0, 0);
#endif

    /*    blc_ll_setExtScanParam( OWN_ADDRESS_PUBLIC, SCAN_FP_ALLOW_ADV_ANY, SCAN_PHY_1M_CODED, \
                            SCAN_TYPE_PASSIVE,  SCAN_INTERVAL_90MS,   SCAN_WINDOW_90MS, \
                            SCAN_TYPE_PASSIVE,  SCAN_INTERVAL_90MS,   SCAN_WINDOW_90MS);

  blc_ll_setExtScanEnable( BLC_SCAN_ENABLE, DUP_FILTER_DISABLE, SCAN_DURATION_CONTINUOUS, SCAN_WINDOW_CONTINUOUS);*/

    rf_set_power_level_index(RF_POWER_P3dBm);
    app_esl_init();
#if (BLE_APP_PM_ENABLE)
    blc_ll_initPowerManagement_module();
    blc_pm_setSleepMask(PM_SLEEP_LEG_ADV | PM_SLEEP_ACL_PERIPHR | PM_SLEEP_PAWRS_RSP | PM_SLEEP_PAWRS_SUB);

    #if (PM_DEEPSLEEP_RETENTION_ENABLE)
    blc_pm_setDeepsleepRetentionEnable(PM_DeepRetn_Enable);
    blc_pm_setDeepsleepRetentionThreshold(95);

        /*!< early wakeup time with a threshold of approxiamtely 30us. */
        #if (MCU_CORE_TYPE == MCU_CORE_B91)
    blc_pm_setDeepsleepRetentionEarlyWakeupTiming(530);
        #elif (MCU_CORE_TYPE == MCU_CORE_TL321X)
    blc_pm_setDeepsleepRetentionEarlyWakeupTiming(540);
        #elif (MCU_CORE_TYPE == MCU_CORE_TL721X)
    blc_pm_setDeepsleepRetentionEarlyWakeupTiming(580);
        #endif
    #else
    blc_pm_setDeepsleepRetentionEnable(PM_DeepRetn_Disable);
    #endif

    //        blc_ll_registerTelinkControllerEventCallback (BLT_EV_FLAG_SUSPEND_EXIT, &user_set_flag_suspend_exit);
#endif
#if (BLE_OTA_SERVER_ENABLE)
    #if (TLKAPI_DEBUG_ENABLE)
        /* user can enable OTA flow log in BLE stack */
        //blc_debug_addStackLog(STK_LOG_OTA_FLOW);
    #endif
    blc_svc_addOtaGroup();
    blc_ota_setOtaProcessTimeout(30);
#endif

    tlkapi_printf(APP_LOG_EN, "[APP][INI] feature_eslp_esl init");
    ////////////////////////////////////////////////////////////////////////////////////////////////
}

/**
 * @brief       user initialization when MCU wake_up from deepSleep_retention mode
 * @param[in]   none
 * @return      none
 */
void user_init_deepRetn(void)
{
#if (PM_DEEPSLEEP_RETENTION_ENABLE)
    blc_app_loadCustomizedParameters_deepRetn();
    blc_ll_initBasicMCU(); //mandatory
    blc_ll_recoverDeepRetention();
    /***
     * TL321X(buteo):
     *   PLL_192M_CCLK_96M_HCLK_48M_PCLK_24M_MSPI_48M, the time from retention_reset(.s file) to here is 550us.
     *   PLL_192M_CCLK_32M_HCLK_32M_PCLK_32M_MSPI_48M, the time from retention_reset(.s file) to here is 578us
     */
    DBG_CHN0_HIGH;
    irq_enable();
    #if (UI_KEYBOARD_ENABLE)
    /////////// keyboard GPIO wakeup init ////////
    u32 pin[] = KB_DRIVE_PINS;
    for (unsigned int i = 0; i < (sizeof(pin) / sizeof(*pin)); i++) {
        cpu_set_gpio_wakeup(pin[i], WAKEUP_LEVEL_HIGH, 1); //drive pin pad high level wakeup deepsleep
    }
    #endif

    #if (BATT_CHECK_ENABLE)
    adc_hw_initialized = 0;
    #endif

    #if (TLKAPI_DEBUG_ENABLE)
    tlkapi_debug_deepRetn_init();
    #endif

#endif
}

void app_process_power_management(void)
{
#if (BLE_APP_PM_ENABLE)
    //Log needs to be output ASAP, and UART invalid after suspend. So Log disable sleep.
    //User tasks can go into suspend, but no deep sleep. So we use manual latency.
    if (tlkapi_debug_isBusy() || app_esl_task_isBusy()) {
        blc_pm_setSleepMask(PM_SLEEP_DISABLE);
    } else {
        int user_task_flg = 0;

        blc_pm_setSleepMask(PM_SLEEP_LEG_ADV | PM_SLEEP_ACL_PERIPHR | PM_SLEEP_PAWRS_RSP | PM_SLEEP_PAWRS_SUB);

    #if (BLE_OTA_SERVER_ENABLE)
        user_task_flg |= ota_is_working;
    #endif

    #if (UI_KEYBOARD_ENABLE)
        user_task_flg |= user_task_flg || scan_pin_need || key_not_released;
    #endif

        if (user_task_flg) {
            bls_pm_setManualLatency(0);
        }
    }
#endif
}

/////////////////////////////////////////////////////////////////////
// main loop flow
/////////////////////////////////////////////////////////////////////

/**
 * @brief     BLE main idle loop
 * @param[in]  none.
 * @return     none.
 */
int main_idle_loop(void)
{
    ////////////////////////////////////// BLE entry /////////////////////////////////
    blc_sdk_main_loop();
    blc_prf_main_loop();
    app_esl_loop();
////////////////////////////////////// Debug entry /////////////////////////////////
#if (TLKAPI_DEBUG_ENABLE)
    tlkapi_debug_handler();
#endif

    ////////////////////////////////////// PM entry /////////////////////////////////
    app_process_power_management();

    return 0; //must return 0 due to SDP flow
}

/**
 * @brief     BLE main loop
 * @param[in]  none.
 * @return     none.
 */
_attribute_no_inline_ void main_loop(void)
{
    main_idle_loop();

#if (ACL_CENTRAL_SIMPLE_SDP_ENABLE)
    simple_sdp_loop();
#endif
}
