/********************************************************************************************************
 * @file    hci_tr_h5.c
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
#include "hci_tr_h5.h"
#include "hci_tr_def.h"
#include "hci_slip.h"
#include "hci_tr.h"
#include "hci_h5.h"


#if HCI_TR_EN

    #define HCI_Tr_H5TimerEnable()  hciH5TrCb.flushTimer = clock_time() | 1
    #define HCI_Tr_H5TimerDisable() hciH5TrCb.flushTimer = 0
    #define HCI_Tr_H5FlushHandler()                                                                        \
        do {                                                                                               \
            if (hciH5TrCb.flushTimer && clock_time_exceed(hciH5TrCb.flushTimer, HCI_H5_FLUSH_TO * 1000)) { \
                hciH5TrCb.flushTimer = 0;                                                                  \
                hciH5TrCb.backupCnt  = 0;                                                                  \
            }                                                                                              \
        } while (0)

/*! Backup buffer define. */
static u8 h5TrBackupBuf[HCI_H5_TR_RX_BUF_SIZE];

/*! HCI H5 transport Rx buffer define. */
static u8 h5TrRxBuf[HCI_H5_TR_RX_BUF_SIZE * HCI_H5_TR_RX_BUF_NUM];

/*! HCI H5 transport Rx Fifo define. */
static hci_fifo_t h5TrRxFifo = {
    HCI_H5_TR_RX_BUF_SIZE,
    HCI_H5_TR_RX_BUF_NUM,
    HCI_H5_TR_RX_BUF_NUM - 1,
    0,
    0,
    &h5TrRxBuf[0],
};

/*! HCI H5 transport main control block. */
typedef struct
{
    hci_fifo_t          *pRxFifo;
    HciH5PacketHandler_t HCI_Tr_SlipHandler;
    u8                  *pBackupBuf;
    u32                  flushTimer;
    u8                   backupCnt;
    u8                   align[3];
} HciH5TrCb_t;

static HciH5TrCb_t hciH5TrCb;

void HCI_Tr_DefaultSlipHandler(u8 *pPacket, u32 len)
{
    (void)pPacket;
    (void)len;
}

void                HCI_Tr_H5UartRxIRQHandler(unsigned int *param);
ext_hci_InitTypeDef hci_h5_uart;

/**
 * @brief : H5 protocol initialization.
 * @param : none.
 * @param : none.
 */
void HCI_Tr_H5Init(void)
{
    hciH5TrCb.pRxFifo    = &h5TrRxFifo;
    hciH5TrCb.pBackupBuf = h5TrBackupBuf;
    hciH5TrCb.backupCnt  = 0;
    hciH5TrCb.flushTimer = 0;

    hciH5TrCb.HCI_Tr_SlipHandler = HCI_Tr_DefaultSlipHandler;

    //UART HARDWARE INIT
    memset(&hci_h5_uart, 0, sizeof(ext_hci_InitTypeDef));
    hci_h5_uart.baudrate       = HCI_TR_BAUDRATE;
    hci_h5_uart.tx_Pin         = HCI_TR_TX_PIN; //
    hci_h5_uart.rx_Pin         = HCI_TR_RX_PIN; //
    hci_h5_uart.HwFlowCtl      = 0;
    hci_h5_uart.cts_Pin        = 0;
    hci_h5_uart.rts_Pin        = 0;
    hci_h5_uart.RxCpltCallback = HCI_Tr_H5UartRxIRQHandler;
    hci_h5_uart.TxCpltCallback = NULL;
    ext_hci_uartInit(&hci_h5_uart);

    ext_hci_uartReceData((hciH5TrCb.pRxFifo->p + 4), hciH5TrCb.pRxFifo->size - 4);
}

/**
 * @brief : Register slip handler
 * @param : func    Pointer point to handler function.
 * @param : none.
 */
void HCI_Tr_H5RegisterSlipHandler(HciH5PacketHandler_t func)
{
    ASSERT(func != NULL, HCI_TR_ERR_INVALID_PARAM);

    hciH5TrCb.HCI_Tr_SlipHandler = func;
}

static u8 *HCI_Tr_H5FindSlipHeadFlag(u8 *pPacket, u32 len)
{
    u8 *pBuf = pPacket;
    while (len) {
        if (len >= 2) {
            if (pBuf[0] == 0xC0 && pBuf[1] != 0xC0) {
                return pBuf;
            }
        } else {
            if (pBuf[0] == 0xC0) {
                return pBuf;
            }
        }
        pBuf++;
        len--;
    }

    return NULL;
}

/**
 * @brief : Find slip delimiter
 * @param : pPacket     Pointer point to slip packet buffer.
 * @param : len         the length of data.
 * @param : none.
 */
static u8 *HCI_Tr_H5FindSlipFlag(u8 *pPacket, u32 len)
{
    u8 *pBuf = pPacket;
    while (len) {
        u8 slipFlag = pBuf[0];
        if (slipFlag != 0xC0) {
            pBuf++;
            len--;
            continue;
        }
        return pBuf; // pointer point to 0xC0
    }
    return NULL;
}

int HCI_Tr_H5BackupHandler(u8 *pPacket, u32 len)
{
    u8 *pBuf      = hciH5TrCb.pBackupBuf;
    u16 backupLen = hciH5TrCb.backupCnt;

    if (backupLen == 0) {
        ASSERT(len < HCI_H5_TR_RX_BUF_SIZE, HCI_TR_ERR_TR_BACKUP_BUF);

        memcpy(pBuf, pPacket, len);
        hciH5TrCb.backupCnt += len;
        HCI_Tr_H5TimerEnable();

        TR_TRACK_INFO("Backup start: ");
        HCI_TRACK_DATA(pBuf, hciH5TrCb.backupCnt);
        return len;
    }

    u16 cnt = 0;
    while (len) {
        if (*pPacket == 0xC0) {
            ASSERT(hciH5TrCb.backupCnt + 1 < HCI_H5_TR_RX_BUF_SIZE, HCI_TR_ERR_TR_BACKUP_BUF);

            if (hciH5TrCb.backupCnt + 1 > HCI_H5_TR_RX_BUF_SIZE) {
                //cnt++;
                hciH5TrCb.backupCnt = 0;
                HCI_Tr_H5TimerDisable();
                return cnt; //discard
            }

            if (hciH5TrCb.backupCnt == 1) {
                cnt++;
                pPacket++;
                len--;
                continue;
            } else {
                ASSERT(hciH5TrCb.backupCnt + 1 < HCI_H5_TR_RX_BUF_SIZE, HCI_TR_ERR_TR_BACKUP_BUF);
                if (hciH5TrCb.backupCnt + 1 > HCI_H5_TR_RX_BUF_SIZE) {
                    //cnt++;
                    hciH5TrCb.backupCnt = 0;
                    HCI_Tr_H5TimerDisable();
                    return cnt; //discard
                }

                memcpy(pBuf + hciH5TrCb.backupCnt, pPacket, 1);
                hciH5TrCb.backupCnt += 1;
                cnt++;

                TR_TRACK_INFO("Rx data2: ");
                HCI_TRACK_DATA(hciH5TrCb.pBackupBuf, hciH5TrCb.backupCnt);

                hciH5TrCb.HCI_Tr_SlipHandler(hciH5TrCb.pBackupBuf, hciH5TrCb.backupCnt);

                hciH5TrCb.backupCnt = 0;
                HCI_Tr_H5TimerDisable();
            }
            return cnt;
        } else {
            ASSERT(hciH5TrCb.backupCnt + 1 < HCI_H5_TR_RX_BUF_SIZE, HCI_TR_ERR_TR_BACKUP_BUF);
            if (hciH5TrCb.backupCnt + 1 > HCI_H5_TR_RX_BUF_SIZE) {
                //cnt++;
                hciH5TrCb.backupCnt = 0;
                HCI_Tr_H5TimerDisable();
                return cnt;                                 //discard
            }

            memcpy(pBuf + hciH5TrCb.backupCnt, pPacket, 1); //TODO: Check len
            hciH5TrCb.backupCnt += 1;
            cnt++;
            HCI_Tr_H5TimerEnable();
        }

        pPacket++;
        len--;
    }

    return cnt;
}

/**
 * @brief : H5 transport rx handler.
 * @param : none.
 * @param : none.
 */
void HCI_Tr_H5RxHandler(void)
{
    /* H5 Flush handle */
    HCI_Tr_H5FlushHandler();

    hci_fifo_t *pFifo = hciH5TrCb.pRxFifo;
    if (pFifo->wptr == pFifo->rptr) {
        return;
    }

    while (pFifo->wptr != pFifo->rptr) {
        u8 *pPacket = pFifo->p + (pFifo->rptr & pFifo->mask) * pFifo->size;
        u32 len     = 0;
        BSTREAM_TO_UINT32(len, pPacket);

        TR_TRACK_INFO("UART data: ");
        HCI_TRACK_DATA(pPacket, len);

        while (len) {
            if (hciH5TrCb.backupCnt) {
                u8 res = HCI_Tr_H5BackupHandler(pPacket, len);
                len -= res;
                pPacket += res;
            }

            u8 *pBuf  = NULL;
            u8 *pBuf2 = NULL;
            pBuf      = HCI_Tr_H5FindSlipHeadFlag(pPacket, len);
            len -= (pBuf - pPacket);
            pPacket = pBuf;

            if (pBuf) {
                u16 tmpLen = pBuf + 1 - pPacket;
                pBuf2      = HCI_Tr_H5FindSlipFlag(pBuf + 1, len - tmpLen);
                if (pBuf2 == NULL || len == 1) {
                    HCI_Tr_H5BackupHandler(pBuf, len - (pBuf - pPacket));
                    len = 0;
                    break;
                }

                u16 slipLen = pBuf2 + 1 - pBuf;
                if (slipLen < 6) {
                    pPacket += slipLen;
                    len -= slipLen;
                    continue;
                }

                TR_TRACK_INFO("Slip1: ");
                HCI_TRACK_DATA(pBuf, slipLen);

                hciH5TrCb.HCI_Tr_SlipHandler(pBuf, slipLen);
                pPacket += slipLen;
                len -= slipLen;
            } else {
                len = 0;
                break;
            }
        }

        pFifo->rptr++;
    }
}

/**
 * @brief : H5 transport IRQ handler.
 * @param : none.
 * @param : none.
 */
_attribute_ram_code_ void HCI_Tr_H5UartRxIRQHandler(unsigned int *param)
{
    (void)*param;
    /* Get the length of Rx data */
    u32         rxLen   = 0;
    hci_fifo_t *pRxFifo = hciH5TrCb.pRxFifo;
    u8         *p       = pRxFifo->p + (pRxFifo->wptr & pRxFifo->mask) * pRxFifo->size;

    if ((u8)(pRxFifo->wptr - pRxFifo->rptr) >= pRxFifo->num) {
        ext_hci_uartReceData((p + 4), pRxFifo->size - 4); //[!!important]
        tlkapi_send_string_data(DBG_HCI_TR, "[I] UART Rx overflow", 0, 0);

        return;                                           //have no memory.
    }
    BSTREAM_TO_UINT32(rxLen, p);
    if (rxLen) {
        pRxFifo->wptr++;
        p = pRxFifo->p + (pRxFifo->wptr & pRxFifo->mask) * pRxFifo->size;
    }
    ext_hci_uartReceData((p + 4), pRxFifo->size - 4); //[!!important]
}

#endif                                                /* End of HCI_TR_EN */
