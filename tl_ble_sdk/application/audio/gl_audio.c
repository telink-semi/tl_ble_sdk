/********************************************************************************************************
 * @file    gl_audio.c
 *
 * @brief   This is the source file for BLE SDK
 *
 * @author  BLE GROUP
 * @date    06,2020
 *
 * @par     Copyright (c) 2020, Telink Semiconductor (Shanghai) Co., Ltd. ("TELINK")
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
#include    "tl_common.h"
#include    "drivers.h"
#include    "audio_config.h"
#include    "adpcm.h"
#include    "gl_audio.h"

#if (TL_AUDIO_MODE == TL_AUDIO_RCU_ADPCM_GATT_GOOGLE)

#include    "stack/ble/ble.h"

_attribute_data_retention_ u32 app_audio_timer = 0;
_attribute_data_retention_ u32 app_audio_waitack_timer = 0;

_attribute_data_retention_ u8 app_audio_key_flags = 0;  //bit[7]:1-press,0-release;
                                                       //bit[0]:1-report press; bit[1]:1-report release
                                                      //bit[2]:1-report search for google
_attribute_data_retention_ u16 app_audio_sync_serial = 0;

_attribute_data_retention_ u8 htt_audio_model_enable_flags = 0;
_attribute_data_retention_ u8 on_request_audio_model_enable_flags = 0;
_attribute_data_retention_ u8 ptt_audio_model_enable_flags = 0;

_attribute_data_retention_ u8 htt_audio_model_key_press_flags = 0;
_attribute_data_retention_ u8 ptt_audio_model_key_press_flags = 0;

_attribute_data_retention_ u8 g_stream_id = 1;
_attribute_data_retention_ u8 app_audio_status = 0;

_attribute_data_retention_ u16 audio_google_ctl_dp_h = 0;// = 57;        //AUDIO_GOOGLE_CTL_DP_H
_attribute_data_retention_ u16 hid_consume_report_input_dp_h = 0;// = 25;//HID_CONSUME_REPORT_INPUT_DP_H
void google_handle_init(u16 ctl_dp_h, u16 report_dp_h)
{
    audio_google_ctl_dp_h = ctl_dp_h;
    hid_consume_report_input_dp_h = report_dp_h;
}

unsigned char app_audio_key_start(u16 connHandle, unsigned char isPress)
{
    u8 temp = 0;
    u8 sendBuff[20]  = {0};

    if(isPress)
    {
        if(htt_audio_model_enable_flags || ptt_audio_model_enable_flags)
        {
            // htt model


            //htt audio start
            sendBuff[0] = ATV_MIC_CHAR_RSP_OPEN;
            if(htt_audio_model_enable_flags )
            {
                htt_audio_model_key_press_flags = 1;
                sendBuff[1] = REASON_HTT; //reason :HTT Audio transfer is triggered by button press and will stop once the button is released.
            }
            else
            {
                //ptt model
                ptt_audio_model_key_press_flags = 1;
                sendBuff[1] = REASON_PTT; //reason :PTT Audio transfer is triggered by button press
            }

            sendBuff[2] = CS_16K16B; //codec used :ADPCM 16kHz/16bit
            if(g_stream_id > 0x80)
            {
                g_stream_id = 1;
            }
            sendBuff[3] = g_stream_id; //streamid :1 - 0x80;

            g_stream_id ++;

            if(BLE_SUCCESS != blc_gatt_pushHandleValueNotify (connHandle,audio_google_ctl_dp_h, sendBuff, 4))
            {
                return APP_AUDIO_PROCESS_ERR;
            }

            temp = APP_AUDIO_ENABLE;//enable audio

        }
        else if(on_request_audio_model_enable_flags)
        {
            //on request model
            if(app_audio_key_flags & APP_AUDIO_KEY_FLAG_PRESS) {
                return APP_AUDIO_PROCESS_ERR;
            }
            app_audio_key_flags |= APP_AUDIO_KEY_FLAG_PRESS;

            app_audio_waitack_timer = clock_time() | 1;

            app_audio_key_flags |= APP_AUDIO_KEY_FLAG_REPORT_PRESS;

            if((app_audio_key_flags & APP_AUDIO_KEY_FLAG_REPORT_PRESS))
            {
                ble_sts_t ret;
                u16 search_key = 0x0221;
                ret = blc_gatt_pushHandleValueNotify (connHandle,hid_consume_report_input_dp_h, (u8*)&search_key, 2);
                if(ret != BLE_SUCCESS) {
                    return APP_AUDIO_PROCESS_ERR;
                }
                app_audio_key_flags &= ~APP_AUDIO_KEY_FLAG_REPORT_PRESS;
                app_audio_key_flags |= APP_AUDIO_KEY_FLAG_REPORT_RELEASE;

            }
            if(app_audio_key_flags & APP_AUDIO_KEY_FLAG_REPORT_RELEASE)
            {
                ble_sts_t ret;
                u16 consumerKey_release = 0x0000;
                ret = blc_gatt_pushHandleValueNotify (connHandle,hid_consume_report_input_dp_h, (u8*)&consumerKey_release, 2);
                if(ret != BLE_SUCCESS) {
                    return APP_AUDIO_PROCESS_ERR;
                }
                app_audio_key_flags &= ~APP_AUDIO_KEY_FLAG_REPORT_RELEASE;
                app_audio_key_flags |= APP_AUDIO_KEY_FLAG_REPORT_START;

            }

            if((app_audio_key_flags & APP_AUDIO_KEY_FLAG_REPORT_START))
            {
                ble_sts_t ret;
                u8 search = 0x08;
                ret = blc_gatt_pushHandleValueNotify (connHandle,audio_google_ctl_dp_h, &search, 1);
                if(ret != BLE_SUCCESS) {
                    return APP_AUDIO_PROCESS_ERR;
                }
                app_audio_key_flags &= ~APP_AUDIO_KEY_FLAG_REPORT_START;

            }
        }
        else
        {
            return APP_AUDIO_PROCESS_ERR;
        }

    }
    else
    {
        if(htt_audio_model_enable_flags)
        {
            htt_audio_model_key_press_flags = 0;
            //disable audio
            temp = APP_AUDIO_DISABLE;
            //send audio stop to  master
            ble_sts_t ret;
            sendBuff[0] = ATV_MIC_CHAR_RSP_CLOSE;
            sendBuff[1] = AS_RELEASE_BUTTON; //triggered by releasing an Assistant button during HTT interaction;
            ret = blc_gatt_pushHandleValueNotify (connHandle,audio_google_ctl_dp_h, sendBuff, 2);
            if(ret != BLE_SUCCESS) {
                return APP_AUDIO_PROCESS_ERR;
            }

        }
        else if(on_request_audio_model_enable_flags)
        {
            app_audio_key_flags &= ~APP_AUDIO_KEY_FLAG_PRESS;
        }
    }
    return temp;
}

extern void ui_enable_mic(int en);

int app_audio_timeout_proc(void){
    if(app_audio_timer != 0 && clock_time_exceed(app_audio_timer, APP_AUDIO_GOOGLE_TRANSFER_TIMEOUT))
    {
        ui_enable_mic(0);

        u8 sendBuff[2] = {0};
        sendBuff[0] = ATV_MIC_CHAR_RSP_CLOSE;
        sendBuff[1] = AS_TIMEOUT;
        for (int i = ACL_CENTRAL_MAX_NUM; i < (ACL_CENTRAL_MAX_NUM + ACL_PERIPHR_MAX_NUM); i++) { //peripheral index is from "ACL_CENTRAL_MAX_NUM" to "ACL_CENTRAL_MAX_NUM + ACL_PERIPHR_MAX_NUM - 1"
            ble_sts_t ret;
            if (conn_dev_list[i].conn_state) {
                ret = blc_gatt_pushHandleValueNotify (conn_dev_list[i].conn_handle,audio_google_ctl_dp_h, sendBuff, 2);
            }
            if(ret != BLE_SUCCESS) {
                return 1;
            }
        }
    }

    if(on_request_audio_model_enable_flags)
    {
        if(app_audio_waitack_timer != 0 && clock_time_exceed(app_audio_waitack_timer, APP_AUDIO_GOOGLE_REMOTE_TIMEOUT))
        {
            app_audio_waitack_timer = 0;
            app_audio_key_flags = 0;
            ui_enable_mic(0);
            return  1;
        }
    }
    return 0;
}

int app_auido_google_callback(u16 connHandle, void* p)
{
    u8 sendBuff[20]  = {0};
    rf_packet_att_data_t *pw = (rf_packet_att_data_t *)p;

    if(pw->dat[0] == AUDIO_GOOGLE_CMD_OPEN)
    {
        if(htt_audio_model_enable_flags)
        {
            //when htt model ongoing mic open comes send mic already open error (0x0f01) and continue
            sendBuff[0] = ATV_MIC_CHAR_RSP_MIC_OPEN_ERROR;
            sendBuff[1] = MIC_ALREADY_OPEN_HIGH;
            sendBuff[2] = MIC_ALREADY_OPEN_LOW;
            if(BLE_SUCCESS != blc_gatt_pushHandleValueNotify (connHandle,audio_google_ctl_dp_h, sendBuff, 3))
            {
                return 0;
            }
        }
        else if(on_request_audio_model_enable_flags)
        {
            //on request model -- audio start
            sendBuff[0] = ATV_MIC_CHAR_RSP_OPEN;
            sendBuff[1] = REASON_MICOPEN; //reason :Audio transfer is triggered by MIC_OPEN request;
            sendBuff[2] = CS_16K16B; //codec used :ADPCM 16kHz/16bit
            sendBuff[3] = 0x00; //streamid = 0;
            if(BLE_SUCCESS != blc_gatt_pushHandleValueNotify (connHandle,audio_google_ctl_dp_h, sendBuff, 4))
            {
                return 0;
            }

            ui_enable_mic(1);

            app_audio_waitack_timer = 0;

        }

    }
    else if(pw->dat[0] == AUDIO_GOOGLE_CMD_CLOSE)
    {
        u8 streamid = pw->dat[1];

        if(htt_audio_model_enable_flags || ptt_audio_model_enable_flags )
        {
            //htt model or ptt model
            if(streamid != 0)
            {
                ui_enable_mic(0);
                sendBuff[0] = ATV_MIC_CHAR_RSP_CLOSE;
                sendBuff[1] = AS_MICCLOSE; //triggered by MIC_CLOSE message;
                if(BLE_SUCCESS != blc_gatt_pushHandleValueNotify (connHandle,audio_google_ctl_dp_h, sendBuff, 2))
                {
                    return 0;
                }
            }
        }
        else
        {
            //on request model
            ui_enable_mic(0);
            sendBuff[0] = ATV_MIC_CHAR_RSP_CLOSE;
            sendBuff[1] = AS_MICCLOSE; //triggered by MIC_CLOSE message;
            if(BLE_SUCCESS != blc_gatt_pushHandleValueNotify (connHandle,audio_google_ctl_dp_h, sendBuff, 2))
            {
                return 0;
            }
        }
    }
    else if(pw->dat[0] == AUDIO_GOOGLE_CMD_CAP)
    {
        u8 model = pw->dat[5];

        if(ON_REQUEST == model)
        {
            //on request model
            on_request_audio_model_enable_flags = 1;
        }

        else if (PTT == model)
        {
            //ptt model and on request model
            on_request_audio_model_enable_flags = 1;
            ptt_audio_model_enable_flags = 1;
        }

        else if (HTT == model)
        {
            //htt model, ptt model and on request model
            on_request_audio_model_enable_flags = 1;
            ptt_audio_model_enable_flags = 1;
            htt_audio_model_enable_flags = 1;
        }


        if(htt_audio_model_enable_flags || ptt_audio_model_enable_flags)
        {
            sendBuff[0] = ATV_MIC_CHAR_RSP_CAP;
            sendBuff[1] = VERSION_10_HIGH; // version high
            sendBuff[2] = VERSION_10_LOW; // version low 1.0
            sendBuff[3] = CS_16K16B; //codecs_supported 16Khz(ADPCM,16bits)
            if(htt_audio_model_enable_flags)
            {
                sendBuff[4] = HTT; //assistant interaction model ON_Request(0x00)HTT(0x03)
            }
            else
            {
                sendBuff[4] = PTT; //assistant interaction model ON_Request(0x00)HTT(0x03)
            }

            sendBuff[5] = BYTE_20_HIGH; //frame lengths high
            sendBuff[6] = BYTE_20_LOW; //frame lengths low -- 20byte
            sendBuff[7] = EcReserved; // not used
            sendBuff[8] = 0x00; //not used
        }
        else
        {
            //on request
            sendBuff[0] = ATV_MIC_CHAR_RSP_CAP;
            sendBuff[1] = VERSION_10_HIGH; //major version
            sendBuff[2] = VERSION_10_LOW; //minor version
            sendBuff[3] = CS_16K16B; //codecs_supported 16Khz(ADPCM,16bits)
            sendBuff[4] = ON_REQUEST;
            sendBuff[5] = BYTE_20_HIGH; //frame lengths high
            sendBuff[6] = BYTE_20_LOW; //frame lengths low -- 20byte
            sendBuff[7] = EcReserved; // not used
            sendBuff[8] = 0x00; //not used

        }

        if(blc_gatt_pushHandleValueNotify (connHandle,audio_google_ctl_dp_h, sendBuff, 9))
        {
            return 0;
        }

    }
    else if(pw->dat[0] == AUDIO_GOOGLE_CMD_EXTEND)
    {
        //fresh the timeout
        app_audio_timer = clock_time() | 1;
    }

    return 0;

}


#else

#endif
