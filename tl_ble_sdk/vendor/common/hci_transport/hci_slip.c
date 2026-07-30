/********************************************************************************************************
 * @file    hci_slip.c
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
#include "hci_slip.h"
#include "hci_tr_h5.h"
#include "hci_tr_def.h"
#include "hci_h5.h"
#include "hci_tr.h"

#if HCI_TR_EN

/*! Slip decode buffer define. */
u8 slipDecodeBuf[HCI_SLIP_DECODE_BUF_SIZE];

/*! Slip encode buffer define. */
u8 slipEncodeBuf[4 + HCI_SLIP_ENCODE_BUF_SIZE]; /*!< DMAlen=4 */

typedef struct
{
    u16 escapeSeq;
    u16 unencoded;
} SlipEscapeMap_t;

/*! Slip escape table. */
static const SlipEscapeMap_t slipEscapeTbl[] = {
    {0xDCDB, 0xC0},
    {0xDDDB, 0xDB},
    {0xDEDB, 0x11}, /*!< Only valid when OOF Software Flow Control is enabled*/
    {0xDFDB, 0x13}, /*!< Only valid when OOF Software Flow Control is enabled*/
};

/*! Slip main control block. */
typedef struct
{
    HciH5PacketHandler_t HCI_H5_PktHandler;
    u8                  *pDecodeBuf;
    u8                  *pEncodeBuf;
    u16                  decodeLen;
    u16                  encodeLen;
    u8                   oofEnable; /*!< OOF Software Flow Control. */
    u8                   recvd[3];
} HciH5SlipCb_t;

static HciH5SlipCb_t hciSlipCb;

/**
 * @brief : Register H5 protocol handler.
 * @param : func    Pointer point to handler function.
 * @param : none.
 */
void HCI_Slip_RegisterPktHandler(HciH5PacketHandler_t func)
{
    if (func == NULL) {
        return;
    }
    hciSlipCb.HCI_H5_PktHandler = func;
}

static u8 HCI_Slip_GetUnencoded(u16 escapeSeq)
{
    for (unsigned int i = 0; i < COUNTOF(slipEscapeTbl); i++) {
        if (escapeSeq == slipEscapeTbl[i].escapeSeq) {
            return slipEscapeTbl[i].unencoded;
        }
    }
    return SLIP_INVALID_UNENCODED;
}

static u16 HCI_SLip_GetEscapeCode(u8 unencoded)
{
    for (unsigned int i = 0; i < COUNTOF(slipEscapeTbl); i++) {
        if (unencoded == slipEscapeTbl[i].unencoded) {
            return slipEscapeTbl[i].escapeSeq;
        }
    }
    return SLIP_INVALID_ESCAPE;
}

/**
 * @brief : Slip decoder.
 * @param : pPacket   Pointer point to buffer.
 * @param : len       The length of data.
 * @param : none.
 */
void HCI_Slip_DecodePacket(u8 *pPacket, u32 len)
{
    SLIP_TRACK_INFO("Slip Decode Start...\n");

    if (pPacket == NULL || len == 0 || len < 6) {
        return;             //discard
    }

    u8 *pBuf = pPacket + 1; //skip slip start flag.
    len -= 2;               //reduce slip start/end flag.
    u16 slipEscape      = 0x0000;
    u8  unencoded       = 0;
    hciSlipCb.decodeLen = 0;
    u8 *pDecodeBuf      = hciSlipCb.pDecodeBuf;

    while (len) {
        if (*pBuf == 0xDB) {
            BYTES_TO_UINT16(slipEscape, pBuf);
            unencoded = HCI_Slip_GetUnencoded(slipEscape);
            if (unencoded == SLIP_INVALID_UNENCODED) {
                return; //discard
            }

            if ((hciSlipCb.decodeLen + 1) > HCI_SLIP_DECODE_BUF_SIZE) {
                SLIP_TRACK_ERR("Decode Length: %d, Decode buffer size: %d\n", hciSlipCb.decodeLen, HCI_SLIP_DECODE_BUF_SIZE);
                ASSERT(false, HCI_TR_ERR_SLIP_DECODE_BUF);
                return; //discard
            }

            pDecodeBuf[hciSlipCb.decodeLen++] = unencoded;
            pBuf += 2;
            len -= 2;
        } else {
            if ((hciSlipCb.decodeLen + 1) > HCI_SLIP_DECODE_BUF_SIZE) {
                SLIP_TRACK_ERR("Decode Length: %d, Decode buffer size: %d\n", hciSlipCb.decodeLen, HCI_SLIP_DECODE_BUF_SIZE);
                ASSERT(false, HCI_TR_ERR_SLIP_DECODE_BUF);
                return; //discard
            }

            pDecodeBuf[hciSlipCb.decodeLen++] = *pBuf;
            pBuf += 1;
            len -= 1;
        }
    }

    SLIP_TRACK_INFO("Slip decode data:");
    HCI_TRACK_DATA(hciSlipCb.pDecodeBuf, hciSlipCb.decodeLen);
    hciSlipCb.HCI_H5_PktHandler(hciSlipCb.pDecodeBuf, hciSlipCb.decodeLen);
}

/**
 * @brief : Slip encoder.
 * @param : pPacket   Pointer point to buffer.
 * @param : len       The length of data.
 * @param : none.
 */
void HCI_Slip_EncodePacket(u8 *pPacket, u32 len)
{
    hciSlipCb.encodeLen = 0;
    u8 *pEncodeBuf      = hciSlipCb.pEncodeBuf + 4;
    UINT8_TO_BSTREAM(pEncodeBuf, SLIP_DELIMITER);
    hciSlipCb.encodeLen++;

    u8 *pBuf      = pPacket;
    u16 escapeSeq = 0;

    for (unsigned int i = 0; i < len; i++) {
        switch (pBuf[i]) {
        case 0xC0:
        case 0xDB:
        case 0x11:
        case 0x13:
            if (!hciSlipCb.oofEnable && (pBuf[i] == 0x11 || pBuf[i] == 0x13)) {
                UINT8_TO_BSTREAM(pEncodeBuf, pBuf[i]);
                hciSlipCb.encodeLen += 1;
            } else {
                escapeSeq = HCI_SLip_GetEscapeCode(pBuf[i]);
                UINT16_TO_BSTREAM(pEncodeBuf, escapeSeq);
                hciSlipCb.encodeLen += 2;
            }

            if (hciSlipCb.encodeLen > HCI_SLIP_ENCODE_BUF_SIZE) {
                SLIP_TRACK_ERR("Encode length: %d, Encode buf size: %d\n", hciSlipCb.encodeLen, HCI_SLIP_ENCODE_BUF_SIZE);
                ASSERT(FALSE, HCI_TR_ERR_SLIP_ENCODE_BUF);
            }
            break;

        default:
            UINT8_TO_BSTREAM(pEncodeBuf, pBuf[i]);
            hciSlipCb.encodeLen += 1;

            if (hciSlipCb.encodeLen > HCI_SLIP_ENCODE_BUF_SIZE) {
                SLIP_TRACK_ERR("Encode length: %d, Encode buf size: %d\n", hciSlipCb.encodeLen, HCI_SLIP_ENCODE_BUF_SIZE);
                ASSERT(FALSE, HCI_TR_ERR_SLIP_ENCODE_BUF);
            }
            break;
        }
    }

    UINT8_TO_BSTREAM(pEncodeBuf, SLIP_DELIMITER);
    hciSlipCb.encodeLen += 1;

    if (hciSlipCb.encodeLen > HCI_SLIP_ENCODE_BUF_SIZE) {
        SLIP_TRACK_ERR("Encode length: %d, Encode buf size: %d\n", hciSlipCb.encodeLen, HCI_SLIP_ENCODE_BUF_SIZE);
        ASSERT(FALSE, HCI_TR_ERR_SLIP_ENCODE_BUF);
    }
}

/**
 * @brief : Slip send.
 * @param : pPacket   Pointer point to buffer.
 * @param : len       The length of data.
 * @param : none.
 */
bool HCI_Slip_Send(u8 *pPacket, u32 len)
{
    if (!ext_hci_getTxCompleteDone()) {
        return false;
    }
    HCI_Slip_EncodePacket(pPacket, len);

    u8 *pBuf = hciSlipCb.pEncodeBuf;
    UINT32_TO_BSTREAM(pBuf, hciSlipCb.encodeLen);

    u8 res = ext_hci_uartSendData(pBuf, hciSlipCb.encodeLen);
    return res == 0 ? false : true;
}

static void HCI_Slip_DefaultPktHandler(u8 *pPacket, u32 len)
{
    (void)pPacket;
    (void)len;
}

/**
 * @brief : Slip initialization.
 * @param : none.
 * @param : none.
 */
void HCI_Slip_Init(void)
{
    hciSlipCb.pDecodeBuf = slipDecodeBuf;
    hciSlipCb.decodeLen  = 0;

    hciSlipCb.pEncodeBuf = slipEncodeBuf;
    hciSlipCb.encodeLen  = 0;

    hciSlipCb.HCI_H5_PktHandler = HCI_Slip_DefaultPktHandler;

    hciSlipCb.oofEnable = 0;

    /* Register Slip Handler to Transport. */
    HCI_Tr_H5RegisterSlipHandler(HCI_Slip_DecodePacket);
}

/**
 * @brief : Set OOF software flow control enable/disable.
 * @param : enable   enable/disable OOF.
 * @param : none.
 */
void HCI_SLip_SetFlowCtrlEnable(bool enable)
{
    hciSlipCb.oofEnable = enable;
}

#endif /* End of HCI_TR_EN */
