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

#include "app.h"
#include "app_buffer.h"
#include "app_att.h"
#include "app_ui.h"



_attribute_ble_data_retention_      int central_smp_pending = 0;        // SMP: security & encryption;




/**
 * @brief   BLE Advertising data
 */
const u8    tbl_advData[] = {
     11, DT_COMPLETE_LOCAL_NAME,                'm','u','l','t','i','_','c','o','n','n',
     2,  DT_FLAGS,                              0x05,                   // BLE limited discoverable mode and BR/EDR not supported
     3,  DT_APPEARANCE,                         0x80, 0x01,             // 384, Generic Remote Control, Generic category
     5,  DT_INCOMPLETE_LIST_16BIT_SERVICE_UUID, 0x12, 0x18, 0x0F, 0x18, // incomplete list of service class UUIDs (0x1812, 0x180F)
};

/**
 * @brief   BLE Scan Response Packet data
 */
const u8    tbl_scanRsp [] = {
     11, DT_COMPLETE_LOCAL_NAME,                'm','u','l','t','i','_','c','o','n','n',
};





/**
 * @brief      BLE Adv report event handler
 * @param[in]  p         Pointer point to event parameter buffer.
 * @return
 */
int AA_dbg_adv_rpt = 0;
u32 tick_adv_rpt = 0;

int app_le_adv_report_event_handle(u8 *p)
{
    event_adv_report_t *pa = (event_adv_report_t *)p;
    s8 rssi = pa->data[pa->len];

    #if 0  //debug, print ADV report number every 5 seconds
        AA_dbg_adv_rpt ++;
        if(clock_time_exceed(tick_adv_rpt, 5000000)){
            tlkapi_send_string_data(APP_CONTR_EVT_LOG_EN, "[APP][EVT] Adv report", pa->mac, 6);
            tick_adv_rpt = clock_time();
        }
    #endif

    /*********************** Central Create connection demo: Key press or ADV pair packet triggers pair  ********************/
    #if (ACL_CENTRAL_SMP_ENABLE)
        if(central_smp_pending){     //if previous connection SMP not finish, can not create a new connection
            return 1;
        }
    #endif

    #if (ACL_CENTRAL_SIMPLE_SDP_ENABLE)
        if(central_sdp_pending){     //if previous connection SDP not finish, can not create a new connection
            return 1;
        }
    #endif

    if (central_disconnect_connhandle){ //one ACL connection central role is in un_pair disconnection flow, do not create a new one
        return 1;
    }

    int central_auto_connect = 0;
    int user_manual_pairing = 0;

    //manual pairing methods 1: key press triggers
    user_manual_pairing = central_pairing_enable && (rssi > -66);  //button trigger pairing(RSSI threshold, short distance)

    #if (ACL_CENTRAL_SMP_ENABLE)
        central_auto_connect = blc_smp_searchBondingPeripheralDevice_by_PeerMacAddress(pa->adr_type, pa->mac);
    #endif

    if(central_auto_connect || user_manual_pairing){

        /* send create connection command to Controller, trigger it switch to initiating state. After this command, Controller
         * will scan all the ADV packets it received but not report to host, to find the specified device(mac_adr_type & mac_adr),
         * then send a "CONN_REQ" packet, enter to connection state and send a connection complete event
         * (HCI_SUB_EVT_LE_CONNECTION_COMPLETE) to Host*/
        u8 status = blc_ll_createConnection( SCAN_INTERVAL_100MS, SCAN_WINDOW_100MS, INITIATE_FP_ADV_SPECIFY,  \
                                 pa->adr_type, pa->mac, OWN_ADDRESS_PUBLIC, \
                                 CONN_INTERVAL_31P25MS, CONN_INTERVAL_48P75MS, 0, CONN_TIMEOUT_4S, \
                                 0, 0xFFFF);


        if(status == BLE_SUCCESS){ //create connection success
            tlkapi_send_string_data(APP_LOG_EN, "[APP][CMD] create connection success", pa->mac, 6);
        }
    }
    /*********************** Central Create connection demo code end  *******************************************************/


    return 0;
}


/**
 * @brief      BLE Connection complete event handler
 * @param[in]  p         Pointer point to event parameter buffer.
 * @return
 */
int app_le_connection_complete_event_handle(u8 *p)
{
    hci_le_connectionCompleteEvt_t *pConnEvt = (hci_le_connectionCompleteEvt_t *)p;

    if(pConnEvt->status == BLE_SUCCESS){

        tlkapi_send_string_data(APP_CONTR_EVT_LOG_EN, "[APP][EVT] Connection complete event", &pConnEvt->connHandle, sizeof(hci_le_connectionCompleteEvt_t) - 2);

        dev_char_info_insert_by_conn_event(pConnEvt);

        if(pConnEvt->role == ACL_ROLE_CENTRAL) // central role, process SMP and SDP if necessary
        {
            #if (ACL_CENTRAL_SMP_ENABLE)
                central_smp_pending = pConnEvt->connHandle; // this connection need SMP
            #endif



            #if (ACL_CENTRAL_SIMPLE_SDP_ENABLE)
                memset(&cur_sdp_device, 0, sizeof(dev_char_info_t));
                cur_sdp_device.conn_handle = pConnEvt->connHandle;
                cur_sdp_device.peer_adrType = pConnEvt->peerAddrType;
                memcpy(cur_sdp_device.peer_addr, pConnEvt->peerAddr, 6);

                u8  temp_buff[sizeof(dev_att_t)];
                dev_att_t *pdev_att = (dev_att_t *)temp_buff;

                /* att_handle search in flash, if success, load char_handle directly from flash, no need SDP again */
                if( dev_char_info_search_peer_att_handle_by_peer_mac(pConnEvt->peerAddrType, pConnEvt->peerAddr, pdev_att) ){
                    //cur_sdp_device.char_handle[1] =                                   //Speaker
                    cur_sdp_device.char_handle[2] = pdev_att->char_handle[2];           //OTA
                    cur_sdp_device.char_handle[3] = pdev_att->char_handle[3];           //consume report
                    cur_sdp_device.char_handle[4] = pdev_att->char_handle[4];           //normal key report
                    //cur_sdp_device.char_handle[6] =                                   //BLE Module, SPP Server to Client
                    //cur_sdp_device.char_handle[7] =                                   //BLE Module, SPP Client to Server

                    /* add the peer device att_handle value to conn_dev_list */
                    dev_char_info_add_peer_att_handle(&cur_sdp_device);
                }
                else
                {
                    central_sdp_pending = pConnEvt->connHandle;  // mark this connection need SDP

                    #if (ACL_CENTRAL_SMP_ENABLE)
                         //service discovery initiated after SMP done, trigger it in "GAP_EVT_MASK_SMP_SECURITY_PROCESS_DONE" event callBack.
                    #else
                         app_register_service(&app_service_discovery);  //No SMP, service discovery can initiated now
                    #endif
                }
            #endif
        }
    }

    return 0;
}



/**
 * @brief      BLE Disconnection event handler
 * @param[in]  p         Pointer point to event parameter buffer.
 * @return
 */
int     app_disconnect_event_handle(u8 *p)
{
    hci_disconnectionCompleteEvt_t  *pDisConn = (hci_disconnectionCompleteEvt_t *)p;
    tlkapi_send_string_data(APP_CONTR_EVT_LOG_EN, "[APP][EVT] disconnect event", &pDisConn->connHandle, 3);

    //terminate reason
    if(pDisConn->reason == HCI_ERR_CONN_TIMEOUT){   //connection timeout

    }
    else if(pDisConn->reason == HCI_ERR_REMOTE_USER_TERM_CONN){     //peer device send terminate command on link layer

    }
    else if(pDisConn->reason == HCI_ERR_CONN_TERM_BY_LOCAL_HOST){

    }
    else{

    }


    /* if previous connection SMP & SDP not finished, clear flag */
    #if (ACL_CENTRAL_SMP_ENABLE)
        if(central_smp_pending == pDisConn->connHandle){
            central_smp_pending = 0;
        }
    #endif
    #if (ACL_CENTRAL_SIMPLE_SDP_ENABLE)
        if(central_sdp_pending == pDisConn->connHandle){
            central_sdp_pending = 0;
        }
    #endif

    if(central_disconnect_connhandle == pDisConn->connHandle){  //un_pair disconnection flow finish, clear flag
        central_disconnect_connhandle = 0;
    }

    dev_char_info_delete_by_connhandle(pDisConn->connHandle);


    return 0;
}


/**
 * @brief      BLE Connection update complete event handler
 * @param[in]  p         Pointer point to event parameter buffer.
 * @return
 */
int app_le_connection_update_complete_event_handle(u8 *p)
{
    hci_le_connectionUpdateCompleteEvt_t *pUpt = (hci_le_connectionUpdateCompleteEvt_t *)p;
    tlkapi_send_string_data(APP_CONTR_EVT_LOG_EN, "[APP][EVT] Connection Update Event", &pUpt->connHandle, 8);

    if(pUpt->status == BLE_SUCCESS){

    }

    return 0;
}

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
    (void)n; //unused, remove warning

    if (h &HCI_FLAG_EVENT_BT_STD)       //Controller HCI event
    {
        u8 evtCode = h & 0xff;

        //------------ disconnect -------------------------------------
        if(evtCode == HCI_EVT_DISCONNECTION_COMPLETE)  //connection terminate
        {
            app_disconnect_event_handle(p);
        }
        else if(evtCode == HCI_EVT_LE_META)  //LE Event
        {
            u8 subEvt_code = p[0];

            //------hci le event: le connection complete event---------------------------------
            if (subEvt_code == HCI_SUB_EVT_LE_CONNECTION_COMPLETE)  // connection complete
            {
                app_le_connection_complete_event_handle(p);
            }
            //--------hci le event: le adv report event ----------------------------------------
            else if (subEvt_code == HCI_SUB_EVT_LE_ADVERTISING_REPORT)  // ADV packet
            {
                //after controller is set to scan state, it will report all the adv packet it received by this event

                app_le_adv_report_event_handle(p);
            }
            //------hci le event: le connection update complete event-------------------------------
            else if (subEvt_code == HCI_SUB_EVT_LE_CONNECTION_UPDATE_COMPLETE)  // connection update
            {
                app_le_connection_update_complete_event_handle(p);
            }
        }
    }


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
    (void)n; //unused, remove warning

    u8 event = h & 0xFF;

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



#define         HID_HANDLE_CONSUME_REPORT           25
#define         HID_HANDLE_KEYBOARD_REPORT          29
#define         AUDIO_HANDLE_MIC                    52
#define         OTA_HANDLE_DATA                     48

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
        #if (ACL_CENTRAL_SIMPLE_SDP_ENABLE)
            if(central_sdp_pending == connHandle ){  //ATT service discovery is ongoing on this conn_handle
                //when service discovery function is running, all the ATT data from peripheral
                //will be processed by it,  user can only send your own att cmd after  service discovery is over
                host_att_client_handler (connHandle, pkt); //handle this ATT data by service discovery process
            }
        #endif

        rf_packet_att_t *pAtt = (rf_packet_att_t*)pkt;

        //so any ATT data before service discovery will be dropped
        dev_char_info_t* dev_info = dev_char_info_search_by_connhandle (connHandle);
        if(dev_info)
        {
            //-------   user process ------------------------------------------------
            u16 attHandle = pAtt->handle;

            if(pAtt->opcode == ATT_OP_HANDLE_VALUE_NOTI)
            {
                    //---------------   consumer key --------------------------
                #if (ACL_CENTRAL_SIMPLE_SDP_ENABLE)
                    if(attHandle == dev_info->char_handle[3])  // Consume Report In (Media Key)
                #else
                    if(attHandle == HID_HANDLE_CONSUME_REPORT)   //Demo device(825x_ble_sample) Consume Report AttHandle value is 25
                #endif
                    {
                        att_keyboard_media (connHandle, pAtt->dat);
                    }
                    //---------------   keyboard key --------------------------
                #if (ACL_CENTRAL_SIMPLE_SDP_ENABLE)
                    else if(attHandle == dev_info->char_handle[4])     // Key Report In
                #else
                    else if(attHandle == HID_HANDLE_KEYBOARD_REPORT)   // Demo device(825x_ble_sample) Key Report AttHandle value is 29
                #endif
                    {
                        att_keyboard (connHandle, pAtt->dat);
                    }
                #if (ACL_CENTRAL_SIMPLE_SDP_ENABLE)
                    else if(attHandle == dev_info->char_handle[0])     // AUDIO Notify
                #else
                    else if(attHandle == AUDIO_HANDLE_MIC)   // Demo device(825x_ble_remote) Key Report AttHandle value is 52
                #endif
                    {

                    }
                    else
                    {

                    }
            }
            else if (pAtt->opcode == ATT_OP_HANDLE_VALUE_IND)
            {

            }
        }

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
    else{   //GATT data for Peripheral


    }


    return 0;
}



#if (BATT_CHECK_ENABLE)  //battery check must do before OTA relative operation

_attribute_data_retention_  u32 lowBattDet_tick   = 0;

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
    u8 battery_check_returnValue=0;
    if(analog_read(USED_DEEP_ANA_REG) & LOW_BATT_FLG){
        battery_check_returnValue=app_battery_power_check(alarm_vol_mv+200);
    }
    else{
        battery_check_returnValue=app_battery_power_check(alarm_vol_mv);
    }
    if(battery_check_returnValue)
    {
        analog_write_reg8(USED_DEEP_ANA_REG,  analog_read_reg8(USED_DEEP_ANA_REG) &(~LOW_BATT_FLG));  //clr
    }
    else
    {
        #if (UI_LED_ENABLE)  //led indicate
            for(int k=0;k<3;k++){
                gpio_write(GPIO_LED_BLUE, LED_ON_LEVEL);
                sleep_us(200000);
                gpio_write(GPIO_LED_BLUE, !LED_ON_LEVEL);
                sleep_us(200000);
            }
        #endif
        analog_write_reg8(USED_DEEP_ANA_REG,  analog_read_reg8(USED_DEEP_ANA_REG) | LOW_BATT_FLG);  //mark

        #if (UI_KEYBOARD_ENABLE)
        u32 pin[] = KB_DRIVE_PINS;
        for (u32 i=0; i<(sizeof (pin)/sizeof(*pin)); i++)
        {
            cpu_set_gpio_wakeup (pin[i], 1, 1);  //drive pin pad high wakeup deepsleep
        }

        cpu_sleep_wakeup(DEEPSLEEP_MODE, PM_WAKEUP_PAD, 0);  //deepsleep
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
_attribute_data_retention_ u16  flash_lockBlock_cmd;
void app_flash_protection_operation(u8 flash_op_evt, u32 op_addr_begin, u32 op_addr_end)
{
    (void)op_addr_begin;
    (void)op_addr_end;
    if(flash_op_evt == FLASH_OP_EVT_APP_INITIALIZATION)
    {
        /* ignore "op addr_begin" and "op addr_end" for initialization event
         * must call "flash protection_init" first, will choose correct flash protection relative API according to current internal flash type in MCU */
        flash_protection_init();

        /* just sample code here, protect all flash area for old firmware and OTA new firmware.
         * user can change this design if have other consideration */
        u32  app_lockBlock = 0;
        #if (BLE_OTA_SERVER_ENABLE)
            u32 multiBootAddress = blc_ota_getCurrentUsedMultipleBootAddress();
            if(multiBootAddress == MULTI_BOOT_ADDR_0x20000){
                app_lockBlock = FLASH_LOCK_FW_LOW_256K;
            }
            else if(multiBootAddress == MULTI_BOOT_ADDR_0x40000){
                app_lockBlock = FLASH_LOCK_FW_LOW_512K;
            }
            else if(multiBootAddress == MULTI_BOOT_ADDR_0x80000){
                /* attention that 1M capacity flash can not lock all 1M area, should leave some upper sector
                 * for system data(SMP storage data & calibration data & MAC address) and user data
                 * will use a approximate value */
                app_lockBlock = FLASH_LOCK_FW_LOW_1M;
            }
        #else
            app_lockBlock = FLASH_LOCK_FW_LOW_512K; //just demo value, user can change this value according to application
        #endif


        flash_lockBlock_cmd = flash_change_app_lock_block_to_flash_lock_block(app_lockBlock);

        if(blc_flashProt.init_err){
            tlkapi_printf(APP_FLASH_PROT_LOG_EN, "[FLASH][PROT] flash protection initialization error!!!\n");
        }

        tlkapi_printf(APP_FLASH_PROT_LOG_EN, "[FLASH][PROT] initialization, lock flash\n");
        flash_lock(flash_lockBlock_cmd);
    }
#if (BLE_OTA_SERVER_ENABLE)
    else if(flash_op_evt == FLASH_OP_EVT_STACK_OTA_CLEAR_OLD_FW_BEGIN)
    {
        /* OTA clear old firmware begin event is triggered by stack, in "blc ota_initOtaServer_module", rebooting from a successful OTA.
         * Software will erase whole old firmware for potential next new OTA, need unlock flash if any part of flash address from
         * "op addr_begin" to "op addr_end" is in locking block area.
         * In this sample code, we protect whole flash area for old and new firmware, so here we do not need judge "op addr_begin" and "op addr_end",
         * must unlock flash */
        tlkapi_printf(APP_FLASH_PROT_LOG_EN, "[FLASH][PROT] OTA clear old FW begin, unlock flash\n");
        flash_unlock();
    }
    else if(flash_op_evt == FLASH_OP_EVT_STACK_OTA_CLEAR_OLD_FW_END)
    {
        /* ignore "op addr_begin" and "op addr_end" for END event
         * OTA clear old firmware end event is triggered by stack, in "blc ota_initOtaServer_module", erasing old firmware data finished.
         * In this sample code, we need lock flash again, because we have unlocked it at the begin event of clear old firmware */
        tlkapi_printf(APP_FLASH_PROT_LOG_EN, "[FLASH][PROT] OTA clear old FW end, restore flash locking\n");
        flash_lock(flash_lockBlock_cmd);
    }
    else if(flash_op_evt == FLASH_OP_EVT_STACK_OTA_WRITE_NEW_FW_BEGIN)
    {
        /* OTA write new firmware begin event is triggered by stack, when receive first OTA data PDU.
         * Software will write data to flash on new firmware area,  need unlock flash if any part of flash address from
         * "op addr_begin" to "op addr_end" is in locking block area.
         * In this sample code, we protect whole flash area for old and new firmware, so here we do not need judge "op addr_begin" and "op addr_end",
         * must unlock flash */
        tlkapi_printf(APP_FLASH_PROT_LOG_EN, "[FLASH][PROT] OTA write new FW begin, unlock flash\n");
        flash_unlock();
    }
    else if(flash_op_evt == FLASH_OP_EVT_STACK_OTA_WRITE_NEW_FW_END)
    {
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








///////////////////////////////////////////

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

    #if(FLASH_4LINE_MODE_ENABLE)
    /* This function is used to set the use of 4 line mode when reading and writing flash.*/
    ble_flash_4line_enable();
    #endif

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
    
    blc_initMacAddress(flash_sector_mac_address, mac_public, mac_random_static);


    //////////// LinkLayer Initialization  Begin /////////////////////////
    blc_ll_initBasicMCU();

    blc_ll_initStandby_module(mac_public);

    blc_ll_initLegacyAdvertising_module();

    blc_ll_initLegacyScanning_module();

    blc_ll_initLegacyInitiating_module();

    blc_ll_initAclConnection_module();
    blc_ll_initAclCentralRole_module();
    blc_ll_initAclPeriphrRole_module();


    blc_ll_setMaxConnectionNumber(ACL_CENTRAL_MAX_NUM, ACL_PERIPHR_MAX_NUM);

    blc_ll_setAclConnMaxOctetsNumber(ACL_CONN_MAX_RX_OCTETS, ACL_CENTRAL_MAX_TX_OCTETS, ACL_PERIPHR_MAX_TX_OCTETS);

    /* all ACL connection share same RX FIFO */
    blc_ll_initAclConnRxFifo(app_acl_rx_fifo, ACL_RX_FIFO_SIZE, ACL_RX_FIFO_NUM);
    /* ACL Central TX FIFO */
    blc_ll_initAclCentralTxFifo(app_acl_cen_tx_fifo, ACL_CENTRAL_TX_FIFO_SIZE, ACL_CENTRAL_TX_FIFO_NUM, ACL_CENTRAL_MAX_NUM);
    /* ACL Peripheral TX FIFO */
    blc_ll_initAclPeriphrTxFifo(app_acl_per_tx_fifo, ACL_PERIPHR_TX_FIFO_SIZE, ACL_PERIPHR_TX_FIFO_NUM, ACL_PERIPHR_MAX_NUM);

    blc_ll_setAclCentralBaseConnectionInterval(CONN_INTERVAL_31P25MS);


    //////////// LinkLayer Initialization  End /////////////////////////



    //////////// HCI Initialization  Begin /////////////////////////
    blc_hci_registerControllerDataHandler (blc_l2cap_pktHandler);

    blc_hci_registerControllerEventHandler(app_controller_event_callback); //controller hci event to host all processed in this func

    //bluetooth event
    blc_hci_setEventMask_cmd (HCI_EVT_MASK_DISCONNECTION_COMPLETE);

    //bluetooth low energy(LE) event
    blc_hci_le_setEventMask_cmd(        HCI_LE_EVT_MASK_CONNECTION_COMPLETE  \
                                    |   HCI_LE_EVT_MASK_ADVERTISING_REPORT \
                                    |   HCI_LE_EVT_MASK_CONNECTION_UPDATE_COMPLETE);

    //////////// HCI Initialization  End /////////////////////////


    //////////// Host Initialization  Begin /////////////////////////
    /* Host Initialization */
    /* GAP initialization must be done before any other host feature initialization !!! */
    blc_gap_init();

    /* L2CAP data buffer Initialization */
    blc_l2cap_initAclCentralBuffer(app_cen_l2cap_rx_buf, CENTRAL_L2CAP_BUFF_SIZE, NULL, 0);
    blc_l2cap_initAclPeripheralBuffer(app_per_l2cap_rx_buf, PERIPHR_L2CAP_BUFF_SIZE, app_per_l2cap_tx_buf, PERIPHR_L2CAP_BUFF_SIZE);

    blc_att_setCentralRxMtuSize(CENTRAL_ATT_RX_MTU); ///must be placed after "blc_gap_init"
    blc_att_setPeripheralRxMtuSize(PERIPHR_ATT_RX_MTU);   ///must be placed after "blc_gap_init"

    /* GATT Initialization */
    my_gatt_init();
    #if (ACL_CENTRAL_SIMPLE_SDP_ENABLE)
        host_att_register_idle_func (main_idle_loop);
    #endif
    blc_gatt_register_data_handler(app_gatt_data_handler);

    /* SMP Initialization */
    #if (ACL_PERIPHR_SMP_ENABLE || ACL_CENTRAL_SMP_ENABLE)
        
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
    blc_gap_setEventMask( GAP_EVT_MASK_SMP_PAIRING_BEGIN            |  \
                          GAP_EVT_MASK_SMP_PAIRING_SUCCESS          |  \
                          GAP_EVT_MASK_SMP_PAIRING_FAIL             |  \
                          GAP_EVT_MASK_SMP_SECURITY_PROCESS_DONE);
    //////////// Host Initialization  End /////////////////////////

    /* Check if any Stack(Controller & Host) Initialization error after all BLE initialization done.
     * attention: user can not delete !!! */
    u32 error_code1 = blc_contr_checkControllerInitialization();
    u32 error_code2 = blc_host_checkHostInitialization();
    if(error_code1 != INIT_SUCCESS || error_code2 != INIT_SUCCESS){
        /* It's recommended that user set some UI alarm to know the exact error, e.g. LED shine, print log */
        #if (UI_LED_ENABLE)
            gpio_write(GPIO_LED_RED, LED_ON_LEVEL);
        #endif

        #if (TLKAPI_DEBUG_ENABLE)
            tlkapi_printf(APP_LOG_EN, "[APP][INI] Stack INIT ERROR 0x%04x, 0x%04x", error_code1, error_code2);
            while(1){
                tlkapi_debug_handler();
            }
        #else
            while(1);
        #endif
    }



//////////////////////////// BLE stack Initialization  End //////////////////////////////////




//////////////////////////// User Configuration for BLE application ////////////////////////////
    blc_ll_setAdvData(tbl_advData, sizeof(tbl_advData));
    blc_ll_setScanRspData(tbl_scanRsp, sizeof(tbl_scanRsp));
    blc_ll_setAdvParam(ADV_INTERVAL_50MS, ADV_INTERVAL_50MS, ADV_TYPE_CONNECTABLE_UNDIRECTED, OWN_ADDRESS_PUBLIC, 0, NULL, BLT_ENABLE_ADV_ALL, ADV_FP_NONE);
    blc_ll_setAdvEnable(BLC_ADV_ENABLE);  //ADV enable

    blc_ll_setScanParameter(SCAN_TYPE_PASSIVE, SCAN_INTERVAL_100MS, SCAN_WINDOW_50MS, OWN_ADDRESS_PUBLIC, SCAN_FP_ALLOW_ADV_ANY);
    blc_ll_setScanEnable (BLC_SCAN_ENABLE, DUP_FILTER_DISABLE);

    rf_set_power_level_index(RF_POWER_P3dBm);

    #if (BLE_APP_PM_ENABLE)
        blc_ll_initPowerManagement_module();
        blc_pm_setSleepMask(PM_SLEEP_LEG_ADV | PM_SLEEP_LEG_SCAN | PM_SLEEP_ACL_PERIPHR | PM_SLEEP_ACL_CENTRAL);
    #endif

    #if (BLE_OTA_SERVER_ENABLE)
        #if (TLKAPI_DEBUG_ENABLE)
            /* user can enable OTA flow log in BLE stack */
            //blc_debug_addStackLog(STK_LOG_OTA_FLOW);
        #endif

        blc_ota_initOtaServer_module();
    #endif

    tlkapi_send_string_data(APP_LOG_EN, "[APP][INI] acl connection demo init", 0, 0);
////////////////////////////////////////////////////////////////////////////////////////////////
}



/**
 * @brief       user initialization when MCU wake_up from deepSleep_retention mode
 * @param[in]   none
 * @return      none
 */
void user_init_deepRetn(void)
{
    //blc_app_loadCustomizedParameters_deepRetn();
}

void app_process_power_management(void)
{
#if (BLE_APP_PM_ENABLE)
    //Log needs to be output ASAP, and UART invalid after suspend. So Log disable sleep.
    //User tasks can go into suspend, but no deep sleep. So we use manual latency.
    if (tlkapi_debug_isBusy()) {
        blc_pm_setSleepMask(PM_SLEEP_DISABLE);
    } else {
        int user_task_flg = 0;

        blc_pm_setSleepMask(PM_SLEEP_LEG_ADV | PM_SLEEP_LEG_SCAN | PM_SLEEP_ACL_PERIPHR | PM_SLEEP_ACL_CENTRAL);

        #if (UI_KEYBOARD_ENABLE)
            user_task_flg |= user_task_flg || scan_pin_need || key_not_released;
        #endif

        if (user_task_flg){
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
int main_idle_loop (void)
{

    ////////////////////////////////////// BLE entry /////////////////////////////////
    blc_sdk_main_loop();


    ////////////////////////////////////// Debug entry /////////////////////////////////
    #if (TLKAPI_DEBUG_ENABLE)
        tlkapi_debug_handler();
    #endif

    ////////////////////////////////////// UI entry /////////////////////////////////
    #if (BATT_CHECK_ENABLE)
        /*The frequency of low battery detect is controlled by the variable lowBattDet_tick, which is executed every
         500ms in the demo. Users can modify this time according to their needs.*/
        if(battery_get_detect_enable() && clock_time_exceed(lowBattDet_tick, 500000) ){
            lowBattDet_tick = clock_time();
            user_battery_power_check(BAT_DEEP_THRESHOLD_MV);
        }
    #endif

    #if (UI_KEYBOARD_ENABLE)
        proc_keyboard (0, 0, 0);
    #endif


    proc_central_role_unpair();

    ////////////////////////////////////// PM entry /////////////////////////////////
    app_process_power_management();

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




