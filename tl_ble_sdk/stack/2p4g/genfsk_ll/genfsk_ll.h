/********************************************************************************************************
 * @file    genfsk_ll.h
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
#ifndef _GENFSK_LL_H_
#define _GENFSK_LL_H_

#include "driver.h"
#include "types.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define GEN_FSK_MAX_FIXED_LENGTH        0x3e                         /** The maximum size of the payload */
#define GEN_FSK_MAX_VARIABLE_LENGTH     0xff                         /** The maximum size of the payload */
#define GEN_FSK_MAX_GNC_VARIABLE_LENGTH 0xffff                       /** The maximum size of the payload */
#define GEN_FSK_ERROR_BASE_NUM          (0x0)                        /** Global error base */
#define GEN_FSK_SUCCESS                 (GEN_FSK_ERROR_BASE_NUM + 0) /** Successful command */
#define GEN_FSK_ERROR_NULL              (GEN_FSK_ERROR_BASE_NUM + 1) /** Null Pointer */
#define GEN_FSK_ERROR_INVALID_PARAM     (GEN_FSK_ERROR_BASE_NUM + 2) /** Invalid Parameter */
#define GEN_FSK_ERROR_BUSY              (GEN_FSK_ERROR_BASE_NUM + 3) /** Busy */
#define GEN_FSK_ERROR_INVALID_STATE     (GEN_FSK_ERROR_BASE_NUM + 4) /** Invalid state, operation disallowed in this state */
#define GEN_FSK_ERROR_BUFFER_EMPTY      (GEN_FSK_ERROR_BASE_NUM + 5) /** FIFO is empty */
#define GEN_FSK_ERROR_NO_MEM            (GEN_FSK_ERROR_BASE_NUM + 6) /** No Memory for operation */
#define GEN_FSK_ERROR_INVALID_LENGTH    (GEN_FSK_ERROR_BASE_NUM + 7) /** Invalid Length */

    /** An enum describing the radio's state.
 *
 */
    typedef enum
    {
        GEN_FSK_STATE_TX = 0,     //config RF transceiver in TX state
        GEN_FSK_STATE_RX,         //config RF transceiver in RX state
        GEN_FSK_STATE_AUTO,       //enable the automatic Single TX/RX/TX2RX/RX2TX FSM engine
        GEN_FSK_STATE_OFF = 0xff, //close the RF transceiver
    } gen_fsk_state_t;

    /** An enum describing the radio's power level.
 *
 */
    typedef enum
    {
        /*VBAT*/
        GEN_FSK_POWER_INDEX_P11p3dBm, /**<  11.3 dbm */
        GEN_FSK_POWER_INDEX_P11p1dBm, /**<  11.1 dbm */
        GEN_FSK_POWER_INDEX_P10p8dBm, /**<  10.8 dbm */
        GEN_FSK_POWER_INDEX_P10p5dBm, /**<  10.5 dbm */
        GEN_FSK_POWER_INDEX_P10p0dBm, /**<  10.0 dbm */
        GEN_FSK_POWER_INDEX_P9p5dBm,  /**<  9.5 dbm */
        GEN_FSK_POWER_INDEX_P9p0dBm,  /**<  9.0 dbm */
        GEN_FSK_POWER_INDEX_P8p4dBm,  /**<  8.4 dbm */
        GEN_FSK_POWER_INDEX_P8p0dBm,  /**<  8.0 dbm */
        GEN_FSK_POWER_INDEX_P7p6dBm,  /**<  7.6 dbm */
        GEN_FSK_POWER_INDEX_P7p2dBm,  /**<  7.2 dbm */
        GEN_FSK_POWER_INDEX_P7p0dBm,  /**<  7.0 dbm */
        GEN_FSK_POWER_INDEX_P6p5dBm,  /**<  6.5 dbm */
        GEN_FSK_POWER_INDEX_P6p0dBm,  /**<  6.0 dbm */

        /*VANT*/
        GEN_FSK_POWER_INDEX_P5p9dBm,  /**<   5.9 dbm */
        GEN_FSK_POWER_INDEX_P5p5dBm,  /**<   5.5 dbm */
        GEN_FSK_POWER_INDEX_P5p2dBm,  /**<   5.2 dbm */
        GEN_FSK_POWER_INDEX_P5p1dBm,  /**<   5.1 dbm */
        GEN_FSK_POWER_INDEX_P5p0dBm,  /**<   5.0 dbm */
        GEN_FSK_POWER_INDEX_P4p7dBm,  /**<   4.7 dbm */
        GEN_FSK_POWER_INDEX_P4p5dBm,  /**<   4.5 dbm */
        GEN_FSK_POWER_INDEX_P4p0dBm,  /**<   4.0 dbm */
        GEN_FSK_POWER_INDEX_P3p5dBm,  /**<   3.5 dbm */
        GEN_FSK_POWER_INDEX_P3p0dBm,  /**<   3.0 dbm */
        GEN_FSK_POWER_INDEX_P2p5dBm,  /**<   2.5 dbm */
        GEN_FSK_POWER_INDEX_P2p0dBm,  /**<   2.0 dbm */
        GEN_FSK_POWER_INDEX_P1p6dBm,  /**<   1.6 dbm */
        GEN_FSK_POWER_INDEX_P1p2dBm,  /**<   1.2 dbm */
        GEN_FSK_POWER_INDEX_P0p9dBm,  /**<   0.9 dbm */
        GEN_FSK_POWER_INDEX_P0p6dBm,  /**<   0.6 dbm */
        GEN_FSK_POWER_INDEX_P0p3dBm,  /**<   0.3 dbm */
        GEN_FSK_POWER_INDEX_P0p0dBm,  /**<   0.0 dbm */
        GEN_FSK_POWER_INDEX_N0p2dBm,  /**<   -0.2 dbm */
        GEN_FSK_POWER_INDEX_N0p5dBm,  /**<   -0.5 dbm */
        GEN_FSK_POWER_INDEX_N1p2dBm,  /**<   -1.2 dbm */
        GEN_FSK_POWER_INDEX_N1p6dBm,  /**<   -1.6 dbm */
        GEN_FSK_POWER_INDEX_N2p0dBm,  /**<  -2.0 dbm */
        GEN_FSK_POWER_INDEX_N2p8dBm,  /**<  -2.8 dbm */
        GEN_FSK_POWER_INDEX_N3p3dBm,  /**<  -3.3 dbm */
        GEN_FSK_POWER_INDEX_N4p4dBm,  /**<  -4.4 dbm */
        GEN_FSK_POWER_INDEX_N4p9dBm,  /**<  -4.9 dbm */
        GEN_FSK_POWER_INDEX_N5p6dBm,  /**<  -5.6 dbm */
        GEN_FSK_POWER_INDEX_N6p2dBm,  /**<  -6.2 dbm */
        GEN_FSK_POWER_INDEX_N7p8dBm,  /**<  -7.8 dbm */
        GEN_FSK_POWER_INDEX_N8p8dBm,  /**<  -8.8 dbm */
        GEN_FSK_POWER_INDEX_N9p9dBm,  /**<  -9.9 dbm */
        GEN_FSK_POWER_INDEX_N11p3dBm, /**<  -11.3 dbm */
        GEN_FSK_POWER_INDEX_N12p6dBm, /**<  -12.6 dbm */
        GEN_FSK_POWER_INDEX_N14p6dBm, /**<  -14.6 dbm */
        GEN_FSK_POWER_INDEX_N20p0dBm, /**<  -20.0 dbm */
        GEN_FSK_POWER_INDEX_N25p5dBm, /**<  -25.5 dbm */
        GEN_FSK_POWER_INDEX_N50p2dBm, /**<  -50.2 dbm */
    } gen_fsk_radio_power_t;

    /** An enum describing the radio's data rate.
 *
 */
    typedef enum
    {
        GEN_FSK_DATARATE_250KBPS = BIT(6),
        GEN_FSK_DATARATE_500KBPS = BIT(7),
        GEN_FSK_DATARATE_1MBPS   = BIT(8),
        GEN_FSK_DATARATE_2MBPS   = BIT(9),
    } gen_fsk_datarate_t;

    /** An enum describing the radio's sync word length.
 *
 */
    typedef enum
    {
        SYNC_WORD_LEN_3BYTE = 3, /**< Set  length to 3 bytes */
        SYNC_WORD_LEN_4BYTE,     /**< Set  length to 4 bytes */
        SYNC_WORD_LEN_5BYTE,     /**< Set  length to 5 bytes */
    } gen_fsk_sync_word_len_t;

    /** An enum describing the pipe IDs.
 *
 */
    typedef enum
    {
        GEN_FSK_PIPE0 = 0,        /**< Select pipe0 */
        GEN_FSK_PIPE1,            /**< Select pipe1 */
        GEN_FSK_PIPE2,            /**< Select pipe2 */
        GEN_FSK_PIPE3,            /**< Select pipe3 */
        GEN_FSK_PIPE4,            /**< Select pipe4 */
        GEN_FSK_PIPE5,            /**< Select pipe5 */
        GEN_FSK_PIPE6,            /**< Select pipe6 */
        GEN_FSK_PIPE7,            /**< Select pipe7 */
        GEN_FSK_PIPE_NONE = 0x00, /**< none pipe */
        GEN_FSK_PIPE_ALL  = 0xFF  /**< all pipes*/
    } gen_fsk_pipe_id_t;

    /**
 *
 */
    typedef struct
    {
        unsigned char h0_size;
        unsigned char h1_size;
        unsigned char length_size;
        unsigned int  h0_val;
        unsigned int  length_val;
        unsigned int  h1_val;
    } gen_fsk_generic_header_t;

    extern gen_fsk_generic_header_t GEN_FSK_GenericHeader;

    /** An enum describing the packet format of on-air data.
 *
 */
    typedef enum
    {
        GEN_FSK_MODE_FIXED_FORMAT            = 0x03, //preamble + sync word + payload + crc, payload is of fixed-length
        GEN_FSK_MODE_LEGACY_VARIABLE_FORMAT  = 0x02, //preamble + sync word + header(9bit) + payload + crc, payload is of variable-length
        GEN_FSK_MODE_GENERIC_VARIABLE_FORMAT = 0x04  //preamble + sync word + header(h0 + length + h1) + payload + crc, payload is of variable-length
    } gen_fsk_packet_format_t;

    /** An enum describing the radio's crc length.
 *
 */
    typedef enum
    {
        CRC_DISABLE = 0, /**< disable crc */
        CRC_1BYTE,       /**< 1byte crc */
        CRC_2BYTE,       /**< 2byte crc */
        CRC_3BYTE,       /**< 3byte crc */
        CRC_4BYTE        /**< 4byte crc */
    } gen_fsk_crc_len_t;

    /**@brief RF modulation index. */
    typedef enum
    {
        GEN_FSK_RF_MI_P0p00  = 0,    /**< MI = 0 */
        GEN_FSK_RF_MI_P0p076 = 76,   /**< MI = 0.076 */
        GEN_FSK_RF_MI_P0p32  = 320,  /**< MI = 0.32 */
        GEN_FSK_RF_MI_P0p50  = 500,  /**< MI = 0.5 */
        GEN_FSK_RF_MI_P0p60  = 600,  /**< MI = 0.6 */
        GEN_FSK_RF_MI_P0p70  = 700,  /**< MI = 0.7 */
        GEN_FSK_RF_MI_P0p80  = 800,  /**< MI = 0.8 */
        GEN_FSK_RF_MI_P0p90  = 900,  /**< MI = 0.9 */
        GEN_FSK_RF_MI_P1p20  = 1200, /**< MI = 1.2 */
        GEN_FSK_RF_MI_P1p30  = 1300, /**< MI = 1.3 */
        GEN_FSK_RF_MI_P1p40  = 1400, /**< MI = 1.4 */
    } GEN_MIVauleTypeDef;

    /**
 *  @brief  RX fast settle time
 *  @note
 *  1:Call rf_fast_settle_config to configure timing during initialization.
 *  2:Call the enable function rf_rx_fast_settle_en when using the configured timing sequence.
 *    To close it, call rf_rx_fast_settle_dis.
 *  3:The deleted hardware calibration values are influenced by environmental temperature and require periodic recalibration.
 *    Calibration method: Call rf_rx_fast_settle_dis, then for different frequency points:
 *    stop RF-related states, enable RX, wait for packet transmission to end -> rf_rx_fast_settle_update_cal_val.
 */
    typedef enum
    {
        GEN_RX_SETTLE_TIME_15US = 0, /**<   reduce 74us of rx settle time.
                                        Receive for a period of time and then do a normal calibration.
                                        note:
                                        1. Related to frequency points. When enabling fast settle on both TX and RX ends,
                                           if 15us option is selected, both TX and RX ends must use the 15us option simultaneously.
                                        2. Refer to the following table to determine the range of frequency points used.
                                           In addition to configuring the calibration values for the used frequency points,
                                           corresponding channel values in the respective intervals need to be configured.
                                           For example, if using the 2426 frequency point, channel should be configured as 26 and 24.
                                       ----------------------------------------------------------------------------------
                                       | Frequency     2400   2410   2420   2430   2440   2450   2460   2470   2480     |
                                       |-----------------|------|------|------|------|------|------|------|------|------|
                                       | channel Value   |   4  |  14  |  24  |  34  |  44  |  54  |  64  |  74  |  74  |
                                       ----------------------------------------------------------------------------------*/

        GEN_RX_SETTLE_TIME_37US = 1, /**<   reduce 44.5us of rx settle time.
                                        Receive for a period of time and then do a normal calibration.*/

        GEN_RX_SETTLE_TIME_77US = 2, /**<  reduce 8.5us of rx settle time.
                                        Do a normal calibration at the beginning.*/
        GEN_RX_FAST_SETTLE_NONE = 3

    } GEN_FSK_RxSettleTimeTypeDef;

    /**
 *  @brief  TX fast settle time
 *  @note
 *  1:Call rf_fast_settle_config to configure timing during initialization.
 *  2:Call the enable function rf_tx_fast_settle_en when using the configured timing sequence.
 *    To close it, call rf_tx_fast_settle_dis.
 *  3:The deleted hardware calibration values are influenced by environmental temperature and require periodic recalibration.
 *    Calibration method: Call rf_tx_fast_settle_dis, then for different frequency points:
 *    stop RF-related states, enable TX, wait for packet transmission to end -> rf_tx_fast_settle_update_cal_val.
 */
    typedef enum
    {
        GEN_TX_SETTLE_TIME_15US = 0,  /**<   reduce 87.5us of tx settle time.
                                       After frequency hopping, a normal calibration must be done.
                                       note:
                                       1. Related to frequency points. When enabling fast settle on both TX and RX ends,
                                       if 15us option is selected, both TX and RX ends must use the 15us option simultaneously.
                                       2. Refer to the following table to determine the range of frequency points used.
                                          Corresponding channel values in the respective intervals need to be configured.
                                          For example, if using the 2426 frequency point, channel should be configured 24.
                                       ----------------------------------------------------------------------------------
                                       | Frequency     2400   2410   2420   2430   2440   2450   2460   2470   2480     |
                                       |-----------------|------|------|------|------|------|------|------|------|------|
                                       | channel Value   |   4  |  14  |  24  |  34  |  44  |  54  |  64  |  74  |  74  |
                                       ----------------------------------------------------------------------------------*/

        GEN_TX_SETTLE_TIME_51US = 1,  /**<   reduce 61.5us of tx settle time.
                                       note: Related to frequency points. After frequency hopping, a normal calibration must be done.*/

        GEN_TX_SETTLE_TIME_104US = 2, /**<   reduce 8.5us of tx settle time.
                                       Do a normal calibration at the beginning.*/
        GEN_TX_FAST_SETTLE_NONE  = 3

    } GEN_FSK_TxSettleTimeTypeDef;

    /**
 * @brief      This function servers to set the radio's TX output power.
 * @param[in]  level  specify the tx power level
 * @param[out] none
 * @return     none.
 */
    extern void gen_fsk_radio_power_set(gen_fsk_radio_power_t level);

    /**
 * @brief      This function servers to set crc for RF.
 * @param[in]  config  crc config address pointer
 * @param[out] none
 * @return     none.
 */
    extern void gen_fsk_set_crc_config(const rf_crc_config_t *config);

    /**
 * @brief      This function servers to set radio's on-air datarate.
 * @param[in]  datarate  specify the radio's datarate
 * @param[out] none
 * @return     none.
 */
    extern void gen_fsk_datarate_set(gen_fsk_datarate_t datarate);

    /**
 * @brief      This function servers to set radio's RF channel.
 *             rf central frequency = (2400 + channel_num) MHz
 * @param[in]  channel_num  specify the radio's channel
 * @param[out] none
 * @return     none.
 */
    extern void gen_fsk_channel_set(signed short channel_num);

    /**
 * @brief      This function servers to set rf transceiver's operation state.
 * @param[in]  state  specify the radio's state
 * @param[out] none
 * @return     none.
 */
    extern void gen_fsk_radio_state_set(gen_fsk_state_t state);

    /**
 * @brief      This function servers to set the length in octet of the preamble
 *             field of the on-air data packet.Note that the valid range is 1-16.
 * @param[in]  preamble_len  specify the length of preamble field in octet.
 * @param[out] none
 * @return     none.
 */
    extern void gen_fsk_preamble_len_set(unsigned char preamble_len);

    /**
 * @brief      This function servers to set the length in octet of the sync word
 *             field of the on-air data packet.
 * @param[in]  length  specify the length of sync word field in octet, note that
 *                     the valid range of it is 3-5.
 * @param[out] none
 * @return     none.
 */
    extern void gen_fsk_sync_word_len_set(gen_fsk_sync_word_len_t length);

    /**
 * @brief      This function servers to set the length in octet of the sync word
 *             field of the on-air data packet.
 * @param[in]  pipe  specify which pipe the sync word is to set for
 * @param[in]  sync_word  pointer to the sync word needs to be set
 * @param[out] none
 * @return     none.
 */
    extern void gen_fsk_sync_word_set(gen_fsk_pipe_id_t pipe_id, unsigned char *sync_word);

    /**
 * @brief      This function servers to open one or all pipes.
 *             field of the on-air data packet.
 * @param[in]  pipe  specify which pipe needs to be open
 * @param[out] none
 * @return     none.
 */
    extern void gen_fsk_pipe_open(gen_fsk_pipe_id_t pipe);

    /**
 * @brief      This function servers to close one or all pipes.
 *             field of the on-air data packet.
 * @param[in]  pipe  specify which pipe needs to be closed
 * @param[out] none
 * @return     none.
 */
    extern void gen_fsk_pipe_close(gen_fsk_pipe_id_t pipe);

    /**
 * @brief      This function servers to set TX pipe manually
 * @param[in]  pipe  The pipe is selected as TX pipe
 * @param[out] none
 * @return     none.
 */
    extern void gen_fsk_tx_pipe_set(gen_fsk_pipe_id_t pipe);


    /**
 * @brief      This function servers to set the paket format of on-air data.
 * @param[in]  format
 *               1)legacy:
 *                    -----------------------------------------------------------------------
 *                    |GEN_FSK_PACKET_FORMAT_FIXED_PAYLOAD:                                 |
 *                    |preamble + sync word + payload + crc, payload is of fixed-length     |
 *                    -----------------------------------------------------------------------
 *                    -----------------------------------------------------------------------
 *                    |GEN_FSK_PACKET_FORMAT_VARIABLE_PAYLOAD:                              |
 *                    |preamble + sync word + header + payload + crc, the header contains   |
 *                    |9bit:6bit length + 2bit pid + 1bit noack                             |
 *                    -----------------------------------------------------------------------
 *               2)generic:
 *                    -----------------------------------------------------------------------
 *                    |GEN_FSK_PACKET_FORMAT_NEW_PAYLOAD:                                   |
 *                    |preamble + sync word + header + payload + crc, the header contains   |
 *                    |h0(0~2byte),length(0~2byte),h1(0~2byte). header must be An integer   |
 *                    |multiple of 8 bits                                                   |
 *                    -----------------------------------------------------------------------
 * @param[in]  payload_len if the GEN_FSK_PACKET_FORMAT_FIXED_PAYLOAD format is selected,
 *                         this parameter specifies the fixed length of payload, otherwise,
 *                         this parameter should be ignored
 * @param[out] none
 * @return     none.
 */
    extern void gen_fsk_packet_format_set(gen_fsk_packet_format_t mode, unsigned short legacy_fixed_plength);

    /**
 * @brief     This function is used to set a fixed offset for the extracted length field for generic format.
 * @param[in] length_adj  - The fixed offset for the extracted length field.
 *                          Length_adj range:-31 <=  length_adj <=31
 * @return    none.
 * @note      If length_adj is configured,the sum of length + length_adj represents the number of payload + crc octets.
 *            1. length_adj configuration positive number, CRC length (byte)
 *            2. Configuration 0, no CRC
 *            3. Configure negative numbers, no CRC, and the payload length is less than the corresponding value
 */
    extern void gen_fsk_set_pri_generic_length_adj(signed char length_adj);

    /**
 * @brief      This function set the packet filter.
 * @param[in]  rf_pkt_flt - RF packet filtering parameters
 * @return     none.
 * @note       1. Filter from high bit to low bit
 *             2. Maximum matching 64bit
 *             3. Interrupt:FLD_RF_IRQ_PKT_MATCH/FLD_RF_IRQ_PKT_UNMATCH
 */
    extern void gen_fsk_set_pkt_filter(rf_pkt_flt_t rf_pkt_flt);

    /**
 * @brief      This function sets the length of CRC field
 * @param[in]  crc_len  length of CRC field
 * @param[out] none
 * @return     none.
 */
    extern void gen_fsk_crc_len_set(gen_fsk_crc_len_t crc_len);

    /**
 * @brief      This function sets DMA RX buffer. Note that the start address of RX
 *             buffer must be 4-byte aligned and the length of RX buffer must be
 *             integral multiple of 16-byte. Also the RX buffer length must be larger
 *             than the whole packet length plus 16 to accommodate data and necessary
 *             RX related information.
 * @param[in]  rx_buffer  pointer to the RX buffer
 * @param[in]  rx_buffer_len  length of RX buffer in octet
 * @param[out] none
 * @return     none.
 */
    extern void gen_fsk_rx_buffer_set(unsigned char *rx_buffer, unsigned char rx_buffer_num, unsigned short rx_buffer_len);

    /**
 * @brief     This function serves to set RF tx DMA setting.
 * @param[in] tx_buffer_num         - tx chn deep,fifo_depth range: 0~5,Number of fifo=2^fifo_depth.
 * @param[in] tx_buffer_len         - The length of one dma fifo,the range is 0x10~0xff0(the corresponding number of fifo bytes is fifo_byte_size;and must be a multiple of 16).
 * @return    none.
 */
    extern void gen_fsk_tx_buffer_set(unsigned char tx_buffer_num, unsigned short tx_buffer_len);

    /**
 * @brief     This function serves to write tx payload into tx buffer and prepare totally tx buffer for different packet format.
 * @param[in] tx_buffer          user define a buffer for tx which must be 4-byte aligned and the length must be integral multiple of 16-byte.
 * @param[in] payload            the point of tx payload.
 * @param[in] payload_length     the length of payload to send.
 * @return    none.
 */
    _attribute_ram_code_sec_noinline_ unsigned int gen_fsk_write_payload(unsigned char *tx_buffer, const unsigned char *payload, unsigned int payload_length);

    /**
 * @brief      This function determines whether the result of RX CRC check is ok.
 * @param[in]  rx_buffer  pointer to the RX buffer containing currently received packet.
 * @param[out] none
 * @return     1: the RX CRC check passes. 0: the RX CRC check fails.
 */
    extern unsigned char gen_fsk_is_rx_crc_ok(unsigned char *rx_buffer);

    /**
 * @brief      This function returns the pointer to the payload field of received packet.
 * @param[in]  rx_buffer  pointer to the RX buffer containing currently received packet.
 * @param[out] payload_len return the length of received payload.
 * @return     pointer to the the payload field of received packet.
 */
    extern unsigned char *gen_fsk_rx_payload_get(unsigned char *rx_buffer, unsigned char *payload_len);

    /**
 * @brief      This function returns the freezing rssi of received packet.
 * @param[in]  rx_buffer  pointer to the RX buffer containing currently received packet.
 * @param[out] none
 * @return     rssi in dBm of the received packet.
 */
    extern signed char gen_fsk_rx_packet_rssi_get(unsigned char *rx_buffer);

    /**
 * @brief      This function returns the instantaneous rssi of current channel.
 * @param[in]  none
 * @param[out] none
 * @return     instantaneous rssi in dBm of current channel.
 */
    extern signed char gen_fsk_rx_instantaneous_rssi_get(void);

    /**
 * @brief      This function returns the timestamp of received packet.
 * @param[in]  rx_buffer  pointer to the RX buffer containing currently received packet.
 * @param[out] none
 * @return     timestamp of the received packet, i.e. the freezing system-timer tick count
 *             value once the sync word is recongnized.
 */
    extern unsigned int gen_fsk_rx_timestamp_get(unsigned char *rx_buffer);

    /**
 * @brief      This function manually starts the transmission of a prepared
 *             packet(i.e. the packet has been written into the TX Buffer) immediately.
 * @param[in]  tx_buffer  pointer to the TX buffer which has been filled with the packet needs transmitting.
 * @param[out] none
 * @return     none
 */
    extern void gen_fsk_tx_start(unsigned char *tx_buffer);

    /**
 *   @brief      This function determines whether the transmission activity started by gen_fsk_tx_start() is finished.
 *   @param[in]  none.
 *   @return     0: tx activity is not done. otherwhise: tx is done
 */
    extern unsigned char gen_fsk_is_tx_done(void);

    /**
 *   @brief      This function serves to clear the Tx done status bit once it is set.
 *
 *   @param      none
 *   @return     none
 */
    extern void gen_fsk_tx_done_status_clear(void);

    /**
 * @brief      This function starts the automatic Single-TX FSM to schedule one transmission activity.
 * @param[in]  tx_buffer  pointer to the TX buffer which has been filled with the packet needs transmitting.
 * @param[in]  start_point  a timer tick count value, when the system-timer's tick count matches this value,
 *                          the transmission activity starts immediately.
 * @param[out] none
 * @return     none
 */
    extern void gen_fsk_stx_start(unsigned char *tx_buffer, unsigned int start_point);

    /**
 * @brief      This function starts the automatic Single-RX FSM to schedule one reception activity.
 * @param[in]  start_point  a timer tick count value, when the system-timer's tick count matches this value,
 *                          the transceiver enters RX state and waits for a packet coming.
 * @param[in]  timeout_us  specifies the period the transceiver will remain in RX state to waits for
 *                         a packet coming, and a RX first timeout irq occurs if no packet is received
 *                         during that period. If timeout_us equals to 0, it denotes the RX first timeout is
 *                         disabled and the transceiver won't exit the RX state until a packet arrives. Note
 *                         that RX settle period is included in timeout_us, namely:
 *                         timeout_us = Rx settle period + Actual Rx period.
 * @param[out] none
 * @return     none
 */
    extern void gen_fsk_srx_start(unsigned int start_point, unsigned int timeout_us);

    /**
 * @brief      This function starts the automatic Single-TX-to-RX FSM to schedule one transmission and
 *             corresponding reception activity. The transceiver enters TX state and start the packet's
 *             transmission when the system timer matches the start_point. Once the trannsmission is done,
 *             the transceiver transits into RX State and waits for a packet coming. The RX state lasts for
 *             timeout_us microseconds at most. If no packet arrives during that period, a RX Timeout irq
 *             will occur immediately. It usually applies to the case where a packet needs transmitting
 *             with an ACK packet required.
 * @param[in]  tx_buffer  pointer to the TX buffer which has been filled with the packet needs transmitting.
 * @param[in]  start_point  a timer tick count value, when the system-timer's tick count matches this value,
 *                          the transceiver transits into TX state and starts the trannsmission of the packet
 *                          in TX Buffer.
 * @param[in]  timeout_us  specifies the period the transceiver will remain in RX state to waits for
 *                         a packet coming, and a RX timeout irq occurs if no packet is received
 *                         during that period. Note that RX settle period is included in timeout_us,
 *                         namely:
 *                         timeout_us = Rx settle period + Actual Rx period.
 * @param[out] none
 * @return     none
 */
    extern void gen_fsk_stx2rx_start(unsigned char *tx_buffer, unsigned int start_point, unsigned int timeout_us);

    /**
 * @brief      This function starts the automatic Single-RX-to-TX FSM to schedule one reception and
 *             corresponding transmission activity. The transceiver enters RX state and waits for a
 *             packet coming. The RX state lasts for timeout_us microseconds at most. If no packet arrives,
 *             a RX First Timeout irq will occur immediately. If a packet is received before the timeout,
 *             the transceiver transits into TX state and start the transmission of the packet in TX Buffer.
 *             It usually applies to the case where a packet needs receiving first and then a response packet
 *             needs sending back.
 * @param[in]  tx_buffer  pointer to the TX buffer which has been filled with the packet needs transmitting.
 * @param[in]  start_point  a timer tick count value, when the system-timer's tick count matches this value,
 *                          the transceiver transits into RX state and waits for a packet coming.
 * @param[in]  timeout_us  specifies the period the transceiver will remain in RX state to waits for
 *                         a packet coming, and a RX first timeout irq occurs if no packet is received
 *                         during that period. If timeout_us equals to 0, it denotes the RX first timeout is
 *                         disabled and the transceiver won't exit the RX state until a packet arrives. Note
 *                         that RX settle period is included in timeout_us, namely:
 *                         timeout_us = Rx settle period + Actual Rx period.
 * @param[out] none
 * @return     none
 */
    extern void gen_fsk_srx2tx_start(unsigned char *tx_buffer, unsigned int start_point, unsigned int timeout_us);

    /**
 * @brief      This function sets the tx settle period of transceiver for automatic Single-TX , Single-TX-to-RX and Single-RX-to-TX.
 *             In those three automatic modes, the actual transmission starts a short while later after the transceiver enters TX
 *             state. That short while is so-called tx settle period which is used to wait for the RF PLL settling down before
 *             transmission. The TX settle period must be larger than 130uS. The default value is 150uS.
 * @param[in]  period_us  specifying the TX settle period in microsecond.
 * @param[out] none
 * @return     none
 */
    void gen_fsk_tx_settle_set(unsigned short period_us);

    /**
 * @brief      This function sets the rx settle period of transceiver for automatic Single-RX, Single-RX-to-TX and Single-TX-to-RX.
 *             In those three automatic modes, the actual reception starts a short while later after the transceiver enters RX
 *             state. That short while is so-called rx settle period which is used to wait for the RF PLL settling down before
 *             reception. The RX settle period must be larger than 85uS. The default value is 90uS.
 * @param[in]  period_us  specifying the RX settle period in microsecond.
 * @param[out] none
 * @return     none
 */
    void gen_fsk_rx_settle_set(unsigned short period_us);

    /**
 * @brief      This function disables the auto pid feature, which is necessary to set the pid manuallly.
 *
 * @param[in]  none
 * @param[out] none
 * @return     none
 */
    void gen_fsk_auto_pid_disable(void);

    /**
 * @brief      This function serves to set the pid field(2bit) manually in the packet header(9bit) of DPL-
 *             format packet.
 * @param[in]  tx_buffer  pointer to the TX buffer which has been filled with the packet needs transmitting.
 * @param[in]  pid  the valule of the pid needs to be set. 0 =< pid <=3.
 * @param[out] none
 * @return     none
 */
    void gen_fsk_set_pid(unsigned char *tx_buffer, unsigned char pid);

    /**
 * @brief       Set the frequency deviation of the transmitter, which follows the equation below.
 *              frequency deviation = bitrate/(modulation index)^2
 *              Note:configure this function before state_machine acting.
 * @param       mi_value    Modulation index.
 * @return      none.
 */
    void gen_fsk_tx_set_mi(GEN_MIVauleTypeDef mi_value);

    /**
 * @brief       Set the frequency deviation of the transmitter, which follows the equation below.
 *              frequency deviation = bitrate/(modulation index)^2
 *              Note:configure this function before state_machine acting.
 * @param       mi_value    Modulation index.
 * @return      none.
 */
    void gen_fsk_rx_set_mi(GEN_MIVauleTypeDef mi_value);
    /**
 *  @brief      This function serve to adjust tx/rx settle timing sequence.
 *  @param[in]  tx_settle_us    After adjusting the timing sequence, the time required for tx to settle.
 *              rx_settle_us    After adjusting the timing sequence, the time required for rx to settle.
 *  @return      0                   -  Correct configuration.
 *              -1                   -  Incorrect configuration.
 */
    void gen_fsk_fast_settle_config(GEN_FSK_TxSettleTimeTypeDef tx_settle_us, GEN_FSK_RxSettleTimeTypeDef rx_settle_us);

    /**
 *  @brief      this function serve to enable the fast tx timing sequence adjusted.
 *  @param[in]  none.
 *  @return     none.
*/
    void gen_fsk_fast_txsettleEnable(void);

    /**
 *  @brief      this function serve to disable the fast tx timing sequence adjusted.
 *  @param[in]  none.
 *  @return     none.
 */
    void gen_fsk_fast_txsettleDisable(void);

    /**
 *  @brief      this function serve to enable the fast rx timing sequence adjusted.
 *  @param[in]  none.
 *  @return     none.
 */
    void gen_fsk_fast_rxsettleEnable(void);

    /**
 *  @brief      this function serve to disable the fast rx timing sequence adjusted.
 *  @param[in]  none.
 *  @return     none.
 */
    void gen_fsk_fast_rxsettleDisable(void);

#endif /* _GEN_FSK_LL_H_ */
    
