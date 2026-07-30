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

#include "app_buffer.h"
#include "hci_transport/hci_tr.h"
#include "hci_transport/hci_dfu.h"

#define MY_APP_ADV_CHANNEL  BLT_ENABLE_ADV_ALL
#define MY_ADV_INTERVAL_MIN ADV_INTERVAL_30MS
#define MY_ADV_INTERVAL_MAX ADV_INTERVAL_40MS

#include "stack/ble/hci/hci_vendor.h"


//////////////////////////////////////////////////////////////////////////////
//  Adv Packet, Response Packet
//////////////////////////////////////////////////////////////////////////////
const u8 tbl_advData[] = {
    0x05,
    0x09,
    't',
    'H',
    'C',
    'I',
    0x02,
    0x01,
    0x05, // BLE limited discoverable mode and BR/EDR not supported
};

const u8 tbl_scanRsp[] = {
    0x07,
    0x09,
    't',
    'H',
    'C',
    'I',
    '0',
    '1', //scan name " tHCI01"
};

/////////////////////////////////////blc_register_hci_handler for spp//////////////////////////////
int rx_from_uart_cb(void) //UART data send to Master,we will handler the data as CMD or DATA
{
    return 0;
}

int tx_to_uart_cb(void)
{
    return 0;
}




#if APP_LE_PERIODIC_ADV_EN
    /*
     * @brief   Periodic Advertising Set number and data buffer length
     *
     * APP_PERID_ADV_SETS_NUMBER:
     * Number of Supported Periodic Advertising Sets, no exceed "PERIODIC_ADV_NUMBER_MAX"
     *
     * APP_PERID_ADV_DATA_LENGTH:
     * Maximum Periodic Advertising Data Length. can not exceed 1650.
     */
    #define APP_PERID_ADV_SETS_NUMBER 1   //1//EBQ test need to change it to the supported value
    #define APP_PERID_ADV_DATA_LENGTH 100 //1024

_attribute_ble_data_retention_ u8 app_peridAdvSet_buffer[PERD_ADV_PARAM_LENGTH * APP_PERID_ADV_SETS_NUMBER];
_attribute_iram_noinit_data_ u8   app_peridAdvData_buffer[APP_PERID_ADV_DATA_LENGTH * APP_PERID_ADV_SETS_NUMBER];
#endif


#if APP_LE_PAWR_ADV_EN

    /*
     * @brief   Periodic Advertising with Response data buffer length and data buffer number.
     *
     * APP_PAWR_SUBEVT_DATA_LENGTH:
     * Maximum Periodic Advertising Subevent Data Length. can not exceed 251.
     *
     * APP_PAWR_SUBEVT_DATA_COUNT
     * Maximum number of subevent data allowed to be updated at one time. can not exceed 15.
     */
     #define APP_PAWR_SUBEVT_DATA_LENGTH     251
     #define APP_PAWR_SUBEVT_DATA_COUNT      4

_attribute_ble_data_retention_ u8 app_peridAdvWrData_buffer[APP_PERID_ADV_SETS_NUMBER * APP_PAWR_SUBEVT_DATA_LENGTH * APP_PAWR_SUBEVT_DATA_COUNT];
#endif

///////////////////////////////////////////

/**
 * @brief      use initialization
 * @param[in]  none.
 * @return     none.
 */
void user_init_normal(void)
{
    //////////////////////////// basic hardware Initialization  Begin //////////////////////////////////
    /* random number generator must be initiated here( in the beginning of user_init_normal).
     * When deepSleep retention wakeUp, no need initialize again */
    random_generator_init();

#if (TLKAPI_DEBUG_ENABLE)
    tlkapi_debug_init();
    blc_debug_enableStackLog(STK_LOG_HCI);
#endif

    blc_readFlashSize_autoConfigCustomFlashSector();

    /* attention that this function must be called after "blc readFlashSize_autoConfigCustomFlashSector" !!!*/
    blc_app_loadCustomizedParameters_normal();
    //////////////////////////// basic hardware Initialization  End /////////////////////////////////


    //////////////////////////// BLE stack Initialization  Begin //////////////////////////////////

    /* for 512K Flash, flash_sector_mac_address equals to 0x76000
     * for 1M   Flash, flash_sector_mac_address equals to 0xFF000 */
    u8 mac_public[6];
    u8 mac_random_static[6];

    blc_initMacAddress(flash_sector_mac_address, mac_public, mac_random_static);

    //////////// Controller Initialization  Begin /////////////////////////
    blc_ll_initBasicMCU();

    blc_ll_initStandby_module(mac_public); //mandatory

#if !APP_LE_EXTENDED_ADV_EN
    blc_ll_initLegacyAdvertising_module();
#endif

#if !APP_LE_EXTENDED_SCAN_EN
    blc_ll_initLegacyScanning_module();
#endif

#if !APP_LE_EXTENDED_INIT_EN
    blc_ll_initLegacyInitiating_module();
#endif

    blc_ll_initAclConnection_module();
#if ACL_CENTRAL_MAX_NUM
    blc_ll_initAclCentralRole_module();
#endif
#if ACL_PERIPHR_MAX_NUM
    blc_ll_initAclPeriphrRole_module();
#endif
    blc_ll_configLegacyAdvEnableStrategy(LEG_ADV_EN_STRATEGY_3);

    blc_ll_setAclConnMaxOctetsNumber(ACL_CONN_MAX_RX_OCTETS, ACL_CENTRAL_MAX_TX_OCTETS, ACL_PERIPHR_MAX_TX_OCTETS);

    /* all ACL connection share same RX FIFO */
    blc_ll_initAclConnRxFifo(app_acl_rx_fifo, ACL_RX_FIFO_SIZE, ACL_RX_FIFO_NUM);
    /* ACL Central TX FIFO */
    blc_ll_initAclCentralTxFifo(app_acl_cen_tx_fifo, ACL_CENTRAL_TX_FIFO_SIZE, ACL_CENTRAL_TX_FIFO_NUM, ACL_CENTRAL_MAX_NUM);
    /* ACL Peripheral TX FIFO */
    blc_ll_initAclPeriphrTxFifo(app_acl_per_tx_fifo, ACL_PERIPHR_TX_FIFO_SIZE, ACL_PERIPHR_TX_FIFO_NUM, ACL_PERIPHR_MAX_NUM);

#if (APP_WORKAROUND_TX_FIFO_4K_LIMITATION_EN && (ACL_CENTRAL_MAX_TX_OCTETS > 230 || ACL_PERIPHR_MAX_TX_OCTETS > 230))
    /* extend TX FIFO size for MAX_TX_OCTETS > 230 if user want use 16 or 32 FIFO */
    blc_ll_initAclConnCacheTxFifo(app_acl_cache_Txfifo, 260, 32);
#endif

    blc_ll_setMaxConnectionNumber(ACL_CENTRAL_MAX_NUM, ACL_PERIPHR_MAX_NUM);
    blc_ll_setAclCentralBaseConnectionInterval(CONN_INTERVAL_10MS);
    blc_ll_setCreateConnectionTimeout(50000); //[!!!important]

    rf_set_power_level_index(RF_POWER_P3dBm);

    //////////// HCI Initialization  Begin /////////////////////////
    /* HCI RX FIFO */
    if (blc_ll_initHciRxFifo(app_hci_rxfifo, HCI_RX_FIFO_SIZE, HCI_RX_FIFO_NUM) != BLE_SUCCESS) {
        while (1)
            ;
    }
    /* HCI TX FIFO */
    if (blc_ll_initHciTxFifo(app_hci_txfifo, HCI_TX_FIFO_SIZE, HCI_TX_FIFO_NUM) != BLE_SUCCESS) {
        while (1)
            ;
    }

    /* HCI RX ACL FIFO (host to controller)*/
    if (blc_ll_initHciAclDataFifo(app_hci_rxAclfifo, HCI_RX_ACL_FIFO_SIZE, HCI_RX_ACL_FIFO_NUM) != BLE_SUCCESS) {
        while (1)
            ;
    }


    /* HCI Data && Event */
    blc_hci_registerControllerDataHandler(blc_hci_sendACLData2Host);
    blc_hci_registerControllerEventHandler(blc_hci_send_data); //controller hci event to host all processed in this func


    //bluetooth event
    blc_hci_setEventMask_cmd(HCI_EVT_MASK_DISCONNECTION_COMPLETE);
    //bluetooth low energy(LE) event, all enable
    blc_hci_le_setEventMask_cmd(0xFFFFFFFF);
    blc_hci_le_setEventMask_2_cmd(0x7FFFFFFF);


#if APP_LE_EXTENDED_ADV_EN
    /* Extended ADV module and ADV Set Parameters buffer initialization */
    blc_ll_initExtendedAdvModule_initExtendedAdvSetParamBuffer(app_extAdvSetParam_buf, APP_EXT_ADV_SETS_NUMBER);
    blc_ll_initExtendedAdvDataBuffer(app_extAdvData_buf, APP_EXT_ADV_DATA_LENGTH);
    blc_ll_initExtendedScanRspDataBuffer(app_extScanRspData_buf, APP_EXT_SCANRSP_DATA_LENGTH);
#endif

#if APP_LE_PERIODIC_ADV_EN
    blc_ll_initPeriodicAdvModule_initPeriodicdAdvSetParamBuffer(app_peridAdvSet_buffer, APP_PERID_ADV_SETS_NUMBER);
    blc_ll_initPeriodicAdvDataBuffer(app_peridAdvData_buffer, APP_PERID_ADV_DATA_LENGTH);
#endif

#if APP_LE_PAWR_ADV_EN
    blc_ll_initPeriodicAdvWrModule_initPeriodicdAdvWrSetParamBuffer(app_peridAdvSet_buffer, APP_PERID_ADV_SETS_NUMBER);
    blc_ll_initPeriodicAdvWrDataBuffer(app_peridAdvWrData_buffer, APP_PAWR_SUBEVT_DATA_LENGTH, APP_PAWR_SUBEVT_DATA_COUNT);
#endif

#if APP_ADVERTISING_CODING_SELECT_EN
    blc_ll_initAdvCodingSelection_feature();
#endif

#if APP_LE_EXTENDED_SCAN_EN
    blc_ll_initExtendedScanning_module();
#endif

#if (APP_LE_EXTENDED_INIT_EN)
    blc_ll_initExtendedInitiating_module();
#endif

#if (APP_SYNCHRONIZED_RECEIVER_EN)
    blc_ll_initPeriodicAdvertisingSynchronization_module();
#endif

#if (APP_PAST_EN)
    blc_ll_initPAST_module();
#endif



#if (APP_POWER_CONTROL)
    blc_ll_initPCL_module();
#endif

#if (APP_CHN_CLASS_EN)
    blc_ll_initChnClass_feature();
#endif

#if (APP_LE_CHANNEL_SOUNDING)
    blc_ll_initCsConfigParam(app_CsConfigParam, APP_CS_CONFIG_NUM);
    blc_ll_initCsRxFifo(app_cs_rx_buf, CS_RX_FIFO_SIZE, CS_RX_FIFO_NUM);

    blc_cs_set_tx_power_level(RF_POWER_INDEX_P4p61dBm);

    blc_ll_initCsInitiatorModule();

    blc_ll_initCsReflectorModule();
#endif

#if APP_LE_2M_CODED_PHY_EN
    blc_ll_init2MPhyCodedPhy_feature(); //need 2M/Coded PHY feature
#endif

#if APP_LE_CHANNEL_SELECTION_ALGORITHM_2_EN
    blc_ll_initChannelSelectionAlgorithm_2_feature(); //need CSA #2
#endif


#if (APP_LE_PHY_TEST_EN)
    blc_phy_initPhyTest_module();
#endif

    blc_ll_setAutoExchangeDataLengthEnable(0);

    u8 error_code = blc_contr_checkControllerInitialization();
    if (error_code != INIT_SUCCESS) {
        /* It's recommended that user set some UI alarm to know the exact error, e.g. LED shine, print log */
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

#if HCI_TR_EN
    /* HCI Transport initialization */
    HCI_TransportInit();
#endif
#if HCI_DFU_EN
    DFU_Init();
#endif

    blc_hci_registerControllerVendorCmdProcess_Callback(hci_vendor_Process);
    tlkapi_send_string_data(APP_LOG_EN, "[APP][INI] BQB controller Init", 0, 0);
}

void user_init_deepRetn(void)
{
}


#if (TIFS_VARIATION_WORKAROUND_MLP_CODE_IN_RAM)
_attribute_ram_code_
#endif
    void
    main_loop(void)
{
#if HCI_TR_EN
    HCI_TransportPoll();
#endif

#if HCI_DFU_EN
    DFU_TaskStart();
#endif

    ////////////////////////////////////// BLE entry /////////////////////////////////
    blc_sdk_main_loop();


////////////////////////////////////// Debug entry /////////////////////////////////
#if (TLKAPI_DEBUG_ENABLE)
    tlkapi_debug_handler();
#endif

////////////////////////////////////// UI entry /////////////////////////////////
#if 0
        static u32 tickLoop = 1;
        if(tickLoop && clock_time_exceed(tickLoop, 100000)){
            tickLoop = clock_time();
            gpio_toggle(GPIO_LED_WHITE);
        }
#endif


#if 0
        static u32  ledToggleTick = 0;
        static u32 loop_cnt = 0;
        if(clock_time_exceed(ledToggleTick, 1000 * 1000))
        {  //led toggle interval: 1000mS
            ledToggleTick = clock_time();
            loop_cnt ++;
            tlkapi_send_string_data(APP_LOG_EN, "[APP] mainloop", &loop_cnt, 4);
        }
#endif
}
