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


#if (FEATURE_TEST_MODE == TEST_EXT_ADV)



#if (UI_KEYBOARD_ENABLE)

_attribute_ble_data_retention_    int     key_not_released;


#define CONSUMER_KEY                      1
#define KEYBOARD_KEY                      2
#define PAIR_UNPAIR_KEY                  3

_attribute_ble_data_retention_    u8         key_type;

/**
 * @brief   Check changed key value.
 * @param   none.
 * @return  none.
 */
void key_change_proc(void)
{

    u8 key0 = kb_event.keycode[0];
//    u8 key_buf[8] = {0,0,0,0,0,0,0,0};

    key_not_released = 1;
    if (kb_event.cnt == 2)   //two key press
    {

    }
    else if(kb_event.cnt == 1)
    {
        if(key0 >= CR_VOL_UP )  //volume up/down
        {
            key_type = CONSUMER_KEY;
            u16 consumer_key;
            if(key0 == CR_VOL_UP){      //volume up
                consumer_key = MKEY_VOL_UP;
                tlkapi_send_string_data(APP_KEY_LOG_EN, "[UI][KEY] send Vol +", 0, 0);
            }
            else if(key0 == CR_VOL_DN){ //volume down
                consumer_key = MKEY_VOL_DN;
                tlkapi_send_string_data(APP_KEY_LOG_EN, "[UI][KEY] send Vol -", 0, 0);
            }


            /*Here is just Telink Demonstration effect. Cause the demo board has limited key to use, when Vol+/Vol- key pressed, we
            send media key "Vol+" or "Vol-" to central for all peripheral role in connection.
            For users, you should known that this is not a good method, you should manage your device and GATT data transfer
            according to  conn_dev_list[]
             * */
            for(int i=ACL_CENTRAL_MAX_NUM; i < (ACL_CENTRAL_MAX_NUM + ACL_PERIPHR_MAX_NUM); i++){ //peripheral index is from "ACL_CENTRAL_MAX_NUM" to "ACL_CENTRAL_MAX_NUM + ACL_PERIPHR_MAX_NUM - 1"
                if(conn_dev_list[i].conn_state){
                    blc_gatt_pushHandleValueNotify (conn_dev_list[i].conn_handle, HID_CONSUME_REPORT_INPUT_DP_H, (u8 *)&consumer_key, 2);
                }
            }
        }
        else{
            key_type = PAIR_UNPAIR_KEY;

            if(key0 == BTN_PAIR)   //Manual pair triggered by Key Press
            {
              extern void app_ext_adv_set_change(void);
              app_ext_adv_set_change();
            }
            else if(key0 == BTN_UNPAIR) //Manual un_pair triggered by Key Press
            {

            }

        }

    }
    else   //kb_event.cnt == 0,  key release
    {
        key_not_released = 0;
        if(key_type == CONSUMER_KEY)
        {
            u16 consumer_key = 0;
            //Here is just Telink Demonstration effect. for all peripheral in connection, send release for previous "Vol+" or "Vol-" to central
            for(int i=ACL_CENTRAL_MAX_NUM; i < (ACL_CENTRAL_MAX_NUM + ACL_PERIPHR_MAX_NUM); i++){ //peripheral index is from "ACL_CENTRAL_MAX_NUM" to "ACL_CENTRAL_MAX_NUM + ACL_PERIPHR_MAX_NUM - 1"
                if(conn_dev_list[i].conn_state){
                    blc_gatt_pushHandleValueNotify ( conn_dev_list[i].conn_handle, HID_CONSUME_REPORT_INPUT_DP_H, (u8 *)&consumer_key, 2);
                }
            }
        }
        else if(key_type == KEYBOARD_KEY)
        {

        }
        else if(key_type == PAIR_UNPAIR_KEY){

        }
    }


}



_attribute_ble_data_retention_        static u32 keyScanTick = 0;

/**
 * @brief      keyboard task handler
 * @param[in]  e    - event type
 * @param[in]  p    - Pointer point to event parameter.
 * @param[in]  n    - the length of event parameter.
 * @return     none.
 */
void proc_keyboard (u8 e, u8 *p, int n)
{
    (void)e;(void)n;(void)p;
    if(clock_time_exceed(keyScanTick, 10 * 1000)){  //keyScan interval: 10mS
        keyScanTick = clock_time();
    }
    else{
        return;
    }

    kb_event.keycode[0] = 0;
    int det_key = kb_scan_key (0, 1);

    if (det_key){
        key_change_proc();
    }
}




#endif   //end of UI_KEYBOARD_ENABLE




#endif //end of (FEATURE_TEST_MODE == ...)
