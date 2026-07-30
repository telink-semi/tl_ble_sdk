/********************************************************************************************************
 * @file    hci_tr.c
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
#include "hci_tr.h"
#include "hci_tr_def.h"
#include "hci_tr_h4.h"
#include "hci_tr_h5.h"
#include "hci_slip.h"
#include "hci_h5.h"
#include "stack/ble/controller/ble_controller.h"

#if (CHIP_TYPE == CHIP_TYPE_TL322X)
#if defined(HCI_INTERFACE) && defined(HCI_SHAREMEMORY) && (HCI_INTERFACE==HCI_SHAREMEMORY)
#include"stack/multicore_comm/service/service_n22.h"
#endif
#endif

#if HCI_TR_EN


void HCI_Handler(void);

/**
 * @brief : HCI transport initialization.
 * @param : none.
 * @param : none.
 */
void HCI_TransportInit(void)
{
    #if HCI_TR_MODE == HCI_TR_H4
    HCI_Tr_H4Init(&bltHci_rxfifo);

    #elif HCI_TR_MODE == HCI_TR_H5
    HCI_Tr_H5Init();
    HCI_Slip_Init();
    HCI_H5_Init(&bltHci_rxfifo, &bltHci_txfifo);

    #elif HCI_TR_MODE == HCI_TR_USB
        //TODO:

    #endif
}

    /**
 * @brief : HCI transport main loop.
 * @param : none.
 * @param : none.
 */
#if (TIFS_VARIATION_WORKAROUND_MLP_CODE_IN_RAM)
_attribute_ram_code_
#endif
void HCI_TransportPoll(void)
{
    #if HCI_TR_MODE == HCI_TR_H4
    HCI_Tr_H4RxHandler();
    HCI_Handler();

    #elif HCI_TR_MODE == HCI_TR_H5
    HCI_Tr_H5RxHandler();
    HCI_H5_Poll();
    HCI_Handler();

    #elif HCI_TR_MODE == HCI_TR_USB
        //TODO:

    #endif
}


#if (TIFS_VARIATION_WORKAROUND_MLP_CODE_IN_RAM)
_attribute_ram_code_
#endif
void HCI_RxHandler(void)
{
    #if HCI_TR_MODE == HCI_TR_H4 || HCI_TR_MODE == HCI_TR_H5
    if (bltHci_rxfifo.wptr == bltHci_rxfifo.rptr) {
        return; //have no data
    }

    u8 *p = bltHci_rxfifo.p + (bltHci_rxfifo.rptr & bltHci_rxfifo.mask) * bltHci_rxfifo.size;

    if (p) {
        blc_hci_handler(&p[0], 0); //the second parameter is not used.
        bltHci_rxfifo.rptr++;
        return;
    }

    #elif HCI_TR_MODE == HCI_TR_USB
        //TODO:

    #endif
}

#if (TIFS_VARIATION_WORKAROUND_MLP_CODE_IN_RAM)
_attribute_ram_code_
#endif
void HCI_TxHandler(void)
{
    #if HCI_TR_MODE == HCI_TR_H4

    static u8 uartTxBuf[4 + HCI_TR_TX_BUF_SIZE] = {0}; //[!!important]

    #if (defined(HCI_INTERFACE) && (CHIP_TYPE == CHIP_TYPE_TL322X))
    {
        #if (HCI_INTERFACE == HCI_UART)
        {
            if (!ext_hci_getTxCompleteDone()) {
                return;
            }
        }
        #endif
    }
    #else
    {
        if (!ext_hci_getTxCompleteDone()) {
            return;
        }
    }
    #endif

    u8 *pBuf = uartTxBuf;

    u8        *p = NULL;
    hci_type_t type;
    if (bltHci_outIsofifo.wptr != bltHci_outIsofifo.rptr) { //Priority of HCI ISO DATA higher than HCI ACL data

        type = HCI_TYPE_ISO_DATA;
        p    = bltHci_outIsofifo.p + (bltHci_outIsofifo.rptr & bltHci_outIsofifo.mask) * bltHci_outIsofifo.size;
    } else if (bltHci_txfifo.wptr != bltHci_txfifo.rptr) {
        type = HCI_TYPE_ACL_DATA;
        p    = bltHci_txfifo.p + (bltHci_txfifo.rptr & bltHci_txfifo.mask) * bltHci_txfifo.size;
    }


    if (p) {
        u32 len = 0;
        BSTREAM_TO_UINT16(len, p);

        #if (TIFS_VARIATION_WORKAROUND_MLP_CODE_IN_RAM)
        smemcpy(pBuf, p, len);
        #else
        memcpy(pBuf, p, len);
        #endif

        ASSERT(len <= HCI_TX_FIFO_SIZE, HCI_TR_ERR_TR_TX_BUF);
    #if (defined(HCI_INTERFACE)&& (CHIP_TYPE == CHIP_TYPE_TL322X))
        #if (HCI_INTERFACE==HCI_UART)
            if (ext_hci_uartSendData(pBuf, len)) {
        #else
            if(!mcc_n22_hci_send_msg(pBuf, len)){
        #endif
    #else
        if (ext_hci_uartSendData(pBuf, len)) {
    #endif
            if (type == HCI_TYPE_ACL_DATA) {
                bltHci_txfifo.rptr++;
            } else {
                bltHci_outIsofifo.rptr++;
            }

            return;
        }
    }
    else
    {
       extern u32 btc_hci_vendor_change_baudrate ;  //hci vendor has internally defined ,
        if(btc_hci_vendor_change_baudrate != 0)
       {
           void HCI_Tr_H4ChangeBaudrate(hci_fifo_t *pHciRxFifo,u32 baudrate);
           HCI_Tr_H4ChangeBaudrate(&bltHci_rxfifo,btc_hci_vendor_change_baudrate);
           btc_hci_vendor_change_baudrate = 0;
        }
     }


    #elif HCI_TR_MODE == HCI_TR_H5
        //TX handle has been taken over by H5 protocol.

    #elif HCI_TR_MODE == HCI_TR_USB
        //TODO:

    #endif
}

#if (TIFS_VARIATION_WORKAROUND_MLP_CODE_IN_RAM)
_attribute_ram_code_
#endif
void HCI_Handler(void)
{
    #if HCI_TR_MODE == HCI_TR_H4
    HCI_RxHandler();
    HCI_TxHandler();

    #elif HCI_TR_MODE == HCI_TR_H5
    HCI_RxHandler();
        //HCI_TxHandler(); //This is not needed because the H5 takes over the handling of the HCI TX FIFO.

    #elif HCI_TR_MODE == HCI_TR_USB
        //TODO:
    #endif
}

#endif /* End of HCI_TR_EN */
