/********************************************************************************************************
 * @file    app_ap.c
 *
 * @brief   This is the source file for BLE SDK
 *
 * @author  BLE GROUP
 * @date    12,2023
 *
 * @par     Copyright (c) 2023, Telink Semiconductor (Shanghai) Co., Ltd. ("TELINK")
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
#include "app_parse_char.h"
#include "app_ota_client.h"
#include "app_ap.h"
#include <strings.h>

#define DEFAULT_GROUP_ID            0
#define MAX_SCAN_DEVICES            0x20
#define BDADDR_STR_LEN              17
#define MAX_NUMBER_DISPLAYS         32
#define MAX_NUMBER_SENSORS          32
#define MAX_NUMBER_LEDS             32
#define PRELOAD_IMAGE_SIZE          0x2000

typedef struct {
    u8 cmdBuf[MAX_ESL_PAYLOAD_SIZE - 1];
    u16 cmdLength;
    u16 cmdNum;
} app_ap_pendingEslCmd_t;

typedef struct {
    u32 tick;
    u16 secRemaining;
} app_ap_secTimer_t;

typedef struct {
    bool active : 1;
    bool connected : 1;
    bool advertising : 1;
    bool eslConfigured : 1;
    u8 addrType;
    u8 addr[6];
    u16 connHandle;
    blc_esls_eslAddress_t address;
} app_ap_eslInfo_t;

typedef struct {
    bool active : 1;
    bool name_reported : 1;
    u8 addr[6];
    u8 addr_type;
} app_ap_scanDevice_t;

typedef struct {
    app_ap_scanDevice_t scanDevices[MAX_SCAN_DEVICES];
    bool any : 1;
    bool on : 1;
} app_ap_scanState_t;

static app_ap_pendingEslCmd_t pendingEslCmds[ESLP_AP_MAX_GROUPS];

extern int central_smp_pending;
static blc_esls_keyMaterial_t defaultKeyMaterial = {
    .sessionKey = {0x19, 0x6a, 0x0a, 0xd1, 0x2a, 0x61, 0x20, 0x1e, 0x13, 0x6e, 0x2e, 0xd1, 0x12, 0xda, 0xa9, 0x57},
    .IV = {0x9e, 0x7a, 0x00, 0xef, 0xb1, 0x7a, 0xe7, 0x46},
};

static const accessPointParam_t apParams = {
        .eslscParam = NULL,
        .apSyncKey = &defaultKeyMaterial,
};

static app_ap_eslInfo_t devices[ACL_CENTRAL_MAX_NUM + ACL_PERIPHR_MAX_NUM + ESLP_AP_ESL_RECORDS];
static u32 currentTime;
static u32 currentTimeTick;
static app_ap_scanState_t scanState;
static u8 preload_image[PRELOAD_IMAGE_SIZE];

static app_ap_eslInfo_t *newEslInfo(u8 *addr, u8 addrType, u16 connHandle)
{
    foreach_arr(i, devices) {
        if (!devices[i].active) {
            memset(&devices[i], 0, sizeof(devices[i]));
            devices[i].active = true;
            memcpy(devices[i].addr, addr, sizeof(devices[i].addr));
            devices[i].addrType = addrType;
            devices[i].connHandle = connHandle;
            return &devices[i];
        } else {
            continue;
        }
    }

    return NULL;
}

static app_ap_eslInfo_t *getEslInfoByAddr(u8 *addr, u8 addrType)
{
    foreach_arr(i, devices) {
        if (devices[i].active && !memcmp(devices[i].addr, addr, sizeof(devices[i].addr)) && addrType == devices[i].addrType) {
            return &devices[i];
        } else {
            continue;
        }
    }

    return NULL;
}

static app_ap_eslInfo_t *getEslInfoByConnHandle(u16 connHandle)
{
    foreach_arr(i, devices) {
        if (devices[i].active && devices[i].connHandle == connHandle && devices[i].connected) {
            return &devices[i];
        } else {
            continue;
        }
    }

    return NULL;
}

static app_ap_eslInfo_t *getEslInfoByEslAddress(blc_esls_eslAddress_t *address)
{
    foreach_arr(i, devices) {
        if (devices[i].active && devices[i].eslConfigured && devices[i].address.eslId == address->eslId && devices[i].address.groupId == address->groupId) {
            return &devices[i];
        } else {
            continue;
        }
    }

    return NULL;
}

static u16 getEslInfoCnt(void)
{
    u16 cnt = 0;

    foreach_arr(i, devices) {
        cnt += devices[i].active ? 1 : 0;
    }

    return cnt;
}

static u16 str2hex(char * ps, u8 *data)
{
    u8 n = 0;
    int i = 0;
    u8 *ptr = data;

    while (ps[i]) {
        u8 c = ps[i];

        if (c>='A' && c<='F') {
            c = c - 'A' + 10;
        } else if (c>='a' && c<='f') {
            c = c - 'a' + 10;
        } else if (c>='0' && c<='9' ) {
            c -= '0';
        } else {
            c = 0;
        }

        n += c * ((i & 1) ? 1 : 16);
        if ((i & 1)) {
            *ptr = n;
            ptr++;
            n = 0;
        }
        i++;
    }

    return ptr - data;
}

static bool app_ap_parse_bdaddr(const char *str, u8 *addr)
{
    u8 pos = 0;

    if (strlen(str) != BDADDR_STR_LEN) {
        return false;
    }

    for (u8 i = 0; i < 6; i++) {
        char temp[3] = {str[pos], str[pos + 1], 0};

        str2hex(temp, &addr[i]);
        pos += 2;
        if (i < 5 && str[pos++] != ':') {
            return false;
        }
    }

    return true;
}

static void conn_v2_fun(char *argv[], int argc, void *user_data)
{
    (void)user_data;
    blc_esls_eslAddress_t address;
    app_ap_eslInfo_t *eslInfo;
    ble_sts_t status;

    if (argc < 1) {
        app_parse_printf("conn_v2 [group_id] <esl_id>\r\n");
        return;
    }

    if (argc == 1) {
        address.groupId = DEFAULT_GROUP_ID;
        address.eslId = app_parse_str2n(argv[0]);
    } else {
        address.groupId = app_parse_str2n(argv[0]);
        address.eslId = app_parse_str2n(argv[1]);
    }

    eslInfo = getEslInfoByEslAddress(&address);
    if (!eslInfo) {
        app_parse_printf("No ESL device found [group_id:%d esl_id:%d]\r\n", address.groupId, address.eslId);
        return;
    }

    eslInfo->advertising = false;

    status = blc_ll_extended_createConnection_v2(ADV_HANDLE0, eslInfo->address.groupId, /* adv_handle, subevent */
            INITIATE_FP_ADV_SPECIFY, OWN_ADDRESS_PUBLIC, eslInfo->addrType, eslInfo->addr, INIT_PHY_1M, /* init_phys */
            0, 0, CONN_INTERVAL_7P5MS, CONN_INTERVAL_7P5MS, CONN_TIMEOUT_4S, /* scanInter_0, scanWindow_0, conn_min_0, conn_max_0, timeout_0 */
            0, 0, 0, 0, 0, /* scanInter_1, scanWindow_1, conn_min_1, conn_max_1, timeout_1 */
            0, 0, 0, 0, 0  /* scanInter_2, scanWindow_2, conn_min_2, conn_max_2, timeout_2 */ );

    app_parse_printf("Create Connection [group_id:%d esl_id:%d] status:%02X\r\n", address.groupId, address.eslId, status);
}

static u8 cmd_parse(char *argv[], int argc, int *cur_argc, u8 *cmdBuf, blc_esls_eslAddress_t *address)
{
    blc_eslss_controlPointCommandHdr_t *cmd = (blc_eslss_controlPointCommandHdr_t *) cmdBuf;
    char *cmd_name = argv[*cur_argc];

    if ((argc - *cur_argc) < 2) {
        goto help;
    }

    (*cur_argc)++;

    if (!strcasecmp(cmd_name, "ping")) {
        cmd->opcode = BLC_ESLSS_CONTROL_POINT_COMMAND_OPCODE_PING;
    } else if (!strcasecmp(cmd_name, "upd_cmpl")) {
        cmd->opcode = BLC_ESLSS_CONTROL_POINT_COMMAND_OPCODE_UPDATE_COMPLETE;
    } else if (!strcasecmp(cmd_name, "read_sensor")) {
        blc_eslss_controlPointCommandReadSensorData_t *cmd_read_sensor = (blc_eslss_controlPointCommandReadSensorData_t *)cmdBuf;

        cmd_read_sensor->hdr.opcode = BLC_ESLSS_CONTROL_POINT_COMMAND_OPCODE_READ_SENSOR_DATA;
        cmd_read_sensor->sensorId = app_parse_str2n(argv[(*cur_argc)++]);
    } else if (!strcasecmp(cmd_name, "refresh_display")) {
        blc_eslss_controlPointCommandRefreshDisplay_t *cmd_refresh_display = (blc_eslss_controlPointCommandRefreshDisplay_t *)cmd;

        cmd_refresh_display->hdr.opcode = BLC_ESLSS_CONTROL_POINT_COMMAND_OPCODE_REFRESH_DISPLAY;
        cmd_refresh_display->displayId = app_parse_str2n(argv[(*cur_argc)++]);
    } else if (!strcasecmp(cmd_name, "display_image")) {
        blc_eslss_controlPointCommandDisplayImage_t *cmd_display_image = (blc_eslss_controlPointCommandDisplayImage_t *)cmd;

        if ((argc - *cur_argc) < 3) {
            goto help;
        }

        cmd_display_image->hdr.opcode = BLC_ESLSS_CONTROL_POINT_COMMAND_OPCODE_DISPLAY_IMAGE;
        cmd_display_image->displayId = app_parse_str2n(argv[(*cur_argc)++]);
        cmd_display_image->imageId = app_parse_str2n(argv[(*cur_argc)++]);
    } else if (!strcasecmp(cmd_name, "display_timed_image")) {
        blc_eslss_controlPointCommandDisplayTimedImage_t *cmd_display_image = (blc_eslss_controlPointCommandDisplayTimedImage_t *)cmd;

        if ((argc - *cur_argc) < 4) {
            goto help;
        }

        cmd_display_image->hdr.opcode = BLC_ESLSS_CONTROL_POINT_COMMAND_OPCODE_DISPLAY_TIMED_IMAGE;
        cmd_display_image->absoluteTime = app_parse_str2n(argv[(*cur_argc)++]);
        cmd_display_image->displayId = app_parse_str2n(argv[(*cur_argc)++]);
        cmd_display_image->imageId = app_parse_str2n(argv[(*cur_argc)++]);
    } else if (!strcasecmp(cmd_name, "led_control")) {
        blc_eslss_controlPointCommandLedControl_t *cmd_led_control = (blc_eslss_controlPointCommandLedControl_t *)cmd;

        if ((argc - *cur_argc) < 9) {
            goto help;
        }

        cmd_led_control->hdr.opcode = BLC_ESLSS_CONTROL_POINT_COMMAND_OPCODE_LED_CONTROL;
        cmd_led_control->ledId = app_parse_str2n(argv[(*cur_argc)++]);
        cmd_led_control->colorRed = app_parse_str2n(argv[(*cur_argc)++]);
        cmd_led_control->colorGreen = app_parse_str2n(argv[(*cur_argc)++]);
        cmd_led_control->colorBlue = app_parse_str2n(argv[(*cur_argc)++]);
        cmd_led_control->brightness = app_parse_str2n(argv[(*cur_argc)++]);
        if (strlen(argv[(*cur_argc)]) > (sizeof(cmd_led_control->flashingPattern) * 2)) {
            goto help;
        }

        str2hex(argv[(*cur_argc)++], cmd_led_control->flashingPattern);
        cmd_led_control->repeatType = app_parse_str2n(argv[(*cur_argc)++]);
        cmd_led_control->repeatDuration = app_parse_str2n(argv[(*cur_argc)++]);
    } else if (!strcasecmp(cmd_name, "led_timed_control")) {
        blc_eslss_controlPointCommandLedTimedControl_t *cmd_led_control = (blc_eslss_controlPointCommandLedTimedControl_t *)cmd;

        if ((argc - *cur_argc) < 10) {
            goto help;
        }

        cmd_led_control->hdr.opcode = BLC_ESLSS_CONTROL_POINT_COMMAND_OPCODE_LED_TIMED_CONTROL;
        cmd_led_control->absoluteTime = app_parse_str2n(argv[(*cur_argc)++]);
        cmd_led_control->ledId = app_parse_str2n(argv[(*cur_argc)++]);
        cmd_led_control->colorRed = app_parse_str2n(argv[(*cur_argc)++]);
        cmd_led_control->colorGreen = app_parse_str2n(argv[(*cur_argc)++]);
        cmd_led_control->colorBlue = app_parse_str2n(argv[(*cur_argc)++]);
        cmd_led_control->brightness = app_parse_str2n(argv[(*cur_argc)++]);
        if (strlen(argv[(*cur_argc)]) > (sizeof(cmd_led_control->flashingPattern) * 2)) {
            goto help;
        }

        str2hex(argv[(*cur_argc)++], cmd_led_control->flashingPattern);
        cmd_led_control->repeatType = app_parse_str2n(argv[(*cur_argc)++]);
        cmd_led_control->repeatDuration = app_parse_str2n(argv[(*cur_argc)++]);
    } else if (!strcasecmp(argv[0], "factory_reset")) {
        cmd->opcode = BLC_ESLSS_CONTROL_POINT_COMMAND_OPCODE_FACTORY_RESET;
    } else if (!strcasecmp(argv[0], "unassociate")) {
        cmd->opcode = BLC_ESLSS_CONTROL_POINT_COMMAND_OPCODE_UNASSOCIATE_FROM_AP;
    } else if (!strcasecmp(argv[0], "vendor")) {
        blc_eslss_controlPointCommandVendorSpecific_t *cmd_vendor = (blc_eslss_controlPointCommandVendorSpecific_t *)cmd;
        u16 parsed;

        if (strlen(argv[(*cur_argc)]) > (0x0F * 2)) {
            goto help;
        }

        parsed = str2hex(argv[(*cur_argc)++], cmd_vendor->parameters);
        cmd_vendor->hdr.opcode = (BLC_ESLSS_CONTROL_POINT_COMMAND_OPCODE_VENDOR_0 | ((parsed) << 4));
    } else {
        goto help;
    }

    if (argc <= *cur_argc) {
        goto help;
    } else if (argc == ((*cur_argc) + 1)) {
        address->groupId = DEFAULT_GROUP_ID;
        address->eslId = app_parse_str2n(argv[*cur_argc]);
    } else {
        address->groupId = app_parse_str2n(argv[(*cur_argc)++]);
        address->eslId = app_parse_str2n(argv[*cur_argc]);
    }

    cmd->eslId = address->eslId;

    return blc_esl_getCommandSize(cmd);

help:
    app_parse_printf("<ping|upd_cmpl|read_sensor <sensor_id>|"\
            "refresh_display <display_id>|display_image <display_id> <image_id>|"\
            "led_control <led_id> <color_red> <color_green> <color_blue> <brightness>"\
            "<pattern> <repeat_type> <repeat_duration>> [group_id] <esl_id>\r\n");

    return 0;
}

static void cmd_fun(char *argv[], int argc, void *user_data)
{
    (void)user_data;
    u8 cmdBuf[BLC_ESLS_CMD_RSP_MAX_LENGTH];
    blc_esls_eslAddress_t address;
    app_ap_eslInfo_t *eslInfo;
    int cur_argc = 0;
    ble_sts_t status;

    if (!cmd_parse(argv, argc, &cur_argc, cmdBuf, &address)) {
        return;
    }

    eslInfo = getEslInfoByEslAddress(&address);
    if (!eslInfo) {
        app_parse_printf("No ESL [group_id:%d esl_id:%d] found\r\n", address.groupId, address.eslId);
        return;
    } else if (!eslInfo->connected) {
        app_parse_printf("ESL [group_id:%d esl_id:%d] not connected\r\n", address.groupId, address.eslId);
        return;
    }

//  status = blc_eslsc_writeControlPointNoRsp(eslInfo->connHandle, cmd);
    status = blc_eslp_ap_writeControlPointNoRsp(eslInfo->connHandle, (blc_eslss_controlPointCommandHdr_t *) cmdBuf);
    if (((blc_eslss_controlPointCommandHdr_t *) cmdBuf)->opcode == BLC_ESLSS_CONTROL_POINT_COMMAND_OPCODE_UPDATE_COMPLETE && status == BLE_SUCCESS) {
        blc_ll_periodicAdvSetInfoTransfer(eslInfo->connHandle, 0xff, ADV_HANDLE0);
    }

    app_parse_printf("Command %s sent [group_id:%d esl_id:%d] status:%02X\r\n", argv[0], address.groupId, address.eslId, status);
}

static void cmd_pawr_append_fun(char *argv[], int argc, void *user_data)
{
    (void)user_data;
    u8 cmdBuf[BLC_ESLS_CMD_RSP_MAX_LENGTH];
    blc_eslss_controlPointCommandHdr_t *cmd = (blc_eslss_controlPointCommandHdr_t *) cmdBuf;
    blc_esls_eslAddress_t address;
    int cur_argc = 0;
    u16 cmd_length;

    if (!cmd_parse(argv, argc, &cur_argc, cmdBuf, &address)) {
        return;
    }


    if (address.eslId != BLC_ESLS_ESL_ID_BROADCAST && !getEslInfoByEslAddress(&address)) {
        app_parse_printf("No ESL device found [group_id:%d esl_id:%d]\r\n", address.groupId, address.eslId);
        return;
    }

    cmd->eslId = address.eslId;
    cmd_length = blc_esl_getCommandSize(cmd);
    if (pendingEslCmds[address.groupId].cmdLength + cmd_length > sizeof(pendingEslCmds[address.groupId].cmdBuf)) {
        app_parse_printf("Failed to append PaWR command [group_id:%d esl_id:%d]\r\n", address.groupId, address.eslId);
        return;
    }

    memcpy(&pendingEslCmds[address.groupId].cmdBuf[pendingEslCmds[address.groupId].cmdLength], cmdBuf, cmd_length);
    pendingEslCmds[address.groupId].cmdLength += cmd_length;
    pendingEslCmds[address.groupId].cmdNum++;

    app_parse_printf("PaWR command %s append success [group_id:%d esl_id:%d]\r\n", argv[0], address.groupId, address.eslId);
}

static void cmd_pawr_clear_fun(char *argv[], int argc, void *user_data)
{
    (void)user_data;
    u8 groupId;

    if (argc < 1) {
        groupId = DEFAULT_GROUP_ID;
    } else {
        groupId = app_parse_str2n(argv[0]);
    }

    if (groupId >= ARRAY_SIZE(pendingEslCmds)) {
        app_parse_printf("No group_id:%d found\r\n", groupId);
        return;
    }

    pendingEslCmds[groupId].cmdLength = 0;
    pendingEslCmds[groupId].cmdNum = 0;

    app_parse_printf("PaWR command clear success [group_id:%d]\r\n", groupId);
}

static void cmd_pawr_send_fun(char *argv[], int argc, void *user_data)
{
    (void)user_data;
    ble_sts_t status;
    u8 groupId;

    if (argc < 1) {
        groupId = DEFAULT_GROUP_ID;
    } else {
        groupId = app_parse_str2n(argv[0]);
    }

    if (groupId >= ARRAY_SIZE(pendingEslCmds)) {
        app_parse_printf("No group_id:%d found\r\n", groupId);
        return;
    } else if (!pendingEslCmds[groupId].cmdNum) {
        app_parse_printf("No commands in group_id:%d\r\n", groupId);
        return;
    }

    status = blc_eslp_ap_writePawrCommand(groupId, pendingEslCmds[groupId].cmdNum, (blc_eslss_controlPointCommandHdr_t *) pendingEslCmds[groupId].cmdBuf);
    if (status == BLE_SUCCESS) {
        pendingEslCmds[groupId].cmdNum = 0;
        pendingEslCmds[groupId].cmdLength = 0;
    }

    app_parse_printf("PaWR command send %s [group_id:%d]\r\n", status == BLE_SUCCESS ? "success" : "failed", groupId);
}

static void app_ap_scanDevicesClear(void)
{
    foreach_arr(i, scanState.scanDevices) {
        scanState.scanDevices[i].active = false;
    }
}

static app_ap_scanDevice_t *app_ap_scanDevicesFind(u8 *addr, u8 addr_type)
{
    foreach_arr(i, scanState.scanDevices) {
        if (scanState.scanDevices[i].active &&
                !memcmp(addr, scanState.scanDevices[i].addr, sizeof(scanState.scanDevices[i].addr)) &&
                addr_type == scanState.scanDevices[i].addr_type) {
            return &scanState.scanDevices[i];
        }
    }

    return NULL;
}

static void app_ap_scanDevicesAdd(u8 *addr, u8 addr_type, u8 *name, u8 name_length)
{
    app_ap_scanDevice_t *dev = NULL;
    u8 i = 0;

    dev = app_ap_scanDevicesFind(addr, addr_type);
    if (dev && (!name || dev->name_reported)) {
        return;
    }

    if (!dev) {
        foreach_arr(j, scanState.scanDevices) {
            if (!scanState.scanDevices[j].active) {
                dev = &scanState.scanDevices[j];
                dev->active = true;
                memcpy(dev->addr, addr, sizeof(dev->addr));
                dev->addr_type = addr_type;
                break;
            }
        }
    }

    if (!dev) {
        goto done;
    }

    dev->name_reported = !!name;
    i = (((u32) dev - (u32) scanState.scanDevices) / sizeof(*dev)) + 1;

done:
    if (name) {
        app_parse_printf("[%d] Device found addr: %02X:%02X:%02X:%02X:%02X:%02X addr_type:%02X name:%.*s\r\n",
                    i, addr[0], addr[1], addr[2], addr[3], addr[4], addr[5], addr_type,
                    name_length, name);
    } else {
        app_parse_printf("[%d] Device found addr: %02X:%02X:%02X:%02X:%02X:%02X addr_type:%02X\r\n",
                    i, addr[0], addr[1], addr[2], addr[3], addr[4], addr[5], addr_type);
    }
}

static void cmd_scan(char *argv[], int argc, void *user_data)
{
    (void)user_data;
    bool any = false;
    int argc_on_off = 0;

    if (argc < 1) {
        goto help;
    }

    if (argc > 1) {
        if (!strcasecmp(argv[0], "any")) {
            any = true;
        } else {
            goto help;
        }

        argc_on_off++;
    }

    if (!strcasecmp(argv[argc_on_off], "on")) {
        scanState.on = true;
        app_ap_scanDevicesClear();
    } else if (!strcasecmp(argv[argc_on_off], "off")) {
        scanState.on = false;
    } else {
        goto help;
    }

    scanState.any = any;

    app_parse_printf("Scan %s\r\n", scanState.on ? "on" : "off");

    return;

help:
    app_parse_printf("scan [any] <on|off>\r\n");
}

static void cmd_conn(char *argv[], int argc, void *user_data)
{
    (void)user_data;
    u8 addr_type = PEERATYPE_PUBLIC_DEVICE_ADDRESS;
    app_ap_eslInfo_t *eslInfo;
    int argc_addr = 0;
    ble_sts_t status;
    u8 addr[6];

    if (argc < 1) {
        goto help;
    }

    if (argc > 1) {
        addr_type = app_parse_str2n(argv[argc_addr++]);
    }

    if (!app_ap_parse_bdaddr(argv[argc_addr], addr)) {
        goto help;
    }

    eslInfo = getEslInfoByAddr(addr, addr_type);
    if (eslInfo) {
        eslInfo->advertising = false;
    }

    status = blc_ll_extended_createConnection( INITIATE_FP_ADV_SPECIFY, OWN_ADDRESS_PUBLIC, addr_type, addr, INIT_PHY_1M, \
                                               SCAN_INTERVAL_100MS, SCAN_WINDOW_100MS, CONN_INTERVAL_100MS, CONN_INTERVAL_100MS, CONN_TIMEOUT_4S, \
                                               0, 0, 0, 0, 0, \
                                               0, 0, 0, 0, 0);

    app_parse_printf("Connect %02X:%02X:%02X:%02X:%02X:%02X status:%02X\r\n", addr[0], addr[1],addr[2],addr[3],addr[4],addr[5], status);

    return;

help:
    app_parse_printf("conn [addr_type] <bdaddr>\r\n");
}

static void cmd_disc(char *argv[], int argc, void *user_data)
{
    (void)user_data;
    ble_sts_t status;
    u16 connHandle;

    if (argc < 1) {
        app_parse_printf("disc <connHandle>\r\n");
        return;
    }

    connHandle = app_parse_str2n(argv[0]);

    status = blc_ll_disconnect(connHandle, HCI_ERR_REMOTE_USER_TERM_CONN);
    app_parse_printf("Disconnect %d status:%02X\r\n", connHandle, status);
}

static void help_fun(char *argv[], int argc, void *user_data);

static void cmd_add_esl(char *argv[], int argc, void *user_data)
{
    (void)user_data;
    blc_esls_eslAddress_t address = {
        .groupId = DEFAULT_GROUP_ID,
    };
    app_ap_eslInfo_t *eslInfo;
    int cur_argc = 0;
    u16 connHandle;

    if (argc < 2) {
        goto help;
    }

    connHandle = app_parse_str2n(argv[cur_argc++]);
    eslInfo = getEslInfoByConnHandle(connHandle);
    if (!eslInfo) {
        app_parse_printf("Add ESL failed - invalid connHandle:%d\r\n", connHandle);
        return;
    } else if (eslInfo->eslConfigured) {
        app_parse_printf("Add ESL already done connHandle:%d\r\n", connHandle);
        return;
    }

    if (argc > 2) {
        address.groupId = app_parse_str2n(argv[cur_argc++]);
    }

    address.eslId = app_parse_str2n(argv[cur_argc]);

    if (blc_eslp_ap_addEsl(&address, &defaultKeyMaterial) == BLE_SUCCESS) {
        eslInfo->eslConfigured = true;
        eslInfo->address = address;
        app_parse_printf("Add ESL success [group_id:%d esl_id:%d] connHandle:%d\r\n",
                        eslInfo->address.groupId, eslInfo->address.eslId, connHandle);
    } else {
        app_parse_printf("Add ESL failed connHandle:%d\r\n", connHandle);
    }

    return;
help:
    app_parse_printf("add_esl <connHandle> [groupId] <eslId>\r\n");
}

static void cmd_rm_esl(char *argv[], int argc, void *user_data)
{
    (void)user_data;
    blc_esls_eslAddress_t address;
    app_ap_eslInfo_t *eslInfo;
    int cur_argc = 0;

    if (argc < 1) {
        goto help;
    }

    address.groupId = argc > 1 ? app_parse_str2n(argv[cur_argc++]) : DEFAULT_GROUP_ID;
    address.eslId = app_parse_str2n(argv[cur_argc]);

    eslInfo = getEslInfoByEslAddress(&address);
    if (!eslInfo) {
        app_parse_printf("Remove ESL failed - no ESL found [group_id:%d esl_id:%d]\r\n",
                address.groupId, address.eslId);
        return;
    }

    blc_eslp_ap_removeEsl(&address);
    eslInfo->eslConfigured = false;
    if (!eslInfo->connected) {
        eslInfo->active = false;
    }

    app_parse_printf("Remove ESL success [group_id:%d esl_id:%d]\r\n",
            address.groupId, address.eslId);

    return;
help:
    app_parse_printf("rm_esl [groupId] <eslId>\r\n");
}

static void eslCurrentTimeWriteCb(u16 connHandle, att_err_t err)
{
    app_ap_eslInfo_t *eslInfo = getEslInfoByConnHandle(connHandle);
    ble_sts_t status = BLE_SUCCESS;

    if (!eslInfo || !eslInfo->eslConfigured) {
        return;
    }

    if (err == ATT_SUCCESS) {
        app_parse_printf("ESL configure complete [group_id:%d esl_id:%d] success\r\n",
                                eslInfo->address.groupId, eslInfo->address.eslId, status);
    } else {
        app_parse_printf("ESL configure complete [group_id:%d esl_id:%d] failed (%d)\r\n",
                                eslInfo->address.groupId, eslInfo->address.eslId, err);
    }
}

static void eslResponseKeyWriteCb(u16 connHandle, att_err_t err)
{
    app_ap_eslInfo_t *eslInfo = getEslInfoByConnHandle(connHandle);
    ble_sts_t status;

    if (!eslInfo || !eslInfo->eslConfigured) {
        return;
    }

    if (err == ATT_SUCCESS) {
        status = blc_eslp_ap_writeEslCurrentAbsolutTime(connHandle, currentTime, eslCurrentTimeWriteCb);
        if (status != BLE_SUCCESS) {
            app_parse_printf("ESL configure complete [group_id:%d esl_id:%d] failed (%d)\r\n",
                                    eslInfo->address.groupId, eslInfo->address.eslId, status);
        }
    } else {
        app_parse_printf("ESL configure complete [group_id:%d esl_id:%d] failed (%d)\r\n",
                                eslInfo->address.groupId, eslInfo->address.eslId, err);
    }
}

static void apSyncKeyWriteCb(u16 connHandle, att_err_t err)
{
    app_ap_eslInfo_t *eslInfo = getEslInfoByConnHandle(connHandle);
    ble_sts_t status;

    if (!eslInfo || !eslInfo->eslConfigured) {
        return;
    }

    if (err == ATT_SUCCESS) {
        status = blc_eslp_ap_writeEslResponseKeyMaterial(connHandle, &eslInfo->address, eslResponseKeyWriteCb);
        if (status != BLE_SUCCESS) {
            app_parse_printf("ESL configure complete [group_id:%d esl_id:%d] failed (%d)\r\n",
                                    eslInfo->address.groupId, eslInfo->address.eslId, status);
        }
    } else {
        app_parse_printf("ESL configure complete [group_id:%d esl_id:%d] failed (%d)\r\n",
                                eslInfo->address.groupId, eslInfo->address.eslId, err);
    }
}

static void eslAddressWriteCb(u16 connHandle, att_err_t err)
{
    app_ap_eslInfo_t *eslInfo = getEslInfoByConnHandle(connHandle);
    ble_sts_t status;

    if (!eslInfo || !eslInfo->eslConfigured) {
        return;
    }

    if (err == ATT_SUCCESS) {
        status = blc_eslp_ap_writeApSyncKeyMaterial(connHandle, apSyncKeyWriteCb);
        if (status != BLE_SUCCESS) {
            app_parse_printf("ESL configure complete [group_id:%d esl_id:%d] failed (%d)\r\n",
                                    eslInfo->address.groupId, eslInfo->address.eslId, status);
        }
    } else {
        app_parse_printf("ESL configure complete [group_id:%d esl_id:%d] failed (%d)\r\n",
                                eslInfo->address.groupId, eslInfo->address.eslId, err);
    }
}

static void cmd_esl_configure(char *argv[], int argc, void *user_data)
{
    (void)user_data;
    blc_esls_eslAddress_t address;
    app_ap_eslInfo_t *eslInfo;
    int cur_argc = 0;

    if (argc < 1) {
        goto help;
    }

    address.groupId = argc > 1 ? app_parse_str2n(argv[cur_argc++]) : DEFAULT_GROUP_ID;
    address.eslId = app_parse_str2n(argv[cur_argc]);
    eslInfo = getEslInfoByEslAddress(&address);
    if (!eslInfo) {
        app_parse_printf("ESL configure [group_id:%d esl_id:%d] failed - invalid ESL address\r\n",
                            address.groupId, address.eslId);
    } else if (!eslInfo->connected) {
        app_parse_printf("ESL configure [group_id:%d esl_id:%d] failed - not connected\r\n",
                            address.groupId, address.eslId);
    } else {
        ble_sts_t status = blc_eslp_ap_writeEslAddress(eslInfo->connHandle, &eslInfo->address, eslAddressWriteCb);

        if (status == BLE_SUCCESS) {
            app_parse_printf("ESL configure [group_id:%d esl_id:%d] success\r\n",
                                address.groupId, address.eslId);
        } else {
            app_parse_printf("ESL configure [group_id:%d esl_id:%d] failed status:%02X\r\n",
                                address.groupId, address.eslId, status);
        }
    }

    return;

help:
    app_parse_printf("esl_configure [groupId] <eslId>\r\n");
}

static void dumpEslInfo(app_ap_eslInfo_t *eslInfo, u16 i, u16 cnt)
{
    char buf[80];
    int ret;

    ret = snprintf(buf, sizeof(buf), "[%d/%d] %02X:%02X:%02X:%02X:%02X:%02X (%d)", i, cnt,
                    eslInfo->addr[0], eslInfo->addr[1], eslInfo->addr[2],
                    eslInfo->addr[3], eslInfo->addr[4], eslInfo->addr[5], eslInfo->addrType);

    if (eslInfo->eslConfigured) {
        ret += snprintf(buf + ret, sizeof(buf) - ret, " [group_id:%d esl_id:%d]", eslInfo->address.groupId, eslInfo->address.eslId);
    }

    if (eslInfo->connected) {
        ret += snprintf(buf + ret, sizeof(buf) - ret, " conn_h:%d", eslInfo->connHandle);
    }

    if (eslInfo->advertising) {
        ret += snprintf(buf + ret, sizeof(buf) - ret, " adv");
    }

    app_parse_printf("%s\r\n", buf);
}

static void cmd_devs(char *argv[], int argc, void *user_data)
{
    (void)argv;
    (void)argc;
    (void)user_data;
    u16 devicesCnt = getEslInfoCnt();
    u16 cnt = 1;

    if (!devicesCnt) {
        app_parse_printf("No cached devices\r\n");
        return;
    }

    foreach_arr(i, devices) {
        if (!devices[i].active) {
            continue;
        }

        dumpEslInfo(&devices[i], cnt, devicesCnt);
        cnt++;
    }
}

static void cmd_ots_get(char *argv[], int argc, void *user_data)
{
    (void)user_data;
    u16 conn_handle;
    ble_sts_t status;

    if (argc < 2) {
        app_parse_printf("ots_get <conn_handle> <feat|name|type|size|first_created|last_modified|"\
                "obj_id|props|list_filter_0|list_filter_1|list_filter_2>\r\n");
        return;
    }

    conn_handle = app_parse_str2n(argv[0]);

    if (!strcasecmp(argv[1], "feat")) {
        blc_ots_feature_t feature;

        status = blc_otsc_getOtsFeature(conn_handle, &feature);
        if (status != BLE_SUCCESS) {
            goto failed;
        }

        app_parse_printf("OTS feature conn_handle:%d oacp:%08X olcp:%08X\r\n",
                conn_handle, feature.oacpFeatures, feature.olcpFeatures);
    } else if (!strcasecmp(argv[1], "name")) {
        u8 name[OTS_CLIENT_OBJECT_NAME_MAX_SIZE];
        u16 name_length;

        status = blc_otsc_getObjectName(conn_handle, name, &name_length);
        if (status != BLE_SUCCESS) {
            goto failed;
        }

        app_parse_printf("OTS conn_handle:%d object name:%.*s\r\n", conn_handle, name_length, name);
    } else if (!strcasecmp(argv[1], "type")) {
        uuid_t type;

        status = blc_otsc_getObjectType(conn_handle, &type);
        if (status != BLE_SUCCESS) {
            goto failed;
        }

        if (type.uuidLen == ATT_16_UUID_LEN) {
            app_parse_printf("OTS conn_handle:%d obj type (16bit):%04X\r\n", conn_handle, type.uuidVal.u16);
        } else {
            app_parse_printf("OTS conn_handle:%d obj type (128bit):%02X%02X%02X%02X%02X%02X%02X"\
                    "%02X%02X%02X%02X%02X%02X%02X%02X%02X\r\n", conn_handle, type.uuidVal.u128[0], type.uuidVal.u128[1],
                    type.uuidVal.u128[2], type.uuidVal.u128[3], type.uuidVal.u128[4], type.uuidVal.u128[5],
                    type.uuidVal.u128[6], type.uuidVal.u128[7], type.uuidVal.u128[8], type.uuidVal.u128[9],
                    type.uuidVal.u128[10], type.uuidVal.u128[11], type.uuidVal.u128[12], type.uuidVal.u128[13],
                    type.uuidVal.u128[14], type.uuidVal.u128[15]);
        }
    } else if (!strcasecmp(argv[1], "size")) {
        blc_ots_object_size_t size;

        status = blc_otsc_getObjectSize(conn_handle, &size);
        if (status != BLE_SUCCESS) {
            goto failed;
        }

        app_parse_printf("OTS conn_handle:%d obj current size:%08X allocated size:%08X\r\n", conn_handle, size.currentSize, size.allocatedSize);
    } else if (!strcasecmp(argv[1], "first_created")) {
        blc_ots_utc_t time;

        status = blc_otsc_getObjectFirstCreated(conn_handle, &time);
        if (status != BLE_SUCCESS) {
            goto failed;
        }

        app_parse_printf("OTS conn_handle:%d Object First Created:%04d-%02d-%02dT%02d:%02d:%02d\r\n", conn_handle,
                time.year, time.month, time.day, time.hour, time.minute, time.second);
    } else if (!strcasecmp(argv[1], "last_modified")) {
        blc_ots_utc_t time;

        status = blc_otsc_getObjectLastModified(conn_handle, &time);
        if (status != BLE_SUCCESS) {
            goto failed;
        }

        app_parse_printf("OTS conn_handle:%d Object Last Modified:%04d-%02d-%02dT%02d:%02d:%02d\r\n", conn_handle,
                time.year, time.month, time.day, time.hour, time.minute, time.second);
    } else if (!strcasecmp(argv[1], "id")) {
        blc_ots_object_id_t obj_id;
        u64 u;

        status = blc_otsc_getObjectId(conn_handle, &obj_id);
        if (status != BLE_SUCCESS) {
            goto failed;
        }

        u = bstream_to_u48_le(obj_id.objectId);
        app_parse_printf("OTS conn_handle:%d Object Id:%llu\r\n", conn_handle, u);
    } else if (!strcasecmp(argv[1], "props")) {
        u32 properties;

        status = blc_otsc_getObjectProperties(conn_handle, &properties);
        if (status != BLE_SUCCESS) {
            goto failed;
        }

        app_parse_printf("OTS conn_handle:%d Object Props:%lu\r\n", conn_handle, properties);
    } else if (!strcasecmp(argv[1], "list_filter_0") ||
                !strcasecmp(argv[1], "list_filter_1") ||
                !strcasecmp(argv[1], "list_filter_2")) {
        u8 buf[OTS_CLIENT_OBJECT_FILTER_MAX_SIZE];
        blc_ots_object_filter_hdr_t *hdr = (blc_ots_object_filter_hdr_t *) buf;
        u8 id;

        if (!strcasecmp(argv[1], "list_filter_0")) {
            id = 0;
        } else if (!strcasecmp(argv[1], "list_filter_1")) {
            id = 1;
        } else {
            id = 2;
        }

        status = blc_otsc_getObjectListFilter(conn_handle, id, hdr);
        if (status != BLE_SUCCESS) {
            goto failed;
        }

        app_parse_printf("OTS conn_handle:%d filter id:%d filter:%d", conn_handle, id, hdr->filter);

        switch (hdr->filter) {
        case BLC_OTS_FILTER_NAME_STARTS_WITH:
        case BLC_OTS_FILTER_NAME_ENDS_WITH:
        case BLC_OTS_FILTER_NAME_CONTAINS:
        case BLC_OTS_FILTER_NAME_IS_EXACTLY:
        {
            blc_ots_object_filter_name_starts_with_t *name = (blc_ots_object_filter_name_starts_with_t *) hdr;

            app_parse_printf(" param:%.*s\r\n", name->length, name->data);

            break;
        }
        case BLC_OTS_FILTER_OBJECT_TYPE:
        {
            blc_ots_object_filter_object_type_t *type = (blc_ots_object_filter_object_type_t *) hdr;

            if (type->type.uuidLen == ATT_16_UUID_LEN) {
                app_parse_printf(" type (16bit):%04X\r\n", conn_handle, type->type.uuidVal.u16);
            } else {
                app_parse_printf(" type (128bit):%02X%02X%02X%02X%02X%02X%02X"\
                        "%02X%02X%02X%02X%02X%02X%02X%02X%02X\r\n", conn_handle, type->type.uuidVal.u128[0], type->type.uuidVal.u128[1],
                        type->type.uuidVal.u128[2], type->type.uuidVal.u128[3], type->type.uuidVal.u128[4], type->type.uuidVal.u128[5],
                        type->type.uuidVal.u128[6], type->type.uuidVal.u128[7], type->type.uuidVal.u128[8], type->type.uuidVal.u128[9],
                        type->type.uuidVal.u128[10], type->type.uuidVal.u128[11], type->type.uuidVal.u128[12], type->type.uuidVal.u128[13],
                        type->type.uuidVal.u128[14], type->type.uuidVal.u128[15]);
            }

            break;
        }
        case BLC_OTS_FILTER_CREATED_BETWEEN:
        case BLC_OTS_FILTER_MODIFIED_BETWEEN:
        {
            blc_ots_object_filter_created_between_t *between = (blc_ots_object_filter_created_between_t *) hdr;

            app_parse_printf(" timestamp1:%04d-%02d-%02dT%02d:%02d:%02d timestamp2:%04d-%02d-%02dT%02d:%02d:%02d\r\n",
                            between->timestamp1.year, between->timestamp1.month, between->timestamp1.day,
                            between->timestamp1.hour, between->timestamp1.minute, between->timestamp1.second,
                            between->timestamp2.year, between->timestamp2.month, between->timestamp2.day,
                            between->timestamp2.hour, between->timestamp2.minute, between->timestamp2.second);
            break;
        }
        case BLC_OTS_FILTER_CURRENT_SIZE_BETWEEN:
        case BLC_OTS_FILTER_ALLOCATED_SIZE_BETWEEN:
        {
            blc_ots_object_filter_current_size_between_t *size = (blc_ots_object_filter_current_size_between_t *) hdr;

            app_parse_printf(" size1:%d size2:%d", size->size1, size->size2);

            break;
        }
        case BLC_OTS_FILTER_MARKED_OBJECTS:
        case BLC_OTS_FILTER_NO_FILTER:
        default:
            app_parse_printf("\r\n");
            break;
        }
    } else {
        app_parse_printf("ots_get <conn_handle> <feat|name|type|size|first_created|last_modified|"\
                "obj_id|props|list_filter_0|list_filter_1|list_filter_2>\r\n");
    }

    return;

failed:
    app_parse_printf("ots_get conn_handle: %d failed:%d\r\n", conn_handle, status);
}

static void ots_write_cb(u16 connHandle, att_err_t err)
{
    app_parse_printf("ots_write_cb conn_handle:%d err:0x%02X\r\n", connHandle, err);
}

static bool str2utc(int argc, char **argv, blc_ots_utc_t *utc)
{
    if (argc < 6) {
        return false;
    }

    utc->year = app_parse_str2n(argv[0]);
    utc->month = app_parse_str2n(argv[1]);
    utc->day = app_parse_str2n(argv[2]);
    utc->hour = app_parse_str2n(argv[3]);
    utc->minute = app_parse_str2n(argv[4]);
    utc->second = app_parse_str2n(argv[5]);

    return false;
}

static void ots_read_cb(u16 connHandle, att_err_t err)
{
    app_parse_printf("ots_read_cb conn_handle:%d status:0x%02X\r\n", connHandle, err);
}

static void cmd_ots_read(char *argv[], int argc, void *user_data)
{
    (void)user_data;
    ble_sts_t status;
    u16 conn_handle;

    if (argc < 2) {
        goto failed;
    }

    conn_handle = app_parse_str2n(argv[0]);

    if (!strcasecmp(argv[1], "feat")) {
        status = blc_otsc_readOtsFeature(conn_handle, ots_read_cb);
    } else if (!strcasecmp(argv[1], "name")) {
        status = blc_otsc_readObjectName(conn_handle, ots_read_cb);
    } else if (!strcasecmp(argv[1], "type")) {
        status = blc_otsc_readObjectType(conn_handle, ots_read_cb);
    } else if (!strcasecmp(argv[1], "size")) {
        status = blc_otsc_readObjectSize(conn_handle, ots_read_cb);
    } else if (!strcasecmp(argv[1], "first_created")) {
        status = blc_otsc_readObjectFirstCreated(conn_handle, ots_read_cb);
    } else if (!strcasecmp(argv[1], "last_modified")) {
        status = blc_otsc_readObjectLastModified(conn_handle, ots_read_cb);
    } else if (!strcasecmp(argv[1], "id")) {
        status = blc_otsc_readObjectId(conn_handle, ots_read_cb);
    } else if (!strcasecmp(argv[1], "props")) {
        status = blc_otsc_readObjectProperties(conn_handle, ots_read_cb);
    } else if (!strcasecmp(argv[1], "list_filter_0")) {
        status = blc_otsc_readObjectListFilter(conn_handle, 0, ots_read_cb);
    } else if (!strcasecmp(argv[1], "list_filter_1")) {
        status = blc_otsc_readObjectListFilter(conn_handle, 1, ots_read_cb);
    } else if (!strcasecmp(argv[1], "list_filter_2")) {
        status = blc_otsc_readObjectListFilter(conn_handle, 2, ots_read_cb);
    } else {
        goto failed;
    }

    app_parse_printf("ots read %s conn_handle:%d status:0x%02X\r\n", argv[1], conn_handle, status);

    return;

failed:
    app_parse_printf("ots_read <conn_handle> <feat|name|type|size|props|id|first_created|last_modified|list_filter_<0|1|2>>\r\n");
}

static void cmd_ots_write(char *argv[], int argc, void *user_data)
{
    (void)user_data;
    ble_sts_t status;
    u16 conn_handle;

    if (argc < 2) {
        goto failed;
    }

    conn_handle = app_parse_str2n(argv[0]);

    if (!strcasecmp(argv[1], "name")) {
        if (argc < 3) {
            app_parse_printf("ots_write <conn_hande> name <name>");
            return;
        }
        status = blc_otsc_writeObjectName(conn_handle, (u8*)&argv[2], strlen(argv[2]), ots_write_cb);
    } else if (!strcasecmp(argv[1], "first_created")) {
        blc_ots_utc_t utc;

        if (str2utc(argc - 2, &argv[2], &utc)) {
            app_parse_printf("ots_write <conn_handle> first_created <year> <month> <day> <hour> <min> <sec>\r\n");
            return;
        }

        status = blc_otsc_writeObjectFirstCreated(conn_handle, &utc, ots_write_cb);
    } else if (!strcasecmp(argv[1], "last_modified")) {
        blc_ots_utc_t utc;

        if (str2utc(argc - 2, &argv[2], &utc)) {
            app_parse_printf("ots_write <conn_handle> last_modified <year> <month> <day> <hour> <min> <sec>\r\n");
            return;
        }

        status = blc_otsc_writeObjectLastModified(conn_handle, &utc, ots_write_cb);
    } else if (!strcasecmp(argv[1], "props")) {
        u32 props = app_parse_str2n(argv[2]);

        status = blc_otsc_writeObjectProperties(conn_handle, props, ots_write_cb);
    } else if (!strcasecmp(argv[1], "list_filter_0") ||
            !strcasecmp(argv[1], "list_filter_1") ||
            !strcasecmp(argv[1], "list_filter_2")) {
        u8 buf[OTS_CLIENT_OBJECT_FILTER_MAX_SIZE];
        blc_ots_object_filter_hdr_t *hdr = (blc_ots_object_filter_hdr_t *) buf;
        u8 id;

        if (argc < 3) {
            app_parse_printf("ots_write <conn_handle> %s <0..10> [args]\r\n", argv[1]);
            return;
        }

        if (!strcasecmp(argv[1], "list_filter_0")) {
            id = 0;
        } else if (!strcasecmp(argv[1], "list_filter_1")) {
            id = 1;
        } else {
            id = 2;
        }

        hdr->filter = app_parse_str2n(argv[2]);
        switch (hdr->filter) {
        case BLC_OTS_FILTER_NO_FILTER:
        case BLC_OTS_FILTER_MARKED_OBJECTS:
            // no params
            break;
        case BLC_OTS_FILTER_NAME_STARTS_WITH:
        case BLC_OTS_FILTER_NAME_ENDS_WITH:
        case BLC_OTS_FILTER_NAME_CONTAINS:
        case BLC_OTS_FILTER_NAME_IS_EXACTLY:
        {
            blc_ots_object_filter_name_starts_with_t *name = (blc_ots_object_filter_name_starts_with_t *) hdr;

            if (argc < 4) {
                app_parse_printf("ots_write <conn_handle> %s %d <name>\r\n", argv[1], hdr->filter);
                return;
            }

            name->length = strlen(argv[3]);
            memcpy(name->data, argv[3], name->length);
            break;
        }
        case BLC_OTS_FILTER_OBJECT_TYPE:
        {
            blc_ots_object_filter_object_type_t *type = (blc_ots_object_filter_object_type_t *) hdr;

            type->type.uuidLen = str2hex(argv[3], type->type.uuidVal.u);
            if (type->type.uuidLen != ATT_16_UUID_LEN && type->type.uuidLen != ATT_128_UUID_LEN) {
                app_parse_printf("ots_write: Invalid type length: %d\r\n", type->type.uuidLen);
                return;
            }

            break;
        }
        case BLC_OTS_FILTER_CREATED_BETWEEN:
        case BLC_OTS_FILTER_MODIFIED_BETWEEN:
        {
            blc_ots_object_filter_created_between_t *between = (blc_ots_object_filter_created_between_t *) hdr;

            if (argc < 15) {
                app_parse_printf("ots_write <conn_handle> %s %d <year> <month> <day> <hour> <min> <sec>"\
                                    " <year> <month> <day> <hour> <min> <sec>\r\n", argv[1], hdr->filter);
                return;
            }

            str2utc(6, &argv[3], &between->timestamp1);
            str2utc(6, &argv[9], &between->timestamp2);

            break;
        }
        case BLC_OTS_FILTER_CURRENT_SIZE_BETWEEN:
        case BLC_OTS_FILTER_ALLOCATED_SIZE_BETWEEN:
        {
            blc_ots_object_filter_current_size_between_t *between = (blc_ots_object_filter_current_size_between_t *) hdr;

            if (argc < 5) {
                if (argc < 15) {
                    app_parse_printf("ots_write <conn_handle> %s %d <size1> <size2>\r\n", argv[1], hdr->filter);
                    return;
                }
            }

            between->size1 = app_parse_str2n(argv[3]);
            between->size2 = app_parse_str2n(argv[4]);

            break;
        }
        default:
            app_parse_printf("ots_write <conn_handle> %s <0..10> [args]\r\n", argv[1]);
            return;
        }

        status = blc_otsc_writeObjectFilterList(conn_handle, id, hdr, ots_write_cb);
    } else if (!strcasecmp(argv[1], "oacp")) {
        u8 oacp_buf[sizeof(blc_ots_oacp_create_t)];
        blc_ots_oacp_cmd_hdr_t *hdr = (blc_ots_oacp_cmd_hdr_t *) oacp_buf;

        if (argc < 3) {
            app_parse_printf("ots_write <conn_handle> %s <1..7>\r\n", argv[1]);
            return;
        }

        hdr->opcode = app_parse_str2n(argv[2]);

        switch (hdr->opcode) {
        case BLC_OTS_OACP_OPCODE_CREATE:
        {
            blc_ots_oacp_create_t *create = (blc_ots_oacp_create_t *) hdr;

            if (argc < 5) {
                app_parse_printf("ots_write <conn_handle> %s %d <size> <type>\r\n", argv[1], hdr->opcode);
                return;
            }

            create->size = app_parse_str2n(argv[3]);
            create->type.uuidLen = str2hex(argv[4], create->type.uuidVal.u);
            if (create->type.uuidLen != ATT_16_UUID_LEN && create->type.uuidLen != ATT_128_UUID_LEN) {
                app_parse_printf("ots_write: Invalid type length: %d\r\n", create->type.uuidLen);
                return;
            }

            break;
        }
        case BLC_OTS_OACP_OPCODE_DELETE:
        case BLC_OTS_OACP_OPCODE_ABORT:
            break;
        case BLC_OTS_OACP_OPCODE_CALCULATE_CHECKSUM:
        case BLC_OTS_OACP_OPCODE_READ:
        {
            blc_ots_oacp_read_t *read = (blc_ots_oacp_read_t *) hdr;

            if (argc < 5) {
                app_parse_printf("ots_write <conn_handle> %s %d <len> <offset>\r\n", argv[1], hdr->opcode);
                return;
            }

            read->len = app_parse_str2n(argv[3]);
            read->offset = app_parse_str2n(argv[4]);

            break;
        }
        case BLC_OTS_OACP_OPCODE_EXECUTE:
        {
            blc_ots_oacp_execute_t *exec = (blc_ots_oacp_execute_t *) hdr;

            if (argc > 3) {
                exec->len = str2hex(argv[3], exec->data);
            } else {
                exec->len = 0;
            }
            break;
        }
        case BLC_OTS_OACP_OPCODE_WRITE:
        {
            blc_ots_oacp_write_t *write = (blc_ots_oacp_write_t *) hdr;

            if (argc < 6) {
                app_parse_printf("ots_write <conn_handle> %s %d <len> <offset> <mode>\r\n", argv[1], hdr->opcode);
                return;
            }

            write->offset = app_parse_str2n(argv[3]);
            write->len = app_parse_str2n(argv[4]);
            write->mode = app_parse_str2n(argv[5]);
            break;
        }
        default:
            app_parse_printf("ots_write <conn_handle> %s <1..7>\r\n", argv[1]);
            return;
        }

        status = blc_otsc_writeObjectActionControlPoint(conn_handle, hdr, ots_write_cb);
    } else if (!strcasecmp(argv[1], "olcp")) {
        u8 olcp_buf[7];
        blc_ots_olcp_cmd_hdr_t *hdr = (blc_ots_olcp_cmd_hdr_t *) olcp_buf;

        if (argc < 3) {
            app_parse_printf("ots_write <conn_handle> %s [1..8]\r\n", argv[1]);
            return;
        }

        hdr->opcode = app_parse_str2n(argv[2]);
        switch (hdr->opcode) {
        case BLC_OTS_OLCP_OPCODE_FIRST:
        case BLC_OTS_OLCP_OPCODE_LAST:
        case BLC_OTS_OLCP_OPCODE_PREVIOUS:
        case BLC_OTS_OLCP_OPCODE_NEXT:
        case BLC_OTS_OLCP_OPCODE_REQUEST_NUMBER_OF_OBJECTS:
        case BLC_OTS_OLCP_OPCODE_CLEAR_MARKING:
            // no params
            break;
        case BLC_OTS_OLCP_OPCODE_GOTO:
        {
            blc_ots_olcp_goto_t *go_to = (blc_ots_olcp_goto_t *) hdr;
            u64 id;

            if (argc < 4) {
                app_parse_printf("ots_write <conn_handle> %s %d <id>\r\n", argv[1], hdr->opcode);
                return;
            }

            id = app_parse_str2n(argv[3]);
            u48_to_bstream_le(id, go_to->id.objectId);

            break;
        }
        case BLC_OTS_OLCP_OPCODE_ORDER:
        {
            blc_ots_olcp_order_t *order = (blc_ots_olcp_order_t *) hdr;

            if (argc < 4) {
                app_parse_printf("ots_write <conn_handle> %s %d <order>\r\n", argv[1], hdr->opcode);
                return;
            }

            order->order = app_parse_str2n(argv[3]);

            break;
        }
        default:
            app_parse_printf("ots_write <conn_handle> %s [1..8]\r\n", argv[1]);
            return;
        }

        status = blc_otsc_writeObjectListControlPoint(conn_handle, hdr, ots_write_cb);
    } else {
        goto failed;
    }

    app_parse_printf("ots_write %s conn_handle:%d status:0x%02X\r\n", argv[1], conn_handle, status);

    return;
failed:
    app_parse_printf("ots_write <conn_handle> <name|first_created"\
                "|last_modified|props|list_filter_0|list_filter_1|list_filter_2"\
                "|oacp|olcp> <args>>\r\n");
}

static void cmd_ots_channel_open(char *argv[], int argc, void *user_data)
{
    (void)user_data;
    ble_sts_t status;
    u16 conn_handle;

    if (argc < 1) {
        app_parse_printf("ots_chan_open <conn_handle>\r\n");
        return;
    }

    conn_handle = app_parse_str2n(argv[0]);
    status = blc_otsc_openObjectTransferChannel(conn_handle);

    app_parse_printf("OTS channel open conn_handle:%d status:0x%02X\r\n", conn_handle, status);
}

static void cmd_ots_channel_close(char *argv[], int argc, void *user_data)
{
    (void)user_data;
    ble_sts_t status;
    u16 conn_handle;

    if (argc < 1) {
        app_parse_printf("ots_chan_close <conn_handle>\r\n");
        return;
    }

    conn_handle = app_parse_str2n(argv[0]);
    status = blc_otsc_closeObjectTransferChannel(conn_handle);

    app_parse_printf("OTS channel close conn_handle:%d status:0x%02X\r\n", conn_handle, status);
}

static u8 ots_channel_send_data[STACK_PRF_ACL_CENTRAL_MAX_NUM][OTSC_L2CAP_MTU];

static void cmd_ots_channel_send(char *argv[], int argc, void *user_data)
{
    (void)user_data;
    ble_sts_t status;
    u16 conn_handle, length;
    int conn_index;

    if (argc < 2) {
        app_parse_printf("ots_chan_send <conn_handle> <data>\r\n");
        return;
    }

    conn_handle = app_parse_str2n(argv[0]);
    conn_index = blc_prf_getAclConnectIndex(conn_handle);
    if (conn_index < 0) {
        app_parse_printf("OTS channel send: invalid conn_handle:%02X\r\n");
        return;
    }

    length = str2hex(argv[1], ots_channel_send_data[conn_index]);

    status = blc_otsc_writeToObjectTransferChannel(conn_handle, length, ots_channel_send_data[conn_index]);
    app_parse_printf("OTS channel send conn_handle:%d status:0x%02X\r\n", conn_handle, status);
}

static void ots_chan_send_p(char *argv[], int argc, void *user_data)
{
    (void)user_data;
    ble_sts_t status;
    u16 conn_handle, length, offset;
    int conn_index;

    if (argc < 3) {
        app_parse_printf("ots_chan_send_p <conn_handle> <offset> <length>\r\n");
        return;
    }

    conn_handle = app_parse_str2n(argv[0]);
    conn_index = blc_prf_getAclConnectIndex(conn_handle);
    if (conn_index < 0) {
        app_parse_printf("OTS channel send: invalid conn_handle:%02X\r\n");
        return;
    }

    offset = app_parse_str2n(argv[1]);
    length = app_parse_str2n(argv[2]);
    if (offset >= sizeof(preload_image) || (offset + length) > sizeof(preload_image)) {
        app_parse_printf("OTS channel send: invalid params conn_handle:%02X\r\n");
        return;
    }

    status = blc_otsc_writeToObjectTransferChannel(conn_handle, length, &preload_image[offset]);
    app_parse_printf("OTS channel send conn_handle:%d status:0x%02X\r\n", conn_handle, status);
}

static void cmd_load_image(char *argv[], int argc, void *user_data)
{
    (void)user_data;
    u16 length, offset;

    if (argc < 2) {
        app_parse_printf("load_image <offset> <data>\r\n");
        return;
    }

    offset = app_parse_str2n(argv[0]);
    length = str2hex(argv[1], &preload_image[offset]);

    app_parse_printf("load_image offset:%d length:%d\r\n", offset, length);
}

static void cmd_pre_image_size(char *argv[], int argc, void *user_data)
{
    (void)user_data;
    (void)argv;
    (void)argc;

    app_parse_printf("Max image size:%d\r\n", sizeof(preload_image));
}

static void cmd_gatts_get(char *argv[], int argc, void *user_data)
{
    (void)user_data;
    ble_sts_t status;
    u16 conn_handle;

    if (argc < 2) {
        goto failed;
    }

    conn_handle = app_parse_str2n(argv[0]);
    if (!strcasecmp(argv[1], "database_hash")) {
        blc_gatt_database_hash_t hash;

        status = blc_gattsc_getDatabaseHash(conn_handle, &hash);
        if (status == BLE_SUCCESS) {
            app_parse_printf("gatts_get %s conn_handle:%d "\
                    "hash:%02X%02X%02X%02X%02X%02X%02X%02X"\
                    "%02X%02X%02X%02X%02X%02X%02X%02X\r\n",
                    argv[1], conn_handle, hash.hash[0], hash.hash[1],
                    hash.hash[2], hash.hash[3], hash.hash[4], hash.hash[5],
                    hash.hash[6], hash.hash[7], hash.hash[8], hash.hash[9],
                    hash.hash[10], hash.hash[11], hash.hash[12], hash.hash[13],
                    hash.hash[14], hash.hash[15]);
            return;
        }
    } else if (!strcasecmp(argv[1], "client_sup_feat")) {
        u8 features[16];
        u16 features_length;

        status = blc_gattsc_getClientSupportedFeatures(conn_handle, features, &features_length);
        if (status == BLE_SUCCESS) {
            app_parse_printf("gatts_get %s conn_handle:%d features:", argv[1], conn_handle);

            for (u16 i = 0; i < features_length; i++) {
                app_parse_printf("%02X", features[i]);
            }

            app_parse_printf("\r\n");
            return;
        }
    } else {
        goto failed;
    }

    app_parse_printf("gatts_get %s conn_handle:%d failed status:0x%02X\r\n", argv[1], conn_handle, status);
    return;

failed:
    app_parse_printf("gatts_get <conn_handle> <database_hash|client_sup_feat>\r\n");
}

static void gatts_read_cb(u16 connHandle, att_err_t err)
{
    app_parse_printf("gatts_read_cb conn_handle:%d status:0x%02X\r\n", connHandle, err);
}

static void cmd_gatts_read(char *argv[], int argc, void *user_data)
{
    (void)user_data;
    ble_sts_t status;
    u16 conn_handle;

    if (argc < 2) {
        goto failed;
    }

    conn_handle = app_parse_str2n(argv[0]);
    if (!strcasecmp(argv[1], "database_hash")) {
        status = blc_gattsc_readDatabaseHash(conn_handle, gatts_read_cb);
    } else if (!strcasecmp(argv[1], "client_sup_feat")) {
        status = blc_gattsc_readClientSupportedFeatures(conn_handle, gatts_read_cb);
    } else {
        goto failed;
    }

    app_parse_printf("gatts_read %s conn_handle:%d status:0x%02X\r\n", argv[1], conn_handle, status);
    return;

failed:
    app_parse_printf("gatts_read <conn_handle> <database_hash|client_sup_feat>\r\n");
}

static void gatts_write_cb(u16 connHandle, att_err_t err)
{
    app_parse_printf("gatts_write_cb conn_handle:%d err:0x%02X\r\n", connHandle, err);
}

static void cmd_gatts_write(char *argv[], int argc, void *user_data)
{
    (void)user_data;
    ble_sts_t status;
    u16 conn_handle;

    if (argc < 3) {
        goto failed;
    }

    conn_handle = app_parse_str2n(argv[0]);
    if (!strcasecmp(argv[1], "client_sup_feat")) {
        u8 features[16];
        u16 features_length;

        features_length = str2hex(argv[2], features);

        status = blc_gattsc_writeClientSupportedFeatures(conn_handle, features, features_length, gatts_write_cb);
    } else {
        goto failed;
    }

    app_parse_printf("gatts_write %s conn_handle:%d status:0x%02X\r\n", argv[1], conn_handle, status);
    return;

failed:
    app_parse_printf("gatts_write <conn_handle> <client_sup_feat> <data>\r\n");
}

static void app_ap_dump_led_info(u16 conn_handle)
{
    blc_esls_ledInformation_t leds[MAX_NUMBER_LEDS];
    u16 numLeds = ARRAY_SIZE(leds);

    if (blc_eslsc_getLedInformation(conn_handle, &numLeds, leds) == BLE_SUCCESS) {
        if (!numLeds) {
            app_parse_printf("No LEDs supported conn_handle:%d\r\n", conn_handle);
        } else {
            for (u16 i = 0; i < numLeds; i++) {
                app_parse_printf("LED [%d/%d] conn_handle:%d type:%d red:%d green:%d blue:%d\r\n", i + 1, numLeds,
                                    conn_handle, leds[i].type, leds[i].red, leds[i].green, leds[i].blue);
            }
        }
    } else {
        app_parse_printf("Failed to get LEDs conn_handle:%d\r\n", conn_handle);
    }
}

static void app_ap_dump_sensor_info(u16 conn_handle)
{
    blc_esls_sensorInformation_t sensors[MAX_NUMBER_SENSORS];
    u16 numSensors = ARRAY_SIZE(sensors);

    if (blc_eslsc_getSensorInformation(conn_handle, &numSensors, sensors) == BLE_SUCCESS) {
        if (!numSensors) {
            app_parse_printf("No sensors supported conn_handle:%d\r\n", conn_handle);
        } else {
            for (u16 i = 0; i < numSensors; i++) {
                if (sensors[i].size == BLC_ESLS_SENSOR_INFORMATION_SIZE_0) {
                    app_parse_printf("sensor [%d/%d] conn_handle:%d size:0 type:0x%04X\r\n",
                                        i + 1, numSensors, conn_handle, sensors[i].sensorType0);
                } else {
                    app_parse_printf("sensor [%d/%d] conn_handle:%d size:1 type:0x%08X\r\n",
                                        i + 1, numSensors, conn_handle, sensors[i].sensorType1);
                }
            }
        }
    } else {
        app_parse_printf("Failed to get sensors conn_handle:%d\r\n", conn_handle);
    }
}

static void app_ap_dump_display_info(u16 conn_handle)
{
    blc_esls_displayData_t displays[MAX_NUMBER_DISPLAYS];
    u8 numDisplays = ARRAY_SIZE(displays);

    if (blc_eslsc_getDisplayInformation(conn_handle, &numDisplays, displays) == BLE_SUCCESS) {
        if (!numDisplays) {
            app_parse_printf("No displays supported conn_handle:%d\r\n", conn_handle);
        } else {
            for (u16 i = 0; i < numDisplays; i++) {
                app_parse_printf("display [%d/%d] conn_handle:%d type:%d width:0x%04X height:0x%04X\r\n",
                        i + 1, numDisplays, conn_handle, displays[i].displayType, displays[i].width, displays[i].height);
            }
        }
    } else {
        app_parse_printf("Failed to get displays conn_handle:%d\r\n", conn_handle);
    }
}

static void app_ap_dump_image_info(u16 conn_handle)
{
    u8 imageInformation;

    if (blc_eslsc_getImageInformation(conn_handle, &imageInformation) == BLE_SUCCESS) {
        app_parse_printf("Image information conn_handle:%d image_info:0x%02X\r\n", conn_handle, imageInformation);
    } else {
        app_parse_printf("Failed to get displays conn_handle:%d\r\n", conn_handle);
    }
}

static void cmd_esls_get(char *argv[], int argc, void *user_data)
{
    (void)user_data;
    u16 conn_handle;

    if (argc < 2) {
        goto failed;
    }

    conn_handle = app_parse_str2n(argv[0]);
    if (!strcasecmp(argv[1], "led")) {
        app_ap_dump_led_info(conn_handle);
    } else if (!strcasecmp(argv[1], "sensor")) {
        app_ap_dump_sensor_info(conn_handle);
    } else if (!strcasecmp(argv[1], "disp")) {
        app_ap_dump_display_info(conn_handle);
    } else if (!strcasecmp(argv[1], "image")) {
        app_ap_dump_image_info(conn_handle);
    } else {
        goto failed;
    }

    return;

failed:
    app_parse_printf("esls_get <conn_handle> <led|sensor|disp|image>\r\n");
}

static void esls_read_cb(u16 connHandle, att_err_t err)
{
    app_parse_printf("esls_read_cb conn_handle:%d status:0x%02X\r\n", connHandle, err);
}

static void cmd_esls_read(char *argv[], int argc, void *user_data)
{
    (void)user_data;
    u16 conn_handle;
    ble_sts_t status;

    if (argc < 2) {
        goto failed;
    }

    conn_handle = app_parse_str2n(argv[0]);
    if (!strcasecmp(argv[1], "led")) {
        status = blc_eslsc_readLedInformation(conn_handle, esls_read_cb);
    } else if (!strcasecmp(argv[1], "sensor")) {
        status = blc_eslsc_readSensorInformation(conn_handle, esls_read_cb);
    } else if (!strcasecmp(argv[1], "disp")) {
        status = blc_eslsc_readDisplayInformation(conn_handle, esls_read_cb);
    } else if (!strcasecmp(argv[1], "image")) {
        status = blc_eslsc_readImageInformation(conn_handle, esls_read_cb);
    } else {
        goto failed;
    }

    app_parse_printf("esls read %s conn_handle:%d status:0x%02X\r\n", argv[1], conn_handle, status);

    return;

failed:
    app_parse_printf("esls_read <conn_handle> <led|sensor|disp|image>\r\n");
}

static void cmd_get_mtu(char *argv[], int argc, void *user_data)
{
    (void)user_data;
    u16 conn_handle, mtu;

    if (argc < 1) {
        goto failed;
    }

    conn_handle = app_parse_str2n(argv[0]);
    extern u16 blt_gap_getEffectiveMTU(u16);
    mtu = blt_gap_getEffectiveMTU(conn_handle);
    app_parse_printf("MTU get conn_handle:%d mtu:%d\r\n", conn_handle, mtu);

    return;

failed:
    app_parse_printf("get_mtu <conn_handle>\r\n");
}

static void cmd_dis_get(char *argv[], int argc, void *user_data)
{
    (void)user_data;
    u16 conn_handle;

    if (argc < 2) {
        goto failed;
    }

    conn_handle = app_parse_str2n(argv[0]);
    if (!strcasecmp(argv[1], "pnp_id")) {
        dis_pnp_t pnp;

        if (blc_disc_getPnPID(conn_handle, &pnp) == BLE_SUCCESS) {
            app_parse_printf("PnPID conn_handle:%d vidSrc:0x%02X vid:0x%04X pid:0x%04X ver:0x%04X\r\n",
                                conn_handle, pnp.vidSrc, pnp.vid, pnp.pid, pnp.ver);
        } else {
            app_parse_printf("Failed to get pnp_id conn_handle:%d\r\n", conn_handle);
        }
    } else {
        goto failed;
    }

    return;

failed:
    app_parse_printf("dis_get <conn_handle> pnp_id\r\n");
}

static void cmd_time_get(char *argv[], int argc, void *user_data)
{
    (void)argv;
    (void)argc;
    (void)user_data;
    app_parse_printf("Current time:0x%08X\r\n", currentTime);
}

static void cmd_num_group_get(char *argv[], int argc, void *user_data)
{
    (void)argv;
    (void)argc;
    (void)user_data;

    app_parse_printf("Groups number:%d\r\n", ESLP_AP_MAX_GROUPS);
}

static void cmd_version_get(char *argv[], int argc, void *user_data)
{
    (void)argv;
    (void)argc;
    (void)user_data;

    app_parse_printf("BLE ESL\r\n");
}
#if (BLE_OTA_CLIENT_ENABLE)
static void ota_result_cb(u16 conn_handle, int result)
{
    app_parse_printf("OTA result: conn_handle:%d result:%d\r\n", conn_handle, result);
}

static void cmd_start_ota(char *argv[], int argc, void *user_data)
{
    (void)user_data;
    u16 conn_handle;
    int conn_index;
    bool status;

    if (argc < 1) {
        goto failed;
    }

    conn_handle = app_parse_str2n(argv[0]);
    conn_index = blc_prf_getAclConnectIndex(conn_handle);
    if (conn_index < 0) {
        app_parse_printf("OTA start: invalid conn_handle:%d\r\n", conn_handle);
        return;
    }

    status = app_ota_start(NEW_FW_ADDR_512K, conn_handle, ota_result_cb);
    app_parse_printf("OTA start: conn_handle:%d status:%s\r\n", conn_handle, status ? "success" : "fail");

    return;

failed:
    app_parse_printf("start_ota <conn_handle>\r\n");
}
#endif
static const parse_fun_list_t app_ap_funcs[] = {
        { "help", help_fun, NULL },
        { "conn_v2", conn_v2_fun, NULL },
        { "cmd", cmd_fun, NULL },
        { "cmd_pawr_append", cmd_pawr_append_fun, NULL },
        { "cmd_pawr_clear", cmd_pawr_clear_fun, NULL },
        { "cmd_pawr_send", cmd_pawr_send_fun, NULL },
        { "scan", cmd_scan, NULL },
        { "conn", cmd_conn, NULL },
        { "devs", cmd_devs, NULL},
        { "disc", cmd_disc, NULL },
        { "add_esl", cmd_add_esl, NULL},
        { "rm_esl", cmd_rm_esl, NULL},
        { "esl_configure", cmd_esl_configure, NULL },
        { "ots_get", cmd_ots_get, NULL },
        { "ots_read", cmd_ots_read, NULL },
        { "ots_write", cmd_ots_write, NULL },
        { "ots_chan_open", cmd_ots_channel_open, NULL },
        { "ots_chan_close", cmd_ots_channel_close, NULL },
        { "ots_chan_send", cmd_ots_channel_send, NULL },
        { "ots_chan_send_p", ots_chan_send_p, NULL },
        { "gatts_get", cmd_gatts_get, NULL },
        { "gatts_read", cmd_gatts_read, NULL },
        { "gatts_write", cmd_gatts_write, NULL },
        { "esls_get", cmd_esls_get, NULL },
        { "esls_read", cmd_esls_read, NULL },
        { "dis_get", cmd_dis_get, NULL },
        { "time_get", cmd_time_get, NULL },
        { "num_group_get", cmd_num_group_get, NULL },
        { "v", cmd_version_get, NULL },
        { "load_image", cmd_load_image, NULL },
        { "pre_image_size", cmd_pre_image_size, NULL },
        { "get_mtu", cmd_get_mtu, NULL },
#if (BLE_OTA_CLIENT_ENABLE)
        { "start_ota", cmd_start_ota, NULL }
#endif
};

static void help_fun(char *argv[], int argc, void *user_data)
{
    (void)argv;
    (void)argc;
    (void)user_data;
    app_parse_printf("help:\r\n");

    foreach_arr(i, app_ap_funcs) {
        app_parse_printf("\t%s\r\n", app_ap_funcs[i].fun_name);
    }
}

void app_ap_init(void)
{
    blc_eslp_ap_registerAccessPoint(&apParams);
    blc_esl_registerOTSControlClient(NULL);
    blc_basic_registerGATTSControlClient(NULL);
    blc_basic_registerDISControlClient(NULL);
    blc_otas_registerOTASControlClient(NULL);

    app_parse_init(app_ap_funcs, ARRAY_SIZE(app_ap_funcs));
}

static bool app_ap_find_uuid(u8 *data, u8 len, u16 uuid, data_type_t type)
{
    (void)uuid;
    u8 *serviceData;
    u8 serviceDataLen;

    serviceData = blc_eslp_getAdvTypeInfo(data, len, type, &serviceDataLen);
    while (serviceDataLen >= 2) {
        u16 local_uuid;

        STREAM_TO_U16(local_uuid, serviceData);
        if (local_uuid == SERVICE_UUID_ELECTRONIC_SHELF_LABEL) {
            return true;
        }
        serviceDataLen -= 2;
    }

    return false;
}

static int app_ap_le_ext_adv_report_event_handle(u8 *p, int evt_data_len)
{
    (void)evt_data_len;
    hci_le_extAdvReportEvt_t *pExtAdvRpt = (hci_le_extAdvReportEvt_t *)p;
    int offset = 0;
    extAdvEvt_info_t *pExtAdvInfo = NULL;

    for (int i = 0; i<pExtAdvRpt->num_reports; i++)
    {
        pExtAdvInfo = (extAdvEvt_info_t *)(pExtAdvRpt->advEvtInfo + offset);
        offset += (EXTADV_INFO_LENGTH + pExtAdvInfo->data_length);
        u8 ext_evtType = pExtAdvInfo->event_type & EXTADV_RPT_EVTTYPE_MASK;
        u8 conn_adv_flag = 0;
        /* Extended ADV */

        if(ext_evtType == EXTADV_RPT_EVTTYPE_EXT_CONNECTABLE_UNDIRECTED || ext_evtType == EXTADV_RPT_EVTTYPE_EXT_CONNECTABLE_DIRECTED)
        {
            conn_adv_flag = 1;  //Extended
            /* Extended, Connectable Undirected */
        } else if (ext_evtType == EXTADV_RPT_EVTTYPE_LEGACY_ADV_IND || ext_evtType == EXTADV_RPT_EVTTYPE_LEGACY_ADV_DIRECT_IND) {
            conn_adv_flag = 2;  //Legacy
        }

        if(conn_adv_flag)
        {
            app_ap_eslInfo_t *eslInfo;

            /*********************** Central Create connection demo: Key press or ADV pair packet triggers pair  ********************/
            if(central_smp_pending )
            {    //if previous connection SMP not finish, can not create a new connection
                return 1;
            }

            eslInfo = getEslInfoByAddr(pExtAdvInfo->address, pExtAdvInfo->address_type);
            if (eslInfo) {
                if (!eslInfo->connected && !eslInfo->advertising) {
                    eslInfo->advertising = true;

                    app_parse_printf("ESL device [group_id:%d esl_id:%d] found\r\n", eslInfo->address.groupId, eslInfo->address.eslId);
                }
            } else if (scanState.on) {
                u8 *name;
                u8 name_length;

                if (!scanState.any) {
                    if (!app_ap_find_uuid(pExtAdvInfo->data, pExtAdvInfo->data_length,
                        SERVICE_UUID_ELECTRONIC_SHELF_LABEL, DT_INCOMPLETE_LIST_16BIT_SERVICE_UUID) &&
                        !app_ap_find_uuid(pExtAdvInfo->data, pExtAdvInfo->data_length,
                        SERVICE_UUID_ELECTRONIC_SHELF_LABEL, DT_COMPLETE_LIST_16BIT_SERVICE_UUID)) {
                        return 1;
                    }
                }

                name = blc_eslp_getAdvTypeInfo(pExtAdvInfo->data, pExtAdvInfo->data_length, DT_COMPLETE_LOCAL_NAME, &name_length);
                app_ap_scanDevicesAdd(pExtAdvInfo->address, pExtAdvInfo->address_type, name, name_length);
            }
        }
    }
    return 0;
}

static void app_ap_le_enhanced_connection_complete_event_handle(u8 *p)
{
    hci_le_enhancedConnCompleteEvt_t *pConnEvt = (hci_le_enhancedConnCompleteEvt_t *)p;

    tlkapi_printf(APP_LOG_EN,"[APP][EVT] le_connection_complete connHandle:%04X mac:%02X %02X %02X %02X %02X %02X",\
            pConnEvt->connHandle,pConnEvt->PeerAddr[0],pConnEvt->PeerAddr[1],pConnEvt->PeerAddr[2],\
            pConnEvt->PeerAddr[3],pConnEvt->PeerAddr[4],pConnEvt->PeerAddr[5]);

    app_ap_eslInfo_t *eslInfo = getEslInfoByAddr(pConnEvt->PeerAddr, pConnEvt->PeerAddrType);
    if (!eslInfo && pConnEvt->status == BLE_SUCCESS) {
        eslInfo = newEslInfo(pConnEvt->PeerAddr, pConnEvt->PeerAddrType, pConnEvt->connHandle);
        if (!eslInfo) {
            app_parse_printf("Failed to allocate device info, disconnecting\r\n");
            blc_ll_disconnect(pConnEvt->connHandle, HCI_ERR_REMOTE_USER_TERM_CONN);
            return;
        }
    }

    if (pConnEvt->status == BLE_SUCCESS) {
        eslInfo->connected = true;
        eslInfo->connHandle = pConnEvt->connHandle;

        if (!eslInfo->eslConfigured) {
            app_parse_printf("Connected %02X:%02X:%02X:%02X:%02X:%02X (%d) connHandle:%d\r\n",
                            eslInfo->addr[0], eslInfo->addr[1], eslInfo->addr[2], eslInfo->addr[3],
                            eslInfo->addr[4], eslInfo->addr[5], eslInfo->addrType, eslInfo->connHandle);
        } else {
            app_parse_printf("Connected %02X:%02X:%02X:%02X:%02X:%02X (%d) connHandle:%d [group_id:%d esl_id:%d]\r\n",
                            eslInfo->addr[0], eslInfo->addr[1], eslInfo->addr[2], eslInfo->addr[3],
                            eslInfo->addr[4], eslInfo->addr[5], eslInfo->addrType, eslInfo->connHandle,
                            eslInfo->address.groupId, eslInfo->address.eslId);
        }
    } else {
        if (eslInfo && eslInfo->eslConfigured) {
            app_parse_printf("Connect failed %02X:%02X:%02X:%02X:%02X:%02X (%d) [group_id:%d esl_id:%d]\r\n",
                            eslInfo->addr[0], eslInfo->addr[1], eslInfo->addr[2], eslInfo->addr[3],
                            eslInfo->addr[4], eslInfo->addr[5], eslInfo->addrType,
                            eslInfo->address.groupId, eslInfo->address.eslId);
        } else {
            app_parse_printf("Connect failed %02X:%02X:%02X:%02X:%02X:%02X (%d)\r\n",
                            pConnEvt->PeerAddr[0], pConnEvt->PeerAddr[1], pConnEvt->PeerAddr[2], pConnEvt->PeerAddr[3],
                            pConnEvt->PeerAddr[4], pConnEvt->PeerAddr[5], pConnEvt->PeerAddrType);
        }
    }
}

static void app_ap_disconnect_event_handle(u8 *p)
{
    hci_disconnectionCompleteEvt_t*pDisConn = (hci_disconnectionCompleteEvt_t *)p;
#if (BLE_OTA_CLIENT_ENABLE)
    app_ota_connection_terminated(pDisConn->connHandle);
#endif

    app_ap_eslInfo_t *eslInfo = getEslInfoByConnHandle(pDisConn->connHandle);
    if (!eslInfo) {
        return;
    }

    eslInfo->connected = false;
    eslInfo->advertising = false;
    eslInfo->connHandle = 0;

    if (eslInfo->eslConfigured) {
        app_parse_printf("Disconnected connHandle:%d [group_id:%d esl_id:%d]\r\n",
                pDisConn->connHandle, eslInfo->address.groupId, eslInfo->address.eslId);
    } else {
        app_parse_printf("Disconnected connHandle:%d\r\n", pDisConn->connHandle);
        // Remove this entry as ESL is not configured
        eslInfo->active = false;
    }
}

void app_ap_host_event_callback (u32 h, u8 *p, int n)
{
    u8 event = h & 0xFF;

    blc_otsc_hostEventCallback(h, p, n);

    switch (event) {
    case GAP_EVT_SMP_PAIRING_SUCCESS:
    {
        gap_smp_pairingSuccessEvt_t *evt = (gap_smp_pairingSuccessEvt_t *) p;

        app_parse_printf("Pairing success conn_handle:%d bond:0x%02X result:0x%02X\r\n",
                            evt->connHandle, evt->bonding, evt->bonding_result);
        break;
    }
    case GAP_EVT_SMP_PAIRING_FAIL:
    {
        gap_smp_pairingFailEvt_t *evt = (gap_smp_pairingFailEvt_t *) p;

        app_parse_printf("Pairing failed conn_handle:%d reason:0x%02X\r\n",
                            evt->connHandle, evt->reason);
        break;
    }
    case GAP_EVT_SMP_CONN_ENCRYPTION_DONE:
    {
        gap_smp_connEncDoneEvt_t *evt = (gap_smp_connEncDoneEvt_t *) p;

        app_parse_printf("Encryption done conn_handle:%d reconnect:0x%02X\r\n",
                            evt->connHandle, evt->re_connect);
        break;
    }
    case GAP_EVT_ATT_EXCHANGE_MTU:
    {
        gap_gatt_mtuSizeExchangeEvt_t *evt = (gap_gatt_mtuSizeExchangeEvt_t *) p;

        app_parse_printf("MTU exchange conn_handle:%d peer_mtu:0x%04X eff_mtu:0x%04X\r\n",
                            evt->connHandle, evt->peer_MTU, evt->effective_MTU);
        break;
    }
    default:
        break;
    }
}

void app_ap_controller_event_callback (u32 h, u8 *p, int n)
{
    if (h &HCI_FLAG_EVENT_BT_STD)       //Controller HCI event
    {
        u8 evtCode = h & 0xff;

        //------------ disconnect -------------------------------------
        if(evtCode == HCI_EVT_DISCONNECTION_COMPLETE)  //connection terminate
        {
            app_ap_disconnect_event_handle(p);
        }
        else if(evtCode == HCI_EVT_LE_META)  //LE Event
        {
            u8 subEvt_code = p[0];

            //------hci le event: le enhanced_connection complete event---------------------------------
            if (subEvt_code == HCI_SUB_EVT_LE_ENHANCED_CONNECTION_COMPLETE)  // connection complete
            {
                app_ap_le_enhanced_connection_complete_event_handle(p);
            }
            else if (subEvt_code == HCI_SUB_EVT_LE_EXTENDED_ADVERTISING_REPORT) // ADV packet
            {
                app_ap_le_ext_adv_report_event_handle(p, n);
            }
        }
    }
}

static const char *svc_id2str(int svc_id)
{
    switch (svc_id) {
    case ESL_ESLS_CLIENT:
        return "ESLS";
    case ESL_OTS_CLIENT:
        return "OTS";
    case GATT_SERVICE_CLIENT:
        return "GATTS";
    case DIS_CLIENT:
        return "DIS";
    case OTAS_CLIENT:
        return "OTAS";
    default:
        break;
    }

    return "UNKNOWN";
}

static int app_ap_client_sdpFail(u16 connHandle, u8 *pData, u16 dataLen)
{
    (void)dataLen;
    blc_prf_sdpFailEvt_t* sdpFail = (blc_prf_sdpFailEvt_t*)pData;

    app_parse_printf("%s client not found connHandle:%d\r\n", svc_id2str(sdpFail->svcId), connHandle);

    return 0;
}

static int app_ap_client_sdpFound(u16 connHandle, u8 *pData, u16 dataLen)
{
    (void)dataLen;
    blc_prf_sdpFoundEvt_t* sdpFound = (blc_prf_sdpFoundEvt_t*)pData;

    app_parse_printf("%s client start connHandle:%d\r\n", svc_id2str(sdpFound->svcId), connHandle);

    return 0;
}

static int app_ap_client_sdpEnd(u16 connHandle, u8 *pData, u16 dataLen)
{
    (void)dataLen;
    blc_prf_sdpFoundEvt_t* sdpFound = (blc_prf_sdpFoundEvt_t*)pData;

    app_parse_printf("%s client found connHandle:%d\r\n", svc_id2str(sdpFound->svcId), connHandle);

    return 0;
}

static void app_ap_eslResponse(blc_esls_eslAddress_t *address, blc_eslss_controlPointResponseHdr_t *rsp, bool pawr)
{
    if (pawr) {
        app_parse_printf("PAwR ");
    }

    switch (rsp->opcode) {
    case BLC_ESLSS_CONTROL_POINT_RESPONSE_OPCODE_ERROR:
    {
        blc_eslss_controlPointResponseError_t *errRsp = (blc_eslss_controlPointResponseError_t *) rsp;
        app_parse_printf("ESL Response Error:0x%02X [group_id: %d esl_id: %d]\r\n", errRsp->error, address->groupId, address->eslId);
        break;
    }
    case BLC_ESLSS_CONTROL_POINT_RESPONSE_OPCODE_LED_STATE:
    {
        blc_eslss_controlPointResponseLedState_t *ledRsp = (blc_eslss_controlPointResponseLedState_t *) rsp;

        app_parse_printf("ESL response Led Id:0x%02X [group_id: %d esl_id: %d]\r\n", ledRsp->ledId, address->groupId, address->eslId);
        break;
    }
    case BLC_ESLSS_CONTROL_POINT_RESPONSE_OPCODE_BASIC_STATE:
    {
        blc_eslss_controlPointResponseBasicState_t *basicRsp = (blc_eslss_controlPointResponseBasicState_t *) rsp;

        app_parse_printf("ESL response Basic State [group_id: %d esl_id: %d] %01X%01X%01X%01X%01X\r\n", address->groupId, address->eslId,
                basicRsp->pendingDisplayUpdate, basicRsp->pendingLedUpdate, basicRsp->activeLed, basicRsp->synchronized, basicRsp->serviceNeeded);
        break;
    }
    case BLC_ESLSS_CONTROL_POINT_RESPONSE_OPCODE_DISPLAY_STATE:
    {
        blc_eslss_controlPointResponseDisplayState_t *dispState = (blc_eslss_controlPointResponseDisplayState_t *) rsp;

        app_parse_printf("ESL response Display State DisplayId:%d ImageId:%d [group_id: %d esl_id: %d]\r\n", dispState->displayId, dispState->imageId, address->groupId, address->eslId);

        break;
    }
    case BLC_ESLSS_CONTROL_POINT_RESPONSE_OPCODE_SENSOR_VALUE_0:
    case BLC_ESLSS_CONTROL_POINT_RESPONSE_OPCODE_SENSOR_VALUE_1:
    case BLC_ESLSS_CONTROL_POINT_RESPONSE_OPCODE_SENSOR_VALUE_2:
    case BLC_ESLSS_CONTROL_POINT_RESPONSE_OPCODE_SENSOR_VALUE_3:
    case BLC_ESLSS_CONTROL_POINT_RESPONSE_OPCODE_SENSOR_VALUE_4:
    case BLC_ESLSS_CONTROL_POINT_RESPONSE_OPCODE_SENSOR_VALUE_5:
    case BLC_ESLSS_CONTROL_POINT_RESPONSE_OPCODE_SENSOR_VALUE_6:
    case BLC_ESLSS_CONTROL_POINT_RESPONSE_OPCODE_SENSOR_VALUE_7:
    case BLC_ESLSS_CONTROL_POINT_RESPONSE_OPCODE_SENSOR_VALUE_8:
    case BLC_ESLSS_CONTROL_POINT_RESPONSE_OPCODE_SENSOR_VALUE_9:
    case BLC_ESLSS_CONTROL_POINT_RESPONSE_OPCODE_SENSOR_VALUE_A:
    case BLC_ESLSS_CONTROL_POINT_RESPONSE_OPCODE_SENSOR_VALUE_B:
    case BLC_ESLSS_CONTROL_POINT_RESPONSE_OPCODE_SENSOR_VALUE_C:
    case BLC_ESLSS_CONTROL_POINT_RESPONSE_OPCODE_SENSOR_VALUE_D:
    case BLC_ESLSS_CONTROL_POINT_RESPONSE_OPCODE_SENSOR_VALUE_E:
    case BLC_ESLSS_CONTROL_POINT_RESPONSE_OPCODE_SENSOR_VALUE_F:
    {
        blc_eslss_controlPointResponseSensorValue_t *sensVal = (blc_eslss_controlPointResponseSensorValue_t *) rsp;

        app_parse_printf("ESL response Sensor Value [group_id: %d esl_id: %d] sensor id:%d len %d data: ", address->groupId, address->eslId, sensVal->sensorId, ((sensVal->hdr.opcode & 0xF0) >> 4));
        for (u8 i = 0; i < (sensVal->hdr.opcode & 0xF0) >> 4; i++) {
            app_parse_printf("%02X", sensVal->sensorData[i]);
        }

        app_parse_printf("\r\n");
        break;
    }
    case BLC_ESLSS_CONTROL_POINT_RESPONSE_OPCODE_VENDOR_SPECIFIC_RESPONSE_0:
    case BLC_ESLSS_CONTROL_POINT_RESPONSE_OPCODE_VENDOR_SPECIFIC_RESPONSE_1:
    case BLC_ESLSS_CONTROL_POINT_RESPONSE_OPCODE_VENDOR_SPECIFIC_RESPONSE_2:
    case BLC_ESLSS_CONTROL_POINT_RESPONSE_OPCODE_VENDOR_SPECIFIC_RESPONSE_3:
    case BLC_ESLSS_CONTROL_POINT_RESPONSE_OPCODE_VENDOR_SPECIFIC_RESPONSE_4:
    case BLC_ESLSS_CONTROL_POINT_RESPONSE_OPCODE_VENDOR_SPECIFIC_RESPONSE_5:
    case BLC_ESLSS_CONTROL_POINT_RESPONSE_OPCODE_VENDOR_SPECIFIC_RESPONSE_6:
    case BLC_ESLSS_CONTROL_POINT_RESPONSE_OPCODE_VENDOR_SPECIFIC_RESPONSE_7:
    case BLC_ESLSS_CONTROL_POINT_RESPONSE_OPCODE_VENDOR_SPECIFIC_RESPONSE_8:
    case BLC_ESLSS_CONTROL_POINT_RESPONSE_OPCODE_VENDOR_SPECIFIC_RESPONSE_9:
    case BLC_ESLSS_CONTROL_POINT_RESPONSE_OPCODE_VENDOR_SPECIFIC_RESPONSE_A:
    case BLC_ESLSS_CONTROL_POINT_RESPONSE_OPCODE_VENDOR_SPECIFIC_RESPONSE_B:
    case BLC_ESLSS_CONTROL_POINT_RESPONSE_OPCODE_VENDOR_SPECIFIC_RESPONSE_C:
    case BLC_ESLSS_CONTROL_POINT_RESPONSE_OPCODE_VENDOR_SPECIFIC_RESPONSE_D:
    case BLC_ESLSS_CONTROL_POINT_RESPONSE_OPCODE_VENDOR_SPECIFIC_RESPONSE_E:
    case BLC_ESLSS_CONTROL_POINT_RESPONSE_OPCODE_VENDOR_SPECIFIC_RESPONSE_F:
    {
        blc_eslss_controlPointResponseVendorSpecific_t *vendorResp = (blc_eslss_controlPointResponseVendorSpecific_t *) rsp;
        app_parse_printf("ESL response Vendor Specific [group_id: %d esl_id: %d] vendor response:0x%02X len:%d data:", address->groupId, address->eslId, rsp->opcode, (((vendorResp->hdr.opcode & 0xF0) >> 4) + 1));
        for (u8 i = 0; i < (((vendorResp->hdr.opcode & 0xF0) >> 4) + 1); i++) {
            app_parse_printf("%02X", vendorResp->parameters[i]);
        }
        app_parse_printf("\r\n");
        break;
    }
    }
}

static int app_ap_eslResponseCb(u16 connHandle, u8 *pData, u16 dataLen)
{
    (void)connHandle;
    (void)dataLen;
    blc_eslp_ap_pawrResponseRcvd_t *pEvt = (blc_eslp_ap_pawrResponseRcvd_t *) pData;
    blc_eslss_controlPointResponseHdr_t *rsp = pEvt->rsp;

    if (pEvt->status == BLE_SUCCESS) {
        for (u8 i = 0; i < pEvt->numRsp; i++) {
            u16 rspSize = blc_esl_getResponseSize(rsp);

            app_ap_eslResponse(&pEvt->address, rsp, true);

            rsp = (blc_eslss_controlPointResponseHdr_t *) (((u8 *) rsp) + rspSize);
        }
    } else {
        app_parse_printf("Failed to receive response from [group_id:%d esl_id:%d]\r\n", pEvt->address.groupId, pEvt->address.eslId);
    }

    return 0;
}

static int app_ap_eslControlPointCb(u16 connHandle, u8 *data, u16 dataLen)
{
    (void)dataLen;
    bls_eslsc_elsControlPointResponseEvt_t *rsp = (bls_eslsc_elsControlPointResponseEvt_t *) data;
    app_ap_eslInfo_t *eslInfo = getEslInfoByConnHandle(connHandle);
    if (eslInfo) {
        app_ap_eslResponse(&eslInfo->address, rsp->rsp, false);
    }

    return 0;
}

static int app_ap_eslCommandSentCb(u16 connHandle, u8 *pData, u16 dataLen)
{
    (void)connHandle;
    (void)dataLen;
    blc_eslp_ap_pawrCommandSentEvt_t *pEvt = (blc_eslp_ap_pawrCommandSentEvt_t *) pData;

    app_parse_printf("Command sent [group_id:%d] status: %d\r\n", pEvt->groupId, pEvt->status);

    return 0;
}

static int app_ap_otsOacpCb(u16 connHandle, u8 *pData, u16 dataLen)
{
    blc_otsc_objectActionControlPointEvt_t *oacp = (blc_otsc_objectActionControlPointEvt_t *) pData;

    if (dataLen > sizeof(*oacp)) {
        app_parse_printf("OTS Oacp conn_handle:%d opcode:0x%02X req_opcode:0x%02X res:0x%02X data:",
                connHandle, oacp->rsp[0].opCode, oacp->rsp[0].requestedOpCode, oacp->rsp[0].resultCode);

        for (u8 i = 0; i < (dataLen - sizeof(*oacp)); i++) {
            app_parse_printf("%02X", oacp->rsp[0].params[i]);
        }

        app_parse_printf("\r\n");
    } else {
        app_parse_printf("OTS Oacp conn_handle:%d opcode:0x%02X req_opcode:0x%02X res:0x%02X\r\n",
                connHandle, oacp->rsp[0].opCode, oacp->rsp[0].requestedOpCode, oacp->rsp[0].resultCode);
    }

    return 0;
}

static int app_ap_otsOacpTimeoutCb(u16 connHandle, u8 *pData, u16 dataLen)
{
    (void)pData;
    (void)dataLen;
    app_parse_printf("OTS Oacp timeout conn_handle:%d\r\n", connHandle);

    return 0;
}

static int app_ap_otsOlcpCb(u16 connHandle, u8 *pData, u16 dataLen)
{
    blc_otsc_objectListControlPointEvt_t *olcp = (blc_otsc_objectListControlPointEvt_t *) pData;

    if (dataLen > sizeof(*olcp)) {
        app_parse_printf("OTS Olcp conn_handle:%d opcode:0x%02X req_opcode:0x%02X res:0x%02X data:",
                connHandle, olcp->rsp[0].opCode, olcp->rsp[0].requestedOpCode, olcp->rsp[0].resultCode);

        for (u8 i = 0; i < (dataLen - sizeof(*olcp)); i++) {
            app_parse_printf("%02X", olcp->rsp[0].params[i]);
        }

        app_parse_printf("\r\n");
    } else {
        app_parse_printf("OTS Olcp conn_handle:%d opcode:0x%02X req_opcode:0x%02X res:0x%02X\r\n",
                connHandle, olcp->rsp[0].opCode, olcp->rsp[0].requestedOpCode, olcp->rsp[0].resultCode);
    }

    return 0;
}

static int app_ap_otsOlcpTimeoutCb(u16 connHandle, u8 *pData, u16 dataLen)
{
    (void)pData;
    (void)dataLen;
    app_parse_printf("OTS Olcp timeout conn_handle:%d\r\n", connHandle);

    return 0;
}

static int app_ap_otsObjectChangedCb(u16 connHandle, u8 *pData, u16 dataLen)
{
    (void)dataLen;
    blc_otsc_objectChangedEvt_t *changed = (blc_otsc_objectChangedEvt_t *) pData;
    u64 objectId = bstream_to_u48_le(changed->id.objectId);

    app_parse_printf("Object Changed conn_handle:%d flags:%d objectId:%llu\r\n", connHandle, changed->flags, objectId);

    return 0;
}

static int app_ap_otsObjectTransferOpenedCb(u16 connHandle, u8 *pData, u16 dataLen)
{
    (void)dataLen;
    blc_otsc_objectTransferChannelConnectedEvt_t *connected = (blc_otsc_objectTransferChannelConnectedEvt_t *) pData;

    app_parse_printf("OTS object transfer opened conn_handle:%d mtu:%d\r\n", connHandle, connected->mtu);

    return 0;
}

static int app_ap_otsObjectTransferClosedCb(u16 connHandle, u8 *pData, u16 dataLen)
{
    (void)pData;
    (void)dataLen;
    app_parse_printf("OTS object transfer closed conn_handle:%d\r\n", connHandle);

    return 0;
}

static int app_ap_otsObjectTransferDataSentCb(u16 connHandle, u8 *pData, u16 dataLen)
{
    (void)pData;
    (void)dataLen;
    app_parse_printf("OTS object transfer data sent conn_handle:%d\r\n", connHandle);

    return 0;
}

static int app_ap_otsObjectTransferDataReceivedCb(u16 connHandle, u8 *pData, u16 dataLen)
{
    (void)dataLen;
    blc_otsc_objectTransferChannelDataReceivedEvt_t *data_rcvd = (blc_otsc_objectTransferChannelDataReceivedEvt_t *) pData;

    app_parse_printf("OTS data rcvd conn_handle:%d len:%d data:", connHandle, data_rcvd->len);

    for (u16 i = 0; i < data_rcvd->len; i++) {
        app_parse_printf("%02X", data_rcvd->data[i]);
    }

    app_parse_printf("\r\n");

    return 0;
}

static int app_ap_svcChangedCb(u16 connHandle, u8 *pData, u16 dataLen)
{
    (void)dataLen;
    blc_gattsc_serviceChangedIndication_t *pEvt = (blc_gattsc_serviceChangedIndication_t *) pData;

    app_parse_printf("Service changed conn_h:%d start_h:0x%04X end_h:0x%04X\r\n",
            connHandle, pEvt->svc_changed.startHandle, pEvt->svc_changed.endHandle);

    return 0;
}

static const app_prf_evtCb_t app_ap_cb[] = {
    /* Event for controller or Host */
    {PRF_EVTID_CLIENT_SDP_FAIL, app_ap_client_sdpFail},
    {PRF_EVTID_CLIENT_SDP_FOUND, app_ap_client_sdpFound},
    {PRF_EVTID_CLIENT_SDP_END, app_ap_client_sdpEnd},
    {BASIC_EVT_GATTSC_RECV_SERVICE_CHANGED_INDICATION, app_ap_svcChangedCb},
    {ESL_EVT_ESLSC_ESL_CONTROL_POINT_RESPONSE, app_ap_eslControlPointCb},
    {ESL_EVT_ESLP_AP_PAWR_RESPONSE_RCVD, app_ap_eslResponseCb},
    {ESL_EVT_ESLP_AP_PAWR_COMMAND_SENT, app_ap_eslCommandSentCb},
    {ESL_EVT_OTSC_OBJECT_ACTION_CONTROL_POINT_EVT, app_ap_otsOacpCb},
    {ESL_EVT_OTSC_OBJECT_ACTION_CONTROL_POINT_TIMEOUT_EVT, app_ap_otsOacpTimeoutCb},
    {ESL_EVT_OTSC_OBJECT_LIST_CONTROL_POINT_EVT, app_ap_otsOlcpCb},
    {ESL_EVT_OTSC_OBJECT_LIST_CONTROL_POINT_TIMEOUT_EVT, app_ap_otsOlcpTimeoutCb},
    {ESL_EVT_OTSC_OBJECT_CHANGED_EVT, app_ap_otsObjectChangedCb},
    {ESL_EVT_OTSC_OBJECT_TRANSFER_CHANNEL_CONNECTED, app_ap_otsObjectTransferOpenedCb},
    {ESL_EVT_OTSC_OBJECT_TRANSFER_CHANNEL_DISCONNECTED, app_ap_otsObjectTransferClosedCb},
    {ESL_EVT_OTSC_OBJECT_TRANSFER_CHANNEL_DATA_SENT, app_ap_otsObjectTransferDataSentCb},
    {ESL_EVT_OTSC_OBJECT_TRANSFER_CHANNEL_DATA_RECEIVED, app_ap_otsObjectTransferDataReceivedCb},
};

int app_ap_prf_event_callback(u16 aclHandle, int evtID, u8 *pData, u16 dataLen)
{
#if (BLE_OTA_CLIENT_ENABLE)
    app_ota_prf_event_callback(aclHandle, evtID, pData, dataLen);
#endif

    for(u32 i=0; i < ARRAY_SIZE(app_ap_cb); i++)
    {
        if(app_ap_cb[i].evtId == evtID)
            return app_ap_cb[i].evtCb(aclHandle, pData, dataLen);
    }
    return 0;
}

void app_ap_loop(void)
{
    if (clock_time_exceed(currentTimeTick, 10*1000))
    {
        currentTimeTick = clock_time();
        currentTime += 10;
    }
#if (BLE_OTA_CLIENT_ENABLE)
    app_ota_mainloop();
#endif
}
