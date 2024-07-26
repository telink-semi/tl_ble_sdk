/********************************************************************************************************
 * @file    default_att.h
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
#ifndef DEFAULT_ATT_H_
#define DEFAULT_ATT_H_

#include "app_config.h"

#ifndef APP_DEFAULT_HID_BATTERY_OTA_ATTRIBUTE_TABLE
#define APP_DEFAULT_HID_BATTERY_OTA_ATTRIBUTE_TABLE        0
#endif

#if (APP_DEFAULT_HID_BATTERY_OTA_ATTRIBUTE_TABLE)


///////////////////////////////////// peripheral-role ATT service HANDLER define ///////////////////////////////////////
typedef enum
{
    ATT_H_START = 0,


    //// Gap ////
    /**********************************************************************************************/
    GenericAccess_PS_H,                     //UUID: 2800,   VALUE: uuid 1800
    GenericAccess_DeviceName_CD_H,          //UUID: 2803,   VALUE:              Prop: Read | Notify
    GenericAccess_DeviceName_DP_H,          //UUID: 2A00,   VALUE: device name
    GenericAccess_Appearance_CD_H,          //UUID: 2803,   VALUE:              Prop: Read
    GenericAccess_Appearance_DP_H,          //UUID: 2A01,   VALUE: appearance
    CONN_PARAM_CD_H,                        //UUID: 2803,   VALUE:              Prop: Read
    CONN_PARAM_DP_H,                        //UUID: 2A04,   VALUE: connParameter


    //// gatt ////
    /**********************************************************************************************/
    GenericAttribute_PS_H,                  //UUID: 2800,   VALUE: uuid 1801
    GenericAttribute_ServiceChanged_CD_H,   //UUID: 2803,   VALUE:              Prop: Indicate
    GenericAttribute_ServiceChanged_DP_H,   //UUID: 2A05,   VALUE: service change
    GenericAttribute_ServiceChanged_CCB_H,  //UUID: 2902,   VALUE: serviceChangeCCC


    //// device information ////
    /**********************************************************************************************/
    DeviceInformation_PS_H,                 //UUID: 2800,   VALUE: uuid 180A
    DeviceInformation_pnpID_CD_H,           //UUID: 2803,   VALUE:              Prop: Read
    DeviceInformation_pnpID_DP_H,           //UUID: 2A50,   VALUE: PnPtrs


    //// HID ////
    /**********************************************************************************************/
    HID_PS_H,                               //UUID: 2800,   VALUE: uuid 1812

    //include
    HID_INCLUDE_H,                          //UUID: 2802,   VALUE: include

    //protocol
    HID_PROTOCOL_MODE_CD_H,                 //UUID: 2803,   VALUE:              Prop: read | write_without_rsp
    HID_PROTOCOL_MODE_DP_H,                 //UUID: 2A4E,   VALUE: protocolMode

    //boot keyboard input report
    HID_BOOT_KB_REPORT_INPUT_CD_H,          //UUID: 2803,   VALUE:              Prop: Read | Notify
    HID_BOOT_KB_REPORT_INPUT_DP_H,          //UUID: 2A22,   VALUE: bootKeyInReport
    HID_BOOT_KB_REPORT_INPUT_CCB_H,         //UUID: 2902,   VALUE: bootKeyInReportCCC

    //boot keyboard output report
    HID_BOOT_KB_REPORT_OUTPUT_CD_H,         //UUID: 2803,   VALUE:              Prop: Read | write| write_without_rsp
    HID_BOOT_KB_REPORT_OUTPUT_DP_H,         //UUID: 2A32,   VALUE: bootKeyOutReport

    //consume report in
    HID_CONSUME_REPORT_INPUT_CD_H,          //UUID: 2803,   VALUE:              Prop: Read | Notify
    HID_CONSUME_REPORT_INPUT_DP_H,          //UUID: 2A4D,   VALUE: reportConsumerIn
    HID_CONSUME_REPORT_INPUT_CCB_H,         //UUID: 2902,   VALUE: reportConsumerInCCC
    HID_CONSUME_REPORT_INPUT_REF_H,         //UUID: 2908    VALUE: REPORT_ID_CONSUMER, TYPE_INPUT

    //keyboard report in
    HID_NORMAL_KB_REPORT_INPUT_CD_H,        //UUID: 2803,   VALUE:              Prop: Read | Notify
    HID_NORMAL_KB_REPORT_INPUT_DP_H,        //UUID: 2A4D,   VALUE: reportKeyIn
    HID_NORMAL_KB_REPORT_INPUT_CCB_H,       //UUID: 2902,   VALUE: reportKeyInInCCC
    HID_NORMAL_KB_REPORT_INPUT_REF_H,       //UUID: 2908    VALUE: REPORT_ID_KEYBOARD, TYPE_INPUT

    //keyboard report out
    HID_NORMAL_KB_REPORT_OUTPUT_CD_H,       //UUID: 2803,   VALUE:              Prop: Read | write| write_without_rsp
    HID_NORMAL_KB_REPORT_OUTPUT_DP_H,       //UUID: 2A4D,   VALUE: reportKeyOut
    HID_NORMAL_KB_REPORT_OUTPUT_REF_H,      //UUID: 2908    VALUE: REPORT_ID_KEYBOARD, TYPE_OUTPUT

    // report map
    HID_REPORT_MAP_CD_H,                    //UUID: 2803,   VALUE:              Prop: Read
    HID_REPORT_MAP_DP_H,                    //UUID: 2A4B,   VALUE: reportKeyIn
    HID_REPORT_MAP_EXT_REF_H,               //UUID: 2907    VALUE: extService

    //hid information
    HID_INFORMATION_CD_H,                   //UUID: 2803,   VALUE:              Prop: read
    HID_INFORMATION_DP_H,                   //UUID: 2A4A    VALUE: hidInformation

    //control point
    HID_CONTROL_POINT_CD_H,                 //UUID: 2803,   VALUE:              Prop: write_without_rsp
    HID_CONTROL_POINT_DP_H,                 //UUID: 2A4C    VALUE: controlPoint


    //// battery service ////
    /**********************************************************************************************/
    BATT_PS_H,                              //UUID: 2800,   VALUE: uuid 180f
    BATT_LEVEL_INPUT_CD_H,                  //UUID: 2803,   VALUE:              Prop: Read | Notify
    BATT_LEVEL_INPUT_DP_H,                  //UUID: 2A19    VALUE: batVal
    BATT_LEVEL_INPUT_CCB_H,                 //UUID: 2902,   VALUE: batValCCC

    //// SPP ////
    /**********************************************************************************************/
    SPP_PS_H,                            //UUID: 2800,  VALUE: telink spp service uuid

    //server to client
    SPP_SERVER_TO_CLIENT_CD_H,           //UUID: 2803,  VALUE:              Prop: read | Notify
    SPP_SERVER_TO_CLIENT_DP_H,           //UUID: telink spp s2c uuid,  VALUE: SppDataServer2ClientData
    SPP_SERVER_TO_CLIENT_CCB_H,          //UUID: 2902,  VALUE: SppDataServer2ClientDataCCC
    SPP_SERVER_TO_CLIENT_DESC_H,         //UUID: 2901,  VALUE: TelinkSPPS2CDescriptor

    //client to server
    SPP_CLIENT_TO_SERVER_CD_H,           //UUID: 2803,  VALUE:              Prop: read | write_without_rsp
    SPP_CLIENT_TO_SERVER_DP_H,           //UUID: telink spp c2s uuid,  VALUE: SppDataClient2ServerData
    SPP_CLIENT_TO_SERVER_DESC_H,         //UUID: 2901,  VALUE: TelinkSPPC2SDescriptor

    //// Ota ////
    /**********************************************************************************************/
    OTA_PS_H,                               //UUID: 2800,   VALUE: telink ota service uuid
    OTA_CMD_OUT_CD_H,                       //UUID: 2803,   VALUE:              Prop: read | write_without_rsp
    OTA_CMD_OUT_DP_H,                       //UUID: telink ota uuid,  VALUE: otaData
    OTA_CMD_OUT_CCB_H,                      //UUID: 2902,   VALUE: my_OtaDataCCC
    OTA_CMD_OUT_DESC_H,                     //UUID: 2901,   VALUE: otaName


    ATT_END_H,

}ATT_HANDLE;




typedef int (*spp_write_callback_t)(u16, void *p);
//extern spp_write_callback_t spp_write_cb;

void blc_spp_registerWriteCallback(spp_write_callback_t cb);

/**
 * @brief   GATT initialization.
 *          !!!Note: this function is used to register ATT table to BLE Stack.
 * @param   none.
 * @return  none.
 */
void my_gatt_init (void);



void app_gatt_storeLatestReadByTypeReq_uuid(u8 *uuid, u8 uuid_len);

extern u8 read_by_type_req_uuidLen;
extern u8 read_by_type_req_uuid[];
///////////////////////////////////// central-role ATT client HANDLER define ///////////////////////////////////////

/**
 * @brief       This function is used to send keyboard HID report by USB.
 * @param[in]   conn     - connection handle
 * @param[in]   p        - Pointer point to data buffer.
 * @return
 */
void    att_keyboard (u16 conn, u8 *p);

/**
 * @brief       This function is used to send consumer HID report by USB.
 * @param[in]   conn     - connection handle
 * @param[in]   p        - Pointer point to data buffer.
 * @return
 */
void    att_keyboard_media (u16 conn, u8 *p);

#endif  //end of APP_USE_DEFAULT_HID_BATTERY_OTA_ATT_TABLE
#endif /* DEFAULT_ATT_H_ */
