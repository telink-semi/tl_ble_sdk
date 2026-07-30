/********************************************************************************************************
 * @file    app_ui.c
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
#include "../default_att.h"
#include "app_ui.h"


#if (FEATURE_TEST_MODE == TEST_FEATURE_SCI)

/* Current single-link connection handle. */
u16 sci_conn_handle = SCI_INVALID_CONN_HANDLE;

/* RX/TX pattern table, polling index advances on each key press. */
static const u8 sci_pattern_tbl[SCI_TX_PATTERN_NUM] = SCI_TX_PATTERN_TBL;
static u8       sci_tx_pattern_idx  = 0;

/* SCI connection rate parameter sets, cycling on each rate-switch key press.
 * NOTE: connIntvl unit is 125us, supervisionTimeout unit is 10ms. */
typedef struct
{
    u16 connIntvlMin;
    u16 connIntvlMax;
    u16 subrateMin;
    u16 subrateMax;
    u16 maxLatency;
    u16 continueNum;
    u16 supervisionTimeout;
} sci_rate_param_t;

static const sci_rate_param_t sci_rate_tbl[] = {
    {10,  10,  1, 1, 0, 0, SCI_CONN_TIMEOUT}, //1.25ms, subrate 1
    {40,  40,  1, 1, 0, 0, SCI_CONN_TIMEOUT}, //5ms, subrate 1
    {20,  20,  1, 1, 0, 0, SCI_CONN_TIMEOUT}, //2.5ms, subrate 1

};
static u8 sci_rate_idx = 0;

/* Disconnect tick for auto-reconnect delay. */
static u32 sci_disconnect_tick  = 0;
static u8  sci_reconnect_pending = 0;

/* Connection setup state machine state. */
volatile sci_setup_state_t sci_setup_state = SCI_SETUP_IDLE;


#if (UI_KEYBOARD_ENABLE)
_attribute_ble_data_retention_ static u32 keyScanTick = 0;
#endif

/**
 * @brief   Trigger SCI connection rate request, cycling through preset parameter sets.
 */
void sci_trigger_conn_rate_req(void)
{
    if (sci_conn_handle == SCI_INVALID_CONN_HANDLE) {
        tlkapi_printf(APP_LOG_EN, "[SCI] ConnRateReq fail: no connection\n");
        return;
    }

    const sci_rate_param_t *p = &sci_rate_tbl[sci_rate_idx];

    ble_sts_t ret = blc_ll_handle_HostConnRateReq(sci_conn_handle,
                                                   p->connIntvlMin, p->connIntvlMax,
                                                   p->subrateMin, p->subrateMax,
                                                   p->maxLatency, p->continueNum,
                                                   p->supervisionTimeout);
    tlkapi_printf(APP_LOG_EN, "[SCI] ConnRateReq[%d] intvl=%d sub=%d ret=0x%02X\n",
                  sci_rate_idx, p->connIntvlMin, p->subrateMin, ret);

    if (ret == BLE_SUCCESS) {
        sci_rate_idx++;
        if (sci_rate_idx >= (sizeof(sci_rate_tbl) / sizeof(sci_rate_tbl[0]))) {
            sci_rate_idx = 0; //loop
        }
    }
}

/**
 * @brief   Send one test data packet, polling through pattern fill.
 */
void sci_send_test_packet(void)
{
    if (sci_conn_handle == SCI_INVALID_CONN_HANDLE) {
        tlkapi_printf(APP_LOG_EN, "[SCI] Send fail: no connection\n");
        return;
    }

    u8 buf[SCI_TX_PAYLOAD_LEN];
    u8 pattern = sci_pattern_tbl[sci_tx_pattern_idx];
    memset(buf, pattern, SCI_TX_PAYLOAD_LEN);

    ble_sts_t ret;
#if (SCI_PERIPHERAL)
    ret = blc_gatt_pushHandleValueNotify(sci_conn_handle, HID_CONSUME_REPORT_INPUT_DP_H, buf, SCI_TX_PAYLOAD_LEN);
#elif (SCI_CENTRAL)
    ret = blc_gatt_pushWriteCommand(sci_conn_handle, HID_CONSUME_REPORT_INPUT_DP_H, buf, SCI_TX_PAYLOAD_LEN);
#endif

    tlkapi_printf(APP_LOG_EN, "[SCI] Send pkt pattern=0x%02X len=%d ret=0x%02X\n",
                  pattern, SCI_TX_PAYLOAD_LEN, ret);

    if (ret == BLE_SUCCESS) {
        sci_tx_pattern_idx++;
        if (sci_tx_pattern_idx >= SCI_TX_PATTERN_NUM) {
            sci_tx_pattern_idx = 0; //loop
        }
    }
}

/**
 * @brief   Verify received test packet legitimacy and print log.
 */
void sci_verify_rx_packet(u16 connHandle, u8 *p, int len)
{
    if (len <= 0) {
        tlkapi_printf(APP_LOG_EN, "[SCI] RX invalid len=%d\n", len);
        return;
    }

    u8 pattern = p[0];
    int valid  = 1;
    int i;
    for (i = 1; i < len; i++) {
        if (p[i] != pattern) {
            valid = 0;
            break;
        }
    }

    /* Check if pattern is one of the legitimate test patterns. */
    int pattern_match = 0;
    for (i = 0; i < SCI_TX_PATTERN_NUM; i++) {
        if (pattern == sci_pattern_tbl[i]) {
            pattern_match = 1;
            break;
        }
    }

    if (valid && pattern_match) {
        tlkapi_printf(APP_LOG_EN, "[SCI] RX OK h=%04X pattern=0x%02X len=%d\n", connHandle, pattern, len);
    } else {
        tlkapi_printf(APP_LOG_EN, "[SCI] RX ERR h=%04X pattern=0x%02X len=%d match=%d\n",
                      connHandle, pattern, len, pattern_match);
    }
}

#if (UI_KEYBOARD_ENABLE)
/**
 * @brief   Check changed key value, single-shot trigger on key press.
 */
void key_change_proc(void)
{
    u8 key0 = kb_event.keycode[0];

    if (kb_event.cnt == 1) {
    if (key0 >= CR_VOL_UP){ //volume up/down
        if (key0 == CR_VOL_DN) {
            /* Key 1: trigger SCI connection rate switch. */
            tlkapi_send_string_data(APP_KEY_LOG_EN, "[UI][KEY] SCI rate switch key", 0, 0);
            sci_trigger_conn_rate_req();
        } else if (key0 == CR_VOL_UP) {
            /* Key 2: trigger test data send. */
            tlkapi_send_string_data(APP_KEY_LOG_EN, "[UI][KEY] SCI data send key", 0, 0);
            sci_send_test_packet();
        }
    }
    else{
        if (key0 == BTN_PAIR) {
            /* Key 3: trigger connection setup state machine. */
            tlkapi_send_string_data(APP_KEY_LOG_EN, "[UI][KEY] SCI setup state machine key", 0, 0);
            sci_setup_trigger();
        }
        else if (key0 == BTN_UNPAIR)
        {

        }
    }
  }
}

/**
 * @brief      keyboard task handler
 */
void proc_keyboard(u8 e, u8 *p, int n)
{
    (void)e;
    (void)p;
    (void)n;
    if (clock_time_exceed(keyScanTick, 10 * 1000)) { //keyScan interval: 10mS
        keyScanTick = clock_time();
    } else {
        return;
    }

    kb_event.keycode[0] = 0;
    int det_key = kb_scan_key(0, 1);

    if (det_key) {
        key_change_proc();
    }
}

#endif //end of UI_KEYBOARD_ENABLE


/**
 * @brief   Auto reconnect handler, called in main loop.
 */
void proc_sci_reconnect(void)
{
#if (SCI_AUTO_RECONNECT_EN)
    if (!sci_reconnect_pending) {
        return; //no pending reconnect
    }

    /* Delay before re-enabling adv/scan to avoid rapid loop. */
    if (sci_disconnect_tick && !clock_time_exceed(sci_disconnect_tick, SCI_RECONNECT_DELAY_MS * 1000)) {
        return;
    }
    sci_disconnect_tick   = 0;
    sci_reconnect_pending = 0;

#if (SCI_PERIPHERAL)
    /* Peripheral: re-enable advertising. */
    blc_ll_setAdvEnable(BLC_ADV_ENABLE);
    tlkapi_send_string_data(APP_LOG_EN, "[SCI] Peripheral re-adv start", 0, 0);
#elif (SCI_CENTRAL)
    /* Central: re-enable scanning. */
    blc_ll_setScanEnable(BLC_SCAN_ENABLE, DUP_FILTER_DISABLE);
    tlkapi_send_string_data(APP_LOG_EN, "[SCI] Central re-scan start", 0, 0);
#endif

#endif //end of SCI_AUTO_RECONNECT_EN
}

/**
 * @brief   Mark connection lost, called by disconnect event handler.
 */
void sci_mark_disconnect(void)
{
    sci_conn_handle       = SCI_INVALID_CONN_HANDLE;
    sci_disconnect_tick   = clock_time();
    sci_reconnect_pending = 1;
    sci_setup_reset();
}


/*===========================================================================*
 *          Connection Setup State Machine Implementation                    *
 *===========================================================================*/

/**
 * @brief   Trigger connection setup state machine (called by key 3 press).
 *          Starts step1: blc_hci_le_getRemoteSupportedFeatures.
 */
void sci_setup_trigger(void)
{
    if (sci_conn_handle == SCI_INVALID_CONN_HANDLE) {
        tlkapi_printf(APP_LOG_EN, "[SCI] Setup trigger fail: no connection\n");
        return;
    }

    if (sci_setup_state != SCI_SETUP_IDLE && sci_setup_state != SCI_SETUP_DONE) {
        tlkapi_printf(APP_LOG_EN, "[SCI] Setup trigger fail: busy state=%d\n", sci_setup_state);
        return;
    }

    /* Step 1: request remote supported features. */
    ble_sts_t ret = blc_hci_le_getRemoteSupportedFeatures(sci_conn_handle);
    tlkapi_printf(APP_LOG_EN, "[SCI] Setup step1 getRemoteFeature ret=0x%02X\n", ret);

    if (ret == BLE_SUCCESS) {
        sci_setup_state = SCI_SETUP_WAIT_REMOTE_FEATURE;
    }
}

/**
 * @brief   Called by HCI event handler on READ_REMOTE_FEATURES_COMPLETE.
 *          Advances state machine to step2: blc_ll_setPhy.
 */
void sci_setup_on_remote_feature_complete(void)
{
    if (sci_setup_state != SCI_SETUP_WAIT_REMOTE_FEATURE) {
        return; //not in expected state, ignore
    }

    /* Step 2: request PHY update to 2M. */
    ble_sts_t ret = blc_ll_setPhy(sci_conn_handle,
                                   SCI_SETUP_PHY_ALL_PHYS,
                                   SCI_SETUP_PHY_TX,
                                   SCI_SETUP_PHY_RX,
                                   SCI_SETUP_PHY_OPTIONS);
    tlkapi_printf(APP_LOG_EN, "[SCI] Setup step2 setPhy ret=0x%02X\n", ret);

    if (ret == BLE_SUCCESS) {
        sci_setup_state = SCI_SETUP_WAIT_PHY_UPDATE;
    } else {
        sci_setup_state = SCI_SETUP_DONE;
    }
}

/**
 * @brief   Called by HCI event handler on PHY_UPDATE_COMPLETE.
 *          Advances state machine to step3: blc_ll_frameSpaceUpdate.
 */
void sci_setup_on_phy_update_complete(void)
{
    if (sci_setup_state != SCI_SETUP_WAIT_PHY_UPDATE) {
        return; //not in expected state, ignore
    }

    /* Step 3: request frame space update. */
    ble_sts_t ret = blc_ll_frameSpaceUpdate(sci_conn_handle,
                                             SCI_SETUP_FS_MIN,
                                             SCI_SETUP_FS_MAX,
                                             SCI_SETUP_FS_PHY_MASK,
                                             SCI_SETUP_FS_SPACING_TYPE);
    tlkapi_printf(APP_LOG_EN, "[SCI] Setup step3 frameSpaceUpdate ret=0x%02X\n", ret);

    if (ret == BLE_SUCCESS) {
        sci_setup_state = SCI_SETUP_WAIT_FRAME_SPACE_UPDATE;
    } else {
        sci_setup_state = SCI_SETUP_DONE;
    }
}

/**
 * @brief   Called by HCI event handler on FRAME_SPACE_UPDATE_COMPLETE.
 *          Advances state machine to DONE.
 */
void sci_setup_on_frame_space_update_complete(void)
{
    if (sci_setup_state != SCI_SETUP_WAIT_FRAME_SPACE_UPDATE) {
        return; //not in expected state, ignore
    }

    sci_setup_state = SCI_SETUP_DONE;
    tlkapi_printf(APP_LOG_EN, "[SCI] Setup all steps done\n");
}

/**
 * @brief   Reset state machine to IDLE. Called on disconnect.
 */
void sci_setup_reset(void)
{
    sci_setup_state = SCI_SETUP_IDLE;
}


#endif //end of (FEATURE_TEST_MODE == ...)
