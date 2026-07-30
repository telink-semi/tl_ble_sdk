/********************************************************************************************************
 * @file    tpll.h
 *
 * @brief   This is the header file for 2.4G SDK
 *
 * @author  2.4G Group
 * @date    2024
 *
 * @par     Copyright (c) 2024, Telink Semiconductor (Shanghai) Co., Ltd. ("TELINK")
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

#define TPLL_PIPE_NUM                            8
#define TPLL_TX_FIFO_DEP                         2   // fifo num = 2^FIFO_DEP
#define TPLL_TX_FIFO_SIZE                        128 /** The size of the transmission first-in, first-out buffer */
#define TPLL_TX_FIFO_NUM                         4   /** The number of the transmission first-in, first-out buffer */
#define TPLL_PIPE_RX_FIFO_SIZE                   256 // must be >= 256 if use TPLL_MODE_GENERIC_FORMAT in TL321X
#define TPLL_PIPE_RX_FIFO_NUM                    4

#define TPLL_MAX_LEGACY_VARIABLE_PAYLOAD_LENGTH  63                        /** The maximum size of the payload */
#define TPLL_MAX_GENERIC_VARIABLE_PAYLOAD_LENGTH 65535                     /** The maximum size of the payload */
#define TPLL_ERROR_BASE_NUM                      (0x0)                     /** Global error base */
#define TPLL_SUCCESS                             (TPLL_ERROR_BASE_NUM + 0) /** Successful command */
#define TPLL_ERROR_NULL                          (TPLL_ERROR_BASE_NUM + 1) /** Null Pointer */
#define TPLL_ERROR_INVALID_PARAM                 (TPLL_ERROR_BASE_NUM + 2) /** Invalid Parameter */
#define TPLL_ERROR_BUSY                          (TPLL_ERROR_BASE_NUM + 3) /** Busy */
#define TPLL_ERROR_INVALID_STATE                 (TPLL_ERROR_BASE_NUM + 4) /** Invalid state, operation disallowed in this state */
#define TPLL_ERROR_BUFFER_EMPTY                  (TPLL_ERROR_BASE_NUM + 5) /** FIFO is empty */
#define TPLL_ERROR_NO_MEM                        (TPLL_ERROR_BASE_NUM + 6) /** No Memory for operation */
#define TPLL_ERROR_INVALID_LENGTH                (TPLL_ERROR_BASE_NUM + 7) /** Invalid Length */

/**@brief Telink primary link layer mode. */
typedef enum
{
    TPLL_MODE_LEGACY_FORMAT = 0, /**< old 9 bit header variable packet format radio mode. */
    TPLL_MODE_GENERIC_FORMAT,    /**< new variable packet format radio mode. */
} TPLL_ModeFormatTypeDef;

typedef struct
{
    unsigned char h0_size;
    unsigned char h1_size;
    unsigned char length_size;
    unsigned int  h0_val;
    unsigned int  h1_val;
    unsigned int  length_val;
    unsigned int  pid_start_bit;
    unsigned int  noack_start_bit;
} TPLL_GenericHeader_t;

extern TPLL_GenericHeader_t TPLL_GenericHeader;

typedef struct
{
    unsigned int  rx_payload_len;
    unsigned int  rx_timestamp;
    unsigned int  header_len;
    signed char   rx_rssi;
    unsigned char rx_pipe;
    unsigned char crc_len;
} TPLL_ReadRxPayload_t;

/**
 * @brief       RF CRC config.
 * @note        init_value - The CRC init value to be set for RF.
 *              poly - The CRC polynomial to be set for RF.
 *              xor_out - XOR mask for CRC result (for no mask, should be 0)
 *              byte_order - CRC output byte order (1:MSByte first,0:LSByte first)
 *              start_cal_pos - CRC start byte position, 0 is the first byte
 *              len - The CRC length to be set for RF
 */
typedef struct
{
    unsigned int  init_value;
    unsigned int  poly;
    unsigned int  xor_out;
    unsigned char byte_order;
    unsigned char start_cal_pos;
    unsigned char len;
} TPLL_CrcConfig_t;

extern TPLL_CrcConfig_t TPLL_CrcConfig;


extern rf_pkt_flt_t TPLL_PktFlt;

/**@brief Telink primary link layer bitrate mode. */
typedef enum
{
    TPLL_BITRATE_1MBPS = 0, /**< 1Mbit radio mode. */
    TPLL_BITRATE_2MBPS,     /**< 2Mbit radio mode. */
    TPLL_BITRATE_500kBPS,   /**< 500kbit radio mode. */
    TPLL_BITRATE_250KBPS,   /**< 250Kbit radio mode. */
} TPLL_BitrateTypeDef;

/** An enum describing the radio's power level.
*
*/
 typedef enum
 {
     /*VBAT*/
     TPLL_POWER_INDEX_P11p3dBm, /**<  11.3 dbm */
     TPLL_POWER_INDEX_P11p1dBm, /**<  11.1 dbm */
     TPLL_POWER_INDEX_P10p8dBm, /**<  10.8 dbm */
     TPLL_POWER_INDEX_P10p5dBm, /**<  10.5 dbm */
     TPLL_POWER_INDEX_P10p0dBm, /**<  10.0 dbm */
     TPLL_POWER_INDEX_P9p5dBm,  /**<  9.5 dbm */
     TPLL_POWER_INDEX_P9p0dBm,  /**<  9.0 dbm */
     TPLL_POWER_INDEX_P8p4dBm,  /**<  8.4 dbm */
     TPLL_POWER_INDEX_P8p0dBm,  /**<  8.0 dbm */
     TPLL_POWER_INDEX_P7p6dBm,  /**<  7.6 dbm */
     TPLL_POWER_INDEX_P7p2dBm,  /**<  7.2 dbm */
     TPLL_POWER_INDEX_P7p0dBm,  /**<  7.0 dbm */
     TPLL_POWER_INDEX_P6p5dBm,  /**<  6.5 dbm */
     TPLL_POWER_INDEX_P6p0dBm,  /**<  6.0 dbm */

     /*VANT*/
     TPLL_POWER_INDEX_P5p9dBm,  /**<   5.9 dbm */
     TPLL_POWER_INDEX_P5p5dBm,  /**<   5.5 dbm */
     TPLL_POWER_INDEX_P5p2dBm,  /**<   5.2 dbm */
     TPLL_POWER_INDEX_P5p1dBm,  /**<   5.1 dbm */
     TPLL_POWER_INDEX_P5p0dBm,  /**<   5.0 dbm */
     TPLL_POWER_INDEX_P4p7dBm,  /**<   4.7 dbm */
     TPLL_POWER_INDEX_P4p5dBm,  /**<   4.5 dbm */
     TPLL_POWER_INDEX_P4p0dBm,  /**<   4.0 dbm */
     TPLL_POWER_INDEX_P3p5dBm,  /**<   3.5 dbm */
     TPLL_POWER_INDEX_P3p0dBm,  /**<   3.0 dbm */
     TPLL_POWER_INDEX_P2p5dBm,  /**<   2.5 dbm */
     TPLL_POWER_INDEX_P2p0dBm,  /**<   2.0 dbm */
     TPLL_POWER_INDEX_P1p6dBm,  /**<   1.6 dbm */
     TPLL_POWER_INDEX_P1p2dBm,  /**<   1.2 dbm */
     TPLL_POWER_INDEX_P0p9dBm,  /**<   0.9 dbm */
     TPLL_POWER_INDEX_P0p6dBm,  /**<   0.6 dbm */
     TPLL_POWER_INDEX_P0p3dBm,  /**<   0.3 dbm */
     TPLL_POWER_INDEX_P0p0dBm,  /**<   0.0 dbm */
     TPLL_POWER_INDEX_N0p2dBm,  /**<   -0.2 dbm */
     TPLL_POWER_INDEX_N0p5dBm,  /**<   -0.5 dbm */
     TPLL_POWER_INDEX_N1p2dBm,  /**<   -1.2 dbm */
     TPLL_POWER_INDEX_N1p6dBm,  /**<   -1.6 dbm */
     TPLL_POWER_INDEX_N2p0dBm,  /**<  -2.0 dbm */
     TPLL_POWER_INDEX_N2p8dBm,  /**<  -2.8 dbm */
     TPLL_POWER_INDEX_N3p3dBm,  /**<  -3.3 dbm */
     TPLL_POWER_INDEX_N4p4dBm,  /**<  -4.4 dbm */
     TPLL_POWER_INDEX_N4p9dBm,  /**<  -4.9 dbm */
     TPLL_POWER_INDEX_N5p6dBm,  /**<  -5.6 dbm */
     TPLL_POWER_INDEX_N6p2dBm,  /**<  -6.2 dbm */
     TPLL_POWER_INDEX_N7p8dBm,  /**<  -7.8 dbm */
     TPLL_POWER_INDEX_N8p8dBm,  /**<  -8.8 dbm */
     TPLL_POWER_INDEX_N9p9dBm,  /**<  -9.9 dbm */
     TPLL_POWER_INDEX_N11p3dBm, /**<  -11.3 dbm */
     TPLL_POWER_INDEX_N12p6dBm, /**<  -12.6 dbm */
     TPLL_POWER_INDEX_N14p6dBm, /**<  -14.6 dbm */
     TPLL_POWER_INDEX_N20p0dBm, /**<  -20.0 dbm */
     TPLL_POWER_INDEX_N25p5dBm, /**<  -25.5 dbm */
     TPLL_POWER_INDEX_N50p2dBm, /**<  -50.2 dbm */
 } tpll_radio_power_t;


/**@brief Telink primary link layer pipe IDs. */
typedef enum
{
    TPLL_PIPE0,            /**< Select pipe0 */
    TPLL_PIPE1,            /**< Select pipe1 */
    TPLL_PIPE2,            /**< Select pipe2 */
    TPLL_PIPE3,            /**< Select pipe3 */
    TPLL_PIPE4,            /**< Select pipe4 */
    TPLL_PIPE5,            /**< Select pipe5 */
    TPLL_PIPE6,            /**< Select pipe6 */
    TPLL_PIPE7,            /**< Select pipe7 */
    TPLL_PIPE_NONE = 0x00, /**< none pipe */
    TPLL_PIPE_ALL  = 0xFF  /**< all pipes*/
} TPLL_PipeIDTypeDef;

/**@brief Telink primary link layer address width. */
typedef enum
{
    ADDRESS_WIDTH_3BYTES = 3, /**< Set address width to 3 bytes */
    ADDRESS_WIDTH_4BYTES,     /**< Set address width to 4 bytes */
    ADDRESS_WIDTH_5BYTES      /**< Set address width to 5 bytes */
} TPLL_AddressWidthTypeDef;

/**@brief Telink primary link layer state machine status. */
typedef enum
{
    TPLL_STATE_MACHINE_STATUS_IDLE = 0,  /**< Idle */
    TPLL_STATE_MACHINE_STATUS_ACTIVE,    /**< active */
    TPLL_STATE_MACHINE_STATUS_TX_SETTLE, /**< TX Settle*/
    TPLL_STATE_MACHINE_STATUS_TX,        /**< TX */
    TPLL_STATE_MACHINE_STATUS_RX_WAIT,   /**< RX Wait */
    TPLL_STATE_MACHINE_STATUS_RX,        /**< RX */
    TPLL_STATE_MACHINE_STATUS_TX_WAIT,   /**< RX Wait */
} TPLL_StatemachineStatusTypeDef;

/**@brief Telink primary link layer mode. */
typedef enum
{
    TPLL_MODE_PTX = 0, /**< PTX Mode */
    TPLL_MODE_PRX,     /**< PRX Mode */
} TPLL_ModeTypeDef;

extern rf_crc_config_t generic_crc_config;

/**
 * @brief Enables the W_TX_PAYLOAD_NOACK command
 * @param enable Whether to 1:enable or 0:disable NoAck option in 9-bit Packet(legacy packet) control field
 * @note when use generic mode you should write h0 no ack bit rather than write this register to make peer no ack or ack.
 */
void TPLL_EnableNoAck(unsigned char enable);

/**
 * @brief   Enables the ACK payload feature
 * @param   enable   Whether to enable or disable ACK payload
 */
void TPLL_Set_Local_Ack_Bit(unsigned char enable);

/**
 * @brief       select tpll packet format.
 * @param       mode_format  packet format.
 * @return      none.
 */
void TPLL_SetFormatMode(TPLL_ModeFormatTypeDef mode_format);

/**
 * @brief       Initiate the the Telink primary link layer module and Set the radio bitrate.
 * @param       bitrate  Radio bitrate.
 * @return      none.
 */
uint32_t TPLL_SetBitrate(TPLL_BitrateTypeDef bitrate);

/**
 * @brief       Set the radio output power.
 * @param       power   Output power.
 * @return      none.
 */
void TPLL_SetOutputPower(tpll_radio_power_t power);

/**
 * @brief       Set the channel to use for the radio.
 * @param       channel Channel to use for the radio.
 * @return      TPLL_SUCCESS                     If the operation completed successfully.
 * @return      TPLL_ERROR_BUSY                  If the function failed because the radio is busy.
 * @return      TPLL_ERROR_INVALID_PARAM         If the param invalid.
 */
uint32_t TPLL_SetRFChannel(signed short channel);

/**
 * @brief       Set one pipe as a TX pipe.
 * @param       pipe_id Pipe to be set as a TX pipe.
 * @return      none.
 */
void TPLL_SetTXPipe(TPLL_PipeIDTypeDef pipe_id);

/**
 * @brief       Get the current TX pipe.
 * @param       none.
 * @return      The pipe set as a TX pipe.
*/
unsigned char TPLL_GetTXPipe(void);

/**
 * @brief       Update the read pointer of the TX FIFO.
 * @param       pipe_id Pipe id.
 * @return      none.
 */
void TPLL_UpdateTXFifoRptr(TPLL_PipeIDTypeDef pipe_id);

/**
 * @brief       Set the width of the address.
 * @param       address_width   Width of the TPLL address (in bytes).
 * @return      none.
 */
void TPLL_SetAddressWidth(TPLL_AddressWidthTypeDef address_width);

/**
 * @brief       Get the width of the address.
 * @param       none.
 * @return      Width of the TPLL address width(in bytes).
 */
unsigned char TPLL_GetAddressWidth(void);

/**
 * @brief       Check status for a selected pipe.
 * @param       pipe_id Pipe number to check status for.
 * @return      Pipe status.
 */
unsigned char TPLL_GetPipeStatus(TPLL_PipeIDTypeDef pipe_id);

/**
 * @brief       Set the address for pipes.
 *              Beware of the difference for single and multibyte address registers
 * @param       pipe_id Radio pipe to set.
 * @param       addr    Buffer from which the address is stored in
 * @return      none.
 */
void TPLL_SetAddress(TPLL_PipeIDTypeDef pipe_id, const unsigned char *addr);

/**
 * @brief       Get the address for selected pipe.
 * @param       pipe_id Pipe for which to get the address.
 * @param       addr    Pointer t a buffer that address bytes are written to.
 *               <BR><BR>For pipes containing only LSB byte of address, this byte is returned.
 * @return      Numbers of bytes copied to addr.
 */
unsigned char TPLL_GetAddress(unsigned char pipe_id, unsigned char *addr);

/**
 * @brief       Open one or all pipes.
 * @param       pipe_id Radio pipes to open.
 * @return      none.
 */
void TPLL_OpenPipe(TPLL_PipeIDTypeDef pipe_id);

/**
 * @brief       Close one or all pipes.
 * @param       pipe_id Radio pipes to close.
 * @return      none.
 */
void TPLL_ClosePipe(TPLL_PipeIDTypeDef pipe_id);
/**
 * @brief       Set the the number of retransmission attempts and the packet retransmit delay.
 * @param       retr_times  Number of retransmissions. Setting the parmater to 0 disables retransmission.
 * @param       retry_delay Delay between retransmissions.
 * @return      none.
 */
void TPLL_SetAutoRetry(unsigned char retry_times, unsigned short retry_delay);

/**
 * @brief     This function serves to inti rx/tx dma setting.
 * @param       rx_buffer  rx buffer address.
 * @param       tx_buffer  tx buffer address.
 * @return    none.
 */
void TPLL_DmaInit(unsigned char *rx_buffer, unsigned char *tx_buffer);

/**
 * @brief       Check if the TX FIFO is empty.
 * @param       pipe_id pipe id for which to check.
 * @return      1: the TX FIFO is empty; 0: the packet is not empty.
 */
unsigned char TPLL_TxFifoEmpty(TPLL_PipeIDTypeDef pipe_id);

/**
 * @brief       Check if TX FIFO is full.
 * @param       pipe_id pipe id for which to check.
 * @return      TRUE TX FIFO full or not.
 */
unsigned char TPLL_TxFifoFull(TPLL_PipeIDTypeDef pipe_id);

/**
 * @brief       Get the number of retransmission attempts when it goes max retransmission attempts it will be reset to 0.
 * @param       none.
 * @return      Number of retransmissions.
 */
unsigned char TPLL_GetTransmitAttempts(void);

/**
 * @brief       Get the carrier detect status.
 * @param       rx_buf  rx buffer address.
 * @return      Carrier detect status.
 */
unsigned char TPLL_GetCarrierDetect(unsigned char *rx_buf);

/**
 * @brief       Check if the RX FIFO is empty.
 * @param       pipe_id pipe id for which to check.
 * @return      1: the RX FIFO is empty; 0: the packet is not empty.
 */
unsigned char TPLL_RxFifoEmpty(void);

/**
 * @brief       Check if RX FIFO is full.
 * @return      TRUE RX FIFO full or not.
 */
unsigned char TPLL_RxFifoFull(void);

/**
 * @brief Function for reading an RX payload.
 * @param  rx_pload   Pointer to buffer where the RX payload is stored.
 * @param  rx_buf  rx buffer address.
 * @return TPLL_ReadRxPayload_t        param in TPLL_ReadRxPayload_t.
 */
TPLL_ReadRxPayload_t TPLL_ReadRxPayload(unsigned char *rx_pload, unsigned char *rx_buf);

/**
 * @brief       Get the RX timestamp.
 * @note        It is required to call TPLL_ReadRxPayload() before this function is called.
 * @param       none.
 * @return      RX timestamp.
 */
unsigned int TPLL_GetTimestamp(volatile unsigned char *rx_packet);

/**
 * @brief       Get the RX RSSI.
 * @note        It is required to call TPLL_ReadRxPayload() before this function is called.
 * @param       none.
 * @return      RX RSSI.
 */
signed char TPLL_GetRxRssiValue(volatile unsigned char *rx_packet);

/**
 * @brief       Check if the crc of the received packet is valid.
 * @param       rx_packet.
 * @return      1: the packet crc is valid; 0: the packet crc is invalid.
 */
uint8_t TPLL_IsCrcValid(volatile uint8_t *rx_packet);

/**
 * @brief       Check whether the received packet is valid.
 * @param       rx_buf  rx buffer address.
 * @return      1: the packet is valid; 0: the packet is invalid.
 */
unsigned char TPLL_IsRxPacketValid(unsigned char *rx_buf);

/**
 * @brief       Disable / enable sending an ACK packet when a crc error occurs.
 * @param       1:enable,0:disable.
 * @return      none.
 */
void TPLL_EnableCrcfilter(uint8_t enable);

/**
 * @brief       Get the packet received.
 * @param       rx_buf  rx buffer address.
 * @return      rx_packet.
 */
volatile unsigned char *TPLL_GetRxPacket(unsigned char *rx_buf);

/**
 * @brief       Get the pid of the received packet.
 * @param       rx_packet.
 * @return      packet id.
 */
unsigned char TPLL_GetRxPacketId(volatile unsigned char *rx_packet);

/**
 * @brief       Get the crc of the received packet.
 * @param       rx_packet.
 * @return      packet crc.
 */
void TPLL_GetRxPacketCrc(volatile unsigned char *rx_packet, volatile unsigned char *crc);


/**
 * @brief       Check if the payload length is zero.
 * @param       rx_packet.
 * @return      1:payload length is zero,0:payload length is not zero.
 */
unsigned char TPLL_IsPacketEmpty(unsigned char *rx_packet);

/**
 * @brief       Get the local pid.
 * @return      local pid.
 */
unsigned char TPLL_GetLocalPid(TPLL_PipeIDTypeDef pipe_id);

/**
 * @brief       Set the pid reset bit to 0.
 * @return      None.
 */
void TPLL_Pid_Reset_Disable(void);


/** @brief Function for writing a payload for transmission or acknowledgement.
 *
 * @details This function writes a payload that is added to the tx fifo.
 *
 * @param[in]   pipe_id     tx pipe id.
 * @param[in]   tx_pload    Pointer to the array that contains the payload.
 * @param[in]   length      tx payload length.
 * @param[in]   ptx_buffer  tx buffer address.
 *
 * @retval  TPLL_SUCCESS                     If the payload was successfully queued for writing.
 * @retval  TPLL_ERROR_NULL                  If the required parameter was NULL.
 * @retval  TPLL_ERROR_INVALID_PARAM         If the param invalid.
 * @retval  TPLL_ERROR_NO_MEM                If the TX FIFO is full.
 * @retval  TPLL_ERROR_INVALID_LENGTH        If the payload length was invalid (zero or larger than the allowed
 * maximum).
 */
uint32_t TPLL_WriteTxPayload(unsigned char pipe_id, unsigned char *ptx_buffer, const unsigned char *tx_pload, unsigned int length);


/**
 * @brief       Write payload bytes of the ACK packet.
 *              Writes the payload that will be transmitted with the ack on the given pipe.
 * @param       pipe_id     Pipe that transmits the payload.
 * @param       payload     Pointer to the payload data.
 * @param       length      Size of the data to transmit.
 * @param       ptx_buffer  tx buffer address.
 * @return
 */
void TPLL_WriteAckPayload(unsigned char pipe_id, unsigned char *ptx_buffer, const unsigned char *tx_pload, unsigned char length);


/**
 * @brief       Trigger the transmission in the specified pipe
 * @param       none.
 * @return      TPLL_SUCCESS                     If the operation completed successfully.
 * @return      TPLL_ERROR_BUSY                  If the function failed because the radio is busy.
 */
int TPLL_PTXTrig(void);

/**
 * @brief       Trigger the receiver activity in the specified pipe
 * @param       none.
 * @return      TPLL_SUCCESS                     If the operation completed successfully.
 * @return      TPLL_ERROR_BUSY                  If the function failed because the radio is busy.
 */
int TPLL_PRXTrig(void);

/**
 * @brief       Set the wait time between the end of an Ack-required packet's transmission
 *              and the start of Ack receiving to accommodate with another chip.
 * @param       wait_us Wait time between the end of an Ack-required packet's transmission
 *              and the start of Ack receiving.
 * @return      TPLL_SUCCESS                         If the operation completed successfully.
 * @return      TPLL_ERROR_INVALID_LENGTH            If the param was invalid.
 */
int TPLL_RxWaitSet(unsigned short wait_us);

/**
 * @brief       Set the TX wait time.
 * @param       wait_us   Period in microseconds.
 * @return      TPLL_SUCCESS                         If the operation completed successfully.
 * @return      TPLL_ERROR_INVALID_LENGTH            If the param was invalid.
 */
int TPLL_TxWaitSet(unsigned short wait_us);

/**
 * @brief       Set the rx duration when an Ack-required packet has been
 *              transmitted and an Ack is expected.
 * @param       period_us   specifies the time of rx duration.
 * @return      TPLL_SUCCESS                         If the operation completed successfully.
 * @return      TPLL_ERROR_INVALID_LENGTH            If the param was invalid.
 */
int TPLL_RxTimeoutSet(unsigned short period_us);

/**
 * @brief       Set the TX Settle phase.
 * @param       period_us   specifies the time.
 * @return      TPLL_SUCCESS                         If the operation completed successfully.
 * @return      TPLL_ERROR_INVALID_LENGTH            If the param was invalid.
 */
int TPLL_TxSettleSet(unsigned short period_us);

/**
 * @brief       Set the RX Settle phase.
 * @param       period_us   specifies the time.
 * @return      TPLL_SUCCESS                         If the operation completed successfully.
 * @return      TPLL_ERROR_INVALID_LENGTH            If the param was invalid.
 */
int TPLL_RxSettleSet(unsigned short period_us);

/**
 * @brief       Set the mode of TPLL radio.
 * @param       mode    TPLL_MODE_PTX or TPLL_MODE_PRX.
 * @return      none.
 */
void TPLL_ModeSet(TPLL_ModeTypeDef mode);

/**
 * @brief       Stop the TPLL state machine.
 * @param       none.
 * @return      none.
 */
void TPLL_ModeStop(void);

/**
 * @brief       Remove remaining items from the RX buffer.
 * @param       none.
 * @return      none.
 */
void TPLL_FlushRx(void);

/**
 * @brief       Reuse the last transmitted payload for the next packet.
 * @param       pipe_id pipe id.
 * @return      none.
 */
void TPLL_ReuseTx(TPLL_PipeIDTypeDef pipe_id);

/**
 * @brief       Remove remaining items from the TX buffer.
 * @param       pipe_id Pipe id.
 * @return      none.
 */
void TPLL_FlushTx(TPLL_PipeIDTypeDef pipe_id);

/**
 * @brief      Set the length of the preamble field of the on-air data packet.
 * @note       The valid range of this parameter is 1-16.
 * @param      preamble_len Preamble length.
 * @return     none.
 */
void TPLL_Preamble_Set(unsigned char preamble_len);

/**
 * @brief      Read the length of the preamble field of the on-air data packet.
 * @param      none.
 * @return     Preamble length.
 */
unsigned char TPLL_Preamble_Read(void);

/**
 * @brief      disable the receiver preamble detection banking duiring the first byte of pdu.
 * @param      none.
 * @return     none.
 */
void TPLL_Preamble_Detect_Disable(void);

/**
 *  @brief      this function serve to reset buffer.
 *  @param[in]  none.
 *  @return     none.
*/
void TPLL_ResetBuf(void);

/**
 * @brief       This function set generic packet header.
 * @param[in]   TPLL_GenericHeader_t - generic packet header parameters
 * @return      none.
 */
void TPLL_GenericHeaderSet(TPLL_GenericHeader_t GenericHeader);

/**
 * @brief       This function set packet crc.
 * @param[in]   TPLL_CrcConfig_t - packet crc parameters
 * @return      none.
 */
void TPLL_CrcSet(TPLL_CrcConfig_t crc_cfg);

/**
 * @brief       This function set the packet filter.
 * @param[in]   TPLL_PktFlt_t - RF packet filtering parameters
 * @return      none.
 */
void TPLL_PktFilter(rf_pkt_flt_t PktFlt);

/**
 * @brief   This function serve to enable/disable generic format mode ack
 * @return  none.
 */
void TPLL_GenericNoAckEnable(void);

/**
 * @brief   This function serve to enable/disable generic format mode pid
 * @return  none.
 */
void TPLL_GenericPidEnable(void);

/**
 * @brief   This function serve to enable/disable new/old crc value check for PRX new/old pkt
 * @param[in]   1:enable 0:disable
 * @return  none.
 */
void TPLL_PrxCrcCheckEnable(unsigned char enable);

/**
 * @brief   This function serve to enable/disable PTX/PRX ACK
 * @return  none.
 */
void TPLL_PtxPrxAckEnable(unsigned char Enable);
/**
 * @brief   This function serve to change payload bit order.
 * @param[in]   1: MSBit (Most Significant Bit) of the LSByte (Least Significant Byte) first
 *              0: LSBit (Least Significant Bit) of the LSByte (Least Significant Byte) first
 * @return  none.
 */
void TPLL_PlayloadBitOrder(unsigned char bitorder);
    