/********************************************************************************************************
 * @file    prf_common.h
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

#pragma once

#ifndef BLC_ZEPHYR_BLE_INTEGRATION
    #include <sys/queue.h>
#endif

#define BLC_PRF_DBG(en, fmt, ...) tlkapi_printf(en, "[PRF]" fmt "\n", ##__VA_ARGS__)

/* Stack max settings */
#define STACK_PRF_ACL_CONN_MAX_NUM       ACL_CENTRAL_MAX_NUM+ACL_PERIPHR_MAX_NUM //LL_MAX_ACL_CONN_NUM
#define STACK_PRF_ACL_CENTRAL_MAX_NUM    ACL_CENTRAL_MAX_NUM  //LL_MAX_ACL_CEN_NUM
#define STACK_PRF_ACL_PERIPHERAL_MAX_NUM ACL_PERIPHR_MAX_NUM  //LL_MAX_ACL_PER_NUM

/**
 * profile bound ACL role enumeration.
 * PRF_GAP_ACL_CENTRAL: ACL central.
 * PRF_GAP_ACL_PERIPHERAL: ACL peripheral.
 * PRF_GAP_ACL_UNSPECIF: ACL central and ACL peripheral.
 */
typedef enum
{
    PRF_GAP_ACL_CENTRAL    = BIT(0),
    PRF_GAP_ACL_PERIPHERAL = BIT(1),
    PRF_GAP_ACL_UNSPECIF   = BITS(0, 1),
} prf_bound_acl_role_enum;

typedef enum
{
    PRF_COMMON_SUCC = 0x00, /* Success */
    PRF_COMMON_ERR,
    PRF_COMMON_ERR_INPUT_NULL,
    PRF_COMMON_ERR_INPUT_PARAM_INVALID,
    PRF_COMMON_ERR_CTRL_MODULE_NOT_FOUND,
    PRF_COMMON_ERR_ATTR_HANDLE_NOT_FOUND,
    PRF_ATT_ERROR_FLAG = 0x100,
    PRF_HCI_ERROR_FLAG = 0x200,
    PRF_OTHER_ERROR    = 0x300,
    PRF_MODULE_ERROR   = 0x400,
} prf_error_enum;

/**
 * profile ACL state enumeration.
 * PRF_ACL_STATE_CONNECT: ACL connect.
 * PRF_ACL_STATE_DISCONN: ACL disconnect.
 * PRF_ACL_STATE_RECONNECT: RFU.
 */
typedef enum
{
    PRF_ACL_STATE_CONNECT,
    PRF_ACL_STATE_DISCONN,
    PRF_ACL_STATE_RECONNECT,
} prf_acl_state_enum;

/**
 * profile NV state enumeration.
 * PRF_NV_STATE_STORE: profile store NV data.
 * PRF_NV_STATE_LOAD: profile load NV data.
 */
typedef enum
{
    PRF_NV_STATE_STORE,
    PRF_NV_STATE_LOAD,
} prf_nv_state_enum;

typedef enum
{
    PRF_CLIENT_MIN = 0x00,
    PRF_BASIC_CLIENT_START,                                                                 //for GATT, GAP, BAS, DIS.
    PRF_BASIC_CLIENT_END = PRF_BASIC_CLIENT_START + 0x0E,                                   //used:4, other reserved
    PRF_LE_AUDIO_CLIENT_START,
    PRF_LE_AUDIO_CLIENT_END = PRF_LE_AUDIO_CLIENT_START + 0x1F,                             //used:10, other reserved.
    PRF_CHANNEL_SOUNDING_CLIENT_START,
    PRF_CHANNEL_SOUNDING_CLIENT_END = PRF_CHANNEL_SOUNDING_CLIENT_START + 0x03,             //used:1 other reserved.
    PRF_HUMAN_INTERFACE_DEVICE_CLIENT_START,
    PRF_HUMAN_INTERFACE_DEVICE_CLIENT_ENC = PRF_HUMAN_INTERFACE_DEVICE_CLIENT_START + 0x03, //used:2 other reserved
    PRF_ELECTRONIC_SHELF_LABEL_CLIENT_START,
    PRF_ELECTRONIC_SHELF_LABEL_CLIENT_END = PRF_ELECTRONIC_SHELF_LABEL_CLIENT_START + 0x08, //used:4 other reserved
    PRF_OTA_CLIENT_START,
    PRF_OTA_CLIENT_END = PRF_OTA_CLIENT_START + 0x01,                                       //used:2
    PRF_SPP_CLIENT_START,
    PRF_SPP_CLIENT_END = PRF_SPP_CLIENT_START + 0x01,
    PRF_TEST_PROFILE_CLIENT_START         = 0x70,                                           //only user 1
    PRF_USE_DEFINE_CLIENT_START           = 0x71,
    PRF_SERVER_OFFSET                     = 0x80,
} profile_service_role_enum;

#define PRF_EVENT_ID_SIZE 0x0100

typedef enum
{
    PRF_EVTID_NULL,
    PRF_EVTID_COMMON_START,
    PRF_EVTID_BASIC_START                  = PRF_BASIC_CLIENT_START * PRF_EVENT_ID_SIZE,
    PRF_EVTID_LE_AUDIO_START               = PRF_LE_AUDIO_CLIENT_START * PRF_EVENT_ID_SIZE,
    PRF_EVTID_CHANNEL_SOUNDING_START       = PRF_CHANNEL_SOUNDING_CLIENT_START * PRF_EVENT_ID_SIZE,
    PRF_EVTID_HUMAN_INTERFACE_DEVICE_START = PRF_HUMAN_INTERFACE_DEVICE_CLIENT_START * PRF_EVENT_ID_SIZE,
    PRF_EVTID_ELECTRONIC_SHELF_LABEL_START = PRF_ELECTRONIC_SHELF_LABEL_CLIENT_START * PRF_EVENT_ID_SIZE,
    PRF_EVTID_OTA_START                    = PRF_OTA_CLIENT_START * PRF_EVENT_ID_SIZE,
    PRF_EVTID_SPP_START                    = PRF_SPP_CLIENT_START * PRF_EVENT_ID_SIZE,
} prf_event_id_enum;

typedef enum
{
    PRF_EVTID_TYPE_COMMON = PRF_EVTID_COMMON_START,
    PRF_EVTID_CLIENT_SDP_FAIL,
    PRF_EVTID_CLIENT_SDP_FOUND,
    PRF_EVTID_CLIENT_SDP_END,
    PRF_EVTID_CLIENT_ALL_SDP_OVER,
    PRF_EVTID_SERVICE_ACL_ROLE_FAIL,
    PRF_EVTID_ACL_CONNECT,
    PRF_EVTID_ACL_DISCONNECT,
    PRF_EVTID_SMP_SECURITY_DONE,
    PRF_EVTID_ACL_CONNECT_UPDATE,
    PRF_EVTID_LE_ADVERTISING_REPORT,

    PRF_EVTID_LE_CS_READ_REMOTE_SUPPORTED_CAPABILITIES_COMPLETE,
    PRF_EVTID_LE_CS_READ_REMOTE_FAE_TABLE_COMPLETE,
    PRF_EVTID_LE_CS_CONFIG_COMPLETE,
    PRF_EVTID_LE_CS_SECURITY_ENABLE_COMPLETE,
    PRF_EVTID_LE_CS_PROCEDURE_ENABLE_COMPLETE,
    PRF_EVTID_LE_CS_SUBEVENT_RESULT,
    PRF_EVTID_LE_CS_SUBEVENT_RESULT_CONTINUE,
} prf_common_event_id_enum;

typedef enum
{
    PRF_PROC_CONN = 0x0000, /* process when le_connected */
    PRF_PROC_DISCONN,       /* process when le_disconnected */
    PRF_PROC_DISCOVERY,     /* process discovery (better after smp_pairing_done OR re_connection encryption done) */
    PRF_PROC_LOOP,          /* process audio loop */
    PRF_PROC_INIT,          /* process when register SVC node */
    PRF_PROC_DEINIT,        /* process when unregister SVC node */
} prf_proc_type_enum;


typedef struct
{
    //if load nv data, ptr is NV block start address.
    //if store nv data, ptr is current write NV data address.
    u8 *dataPtr;

    union
    {
        u16 currentTotalLen; //only user in store mode.
        u16 nvItemLen;       //only used in load mode.
    };
} prf_nv_param_t;

typedef int (*prf_evt_cb_t)(u16 connHandle, int evtID, u8 *pData, u16 dataLen);



typedef void (*prf_read_cb_t)(u16 connHandle, att_err_t err);

typedef void (*prf_write_cb_t)(u16 connHandle, att_err_t err);

typedef void (*prf_ind_cb_t)(u16 connHandle, u16 scid);


typedef struct blc_prf_proc
{
    struct blc_prf_proc *pNext;
    u8                   id;          //profile_service_role_enum;
    u8                   usedAclRole; //prf_bound_acl_role_enum;
    u8                   reserved1[2];
    int (*init)(u8 initType, const void *initParam);
    int (*connect)(u16 connHandle, prf_acl_state_enum connState);
    int (*discov)(u16 connHandle);
    int (*loop)(u16 connHandle);
    int (*store)(u16 connHandle, prf_nv_state_enum nvState, prf_nv_param_t *param);
} blc_prf_proc_t;


typedef struct
{
    int evtId;
    int (*evtCb)(u16 connHandle, u8 *pData, u16 dataLen);
} app_prf_evtCb_t;



/**
 *  @brief  Event Parameters for "PRF_EVTID_ACL_CONNECT"
 * The LE protocol stack event is taken over by the Audio Profile, and
 * the event is wrapped and used as a callback for the profile event.
 */
typedef struct

{
    u16 aclHandle;
    u16 connInterval;
    u8  PeerAddrType;
    u8  PeerAddr[6];
} blc_prf_aclConnEvt_t;

/**
 *  @brief  Event Parameters for "PRF_EVTID_ACL_DISCONNECT"
 */

typedef struct
{
    u16 aclHandle;
    u8  reason;
} blc_prf_aclDisconnEvt_t;



/**
 *  @brief  Event Parameters for "PRF_EVTID_SERVICE_ACL_ROLE_FAIL"
 */
typedef struct
{
    u16                   connHandle;
    int                   svcId;
    acl_connection_role_t currAclRole;
} blc_prf_svrGapRoleErrorEvt_t;

/**
 *  @brief  Event Parameters for "PRF_EVTID_CLIENT_SDP_FAIL"
 */
typedef struct
{
    u16 aclHandle;
    int svcId;
} blc_prf_sdpFailEvt_t, blc_prf_sdpEndEvt_t;

/**
 *  @brief  Event Parameters for "PRF_EVTID_CLIENT_SDP_FOUND"
 */
typedef struct
{
    u16 aclHandle;
    int svcId;
    u16 startHdl;
    u16 endHdl;
} blc_prf_sdpFoundEvt_t;

/**
 *  @brief  Event Parameters for "PRF_EVTID_CLIENT_ALL_SDP_OVER"
 */
typedef struct
{
    u16 aclHandle;
} blc_prf_sdpOverEvt_t;

/**
 *  @brief  Event Parameters for "PRF_EVTID_SMP_SECURITY_DONE"
 */
typedef struct
{
    u16 aclHandle;
} blc_prf_securityDoneEvt_t;

#define PRF_EVT_CB(EVT_CALLBACK)                                                  \
    static int app_prf_eventCb(u16 connHandle, int evtID, u8 *pData, u16 dataLen) \
    {                                                                             \
        for (unsigned int i = 0; i < ARRAY_SIZE(EVT_CALLBACK); i++) {             \
            if (EVT_CALLBACK[i].evtId == evtID) {                                 \
                return EVT_CALLBACK[i].evtCb(connHandle, pData, dataLen);         \
            }                                                                     \
        }                                                                         \
        return 0;                                                                 \
    }

void blc_prf_initialModule(prf_evt_cb_t evtCb,void *base, u32 size);

/**
 * @brief       This function retrieves the ACL connection index.
 * @param[in]   connHandle - The connection handle.
 * @return      int - The ACL connection index associated with the given connection handle.
 */
int blc_prf_getAclConnectIndex(u16 connHandle);

/**
 * @brief   main_loop for Bluetooth LE Audio profile, process data and event
 * @param   none
 * @return  none
 */
void blc_prf_main_loop(void);

/**
 * @brief       This function serves to register Client or Server module.
 * @param[in]   usedAclRole - refer to 'prf_bound_acl_role_enum'.
 * @param[in]   pSvc        - refer to 'blc_prf_proc_t'.
 * @param[in]   param       - setting parameters.
 * @return      none.
 */

void blc_prf_registerServiceModule(prf_bound_acl_role_enum usedAclRole, blc_prf_proc_t *pSvc, const void *param);

/**
 * @brief       This function use read Attribute Value finish callback to high layer.
 * @param[in]   connHandle - ACL Connect.
 * @param[in]   err - ATT layer error code.
 * @return      none.
 */
void blc_prf_readAttributeValueCallback(u16 connHandle, att_err_t err);

/**
 * @brief       This function use read Attribute Value, with high layer callback function.
 * @param[in]   connHandle  - ACL Connect Handle..
 * @param[in]   pGapReCfg   - GAP client read Attribute value configuration.
 * @param[in]   readCb      - high layer callback.
 * @return      Reference API: blc_gapc_readAttributeValue.
 */
int blc_prf_readAttributeValue(u16 connHandle, gapc_read_cfg_t *pGapReCfg, prf_read_cb_t readCb);

/**
 * @brief       This function use read Attribute Value default callback.
 * @return      none.
 */
void blc_prf_readAttributeValueDefaultCallback(u16 connHandle, u8 err, gattc_read_cfg_t *pRdCfg);

/**
 * @brief       This function use write Attribute Value finish callback to high layer.
 * @param[in]   connHandle - ACL Connect.
 * @param[in]   err - ATT layer error code.
 * @return      none.
 */
void blc_prf_writeAttributeValueCallback(u16 connHandle, att_err_t err);

/**
 * @brief       This function use write Attribute Value, with high layer callback function.
 * @param[in]   connHandle  - ACL Connect Handle..
 * @param[in]   pGapWrCfg   - GAP client write Attribute value configuration.
 * @param[in]   readCb      - high layer callback.
 * @return      Reference API: blc_gapc_writeAttributeValue.
 */
int blc_prf_writeAttributeValue(u16 connHandle, gapc_write_cfg_t *pGapWrCfg, prf_write_cb_t writeCb);

/**
 * @brief       This function use write Attribute Value default callback.
 * @return      none.
 */
void blc_prf_writeAttributeValueDefaultCallback(u16 connHandle, u8 err, void *data);


/**
 * @brief       This function use set SDP/reconnect discovery status to busy.
 * @param[in]   connHandle  - ACL Connect Handle..
 * @return      none.
 */
void blc_prf_setDiscoveryStatusFinish(u16 connHandle);

/**
 * @brief       This function use check SDP/reconnect discovery status busy or not.
 * @param[in]   connHandle  - ACL Connect Handle..
 * @return      true mean busy, false mean idle.
 */
bool blc_prf_checkDiscoveryBusy(u16 connHandle);

/**
 * @brief       This function use check the connect state is reconnect or not.
 * @param[in]   connHandle  - ACL Connect Handle..
 * @return      true mean reconnect device, false mean first paired.
 */
bool blc_prf_checkReconnectFlag(u16 connHandle);

/**
 * @brief       This function use set SDP/reconnect discovery status to busy.
 * @param[in]   connHandle  - ACL Connect Handle..
 * @return      none.
 */
void blc_prf_setDiscoveryStatusBusy(u16 connHandle);


/**
 * @brief       This function use send service client SDP/reconnect Fail event to high layer.
 * @param[in]   connHandle  - ACL Connect Handle..
 * @param[in]   svcId       - service ID, only for client ID.
 * @return      none.
 */
void blc_prf_sendServiceDiscoveryFailEvent(u16 connHandle, int svcId);

/**
 * @brief       This function use send service client start/found SDP/reconnect event to high layer.
 * @param[in]   connHandle  - ACL Connect Handle..
 * @param[in]   svcId       - service ID, only for client ID.
 * @param[in]   startHdl    - found service UUID start Handle.
 * @param[in]   endHdl      - found service UUID end Handle
 * @return      none.
 */
void blc_prf_sendServiceDiscoveryFoundEvent(u16 connHandle, int svcId, u16 startHdl, u16 endHdl);

/**
 * @brief       This function use send service single client SDP/reconnect finish event to high layer.
 * @param[in]   connHandle  - ACL Connect Handle..
 * @param[in]   svcId       - service ID, only for client ID.
 * @return      none.
 */
void blc_prf_sendSingleServiceDiscoveryFinishEvent(u16 connHandle, int svcId);

/**
 * @brief       This function is used for updating storage of parameters in nv. Overwrites an existing entry.
 * @param[in]   connHandle  - ACL Connect Handle..
 * @return      none.
 */
void blt_prf_updatePairingInfoByAclHandle(u16 connHandle);
