/********************************************************************************************************
 * @file    app_ui.h
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
#ifndef APP_UI_H_
#define APP_UI_H_

#include "app_config.h"

#if (FEATURE_TEST_MODE == TEST_FEATURE_SCI)

/* Invalid connection handle value, indicates no active connection. */
#define SCI_INVALID_CONN_HANDLE  0xFFFF

/* Current connection handle of the single ACL link, INVALID_CONN_IDX means no connection. */
extern u16 sci_conn_handle;


/**
 * @brief      keyboard task handler
 * @param[in]  e    - event type
 * @param[in]  p    - Pointer point to event parameter.
 * @param[in]  n    - the length of event parameter.
 * @return     none.
 */
void proc_keyboard(u8 e, u8 *p, int n);


/**
 * @brief   Trigger SCI connection rate request, cycling through preset parameter sets.
 *          Called by key press, sends LL_CONNECTION_RATE_REQ via host API.
 * @param   none.
 * @return  none.
 */
void sci_trigger_conn_rate_req(void);


/**
 * @brief   Send one test data packet, polling through 0x11/0x22/0x33/0x44 pattern fill.
 *          Peripheral sends Notify, Central sends Write Command.
 * @param   none.
 * @return  none.
 */
void sci_send_test_packet(void);


/**
 * @brief   Verify received test packet legitimacy and print log.
 * @param   connHandle  connection handle.
 * @param   p           pointer to payload data.
 * @param   len         payload length.
 * @return  none.
 */
void sci_verify_rx_packet(u16 connHandle, u8 *p, int len);


/**
 * @brief   Auto reconnect handler, called in main loop.
 *          Peripheral re-enables advertising, Central re-enables scanning after disconnect.
 * @param   none.
 * @return  none.
 */
void proc_sci_reconnect(void);


/**
 * @brief   Mark connection lost and record disconnect tick for reconnect delay.
 *          Called by disconnect event handler.
 * @param   none.
 * @return  none.
 */
void sci_mark_disconnect(void);


/*===========================================================================*
 *          Connection Setup State Machine (key 3 trigger)                   *
 *===========================================================================*
 * Sequence after key press (requires active connection):
 *   1. blc_hci_le_getRemoteSupportedFeatures
 *   2. blc_ll_setPhy
 *   3. blc_ll_frameSpaceUpdate
 * Each step advances on its HCI complete event.
 *===========================================================================*/

/* State machine states. */
typedef enum {
    SCI_SETUP_IDLE = 0,                 //not started
    SCI_SETUP_WAIT_REMOTE_FEATURE,      //step1: waiting for READ_REMOTE_FEATURES_COMPLETE
    SCI_SETUP_WAIT_PHY_UPDATE,          //step2: waiting for PHY_UPDATE_COMPLETE
    SCI_SETUP_WAIT_FRAME_SPACE_UPDATE,  //step3: waiting for FRAME_SPACE_UPDATE_COMPLETE
    SCI_SETUP_DONE,                     //all steps complete
} sci_setup_state_t;

/* Current state machine state. */
extern volatile sci_setup_state_t sci_setup_state;

/**
 * @brief   Trigger connection setup state machine (called by key 3 press).
 *          Starts step1: getRemoteSupportedFeatures.
 * @return  none.
 */
void sci_setup_trigger(void);

/**
 * @brief   Called by HCI event handler on READ_REMOTE_FEATURES_COMPLETE.
 *          Advances state machine to step2: setPhy.
 * @return  none.
 */
void sci_setup_on_remote_feature_complete(void);

/**
 * @brief   Called by HCI event handler on PHY_UPDATE_COMPLETE.
 *          Advances state machine to step3: frameSpaceUpdate.
 * @return  none.
 */
void sci_setup_on_phy_update_complete(void);

/**
 * @brief   Called by HCI event handler on FRAME_SPACE_UPDATE_COMPLETE.
 *          Advances state machine to DONE.
 * @return  none.
 */
void sci_setup_on_frame_space_update_complete(void);

/**
 * @brief   Reset state machine to IDLE. Called on disconnect.
 * @return  none.
 */
void sci_setup_reset(void);


#endif //end of (FEATURE_TEST_MODE == ...)

#endif /* APP_UI_H_ */
