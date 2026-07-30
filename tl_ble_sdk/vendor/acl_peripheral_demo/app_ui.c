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

#include "app.h"
#include "app_att.h"
#include "app_ui.h"

#if defined(TLK_ONLY_BLE_HOST)
#include "stack/pm/pm_sys.h"
#endif

#if (UI_KEYBOARD_ENABLE)

_attribute_ble_data_retention_ int key_not_released;


    #define CONSUMER_KEY    1
    #define KEYBOARD_KEY    2
    #define PAIR_UNPAIR_KEY 3

_attribute_ble_data_retention_ u8 key_type;

/**
 * @brief   Check changed key value.
 * @param   none.
 * @return  none.
 */
void key_change_proc(void)
{
    u8 key0 = kb_event.keycode[0];
    //  u8 key_buf[8] = {0,0,0,0,0,0,0,0};

    key_not_released = 1;
    if (kb_event.cnt == 1) {
        if (key0 >= CR_VOL_UP) //volume up/down
        {
            key_type = CONSUMER_KEY;
            u16 consumer_key;
            if (key0 == CR_VOL_UP) {        //volume up
                consumer_key = MKEY_VOL_UP;
                tlkapi_send_string_data(APP_KEY_LOG_EN, "[UI][KEY] send Vol +", 0, 0);
            } else if (key0 == CR_VOL_DN) { //volume down
                consumer_key = MKEY_VOL_DN;
                tlkapi_send_string_data(APP_KEY_LOG_EN, "[UI][KEY] send Vol -", 0, 0);
            }


            /*Here is just Telink Demonstration effect. Cause the demo board has limited key to use, when Vol+/Vol- key pressed, we
            send media key "Vol+" or "Vol-" to central for all peripheral role in connection.
            For users, you should known that this is not a good method, you should manage your device and GATT data transfer
            according to  conn_dev_list[]
             * */
            for (int i = ACL_CENTRAL_MAX_NUM; i < (ACL_CENTRAL_MAX_NUM + ACL_PERIPHR_MAX_NUM); i++) { //peripheral index is from "ACL_CENTRAL_MAX_NUM" to "ACL_CENTRAL_MAX_NUM + ACL_PERIPHR_MAX_NUM - 1"
                if (conn_dev_list[i].conn_state) {
                    blc_gatt_pushHandleValueNotify(conn_dev_list[i].conn_handle, HID_CONSUME_REPORT_INPUT_DP_H, (u8 *)&consumer_key, 2);
                }
            }
        } else {
            key_type = PAIR_UNPAIR_KEY;

    #if 0
            if (key0 == BTN_PAIR) {
                static _attribute_ble_data_retention_ u8 adv_status = 0;
                blc_ll_setAdvEnable(adv_status);
                adv_status = !adv_status;
            }
            else if (key0 == BTN_UNPAIR) {
                for (int i = ACL_CENTRAL_MAX_NUM; i < (ACL_CENTRAL_MAX_NUM + ACL_PERIPHR_MAX_NUM); i++) { //peripheral index is from "ACL_CENTRAL_MAX_NUM" to "ACL_CENTRAL_MAX_NUM + ACL_PERIPHR_MAX_NUM - 1"
                    if (conn_dev_list[i].conn_state) {
                        static _attribute_ble_data_retention_ u16 latency = 0;
                        bls_l2cap_requestConnParamUpdate(conn_dev_list[i].conn_handle, CONN_INTERVAL_15MS, CONN_INTERVAL_15MS, latency, CONN_TIMEOUT_4S);
                        if (latency) {
                            latency = 0;
                        }
                        else {
                            latency = 99;
                        }
                    }
                }
            }
    #endif
        }

    } else //kb_event.cnt == 0,  key release
    {
        key_not_released = 0;
        if (key_type == CONSUMER_KEY) {
            u16 consumer_key = 0;
            //Here is just Telink Demonstration effect. for all peripheral in connection, send release for previous "Vol+" or "Vol-" to central
            for (int i = ACL_CENTRAL_MAX_NUM; i < (ACL_CENTRAL_MAX_NUM + ACL_PERIPHR_MAX_NUM); i++) { //peripheral index is from "ACL_CENTRAL_MAX_NUM" to "ACL_CENTRAL_MAX_NUM + ACL_PERIPHR_MAX_NUM - 1"
                if (conn_dev_list[i].conn_state) {
                    ble_sts_t value = blc_gatt_pushHandleValueNotify(conn_dev_list[i].conn_handle, HID_CONSUME_REPORT_INPUT_DP_H, (u8 *)&consumer_key, 2);
                    if(value != BLE_SUCCESS)
                    {
                        tlkapi_printf(APP_KEY_LOG_EN, "[UI][KEY] blc_gatt_pushHandleValueNotify fail %X",value);
                    }


                }
            }
        } else if (key_type == KEYBOARD_KEY) {
        } else if (key_type == PAIR_UNPAIR_KEY) {
        }
    }
}

    #define GPIO_WAKEUP_KEYPROC_CNT 3
_attribute_ble_data_retention_ static u32 keyScanTick            = 0;
_attribute_ble_data_retention_ static int gpioWakeup_keyProc_cnt = 0;

/**
 * @brief      keyboard task handler
 * @param[in]  e    - event type
 * @param[in]  p    - Pointer point to event parameter.
 * @param[in]  n    - the length of event parameter.
 * @return     none.
 */
void proc_keyboard(u8 e, u8 *p, int n)
{
    (void)n;
    (void)p;
    //when key press GPIO wake_up sleep, process key_scan at least GPIO_WAKEUP_KEYPROC_CNT times
#if BLE_APP_PM_ENABLE
    if (e == BLT_EV_FLAG_GPIO_EARLY_WAKEUP) {
        gpioWakeup_keyProc_cnt = GPIO_WAKEUP_KEYPROC_CNT;
    } else if (gpioWakeup_keyProc_cnt) {
        gpioWakeup_keyProc_cnt--;
    }
#endif
    if (gpioWakeup_keyProc_cnt || clock_time_exceed(keyScanTick, 10 * 1000)) { //keyScan interval: 10mS
        keyScanTick = clock_time();
    } else {
        return;
    }

    kb_event.keycode[0] = 0;
    int det_key         = kb_scan_key(0, 1);

    if (det_key) {
        key_change_proc();
    }
}

/**
 * @brief      callback function of LinkLayer Event "BLT_EV_FLAG_SUSPEND_ENTER"
 * @param[in]  e - LinkLayer Event type
 * @param[in]  p - data pointer of event
 * @param[in]  n - data length of event
 * @return     none
 */
_attribute_ram_code_ void app_set_kb_wakeup(u8 e, u8 *p, int n)
{
    (void)e;
    (void)p;
    (void)n;
    #if (BLE_APP_PM_ENABLE)
    /* suspend time > 50ms.add GPIO wake_up */
    if (((u32)(blc_pm_getWakeupSystemTick() - clock_time())) > 100 * SYSTEM_TIMER_TICK_1MS) {
        blc_pm_setWakeupSource(PM_WAKEUP_PAD); //GPIO PAD wake_up
    }
    #endif
}

/**
 * @brief      keyboard initialization
 * @param[in]  none
 * @return     none.
 */
void keyboard_init(void)
{
    #if (BLE_APP_PM_ENABLE)
    u32 pin[] = KB_DRIVE_PINS;
    /////////// keyboard GPIO wakeup init ////////
    for (unsigned int i = 0; i < (sizeof(pin) / sizeof(*pin)); i++) {
        pm_set_gpio_wakeup(pin[i], WAKEUP_LEVEL_HIGH, 1); //drive pin pad high level wakeup deepsleep
    }
    blc_ll_registerTelinkControllerEventCallback(BLT_EV_FLAG_SLEEP_ENTER, &app_set_kb_wakeup);
    blc_ll_registerTelinkControllerEventCallback(BLT_EV_FLAG_GPIO_EARLY_WAKEUP, &proc_keyboard);
    #endif
}


#endif //end of UI_KEYBOARD_ENABLE


#if (UI_BUTTON_ENABLE)

/////////////////////////////////////////////////////////////////////
    #define MAX_BTN_SIZE    2
    #define BTN_VALID_LEVEL 0
    #define BTN_VOL_UP        0x01
    #define BTN_VOL_DOWN      0x02

u32 ctrl_btn[]            = {GPIO_PE3, GPIO_PE5};
u8  btn_map[MAX_BTN_SIZE] = {BTN_VOL_UP, BTN_VOL_DOWN};
_attribute_ble_data_retention_ static u32 button_det_tick;
/**
     * @brief   record the result of key detect
     */
typedef struct
{
    u8 cnt;                   //count button num
    u8 btn_press;
    u8 keycode[MAX_BTN_SIZE]; //6 btn
} vc_data_t;

vc_data_t vc_event;

/**
     * @brief   record the status of button process
     */
typedef struct
{
    u8 btn_history[4]; //vc history btn save
    u8 btn_filter_last;
    u8 btn_not_release;
    u8 btn_new;        //new btn  flag
} btn_status_t;

_attribute_ble_data_retention_ btn_status_t btn_status;

/**
     * @brief      Debounce processing during button detection
     * @param[in]  btn_v - vc_event.btn_press
     * @return     1:Detect new button;0:Button isn't changed
     */
u8 btn_debounce_filter(u8 *btn_v)
{
    u8 change = 0;

    for (int i = 3; i > 0; i--) {
        btn_status.btn_history[i] = btn_status.btn_history[i - 1];
    }
    btn_status.btn_history[0] = *btn_v;

    if (btn_status.btn_history[0] == btn_status.btn_history[1] && btn_status.btn_history[1] == btn_status.btn_history[2] &&
        btn_status.btn_history[0] != btn_status.btn_filter_last) {
        change = 1;

        btn_status.btn_filter_last = btn_status.btn_history[0];
    }

    return change;
}

/**
     * @brief      This function is key detection processing
     * @param[in]  read_key - Decide whether to return the key detection result
     * @return     1:Detect new button;0:Button isn't changed
     */
u8 vc_detect_button(int read_key)
{
    u8 btn_changed, i;
    memset(&vc_event, 0, sizeof(vc_data_t)); //clear vc_event
    //vc_event.btn_press = 0;

    for (i = 0; i < MAX_BTN_SIZE; i++) {
        if (BTN_VALID_LEVEL != !gpio_read(ctrl_btn[i])) {
            vc_event.btn_press |= BIT(i);
        }
    }

    btn_changed = btn_debounce_filter(&vc_event.btn_press);


    if (btn_changed && read_key) {
        for (i = 0; i < MAX_BTN_SIZE; i++) {
            if (vc_event.btn_press & BIT(i)) {
                vc_event.keycode[vc_event.cnt++] = btn_map[i];
            }
        }

        return 1;
    }

    return 0;
}

/**
     * @brief       this function is used to detect if button pressed or released.
     * @param[in]   none
     * @return      none
     */
void proc_button(void)
{

    if (clock_time_exceed(button_det_tick, 10 * 1000)) {
        button_det_tick = clock_time();
    } else {
        return;
    }

    int det_key = vc_detect_button(1);

    if (det_key)                      //key change: press or release
    {
        u8 key0 = vc_event.keycode[0];
        if (vc_event.cnt == 2)        //two key press
        {
        } else if (vc_event.cnt == 1) //one key press
        {
            u16 consumer_key;
            if (key0 == BTN_VOL_UP) {
                consumer_key = MKEY_VOL_UP;
                tlkapi_send_string_data(APP_BUTTON_LOG_EN, "[UI][BUTTON] Vol +", 0, 0);
            } else if (key0 == BTN_VOL_DOWN) {
                consumer_key = MKEY_VOL_DN;
                tlkapi_send_string_data(APP_BUTTON_LOG_EN, "[UI][BUTTON] Vol-", 0, 0);
            }

            /*Here is just Telink Demonstration effect. Cause the demo board has limited key to use, when Vol+/Vol- key pressed, we
            send media key "Vol+" or "Vol-" to central for all peripheral role in connection.
            For users, you should known that this is not a good method, you should manage your device and GATT data transfer
            according to  conn_dev_list[]
             * */
            for (int i = ACL_CENTRAL_MAX_NUM; i < (ACL_CENTRAL_MAX_NUM + ACL_PERIPHR_MAX_NUM); i++) { //peripheral index is from "ACL_CENTRAL_MAX_NUM" to "ACL_CENTRAL_MAX_NUM + ACL_PERIPHR_MAX_NUM - 1"
                if (conn_dev_list[i].conn_state) {
                    blc_gatt_pushHandleValueNotify(conn_dev_list[i].conn_handle, HID_CONSUME_REPORT_INPUT_DP_H, (u8 *)&consumer_key, 2);
                }
            }
        } else { //release
            u16 consumer_key = 0;
            //Here is just Telink Demonstration effect. for all peripheral in connection, send release for previous "Vol+" or "Vol-" to central
            for (int i = ACL_CENTRAL_MAX_NUM; i < (ACL_CENTRAL_MAX_NUM + ACL_PERIPHR_MAX_NUM); i++) { //peripheral index is from "ACL_CENTRAL_MAX_NUM" to "ACL_CENTRAL_MAX_NUM + ACL_PERIPHR_MAX_NUM - 1"
                if (conn_dev_list[i].conn_state) {
                    blc_gatt_pushHandleValueNotify(conn_dev_list[i].conn_handle, HID_CONSUME_REPORT_INPUT_DP_H, (u8 *)&consumer_key, 2);
                }
            }
        }
    }
}
#endif //end of UI_BUTTON_ENABLE
