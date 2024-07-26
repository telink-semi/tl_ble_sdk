/********************************************************************************************************
 * @file    default_att.c
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

#include "default_att.h"

#include "app_config.h"

#if (APP_DEFAULT_HID_BATTERY_OTA_ATTRIBUTE_TABLE)

_attribute_ble_data_retention_  spp_write_callback_t spp_write_cb = NULL;


void blc_spp_registerWriteCallback(spp_write_callback_t cb)
{
    spp_write_cb = cb;
}

////////////////////////////////////////// peripheral-role ATT service concerned ///////////////////////////////////////////////
typedef struct
{
  /** Minimum value for the connection event (interval. 0x0006 - 0x0C80 * 1.25 ms) */
  u16 intervalMin;
  /** Maximum value for the connection event (interval. 0x0006 - 0x0C80 * 1.25 ms) */
  u16 intervalMax;
  /** Number of LL latency connection events (0x0000 - 0x03e8) */
  u16 latency;
  /** Connection Timeout (0x000A - 0x0C80 * 10 ms) */
  u16 timeout;
} gap_periConnectParams_t;

static const u16 clientCharacterCfgUUID = GATT_UUID_CLIENT_CHAR_CFG;

static const u16 extReportRefUUID = GATT_UUID_EXT_REPORT_REF;

static const u16 reportRefUUID = GATT_UUID_REPORT_REF;

static const u16 userdesc_UUID  = GATT_UUID_CHAR_USER_DESC;

static const u16 serviceChangeUUID = GATT_UUID_SERVICE_CHANGE;

static const u16 my_primaryServiceUUID = GATT_UUID_PRIMARY_SERVICE;

static const u16 my_characterUUID = GATT_UUID_CHARACTER;

static const u16 my_devServiceUUID = SERVICE_UUID_DEVICE_INFORMATION;

static const u16 my_PnPUUID = CHARACTERISTIC_UUID_PNP_ID;

static const u16 my_devNameUUID = GATT_UUID_DEVICE_NAME;

static const u16 my_gapServiceUUID = SERVICE_UUID_GENERIC_ACCESS;

static const u16 my_appearanceUUID = GATT_UUID_APPEARANCE;

static const u16 my_periConnParamUUID = GATT_UUID_PERI_CONN_PARAM;

static const u16 my_appearance = GAP_APPEARANCE_UNKNOWN;

static const u16 my_gattServiceUUID = SERVICE_UUID_GENERIC_ATTRIBUTE;

static const gap_periConnectParams_t my_periConnParameters = {20, 40, 0, 1000};

_attribute_ble_data_retention_  static u16 serviceChangeVal[2] = {0};

_attribute_ble_data_retention_  static u8 serviceChangeCCC[2] = {0,0};

static const u8 my_devName[] = {'f','e','a','t','u','r','e'};

static const u8 my_PnPtrs [] = {0x02, 0x8a, 0x24, 0x66, 0x82, 0x01, 0x00};

//////////////////////// Battery /////////////////////////////////////////////////
static const u16 my_batServiceUUID        = SERVICE_UUID_BATTERY;
static const u16 my_batCharUUID           = CHARACTERISTIC_UUID_BATTERY_LEVEL;
_attribute_ble_data_retention_  static u8 batteryValueInCCC[2] = {0,0};
_attribute_ble_data_retention_  static u8 my_batVal[1]  = {99};

//////////////////////// HID /////////////////////////////////////////////////////

static const u16 my_hidServiceUUID        = SERVICE_UUID_HUMAN_INTERFACE_DEVICE;

static const u16 hidProtocolModeUUID      = CHARACTERISTIC_UUID_HID_PROTOCOL_MODE;
static const u16 hidReportUUID            = CHARACTERISTIC_UUID_HID_REPORT;
static const u16 hidReportMapUUID         = CHARACTERISTIC_UUID_HID_REPORT_MAP;
static const u16 hidBootKeyInReportUUID   = CHARACTERISTIC_UUID_HID_BOOT_KEY_INPUT;
static const u16 hidBootKeyOutReportUUID  = CHARACTERISTIC_UUID_HID_BOOT_KEY_OUTPUT;
static const u16 hidInformationUUID       = CHARACTERISTIC_UUID_HID_INFORMATION;
static const u16 hidCtrlPointUUID         = CHARACTERISTIC_UUID_HID_CONTROL_POINT;
static const u16 hidIncludeUUID           = GATT_UUID_INCLUDE;

_attribute_ble_data_retention_  static u8 protocolMode            = DFLT_HID_PROTOCOL_MODE;

// Key in Report characteristic variables
_attribute_ble_data_retention_  static u8 reportKeyIn[8];
_attribute_ble_data_retention_  static u8 reportKeyInCCC[2] = {0,0};
// HID Report Reference characteristic descriptor, key input
_attribute_ble_data_retention_  static u8 reportRefKeyIn[2] =
             { HID_REPORT_ID_KEYBOARD_INPUT, HID_REPORT_TYPE_INPUT };

// Key out Report characteristic variables
_attribute_ble_data_retention_  static u8 reportKeyOut[1];
_attribute_ble_data_retention_  static u8 reportRefKeyOut[2] =
             { HID_REPORT_ID_KEYBOARD_INPUT, HID_REPORT_TYPE_OUTPUT };

// Consumer Control input Report
_attribute_ble_data_retention_  static u8 reportConsumerControlIn[2];
_attribute_ble_data_retention_  static u8 reportConsumerControlInCCC[2] = {0,0};
_attribute_ble_data_retention_  static u8 reportRefConsumerControlIn[2] =
             { HID_REPORT_ID_CONSUME_CONTROL_INPUT, HID_REPORT_TYPE_INPUT };

// Boot Keyboard Input Report
_attribute_ble_data_retention_  static u8 bootKeyInReport;
_attribute_ble_data_retention_  static u8 bootKeyInReportCCC[2] = {0,0};

// Boot Keyboard Output Report
_attribute_ble_data_retention_  static u8 bootKeyOutReport;

// HID Information characteristic
static const u8 hidInformation[] =
{
  U16_LO(0x0111), U16_HI(0x0111),             // bcdHID (USB HID version)
  0x00,                                       // bCountryCode
  0x01                                        // Flags
};

// HID Control Point characteristic
_attribute_ble_data_retention_  static u8 controlPoint;



typedef struct{
    u8  type;
    u8  rf_len;
    u16 l2capLen;
    u16 chanId;
    u8  opcode;
    u16 handle;
    u8 value;
}ble_rf_packet_att_write_t;

// HID Report Map characteristic
// Keyboard report descriptor (using format for Boot interface descriptor)

static const u8 reportMap[] =
{
    //keyboard report in
    0x05, 0x01,     // Usage Pg (Generic Desktop)
    0x09, 0x06,     // Usage (Keyboard)
    0xA1, 0x01,     // Collection: (Application)
    0x85, HID_REPORT_ID_KEYBOARD_INPUT,     // Report Id (keyboard)
                  //
    0x05, 0x07,     // Usage Pg (Key Codes)
    0x19, 0xE0,     // Usage Min (224)  VK_CTRL:0xe0
    0x29, 0xE7,     // Usage Max (231)  VK_RWIN:0xe7
    0x15, 0x00,     // Log Min (0)
    0x25, 0x01,     // Log Max (1)
                  //
                  // Modifier byte
    0x75, 0x01,     // Report Size (1)   1 bit * 8
    0x95, 0x08,     // Report Count (8)
    0x81, 0x02,     // Input: (Data, Variable, Absolute)
                  //
                  // Reserved byte
    0x95, 0x01,     // Report Count (1)
    0x75, 0x08,     // Report Size (8)
    0x81, 0x01,     // Input: (static constant)

    //keyboard output
    //5 bit led ctrl: NumLock CapsLock ScrollLock Compose kana
    0x95, 0x05,    //Report Count (5)
    0x75, 0x01,    //Report Size (1)
    0x05, 0x08,    //Usage Pg (LEDs )
    0x19, 0x01,    //Usage Min
    0x29, 0x05,    //Usage Max
    0x91, 0x02,    //Output (Data, Variable, Absolute)
    //3 bit reserved
    0x95, 0x01,    //Report Count (1)
    0x75, 0x03,    //Report Size (3)
    0x91, 0x01,    //Output (static constant)

    // Key arrays (6 bytes)
    0x95, 0x06,     // Report Count (6)
    0x75, 0x08,     // Report Size (8)
    0x15, 0x00,     // Log Min (0)
    0x25, 0xF1,     // Log Max (241)
    0x05, 0x07,     // Usage Pg (Key Codes)
    0x19, 0x00,     // Usage Min (0)
    0x29, 0xf1,     // Usage Max (241)
    0x81, 0x00,     // Input: (Data, Array)

    0xC0,            // End Collection

    //consumer report in
    0x05, 0x0C,   // Usage Page (Consumer)
    0x09, 0x01,   // Usage (Consumer Control)
    0xA1, 0x01,   // Collection (Application)
    0x85, HID_REPORT_ID_CONSUME_CONTROL_INPUT,   //     Report Id
    0x75,0x10,     //global, report size 16 bits
    0x95,0x01,     //global, report count 1
    0x15,0x01,     //global, min  0x01
    0x26,0x8c,0x02,  //global, max  0x28c
    0x19,0x01,     //local, min   0x01
    0x2a,0x8c,0x02,  //local, max    0x28c
    0x81,0x00,     //main,  input data variable, absolute
    0xc0,        //main, end collection

};

// HID External Report Reference Descriptor for report map
_attribute_ble_data_retention_  static u16 extServiceUUID;


//////////////////////// OTA //////////////////////////////////
static const  u8 my_OtaServiceUUID[16]              = WRAPPING_BRACES(TELINK_OTA_UUID_SERVICE);
static const  u8 my_OtaUUID[16]                     = WRAPPING_BRACES(TELINK_SPP_DATA_OTA);
_attribute_ble_data_retention_  static        u8 my_OtaData                         = 0x00;
_attribute_ble_data_retention_  static        u8 my_OtaDataCCC[2] = {0,0};
static const u8  my_OtaName[] = {'O', 'T', 'A'};


// Include attribute (Battery service)
static const u16 include[3] = {BATT_PS_H, BATT_LEVEL_INPUT_CCB_H, SERVICE_UUID_BATTERY};



////////////////////// SPP ////////////////////////////////////
static const u8 TelinkSppServiceUUID[16]                = WRAPPING_BRACES(TELINK_SPP_UUID_SERVICE);
static const u8 TelinkSppDataServer2ClientUUID[16]      = WRAPPING_BRACES(TELINK_SPP_DATA_SERVER2CLIENT);
static const u8 TelinkSppDataClient2ServerUUID[16]      = WRAPPING_BRACES(TELINK_SPP_DATA_CLIENT2SERVER);


// Spp data from Server to Client characteristic variables
_attribute_ble_data_retention_  static u8 SppDataServer2ClientDataCCC[2]                = {0};
//this array will not used for sending data(directly calling HandleValueNotify API), so cut array length from 20 to 1, saving some SRAM
_attribute_ble_data_retention_  static u8 SppDataServer2ClientData[251]                     = {0};  //SppDataServer2ClientData[20]
// Spp data from Client to Server characteristic variables
//this array will not used for receiving data(data processed by Attribute Write CallBack function), so cut array length from 20 to 1, saving some SRAM
_attribute_ble_data_retention_  static u8 SppDataClient2ServerData[251]                     = {0};  //SppDataClient2ServerData[20]


//SPP data descriptor
static const u8 TelinkSPPS2CDescriptor[]                = "Telink SPP: Module->Phone";
static const u8 TelinkSPPC2SDescriptor[]                = "Telink SPP: Phone->Module";


//// GAP attribute values
static const u8 my_devNameCharVal[5] = {
    CHAR_PROP_READ,
    U16_LO(GenericAccess_DeviceName_DP_H), U16_HI(GenericAccess_DeviceName_DP_H),
    U16_LO(GATT_UUID_DEVICE_NAME), U16_HI(GATT_UUID_DEVICE_NAME)
};
static const u8 my_appearanceCharVal[5] = {
    CHAR_PROP_READ,
    U16_LO(GenericAccess_Appearance_DP_H), U16_HI(GenericAccess_Appearance_DP_H),
    U16_LO(GATT_UUID_APPEARANCE), U16_HI(GATT_UUID_APPEARANCE)
};
static const u8 my_periConnParamCharVal[5] = {
    CHAR_PROP_READ,
    U16_LO(CONN_PARAM_DP_H), U16_HI(CONN_PARAM_DP_H),
    U16_LO(GATT_UUID_PERI_CONN_PARAM), U16_HI(GATT_UUID_PERI_CONN_PARAM)
};


//// GATT attribute values
static const u8 my_serviceChangeCharVal[5] = {
    CHAR_PROP_INDICATE,
    U16_LO(GenericAttribute_ServiceChanged_DP_H), U16_HI(GenericAttribute_ServiceChanged_DP_H),
    U16_LO(GATT_UUID_SERVICE_CHANGE), U16_HI(GATT_UUID_SERVICE_CHANGE)
};


//// device Information  attribute values
static const u8 my_PnCharVal[5] = {
    CHAR_PROP_READ,
    U16_LO(DeviceInformation_pnpID_DP_H), U16_HI(DeviceInformation_pnpID_DP_H),
    U16_LO(CHARACTERISTIC_UUID_PNP_ID), U16_HI(CHARACTERISTIC_UUID_PNP_ID)
};


//// HID attribute values
static const u8 my_hidProtocolModeCharVal[5] = {
    CHAR_PROP_READ | CHAR_PROP_WRITE_WITHOUT_RSP,
    U16_LO(HID_PROTOCOL_MODE_DP_H), U16_HI(HID_PROTOCOL_MODE_DP_H),
    U16_LO(CHARACTERISTIC_UUID_HID_PROTOCOL_MODE), U16_HI(CHARACTERISTIC_UUID_HID_PROTOCOL_MODE)
};
static const u8 my_hidBootKeyInReportCharVal[5] = {
    CHAR_PROP_READ | CHAR_PROP_NOTIFY,
    U16_LO(HID_BOOT_KB_REPORT_INPUT_DP_H), U16_HI(HID_BOOT_KB_REPORT_INPUT_DP_H),
    U16_LO(CHARACTERISTIC_UUID_HID_BOOT_KEY_INPUT), U16_HI(CHARACTERISTIC_UUID_HID_BOOT_KEY_INPUT)
};
static const u8 my_hidBootKeyOutReportCharVal[5] = {
    CHAR_PROP_READ | CHAR_PROP_WRITE | CHAR_PROP_WRITE_WITHOUT_RSP,
    U16_LO(HID_BOOT_KB_REPORT_OUTPUT_DP_H), U16_HI(HID_BOOT_KB_REPORT_OUTPUT_DP_H),
    U16_LO(CHARACTERISTIC_UUID_HID_BOOT_KEY_OUTPUT), U16_HI(CHARACTERISTIC_UUID_HID_BOOT_KEY_OUTPUT)
};
static const u8 my_hidReportCCinCharVal[5] = {
    CHAR_PROP_READ | CHAR_PROP_NOTIFY,
    U16_LO(HID_CONSUME_REPORT_INPUT_DP_H), U16_HI(HID_CONSUME_REPORT_INPUT_DP_H),
    U16_LO(CHARACTERISTIC_UUID_HID_REPORT), U16_HI(CHARACTERISTIC_UUID_HID_REPORT)
};
static const u8 my_hidReportKEYinCharVal[5] = {
    CHAR_PROP_READ | CHAR_PROP_NOTIFY,
    U16_LO(HID_NORMAL_KB_REPORT_INPUT_DP_H), U16_HI(HID_NORMAL_KB_REPORT_INPUT_DP_H),
    U16_LO(CHARACTERISTIC_UUID_HID_REPORT), U16_HI(CHARACTERISTIC_UUID_HID_REPORT)
};
static const u8 my_hidReportKEYoutCharVal[5] = {
    CHAR_PROP_READ | CHAR_PROP_WRITE | CHAR_PROP_WRITE_WITHOUT_RSP,
    U16_LO(HID_NORMAL_KB_REPORT_OUTPUT_DP_H), U16_HI(HID_NORMAL_KB_REPORT_OUTPUT_DP_H),
    U16_LO(CHARACTERISTIC_UUID_HID_REPORT), U16_HI(CHARACTERISTIC_UUID_HID_REPORT)
};
static const u8 my_hidReportMapCharVal[5] = {
    CHAR_PROP_READ,
    U16_LO(HID_REPORT_MAP_DP_H), U16_HI(HID_REPORT_MAP_DP_H),
    U16_LO(CHARACTERISTIC_UUID_HID_REPORT_MAP), U16_HI(CHARACTERISTIC_UUID_HID_REPORT_MAP)
};
static const u8 my_hidInformationCharVal[5] = {
    CHAR_PROP_READ,
    U16_LO(HID_INFORMATION_DP_H), U16_HI(HID_INFORMATION_DP_H),
    U16_LO(CHARACTERISTIC_UUID_HID_INFORMATION), U16_HI(CHARACTERISTIC_UUID_HID_INFORMATION)
};
static const u8 my_hidCtrlPointCharVal[5] = {
    CHAR_PROP_WRITE_WITHOUT_RSP,
    U16_LO(HID_CONTROL_POINT_DP_H), U16_HI(HID_CONTROL_POINT_DP_H),
    U16_LO(CHARACTERISTIC_UUID_HID_CONTROL_POINT), U16_HI(CHARACTERISTIC_UUID_HID_CONTROL_POINT)
};


//// Battery attribute values
static const u8 my_batCharVal[5] = {
    CHAR_PROP_READ | CHAR_PROP_NOTIFY,
    U16_LO(BATT_LEVEL_INPUT_DP_H), U16_HI(BATT_LEVEL_INPUT_DP_H),
    U16_LO(CHARACTERISTIC_UUID_BATTERY_LEVEL), U16_HI(CHARACTERISTIC_UUID_BATTERY_LEVEL)
};


//// Telink spp  attribute values
static const u8 TelinkSppDataServer2ClientCharVal[19] = {
    CHAR_PROP_READ | CHAR_PROP_NOTIFY,
    U16_LO(SPP_SERVER_TO_CLIENT_DP_H), U16_HI(SPP_SERVER_TO_CLIENT_DP_H),
    TELINK_SPP_DATA_SERVER2CLIENT
};
static const u8 TelinkSppDataClient2ServerCharVal[19] = {
    CHAR_PROP_READ | CHAR_PROP_WRITE_WITHOUT_RSP,
    U16_LO(SPP_CLIENT_TO_SERVER_DP_H), U16_HI(SPP_CLIENT_TO_SERVER_DP_H),
    TELINK_SPP_DATA_CLIENT2SERVER
};

//// OTA attribute values
static const u8 my_OtaCharVal[19] = {
    CHAR_PROP_READ | CHAR_PROP_WRITE_WITHOUT_RSP | CHAR_PROP_NOTIFY,
    U16_LO(OTA_CMD_OUT_DP_H), U16_HI(OTA_CMD_OUT_DP_H),
    TELINK_SPP_DATA_OTA,
};


int spp_onReceiveData(u16 connHandle, ble_rf_packet_att_write_t *p)
{
    if(spp_write_cb){
        spp_write_cb(connHandle, p);
    }
    return 0;
}

// TM : to modify
static const attribute_t my_Attributes[] = {

    {ATT_END_H - 1, 0 , 0, 0, NULL, NULL, NULL, NULL},  // total num of attribute


    // 0001 - 0007  gap
    {7,ATT_PERMISSIONS_READ,2,2,(u8*)(size_t)(&my_primaryServiceUUID),  (u8*)(size_t)(&my_gapServiceUUID), 0, 0},
    {0,ATT_PERMISSIONS_READ,2,sizeof(my_devNameCharVal),(u8*)(size_t)(&my_characterUUID), (u8*)(size_t)(my_devNameCharVal), 0, 0},
    {0,ATT_PERMISSIONS_READ,2,sizeof(my_devName), (u8*)(size_t)(&my_devNameUUID), (u8*)(size_t)(my_devName), 0, 0},
    {0,ATT_PERMISSIONS_READ,2,sizeof(my_appearanceCharVal),(u8*)(size_t)(&my_characterUUID), (u8*)(size_t)(my_appearanceCharVal), 0, 0},
    {0,ATT_PERMISSIONS_READ,2,sizeof (my_appearance), (u8*)(size_t)(&my_appearanceUUID),    (u8*)(size_t)(&my_appearance), 0, 0},
    {0,ATT_PERMISSIONS_READ,2,sizeof(my_periConnParamCharVal),(u8*)(size_t)(&my_characterUUID), (u8*)(size_t)(my_periConnParamCharVal), 0, 0},
    {0,ATT_PERMISSIONS_READ,2,sizeof (my_periConnParameters),(u8*)(size_t)(&my_periConnParamUUID),  (u8*)(size_t)(&my_periConnParameters), 0, 0},


    // 0008 - 000b gatt
    {4,ATT_PERMISSIONS_READ,2,2,(u8*)(size_t)(&my_primaryServiceUUID),  (u8*)(size_t)(&my_gattServiceUUID), 0, 0},
    {0,ATT_PERMISSIONS_READ,2,sizeof(my_serviceChangeCharVal),(u8*)(size_t)(&my_characterUUID), (u8*)(size_t)(my_serviceChangeCharVal), 0, 0},
    {0,ATT_PERMISSIONS_READ,2,sizeof (serviceChangeVal), (u8*)(size_t)(&serviceChangeUUID), (u8*)(&serviceChangeVal), 0, 0},
    {0,ATT_PERMISSIONS_RDWR,2,sizeof (serviceChangeCCC),(u8*)(size_t)(&clientCharacterCfgUUID), (u8*)(serviceChangeCCC), 0, 0},


    // 000c - 000e  device Information Service
    {3,ATT_PERMISSIONS_READ,2,2,(u8*)(size_t)(&my_primaryServiceUUID),  (u8*)(size_t)(&my_devServiceUUID), 0, 0},
    {0,ATT_PERMISSIONS_READ,2,sizeof(my_PnCharVal),(u8*)(size_t)(&my_characterUUID), (u8*)(size_t)(my_PnCharVal), 0, 0},
    {0,ATT_PERMISSIONS_READ,2,sizeof(my_PnPtrs),(u8*)(size_t)(&my_PnPUUID), (u8*)(size_t)(my_PnPtrs), 0, 0},


    /////////////////////////////////// 4. HID Service /////////////////////////////////////////////////////////
    // 000f
    //{27, ATT_PERMISSIONS_READ,2,2,(u8*)(size_t)(&my_primaryServiceUUID),  (u8*)(size_t)(&my_hidServiceUUID), 0, 0},
    {HID_CONTROL_POINT_DP_H - HID_PS_H + 1, ATT_PERMISSIONS_READ,2,2,(u8*)(size_t)(&my_primaryServiceUUID),     (u8*)(size_t)(&my_hidServiceUUID), 0, 0},

    // 0010  include battery service
    {0,ATT_PERMISSIONS_READ,2,sizeof(include),(u8*)(size_t)(&hidIncludeUUID), (u8*)(size_t)(include), 0, 0},

    // 0011 - 0012  protocol mode
    {0,ATT_PERMISSIONS_READ,2,sizeof(my_hidProtocolModeCharVal),(u8*)(size_t)(&my_characterUUID), (u8*)(size_t)(my_hidProtocolModeCharVal), 0, 0},              //prop
    {0,ATT_PERMISSIONS_RDWR,2, sizeof(protocolMode),(u8*)(size_t)(&hidProtocolModeUUID),    (u8*)(&protocolMode), 0, 0},    //value

    // 0013 - 0015  boot keyboard input report (char-val-client)
    {0,ATT_PERMISSIONS_READ,2,sizeof(my_hidBootKeyInReportCharVal),(u8*)(size_t)(&my_characterUUID), (u8*)(size_t)(my_hidBootKeyInReportCharVal), 0, 0},                //prop
    {0,ATT_PERMISSIONS_READ,2,sizeof(bootKeyInReport),(u8*)(size_t)(&hidBootKeyInReportUUID),   (u8*)(&bootKeyInReport), 0, 0}, //value
    {0,ATT_PERMISSIONS_RDWR,2,sizeof(bootKeyInReportCCC),(u8*)(size_t)(&clientCharacterCfgUUID),    (u8*)(bootKeyInReportCCC), 0, 0},   //value

    // 0016 - 0017   boot keyboard output report (char-val)
    {0,ATT_PERMISSIONS_READ,2,sizeof(my_hidBootKeyOutReportCharVal),(u8*)(size_t)(&my_characterUUID), (u8*)(size_t)(my_hidBootKeyOutReportCharVal), 0, 0},              //prop
    {0,ATT_PERMISSIONS_RDWR,2, sizeof(bootKeyOutReport), (u8*)(size_t)(&hidBootKeyOutReportUUID),   (u8*)(&bootKeyOutReport), 0, 0},    //value


    // 0018 - 001b. consume report in: 4 (char-val-client-ref)
    {0,ATT_PERMISSIONS_READ,2,sizeof(my_hidReportCCinCharVal),(u8*)(size_t)(&my_characterUUID), (u8*)(size_t)(my_hidReportCCinCharVal), 0, 0},              //prop
    {0,ATT_PERMISSIONS_READ,2, sizeof(reportConsumerControlIn),(u8*)(size_t)(&hidReportUUID),   (u8*)(reportConsumerControlIn), 0, 0},  //value
    {0,ATT_PERMISSIONS_RDWR,2,sizeof(reportConsumerControlInCCC),(u8*)(size_t)(&clientCharacterCfgUUID),    (u8*)(reportConsumerControlInCCC), 0, 0},   //value
    {0,ATT_PERMISSIONS_RDWR,2,sizeof(reportRefConsumerControlIn),(u8*)(size_t)(&reportRefUUID),     (u8*)(reportRefConsumerControlIn), 0, 0},   //value

    // 001c - 001f . keyboard report in : 4 (char-val-client-ref)
    {0,ATT_PERMISSIONS_READ,2,sizeof(my_hidReportKEYinCharVal),(u8*)(size_t)(&my_characterUUID), (u8*)(size_t)(my_hidReportKEYinCharVal), 0, 0},                //prop
    {0,ATT_PERMISSIONS_READ,2, sizeof(reportKeyIn),(u8*)(size_t)(&hidReportUUID),   (u8*)(reportKeyIn), 0, 0},  //value
    {0,ATT_PERMISSIONS_RDWR,2,sizeof(reportKeyInCCC),(u8*)(size_t)(&clientCharacterCfgUUID),    (u8*)(reportKeyInCCC), 0, 0},   //value
    {0,ATT_PERMISSIONS_RDWR,2,sizeof(reportRefKeyIn),(u8*)(size_t)(&reportRefUUID),     (u8*)(reportRefKeyIn), 0, 0},   //value

    // 0020 - 0022 . keyboard report out: 3 (char-val-ref)
    {0,ATT_PERMISSIONS_READ,2,sizeof(my_hidReportKEYoutCharVal),(u8*)(size_t)(&my_characterUUID), (u8*)(size_t)(my_hidReportKEYoutCharVal), 0, 0},              //prop
    {0,ATT_PERMISSIONS_RDWR,2,sizeof(reportKeyOut),(u8*)(size_t)(&hidReportUUID),   (u8*)(reportKeyOut), 0, 0}, //value
    {0,ATT_PERMISSIONS_RDWR,2,sizeof(reportRefKeyOut),(u8*)(size_t)(&reportRefUUID),    (u8*)(reportRefKeyOut), 0, 0},  //value


    // 0023 - 0025 . report map: 3
    {0,ATT_PERMISSIONS_READ,2,sizeof(my_hidReportMapCharVal),(u8*)(size_t)(&my_characterUUID), (u8*)(size_t)(my_hidReportMapCharVal), 0, 0},                //prop
    {0,ATT_PERMISSIONS_READ,2,sizeof(reportMap),(u8*)(size_t)(&hidReportMapUUID),   (u8*)(size_t)(reportMap), 0, 0},    //value
    {0,ATT_PERMISSIONS_RDWR,2,sizeof(extServiceUUID),(u8*)(size_t)(&extReportRefUUID),  (u8*)(&extServiceUUID), 0, 0},  //value

    // 0026 - 0027 . hid information: 2
    {0,ATT_PERMISSIONS_READ,2,sizeof(my_hidInformationCharVal),(u8*)(size_t)(&my_characterUUID), (u8*)(size_t)(my_hidInformationCharVal), 0, 0},                //prop
    {0,ATT_PERMISSIONS_READ,2, sizeof(hidInformation),(u8*)(size_t)(&hidInformationUUID),   (u8*)(size_t)(hidInformation), 0, 0},   //value

    // 0028 - 0029 . control point: 2
    {0,ATT_PERMISSIONS_READ,2,sizeof(my_hidCtrlPointCharVal),(u8*)(size_t)(&my_characterUUID), (u8*)(size_t)(my_hidCtrlPointCharVal), 0, 0},                //prop
    {0,ATT_PERMISSIONS_WRITE,2, sizeof(controlPoint),(u8*)(size_t)(&hidCtrlPointUUID),  (u8*)(&controlPoint), 0, 0},    //value

    ////////////////////////////////////// Battery Service /////////////////////////////////////////////////////
    // 002a - 002d
    {4,ATT_PERMISSIONS_READ,2,2,(u8*)(size_t)(&my_primaryServiceUUID),  (u8*)(size_t)(&my_batServiceUUID), 0, 0},
    {0,ATT_PERMISSIONS_READ,2,sizeof(my_batCharVal),(u8*)(size_t)(&my_characterUUID), (u8*)(size_t)(my_batCharVal), 0, 0},              //prop
    {0,ATT_PERMISSIONS_READ,2,sizeof(my_batVal),(u8*)(size_t)(&my_batCharUUID),     (u8*)(my_batVal), 0, 0},    //value
    {0,ATT_PERMISSIONS_RDWR,2,sizeof(batteryValueInCCC),(u8*)(size_t)(&clientCharacterCfgUUID),     (u8*)(batteryValueInCCC), 0, 0},    //value


    // 002e - 0035 SPP for data test
    {8,ATT_PERMISSIONS_READ,2,16,(u8*)(size_t)(&my_primaryServiceUUID),     (u8*)(size_t)(&TelinkSppServiceUUID), 0, 0},
    {0,ATT_PERMISSIONS_READ,2,sizeof(TelinkSppDataServer2ClientCharVal),(u8*)(size_t)(&my_characterUUID),       (u8*)(size_t)(TelinkSppDataServer2ClientCharVal), 0, 0},                //prop
    {0,ATT_PERMISSIONS_READ,16,sizeof(SppDataServer2ClientData),(u8*)(size_t)(&TelinkSppDataServer2ClientUUID), (u8*)(SppDataServer2ClientData), 0, 0}, //value
    {0,ATT_PERMISSIONS_RDWR,2,2,(u8*)(size_t)&clientCharacterCfgUUID,(u8*)(&SppDataServer2ClientDataCCC), 0, 0},
    {0,ATT_PERMISSIONS_READ,2,sizeof(TelinkSPPS2CDescriptor),(u8*)(size_t)&userdesc_UUID,(u8*)(size_t)(&TelinkSPPS2CDescriptor), 0, 0},
    {0,ATT_PERMISSIONS_READ,2,sizeof(TelinkSppDataClient2ServerCharVal),(u8*)(size_t)(&my_characterUUID),       (u8*)(size_t)(TelinkSppDataClient2ServerCharVal), 0, 0},                //prop
    {0,ATT_PERMISSIONS_RDWR,16,sizeof(SppDataClient2ServerData),(u8*)(size_t)(&TelinkSppDataClient2ServerUUID), (u8*)(SppDataClient2ServerData), (att_readwrite_callback_t)&spp_onReceiveData, 0},  //value
    {0,ATT_PERMISSIONS_READ,2,sizeof(TelinkSPPC2SDescriptor),(u8*)(size_t)&userdesc_UUID,(u8*)(size_t)(&TelinkSPPC2SDescriptor), 0, 0},


    ////////////////////////////////////// OTA /////////////////////////////////////////////////////
    // 0036 - 0039
    {5,ATT_PERMISSIONS_READ, 2,16,(u8*)(size_t)(&my_primaryServiceUUID),    (u8*)(size_t)(&my_OtaServiceUUID), 0, 0},
    {0,ATT_PERMISSIONS_READ, 2, sizeof(my_OtaCharVal),(u8*)(size_t)(&my_characterUUID), (u8*)(size_t)(my_OtaCharVal), 0, 0},                //prop
    {0,ATT_PERMISSIONS_RDWR,16,sizeof(my_OtaData),(u8*)(size_t)(&my_OtaUUID),   (&my_OtaData), &otaWrite, NULL},            //value
    {0,ATT_PERMISSIONS_RDWR,2,sizeof(my_OtaDataCCC),(u8*)(size_t)(&clientCharacterCfgUUID),     (u8*)(my_OtaDataCCC), 0, 0},    //value
    {0,ATT_PERMISSIONS_READ, 2,sizeof (my_OtaName),(u8*)(size_t)(&userdesc_UUID), (u8*)(size_t)(my_OtaName), 0, 0},

};

/**
 * @brief   GATT initialization.
 *          !!!Note: this function is used to register ATT table to BLE Stack.
 * @param   none.
 * @return  none.
 */
void    my_gatt_init (void)
{
    bls_att_setAttributeTable ((u8 *)(size_t)my_Attributes);
}


u8 read_by_type_req_uuid[16] = {};
u8 read_by_type_req_uuidLen;


void app_gatt_storeLatestReadByTypeReq_uuid(u8 *uuid, u8 uuid_len)
{
    if(uuid_len < 17){
        read_by_type_req_uuidLen = uuid_len;
        memcpy(read_by_type_req_uuid, uuid, uuid_len);
    }
}

////////////////////////////////////////// central-role ATT concerned ///////////////////////////////////////////////

/**
 * @brief       This function is used to send consumer HID report by USB.
 * @param[in]   conn     - connection handle
 * @param[in]   p        - Pointer point to data buffer.
 * @return
 */
void    att_keyboard_media (u16 conn, u8 *p)
{
    (void)conn;
    u16 consumer_key = p[0] | p[1]<<8;


#if (1 && UI_LED_ENABLE)    //Demo effect: when peripheral send Vol+/Vol- to central, LED GPIO toggle to show the result
    if(consumer_key == MKEY_VOL_UP){
        gpio_toggle(GPIO_LED_GREEN);
    }
    else if(consumer_key == MKEY_VOL_DN){
        gpio_toggle(GPIO_LED_BLUE);
    }
#endif
}


//////////////// keyboard ///////////////////////////////////////////////////
/**
 * @brief       This function is used to send HID report by USB.
 * @param[in]   conn     - connection handle
 * @param[in]   p        - Pointer point to data buffer.
 * @return
 */
void    att_keyboard (u16 conn, u8 *p)
{
    (void)conn;(void)p;
}


#endif  //end of APP_DEFAULT_HID_BATTERY_OTA_ATTRIBUTE_TABLE
