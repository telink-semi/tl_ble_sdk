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


#if (FEATURE_TEST_MODE == TEST_FEATURE_SCI)


/**
 * @brief   BLE Advertising data, contains SCI device name for central scan filter.
 */
const u8 tbl_advData[] = {
    SCI_DEVICE_NAME_LEN + 1,
    DT_COMPLETE_LOCAL_NAME,
    's', 'c', 'i', '_', 't', 'e', 's', 't',
    2,
    DT_FLAGS,
    0x05, //BLE general discoverable mode, BR/EDR not supported
    3,
    DT_APPEARANCE,
    0x80,
    0x01, //384, Generic Remote Control
};

/**
 * @brief   BLE Scan Response Packet data
 */
const u8 tbl_scanRsp[] = {
    SCI_DEVICE_NAME_LEN + 1,
    DT_COMPLETE_LOCAL_NAME,
    's', 'c', 'i', '_', 't', 'e', 's', 't',
};


/**
 * @brief      Parse ADV data and match SCI device name.
 * @param[in]  data   pointer to ADV data payload.
 * @param[in]  len    ADV data length.
 * @return     1: name matched, 0: not matched.
 */
static int sci_adv_name_match(u8 *data, u8 len)
{
    u8 i = 0;
    while (i < len) {
        u8 ad_len = data[i];
        if (ad_len == 0 || (i + ad_len) >= len) {
            break;
        }
        u8 ad_type = data[i + 1];
        if (ad_type == DT_COMPLETE_LOCAL_NAME) {
            if ((ad_len - 1) == SCI_DEVICE_NAME_LEN &&
                !memcmp(&data[i + 2], SCI_DEVICE_NAME, SCI_DEVICE_NAME_LEN)) {
                return 1;
            }
        }
        i += ad_len + 1;
    }
    return 0;
}

/**
 * @brief      BLE Adv report event handler (Central role).
 *             Filter by SCI device name and auto create connection.
 * @param[in]  p   Pointer point to event_adv_report_t.
 * @return     0.
 */
int app_le_adv_report_event_handle(u8 *p)
{
#if (SCI_CENTRAL)
    event_adv_report_t *pa = (event_adv_report_t *)p;

    /* Only maintain 1 connection, skip if already connected. */
    if (sci_conn_handle != SCI_INVALID_CONN_HANDLE) {
        return 1;
    }

    if (sci_adv_name_match(pa->data, pa->len)) {
        tlkapi_printf(APP_LOG_EN, "[SCI] ADV match, create conn mac:%02X%02X%02X%02X%02X%02X\n",
                      pa->mac[0], pa->mac[1], pa->mac[2], pa->mac[3], pa->mac[4], pa->mac[5]);

        u8 status = blc_ll_createConnection(SCAN_INTERVAL_100MS, SCAN_WINDOW_100MS,
                                            INITIATE_FP_ADV_SPECIFY, pa->adr_type, pa->mac,
                                            OWN_ADDRESS_PUBLIC,
                                            CONN_INTERVAL_10MS, CONN_INTERVAL_10MS,
                                            0, CONN_TIMEOUT_4S, 0, 0xFFFF);
        if (status != BLE_SUCCESS) {
            tlkapi_printf(APP_LOG_EN, "[SCI] createConnection err:0x%02X\n", status);
        }
    }
#else
    (void)p;
#endif
    return 0;
}

/**
 * @brief      BLE Connection complete event handler.
 *             Record the single-link connection handle.
 * @param[in]  p   Pointer point to hci_le_connectionCompleteEvt_t.
 * @return     0.
 */
int app_le_connection_complete_event_handle(u8 *p)
{
    hci_le_connectionCompleteEvt_t *pConnEvt = (hci_le_connectionCompleteEvt_t *)p;
    tlkapi_printf(APP_CONTR_EVT_LOG_EN, "[SCI] conn complete h:%04X role:%d\n",
                  pConnEvt->connHandle, pConnEvt->role);

    if (pConnEvt->status == BLE_SUCCESS) {
        dev_char_info_insert_by_conn_event(pConnEvt);
        sci_conn_handle = pConnEvt->connHandle;

#if (SCI_CENTRAL)
        /* Central stops scanning after connection established. */
        blc_ll_setScanEnable(BLC_SCAN_DISABLE, DUP_FILTER_DISABLE);
#endif
        /* Remote feature read is triggered by key 3 state machine. */
    }
    return 0;
}

/**
 * @brief      BLE Disconnection event handler.
 *             Mark connection lost and trigger auto reconnect.
 * @param[in]  p   Pointer point to hci_disconnectionCompleteEvt_t.
 * @return     0.
 */
int app_disconnect_event_handle(u8 *p)
{
    hci_disconnectionCompleteEvt_t *pDisConn = (hci_disconnectionCompleteEvt_t *)p;
    tlkapi_printf(APP_CONTR_EVT_LOG_EN, "[SCI] disconnect h:%04X reason:0x%02X\n",
                  pDisConn->connHandle, pDisConn->reason);

    dev_char_info_delete_by_connhandle(pDisConn->connHandle);
    sci_mark_disconnect();
    return 0;
}

/**
 * @brief      BLE Connection update complete event handler.
 * @param[in]  p   Pointer point to hci_le_connectionUpdateCompleteEvt_t.
 * @return     0.
 */
int app_le_connection_update_complete_event_handle(u8 *p)
{
    hci_le_connectionUpdateCompleteEvt_t *pUpt = (hci_le_connectionUpdateCompleteEvt_t *)p;
    tlkapi_send_string_data(APP_CONTR_EVT_LOG_EN, "[SCI] Conn Update Event", &pUpt->connHandle, 8);
    return 0;
}

//////////////////////////////////////////////////////////
// event call back
//////////////////////////////////////////////////////////
/**
 * @brief      BLE controller event handler call-back.
 * @param[in]  h   event type.
 * @param[in]  p   Pointer point to event parameter buffer.
 * @param[in]  n   the length of event parameter.
 * @return     0.
 */
int app_controller_event_callback(u32 h, u8 *p, int n)
{
    (void)n;
    if (h & HCI_FLAG_EVENT_BT_STD) {
        u8 evtCode = h & 0xff;

        if (evtCode == HCI_EVT_DISCONNECTION_COMPLETE) {
            app_disconnect_event_handle(p);
        } else if (evtCode == HCI_EVT_LE_META) {
            u8 subEvt_code = p[0];

            if (subEvt_code == HCI_SUB_EVT_LE_CONNECTION_COMPLETE) {
                app_le_connection_complete_event_handle(p);
            } else if (subEvt_code == HCI_SUB_EVT_LE_ADVERTISING_REPORT) {
                app_le_adv_report_event_handle(p);
            } else if (subEvt_code == HCI_SUB_EVT_LE_CONNECTION_UPDATE_COMPLETE) {
                app_le_connection_update_complete_event_handle(p);
            } else if (subEvt_code == HCI_SUB_EVT_LE_READ_REMOTE_USED_FEATURES_COMPLETE) {
                /* State machine step1 complete -> advance to step2 (setPhy). */
                sci_setup_on_remote_feature_complete();
            } else if (subEvt_code == HCI_SUB_EVT_LE_PHY_UPDATE_COMPLETE) {
                /* State machine step2 complete -> advance to step3 (frameSpaceUpdate). */
                sci_setup_on_phy_update_complete();
            } else if (subEvt_code == HCI_SUB_EVT_LE_FRAME_SPACE_UPDATE_COMPLETE) {
                /* State machine step3 complete -> DONE. */
                sci_setup_on_frame_space_update_complete();
            }
        }
    }
    return 0;
}

/**
 * @brief      BLE host event handler call-back.
 * @param[in]  h     event type.
 * @param[in]  para  Pointer point to event parameter buffer.
 * @param[in]  n     the length of event parameter.
 * @return     0.
 */
int app_host_event_callback(u32 h, u8 *para, int n)
{
    (void)h;
    (void)para;
    (void)n;
    return 0;
}

/**
 * @brief      BLE GATT data handler call-back.
 *             Verify received test packet from peer device.
 * @param[in]  connHandle  connection handle.
 * @param[in]  pkt         Pointer point to data packet buffer.
 * @return     0.
 */
int app_gatt_data_handler(u16 connHandle, u8 *pkt)
{
    rf_packet_att_t *pAtt = (rf_packet_att_t *)pkt;

    /* Peripheral receives Write Command from Central. */
    if (pAtt->opcode == ATT_OP_WRITE_CMD) {
        if (pAtt->handle == HID_CONSUME_REPORT_INPUT_DP_H) {
            int payload_len = pAtt->l2capLen - 3; //l2capLen = opcode(1) + handle(2) + value
            if (payload_len > 0) {
                sci_verify_rx_packet(connHandle, pAtt->dat, payload_len);
            }
        }
    }
    /* Central receives Notify from Peripheral. */
    else if (pAtt->opcode == ATT_OP_HANDLE_VALUE_NOTI) {
        if (pAtt->handle == HID_CONSUME_REPORT_INPUT_DP_H) {
            int payload_len = pAtt->l2capLen - 3;
            if (payload_len > 0) {
                sci_verify_rx_packet(connHandle, pAtt->dat, payload_len);
            }
        }
    }

    /* Respond to ATT requests that need response (peripheral server side). */
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
            break;
        default:
            break;
        }
    }

    return 0;
}

///////////////////////////////////////////

/**
 * @brief       user initialization when MCU power on or wake_up from deepSleep mode.
 * @param[in]   none.
 * @return      none.
 */
_attribute_no_inline_ void user_init_normal(void)
{
    //////////////////////////// basic hardware Initialization Begin /////////////////////////////////
    random_generator_init();

#if (TLKAPI_DEBUG_ENABLE)
    tlkapi_debug_init();
    blc_debug_enableStackLog(STK_LOG_LL_RX);
#endif

    blc_readFlashSize_autoConfigCustomFlashSector();
    blc_app_loadCustomizedParameters_normal();
    //////////////////////////// basic hardware Initialization End //////////////////////////////////


    //////////////////////////// BLE stack Initialization Begin ////////////////////////////////////////
    u8 mac_public[6];
    u8 mac_random_static[6];
    blc_initMacAddress(flash_sector_mac_address, mac_public, mac_random_static);

#if defined(TLK_ONLY_BLE_HOST)
    sys_n22_start();
    delay_ms(300);
#endif

    //////////// LinkLayer Initialization Begin /////////////////////////
    blc_ll_initBasicMCU();
    blc_ll_initStandby_module(mac_public);

    /* Role-specific LL module init: only init the module that has a non-zero connection slot.
     * Peripheral needs advertising; Central needs scanning + initiating. */
#if (SCI_PERIPHERAL)
    blc_ll_initLegacyAdvertising_module();
    blc_ll_initAclPeriphrRole_module();
#elif (SCI_CENTRAL)
    blc_ll_initLegacyScanning_module();
    blc_ll_initLegacyInitiating_module();
    blc_ll_initAclCentralRole_module();
#endif
    blc_ll_initAclConnection_module();

    blc_ll_setMaxConnectionNumber(ACL_CENTRAL_MAX_NUM, ACL_PERIPHR_MAX_NUM);
    blc_ll_setAclConnMaxOctetsNumber(ACL_CONN_MAX_RX_OCTETS, ACL_CENTRAL_MAX_TX_OCTETS, ACL_PERIPHR_MAX_TX_OCTETS);

    blc_ll_initAclConnRxFifo(app_acl_rx_fifo, ACL_RX_FIFO_SIZE, ACL_RX_FIFO_NUM);
#if (ACL_CENTRAL_MAX_NUM > 0)
    blc_ll_initAclCentralTxFifo(app_acl_cen_tx_fifo, ACL_CENTRAL_TX_FIFO_SIZE, ACL_CENTRAL_TX_FIFO_NUM, ACL_CENTRAL_MAX_NUM);
#endif
#if (ACL_PERIPHR_MAX_NUM > 0)
    blc_ll_initAclPeriphrTxFifo(app_acl_per_tx_fifo, ACL_PERIPHR_TX_FIFO_SIZE, ACL_PERIPHR_TX_FIFO_NUM, ACL_PERIPHR_MAX_NUM);
#endif

#if (SCI_CENTRAL)
    blc_ll_setAclCentralBaseConnectionInterval(CONN_INTERVAL_1P25MS);
#endif

    blc_ll_init2MPhyCodedPhy_feature();

    blc_ll_sci_init_common();

    /* Per-connection FSU parameter buffer, bound (not copied) into AA_blms.fsu_param. */
    static fsu_hci_param_t sFsuParamBuf[8];  //ll config
    blc_ll_initFrameSpaceUpdate_feature(sFsuParamBuf, sizeof(sFsuParamBuf));
    //////////// LinkLayer Initialization End /////////////////////////


    //////////// HCI Initialization Begin /////////////////////////
    blc_hci_registerControllerDataHandler(blc_l2cap_pktHandler);
    blc_hci_registerControllerEventHandler(app_controller_event_callback);

    blc_hci_setEventMask_cmd(HCI_EVT_MASK_DISCONNECTION_COMPLETE);
#if (SCI_PERIPHERAL)
    blc_hci_le_setEventMask_cmd(HCI_LE_EVT_MASK_CONNECTION_COMPLETE | HCI_LE_EVT_MASK_CONNECTION_UPDATE_COMPLETE |
                                HCI_LE_EVT_MASK_READ_REMOTE_FEATURES_COMPLETE | HCI_LE_EVT_MASK_PHY_UPDATE_COMPLETE);
#elif (SCI_CENTRAL)
    blc_hci_le_setEventMask_cmd(HCI_LE_EVT_MASK_CONNECTION_COMPLETE | HCI_LE_EVT_MASK_ADVERTISING_REPORT |
                                HCI_LE_EVT_MASK_CONNECTION_UPDATE_COMPLETE |
                                HCI_LE_EVT_MASK_READ_REMOTE_FEATURES_COMPLETE | HCI_LE_EVT_MASK_PHY_UPDATE_COMPLETE);
#endif
    /* Enable LE event page 2 for FRAME_SPACE_UPDATE_COMPLETE event. */
    blc_hci_le_setEventMask_2_cmd(HCI_LE_EVT_MASK_2_FRAME_SPACE_UPDATE_COMPLETE);

    u32 error_code = blc_contr_checkControllerInitialization();
    if (error_code != INIT_SUCCESS) {
#if (TLKAPI_DEBUG_ENABLE)
        tlkapi_send_string_data(APP_LOG_EN, "[SCI][INI] Controller INIT ERROR", &error_code, 1);
        while (1) {
            tlkapi_debug_handler();
        }
#else
        while (1)
            ;
#endif
    }
    //////////// HCI Initialization End /////////////////////////


    //////////// Host Initialization Begin /////////////////////////
    blc_gap_init();

#if (ACL_CENTRAL_MAX_NUM > 0)
    blc_l2cap_initAclCentralBuffer(app_cen_l2cap_rx_buf, CENTRAL_L2CAP_BUFF_SIZE, NULL, 0);
    blc_att_setCentralRxMtuSize(CENTRAL_ATT_RX_MTU);
#endif
#if (ACL_PERIPHR_MAX_NUM > 0)
    blc_l2cap_initAclPeripheralBuffer(app_per_l2cap_rx_buf, PERIPHR_L2CAP_BUFF_SIZE, app_per_l2cap_tx_buf, PERIPHR_L2CAP_BUFF_SIZE);
    blc_att_setPeripheralRxMtuSize(PERIPHR_ATT_RX_MTU);
#endif

    my_gatt_init();
    blc_gatt_register_data_handler(app_gatt_data_handler);

    blc_smp_setSecurityLevel_periphr(No_Security);
    blc_smp_setSecurityLevel_central(No_Security);
    blc_smp_smpParamInit();

    blc_gap_registerHostEventHandler(app_host_event_callback);
    blc_gap_setEventMask(0);
    //////////// Host Initialization End /////////////////////////
    //////////////////////////// BLE stack Initialization End ////////////////////////////////////////


    //////////////////////////// User Configuration for BLE application ////////////////////////////
#if (SCI_PERIPHERAL)
    blc_ll_setAdvData(tbl_advData, sizeof(tbl_advData));
    blc_ll_setScanRspData(tbl_scanRsp, sizeof(tbl_scanRsp));
    blc_ll_setAdvParam(ADV_INTERVAL_50MS, ADV_INTERVAL_50MS, ADV_TYPE_CONNECTABLE_UNDIRECTED, OWN_ADDRESS_PUBLIC, 0, NULL, BLT_ENABLE_ADV_ALL, ADV_FP_NONE);
#elif (SCI_CENTRAL)
    blc_ll_setScanParameter(SCAN_TYPE_PASSIVE, SCAN_INTERVAL_100MS, SCAN_WINDOW_50MS, OWN_ADDRESS_PUBLIC, SCAN_FP_ALLOW_ADV_ANY);
#endif

    rf_set_power_level_index(RF_POWER_P3dBm);

#if (BLE_APP_PM_ENABLE)
    blc_ll_initPowerManagement_module();
    blc_pm_setSleepMask(PM_SLEEP_LEG_ADV | PM_SLEEP_LEG_SCAN | PM_SLEEP_ACL_PERIPHR | PM_SLEEP_ACL_CENTRAL);
#endif

    /* Enable role-specific advertising or scanning on power up. */
#if (SCI_PERIPHERAL)
    blc_ll_setAdvEnable(BLC_ADV_ENABLE);
    tlkapi_send_string_data(APP_LOG_EN, "[SCI] Peripheral init, adv start", 0, 0);
#elif (SCI_CENTRAL)
    blc_ll_setScanEnable(BLC_SCAN_ENABLE, DUP_FILTER_DISABLE);
    tlkapi_send_string_data(APP_LOG_EN, "[SCI] Central init, scan start", 0, 0);
#endif
    ////////////////////////////////////////////////////////////////////////////////////////////////
}

/**
 * @brief       user initialization when MCU wake_up from deepSleep_retention mode.
 * @param[in]   none.
 * @return      none.
 */
void user_init_deepRetn(void)
{
}

/////////////////////////////////////////////////////////////////////
// main loop flow
/////////////////////////////////////////////////////////////////////

/**
 * @brief     BLE main idle loop.
 * @param[in]  none.
 * @return     0.
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

    ////////////////////////////////////// SCI reconnect entry /////////////////////////////////
    proc_sci_reconnect();

    return 0;
}

/**
 * @brief     BLE main loop.
 * @param[in]  none.
 * @return     none.
 */
_attribute_no_inline_ void main_loop(void)
{
    main_idle_loop();
}

#endif //end of (FEATURE_TEST_MODE == ...)
