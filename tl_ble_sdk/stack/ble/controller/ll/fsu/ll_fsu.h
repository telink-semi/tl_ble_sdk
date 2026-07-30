/********************************************************************************************************
 * @file    ll_fsu.h
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
#ifndef LL_FSU_H_
#define LL_FSU_H_

#include "tl_common.h"
#include "common/types.h"
#include "stack/ble/ble_common.h"

/* Default frame space value (us) used when fsu_param is unbound (NULL) or on reset. */
#define FSU_DEFAULT_FS_US       150

#define FSU_ACL_MCES_DEFAULT    1250

#define FSU_REQ_PENDING         BIT(0)
#define FSU_RSP_PENDING         BIT(1)

enum{
    FSU_PROCEDURE_CMPLET = 0,
    FSU_PROCEDURE_START    = 1,
};

enum{
    ST_ACL_CP_POS   = 0,
    ST_ACL_PC_POS   = 1,
    ST_ACL_MCES_POS = 2,
    ST_CIS_IFS_POS  = 3,
    ST_CIS_MSS_POS  = 4,
};

enum{
    FSU_ST_ACL_CP   = BIT(0),
    FSU_ST_ACL_PC   = BIT(1),
    FSU_ST_ACL_MCES = BIT(2),
    FSU_ST_CIS_IFS  = BIT(3),
    FSU_ST_CIS_MSS  = BIT(4),
};


enum{
    FSU_REQ_CMD_SEND               = BIT(0),
    FSU_REQ_CMD_REV                = BIT(1),
    FSU_RSP_CMD_SEND               = BIT(2),
    FSU_RSP_CMD_REV                = BIT(3),

    FSU_LL_REJ_CMD_SEND            = BIT(4),
    FSU_LL_REJ_CMD_REV_UNSUP_FEAT  = BIT(5), //Unsupport remote feature
    FSU_LL_REJ_CMD_REV_COLLSION    = BIT(6),  //LL Procedure Collision
    FSU_LL_REJ_CMD_REV_OTHERS      = BIT(7),
    FSU_PROC_COLLISION_DETECTED    = BIT(8),

    FSU_PROCEDURE_COMPLETE         = BIT(9),
};

#define SUPPORT_MIN_FS_US   50
#define SUPPORT_MAX_FS_US   6800//250//6800


typedef struct {
    u16 fs_min; //unit us
    u16 fs_max; //unit us

    u16 spacingType; //refer to 'enum fsu_spacing_t'
    u16 fs_valid; //unit us

    u8  phyMask;
    u8  fsu_pending;
    u8  fsu_error_code;
    u8  fsu_complete_evt;

    u16 fsu_procedure_status;
    u8  initiator;  //refet to 'enum fsu_initiator'
    u8  fsu_procedure_collision;

    /* Per-connection frame space value tables (moved from global arrays).
     * Index [phy][spacingType]: phy 0=1M,1=2M,2=Coded;
     * spacingType 0=ACL_CP,1=ACL_PC,2=ACL_MCES,3=CIS_IFS,4=CIS_MSS. */
    u16 gFsuValidFsVal[3][5]; //current effective frame space values
    u16 gFsuPreFsVal[3][5];   //previous frame space values (snapshot before last update)
    u16 rx_timeout;
    u16 rsvd;
} fsu_hci_param_t;

typedef struct{
    u8  fsu_cmplet_bk_errCode;
    u8  fsu_cmplet_bk_initiator;
    u16 fsu_cmplet_bk_connHandle;

    u16 fsu_cmplet_bk_fs_valid;
    u16 fsu_cmplet_bk_spacingType;

    u8  fsu_cmplet_bk_phyMask;
    u8  fsu_cmplet_bk_valid;
    u8  fsu_rsvd[2];
}fsu_cmplet_evt_t;

extern fsu_cmplet_evt_t fsuCmpletEvt;

/* FSU task callback flags ¡ª single callback handles all FSU operations.
 * p parameter: FLAG_FSU_RESET/CONN_RESET/MAINLOOP -> (void*)(u32)connHandle,
 *              FLAG_FSU_START/GET_MCES/TX_WAIT/POST -> (void*)st_ll_conn_t* */
enum {
    FLAG_FSU_RESET      = 0,  // full reset on disconnect/HCI reset
    FLAG_FSU_CONN_RESET,      // light reset on new connection established
    FLAG_FSU_START,           // set rx_timeout in blms_start_common_1
    FLAG_FSU_TX_WAIT,         // set tx_wait in irq_acl_conn_tx
    FLAG_FSU_POST,            // update fsu_procedure_status in blms_post_common_1
    FLAG_FSU_MAINLOOP,        // mainloop processing
};

typedef int (*ll_fsu_task_cb_t)(int flag, void *p);
extern ll_fsu_task_cb_t ll_fsu_task_cb;

/**
 * @brief   Initialize FSU feature and bind external buffer to each connection's fsu_param.
 * @param   pFsuBuf    Pointer to external fsu_hci_param_t buffer array (one per connection).
 * @param   buf_size   Size of pFsuBuf in bytes. Must be >= LL_MAX_ACL_CONN_NUM * sizeof(fsu_hci_param_t).
 * @return  BLE_SUCCESS on success, HCI_ERR_INVALID_HCI_CMD_PARAMS on invalid parameter.
 */
ble_sts_t blc_ll_initFrameSpaceUpdate_feature(fsu_hci_param_t *pFsuBuf, u16 buf_size);

ble_sts_t blc_ll_frameSpaceUpdate(u16 connHandle, u16 fs_min, u16 fs_max, u8 phyMask, u16 spacingType);


#endif

