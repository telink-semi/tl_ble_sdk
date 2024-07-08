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
#include "app_l2cap_coc.h"


#if (FEATURE_TEST_MODE == TEST_L2CAP_COC)

int    central_pairing_enable = 0;

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
        if(key0 >= CR_VOL_UP )
        {
            key_type = CONSUMER_KEY;

            if(key0 == CR_VOL_UP){
                app_createLeCreditBasedConnect();
//                app_createCreditBasedConnect();
            }
            else if(key0 == CR_VOL_DN){
                app_sendCocData();
            }
        }
        else{
            key_type = PAIR_UNPAIR_KEY;

            if(key0 == BTN_PAIR)   //Manual pair triggered by Key Press
            {
                central_pairing_enable = 1;
                tlkapi_printf(APP_PAIR_LOG_EN, "[UI][PAIR] Pair begin...\r\n");

            }
            else if(key0 == BTN_UNPAIR) //Manual un_pair triggered by Key Press
            {
                app_disconnCocConnect();
            }

        }

    }
    else   //kb_event.cnt == 0,  key release
    {
        key_not_released = 0;

        if(key_type == PAIR_UNPAIR_KEY){
            if(central_pairing_enable){
                central_pairing_enable = 0;
                tlkapi_printf(APP_PAIR_LOG_EN, "[UI][PAIR] Pair end.\r\n");
            }
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
