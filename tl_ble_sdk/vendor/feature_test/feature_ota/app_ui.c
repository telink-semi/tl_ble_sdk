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
#include "app_ota_client.h"



#if (FEATURE_TEST_MODE == TEST_OTA)

int    central_pairing_enable = 0;
u16 central_unpair_enable = 0;

u16 central_disconnect_connhandle;   //mark the central connection which is in un_pair disconnection flow









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
//    u8 key1 = kb_event.keycode[1];
//    u8 key_buf[8] = {0,0,0,0,0,0,0,0};

    key_not_released = 1;
    if (kb_event.cnt == 2)   //two key press
    {

    }
    else if(kb_event.cnt == 1)
    {
        static u32 last_singleKey_press_tick;


        if(key0 == CR_VOL_UP || key0 == CR_VOL_DN)
        {
            #if (BLE_OTA_CLIENT_ENABLE)

                if(blotaClt.ota_test_mode == 0){
                    static u32 button_history = 0;
                    if(!clock_time_exceed(last_singleKey_press_tick, 2000000)){
                        button_history = button_history<<1 | (key0 == CR_VOL_UP || key0 == CR_VOL_DN);
                        if( (button_history & 0x0f) == 0x0f){ //fast key press 4 time
                            blotaClt.ota_test_mode = 1;  //enter OTA test mode, LED shine
                            ota_trigger_tick = clock_time() | 1;
                            tlkapi_send_string_data(APP_OTA_CLIENT_LOG_EN, "[APP][OTA] enter test mode", 0, 0);
                        }
                    }
                    else{
                        button_history = 0;
                    }
                }
                else if(blotaClt.ota_test_mode == 1){
                    if(acl_conn_central_num){ //at least one connection exist
                        if(key0 == CR_VOL_UP){
                            app_key_trigger_ota_start(NEW_FW_ADDR0, conn_dev_list[0].conn_handle);
                        }
                        else if(key0 == CR_VOL_DN){
                            app_key_trigger_ota_start(NEW_FW_ADDR1, conn_dev_list[0].conn_handle);
                        }
                    }
                }



            #endif
        }
        if(key0 == BTN_PAIR)   //Manual pair triggered by Key Press
        {
            central_pairing_enable = 1;
            tlkapi_send_string_data(APP_PAIR_LOG_EN, "[UI][PAIR] Pair begin", 0, 0);
        }
        else if(key0 == BTN_UNPAIR) //Manual un_pair triggered by Key Press
        {
            /*Here is just Telink Demonstration effect. Cause the demo board has limited key to use, only one "un_pair" key is
             available. When "un_pair" key pressed, we will choose and un_pair one device in connection state */
            if(acl_conn_central_num){ //at least 1 ACL Central exist

                if(!central_disconnect_connhandle){  //if one central un_pair disconnection flow not finish, here new un_pair not accepted

                    /* choose one central connection to disconnect */
                    for(int i=0; i < ACL_CENTRAL_MAX_NUM; i++){ //ACL Peripheral index is from 0 to "ACL_CENTRAL_MAX_NUM - 1"
                        if(conn_dev_list[i].conn_state){
                            central_unpair_enable = conn_dev_list[i].conn_handle;  //mark connHandle on central_unpair_enable
                            tlkapi_send_string_data(APP_PAIR_LOG_EN, "[UI][PAIR] Unpair", &central_unpair_enable, 2);
                            break;
                        }
                    }
                }
            }
        }

        last_singleKey_press_tick = clock_time();  //update single key press tick
    }
    else   //kb_event.cnt == 0,  key release
    {
        if(central_pairing_enable){
            central_pairing_enable = 0;
        }

        if(central_unpair_enable){
            central_unpair_enable = 0;
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







_attribute_ble_data_retention_    u8         ota_is_working = 0;


/**
 * @brief      this function is used to register the function for OTA start.
 * @param[in]  none
 * @return     none
 */
void app_enter_ota_mode(void)
{
    ota_is_working = 1;

    #if(UI_LED_ENABLE)  //this is only for debug
        gpio_write(GPIO_LED_BLUE, 1);
    #endif
}






/**
 * @brief       no matter whether the OTA result is successful or fail.
 *              code will run here to tell user the OTA result.
 * @param[in]   result    OTA result:success or fail(different reason)
 * @return      none
 */
void app_ota_result(int result)
{
    #if (1)  //this is only for debug
        if(result == OTA_SUCCESS){  //led for debug: OTA success
            gpio_write(GPIO_LED_GREEN, 1);
        }
        else{  //OTA fail

            gpio_write(GPIO_LED_GREEN, 0);

            #if 0 //this is only for debug,  can not use this in application code
                irq_disable();
                WATCHDOG_DISABLE;

                while(1){
                }
            #endif
        }
    #endif

    gpio_write(GPIO_LED_BLUE, 0);
    gpio_write(GPIO_LED_WHITE, 1);

//    sleep_us(2000000);  //debug
}



void app_ota_server_init(void)
{
    #if (APP_HW_FIRMWARE_ENCRYPTION_ENABLE)
        blc_ota_enableFirmwareEncryption();
    #endif

    #if (APP_HW_SECURE_BOOT_ENABLE)
        /* attention that "blc_ota enableSecureBoot" must be called before "blc_ota initOtaServer_module" !!! */
        blc_ota_enableSecureBoot();
    #endif

    #if (TLKAPI_DEBUG_ENABLE)
        /* user can enable OTA flow log in BLE stack */
        //blc_debug_addStackLog(STK_LOG_OTA_FLOW);
    #endif

    blc_ota_initOtaServer_module();

    blc_ota_registerOtaStartCmdCb(app_enter_ota_mode);
    blc_ota_registerOtaResultIndicationCb(app_ota_result);
    blc_ota_setOtaProcessTimeout(40);   //OTA process timeout:  40 seconds
    blc_ota_setOtaDataPacketTimeout(3); //OTA data packet interval timeout 3 seconds
}




/**
 * @brief   BLE Unpair handle for central
 * @param   none.
 * @return  none.
 */
void proc_central_role_unpair(void)
{

    //terminate and un_pair process, Telink demonstration effect: triggered by "un_pair" key press
    if(central_unpair_enable){

        dev_char_info_t* dev_char_info = dev_char_info_search_by_connhandle(central_unpair_enable); //connHandle has marked on on central_unpair_enable

        if( dev_char_info ){ //un_pair device in still in connection state

            if(blc_ll_disconnect(central_unpair_enable, HCI_ERR_REMOTE_USER_TERM_CONN) == BLE_SUCCESS){

                central_disconnect_connhandle = central_unpair_enable; //mark conn_handle

                central_unpair_enable = 0;  //every "un_pair" key can only triggers one connection disconnect


                // delete this device information(mac_address and distributed keys...) on FLash
                #if (ACL_CENTRAL_SMP_ENABLE)
                    blc_smp_deleteBondingPeripheralInfo_by_PeerMacAddress(dev_char_info->peer_adrType, dev_char_info->peer_addr);
                #endif
            }

        }
        else{ //un_pair device can not find in device list, it's not connected now

            central_unpair_enable = 0;  //every "un_pair" key can only triggers one connection disconnect
        }

    }
}








#endif
