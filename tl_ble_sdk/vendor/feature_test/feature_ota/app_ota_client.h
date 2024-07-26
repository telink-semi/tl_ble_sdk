/********************************************************************************************************
 * @file    app_ota_client.h
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
#ifndef VENDOR_B91_FEATURE_FEATURE_OTA_APP_OTA_CLIENT_H_
#define VENDOR_B91_FEATURE_FEATURE_OTA_APP_OTA_CLIENT_H_

#if (FEATURE_TEST_MODE == TEST_OTA)


/**
 * @brief   peer device's new firmware storage address on local device
 */
#define NEW_FW_ADDR_128K                                0x20000
#define NEW_FW_ADDR_256K                                0x40000
#define NEW_FW_ADDR_512K                                0x80000
#define NEW_FW_ADDR_768K                                0xC0000



#define NEW_FW_ADDR0                                    NEW_FW_ADDR_512K
#define NEW_FW_ADDR1                                    NEW_FW_ADDR_768K
#define FW_SIZE_MIN                                     0x04000  //16K
#define FW_SIZE_MAX                                     0x40000  //256K

#define NEW_DESCRIPTOR_ADDR0                            0x100000
#define NEW_DESCRIPTOR_ADDR1                            0x102000





#define OTA_STEP_0_IDLE                                 0
#define OTA_STEP_1_CHECK_FW                             1   //check Firmware
#define OTA_STEP_2_UPDATE_CONN                          2   //update to a small connection interval to speed up OTA process
#define OTA_STEP_3_REQ_OTA_HANDLE                       3   //send read_by_type_req to request peer device's OTA attribute handle
#define OTA_STEP_4_WAIT_OTA_HANDLE                      4   //wait read_by_type_rsp to get peer device's OTA attribute handle

//step 5 & 6: optional
#define OTA_STEP_5_REQ_FW_VERSION                       5   //send OTA command "CMD_OTA_FW_VERSION_REQ" to request peer device's firmware version
#define OTA_STEP_6_WAIT_FW_VERSION                      6   //wait OTA command "CMD_OTA_FW_VERSION_RSP" to get peer device's firmware version

#define OTA_STEP_7_OTA_START                            7   //send OTA command "CMD_OTA_START" or "CMD_OTA_START_EXT" to trigger OTA

//step 8: optional
#define OTA_STEP_8_OTA_SIGNATURE                        8

#define OTA_STEP_9_OTA_DATA                             9
#define OTA_STEP_10_OTA_END                             10
#define OTA_STEP_11_WAIT_OTA_RESULT                     11






#define OTA_TIMEOUT_S                                   100 //user can change




#if (!OTA_LEGACY_PROTOCOL)
    #define OTA_FW_VERSION_EXCHANGE_ENABLE              0   //user can change
    #define OTA_FW_VERSION_COMPARE_ENABLE               0   //user can change
#endif







typedef struct{
    u8  ota_update_flow;
    u8  version_compare;
    u8  ota_test_mode;
    u8  last_valid_pdu_len;  //maximum value: 240

    u8  cur_sign_index;
    u8  u8_rsvd[3];

    u16 ota_attHandle;
    u16 ota_connHandle;
    u16 cur_adr_index;
    u16 last_adr_index;
    u16 new_fw_version_num;
    u16 u16_rsvd;

    u32 ota_new_fw_addr;
    u32 firmware_offset;
    u32 firmware_size;

    u32 ota_start_tick;
    u32 wait_result_begin_tick;

    //for secure boot
    u32 ota_fw_desc_addr;  //FW descriptor address
    u32 ota_fw_pubkey_addr;
    u32 ota_fw_sign_addr;
}ota_client_t;

extern ota_client_t blotaClt;

extern u32 ota_trigger_tick;


void app_key_trigger_ota_start(int new_fw_addr, u16 connHandle);

void app_process_ota_mainloop(void);

void app_process_ota_connection_terminate(u16 connHandle);

void app_ota_process_handle_value_notify(rf_packet_att_t *);

void app_ota_process_read_by_type_response(u8 *pkt);

void app_ota_set_result(int result);

void app_updateOtaFlow(int step);

#endif
#endif /* VENDOR_B91_FEATURE_FEATURE_OTA_APP_OTA_CLIENT_H_ */
