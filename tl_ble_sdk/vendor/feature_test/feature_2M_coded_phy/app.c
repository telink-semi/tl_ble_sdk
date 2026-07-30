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
#include "../default_buffer.h"
#include "../default_att.h"
#include "app_ui.h"

#include "../feature_app_parse_char.h"


#if (FEATURE_TEST_MODE == TEST_2M_CODED_PHY_CONNECTION)


_attribute_ble_data_retention_ int central_smp_pending = 0; // SMP: security & encryption;

static void print_hex_array(u8 *data, u16 data_length);

/**
 * @brief   BLE Advertising data
 */
const u8 tbl_advData[] = {
    11,
    DT_COMPLETE_LOCAL_NAME,
    'p',
    'h',
    'y',
    '-',
    'u',
    'p',
    'd',
    'a',
    't',
    'e',
    2,
    DT_FLAGS,
    0x05, // BLE limited discoverable mode and BR/EDR not supported
    3,
    DT_APPEARANCE,
    0x80,
    0x01, // 384, Generic Remote Control, Generic category
    5,
    DT_INCOMPLETE_LIST_16BIT_SERVICE_UUID,
    0x12,
    0x18,
    0x0F,
    0x18, // incomplete list of service class UUIDs (0x1812, 0x180F)
};

/**
 * @brief   BLE Scan Response Packet data
 */
const u8 tbl_scanRsp[] = {
    11,
    DT_COMPLETE_LOCAL_NAME,
    'p',
    'h',
    'y',
    '-',
    'u',
    'p',
    'd',
    'a',
    't',
    'e',
};


    /////////////////////////////////////////////////////////////////////
    // phy feature test
    /////////////////////////////////////////////////////////////////////
    #define SPP_DATA_LEN 8

static u8 phy_update_current[DEVICE_CHAR_INFO_MAX_NUM];

static u8 spp_data[DEVICE_CHAR_INFO_MAX_NUM][SPP_DATA_LEN];

static u32 feature_test_data_tick  = 0;
static u32 feature_phy_update_tick = 0;

void feature_2m_phy_test_init(void)
{
    u8 i, j;
    //spp data init
    feature_phy_update_tick = clock_time() | 0x01;
    feature_test_data_tick  = clock_time() | 0x01;
    for (i = 0; i < (DEVICE_CHAR_INFO_MAX_NUM); i++) {
        for (j = 1; j < SPP_DATA_LEN; j++) { ///start from 1, not zero.
            spp_data[i][j - 1] = j;
        }
        phy_update_current[i] = i % 3;
        ;
    }
}

void feature_2m_phy_test_mainloop(void)
{
    u16 connHandle;
    if (feature_test_data_tick && clock_time_exceed(feature_test_data_tick, 1 * 1000 * 1000)) //1S
    {
        feature_test_data_tick = clock_time() | 0x01;
    #if UI_LED_ENABLE
        gpio_toggle(GPIO_LED_BLUE); //led
    #endif
        for (u8 i = 0; i < (DEVICE_CHAR_INFO_MAX_NUM); i++) {
            if (!conn_dev_list[i].conn_state) {
                continue;
            }
            connHandle = conn_dev_list[i].conn_handle;
            /////send data
            u16 att_spp_handle;
            if (dev_char_get_conn_role_by_connhandle(connHandle) == ACL_ROLE_CENTRAL) {
                att_spp_handle = SPP_CLIENT_TO_SERVER_DP_H;                                                                   //blc_gatt_pushWriteCommand ///blc_gatt_pushHandleValueWriteReq
                if (BLE_SUCCESS == blc_gatt_pushWriteCommand(connHandle, att_spp_handle, (u8 *)&spp_data[i], SPP_DATA_LEN)) { //sizeof(spp_data)
                    tlkapi_printf(APP_LOG_EN, "[APP][GATT]blc_gatt_pushWriteCommand connHandle:%04X", connHandle);
                }
            } else {
                att_spp_handle = SPP_SERVER_TO_CLIENT_DP_H;
                ble_sts_t st   = blc_gatt_pushHandleValueNotify(connHandle, att_spp_handle, (u8 *)&spp_data[i], SPP_DATA_LEN);
                if (BLE_SUCCESS == st) { //sizeof(spp_data)
                    tlkapi_printf(APP_LOG_EN, "[APP][GATT]blc_gatt_pushHandleValueNotify connHandle:%04X", connHandle);
                }
            }
        }
    }

    if (feature_phy_update_tick && clock_time_exceed(feature_phy_update_tick, 1250 * 1000)) //1.25S * DEVICE_CHAR_INFO_MAX_NUM = 10 s
    {
        feature_phy_update_tick = clock_time() | 0x01;
    #if UI_LED_ENABLE
        gpio_toggle(GPIO_LED_BLUE); //led
    #endif
        static unsigned char char_num = 0;
        if (conn_dev_list[char_num].conn_state) {
            connHandle = conn_dev_list[char_num].conn_handle;
            ////phy switch
            if ((phy_update_current[0x0f & connHandle] & 0x03) == 0) {
                blc_ll_setPhy(connHandle, PHY_TRX_PREFER, PHY_PREFER_CODED, PHY_PREFER_CODED, CODED_PHY_PREFER_S8);
            } else if ((phy_update_current[0x0f & connHandle] & 0x03) == 1) {
                blc_ll_setPhy(connHandle, PHY_TRX_PREFER, PHY_PREFER_2M, PHY_PREFER_2M, CODED_PHY_PREFER_NONE);
            } else if ((phy_update_current[0x0f & connHandle] & 0x03) == 2) {
                blc_ll_setPhy(connHandle, PHY_TRX_PREFER, PHY_PREFER_1M, PHY_PREFER_1M, CODED_PHY_PREFER_NONE);
            }
            phy_update_current[0x0f & connHandle]++;
            phy_update_current[0x0f & connHandle] %= 3;
        }
        char_num = ((char_num + 1) % DEVICE_CHAR_INFO_MAX_NUM);
    }
}

/**
 * @brief      BLE Adv report event handler
 * @param[in]  p         Pointer point to event parameter buffer.
 * @return
 */
int AA_dbg_adv_rpt = 0;
u32 tick_adv_rpt   = 0;

int app_le_adv_report_event_handle(u8 *p)
{
    event_adv_report_t *pa   = (event_adv_report_t *)p;
    s8                  rssi = pa->data[pa->len];

    #if 0 //debug, print ADV report number every 5 seconds
        AA_dbg_adv_rpt ++;
        if(clock_time_exceed(tick_adv_rpt, 5000000)){
            tlkapi_send_string_data(APP_CONTR_EVT_LOG_EN, "[APP][EVT] Adv report", pa->mac, 6);
            tick_adv_rpt = clock_time();
        }
    #endif

    /*********************** Central Create connection demo: Key press or ADV pair packet triggers pair  ********************/
    #if (ACL_CENTRAL_SMP_ENABLE)
    if (central_smp_pending) { //if previous connection SMP not finish, can not create a new connection
        return 1;
    }
    #endif

    #if (ACL_CENTRAL_SIMPLE_SDP_ENABLE)
    if (central_sdp_pending) { //if previous connection SDP not finish, can not create a new connection
        return 1;
    }
    #endif

    if (central_disconnect_connhandle) { //one ACL connection central role is in un_pair disconnection flow, do not create a new one
        return 1;
    }

    int central_auto_connect = 0;
    int user_manual_pairing  = 0;

    //manual pairing methods 1: key press triggers
    user_manual_pairing = central_pairing_enable && (rssi > -56); //button trigger pairing(RSSI threshold, short distance)

    #if (ACL_CENTRAL_SMP_ENABLE)
    central_auto_connect = blc_smp_searchBondingPeripheralDevice_by_PeerMacAddress(pa->adr_type, pa->mac);
    #endif

    if (central_auto_connect || user_manual_pairing) {
        /* send create connection command to Controller, trigger it switch to initiating state. After this command, Controller
         * will scan all the ADV packets it received but not report to host, to find the specified device(mac_adr_type & mac_adr),
         * then send a "CONN_REQ" packet, enter to connection state and send a connection complete event
         * (HCI_SUB_EVT_LE_CONNECTION_COMPLETE) to Host*/
        tlkapi_printf(APP_LOG_EN, "[APP][EVT]le_createConnection mac:%02X %02X %02X %02X %02X %02X", pa->mac[0], pa->mac[1], pa->mac[2], pa->mac[3], pa->mac[4], pa->mac[5]);

        u8 status = blc_ll_createConnection(SCAN_INTERVAL_100MS, SCAN_WINDOW_100MS, INITIATE_FP_ADV_SPECIFY, pa->adr_type, pa->mac, OWN_ADDRESS_PUBLIC, CONN_INTERVAL_31P25MS, CONN_INTERVAL_48P75MS, 0, CONN_TIMEOUT_4S, 0, 0xFFFF);


        if (status == BLE_SUCCESS) { //create connection success

        } else {
            tlkapi_printf(APP_LOG_EN, "blc_ll_createConnection error code :%02X", status);
        }
    }
    /*********************** Central Create connection demo code end  *******************************************************/


    return 0;
}
static void print_hex_array(u8 *data, u16 data_length)
{
    u8 print_buffer[64] = {0};

    while (data_length) {
        snprintf(print_buffer + strlen(print_buffer), sizeof(print_buffer), "%02X", *data);

        data++;
        data_length--;

        if (sizeof(print_buffer) - strlen(print_buffer) < 2) {
            // No more space in print buffer - print it
            app_parse_printf("%s", print_buffer);
            print_buffer[0] = 0;
        }
    }

    if (strlen(print_buffer)) {
        app_parse_printf("%s", print_buffer);
    }
}
static int app_le_adv_report_event_handle_console(u8 *p)
{
    event_adv_report_t *pa   = (event_adv_report_t *)p;
//    s8                  rssi = pa->data[pa->len];

    app_parse_printf("EXT Adv report type:%04X addr_type: %02X [%02X:%02X:%02X:%02X:%02X:%02X]:",
                        pa->event_type,
                        pa->adr_type,
                        pa->mac[0],
                        pa->mac[1],
                        pa->mac[2],
                        pa->mac[3],
                        pa->mac[4],
                        pa->mac[5]);

    print_hex_array(pa->data, pa->len);

    app_parse_printf("\r\n");

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

    if (pConnEvt->status == BLE_SUCCESS) {
        tlkapi_printf(APP_CONTR_EVT_LOG_EN, "[APP][EVT]le_connection_complete connHandle:%04X mac:%02X %02X %02X %02X %02X %02X", pConnEvt->connHandle, pConnEvt->peerAddr[0], pConnEvt->peerAddr[1], pConnEvt->peerAddr[2], pConnEvt->peerAddr[3], pConnEvt->peerAddr[4], pConnEvt->peerAddr[5]);
        tlkapi_printf(APP_CONTR_EVT_LOG_EN, "[APP][EVT]connection_complete Interval:%04X Latency:%04X Timeout:%04X", pConnEvt->connInterval, pConnEvt->peripheralLatency, pConnEvt->supervisionTimeout);
#if (APP_PARSE_CHAR_ENABLE)
        app_parse_printf("[APP][EVT]le_connection_complete connHandle:%04X mac:%02X %02X %02X %02X %02X %02X\r\n", pConnEvt->connHandle, pConnEvt->peerAddr[0], pConnEvt->peerAddr[1], pConnEvt->peerAddr[2], pConnEvt->peerAddr[3], pConnEvt->peerAddr[4], pConnEvt->peerAddr[5]);
        app_parse_printf("[APP][EVT]connection_complete Interval:%04X Latency:%04X Timeout:%04X\r\n", pConnEvt->connInterval, pConnEvt->peripheralLatency, pConnEvt->supervisionTimeout);
#endif
        dev_char_info_insert_by_conn_event(pConnEvt);

        if (pConnEvt->role == ACL_ROLE_CENTRAL)         // central role, process SMP and SDP if necessary
        {
    #if (ACL_CENTRAL_SMP_ENABLE)
            central_smp_pending = pConnEvt->connHandle; // this connection need SMP
    #endif


    #if (ACL_CENTRAL_SIMPLE_SDP_ENABLE)
            memset(&cur_sdp_device, 0, sizeof(dev_char_info_t));
            cur_sdp_device.conn_handle  = pConnEvt->connHandle;
            cur_sdp_device.peer_adrType = pConnEvt->peerAddrType;
            memcpy(cur_sdp_device.peer_addr, pConnEvt->peerAddr, 6);

            u8         temp_buff[sizeof(dev_att_t)];
            dev_att_t *pdev_att = (dev_att_t *)temp_buff;

            /* att_handle search in flash, if success, load char_handle directly from flash, no need SDP again */
            if (dev_char_info_search_peer_att_handle_by_peer_mac(pConnEvt->peerAddrType, pConnEvt->peerAddr, pdev_att)) {
                //cur_sdp_device.char_handle[1] =                                   //Speaker
                cur_sdp_device.char_handle[2] = pdev_att->char_handle[2]; //OTA
                cur_sdp_device.char_handle[3] = pdev_att->char_handle[3]; //consume report
                cur_sdp_device.char_handle[4] = pdev_att->char_handle[4]; //normal key report
                //cur_sdp_device.char_handle[6] =                                   //BLE Module, SPP Server to Client
                //cur_sdp_device.char_handle[7] =                                   //BLE Module, SPP Client to Server

                /* add the peer device att_handle value to conn_dev_list */
                dev_char_info_add_peer_att_handle(&cur_sdp_device);
                #if (APP_PARSE_CHAR_ENABLE)
                    app_parse_printf("SDP done:flash\r\n");  // Needed to inform automated test
                #endif
            } else {
                central_sdp_pending = pConnEvt->connHandle; // mark this connection need SDP

        #if (ACL_CENTRAL_SMP_ENABLE)
                    //service discovery initiated after SMP done, trigger it in "GAP_EVT_MASK_SMP_SECURITY_PROCESS_DONE" event callBack.
        #else
                app_register_service(&app_service_discovery); //No SMP, service discovery can initiated now
        #endif
            }
    #endif
        }
    } else {
        tlkapi_printf(APP_CONTR_EVT_LOG_EN, "[APP][EVT]le_connection_complete connHandle:%04X status:%02X", pConnEvt->connHandle, pConnEvt->status);
    }

    return 0;
}

/**
 * @brief      BLE Disconnection event handler
 * @param[in]  p         Pointer point to event parameter buffer.
 * @return
 */
int app_disconnect_event_handle(u8 *p)
{
    hci_disconnectionCompleteEvt_t *pDisConn = (hci_disconnectionCompleteEvt_t *)p;
    tlkapi_printf(APP_CONTR_EVT_LOG_EN, "[APP][EVT]Disconnect event connHandle:%04X reason:%02X", pDisConn->connHandle, pDisConn->reason);

    //terminate reason
    if (pDisConn->reason == HCI_ERR_CONN_TIMEOUT) {                 //connection timeout

    } else if (pDisConn->reason == HCI_ERR_REMOTE_USER_TERM_CONN) { //peer device send terminate command on link layer

    }
    //central host disconnect( blm_ll_disconnect(current_connHandle, HCI_ERR_REMOTE_USER_TERM_CONN) )
    else if (pDisConn->reason == HCI_ERR_CONN_TERM_BY_LOCAL_HOST) {
    } else {
    }


    /* if previous connection SMP & SDP not finished, clear flag */
    #if (ACL_CENTRAL_SMP_ENABLE)
    if (central_smp_pending == pDisConn->connHandle) {
        central_smp_pending = 0;
    }
    #endif
    #if (ACL_CENTRAL_SIMPLE_SDP_ENABLE)
    if (central_sdp_pending == pDisConn->connHandle) {
        central_sdp_pending = 0;
    }
    #endif

    if (central_disconnect_connhandle == pDisConn->connHandle) { //un_pair disconnection flow finish, clear flag
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

    if (pUpt->status == BLE_SUCCESS) {
        //connInterval        ->   conn_inter_t
        //connLatency
        //supervisionTimeout  ->   conn_tm_t
        tlkapi_printf(APP_CONTR_EVT_LOG_EN, "[APP][EVT] Connection Update Event handle:%04X Interval:%04X Latency:%04X Timeout:%04X ", pUpt->connHandle, pUpt->connInterval, pUpt->connLatency, pUpt->supervisionTimeout);
#if (APP_PARSE_CHAR_ENABLE)
        app_parse_printf("[APP][EVT] Connection Update Event handle:%04X Interval:%04X Latency:%04X Timeout:%04X\r\n", pUpt->connHandle, pUpt->connInterval, pUpt->connLatency, pUpt->supervisionTimeout);
#endif
    } else {
        tlkapi_printf(APP_CONTR_EVT_LOG_EN, "[APP][EVT] Connection Update Event error code:%02X", pUpt->status);
#if (APP_PARSE_CHAR_ENABLE)
        app_parse_printf("[APP][EVT] Connection Update Event error code:%02X", pUpt->status);
#endif
    }
    return 0;
}

/**
 * @brief      BLE phy update complete event handler
 * @param[in]  p         Pointer point to event parameter buffer.
 * @return
 */
int app_le_phy_update_complete_event_handle(u8 *p)
{
    hci_le_phyUpdateCompleteEvt_t *pPhy = (hci_le_phyUpdateCompleteEvt_t *)p;

    if (pPhy->status == BLE_SUCCESS) {
        //rx|tx phy Value |   Parameter Description
        //    0x01        |   The receiver PHY for the connection is LE 1M
        //    0x02        |   The receiver PHY for the connection is LE 2M
        //    0x03        |   The receiver PHY for the connection is LE Coded
        tlkapi_printf(APP_CONTR_EVT_LOG_EN, "[APP][EVT] phy Update Event handle:%04X rx_phy:%02X tx_phy:%02X", pPhy->connHandle, pPhy->rx_phy, pPhy->tx_phy);
#if (APP_PARSE_CHAR_ENABLE)
        app_parse_printf("[APP][EVT] phy Update Event handle:%04X rx_phy:%02X tx_phy:%02X\r\n", pPhy->connHandle, pPhy->rx_phy, pPhy->tx_phy);
#endif
    } else {
        tlkapi_printf(APP_CONTR_EVT_LOG_EN, "[APP][EVT] phy Update Event error code:%02X", pPhy->status);
#if (APP_PARSE_CHAR_ENABLE)
        app_parse_printf("[APP][EVT] phy Update Event error code:%02X\r\n", pPhy->status);
#endif
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
int app_controller_event_callback(u32 h, u8 *p, int n)
{
    if (h & HCI_FLAG_EVENT_BT_STD) //Controller HCI event
    {
        u8 evtCode = h & 0xff;

        //------------ disconnect -------------------------------------
        if (evtCode == HCI_EVT_DISCONNECTION_COMPLETE) //connection terminate
        {
            app_disconnect_event_handle(p);
        } else if (evtCode == HCI_EVT_LE_META)         //LE Event
        {
            u8 subEvt_code = p[0];

            //------hci le event: le connection complete event---------------------------------
            if (subEvt_code == HCI_SUB_EVT_LE_CONNECTION_COMPLETE) // connection complete
            {
                app_le_connection_complete_event_handle(p);
            }
            //--------hci le event: le adv report event ----------------------------------------
            else if (subEvt_code == HCI_SUB_EVT_LE_ADVERTISING_REPORT) // ADV packet
            {
                //after controller is set to scan state, it will report all the adv packet it received by this event
                #if (APP_PARSE_CHAR_ENABLE)
                app_le_adv_report_event_handle_console(p);
                #else
                app_le_adv_report_event_handle(p);
                #endif
            }
            //------hci le event: le connection update complete event-------------------------------
            else if (subEvt_code == HCI_SUB_EVT_LE_CONNECTION_UPDATE_COMPLETE) // connection update
            {
                app_le_connection_update_complete_event_handle(p);
            } else if (subEvt_code == HCI_SUB_EVT_LE_PHY_UPDATE_COMPLETE) {
                app_le_phy_update_complete_event_handle(p);
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
int app_host_event_callback(u32 h, u8 *para, int n)
{
    u8 event = h & 0xFF;

    switch (event) {
    case GAP_EVT_SMP_PAIRING_BEGIN:
    {
    } break;

    case GAP_EVT_SMP_PAIRING_SUCCESS:
    {
    } break;

    case GAP_EVT_SMP_PAIRING_FAIL:
    {
    #if (ACL_CENTRAL_SMP_ENABLE)
        gap_smp_pairingFailEvt_t *pEvt = (gap_smp_pairingFailEvt_t *)para;

        if (dev_char_get_conn_role_by_connhandle(pEvt->connHandle) == ACL_ROLE_CENTRAL) {
            if (central_smp_pending == pEvt->connHandle) {
                central_smp_pending = 0;
                tlkapi_send_string_data(APP_SMP_LOG_EN, "[APP][SMP] paring fail", &pEvt->connHandle, sizeof(gap_smp_pairingFailEvt_t));
            }
        }
    #endif
    } break;

    case GAP_EVT_SMP_CONN_ENCRYPTION_DONE:
    {
        gap_smp_connEncDoneEvt_t *pEvt = (gap_smp_connEncDoneEvt_t *)para;
        tlkapi_send_string_data(APP_SMP_LOG_EN, "[APP][SMP] Connection encryption done event", &pEvt->connHandle, sizeof(gap_smp_connEncDoneEvt_t));
    } break;

    case GAP_EVT_SMP_SECURITY_PROCESS_DONE:
    {
        gap_smp_connEncDoneEvt_t *pEvt = (gap_smp_connEncDoneEvt_t *)para;
        tlkapi_send_string_data(APP_SMP_LOG_EN, "[APP][SMP] Security process done event", &pEvt->connHandle, sizeof(gap_smp_connEncDoneEvt_t));
        #if (APP_PARSE_CHAR_ENABLE)
            app_parse_printf("GAP_EVT_SMP_SECURITY_PROCESS_DONE\r\n");
        #endif
        if (dev_char_get_conn_role_by_connhandle(pEvt->connHandle) == ACL_ROLE_CENTRAL) {
    #if (ACL_CENTRAL_SMP_ENABLE)
            if (dev_char_get_conn_role_by_connhandle(pEvt->connHandle) == ACL_ROLE_CENTRAL) {
                if (central_smp_pending == pEvt->connHandle) {
                    central_smp_pending = 0;
                }
            }
    #endif

    #if (ACL_CENTRAL_SIMPLE_SDP_ENABLE)                       //SMP finish
            if (central_sdp_pending == pEvt->connHandle) {    //SDP is pending
                app_register_service(&app_service_discovery); //start SDP now
            }
    #endif
        }
    } break;

    case GAP_EVT_SMP_TK_DISPLAY:
    {
    } break;

    case GAP_EVT_SMP_TK_REQUEST_PASSKEY:
    {
    } break;

    case GAP_EVT_SMP_TK_REQUEST_OOB:
    {
    } break;

    case GAP_EVT_SMP_TK_NUMERIC_COMPARE:
    {
    } break;

    case GAP_EVT_ATT_EXCHANGE_MTU:
    {
    } break;

    case GAP_EVT_GATT_HANDLE_VALUE_CONFIRM:
    {
    } break;

    default:
        break;
    }

    return 0;
}

    #define HID_HANDLE_CONSUME_REPORT  25
    #define HID_HANDLE_KEYBOARD_REPORT 29
    #define AUDIO_HANDLE_MIC           52
    #define OTA_HANDLE_DATA            48

/**
 * @brief      BLE GATT data handler call-back.
 * @param[in]  connHandle     connection handle.
 * @param[in]  pkt             Pointer point to data packet buffer.
 * @return
 */
int app_gatt_data_handler(u16 connHandle, u8 *pkt)
{
    if (dev_char_get_conn_role_by_connhandle(connHandle) == ACL_ROLE_CENTRAL) //GATT data for Central
    {
    #if (ACL_CENTRAL_SIMPLE_SDP_ENABLE)
        if (central_sdp_pending == connHandle) {                              //ATT service discovery is ongoing on this conn_handle
            //when service discovery function is running, all the ATT data from peripheral
            //will be processed by it,  user can only send your own att cmd after  service discovery is over
            host_att_client_handler(connHandle, pkt); //handle this ATT data by service discovery process
        }
    #endif

        rf_packet_att_t *pAtt = (rf_packet_att_t *)pkt;

        #if (APP_PARSE_CHAR_ENABLE)
            app_parse_printf("Data:");
            print_hex_array(pAtt->dat, 20); // magic number in structure definition
            app_parse_printf("\r\n");
        #endif

        //so any ATT data before service discovery will be dropped
        dev_char_info_t *dev_info = dev_char_info_search_by_connhandle(connHandle);
        if (dev_info) {
            //-------   user process ------------------------------------------------
            u16 attHandle = pAtt->handle;

            if (pAtt->opcode == ATT_OP_HANDLE_VALUE_NOTI) //peripheral handle notify
            {
                //---------------   consumer key --------------------------
    #if (ACL_CENTRAL_SIMPLE_SDP_ENABLE)
                if (attHandle == dev_info->char_handle[3]) // Consume Report In (Media Key)
    #else
                if (attHandle == HID_HANDLE_CONSUME_REPORT) //Demo device(825x_ble_sample) Consume Report AttHandle value is 25
    #endif
                {
                    att_keyboard_media(connHandle, pAtt->dat);
                }
                //---------------   keyboard key --------------------------
    #if (ACL_CENTRAL_SIMPLE_SDP_ENABLE)
                else if (attHandle == dev_info->char_handle[4]) // Key Report In
    #else
                else if (attHandle == HID_HANDLE_KEYBOARD_REPORT) // Demo device(825x_ble_sample) Key Report AttHandle value is 29
    #endif
                {
                    att_keyboard(connHandle, pAtt->dat);
                }
    #if (ACL_CENTRAL_SIMPLE_SDP_ENABLE)
                else if (attHandle == dev_info->char_handle[0]) // AUDIO Notify
    #else
                else if (attHandle == AUDIO_HANDLE_MIC) // Demo device(825x_ble_remote) Key Report AttHandle value is 52
    #endif
                {

                } else {
                }
            } else if (pAtt->opcode == ATT_OP_HANDLE_VALUE_IND) {
            }
        }

        if (!(pAtt->opcode & 0x01)) {
            switch (pAtt->opcode) {
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
            default: //no action
                break;
            }
        }
    } else { //GATT data for Peripheral
    }


    return 0;
}

#if (APP_PARSE_CHAR_ENABLE)
static void help_fun(char *argv[], int argc, void *user_data);

static void ping_fun(char *argv[], int argc, void *user_data)
{
     app_parse_printf("pong\r\n");

    return;

}

static void scan_fun(char *argv[], int argc, void *user_data)
{
    dupFilter_en_t filter = DUP_FILTER_DISABLE;
    ble_sts_t      status;
    scan_en_t      en;
    tlkapi_send_string_data(APP_LOG_EN, "[APP][EXE] scan en", 0, 0);
    if (argc < 1) {
        goto help;
    }

    if (!strcasecmp(argv[0], "on")) {
        en = BLC_SCAN_ENABLE;
    } else if (!strcasecmp(argv[0], "off")) {
        en = BLC_SCAN_DISABLE;
    } else {
        goto help;
    }

    if (argc > 1 && !strcasecmp(argv[1], "--dup_en")) {
        filter = DUP_FILTER_ENABLE;
    }

    status = blc_ll_setScanEnable(en, filter);
    app_parse_printf("scan_en status:0x%02X\r\n", status);

    return;

help:
    app_parse_printf("scan_en <on|off> [--dup_en]\r\n");
}

static void read_bdaddr(char *argv[], int argc, void *user_data)
{
    u8 addr[6];

    if (blc_ll_readBDAddr(addr) == BLE_SUCCESS) {
        app_parse_printf("read_bdaddr: %02X:%02X:%02X:%02X:%02X:%02X\r\n",
                         addr[0],
                         addr[1],
                         addr[2],
                         addr[3],
                         addr[4],
                         addr[5]);
    } else {
        app_parse_printf("read_bdaddr: failed\r\n");
    }
}

static bool app_parse_bdaddr(const char *str, u8 *addr)
{
    u8 pos = 0;

    if (strlen(str) != 17) {
        return false;
    }

    for (u8 i = 0; i < 6; i++) {
        u8 temp[3] = {str[pos], str[pos + 1], 0};

        app_parse_str2hex(temp, &addr[i], 1);
        pos += 2;
        if (i < 5 && str[pos++] != ':') {
            return false;
        }
    }

    return true;
}

static void cc_fun(char *argv[], int argc, void *user_data)
{
    u8        addr_type = PEERATYPE_PUBLIC_DEVICE_ADDRESS;
    int       argc_addr = 0;
    ble_sts_t status;
    u8        addr[6];

    if (argc < 1) {
        goto help;
    }

    if (argc > 1) {
        addr_type = app_parse_str2n(argv[argc_addr++]);
    }

    if (!app_parse_bdaddr(argv[argc_addr], addr)) {
        goto help;
    }

    status = blc_ll_createConnection(SCAN_INTERVAL_100MS, SCAN_WINDOW_100MS, INITIATE_FP_ADV_SPECIFY, addr_type, addr, OWN_ADDRESS_PUBLIC, CONN_INTERVAL_31P25MS, CONN_INTERVAL_48P75MS, 0, CONN_TIMEOUT_4S, 0, 0xFFFF);
    //status = blc_ll_extended_createConnection(INITIATE_FP_ADV_SPECIFY, OWN_ADDRESS_PUBLIC, addr_type, addr, INIT_PHY_1M, SCAN_INTERVAL_100MS, SCAN_WINDOW_100MS, CONN_INTERVAL_100MS, CONN_INTERVAL_100MS, CONN_TIMEOUT_4S, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);

    app_parse_printf("Connect %02X:%02X:%02X:%02X:%02X:%02X status:%02X\r\n", addr[0], addr[1], addr[2], addr[3], addr[4], addr[5], status);

    return;

help:
    app_parse_printf("cc [addr_type] <bdaddr>\r\n");
}

static void data_fun(char *argv[], int argc, void *user_data) {

    u16 connHandle;
    u16 att_spp_handle;

    if (argc < 1) {
        goto help;
    }
    connHandle = app_parse_str2n(argv[0]);

    if (dev_char_get_conn_role_by_connhandle(connHandle) == ACL_ROLE_CENTRAL) {
        att_spp_handle = SPP_CLIENT_TO_SERVER_DP_H;
        ble_sts_t st =blc_gatt_pushWriteCommand(connHandle, att_spp_handle, (u8 *)&spp_data[0], SPP_DATA_LEN);                                                              //blc_gatt_pushWriteCommand ///blc_gatt_pushHandleValueWriteReq
        if (BLE_SUCCESS == st ) {
            app_parse_printf("OK\r\n");
        } else {
            app_parse_printf("ERROR:%d\r\n", st);
        }
    } else {
        att_spp_handle = SPP_SERVER_TO_CLIENT_DP_H;
        ble_sts_t st   = blc_gatt_pushHandleValueNotify(connHandle, att_spp_handle, (u8 *)&spp_data[0], SPP_DATA_LEN);
        if (BLE_SUCCESS == st) { //sizeof(spp_data)
            app_parse_printf("OK\r\n");
        } else {
            app_parse_printf("ERROR:%d\r\n", st);
        }
    }
    return;
help:
    app_parse_printf("data <connHandle>\r\n");
}

static void phy_fun(char *argv[], int argc, void *user_data) {

    u16 connHandle;
    ble_sts_t retVal = HCI_ERR_CMD_DISALLOWED;

    if (argc < 2) {
        goto help;
    }
    connHandle = app_parse_str2n(argv[0]);

    if(strcmp(argv[1], "CD") == 0) {
         retVal = blc_ll_setPhy(connHandle, PHY_TRX_PREFER, PHY_PREFER_CODED, PHY_PREFER_CODED, CODED_PHY_PREFER_S8);
    } else if(strcmp(argv[1], "2M") == 0) {
        retVal = blc_ll_setPhy(connHandle, PHY_TRX_PREFER, PHY_PREFER_2M, PHY_PREFER_2M, CODED_PHY_PREFER_NONE);
    } else if (strcmp(argv[1], "1M") == 0) {
        retVal = blc_ll_setPhy(connHandle, PHY_TRX_PREFER, PHY_PREFER_1M, PHY_PREFER_1M, CODED_PHY_PREFER_NONE);
    }

    if(retVal == BLE_SUCCESS) {
        app_parse_printf("OK\r\n");
    } else {
        app_parse_printf("ERROR:%d\r\n", retVal);
    }
    return;
help:
    app_parse_printf("phy <connHandle> <PHY:1M or 2M or CD>\r\n");
}

static const parse_fun_list_t app_funcs[] = {
    {"help",        help_fun,    NULL},
    {"ping",        ping_fun,    NULL},
    {"scan",        scan_fun,    NULL},
    {"read_bdaddr", read_bdaddr, NULL},
    {"cc",          cc_fun,      NULL},
    {"data",        data_fun,    NULL},
    {"phy",         phy_fun,     NULL},
};

static void help_fun(char *argv[], int argc, void *user_data)
{
    app_parse_printf("Commands:\r\n");

    foreach_arr(i, app_funcs)
    {
        app_parse_printf("\t%s\r\n", app_funcs[i].fun_name);
    }
}

int my_client_2_server_write_callback(u16 connHandle, void *p)
{
    rf_packet_att_data_t *req = (rf_packet_att_data_t *)p;

    #if (APP_PARSE_CHAR_ENABLE)
        app_parse_printf("Data:");
        print_hex_array(req->dat, 20); // magic number in structure definition
        app_parse_printf("\r\n");
    #endif
}

#endif
///////////////////////////////////////////

#if (ACL_CENTRAL_SIMPLE_SDP_ENABLE)
int cb_simple_discovery_done(u32 current_flash_adr, void *p) {
    #if (APP_PARSE_CHAR_ENABLE)
        app_parse_printf("SDP done:discovery\r\n");  // Needed to inform automated test
    #endif
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

    blc_readFlashSize_autoConfigCustomFlashSector();

    /* attention that this function must be called after "blc readFlashSize_autoConfigCustomFlashSector" !!!*/
    blc_app_loadCustomizedParameters_normal();
    //////////////////////////// basic hardware Initialization  End /////////////////////////////////


    //////////////////////////// BLE stack Initialization  Begin //////////////////////////////////

    u8 mac_public[6];
    u8 mac_random_static[6];

    blc_initMacAddress(flash_sector_mac_address, mac_public, mac_random_static);

    #if defined(TLK_ONLY_BLE_HOST)
    sys_n22_start();
    delay_ms(300);
    #endif

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

    blc_ll_init2MPhyCodedPhy_feature();
    //////////// LinkLayer Initialization  End /////////////////////////


    //////////// HCI Initialization  Begin /////////////////////////
    blc_hci_registerControllerDataHandler(blc_l2cap_pktHandler);

    blc_hci_registerControllerEventHandler(app_controller_event_callback); //controller hci event to host all processed in this func

    //bluetooth event
    blc_hci_setEventMask_cmd(HCI_EVT_MASK_DISCONNECTION_COMPLETE);

    //bluetooth low energy(LE) event
    blc_hci_le_setEventMask_cmd(HCI_LE_EVT_MASK_CONNECTION_COMPLETE | HCI_LE_EVT_MASK_ADVERTISING_REPORT | HCI_LE_EVT_MASK_CONNECTION_UPDATE_COMPLETE | HCI_LE_EVT_MASK_PHY_UPDATE_COMPLETE);


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
    //////////// HCI Initialization  End /////////////////////////


    //////////// Host Initialization  Begin /////////////////////////
    /* Host Initialization */
    /* GAP initialization must be done before any other host feature initialization !!! */
    blc_gap_init();

    /* L2CAP data buffer Initialization */
    blc_l2cap_initAclCentralBuffer(app_cen_l2cap_rx_buf, CENTRAL_L2CAP_BUFF_SIZE, NULL, 0);
    blc_l2cap_initAclPeripheralBuffer(app_per_l2cap_rx_buf, PERIPHR_L2CAP_BUFF_SIZE, app_per_l2cap_tx_buf, PERIPHR_L2CAP_BUFF_SIZE);

    blc_att_setCentralRxMtuSize(CENTRAL_ATT_RX_MTU);    ///must be placed after "blc_gap_init"
    blc_att_setPeripheralRxMtuSize(PERIPHR_ATT_RX_MTU); ///must be placed after "blc_gap_init"

    /* GATT Initialization */
    my_gatt_init();
    #if (ACL_CENTRAL_SIMPLE_SDP_ENABLE)
    host_att_register_idle_func(main_idle_loop);
    simple_sdp_register_store_info_callback(cb_simple_discovery_done);
    #endif
    blc_gatt_register_data_handler(app_gatt_data_handler);

    #if (APP_PARSE_CHAR_ENABLE)
    blc_spp_registerWriteCallback(my_client_2_server_write_callback);
    #endif

    /* SMP Initialization */
    #if (ACL_PERIPHR_SMP_ENABLE || ACL_CENTRAL_SMP_ENABLE)

    blc_smp_configPairingSecurityInfoStorageAddressAndSize(flash_sector_smp_storage, FLASH_SMP_PAIRING_MAX_SIZE);
    #endif

    #if (ACL_PERIPHR_SMP_ENABLE)                                               //Peripheral SMP Enable
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


    //host(GAP/SMP/GATT/ATT) event process: register host event callback and set event mask
    blc_gap_registerHostEventHandler(app_host_event_callback);
    blc_gap_setEventMask(GAP_EVT_MASK_SMP_PAIRING_BEGIN |
                         GAP_EVT_MASK_SMP_PAIRING_SUCCESS |
                         GAP_EVT_MASK_SMP_PAIRING_FAIL |
                         GAP_EVT_MASK_SMP_SECURITY_PROCESS_DONE);
    blc_att_setServerDataPendingTime_upon_ClientCmd(0);
    //////////// Host Initialization  End /////////////////////////

    //////////////////////////// BLE stack Initialization  End //////////////////////////////////


    //////////////////////////// User Configuration for BLE application ////////////////////////////
    blc_ll_setAdvData(tbl_advData, sizeof(tbl_advData));
    blc_ll_setScanRspData(tbl_scanRsp, sizeof(tbl_scanRsp));
    blc_ll_setAdvParam(ADV_INTERVAL_50MS, ADV_INTERVAL_50MS, ADV_TYPE_CONNECTABLE_UNDIRECTED, OWN_ADDRESS_PUBLIC, 0, NULL, BLT_ENABLE_ADV_ALL, ADV_FP_NONE);
    blc_ll_setAdvEnable(BLC_ADV_ENABLE); //ADV enable

    blc_ll_setScanParameter(SCAN_TYPE_PASSIVE, SCAN_INTERVAL_100MS, SCAN_WINDOW_50MS, OWN_ADDRESS_PUBLIC, SCAN_FP_ALLOW_ADV_ANY);
#if !(APP_PARSE_CHAR_ENABLE) // scanning will be enabled via console
    blc_ll_setScanEnable(BLC_SCAN_ENABLE, DUP_FILTER_DISABLE);
#endif
    rf_set_power_level_index(RF_POWER_P3dBm);

    #if (BLE_APP_PM_ENABLE)
    blc_ll_initPowerManagement_module();
    blc_pm_setSleepMask(PM_SLEEP_LEG_ADV | PM_SLEEP_LEG_SCAN | PM_SLEEP_ACL_PERIPHR | PM_SLEEP_ACL_CENTRAL);
    #endif


    tlkapi_printf(APP_CONTR_EVT_LOG_EN, "[APP][INI] feature_2m_coded_phy_test init");
#if (APP_PARSE_CHAR_ENABLE)
    app_parse_init(app_funcs, ARRAY_SIZE(app_funcs));
    app_parse_printf("feature_2M_coded_phy init\r\n");
#endif

    feature_2m_phy_test_init();
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
int main_idle_loop(void)
{
    ////////////////////////////////////// BLE entry /////////////////////////////////
    blc_sdk_main_loop();


    ////////////////////////////////////// Debug entry /////////////////////////////////
    #if (TLKAPI_DEBUG_ENABLE)
    tlkapi_debug_handler();
    #endif

    ////////////////////////////////////// UI entry /////////////////////////////////
    #if (UI_KEYBOARD_ENABLE)
    proc_keyboard(0, 0, 0);
    #endif

    #if (APP_PARSE_CHAR_ENABLE)
    app_parse_loop();
    #endif


    proc_central_role_unpair();

    #if !(APP_PARSE_CHAR_ENABLE)
    #if (FEATURE_TEST_MODE == TEST_2M_CODED_PHY_CONNECTION)
    feature_2m_phy_test_mainloop();
    #endif
    #endif

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

#endif //end of (FEATURE_TEST_MODE == ...)
