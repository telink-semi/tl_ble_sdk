/********************************************************************************************************
 * @file    app.c
 *
 * @brief   This is the source file for BLE SDK
 *
 * @author  BLE GROUP
 * @date    06,2023
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
#include "app_buffer.h"
#include "app_parse_char.h"
#include "app_ap.h"

// Extended ADV
#define APP_EXT_ADV_SETS_NUMBER         1
#define APP_EXT_ADV_DATA_LENGTH         100
#define APP_EXT_SCANRSP_DATA_LENGTH     100

// Periodic ADV
#define APP_PERID_ADV_SETS_NUMBER       1
#define APP_PERID_ADV_DATA_LENGTH       100

// PAwR ADV
#define APP_PAWR_SUBEVT_DATA_LENGTH     100
#define APP_PAWR_SUBEVT_DATA_COUNT      4

#define HOST_MALLOC_BUF_SIZE            (4 * 1024)

const u8    tbl_advData[] = {
     16,  DT_COMPLETE_LOCAL_NAME,               'T','e','l','i','n','k',' ', 'A', 'P', ' ', '(', 'E', 'S', 'L', ')',
     2,  DT_FLAGS,                              0x05,                   // BLE limited discoverable mode and BR/EDR not supported
};

//_attribute_data_retention_ u8 app_extAdvSetParam_buf[ADV_SET_PARAM_LENGTH * APP_EXT_ADV_SETS_NUMBER];
//_attribute_data_retention_ u8 app_extAdvData_buf[APP_EXT_ADV_DATA_LENGTH * APP_EXT_ADV_SETS_NUMBER];
//_attribute_data_retention_ u8 app_extScanRspData_buf[APP_EXT_SCANRSP_DATA_LENGTH * APP_EXT_ADV_SETS_NUMBER];
//_attribute_data_retention_ u8 app_peridAdvSet_buffer[PERD_ADV_PARAM_LENGTH * APP_PERID_ADV_SETS_NUMBER];
//_attribute_data_retention_ u8 app_peridAdvData_buffer[APP_PERID_ADV_DATA_LENGTH * APP_PERID_ADV_SETS_NUMBER];
//_attribute_data_retention_ u8 app_peridAdvWrData_buffer[APP_PERID_ADV_SETS_NUMBER * APP_PAWR_SUBEVT_DATA_LENGTH * APP_PAWR_SUBEVT_DATA_COUNT];
//_attribute_data_retention_ int central_smp_pending = 0;         // SMP: security & encryption;

u8 app_extAdvSetParam_buf[ADV_SET_PARAM_LENGTH * APP_EXT_ADV_SETS_NUMBER];
u8 app_extAdvData_buf[APP_EXT_ADV_DATA_LENGTH * APP_EXT_ADV_SETS_NUMBER];
u8 app_extScanRspData_buf[APP_EXT_SCANRSP_DATA_LENGTH * APP_EXT_ADV_SETS_NUMBER];
u8 app_peridAdvSet_buffer[PERD_ADV_PARAM_LENGTH * APP_PERID_ADV_SETS_NUMBER];
u8 app_peridAdvData_buffer[APP_PERID_ADV_DATA_LENGTH * APP_PERID_ADV_SETS_NUMBER];
u8 app_peridAdvWrData_buffer[APP_PERID_ADV_SETS_NUMBER * APP_PAWR_SUBEVT_DATA_LENGTH * APP_PAWR_SUBEVT_DATA_COUNT];
int central_smp_pending = 0;         // SMP: security & encryption;
static u8 buffer[HOST_MALLOC_BUF_SIZE];

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
int app_controller_event_callback (u32 h, u8 *p, int n)
{
    blc_eslp_ap_stackEventCallback(h, p, n);

    app_ap_controller_event_callback(h, p, n);

    return 0;

}

/**
 * @brief      BLE host event handler call-back.
 * @param[in]  h       event type
 * @param[in]  para    Pointer point to event parameter buffer.
 * @param[in]  n       the length of event parameter.
 * @return
 */
int app_host_event_callback (u32 h, u8 *para, int n)
{
    u8 event = h & 0xFF;

    app_ap_host_event_callback(h, para, n);

    switch(event)
    {
        case GAP_EVT_SMP_PAIRING_BEGIN:
        {

        }
        break;

        case GAP_EVT_SMP_PAIRING_SUCCESS:
        {

        }
        break;

        case GAP_EVT_SMP_PAIRING_FAIL:
        {
            #if (ACL_CENTRAL_SMP_ENABLE)
                gap_smp_pairingFailEvt_t *pEvt = (gap_smp_pairingFailEvt_t *)para;

                if( dev_char_get_conn_role_by_connhandle(pEvt->connHandle) == ACL_ROLE_CENTRAL){
                    if(central_smp_pending == pEvt->connHandle){
                        central_smp_pending = 0;
                        tlkapi_send_string_data(APP_SMP_LOG_EN, "[APP][SMP] paring fail", &pEvt->connHandle, sizeof(gap_smp_pairingFailEvt_t));
                    }
                }
            #endif
        }
        break;

        case GAP_EVT_SMP_CONN_ENCRYPTION_DONE:
        {
            gap_smp_connEncDoneEvt_t *pEvt = (gap_smp_connEncDoneEvt_t *)para;
            tlkapi_send_string_data(APP_SMP_LOG_EN, "[APP][SMP] Connection encryption done event", &pEvt->connHandle, sizeof(gap_smp_connEncDoneEvt_t));
        }
        break;

        case GAP_EVT_SMP_SECURITY_PROCESS_DONE:
        {
            gap_smp_connEncDoneEvt_t* pEvt = (gap_smp_connEncDoneEvt_t*)para;
            tlkapi_send_string_data(APP_SMP_LOG_EN, "[APP][SMP] Security process done event", &pEvt->connHandle, sizeof(gap_smp_connEncDoneEvt_t));

            if( dev_char_get_conn_role_by_connhandle(pEvt->connHandle) == ACL_ROLE_CENTRAL){

                #if (ACL_CENTRAL_SMP_ENABLE)
                    if( dev_char_get_conn_role_by_connhandle(pEvt->connHandle) == ACL_ROLE_CENTRAL){
                        if(central_smp_pending == pEvt->connHandle){
                            central_smp_pending = 0;
                        }
                    }
                #endif

                #if (ACL_CENTRAL_SIMPLE_SDP_ENABLE)  //SMP finish
                    if(central_sdp_pending == pEvt->connHandle){  //SDP is pending
                        app_register_service(&app_service_discovery);  //start SDP now
                    }
                #endif

            }
            /* PAwR-ADV: send LL_PERIODIC_SYNC_WR_IND */
//          ble_sts_t s = blc_ll_periodicAdvSetInfoTransfer(pEvt->connHandle, 0xff, ADV_HANDLE0);
//          tlkapi_send_string_data(1||APP_SMP_LOG_EN, "[APP][SMP] S1'", &s, 1);
        }
        break;

        case GAP_EVT_SMP_TK_DISPLAY:
        {

        }
        break;

        case GAP_EVT_SMP_TK_REQUEST_PASSKEY:
        {

        }
        break;

        case GAP_EVT_SMP_TK_REQUEST_OOB:
        {

        }
        break;

        case GAP_EVT_SMP_TK_NUMERIC_COMPARE:
        {

        }
        break;

        case GAP_EVT_ATT_EXCHANGE_MTU:
        {

        }
        break;

        case GAP_EVT_GATT_HANDLE_VALUE_CONFIRM:
        {

        }
        break;

        default:
        break;
    }

    return 0;
}

int app_prf_event_callback(u16 aclHandle, int evtID, u8 *pData, u16 dataLen)
{
    app_ap_prf_event_callback(aclHandle, evtID, pData, dataLen);

    return 0;
}

/**
 * @brief      BLE GATT data handler call-back.
 * @param[in]  connHandle     connection handle.
 * @param[in]  pkt             Pointer point to data packet buffer.
 * @return
 */
int app_gatt_data_handler (u16 connHandle, u8 *pkt)
{
    if( dev_char_get_conn_role_by_connhandle(connHandle) == ACL_ROLE_CENTRAL )   //GATT data for Central
    {
        rf_packet_att_t *pAtt = (rf_packet_att_t*)pkt;

        if(!(pAtt->opcode & 0x01)){
            switch(pAtt->opcode){
                case ATT_OP_FIND_INFO_REQ:
                case ATT_OP_FIND_BY_TYPE_VALUE_REQ:
                case ATT_OP_READ_BY_TYPE_REQ:
                case ATT_OP_READ_BY_GROUP_TYPE_REQ:
                    blc_gatt_pushErrResponse(connHandle, pAtt->opcode, pAtt->handle, ATT_ERR_ATTR_NOT_FOUND);
                    break;
                case ATT_OP_READ_REQ:
                case ATT_OP_READ_BLOB_REQ:
                case ATT_OP_READ_MULTI_REQ:
                case ATT_OP_WRITE_REQ:
                case ATT_OP_PREPARE_WRITE_REQ:
                    blc_gatt_pushErrResponse(connHandle, pAtt->opcode, pAtt->handle, ATT_ERR_INVALID_HANDLE);
                    break;
                case ATT_OP_EXECUTE_WRITE_REQ:
                case ATT_OP_HANDLE_VALUE_CFM:
                case ATT_OP_WRITE_CMD:
                case ATT_OP_SIGNED_WRITE_CMD:
                    //ignore
                    break;
                default://no action
                    break;
            }
        }
    }

    return 0;
}

static void app_start_advertising(void)
{

    /* Extended ADV module and ADV Set Parameters buffer initialization */
    blc_ll_initExtendedAdvModule_initExtendedAdvSetParamBuffer(app_extAdvSetParam_buf, APP_EXT_ADV_SETS_NUMBER);
    blc_ll_initExtendedAdvDataBuffer(app_extAdvData_buf, APP_EXT_ADV_DATA_LENGTH);
    blc_ll_initExtendedScanRspDataBuffer(app_extScanRspData_buf, APP_EXT_SCANRSP_DATA_LENGTH);
    // Extended, None_Connectable_None_Scannable undirected, with auxiliary packet
    blc_ll_setExtAdvParam( ADV_HANDLE0,         ADV_EVT_PROP_EXTENDED_NON_CONNECTABLE_NON_SCANNABLE_UNDIRECTED, ADV_INTERVAL_200MS,             ADV_INTERVAL_200MS,
                           BLT_ENABLE_ADV_ALL,  OWN_ADDRESS_PUBLIC,                                             BLE_ADDR_PUBLIC,                NULL,
                           ADV_FP_NONE,         TX_POWER_3dBm,                                                  BLE_PHY_1M,                     0,
                           BLE_PHY_1M,          ADV_SID_0,                                                      0);
    blc_ll_setExtAdvData( ADV_HANDLE0,    sizeof(tbl_advData), (const u8 *)tbl_advData);
    blc_ll_setExtAdvEnable( BLC_ADV_ENABLE, ADV_HANDLE0, 0 , 0);

    blc_ll_initPeriodicAdvModule_initPeriodicdAdvSetParamBuffer(app_peridAdvSet_buffer, APP_PERID_ADV_SETS_NUMBER);
    blc_ll_initPeriodicAdvDataBuffer(app_peridAdvData_buffer, APP_PERID_ADV_DATA_LENGTH);

    blc_ll_initPeriodicAdvWrModule_initPeriodicdAdvWrSetParamBuffer(app_peridAdvSet_buffer, APP_PERID_ADV_SETS_NUMBER);
    blc_ll_initPeriodicAdvWrDataBuffer(app_peridAdvWrData_buffer, APP_PAWR_SUBEVT_DATA_LENGTH, APP_PAWR_SUBEVT_DATA_COUNT);

    blc_ll_initPAST_module();

    hci_le_setPeriodicAdvParamV2_retParam_t status_v2 = blc_ll_setPeriodicAdvParam_v2(ADV_HANDLE0,
                    PERADV_INTERVAL_1S6,      // Periodic_Advertising_Interval_Min: 1600ms
                    PERADV_INTERVAL_1S6,      // Periodic_Advertising_Interval_Max: 1600ms
                    PERD_ADV_PROP_MASK_NONE,    // Periodic_Advertising_Properties: None
                    4,                          // Num_Subevents: 4
                    PERADV_INTERVAL_100MS,      // Subevent_Interval: 100ms
                    8,                          // Response_Slot_Delay: 10 ms
                    10,                         // Response_Slot_Spacing: 2.5ms
                    25);                        // Num_Response_Slots: 25
    if (status_v2.status != BLE_SUCCESS) {
        tlkapi_printf(APP_LOG_EN, "[APP] Failed to set periodic advertising param, status: 0x%02X", status_v2.status);
        return;
    }

    ble_sts_t status = blc_ll_setPeriodicAdvEnable( BLC_ADV_ENABLE, ADV_HANDLE0);
    if (status != BLE_SUCCESS) {
        tlkapi_printf(APP_LOG_EN, "[APP] Failed to start periodic advertising, status: 0x%02X", status);
    }
}

#if (BATT_CHECK_ENABLE) //battery check must do before OTA relative operation

_attribute_data_retention_ u32 lowBattDet_tick = 0;

/**
 * @brief       this function is used to process battery power.
 *              The low voltage protection threshold 2.0V is an example and reference value. Customers should
 *              evaluate and modify these thresholds according to the actual situation. If users have unreasonable designs
 *              in the hardware circuit, which leads to a decrease in the stability of the power supply network, the
 *              safety thresholds must be increased as appropriate.
 * @param[in]   none
 * @return      none
 */
_attribute_ram_code_ void user_battery_power_check(u16 alarm_vol_mv)
{
    /*For battery-powered products, as the battery power will gradually drop, when the voltage is low to a certain
      value, it will cause many problems.
        a) When the voltage is lower than operating voltage range of chip, chip can no longer guarantee stable operation.
        b) When the battery voltage is low, due to the unstable power supply, the write and erase operations
            of Flash may have the risk of error, causing the program firmware and user data to be modified abnormally,
            and eventually causing the product to fail. */
    u8 battery_check_returnValue = 0;
    if (analog_read(USED_DEEP_ANA_REG) & LOW_BATT_FLG) {
        battery_check_returnValue = app_battery_power_check(alarm_vol_mv + 200);
    } else {
        battery_check_returnValue = app_battery_power_check(alarm_vol_mv);
    }
    if (battery_check_returnValue) {
        analog_write_reg8(USED_DEEP_ANA_REG, analog_read_reg8(USED_DEEP_ANA_REG) & (~LOW_BATT_FLG)); //clr
    } else {
    #if (UI_LED_ENABLE)                                                                              //led indicate
        for (int k = 0; k < 3; k++) {
            gpio_write(GPIO_LED_BLUE, LED_ON_LEVEL);
            sleep_us(200000);
            gpio_write(GPIO_LED_BLUE, !LED_ON_LEVEL);
            sleep_us(200000);
        }
    #endif
        analog_write_reg8(USED_DEEP_ANA_REG, analog_read_reg8(USED_DEEP_ANA_REG) | LOW_BATT_FLG); //mark

    #if (UI_KEYBOARD_ENABLE)
        u32 pin[] = KB_DRIVE_PINS;
        for (unsigned int i = 0; i < (sizeof(pin) / sizeof(*pin)); i++) {
            cpu_set_gpio_wakeup(pin[i], 1, 1);              //drive pin pad high wakeup deepsleep
        }

        cpu_sleep_wakeup(DEEPSLEEP_MODE, PM_WAKEUP_PAD, 0); //deepsleep
    #endif
    }
}

#endif

#if (APP_FLASH_PROTECTION_ENABLE)

/**
 * @brief      flash protection operation, including all locking & unlocking for application
 *             handle all flash write & erase action for this demo code. use should add more more if they have more flash operation.
 * @param[in]  flash_op_evt - flash operation event, including application layer action and stack layer action event(OTA write & erase)
 *             attention 1: if you have more flash write or erase action, you should should add more type and process them
 *             attention 2: for "end" event, no need to pay attention on op_addr_begin & op_addr_end, we set them to 0 for
 *                          stack event, such as stack OTA write new firmware end event
 * @param[in]  op_addr_begin - operating flash address range begin value
 * @param[in]  op_addr_end - operating flash address range end value
 *             attention that, we use: [op_addr_begin, op_addr_end)
 *             e.g. if we write flash sector from 0x10000 to 0x20000, actual operating flash address is 0x10000 ~ 0x1FFFF
 *                  but we use [0x10000, 0x20000):  op_addr_begin = 0x10000, op_addr_end = 0x20000
 * @return     none
 */
_attribute_data_retention_ u16 flash_lockBlock_cmd;

void app_flash_protection_operation(u8 flash_op_evt, u32 op_addr_begin, u32 op_addr_end)
{
    (void)op_addr_begin;
    (void)op_addr_end;
    if (flash_op_evt == FLASH_OP_EVT_APP_INITIALIZATION) {
        /* ignore "op addr_begin" and "op addr_end" for initialization event
         * must call "flash protection_init" first, will choose correct flash protection relative API according to current internal flash type in MCU */
        flash_protection_init();

        /* just sample code here, protect all flash area for old firmware and OTA new firmware.
         * user can change this design if have other consideration */
        u32 app_lockBlock = 0;
    #if (BLE_OTA_SERVER_ENABLE)
        u32 multiBootAddress = blc_ota_getCurrentUsedMultipleBootAddress();
        if (multiBootAddress == MULTI_BOOT_ADDR_0x20000) {
            app_lockBlock = FLASH_LOCK_FW_LOW_256K;
        } else if (multiBootAddress == MULTI_BOOT_ADDR_0x40000) {
            app_lockBlock = FLASH_LOCK_FW_LOW_512K;
        } else if (multiBootAddress == MULTI_BOOT_ADDR_0x80000) {
            /* attention that 1M capacity flash can not lock all 1M area, should leave some upper sector
                 * for system data(SMP storage data & calibration data & MAC address) and user data
                 * will use a approximate value */
            app_lockBlock = FLASH_LOCK_FW_LOW_1M;
        }
    #else
        app_lockBlock = FLASH_LOCK_FW_LOW_512K; //just demo value, user can change this value according to application
    #endif


        flash_lockBlock_cmd = flash_change_app_lock_block_to_flash_lock_block(app_lockBlock);

        if (blc_flashProt.init_err) {
            tlkapi_printf(APP_FLASH_PROT_LOG_EN, "[FLASH][PROT] flash protection initialization error!!!\n");
        }

        tlkapi_printf(APP_FLASH_PROT_LOG_EN, "[FLASH][PROT] initialization, lock flash\n");
        flash_lock(flash_lockBlock_cmd);
    }
    #if (BLE_OTA_SERVER_ENABLE)
    else if (flash_op_evt == FLASH_OP_EVT_STACK_OTA_CLEAR_OLD_FW_BEGIN) {
        /* OTA clear old firmware begin event is triggered by stack, in "blc ota_initOtaServer_module", rebooting from a successful OTA.
         * Software will erase whole old firmware for potential next new OTA, need unlock flash if any part of flash address from
         * "op addr_begin" to "op addr_end" is in locking block area.
         * In this sample code, we protect whole flash area for old and new firmware, so here we do not need judge "op addr_begin" and "op addr_end",
         * must unlock flash */
        tlkapi_printf(APP_FLASH_PROT_LOG_EN, "[FLASH][PROT] OTA clear old FW begin, unlock flash\n");
        flash_unlock();
    } else if (flash_op_evt == FLASH_OP_EVT_STACK_OTA_CLEAR_OLD_FW_END) {
        /* ignore "op addr_begin" and "op addr_end" for END event
         * OTA clear old firmware end event is triggered by stack, in "blc ota_initOtaServer_module", erasing old firmware data finished.
         * In this sample code, we need lock flash again, because we have unlocked it at the begin event of clear old firmware */
        tlkapi_printf(APP_FLASH_PROT_LOG_EN, "[FLASH][PROT] OTA clear old FW end, restore flash locking\n");
        flash_lock(flash_lockBlock_cmd);
    } else if (flash_op_evt == FLASH_OP_EVT_STACK_OTA_WRITE_NEW_FW_BEGIN) {
        /* OTA write new firmware begin event is triggered by stack, when receive first OTA data PDU.
         * Software will write data to flash on new firmware area,  need unlock flash if any part of flash address from
         * "op addr_begin" to "op addr_end" is in locking block area.
         * In this sample code, we protect whole flash area for old and new firmware, so here we do not need judge "op addr_begin" and "op addr_end",
         * must unlock flash */
        tlkapi_printf(APP_FLASH_PROT_LOG_EN, "[FLASH][PROT] OTA write new FW begin, unlock flash\n");
        flash_unlock();
    } else if (flash_op_evt == FLASH_OP_EVT_STACK_OTA_WRITE_NEW_FW_END) {
        /* ignore "op addr_begin" and "op addr_end" for END event
         * OTA write new firmware end event is triggered by stack, after OTA end or an OTA error happens, writing new firmware data finished.
         * In this sample code, we need lock flash again, because we have unlocked it at the begin event of write new firmware */
        tlkapi_printf(APP_FLASH_PROT_LOG_EN, "[FLASH][PROT] OTA write new FW end, restore flash locking\n");
        flash_lock(flash_lockBlock_cmd);
    }
    #endif
    /* add more flash protection operation for your application if needed */
}


#endif

/**
 * @brief       user initialization when MCU power on or wake_up from deepSleep mode
 * @param[in]   none
 * @return      none
 */
_attribute_no_inline_ void user_init_normal(void)
{
//////////////////////////// basic hardware Initialization  Begin //////////////////////////////////
    /* random number generator must be initiated here( in the beginning of user_init_normal).
     * When deepSleep retention wakeUp, no need initialize again */
    random_generator_init();

    #if (TLKAPI_DEBUG_ENABLE)
        tlkapi_debug_init();
        blc_debug_enableStackLog(STK_LOG_NONE);
    #endif

#if (BATT_CHECK_ENABLE)
    /*The SDK must do a quick low battery detect during user initialization instead of waiting
      until the main_loop. The reason for this process is to avoid application errors that the device
      has already working at low power.
      Considering the working voltage of MCU and the working voltage of flash, if the Demo is set below 2.0V,
      the chip will alarm and deep sleep (Due to PM does not work in the current version of B92, it does not go
      into deepsleep), and once the chip is detected to be lower than 2.0V, it needs to wait until the voltage rises to 2.2V,
      the chip will resume normal operation. Consider the following points in this design:
        At 2.0V, when other modules are operated, the voltage may be pulled down and the flash will not
        work normally. Therefore, it is necessary to enter deepsleep below 2.0V to ensure that the chip no
        longer runs related modules;
        When there is a low voltage situation, need to restore to 2.2V in order to make other functions normal,
        this is to ensure that the power supply voltage is confirmed in the charge and has a certain amount of
        power, then start to restore the function can be safer.*/


    user_battery_power_check(2000);
#endif

    blc_readFlashSize_autoConfigCustomFlashSector();

    /* attention that this function must be called after "blc readFlashSize_autoConfigCustomFlashSector" !!!*/
    blc_app_loadCustomizedParameters_normal();

#if (APP_FLASH_PROTECTION_ENABLE)
    app_flash_protection_operation(FLASH_OP_EVT_APP_INITIALIZATION, 0, 0);
    blc_appRegisterStackFlashOperationCallback(app_flash_protection_operation); //register flash operation callback for stack
#endif
//////////////////////////// basic hardware Initialization  End /////////////////////////////////


//////////////////////////// BLE stack Initialization  Begin //////////////////////////////////

    u8  mac_public[6];
    u8  mac_random_static[6];
    /* Note: If change IC type, need to confirm the FLASH_SIZE_CONFIG */
    blc_initMacAddress(flash_sector_mac_address, mac_public, mac_random_static);


    //////////// LinkLayer Initialization  Begin /////////////////////////
    blc_ll_initBasicMCU();

    blc_ll_initStandby_module(mac_public);

    blc_ll_initExtendedScanning_module();
    blc_ll_initExtendedInitiating_module();

    blc_ll_initAclConnection_module();
    blc_ll_initAclCentralRole_module();
    //blc_ll_initAclPeriphrRole_module();

    blc_ll_setMaxConnectionNumber(ACL_CENTRAL_MAX_NUM, ACL_PERIPHR_MAX_NUM);

    blc_ll_setAclConnMaxOctetsNumber(ACL_CONN_MAX_RX_OCTETS, ACL_CENTRAL_MAX_TX_OCTETS, ACL_PERIPHR_MAX_TX_OCTETS);

    /* all ACL connection share same RX FIFO */
    blc_ll_initAclConnRxFifo(app_acl_rx_fifo, ACL_RX_FIFO_SIZE, ACL_RX_FIFO_NUM);
    /* ACL Central TX FIFO */
    blc_ll_initAclCentralTxFifo(app_acl_cen_tx_fifo, ACL_CENTRAL_TX_FIFO_SIZE, ACL_CENTRAL_TX_FIFO_NUM, ACL_CENTRAL_MAX_NUM);

    blc_ll_setAclCentralBaseConnectionInterval(CONN_INTERVAL_7P5MS); //CONN_INTERVAL_31P25MS);

    //////////// LinkLayer Initialization  End /////////////////////////



    //////////// HCI Initialization  Begin /////////////////////////
    blc_hci_registerControllerDataHandler (blc_l2cap_pktHandler_5_3);

    blc_hci_registerControllerEventHandler(app_controller_event_callback); //controller hci event to host all processed in this func

    //bluetooth event
    blc_hci_setEventMask_cmd (HCI_EVT_MASK_DISCONNECTION_COMPLETE);

    //bluetooth low energy(LE) event
    blc_hci_le_setEventMask_cmd(        HCI_LE_EVT_MASK_CONNECTION_COMPLETE  \
                                    |   HCI_LE_EVT_MASK_ADVERTISING_REPORT \
                                    |   HCI_LE_EVT_MASK_ENHANCED_CONNECTION_COMPLETE \
                                    |   HCI_LE_EVT_MASK_CONNECTION_UPDATE_COMPLETE \
                                    |   HCI_LE_EVT_MASK_EXTENDED_ADVERTISING_REPORT );

    blc_hci_le_setEventMask_2_cmd(      HCI_LE_EVT_MASK_2_PERIODIC_ADVERTISING_SUBEVENT_DATA_REQUEST \
                                    |   HCI_LE_EVT_MASK_2_PERIODIC_ADVERTISING_RESPONSE_REPORT );

    u8 error_code = blc_contr_checkControllerInitialization();
    if(error_code != INIT_SUCCESS){
        /* It's recommended that user set some UI alarm to know the exact error, e.g. LED shine, print log */
        #if (TLKAPI_DEBUG_ENABLE)
            tlkapi_send_string_data(APP_LOG_EN, "[APP][INI] Controller INIT ERROR", &error_code, 1);
            while(1){
                tlkapi_debug_handler();
            }
        #else
            while(1);
        #endif
    }
    //////////// HCI Initialization  End /////////////////////////


    //////////// Host Initialization  Begin /////////////////////////
    /* Host Initialization */
    /* GAP initialization must be done before any other host feature initialization !!! */
    blc_gap_init();
    blc_att_setMtureqSendingTime_after_connCreate(0);

    /* L2CAP data buffer Initialization */
    blc_l2cap_initAclCentralBuffer(app_cen_l2cap_rx_buf, CENTRAL_L2CAP_BUFF_SIZE, app_cen_l2cap_tx_buf, CENTRAL_L2CAP_BUFF_SIZE);
    blc_l2cap_initAclPeripheralBuffer(app_per_l2cap_rx_buf, PERIPHR_L2CAP_BUFF_SIZE, app_per_l2cap_tx_buf, PERIPHR_L2CAP_BUFF_SIZE);

    blc_att_setCentralRxMtuSize(CENTRAL_ATT_RX_MTU); ///must be placed after "blc_gap_init"
    blc_att_setPeripheralRxMtuSize(PERIPHR_ATT_RX_MTU);   ///must be placed after "blc_gap_init"

    /* GATT Initialization */
    #if (ACL_CENTRAL_SIMPLE_SDP_ENABLE)
        host_att_register_idle_func (main_idle_loop);
    #endif
    blc_gatt_register_data_handler(app_gatt_data_handler);

    /* SMP Initialization */
    #if (ACL_PERIPHR_SMP_ENABLE || ACL_CENTRAL_SMP_ENABLE)
        /* Note: If change IC type, need to confirm the FLASH_SIZE_CONFIG */
        blc_smp_configPairingSecurityInfoStorageAddressAndSize(flash_sector_smp_storage, FLASH_SMP_PAIRING_MAX_SIZE);
    #endif

    #if (ACL_PERIPHR_SMP_ENABLE)  //Peripheral SMP Enable
        blc_smp_setSecurityLevel_periphr(Unauthenticated_Pairing_with_Encryption);  //LE_Security_Mode_1_Level_2
    #else
        blc_smp_setSecurityLevel_periphr(No_Security);
    #endif

    #if (ACL_CENTRAL_SMP_ENABLE)
        blc_smp_setSecurityLevel_central(Unauthenticated_Pairing_with_Encryption);  //LE_Security_Mode_1_Level_2
    #else
        blc_smp_setSecurityLevel_central(No_Security);
    #endif

    blc_smp_smpParamInit();


    //host(GAP/SMP/GATT/ATT) event process: register host event callback and set event mask
    blc_gap_registerHostEventHandler( app_host_event_callback );
    blc_gap_setEventMask( GAP_EVT_MASK_SMP_PAIRING_BEGIN               |  \
                          GAP_EVT_MASK_SMP_PAIRING_SUCCESS             |  \
                          GAP_EVT_MASK_SMP_PAIRING_FAIL                |  \
                          GAP_EVT_MASK_SMP_CONN_ENCRYPTION_DONE        |  \
                          GAP_EVT_MASK_L2CAP_COC_CONNECT               |  \
                          GAP_EVT_MASK_L2CAP_COC_DISCONNECT            |  \
                          GAP_EVT_MASK_L2CAP_COC_RECONFIGURE           |  \
                          GAP_EVT_MASK_L2CAP_COC_RECV_DATA             |  \
                          GAP_EVT_MASK_L2CAP_COC_SEND_DATA_FINISH      |  \
                          GAP_EVT_MASK_L2CAP_COC_CREATE_CONNECT_FINISH |  \
                          GAP_EVT_MASK_ATT_EXCHANGE_MTU                |  \
                          GAP_EVT_MASK_SMP_SECURITY_PROCESS_DONE);
    ////////// Host Initialization  End /////////////////////////

////////////////////////// BLE stack Initialization  End //////////////////////////////////




////////////////////////// User Configuration for BLE application ////////////////////////////

    rf_set_power_level_index(RF_POWER_P9dBm);

    #if (BLE_APP_PM_ENABLE)
        blc_ll_initPowerManagement_module();
        blc_pm_setSleepMask(PM_SLEEP_LEG_ADV | PM_SLEEP_LEG_SCAN | PM_SLEEP_ACL_PERIPHR | PM_SLEEP_ACL_CENTRAL);
    #endif
    blc_prf_initPairingInfoStoreModule();

    blc_prf_initialModule(app_prf_event_callback,buffer,HOST_MALLOC_BUF_SIZE);
    blc_ll_setExtScanParam( OWN_ADDRESS_PUBLIC, SCAN_FP_ALLOW_ADV_ANY, SCAN_PHY_1M_CODED, \
                            SCAN_TYPE_PASSIVE,  SCAN_INTERVAL_90MS,   SCAN_WINDOW_90MS, \
                            SCAN_TYPE_PASSIVE,  SCAN_INTERVAL_90MS,   SCAN_WINDOW_90MS);

    blc_ll_setExtScanEnable( BLC_SCAN_ENABLE, DUPE_FLTR_DISABLE, SCAN_DURATION_CONTINUOUS, SCAN_WINDOW_CONTINUOUS);

    app_start_advertising();

    app_ap_init();
    tlkapi_send_string_data(APP_LOG_EN, "[APP][INI] feature_per_adv init", 0, 0);
////////////////////////////////////////////////////////////////////////////////////////////////
}



/**
 * @brief       user initialization when MCU wake_up from deepSleep_retention mode
 * @param[in]   none
 * @return      none
 */
void user_init_deepRetn(void)
{

}


/////////////////////////////////////////////////////////////////////
// main loop flow
/////////////////////////////////////////////////////////////////////

/**
 * @brief     BLE main idle loop
 * @param[in]  none.
 * @return     none.
 */
int main_idle_loop (void)
{

    ////////////////////////////////////// BLE entry /////////////////////////////////
    blc_sdk_main_loop();
    blc_prf_main_loop();

    app_parse_loop();
    app_ap_loop();

    ////////////////////////////////////// Debug entry /////////////////////////////////
    #if (TLKAPI_DEBUG_ENABLE)
        tlkapi_debug_handler();
    #endif

    ////////////////////////////////////// UI entry /////////////////////////////////
    #if (BATT_CHECK_ENABLE)
       /*The frequency of low battery detect is controlled by the variable lowBattDet_tick, which is executed every
       500ms in the demo. Users can modify this time according to their needs.*/
        if (battery_get_detect_enable() && clock_time_exceed(lowBattDet_tick, 500000)) {
            lowBattDet_tick = clock_time();
            user_battery_power_check(BAT_DEEP_THRESHOLD_MV);
        }
    #endif
    #if (UI_KEYBOARD_ENABLE)
        proc_keyboard (0, 0, 0);
    #endif

    return 0; //must return 0 due to SDP flow
}



/**
 * @brief     BLE main loop
 * @param[in]  none.
 * @return     none.
 */
_attribute_no_inline_ void main_loop (void)
{
    main_idle_loop ();

    #if (ACL_CENTRAL_SIMPLE_SDP_ENABLE)
        simple_sdp_loop ();
    #endif
}
