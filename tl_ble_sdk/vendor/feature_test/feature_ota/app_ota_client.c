/********************************************************************************************************
 * @file    app_ota_client.c
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
#include "app_buffer.h"
#include "app_ota_client.h"
#include "../default_att.h"

#if (FEATURE_TEST_MODE == TEST_OTA)




typedef struct{
    u8    type;
    u8  rf_len;
    u16    l2capLen;
    u16    chanId;
    u8  opcode;
    u8  datalen;
    u8  data[1];            // character_handle / property / value_handle / value
}ble_att_data_readByTypeRsp_t;










static const  u8 my_OtaUUID[16]    = WRAPPING_BRACES(TELINK_SPP_DATA_OTA);




#if (OTA_CLIENT_SUPPORT_BIG_PDU_ENABLE)
    #define OTA_PDU_LENGTH                                    240  // n*16 (n= 1 ~ 15),
#else
    #define OTA_PDU_LENGTH                                    16
#endif


typedef struct{
    u16 adr_index;
    u8    data[OTA_PDU_LENGTH];
    u16 crc_16;
}ota_pdu_t;


#define OTA_PUBKEY_SIGN_LENGTH                                    16
typedef struct{
    u16 pubkey_sign_cmd;
    u8    pubkey_sign_data[OTA_PUBKEY_SIGN_LENGTH];
    u16 crc_16;
}ota_sign_t;

typedef struct{
    u16 pubkey_offset;
    u16 sign_offset;
}sbdesc_sw_info_t;

u32 ota_trigger_tick;
u32 ota_result_led_indicate_tick;
u32 led_indicate_gpio;


ota_client_t blotaClt;


void app_ota_reset(void)
{
    app_updateOtaFlow(OTA_STEP_0_IDLE);

    blotaClt.ota_start_tick = 0;

    blotaClt.ota_new_fw_addr = 0;
    blotaClt.ota_connHandle = 0;

    blotaClt.cur_adr_index = 0;

    blotaClt.firmware_offset = 0;
}


void app_ota_set_result(int result)
{
    if(result == OTA_SUCCESS){
        ota_result_led_indicate_tick = clock_time() | 1;
    }
    else{
        ota_result_led_indicate_tick = 0;
    }

#if (UI_LED_ENABLE)
    gpio_write(led_indicate_gpio,!LED_ON_LEVEL);  //ota fail
#endif


    int time_S = (clock_time() - blotaClt.ota_start_tick)/SYSTEM_TIMER_TICK_1S + 1;

    tlkapi_send_string_data(APP_OTA_CLIENT_LOG_EN, "[APP][OTA] ota result", &result, 1);
    tlkapi_send_string_data(APP_OTA_CLIENT_LOG_EN, "[APP][OTA] ota time", &time_S, 4);


    app_ota_reset();
}



void app_key_trigger_ota_start(int new_fw_addr, u16 connHandle)
{
    if(blotaClt.ota_update_flow == OTA_STEP_0_IDLE){
        blotaClt.ota_new_fw_addr = new_fw_addr;

        #if (OTA_CLIENT_SEND_SECURE_BOOT_SIGNATURE_ENABLE)
                if(new_fw_addr == NEW_FW_ADDR0){
                    blotaClt.ota_fw_desc_addr = NEW_DESCRIPTOR_ADDR0;
                }
                else{
                    blotaClt.ota_fw_desc_addr = NEW_DESCRIPTOR_ADDR1;
                }
        #endif

        blotaClt.ota_connHandle = connHandle;
        tlkapi_send_string_data(APP_OTA_CLIENT_LOG_EN, "[APP][OTA] new firmware address", &blotaClt.ota_new_fw_addr, 4);

        app_updateOtaFlow(OTA_STEP_1_CHECK_FW);

    }
}




void app_updateOtaFlow(int step)
{
    blotaClt.ota_update_flow = step;

    tlkapi_send_string_data(APP_OTA_CLIENT_LOG_EN, "[APP][OTA] ota flow", &blotaClt.ota_update_flow, 1);
}





void app_ota_process_read_by_type_response(u8 *pkt)
{
    if(blotaClt.ota_update_flow == OTA_STEP_4_WAIT_OTA_HANDLE){

        ble_att_data_readByTypeRsp_t *pRsp = (ble_att_data_readByTypeRsp_t*)pkt;
        blotaClt.ota_attHandle = pRsp->data[0] | pRsp->data[1]<<8;

        app_updateOtaFlow( OTA_STEP_5_REQ_FW_VERSION);
    }
}



void app_ota_process_handle_value_notify(rf_packet_att_t * pAtt)
{
    if(blotaClt.ota_update_flow && pAtt->handle == blotaClt.ota_attHandle){
        u16 ota_cmd = pAtt->dat[0] | pAtt->dat[1]<<8;
        if(ota_cmd == CMD_OTA_FW_VERSION_RSP){
            ota_versionRsp_t *pVersionRsp = (ota_versionRsp_t *)pAtt->dat;

            tlkapi_send_string_u32s(APP_OTA_CLIENT_LOG_EN, "[APP][OTA] FW version", pVersionRsp->version_num, pVersionRsp->version_accept, blotaClt.new_fw_version_num, blotaClt.version_compare);

            if(blotaClt.ota_update_flow == OTA_STEP_6_WAIT_FW_VERSION){

                if(blotaClt.version_compare && !pVersionRsp->version_accept){ //version compare enable, and peer device reject
                    app_ota_set_result(OTA_VERSION_COMPARE_ERR);
                }
                else{
                    app_updateOtaFlow(OTA_STEP_7_OTA_START);
                }
            }
        }
        else if(ota_cmd == CMD_OTA_RESULT){
            ota_result_t *pResult = (ota_result_t *)pAtt->dat;
            if(blotaClt.ota_update_flow == OTA_STEP_11_WAIT_OTA_RESULT)
            {
                app_ota_set_result(pResult->result);
            }
        }

    }
}


void app_proc_ota_update(void)
{
#if 1

    if(blotaClt.ota_start_tick){     //process OTA timeout
        if( clock_time_exceed(blotaClt.ota_start_tick, OTA_TIMEOUT_S * 1000000)){
            app_ota_set_result(OTA_TIMEOUT);
            return;
        }
    }

    u8 ota_buffer[256]; //cover biggest PDU length

    if(blotaClt.ota_update_flow == OTA_STEP_1_CHECK_FW)
    {
        #if (UI_LED_ENABLE)
            if(blotaClt.ota_new_fw_addr == NEW_FW_ADDR0){
                led_indicate_gpio = GPIO_LED_BLUE;
            }
            else if(blotaClt.ota_new_fw_addr == NEW_FW_ADDR1) {
                led_indicate_gpio = GPIO_LED_GREEN;
            }

            gpio_write(led_indicate_gpio, LED_ON_LEVEL); //OTA begin, TODO: not begin
        #endif

        /* FW size stored in: FW start address + 0x00018 */
        flash_read_page(blotaClt.ota_new_fw_addr + 0x00018, 4, (u8 *)&blotaClt.firmware_size);

        /* attention: user set new firmware version here */
        blotaClt.new_fw_version_num = 0x0001;

        tlkapi_send_string_data(APP_OTA_CLIENT_LOG_EN, "[APP][OTA] FW size", &blotaClt.firmware_size, 4);

        int fw_check_err = 0;
        if( blotaClt.firmware_size < FW_SIZE_MIN || blotaClt.firmware_size > FW_SIZE_MAX){
            fw_check_err = 1;
            app_ota_set_result(OTA_FW_SIZE_ERR);
        }

        #if (OTA_CLIENT_SEND_SECURE_BOOT_SIGNATURE_ENABLE)
            if(!fw_check_err){
                u8 buffer[sizeof(sbdesc_sw_info_t)];
                sbdesc_sw_info_t *pSwInfo = (sbdesc_sw_info_t *)buffer;
                u16 desc_offset = OTA_SECURE_BOOT_DESCRIPTOR_SIZE - 0x20;
                flash_read_page(blotaClt.ota_fw_desc_addr + desc_offset,  sizeof(sbdesc_sw_info_t),  buffer);

                if(pSwInfo->pubkey_offset > OTA_SECURE_BOOT_DESCRIPTOR_SIZE || pSwInfo->sign_offset > OTA_SECURE_BOOT_DESCRIPTOR_SIZE ){
                    fw_check_err = 1;
                    app_ota_set_result(OTA_SECBOOT_SYSTEM_ERR);
                    tlkapi_send_string_data(APP_OTA_CLIENT_LOG_EN, "[APP][OTA] descriptor SW information ERROR", buffer, sizeof(sbdesc_sw_info_t));
                }

                if(!fw_check_err){
                    blotaClt.ota_fw_pubkey_addr = blotaClt.ota_fw_desc_addr + pSwInfo->pubkey_offset;
                    blotaClt.ota_fw_sign_addr = blotaClt.ota_fw_desc_addr + pSwInfo->sign_offset;
                    tlkapi_send_string_u32s(APP_OTA_CLIENT_LOG_EN, "[APP][OTA] descriptor information", blotaClt.ota_fw_desc_addr, blotaClt.ota_fw_pubkey_addr, blotaClt.ota_fw_sign_addr, 0);
                }
            }
        #endif


        if(!fw_check_err){
            blotaClt.last_adr_index = blotaClt.firmware_size/OTA_PDU_LENGTH; //default: max_size div 16
            blotaClt.last_valid_pdu_len = blotaClt.firmware_size%OTA_PDU_LENGTH;
            if(blotaClt.last_valid_pdu_len == 0){
                blotaClt.last_valid_pdu_len = OTA_PDU_LENGTH;
            }

            app_updateOtaFlow(OTA_STEP_2_UPDATE_CONN);
        }
    }
    else if(blotaClt.ota_update_flow == OTA_STEP_2_UPDATE_CONN)
    {
        /* set a small conn_interval for OTA update, with high data efficiency,
         * most important: conn_latency must be 0 !!! */
        if(blc_ll_updateConnection (blotaClt.ota_connHandle, CONN_INTERVAL_7P5MS, CONN_INTERVAL_10MS, 0, CONN_TIMEOUT_4S, 0, 0xFFFF ) == BLE_SUCCESS){
            app_updateOtaFlow(OTA_STEP_3_REQ_OTA_HANDLE);
        }
    }
    else if(blotaClt.ota_update_flow == OTA_STEP_3_REQ_OTA_HANDLE)
    {
        /* Send read_by_type_req to get peer device OTA handle
         * once receive peer device's response, get OTA attribute handle */
        if(blc_gatt_pushReadByTypeRequest(blotaClt.ota_connHandle, 0x0001, 0xFFFF, (u8 *)my_OtaUUID, 16) == BLE_SUCCESS){
            app_gatt_storeLatestReadByTypeReq_uuid((u8 *)my_OtaUUID, 16);
            app_updateOtaFlow( OTA_STEP_4_WAIT_OTA_HANDLE);
        }
    }
    else if (blotaClt.ota_update_flow == OTA_STEP_5_REQ_FW_VERSION)
    {
        /* OTA_STEP_5_REQ_FW_VERSION & OTA_STEP_6_WAIT_FW_VERSION are optional
         * if do not need this function, jump to OTA_STEP_7_OTA_START directly
         *  */
        #if (OTA_FW_VERSION_EXCHANGE_ENABLE)
            ota_versionReq_t *pVersion = (ota_versionReq_t *)ota_buffer;
            pVersion->ota_cmd = CMD_OTA_FW_VERSION_REQ;
            pVersion->version_num = blotaClt.new_fw_version_num;  //debug value

            #if (OTA_FW_VERSION_COMPARE_ENABLE)
                blotaClt.version_compare = 1;
                pVersion->version_compare = 1;
            #else
                blotaClt.version_compare = 0;
                pVersion->version_compare = 0;
            #endif

            if(blc_gatt_pushWriteCommand(blotaClt.ota_connHandle, blotaClt.ota_attHandle, ota_buffer,  sizeof(ota_versionReq_t)) == BLE_SUCCESS){
                app_updateOtaFlow( OTA_STEP_6_WAIT_FW_VERSION);
            }
        #else
            app_updateOtaFlow(OTA_STEP_7_OTA_START);  //jump to OTA start directly
        #endif
    }

    else if (blotaClt.ota_update_flow == OTA_STEP_7_OTA_START)
    {
        u8 status;

        #if (!OTA_LEGACY_PROTOCOL)  // use CMD_OTA_START_EXT
                ota_startExt_t *pExtStart = (ota_startExt_t *)ota_buffer;
                pExtStart->ota_cmd = CMD_OTA_START_EXT;
                pExtStart->pdu_length = OTA_PDU_LENGTH;

                tlkapi_send_string_data(APP_OTA_CLIENT_LOG_EN, "[APP][OTA] OTA pdu", &pExtStart->pdu_length, 1);

                #if (OTA_FW_VERSION_COMPARE_ENABLE)
                    pExtStart->version_compare = 1;
                #else
                    pExtStart->version_compare = 0;
                #endif

                status =  blc_gatt_pushWriteCommand(blotaClt.ota_connHandle, blotaClt.ota_attHandle, ota_buffer,  sizeof(ota_startExt_t));

        #else  // use CMD_OTA_START

                ota_start_t *pStart = (ota_start_t *)ota_buffer;
                pStart->ota_cmd = CMD_OTA_START;

                status = blc_gatt_pushWriteCommand(blotaClt.ota_connHandle, blotaClt.ota_attHandle, ota_buffer,  sizeof(ota_start_t));
        #endif

        if(status == BLE_SUCCESS){
            #if (OTA_CLIENT_SEND_SECURE_BOOT_SIGNATURE_ENABLE)
                app_updateOtaFlow( OTA_STEP_8_OTA_SIGNATURE);
                blotaClt.cur_sign_index = 0;
            #else
                app_updateOtaFlow( OTA_STEP_9_OTA_DATA);
            #endif

            blotaClt.ota_start_tick = clock_time() | 1;
        }
    }
    else if (blotaClt.ota_update_flow == OTA_STEP_8_OTA_SIGNATURE)  //send OTA data form address 0 ~ firmware_size
    {
        #if (OTA_CLIENT_SEND_SECURE_BOOT_SIGNATURE_ENABLE)
            /* delay some time after OTA start send, maybe peer device will send some error back */
            if(!clock_time_exceed(blotaClt.ota_start_tick, 50000)){ //50mS
                return;
            }

            static u32 push_ota_sign_tick = 0;
            if(clock_time_exceed(push_ota_sign_tick, 500)){ //500 uS
                push_ota_sign_tick = clock_time();
            }
            else{
                return;
            }


            /* Central TX FIFO not enough */
            if(blc_ll_getTxFifoNumber(blotaClt.ota_connHandle) >= (ACL_CENTRAL_TX_FIFO_NUM - 4)){
                return;
            }


            ota_sign_t *pSign = (ota_sign_t *)ota_buffer;

            pSign->pubkey_sign_cmd = CMD_OTA_SB_PUBKEY_SIGN_MIN | blotaClt.cur_sign_index;
            u32 flash_address;
            if(blotaClt.cur_sign_index < 4){  //public key
                flash_address = blotaClt.ota_fw_pubkey_addr + OTA_PUBKEY_SIGN_LENGTH*blotaClt.cur_sign_index;
            }
            else{ //signature
                flash_address = blotaClt.ota_fw_sign_addr + OTA_PUBKEY_SIGN_LENGTH*(blotaClt.cur_sign_index - 4);
            }
            flash_read_page(flash_address,  OTA_PUBKEY_SIGN_LENGTH,  pSign->pubkey_sign_data);
            pSign->crc_16 = crc16( (u8 *)&pSign->pubkey_sign_cmd, 2 + OTA_PUBKEY_SIGN_LENGTH);


            u8 ret_status = blc_gatt_pushWriteCommand(blotaClt.ota_connHandle, blotaClt.ota_attHandle, ota_buffer, OTA_PUBKEY_SIGN_LENGTH + 4);
            if(ret_status == BLE_SUCCESS){
                tlkapi_send_string_data(APP_OTA_CLIENT_LOG_EN, "[APP][OTA] push OTA signature", ota_buffer, OTA_PUBKEY_SIGN_LENGTH + 2);
                if(blotaClt.cur_sign_index == 7){ //command:FF17
                    app_updateOtaFlow( OTA_STEP_9_OTA_DATA);
                }
                else{
                    blotaClt.cur_sign_index ++;
                }
            }
            else{
                tlkapi_send_string_u32s(APP_OTA_CLIENT_LOG_EN, "[APP][OTA] push err", ret_status, blotaClt.ota_connHandle, blotaClt.ota_attHandle, 0);
            }
        #endif
    }
    else if (blotaClt.ota_update_flow == OTA_STEP_9_OTA_DATA)  //send OTA data form address 0 ~ firmware_size
    {
        /* delay some time after OTA start send, maybe peer device will send some error back */
        if(!clock_time_exceed(blotaClt.ota_start_tick, 50000)){ //50mS
            return;
        }

        static u32 push_ota_data_tick = 0;
        if(clock_time_exceed(push_ota_data_tick, 500)){ //500 uS
            push_ota_data_tick = clock_time();
        }
        else{
            return;
        }

        DBG_CHN7_TOGGLE;

        /* Central TX FIFO not enough */
        if(blc_ll_getTxFifoNumber(blotaClt.ota_connHandle) >= (ACL_CENTRAL_TX_FIFO_NUM - 4)){
            return;
        }

        #if 0 //special test mode
                if(blotaClt.cur_adr_index >= 2){
                    sleep_ms(3000); //trigger OTA process timeout
                    sleep_ms(6000); //trigger OTA  data packet timeout
                }
        #endif

        ota_pdu_t *pPdu = (ota_pdu_t *)ota_buffer;

        pPdu->adr_index = blotaClt.cur_adr_index;

        if(blotaClt.cur_adr_index <= blotaClt.last_adr_index){

            int actual_pdu_len = OTA_PDU_LENGTH;

            /* last OTA PDU process is complicated
             * to compatible with old protocol, must be 16 Bytes aligned, add 0xFF to make up
             * but can not be OTA_PDU_LENGTH aligned. for example, if OTA_PDU_LENGTH is 64:
             *          when last_valid_pdu_len is     4  B, should add 12 0xFF, actual last data PDU is 16B
             *          when last_valid_pdu_len is (16+4) B, should add 12 0xFF, actual last data PDU is 32B
             *          when last_valid_pdu_len is (32+4) B, should add 12 0xFF, actual last data PDU is 48B
             *          when last_valid_pdu_len is (48+4) B, should add 12 0xFF, actual last data PDU is 64B  */
            if(blotaClt.cur_adr_index == blotaClt.last_adr_index){
                flash_read_page(blotaClt.ota_new_fw_addr + blotaClt.cur_adr_index*OTA_PDU_LENGTH,  blotaClt.last_valid_pdu_len,  pPdu->data);


                int align16_makeup_len = 16 - (blotaClt.last_valid_pdu_len & 15); //only make up to make sure 16B aligned(compatible with old protocol)
                actual_pdu_len = blotaClt.last_valid_pdu_len + align16_makeup_len;   //maybe 16/32/64/128/192/208/224/240

                tlkapi_send_string_u32s(APP_OTA_CLIENT_LOG_EN, "[APP][OTA] last PDU", pPdu->adr_index, blotaClt.last_valid_pdu_len, align16_makeup_len, actual_pdu_len);

                //add 0xFF if align16_makeup_len not 0
                for(int i=0; i<align16_makeup_len;i++){
                    pPdu->data[blotaClt.last_valid_pdu_len + i] = 0xFF;
                }
                u16 crc16_cal = crc16( (u8 *)&pPdu->adr_index, 2 + actual_pdu_len);
                pPdu->data[actual_pdu_len] =  U16_LO(crc16_cal);
                pPdu->data[actual_pdu_len + 1] =  U16_HI(crc16_cal);
            }
            else{
                flash_read_page(blotaClt.ota_new_fw_addr + blotaClt.cur_adr_index*OTA_PDU_LENGTH,  OTA_PDU_LENGTH,  pPdu->data);
                pPdu->crc_16 = crc16( (u8 *)&pPdu->adr_index, 2 + actual_pdu_len);
            }


            #if 0 //special test mode: trigger OTA data PDU length error
                if(blotaClt.cur_adr_index == 3){
                    actual_pdu_len -= 1;
                }
            #endif

            DBG_CHN8_TOGGLE;
            u8 ret_status = blc_gatt_pushWriteCommand(blotaClt.ota_connHandle, blotaClt.ota_attHandle, ota_buffer,  4 + actual_pdu_len);
            if(ret_status == BLE_SUCCESS){
//                    tlkapi_send_string_u32s(APP_OTA_CLIENT_LOG_EN, "[APP][OTA] ota data", pPdu->adr_index, 0, 0, 0);
                DBG_CHN9_TOGGLE;
                blotaClt.cur_adr_index ++;
            }
            else{
                tlkapi_send_string_u32s(APP_OTA_CLIENT_LOG_EN, "[APP][OTA] push err", ret_status, blotaClt.ota_connHandle, blotaClt.ota_attHandle, 0);
            }
        }
        else{
            app_updateOtaFlow( OTA_STEP_10_OTA_END);  //all OTA data send OK, go to next step
        }

    }
    else if(blotaClt.ota_update_flow == OTA_STEP_10_OTA_END)
    {
        ota_end_t *pEnd = (ota_end_t *)ota_buffer;
        pEnd->ota_cmd = CMD_OTA_END;
        pEnd->adr_index_max = blotaClt.last_adr_index;
        pEnd->adr_index_max_xor = blotaClt.last_adr_index ^ 0xFFFF;

        if(blc_gatt_pushWriteCommand(blotaClt.ota_connHandle, blotaClt.ota_attHandle, ota_buffer,  6) == BLE_SUCCESS){
            app_updateOtaFlow( OTA_STEP_11_WAIT_OTA_RESULT);
            blotaClt.wait_result_begin_tick = clock_time() | 1;
        }
    }
    else if(blotaClt.ota_update_flow == OTA_STEP_11_WAIT_OTA_RESULT)
    {

        #if (OTA_LEGACY_PROTOCOL)
            if(blc_ll_getTxFifoNumber(blotaClt.ota_connHandle) == 0){  //all data send over, OTA end send OK
                app_ota_set_result(OTA_SUCCESS);
            }
            else if(blotaClt.wait_result_begin_tick && clock_time_exceed(blotaClt.wait_result_begin_tick, 3000000)){
                app_ota_set_result(OTA_SUCCESS);
            }
        #else
            //wait result timeout control
            if(blotaClt.wait_result_begin_tick && clock_time_exceed(blotaClt.wait_result_begin_tick, 5000000)){  //5S

            }
        #endif
    }
#endif
}



#define OTA_TRIG_OTA_RESULT_LED_INTERVAL_US      500000

void app_ota_trigger_led_shine(void)
{
#if (UI_LED_ENABLE)  // 2 led shine for 3 times
    static int led_step = 0;
    if(led_step==0 && clock_time_exceed(ota_trigger_tick, OTA_TRIG_OTA_RESULT_LED_INTERVAL_US)){
        gpio_write(GPIO_LED_BLUE, LED_ON_LEVEL);
        gpio_write(GPIO_LED_GREEN, LED_ON_LEVEL);
        led_step = 1;
    }
    else if(led_step==1 && clock_time_exceed(ota_trigger_tick, OTA_TRIG_OTA_RESULT_LED_INTERVAL_US*2)){
        gpio_write(GPIO_LED_BLUE, !LED_ON_LEVEL);
        gpio_write(GPIO_LED_GREEN, !LED_ON_LEVEL);
        led_step = 2;
    }
    else if(led_step==2 && clock_time_exceed(ota_trigger_tick, OTA_TRIG_OTA_RESULT_LED_INTERVAL_US*3)){
        gpio_write(GPIO_LED_BLUE, LED_ON_LEVEL);
        gpio_write(GPIO_LED_GREEN, LED_ON_LEVEL);
        led_step = 3;
    }
    else if(led_step==3 && clock_time_exceed(ota_trigger_tick, OTA_TRIG_OTA_RESULT_LED_INTERVAL_US*4)){
        gpio_write(GPIO_LED_BLUE, !LED_ON_LEVEL);
        gpio_write(GPIO_LED_GREEN, !LED_ON_LEVEL);
        led_step = 4;
    }
    else if(led_step==4 && clock_time_exceed(ota_trigger_tick, OTA_TRIG_OTA_RESULT_LED_INTERVAL_US*5)){
        gpio_write(GPIO_LED_BLUE, LED_ON_LEVEL);
        gpio_write(GPIO_LED_GREEN, LED_ON_LEVEL);
        led_step = 5;
    }
    else if(led_step==5 && clock_time_exceed(ota_trigger_tick, OTA_TRIG_OTA_RESULT_LED_INTERVAL_US*6)){
        gpio_write(GPIO_LED_BLUE, !LED_ON_LEVEL);
        gpio_write(GPIO_LED_GREEN, !LED_ON_LEVEL);

        led_step = 0;
        ota_trigger_tick = 0;
    }
#endif
}



#define OTA_RESULT_LED_INTERVAL_US      400000
void app_ota_result_led_shine(void)
{
#if (UI_LED_ENABLE)     //led indicate ota
    static int led_step = 0;
    if(led_step==0 && clock_time_exceed(ota_result_led_indicate_tick, OTA_RESULT_LED_INTERVAL_US)){
        gpio_write(led_indicate_gpio,LED_ON_LEVEL);
        led_step = 1;
    }
    else if(led_step==1 && clock_time_exceed(ota_result_led_indicate_tick, OTA_RESULT_LED_INTERVAL_US*2)){
        gpio_write(led_indicate_gpio,!LED_ON_LEVEL);
        led_step = 2;
    }
    else if(led_step==2 && clock_time_exceed(ota_result_led_indicate_tick, OTA_RESULT_LED_INTERVAL_US*3)){
        gpio_write(led_indicate_gpio,LED_ON_LEVEL);
        led_step = 3;
    }
    else if(led_step==3 && clock_time_exceed(ota_result_led_indicate_tick, OTA_RESULT_LED_INTERVAL_US*4)){
        gpio_write(led_indicate_gpio,!LED_ON_LEVEL);
        led_step = 4;
    }
    else if(led_step==4 && clock_time_exceed(ota_result_led_indicate_tick, OTA_RESULT_LED_INTERVAL_US*5)){
        gpio_write(led_indicate_gpio,LED_ON_LEVEL);
        led_step = 5;
    }
    else if(led_step==5 && clock_time_exceed(ota_result_led_indicate_tick, OTA_RESULT_LED_INTERVAL_US*6)){
        gpio_write(led_indicate_gpio,!LED_ON_LEVEL);

        ota_result_led_indicate_tick = 0;
        led_step = 0;
    }
#else
    ota_result_led_indicate_tick = 0;
#endif
}


void app_process_ota_connection_terminate(u16 connHandle)
{
    if(connHandle == blotaClt.ota_connHandle){ //connection match
        if(blotaClt.ota_update_flow){ //OTA not finish, but connection terminate
            #if (OTA_LEGACY_PROTOCOL)
                if(blotaClt.ota_update_flow == OTA_STEP_11_WAIT_OTA_RESULT && blc_ll_getTxFifoNumber(blotaClt.ota_connHandle) == 0){
                    //all data send over, OTA end send OK
                    app_ota_set_result(OTA_SUCCESS);
                }
                else{
                    app_ota_set_result(OTA_FAIL_DUE_TO_CONNECTION_TERMINATE);
                }
            #else
                app_ota_set_result(OTA_FAIL_DUE_TO_CONNECTION_TERMINATE);
            #endif
        }
    }
}


void app_process_ota_mainloop(void)
{
    if(ota_trigger_tick){
        app_ota_trigger_led_shine();
    }

    if(blotaClt.ota_update_flow){
        app_proc_ota_update();
    }

    if(ota_result_led_indicate_tick){
        app_ota_result_led_shine();
    }
}




#endif
