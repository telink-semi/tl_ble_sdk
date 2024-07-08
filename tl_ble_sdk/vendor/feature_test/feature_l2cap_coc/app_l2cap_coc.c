/********************************************************************************************************
 * @file    app_l2cap_coc.c
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
#include "app_buffer.h"
#include "app_ui.h"
#include "app_l2cap_coc.h"

#if (FEATURE_TEST_MODE == TEST_L2CAP_COC)

typedef struct{
    u16 connHandle;
    u16 mtu;
    u16 srcCid;
    u16 dstCid;
} app_cocCid_t;

app_cocCid_t app_cocCid[COC_CID_COUNT];

static u8 cocBuffer[COC_MODULE_BUFFER_SIZE(CREATE_COC_CONNECT_ACL_COUNT, COC_CID_COUNT, 0, COC_MTU_SIZE)];

u8 sendTestDataBuffer[2000];

void app_l2cap_coc_init(void)
{
    blc_coc_initParam_t regParam = {
        .MTU = COC_MTU_SIZE,
        .SPSM = 0x0080,
        .createConnCnt = CREATE_COC_CONNECT_ACL_COUNT,
        .cocCidCnt = COC_CID_COUNT,
    };
    int state = blc_l2cap_registerCocModule(&regParam, cocBuffer, sizeof(cocBuffer));
    tlkapi_printf(APP_COC_LOG_EN, "[APP][COC] Init CoC module: result<%d>\r\n", state);
}

/**
 * @brief      BLE host event handler call-back.
 * @param[in]  h       event type
 * @param[in]  para    Pointer point to event parameter buffer.
 * @param[in]  n       the length of event parameter.
 * @return
 */
int app_host_coc_event_callback (u32 h, u8 *para, int n)
{
    (void)n;
    u8 event = h & 0xFF;

    switch(event)
    {
        case GAP_EVT_L2CAP_COC_CONNECT:
        {
            gap_l2cap_cocConnectEvt_t* cocConnEvt = (gap_l2cap_cocConnectEvt_t*)para;
            tlkapi_printf(APP_COC_LOG_EN, "[APP][COC] CoC Connect: ConnHandle<0x%04x>, SPSM<0x%x>, MTU<0x%x>, SrcCid<0x%x>, dstCid<0x%x>\r\n",
                    cocConnEvt->connHandle, cocConnEvt->spsm, cocConnEvt->mtu, cocConnEvt->srcCid, cocConnEvt->dstCid);

            for(unsigned int i=0; i<ARRAY_SIZE(app_cocCid); i++)
            {
                if(!app_cocCid[i].connHandle)
                {
                    app_cocCid[i].connHandle = cocConnEvt->connHandle;
                    app_cocCid[i].mtu = cocConnEvt->mtu;
                    app_cocCid[i].srcCid = cocConnEvt->srcCid;
                    app_cocCid[i].dstCid = cocConnEvt->dstCid;
                    break;
                }
            }
        }
        break;

        case GAP_EVT_L2CAP_COC_DISCONNECT:
        {
            gap_l2cap_cocDisconnectEvt_t* cocDisconnEvt = (gap_l2cap_cocDisconnectEvt_t*)para;
            tlkapi_printf(APP_COC_LOG_EN, "[APP][COC] CoC Disconnect: ConnHandle<0x%04x>, SrcCid<0x%x>, dstCid<0x%x>\r\n",
                    cocDisconnEvt->connHandle, cocDisconnEvt->srcCid, cocDisconnEvt->dstCid);

            for(unsigned int i=0; i<ARRAY_SIZE(app_cocCid); i++)
            {
                if(app_cocCid[i].connHandle == cocDisconnEvt->connHandle &&
                        app_cocCid[i].srcCid == cocDisconnEvt->srcCid &&
                        app_cocCid[i].dstCid == cocDisconnEvt->dstCid
                )
                {
                    app_cocCid[i].connHandle = 0;
                    app_cocCid[i].srcCid = 0;
                    app_cocCid[i].dstCid = 0;
                    break;
                }
            }
        }
        break;

        case GAP_EVT_L2CAP_COC_RECONFIGURE:
        {
            gap_l2cap_cocReconfigureEvt_t* cocRecfgEvt = (gap_l2cap_cocReconfigureEvt_t*)para;
            tlkapi_printf(APP_COC_LOG_EN, "[APP][COC] Reconfigure CoC: ConnHandle<0x%04x>, SrcCid<0x%x>, MTU<0x%x>\r\n",
                    cocRecfgEvt->connHandle, cocRecfgEvt->srcCid, cocRecfgEvt->mtu);
        }
        break;

        case GAP_EVT_L2CAP_COC_RECV_DATA:
        {
            gap_l2cap_cocRecvDataEvt_t* cocRecvDataEvt = (gap_l2cap_cocRecvDataEvt_t*)para;
            tlkapi_printf(APP_COC_LOG_EN, "[APP][COC] Receive CoC data: ConnHandle<0x%04x>, dstCid<0x%x>, dataLength<%d>\r\n",
                    cocRecvDataEvt->connHandle, cocRecvDataEvt->dstCid, cocRecvDataEvt->length);

            for(int i=0; i<cocRecvDataEvt->length; i+=128)
            {
                tlkapi_send_string_data(APP_COC_LOG_EN, "[APP][COC] CoC Data", cocRecvDataEvt->data+i, min(128, cocRecvDataEvt->length-i));
            }
        }
        break;

        case GAP_EVT_L2CAP_COC_SEND_DATA_FINISH:
        {
            gap_l2cap_cocSendDataFinishEvt_t* cocSendDataFinishEvt = (gap_l2cap_cocSendDataFinishEvt_t*)para;
            tlkapi_printf(APP_COC_LOG_EN, "[APP][COC] Send CoC data finish, ConnHandle<0x%04x>, SrcCid<0x%x>\r\n",
                    cocSendDataFinishEvt->connHandle, cocSendDataFinishEvt->srcCid);
        }
        break;

        case GAP_EVT_L2CAP_COC_CREATE_CONNECT_FINISH:
        {
            gap_l2cap_cocCreateConnectFinishEvt_t* cocCreateConnFinishEvt = (gap_l2cap_cocCreateConnectFinishEvt_t*)para;
            tlkapi_printf(APP_COC_LOG_EN, "[APP][COC] Create CoC connect finish, ConnHandle<0x%04x>, code<0x%X>, result<%d>\r\n",
                    cocCreateConnFinishEvt->connHandle, cocCreateConnFinishEvt->code, cocCreateConnFinishEvt->result);
        }
        break;

        default:
        break;
    }

    return 0;
}

void app_createLeCreditBasedConnect(void)
{
    for(int i=0; i < (ACL_CENTRAL_MAX_NUM + ACL_PERIPHR_MAX_NUM); i++){
        if(conn_dev_list[i].conn_state){
            ble_sts_t state = blc_l2cap_createLeCreditBasedConnect(conn_dev_list[i].conn_handle);
            tlkapi_printf(APP_COC_LOG_EN, "[APP][COC] Create LE Credit Based Connect: ConnHandle<0x%04x>, result<0x%x>\r\n", conn_dev_list[i].conn_handle, state);
        }
    }
}

void app_createCreditBasedConnect(void)
{
    for(int i=0; i < (ACL_CENTRAL_MAX_NUM + ACL_PERIPHR_MAX_NUM); i++){
        if(conn_dev_list[i].conn_state){
            ble_sts_t state = blc_l2cap_createCreditBasedConnect(conn_dev_list[i].conn_handle, 5);
            tlkapi_printf(APP_COC_LOG_EN, "[APP][COC] Create Credit Based Connect: ConnHandle<0x%04x>, result<0x%x>\r\n", conn_dev_list[i].conn_handle, state);
        }
    }
}

void app_sendCocData(void)
{
    for(unsigned int i=0; i<ARRAY_SIZE(app_cocCid); i++)
    {
        if(app_cocCid[i].connHandle)
        {
//            ble_sts_t state = blc_l2cap_sendCocData(app_cocCid[i].connHandle, app_cocCid[i].srcCid, sendTestDataBuffer, min(sizeof(sendTestDataBuffer), app_cocCid[i].mtu));
            ble_sts_t state = blc_l2cap_sendCocData(app_cocCid[i].connHandle, app_cocCid[i].srcCid, sendTestDataBuffer, 300);
            tlkapi_printf(APP_COC_LOG_EN, "[APP][COC] Send CoC data: ConnHandle<0x%04x>, SrcCid<0x%x>, length<%d>, result<0x%x>\r\n",
                    app_cocCid[i].connHandle, app_cocCid[i].srcCid, min(sizeof(sendTestDataBuffer), app_cocCid[i].mtu), state);
        }
    }
}

void app_disconnCocConnect(void)
{
    for(unsigned int i=0; i<ARRAY_SIZE(app_cocCid); i++)
    {
        if(app_cocCid[i].connHandle)
        {
            ble_sts_t state = blc_l2cap_disconnectCocChannel(app_cocCid[i].connHandle, app_cocCid[i].srcCid);
            tlkapi_printf(APP_COC_LOG_EN, "[APP][COC] Disconnect CoC: ConnHandle<0x%04x>, SrcCid<0x%x>, result<0x%x>\r\n",
                    app_cocCid[i].connHandle, app_cocCid[i].srcCid, state);
        }
    }
}

#endif
