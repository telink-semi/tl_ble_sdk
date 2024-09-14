/********************************************************************************************************
 * @file    app.c
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
#include "../default_att.h"
#include "app_ui.h"


#if (FEATURE_TEST_MODE == TEST_SMP)


///////////////////////// Feature Configuration////////////////////////////////////////////////
#define SMP_TEST_NOT_SUPPORT    0   /* LE_Security_Mode_1_Level_1: No authentication, no encryption    */
#define SMP_TEST_LEGACY_JW      1   /* LE_Security_Mode_1_Level_2: Legacy JustWorks                    */
#define SMP_TEST_LESC_JW        5   /* LE_Security_Mode_1_Level_2: LESC JustWorks                      */
#define SMP_TEST_LEGACY_PKI     2   /* LE_Security_Mode_1_Level_3: Legacy Passkey Entry input          */
#define SMP_TEST_LEGACY_PKD     3   /* LE_Security_Mode_1_Level_3: Legacy Passkey Entry display        */
#define SMP_TEST_LEGACY_OOB     4   /* LE_Security_Mode_1_Level_3: Legacy Out of Band                  */
#define SMP_TEST_LESC_PKI       6   /* LE_Security_Mode_1_Level_4: LESC Passkey Entry input            */
#define SMP_TEST_LESC_PKD       7   /* LE_Security_Mode_1_Level_4: LESC Passkey Entry display          */
#define SMP_TEST_LESC_NC        8   /* LE_Security_Mode_1_Level_4: LESC Numeric Comparison             */
#define SMP_TEST_LESC_OOB       9   /* LE_Security_Mode_1_Level_4: LESC Out of Band                    */


#define SMP_TEST_MODE           SMP_TEST_NOT_SUPPORT//SMP_TEST_NOT_SUPPORT


#if (SMP_TEST_MODE >= SMP_TEST_LESC_JW)
#define APP_SMP_SC_EN           1
#else
#define APP_SMP_SC_EN           0
#endif

#if (SMP_TEST_MODE == SMP_TEST_LEGACY_JW || SMP_TEST_MODE == SMP_TEST_LESC_JW)
#define APP_MITM_EN             0
#else
#define APP_MITM_EN             1
#endif




#if (SMP_TEST_MODE == SMP_TEST_LESC_OOB)
typedef struct{
    /* LESC OOB data */
    smp_sc_oob_data_t scoob_local;
    smp_sc_oob_data_t scoob_remote;
    /* LESC OOB local Public/Private key */
    smp_sc_oob_key_t scoob_local_key;
    /* LESC OOB status */
    bool scoob_local_used; //After Pairing Exchange, then We'll known if local OOB data use or not.
    bool scoob_remote_used; //TRUE: remote OOB used, FALSE: remote OOB NOT used
    bool scoob_remote_geted; //TRUE: received remote OOB data , FALSE: NOT received remote OOB
    bool scoob_remote_waiting;
    /* ACL handle */
    u16 acl_handle;

}AppScOobCtrl_t;

_attribute_ble_data_retention_  AppScOobCtrl_t  appScOobCtrl;

#endif

_attribute_ble_data_retention_      int central_smp_pending = 0;        // SMP: security & encryption;

__attribute__((aligned(4))) typedef struct{
    u8  dat[256];
    u16 len;
    u16 connHandle;
}AppCtrl_t;

AppCtrl_t appCtrl;

/**
 * @brief   BLE Advertising data
 */
const u8    tbl_advData[] = {
     12,  DT_COMPLETE_LOCAL_NAME,               'f','e','a','t','u','r','e','_','s','m','p',
     2,  DT_FLAGS,                              0x05,                   // BLE limited discoverable mode and BR/EDR not supported
     3,  DT_APPEARANCE,                         0x80, 0x01,             // 384, Generic Remote Control, Generic category
     5,  DT_INCOMPLETE_LIST_16BIT_SERVICE_UUID, 0x12, 0x18, 0x0F, 0x18, // incomplete list of service class UUIDs (0x1812, 0x180F)
};

/**
 * @brief   BLE Scan Response Packet data
 */
const u8    tbl_scanRsp [] = {
     12, DT_COMPLETE_LOCAL_NAME,                'f','e','a','t','u','r','e','_','s','m','p',
};


#if APP_MITM_EN
__attribute__((aligned(4))) typedef struct{
    unsigned int    len;        // data max 252
    unsigned char   data[sizeof(smp_sc_oob_data_t)];
} uart_data_t;

/*! UART transport pin define */
#define UART_ID              UART0
#if (MCU_CORE_TYPE == MCU_CORE_B91)
#define UART_RX_PIN          UART0_RX_PA4
#define UART_TX_PIN          UART0_TX_PA3
#elif (MCU_CORE_TYPE == MCU_CORE_B92)
#define UART_RX_PIN          GPIO_FC_PA4
#define UART_TX_PIN          GPIO_FC_PA3
#endif
#define UART_DMA_CHN_RX      DMA2
#define UART_DMA_CHN_TX      DMA3
#define UART_BAUDRATE        (115200)

void UART_Init(u8 *rxBuf, u32 byteNum)
{
    u16 div = 0;
    u8 bwpc = 0;

    uart_reset(UART_ID);

    #if (MCU_CORE_TYPE == MCU_CORE_B91)
        uart_set_pin(UART_TX_PIN, UART_RX_PIN);
    #elif (MCU_CORE_TYPE == MCU_CORE_B92)
        uart_set_pin(UART_ID, UART_TX_PIN, UART_RX_PIN);
    #endif

    uart_cal_div_and_bwpc(UART_BAUDRATE, sys_clk.pclk*1000*1000, &div, &bwpc);
    #if (MCU_CORE_TYPE == MCU_CORE_B91)
        uart_set_rx_timeout(UART_ID, bwpc, 12, UART_BW_MUL3);
    #elif (MCU_CORE_TYPE == MCU_CORE_B92)
        uart_set_rx_timeout(UART_ID, bwpc, 12, UART_BW_MUL2);
    #endif
    uart_init(UART_ID, div, bwpc, UART_PARITY_NONE, UART_STOP_BIT_ONE);
    uart_set_rx_dma_config(UART_ID, UART_DMA_CHN_RX);
    uart_set_tx_dma_config(UART_ID, UART_DMA_CHN_TX);

    uart_set_irq_mask(UART_ID, UART_RXDONE_MASK);
    plic_interrupt_enable(IRQ_UART0);
    core_interrupt_enable();

    uart_receive_dma(UART_ID, (unsigned char*)rxBuf, byteNum);
}

_attribute_ram_code_sec_ void uart0_irq_handler(void)
{
#if (MCU_CORE_TYPE == MCU_CORE_B91)
    if(uart_get_irq_status(UART_ID, UART_RXDONE))
#elif (MCU_CORE_TYPE == MCU_CORE_B92)
    if(uart_get_irq_status(UART_ID,UART_RXDONE_IRQ_STATUS))
#endif
    {
        if((uart_get_irq_status(UART_ID,UART_RX_ERR)))
        {
            #if (MCU_CORE_TYPE == MCU_CORE_B91)
                uart_clr_irq_status(UART_ID, UART_CLR_RX);
            #elif (MCU_CORE_TYPE == MCU_CORE_B92)
                uart_clr_irq_status(UART_ID,UART_RXBUF_IRQ_STATUS);
            #endif
        }
        u32 rev_data_len = 0;
    /************************get the length of receive data****************************/
        #if (MCU_CORE_TYPE == MCU_CORE_B91)
            if(((reg_uart_status1(UART_ID) & FLD_UART_RBCNT) % 4)==0){
                rev_data_len = uart_get_dma_rev_data_len(UART_ID,UART_DMA_CHN_RX);
            }
            else{
                rev_data_len = uart_get_dma_rev_data_len(UART_ID,UART_DMA_CHN_RX);
            }
        #elif (MCU_CORE_TYPE == MCU_CORE_B92)
            rev_data_len = uart_get_dma_rev_data_len(UART_ID,UART_DMA_CHN_RX);
        #endif
        #if (MCU_CORE_TYPE == MCU_CORE_B91)
            uart_clr_irq_status(UART_ID, UART_CLR_RX);
        #elif (MCU_CORE_TYPE == MCU_CORE_B92)
            uart_clr_irq_status(UART_ID,UART_RXDONE_IRQ_STATUS);
        #endif

        //DMA access memory needs to be aligned according to word.
        uart_receive_dma(UART_ID, (unsigned char*)appCtrl.dat,sizeof(appCtrl.dat));

#if (SMP_TEST_MODE == SMP_TEST_LESC_PKI || SMP_TEST_MODE == SMP_TEST_LEGACY_PKI)
        if(rev_data_len == 6 && appCtrl.connHandle){
            u32 pinCode = 0;
            for(u32 i = 0; i < rev_data_len; i++){
                pinCode = pinCode * 10 + appCtrl.dat[i] - '0';
                blc_smp_sendKeypressNotify(appCtrl.connHandle, KEYPRESS_NTF_PKE_DIGIT_ENTERED);
            }
            blc_smp_setTK_by_PasskeyEntry(appCtrl.connHandle, pinCode);
            blc_smp_sendKeypressNotify(appCtrl.connHandle, KEYPRESS_NTF_PKE_COMPLETED);
            appCtrl.connHandle = 0;
            tlkapi_printf(APP_SMP_LOG_EN, "[APP][SMP] TK set:%d\n",pinCode);
        }
#elif (SMP_TEST_MODE == SMP_TEST_LEGACY_OOB)
        if(rev_data_len == 16 && appCtrl.connHandle){
            u8 oobData[16] = {0};
            for(int i=0; i<rev_data_len; i++) {
                oobData[i] = appCtrl.dat[i];
            }
            blc_smp_setTK_by_OOB(appCtrl.connHandle, oobData);
            appCtrl.connHandle = 0;
            tlkapi_send_string_data(APP_SMP_LOG_EN, "[APP][SMP] LEGACY OOB TK set", oobData, 16);
        }
#elif (SMP_TEST_MODE == SMP_TEST_LESC_OOB)
        if(rev_data_len == 32 && appScOobCtrl.scoob_remote_used){
            appScOobCtrl.scoob_remote_geted = 1;

            for(int i=0; i<16; i++) {
                appScOobCtrl.scoob_remote.confirm[i] = appCtrl.dat[i];
                appScOobCtrl.scoob_remote.random[i] = appCtrl.dat[i + 16];
            }
            tlkapi_send_string_data(APP_SMP_LOG_EN, "[APP][SMP]Get Remote SC OOB data-c(be)(by UART)", appScOobCtrl.scoob_remote.confirm, 16);
            tlkapi_send_string_data(APP_SMP_LOG_EN, "[APP][SMP]    Remote SC OOB data-r(be)(by UART)", appScOobCtrl.scoob_remote.random, 16);


            if(appScOobCtrl.scoob_remote_waiting){
                appScOobCtrl.scoob_remote_geted = FALSE; //clear
                appScOobCtrl.scoob_remote_waiting = FALSE; //clear
                blc_smp_setScOobData(appScOobCtrl.acl_handle, &appScOobCtrl.scoob_local, &appScOobCtrl.scoob_local_key,
                                        &appScOobCtrl.scoob_remote);

                appScOobCtrl.acl_handle = 0;
            }

        }
        else if(rev_data_len == 1){
            u8 val = appCtrl.dat[0] - '0';
            tlkapi_printf(APP_SMP_LOG_EN, "[APP][SMP] SC OOB remote used flag:%d", val);

            appScOobCtrl.scoob_remote_used = val ? TRUE : FALSE;
            appScOobCtrl.scoob_remote_geted = FALSE; //clear
            appScOobCtrl.scoob_remote_waiting = FALSE; //clear

            blc_smp_enableOobAuthentication(appScOobCtrl.scoob_remote_used ? 1 : 0);
            blc_smp_smpParamInit();
        }
        else{
            tlkapi_printf(APP_SMP_LOG_EN, "[APP][SMP] uart rx length %d", rev_data_len);
        }
#endif

    }
}

PLIC_ISR_REGISTER(uart0_irq_handler, IRQ_UART0)
#endif


/**
 * @brief      BLE Adv report event handler
 * @param[in]  p         Pointer point to event parameter buffer.
 * @return
 */

int app_le_adv_report_event_handle(u8 *p)
{
    event_adv_report_t *pa = (event_adv_report_t *)p;
    s8 rssi = pa->data[pa->len];


    /*********************** Central Create connection demo: Key press or ADV pair packet triggers pair  ********************/
    if(central_smp_pending){     //if previous connection SMP & SDP not finish, can not create a new connection
        return 1;
    }

    if (central_disconnect_connhandle){ //one ACL connection central role is in un_pair disconnection flow, do not create a new one
        return 1;
    }

    int central_auto_connect = 0;
    int user_manual_pairing = 0;

    //manual pairing methods 1: key press triggers
    user_manual_pairing = central_pairing_enable && (rssi > -56);  //button trigger pairing(RSSI threshold, short distance)

    #if (ACL_CENTRAL_SMP_ENABLE)
        central_auto_connect = blc_smp_searchBondingPeripheralDevice_by_PeerMacAddress(pa->adr_type, pa->mac);
    #endif


    if(central_auto_connect || user_manual_pairing){

        /* send create connection command to Controller, trigger it switch to initiating state. After this command, Controller
         * will scan all the ADV packets it received but not report to host, to find the specified device(mac_adr_type & mac_adr),
         * then send a "CONN_REQ" packet, enter to connection state and send a connection complete event
         * (HCI_SUB_EVT_LE_CONNECTION_COMPLETE) to Host*/
        u8 status = blc_ll_createConnection( SCAN_INTERVAL_100MS, SCAN_WINDOW_100MS, INITIATE_FP_ADV_SPECIFY,  \
                                 pa->adr_type, pa->mac, OWN_ADDRESS_PUBLIC, \
                                 CONN_INTERVAL_31P25MS, CONN_INTERVAL_48P75MS, 0, CONN_TIMEOUT_4S, \
                                 0, 0xFFFF);


        if(status == BLE_SUCCESS){ //create connection success

        }
    }
    /*********************** Central Create connection demo code end  *******************************************************/


    return 0;
}


/**
 * @brief      BLE Connection complete event handler
 * @param[in]  p         Pointer point to event parameter buffer.
 * @return
 */
int app_le_connection_complete_event_handle(u8 *p)
{
    hci_le_connectionCompleteEvt_t *pConnEvt = (hci_le_connectionCompleteEvt_t *)p;
    BLT_APP_LOG("le_connection_complete connHandle:%04X mac:%02X %02X %02X %02X %02X %02X",\
            pConnEvt->connHandle,pConnEvt->peerAddr[0],pConnEvt->peerAddr[1],pConnEvt->peerAddr[2],\
            pConnEvt->peerAddr[3],pConnEvt->peerAddr[4],pConnEvt->peerAddr[5]);
    if(pConnEvt->status == BLE_SUCCESS){

        dev_char_info_insert_by_conn_event(pConnEvt);

        if(pConnEvt->role == ACL_ROLE_CENTRAL) // central role, process SMP and SDP if necessary
        {
            #if (ACL_CENTRAL_SMP_ENABLE)
                central_smp_pending = pConnEvt->connHandle; // this connection need SMP
            #endif

            gpio_write(GPIO_LED_GREEN, LED_ON_LEVEL);
        } else {
            gpio_write(GPIO_LED_RED, LED_ON_LEVEL);
        }
    }

    return 0;
}



/**
 * @brief      BLE Disconnection event handler
 * @param[in]  p         Pointer point to event parameter buffer.
 * @return
 */
int     app_disconnect_event_handle(u8 *p)
{
    hci_disconnectionCompleteEvt_t  *pDisConn = (hci_disconnectionCompleteEvt_t *)p;
    BLT_APP_LOG("app Disconnect event connHandle:%04X reason:%02X",pDisConn->connHandle,pDisConn->reason);

    //terminate reason
    if(pDisConn->reason == HCI_ERR_CONN_TIMEOUT){   //connection timeout

    }
    else if(pDisConn->reason == HCI_ERR_REMOTE_USER_TERM_CONN){     //peer device send terminate command on link layer

    }
    //central host disconnect( blm_ll_disconnect(current_connHandle, HCI_ERR_REMOTE_USER_TERM_CONN) )
    else if(pDisConn->reason == HCI_ERR_CONN_TERM_BY_LOCAL_HOST){

    }
    else{

    }

    gpio_write(GPIO_LED_GREEN, !LED_ON_LEVEL);
    gpio_write(GPIO_LED_RED,   !LED_ON_LEVEL);
    gpio_write(GPIO_LED_BLUE,  !LED_ON_LEVEL);
    gpio_write(GPIO_LED_WHITE, !LED_ON_LEVEL);


    /* if previous connection SMP & SDP not finished, clear flag */
    #if (ACL_CENTRAL_SMP_ENABLE)
        if(central_smp_pending == pDisConn->connHandle){
            central_smp_pending = 0;
        }
    #endif

    if(central_disconnect_connhandle == pDisConn->connHandle){  //un_pair disconnection flow finish, clear flag
        central_disconnect_connhandle = 0;
    }

    dev_char_info_delete_by_connhandle(pDisConn->connHandle);


    return 0;
}



//////////////////////////////////////////////////////////
// event call back
//////////////////////////////////////////////////////////
/**
 * @brief      BLE controller event handler call-back.
 * @param[in]  h       event type
 * @param[in]  p       Pointer point to event parameter buffer.
 * @param[in]  n       the length of event parameter.
 * @return
 */
int app_controller_event_callback (u32 h, u8 *p, int n)
{
    (void)n; //unused, remove warning
    if (h &HCI_FLAG_EVENT_BT_STD)       //Controller HCI event
    {
        u8 evtCode = h & 0xff;

        //------------ disconnect -------------------------------------
        if(evtCode == HCI_EVT_DISCONNECTION_COMPLETE)  //connection terminate
        {
            app_disconnect_event_handle(p);
        }
        else if(evtCode == HCI_EVT_LE_META)  //LE Event
        {
            u8 subEvt_code = p[0];

            //------hci le event: le connection complete event---------------------------------
            if (subEvt_code == HCI_SUB_EVT_LE_CONNECTION_COMPLETE)  // connection complete
            {
                app_le_connection_complete_event_handle(p);
            }
            //--------hci le event: le adv report event ----------------------------------------
            else if (subEvt_code == HCI_SUB_EVT_LE_ADVERTISING_REPORT)  // ADV packet
            {
                //after controller is set to scan state, it will report all the adv packet it received by this event

                app_le_adv_report_event_handle(p);
            }
            //------hci le event: le connection update complete event-------------------------------
            else if (subEvt_code == HCI_SUB_EVT_LE_CONNECTION_UPDATE_COMPLETE)  // connection update
            {

            }
        }
    }


    return 0;

}




/**
 * @brief      BLE host event handler call-back.
 * @param[in]  h       event type
 * @param[in]  para    Pointer point to event parameter buffer.
 * @param[in]  n       the length of event parameter.
 * @return
 */
int app_host_event_callback (u32 h, u8 *para, int n)
{
    (void)n; //unused, remove warning
    u8 event = h & 0xFF;

    switch(event)
    {
        case GAP_EVT_SMP_PAIRING_BEGIN:
        {
            //This event is triggered when Periphr rcv pair req or Central rcv pair rsp
            gap_smp_pairingBeginEvt_t *p = (gap_smp_pairingBeginEvt_t *)para;
            /*
             *  JustWorks = 0, PK_Init_Display_Resp_Input = 1, PK_Resp_Display_Init_Input = 2, PK_BOTH_INPUT = 3,OOB_Authentication = 4, Numeric_Comparison = 5,
             */
            tlkapi_send_string_data(APP_SMP_LOG_EN, "[APP][SMP] Pairing begin", &p->tk_method, 1);
        }
        break;

        case GAP_EVT_SMP_PAIRING_SUCCESS:
        {
            gap_smp_pairingSuccessEvt_t* p = (gap_smp_pairingSuccessEvt_t*)para;
            tlkapi_send_string_data(APP_SMP_LOG_EN, "[APP][SMP] pairing success", &p->bonding_result, 1);
            gpio_write(GPIO_LED_BLUE, LED_ON_LEVEL);
        }
        break;

        case GAP_EVT_SMP_PAIRING_FAIL:
        {
            gap_smp_pairingFailEvt_t *pEvt = (gap_smp_pairingFailEvt_t *)para;
            tlkapi_send_string_data(APP_SMP_LOG_EN, "[APP][SMP] pairing fail", &pEvt->connHandle, sizeof(gap_smp_pairingFailEvt_t));
            //pEvt->reason see PAIRING_FAIL_REASON_PASSKEY_ENTRY...

            #if (ACL_CENTRAL_SMP_ENABLE)
                if( dev_char_get_conn_role_by_connhandle(pEvt->connHandle) == ACL_ROLE_CENTRAL){
                    if(central_smp_pending == pEvt->connHandle){
                        central_smp_pending = 0;
                    }
                }
            #endif
        }
        break;

        case GAP_EVT_SMP_TK_DISPLAY:
        {
            gap_smp_TkDisplayEvt_t* pEvt = (gap_smp_TkDisplayEvt_t*)para;
            tlkapi_printf(APP_SMP_LOG_EN, "[APP][SMP] TK display: %d\r\n", pEvt->tk_pincode);
        }
        break;
        #if(SMP_TEST_MODE == SMP_TEST_LEGACY_OOB)
        case GAP_EVT_SMP_TK_REQUEST_OOB:
        {
            gap_smp_TkRequestOOBEvt_t* pEvt = (gap_smp_TkRequestOOBEvt_t*)para;
            appCtrl.connHandle = pEvt->connHandle;
        }
        break;
        #endif
        case GAP_EVT_SMP_TK_REQUEST_PASSKEY:
        {
            gap_smp_TkReqPassKeyEvt_t* pEvt = (gap_smp_TkReqPassKeyEvt_t*)para;
            tlkapi_send_string_data(APP_SMP_LOG_EN, "[APP][SMP] TK request", &pEvt->connHandle, sizeof(gap_smp_TkReqPassKeyEvt_t));
            appCtrl.connHandle = pEvt->connHandle;

            #if (SMP_TEST_MODE == SMP_TEST_LESC_PKI)
                blc_smp_sendKeypressNotify(pEvt->connHandle, KEYPRESS_NTF_PKE_START);
            #endif

            
        }
        break;
        case GAP_EVT_SMP_CONN_ENCRYPTION_DONE:
        {
            gap_smp_connEncDoneEvt_t *pEvt = (gap_smp_connEncDoneEvt_t *)para;
            tlkapi_send_string_data(APP_SMP_LOG_EN, "[APP][SMP] Connection encryption done event", &pEvt->connHandle, sizeof(gap_smp_connEncDoneEvt_t));
            gpio_write(GPIO_LED_WHITE, LED_ON_LEVEL);
        }
        break;

        case GAP_EVT_SMP_SECURITY_PROCESS_DONE:
        {
        #if (ACL_CENTRAL_SMP_ENABLE)
            gap_smp_securityProcessDoneEvt_t* pEvt = (gap_smp_securityProcessDoneEvt_t*)para;
            tlkapi_send_string_data(APP_SMP_LOG_EN, "[APP][SMP] Security process done event", &pEvt->connHandle, sizeof(gap_smp_connEncDoneEvt_t));

            if( dev_char_get_conn_role_by_connhandle(pEvt->connHandle) == ACL_ROLE_CENTRAL){

                if(central_smp_pending == pEvt->connHandle){
                    central_smp_pending = 0;
                }

            }
        #endif
        }
        break;

#if (SMP_TEST_MODE == SMP_TEST_LESC_OOB)
    case GAP_EVT_SMP_REQUEST_SCOOB_DATA:
    {
        gap_smp_requestScOobDataEvt_t* pEvt = (gap_smp_requestScOobDataEvt_t*)para;
        tlkapi_send_string_data(APP_SMP_LOG_EN, "[APP][SMP] SC OOB request data", &pEvt->connHandle, sizeof(gap_smp_requestScOobDataEvt_t));

        appScOobCtrl.acl_handle = pEvt->connHandle;
        //appScOobCtrl.scoob_remote_used = pEvt->scOobRemoteUsed; //needless set
        appScOobCtrl.scoob_local_used = pEvt->scOobLocalUsed;
        tlkapi_printf(APP_SMP_LOG_EN, "[APP][SMP] SC OOB scOobLocalUsed %d", pEvt->scOobLocalUsed);
        tlkapi_printf(APP_SMP_LOG_EN, "[APP][SMP] SC OOB scOobRemoteUsed %d", pEvt->scOobRemoteUsed);

        if(appScOobCtrl.scoob_local_used){
            uart_data_t uart_tx_data;
            uart_tx_data.len = sizeof(smp_sc_oob_data_t);
            memcpy(uart_tx_data.data, &appScOobCtrl.scoob_local, sizeof(smp_sc_oob_data_t));
            uart_send_dma(UART_ID, (u8 *)(&uart_tx_data.data), uart_tx_data.len);
            tlkapi_send_string_data(APP_SMP_LOG_EN, "[APP][SMP] Send Local SC OOB data-c(be) (by UART simulate OOB)", appScOobCtrl.scoob_local.confirm, 16);
            tlkapi_send_string_data(APP_SMP_LOG_EN, "                      SC OOB data-r(be) (by UART simulate OOB)", appScOobCtrl.scoob_local.random, 16);

        }
        else{
            tlkapi_printf(APP_SMP_LOG_EN, "[APP][SMP] local SC OOB data not used");
        }

        if(appScOobCtrl.scoob_remote_used && !appScOobCtrl.scoob_remote_geted){
            tlkapi_printf(APP_SMP_LOG_EN, "[APP][SMP] SC OOB not receive remote data, waiting");
//          blc_smp_cancel_auth(appScOobCtrl.acl_handle);

            /* Waiting SC OOB remote data from UART */
            appScOobCtrl.scoob_remote_waiting = TRUE;
        }
        else if(appScOobCtrl.scoob_local_used || appScOobCtrl.scoob_remote_used){
            appScOobCtrl.scoob_remote_geted = FALSE; //clear
            blc_smp_setScOobData(appScOobCtrl.acl_handle, &appScOobCtrl.scoob_local, &appScOobCtrl.scoob_local_key,
                                    &appScOobCtrl.scoob_remote);
        }
    }
    break;
#endif

#if (SMP_TEST_MODE == SMP_TEST_LESC_NC)
        case GAP_EVT_SMP_TK_NUMERIC_COMPARE:
        {
            gap_smp_TkDisplayEvt_t* pEvt = (gap_smp_TkDisplayEvt_t*)para;
            tlkapi_printf(APP_SMP_LOG_EN, "[APP][SMP] TK display: %d\r\n", pEvt->tk_pincode);
        }
        break;
#endif

        case GAP_EVT_ATT_EXCHANGE_MTU:
        {

        }
        break;

        case GAP_EVT_GATT_HANDLE_VALUE_CONFIRM:
        {

        }
        break;

        default:
        break;
    }

    return 0;
}



#define         HID_HANDLE_CONSUME_REPORT           25
#define         HID_HANDLE_KEYBOARD_REPORT          29
#define         AUDIO_HANDLE_MIC                    52
#define         OTA_HANDLE_DATA                     48

/**
 * @brief      BLE GATT data handler call-back.
 * @param[in]  connHandle     connection handle.
 * @param[in]  pkt             Pointer point to data packet buffer.
 * @return
 */
int app_gatt_data_handler (u16 connHandle, u8 *pkt)
{
    if( dev_char_get_conn_role_by_connhandle(connHandle) == ACL_ROLE_CENTRAL )   //GATT data for Central
    {
        rf_packet_att_t *pAtt = (rf_packet_att_t*)pkt;

        dev_char_info_t* dev_info = dev_char_info_search_by_connhandle (connHandle);
        if(dev_info)
        {
            //-------   user process ------------------------------------------------
            if(pAtt->opcode == ATT_OP_HANDLE_VALUE_NOTI)  //peripheral handle notify
            {

            }
            else if (pAtt->opcode == ATT_OP_HANDLE_VALUE_IND)
            {

            }
        }

        if(!(pAtt->opcode & 0x01)){
            switch(pAtt->opcode){
                case ATT_OP_FIND_INFO_REQ:
                case ATT_OP_FIND_BY_TYPE_VALUE_REQ:
                case ATT_OP_READ_BY_TYPE_REQ:
                case ATT_OP_READ_BY_GROUP_TYPE_REQ:
                    blc_gatt_pushErrResponse(connHandle, pAtt->opcode, pAtt->handle, ATT_ERR_ATTR_NOT_FOUND);
                    break;
                case ATT_OP_READ_REQ:
                case ATT_OP_READ_BLOB_REQ:
                case ATT_OP_READ_MULTI_REQ:
                case ATT_OP_WRITE_REQ:
                case ATT_OP_PREPARE_WRITE_REQ:
                    blc_gatt_pushErrResponse(connHandle, pAtt->opcode, pAtt->handle, ATT_ERR_INVALID_HANDLE);
                    break;
                case ATT_OP_EXECUTE_WRITE_REQ:
                case ATT_OP_HANDLE_VALUE_CFM:
                case ATT_OP_WRITE_CMD:
                case ATT_OP_SIGNED_WRITE_CMD:
                    //ignore
                    break;
                default://no action
                    break;
            }
        }
    }
    else{   //GATT data for Peripheral

    }


    return 0;
}


///////////////////////////////////////////

/**
 * @brief       user initialization when MCU power on or wake_up from deepSleep mode
 * @param[in]   none
 * @return      none
 */
_attribute_no_inline_ void user_init_normal(void)
{
//////////////////////////// basic hardware Initialization  Begin //////////////////////////////////
    /* random number generator must be initiated here( in the beginning of user_init_normal).
     * When deepSleep retention wakeUp, no need initialize again */
    random_generator_init();

    #if (TLKAPI_DEBUG_ENABLE)
        tlkapi_debug_init();
        blc_debug_enableStackLog(STK_LOG_NONE);
    #endif

    blc_readFlashSize_autoConfigCustomFlashSector();

    /* attention that this function must be called after "blc readFlashSize_autoConfigCustomFlashSector" !!!*/
    blc_app_loadCustomizedParameters_normal();
//////////////////////////// basic hardware Initialization  End /////////////////////////////////


//////////////////////////// BLE stack Initialization  Begin //////////////////////////////////

    u8  mac_public[6];
    u8  mac_random_static[6];
    
    blc_initMacAddress(flash_sector_mac_address, mac_public, mac_random_static);


    //////////// LinkLayer Initialization  Begin /////////////////////////
    blc_ll_initBasicMCU();

    blc_ll_initStandby_module(mac_public);

    blc_ll_initLegacyAdvertising_module();

    blc_ll_initLegacyScanning_module();

    blc_ll_initLegacyInitiating_module();

    blc_ll_initAclConnection_module();
    blc_ll_initAclCentralRole_module();
    blc_ll_initAclPeriphrRole_module();


    blc_ll_setMaxConnectionNumber(ACL_CENTRAL_MAX_NUM, ACL_PERIPHR_MAX_NUM);

    blc_ll_setAclConnMaxOctetsNumber(ACL_CONN_MAX_RX_OCTETS, ACL_CENTRAL_MAX_TX_OCTETS, ACL_PERIPHR_MAX_TX_OCTETS);

    /* all ACL connection share same RX FIFO */
    blc_ll_initAclConnRxFifo(app_acl_rx_fifo, ACL_RX_FIFO_SIZE, ACL_RX_FIFO_NUM);
    /* ACL Central TX FIFO */
    blc_ll_initAclCentralTxFifo(app_acl_cen_tx_fifo, ACL_CENTRAL_TX_FIFO_SIZE, ACL_CENTRAL_TX_FIFO_NUM, ACL_CENTRAL_MAX_NUM);
    /* ACL Peripheral TX FIFO */
    blc_ll_initAclPeriphrTxFifo(app_acl_per_tx_fifo, ACL_PERIPHR_TX_FIFO_SIZE, ACL_PERIPHR_TX_FIFO_NUM, ACL_PERIPHR_MAX_NUM);

    blc_ll_setAclCentralBaseConnectionInterval(CONN_INTERVAL_31P25MS);


    //////////// LinkLayer Initialization  End /////////////////////////



    //////////// HCI Initialization  Begin /////////////////////////
    blc_hci_registerControllerDataHandler (blc_l2cap_pktHandler);

    blc_hci_registerControllerEventHandler(app_controller_event_callback); //controller hci event to host all processed in this func

    //bluetooth event
    blc_hci_setEventMask_cmd (HCI_EVT_MASK_DISCONNECTION_COMPLETE);

    //bluetooth low energy(LE) event
    blc_hci_le_setEventMask_cmd(        HCI_LE_EVT_MASK_CONNECTION_COMPLETE  \
                                    |   HCI_LE_EVT_MASK_ADVERTISING_REPORT \
                                    |   HCI_LE_EVT_MASK_CONNECTION_UPDATE_COMPLETE);


    u8 error_code = blc_contr_checkControllerInitialization();
    if(error_code != INIT_SUCCESS){
        /* It's recommended that user set some UI alarm to know the exact error, e.g. LED shine, print log */
        write_log32(0x88880000 | error_code);
        #if (TLKAPI_DEBUG_ENABLE)
            tlkapi_send_string_data(APP_LOG_EN, "[APP][INI] Controller INIT ERROR", &error_code, 1);
            while(1){
                tlkapi_debug_handler();
            }
        #else
            while(1);
        #endif
    }
    //////////// HCI Initialization  End /////////////////////////


    //////////// Host Initialization  Begin /////////////////////////
    /* Host Initialization */
    /* GAP initialization must be done before any other host feature initialization !!! */
    blc_gap_init();

    /* L2CAP data buffer Initialization */
    blc_l2cap_initAclCentralBuffer(app_cen_l2cap_rx_buf, CENTRAL_L2CAP_BUFF_SIZE, NULL, 0);
    blc_l2cap_initAclPeripheralBuffer(app_per_l2cap_rx_buf, PERIPHR_L2CAP_BUFF_SIZE, app_per_l2cap_tx_buf, PERIPHR_L2CAP_BUFF_SIZE);

    blc_att_setCentralRxMtuSize(CENTRAL_ATT_RX_MTU); ///must be placed after "blc_gap_init"
    blc_att_setPeripheralRxMtuSize(PERIPHR_ATT_RX_MTU);   ///must be placed after "blc_gap_init"

    /* GATT Initialization */
    my_gatt_init();

    blc_gatt_register_data_handler(app_gatt_data_handler);

    /* SMP Initialization */
    #if (ACL_PERIPHR_SMP_ENABLE || ACL_CENTRAL_SMP_ENABLE)
        blc_smp_configPairingSecurityInfoStorageAddressAndSize(flash_sector_smp_storage, FLASH_SMP_PAIRING_MAX_SIZE);
    #endif

    #if (ACL_PERIPHR_SMP_ENABLE)  //Slave SMP Enable
        blc_smp_setSecurityLevel_slave(Unauthenticated_Pairing_with_Encryption);  //LE_Security_Mode_1_Level_2
    #else
        blc_smp_setSecurityLevel_slave(No_Security);
    #endif

    #if (ACL_CENTRAL_SMP_ENABLE) //Master SMP Enable
        blc_smp_setSecurityLevel_master(Unauthenticated_Pairing_with_Encryption);  //LE_Security_Mode_1_Level_2
    #else
        blc_smp_setSecurityLevel_master(No_Security);
        user_master_host_pairing_management_init();         //Telink referenced pairing&bonding without standard pairing in BLE Spec
    #endif

#if   (SMP_TEST_MODE == SMP_TEST_NOT_SUPPORT)   //LE_Security_Mode_1_Level_1 --- No_Encryption
    blc_smp_setSecurityLevel(LE_Security_Mode_1_Level_1);
#elif (SMP_TEST_MODE == SMP_TEST_LEGACY_JW)     //LE_Security_Mode_1_Level_2 --- LG_JUST_WORKS
    blc_smp_setSecurityLevel(LE_Security_Mode_1_Level_2);
    blc_smp_setSecurityParameters(Bondable_Mode, 0, LE_Legacy_Pairing,    0, 0, IO_CAPABILITY_NO_INPUT_NO_OUTPUT);
#elif (SMP_TEST_MODE == SMP_TEST_LESC_JW)       //LE_Security_Mode_1_Level_2 --- LG_JUST_WORKS/SC_JUST_WORKS
    blc_smp_setSecurityLevel(LE_Security_Mode_1_Level_2);
    blc_smp_setSecurityParameters(Bondable_Mode, 0, LE_Secure_Connection, 0, 0, IO_CAPABILITY_NO_INPUT_NO_OUTPUT);
    blc_smp_setEcdhDebugMode(debug_mode); //only one role can use debug mode,if peripheral and central both enable debug mode,SMP will fail.
#elif (SMP_TEST_MODE == SMP_TEST_LEGACY_PKD)    //LE_Security_Mode_1_Level_3 --- LG_PASSKEY_ENTRY_DISPLAY
    blc_smp_setSecurityLevel(LE_Security_Mode_1_Level_3);
    blc_smp_setSecurityParameters(Bondable_Mode, 1, LE_Legacy_Pairing,    0, 0, IO_CAPABILITY_DISPLAY_ONLY);
    blc_smp_setDefaultPinCode(123456);
#elif (SMP_TEST_MODE == SMP_TEST_LEGACY_PKI)    //LE_Security_Mode_1_Level_3 --- LG_PASSKEY_ENTRY_INPUT
    blc_smp_setSecurityLevel(LE_Security_Mode_1_Level_3);
    blc_smp_setSecurityParameters(Bondable_Mode, 1, LE_Legacy_Pairing,    0, 0, IO_CAPABILITY_KEYBOARD_ONLY);
#elif (SMP_TEST_MODE == SMP_TEST_LEGACY_OOB)    //LE_Security_Mode_1_Level_3 --- LG_OOB
    blc_smp_setSecurityLevel(LE_Security_Mode_1_Level_3);
    blc_smp_setSecurityParameters(Bondable_Mode, 1, LE_Legacy_Pairing,    1, 0, IO_CAPABILITY_KEYBOARD_ONLY);
#elif (SMP_TEST_MODE == SMP_TEST_LESC_PKD)      //LE_Security_Mode_1_Level_4 --- SC_PASSKEY_ENTRY_DISPLAY
    blc_smp_setSecurityLevel(LE_Security_Mode_1_Level_4);
    blc_smp_setSecurityParameters(Bondable_Mode, 1, LE_Secure_Connection, 0, 0, IO_CAPABILITY_DISPLAY_ONLY);
    blc_smp_setEcdhDebugMode(debug_mode); //only one role can use debug mode,if peripheral and central both enable debug mode,SMP will fail.
    blc_smp_setDefaultPinCode(123456);
#elif (SMP_TEST_MODE == SMP_TEST_LESC_NC)       //LE_Security_Mode_1_Level_4 --- SC_PASSKEY_ENTRY_DISPLAY/SC_NUMERIC_COMPARISON
    blc_smp_setSecurityLevel(LE_Security_Mode_1_Level_4);
    blc_smp_setSecurityParameters(Bondable_Mode, 1, LE_Secure_Connection, 0, 0, IO_CAPABILITY_DISPLAY_YES_NO);
    blc_smp_setEcdhDebugMode(debug_mode); //only one role can use debug mode,if peripheral and central both enable debug mode,SMP will fail.
    blc_smp_setDefaultPinCode(123456);
#elif (SMP_TEST_MODE == SMP_TEST_LESC_PKI)      //LE_Security_Mode_1_Level_4 --- SC_PASSKEY_ENTRY_INPUT
    blc_smp_setSecurityLevel(LE_Security_Mode_1_Level_4);
    blc_smp_setSecurityParameters(Bondable_Mode, 1, LE_Secure_Connection, 0, 0, IO_CAPABILITY_KEYBOARD_ONLY);
    blc_smp_setEcdhDebugMode(debug_mode); //only one role can use debug mode,if peripheral and central both enable debug mode,SMP will fail.
    blc_smp_enableKeypress(1);
#elif (SMP_TEST_MODE == SMP_TEST_LESC_OOB)      //LE_Security_Mode_1_Level_4 --- SC_OOB
    blc_smp_setSecurityLevel(LE_Security_Mode_1_Level_4);
    blc_smp_setSecurityParameters(Bondable_Mode, 0, LE_Secure_Connection, 0, 0, IO_CAPABILITY_NO_INPUT_NO_OUTPUT);
    blc_smp_setEcdhDebugMode(debug_mode); //only one role can use debug mode,if peripheral and central both enable debug mode,SMP will fail.

    /* if use remote SC OOB data */
    appScOobCtrl.acl_handle = 0;
    appScOobCtrl.scoob_remote_used = TRUE; //FALSE;
    appScOobCtrl.scoob_remote_geted = FALSE; //clear
    appScOobCtrl.scoob_remote_waiting = FALSE; //clear
    blc_smp_enableOobAuthentication(appScOobCtrl.scoob_remote_used ? 1 : 0);

    /* generate local SC OOB data */
    if(!blc_smp_generateScOobData(&appScOobCtrl.scoob_local, &appScOobCtrl.scoob_local_key)){
        tlkapi_printf(APP_SMP_LOG_EN, "Generate local SC OOB data failed");
    }
    else{
        tlkapi_send_string_data(APP_SMP_LOG_EN, "SC OOB data-confirm value", appScOobCtrl.scoob_local.confirm, 16);
        tlkapi_send_string_data(APP_SMP_LOG_EN, "SC OOB data-random value", appScOobCtrl.scoob_local.random, 16);

    }
    tlkapi_printf(APP_SMP_LOG_EN, "SC OOB Initialize");
#endif


    blc_smp_smpParamInit();


    //host(GAP/SMP/GATT/ATT) event process: register host event callback and set event mask
    blc_gap_registerHostEventHandler( app_host_event_callback );
    blc_gap_setEventMask( GAP_EVT_MASK_SMP_PAIRING_BEGIN            |
                          GAP_EVT_MASK_SMP_PAIRING_SUCCESS          |
                          GAP_EVT_MASK_SMP_PAIRING_FAIL             |
                          GAP_EVT_MASK_SMP_SECURITY_PROCESS_DONE    |
                          GAP_EVT_MASK_SMP_TK_DISPLAY               |
                        #if (SMP_TEST_MODE == SMP_TEST_LESC_NC)
                          GAP_EVT_MASK_SMP_TK_NUMERIC_COMPARE       |
                        #endif
                        #if (SMP_TEST_MODE == SMP_TEST_LEGACY_OOB)
                          GAP_EVT_MASK_SMP_TK_REQUEST_OOB           |
                        #endif
                        #if (SMP_TEST_MODE == SMP_TEST_LESC_OOB)
                          GAP_EVT_MASK_SMP_REQUEST_SCOOB_DATA       |
                        #elif (SMP_TEST_MODE == SMP_TEST_LESC_PKI)
                          GAP_EVT_MASK_SMP_KEYPRESS_NOTIFY          |
                        #endif
                          GAP_EVT_MASK_SMP_TK_REQUEST_PASSKEY);
    //////////// Host Initialization  End /////////////////////////

//////////////////////////// BLE stack Initialization  End //////////////////////////////////




//////////////////////////// User Configuration for BLE application ////////////////////////////
    blc_ll_setAdvData(tbl_advData, sizeof(tbl_advData));
    blc_ll_setScanRspData(tbl_scanRsp, sizeof(tbl_scanRsp));
    blc_ll_setAdvParam(ADV_INTERVAL_50MS, ADV_INTERVAL_50MS, ADV_TYPE_CONNECTABLE_UNDIRECTED, OWN_ADDRESS_PUBLIC, 0, NULL, BLT_ENABLE_ADV_ALL, ADV_FP_NONE);
    blc_ll_setAdvEnable(BLC_ADV_ENABLE);  //ADV enable

    blc_ll_setScanParameter(SCAN_TYPE_PASSIVE, SCAN_INTERVAL_100MS, SCAN_WINDOW_50MS, OWN_ADDRESS_PUBLIC, SCAN_FP_ALLOW_ADV_ANY);
    blc_ll_setScanEnable (BLC_SCAN_ENABLE, DUP_FILTER_DISABLE);

    rf_set_power_level_index(RF_POWER_P3dBm);

    tlkapi_send_string_data(APP_LOG_EN, "[APP][INI] FEATURE_SMP init", 0, 0);
////////////////////////////////////////////////////////////////////////////////////////////////

#if APP_MITM_EN
    appCtrl.connHandle = 0;
    UART_Init(appCtrl.dat, sizeof(appCtrl.dat));
#endif
}



/**
 * @brief       user initialization when MCU wake_up from deepSleep_retention mode
 * @param[in]   none
 * @return      none
 */
void user_init_deepRetn(void)
{

}


/////////////////////////////////////////////////////////////////////
// main loop flow
/////////////////////////////////////////////////////////////////////

/**
 * @brief     BLE main idle loop
 * @param[in]  none.
 * @return     none.
 */
int main_idle_loop (void)
{

    ////////////////////////////////////// BLE entry /////////////////////////////////
    blc_sdk_main_loop();


    ////////////////////////////////////// Debug entry /////////////////////////////////
    #if (TLKAPI_DEBUG_ENABLE)
        tlkapi_debug_handler();
    #endif

    ////////////////////////////////////// UI entry /////////////////////////////////
    #if (UI_KEYBOARD_ENABLE)
        proc_keyboard (0, 0, 0);
    #endif

    proc_central_role_unpair();

    return 0; //must return 0 due to SDP flow
}



/**
 * @brief     BLE main loop
 * @param[in]  none.
 * @return     none.
 */
_attribute_no_inline_ void main_loop (void)
{
    main_idle_loop ();

}

#endif //end of (FEATURE_TEST_MODE == ...)



