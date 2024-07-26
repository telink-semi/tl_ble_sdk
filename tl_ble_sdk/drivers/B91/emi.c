/********************************************************************************************************
 * @file    emi.c
 *
 * @brief   This is the source file for B91
 *
 * @author  Driver Group
 * @date    2019
 *
 * @par     Copyright (c) 2019, Telink Semiconductor (Shanghai) Co., Ltd. ("TELINK")
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
#include "emi.h"
#include "stimer.h"
/**********************************************************************************************************************
 *                                           local macro                                                             *
 *********************************************************************************************************************/
#define EMI_STATE0                           0x1234
#define EMI_STATE1                           0x5678
#define EMI_TX_FIFO_ADDR                     0x14081c
#define EMI_TX_PKT_PAYLOAD                   37
/**
 * @brief This definition is used to set the maximum payload for rx
 * @note  When using it, it should be noted that the packet length sent by tx should not exceed the maximum buffer of rx
 *
 *        packet_length = DMA_length + header_length + payload_length + crc_length + hd_info_length
 *
 *        Common package structure:
 *
 *              | DMA_length | header_length | payload_length               | crc_length | hd_info_length
 *       ble    |   4 byte   |    2 byte     |   N byte                     |   3 byte   | 8 byte
 *       private|   4 byte   |    1 byte     |   N byte                     |   2 byte   | 8 byte
 *       zigbee |   4 byte   |    1 byte     |   N-2(crc_length) byte       |   2 byte   | 8 byte
 *
 */
#define EMI_RX_MAX_PKT_PAYLOAD                   255

/**********************************************************************************************************************
 *                                           global constants                                                        *
 *********************************************************************************************************************/

static unsigned char  emi_rx_packet[280] __attribute__ ((aligned (4)));
static unsigned char  emi_ble_tx_packet [48]  __attribute__ ((aligned (4))) = {3,0,0,0,0,10};
static unsigned char  emi_zigbee_tx_packet[48]  __attribute__ ((aligned (4))) = {19,0,0,0,20,0,0};
static unsigned char  Private_TPLL_tx_packet[48] __attribute__ ((aligned (4))) = {3,0,0,0,0,10};
static unsigned int   s_emi_rx_cnt  __attribute__ ((aligned (4))) = 0;
static unsigned int   s_emi_rssibuf = 0;
static signed  char   s_emi_rssi = 0;
static unsigned int   s_state0,s_state1;

/**********************************************************************************************************************
 *                                          function implementation                                                   *
 *********************************************************************************************************************/

/**
 * @brief      This function serves to set singletone power
 * @param[in]  level - the power level.
 * @return     none
 */
void rf_set_power_level_index_singletone (rf_power_level_e level)
{
    unsigned char value = 0;

    if(level&BIT(7))
    {
        write_reg8(0x140e3c, (read_reg8(0x140e3c) & 0xbf) | ((0x01)<<6));// VANT
    }
    else
    {
        write_reg8(0x140e3c, (read_reg8(0x140e3c) & 0xbf));
    }
    reg_rf_ll_ctrl0 = 0x55;                                              // tx_en
    delay_us(150);

    value = (unsigned char)level&0x3f;
    write_reg8(0x140f78,read_reg8(0x140f78)|BIT(6));                     // TX_PA_PWR_OW  BIT6 set 1
    write_reg8(0x140f7c, (read_reg8(0x140f7c)&0x81)|(value<<1));         // TX_PA_PWR  BIT1 t0 BIT6 set value
}

extern unsigned char g_single_tong_freqoffset;

/**
 * @brief      This function serves to set the TX singletone power and channel
 * @param[in]  power_level - the power level.
 * @param[in]  rf_chn      - the channel.
 * @return     none
 */
void rf_emi_tx_single_tone(rf_power_level_e power_level,signed char rf_chn)
{
    g_single_tong_freqoffset = 1;
    rf_mode_init();
    rf_set_zigbee_250K_mode();
    rf_set_chn(rf_chn);
    rf_set_power_level_index_singletone(power_level);
    rf_set_txmode();
    g_single_tong_freqoffset = 0;

}

/**
 * @brief      This function serves to stop emi/(close RF)
 * @return     none
 */
void rf_emi_stop(void)
{
    write_reg8(0x140f78, 0);
    write_reg8(0x140f7c, 0);//TX_PA_PWR
    rf_set_power_level_index (0);
    rf_set_tx_rx_off();
}

/**
 * @brief      This function serves to set the CD mode correlation register
 * @return     none
 */
void rf_emi_tx_continue_setup(void)
{

    write_reg8(0x140800,0x0a);
    write_reg8(0x140808,0x00);  // access code

    write_reg8(0x140801,0x80);  // kick tx controller to wait data
    s_state0 = EMI_STATE0;
    s_state1 = EMI_STATE1;
}

/**
 * @brief      This function serves to update the CD mode data
 * @param[in]  rf_mode     - mode of RF.
 * @param[in]  power_level - power level of RF.
 * @param[in]  rf_chn      - channel of RF.
 * @param[in]  pkt_type    - The type of data sent.
 * -#0:random
 * -#1:0xf0
 * -#2:0x55
 * @return     none
 */
void rf_emi_tx_continue_update_data(rf_mode_e rf_mode,rf_power_level_e power_level,signed char rf_chn,unsigned char pkt_type)
{
    rf_mode_init();
    switch(rf_mode)
    {
        case RF_MODE_BLE_1M_NO_PN:
            rf_set_ble_1M_NO_PN_mode();
            break;
        case RF_MODE_BLE_2M:
            rf_set_ble_2M_NO_PN_mode();
            break;
        case RF_MODE_LR_S2_500K:
            rf_set_ble_500K_mode();
            break;
        case RF_MODE_LR_S8_125K:
            rf_set_ble_125K_mode();
            break;
        case RF_MODE_ZIGBEE_250K:
            rf_set_zigbee_250K_mode();
            break;
        case RF_MODE_PRIVATE_1M:
            rf_set_pri_1M_mode();
            break;
        case RF_MODE_PRIVATE_2M:
            rf_set_pri_2M_mode();
            break;

        default:break;
    }
    rf_pn_disable();
    rf_set_chn(rf_chn);
    reg_rf_ll_ctrl0 = 0x45;   // reset tx/rx state machine
    rf_set_power_level_index_singletone (power_level);
    rf_emi_tx_continue_setup();
    write_reg8(0x140808, pkt_type);  // 0:pbrs9     1:0xf0   2:0x55
}

/**
 * @brief      This function serves to generate random number.
 * @param[in]  state - the old random number.
 * @return     the new random number
 */
unsigned int emi_pn_gen(unsigned int state)
{
    unsigned int feed = 0;
    feed = (state&0x4000) >> 1;
    state ^= feed;
    state <<= 1;
    state = (state&0xfffe) + ((state&0x8000)>>15);
    return state;
}

/**
 * @brief      This function serves to continue to run the CD mode..The missing three in the program is because 
 *             the original writing is problematic and will 
 * @return     none
 */
void rf_continue_mode_run(void)
{
    if(read_reg8(0x140808) == 1){
        write_reg32(EMI_TX_FIFO_ADDR, 0x0f0f0f0f);
    }else if(read_reg8(0x140808)==2){
        write_reg32(EMI_TX_FIFO_ADDR, 0x55555555);
    }else if(read_reg8(0x140808)==4){
        write_reg32(EMI_TX_FIFO_ADDR, 0);
    }else if(read_reg8(0x140808)==5){
        write_reg32(EMI_TX_FIFO_ADDR, 0xffffffff);
    }else{
        write_reg32(EMI_TX_FIFO_ADDR, (s_state0<<16)+s_state1);
        s_state0 = emi_pn_gen(s_state0);
        s_state1 = emi_pn_gen(s_state1);
    }

    while(read_reg8(EMI_TX_FIFO_ADDR) & 0x1){
    }
}

static unsigned char rxpara_flag = 1;
/**
 * @brief      This function serves to set rx mode and channel
 * @param[in]  mode   - mode of RF.
 * @param[in]  rf_chn - the rx channel.
 * @return     none
 */
void rf_emi_rx_setup(rf_mode_e mode,signed char rf_chn)
{
    rf_mode_init();
    switch(mode)
    {
        case RF_MODE_BLE_1M_NO_PN:
            rf_set_ble_1M_NO_PN_mode();
            break;
        case RF_MODE_BLE_2M:
            rf_set_ble_2M_NO_PN_mode();
            break;
        case RF_MODE_LR_S2_500K:
            rf_set_ble_500K_mode();
            break;
        case RF_MODE_LR_S8_125K:
            rf_set_ble_125K_mode();
            break;
        case RF_MODE_ZIGBEE_250K:
            rf_set_zigbee_250K_mode();
            break;
        case RF_MODE_PRIVATE_1M:
            rf_set_pri_1M_mode();
            break;
        case RF_MODE_PRIVATE_2M:
            rf_set_pri_2M_mode();
            break;

        default:break;
    }
    rf_set_rx_maxlen(EMI_RX_MAX_PKT_PAYLOAD-2);//Rx mode in EMI is manual mode, and only one DMA fifo is used in manual mode.
                                        //If multiple DMA fifo are used, it should be noted that rx packet length cannot be greater than the depth of DMA fifo
    rf_set_rx_dma(emi_rx_packet,0,253);
    rf_pn_disable();
    rf_set_chn(rf_chn);//set freq
    if(mode != RF_MODE_ZIGBEE_250K)
        rf_access_code_comm(EMI_ACCESS_CODE);   //access code
    rf_set_tx_rx_off();
    rf_set_rxmode();
    delay_us(150);
    if(rxpara_flag == 1)
    {
        rf_set_rxpara();
        rxpara_flag = 0;
    }

    if(rf_chn == 24 || rf_chn == 48 || rf_chn == 72)
    {
        rf_ldot_ldo_rxtxlf_bypass_en();
    }
    else
    {
        rf_ldot_ldo_rxtxlf_bypass_dis();
    }
    s_emi_rssi = 0;
    s_emi_rssibuf = 0;
    s_emi_rx_cnt = 0;
}

/**
 * @brief    This function serves to update the number of receiving packet and the RSSI
 * @return   none
 */
void rf_emi_rx_loop(void)
{
    if(rf_get_irq_status(FLD_RF_IRQ_RX))  // rx irq
    {
       if((read_reg8(0x140840) & 0xf0) == 0) // crc err
       {
           s_emi_rssibuf += (read_reg8(0x140c5d));
           if(s_emi_rx_cnt)
           {
               if(s_emi_rssibuf != 0)
               {
                   s_emi_rssibuf >>= 1;
               }
           }
           s_emi_rssi = s_emi_rssibuf - 110;
           s_emi_rx_cnt++;
       }
       rf_clr_irq_status(FLD_RF_IRQ_RX);    // clr rx irq
       write_reg8(0x140a00, 0x80);          // stop cmd
    }
}

/**
 * @brief    This function serves to get the number of packets received
 * @return   the number of packets received
 */
unsigned int rf_emi_get_rxpkt_cnt(void)
{
    return s_emi_rx_cnt;
}

/**
 * @brief    This function serves to get the RSSI of packets received
 * @return   the RSSI of packets received
 */
char rf_emi_get_rssi_avg(void)
{
    return s_emi_rssi;
}


/**
 * @brief      This function serves to generate random packets that need to be sent in burst mode
 * @param[in] *p - the address of random packets.
 * @param[in]  n - the number of random packets.
 * @return     none
 */
void rf_phy_test_prbs9 (unsigned char *p, int n)
{
    unsigned short x = 0x1ff;
    int i = 0;
    int j = 0;
    for ( i=0; i<n; i++)
    {
        unsigned char d = 0;
        for (j=0; j<8; j++)
        {
            if (x & 1)
            {
                d |= BIT(j);
            }
            x = (x >> 1) | (((x<<4) ^ (x<<8)) & 0x100);
        }
        *p++ = d;
    }
}

/**
 * @brief      This function serves to send packets in the burst mode
 * @param[in]  rf_mode  - mode of RF.
 * @param[in]  pkt_type - The type of data sent.
 * -#0:random
 * -#1:0xf0
 * -#2:0x55
 * @return     none
 */

void rf_emi_tx_burst_loop(rf_mode_e rf_mode,unsigned char pkt_type)
{
    unsigned char rf_data_len = EMI_TX_PKT_PAYLOAD+1;
    unsigned int rf_tx_dma_len = rf_tx_packet_dma_len(rf_data_len);
    write_reg8(0x140a00, 0x80); // stop SM
    rf_set_txmode();
    if((rf_mode==RF_MODE_BLE_1M_NO_PN)||(rf_mode==RF_MODE_BLE_2M))//ble
    {
        rf_data_len = EMI_TX_PKT_PAYLOAD+2;
        rf_tx_dma_len = rf_tx_packet_dma_len(rf_data_len);
        emi_ble_tx_packet[4]=0;
        emi_ble_tx_packet[5]=EMI_TX_PKT_PAYLOAD;
        emi_ble_tx_packet[3] = (rf_tx_dma_len >> 24)&0xff;
        emi_ble_tx_packet[2] = (rf_tx_dma_len >> 16)&0xff;
        emi_ble_tx_packet[1] = (rf_tx_dma_len >> 8)&0xff;
        emi_ble_tx_packet[0] = rf_tx_dma_len&0xff;
        rf_start_stx ((void *)emi_ble_tx_packet,read_reg32(0x140200) + 10);
        while(!(rf_get_irq_status(FLD_RF_IRQ_TX)));
        rf_clr_irq_status(FLD_RF_IRQ_TX);

        delay_ms(2);
        if(pkt_type == 0)
            rf_phy_test_prbs9(&emi_ble_tx_packet[6],37);
    }
    else if(rf_mode == RF_MODE_LR_S8_125K)
    {
        rf_data_len = EMI_TX_PKT_PAYLOAD+2;
        rf_tx_dma_len = rf_tx_packet_dma_len(rf_data_len);
        emi_ble_tx_packet[4]=0;
        emi_ble_tx_packet[5]=EMI_TX_PKT_PAYLOAD;
        emi_ble_tx_packet[3] = (rf_tx_dma_len >> 24)&0xff;
        emi_ble_tx_packet[2] = (rf_tx_dma_len >> 16)&0xff;
        emi_ble_tx_packet[1] = (rf_tx_dma_len >> 8)&0xff;
        emi_ble_tx_packet[0] = rf_tx_dma_len&0xff;
        rf_start_stx ((void *)emi_ble_tx_packet,read_reg32(0x140200) + 10);
        while(!(rf_get_irq_status(FLD_RF_IRQ_TX)));
        rf_clr_irq_status(FLD_RF_IRQ_TX);


        delay_ms(2);
        if(pkt_type == 0)
            rf_phy_test_prbs9(&emi_ble_tx_packet[6],37);
    }
    else if(rf_mode == RF_MODE_LR_S2_500K)
    {
        rf_data_len = EMI_TX_PKT_PAYLOAD+2;
        rf_tx_dma_len = rf_tx_packet_dma_len(rf_data_len);
        emi_ble_tx_packet[4]=0;
        emi_ble_tx_packet[5]=EMI_TX_PKT_PAYLOAD;
        emi_ble_tx_packet[3] = (rf_tx_dma_len >> 24)&0xff;
        emi_ble_tx_packet[2] = (rf_tx_dma_len >> 16)&0xff;
        emi_ble_tx_packet[1] = (rf_tx_dma_len >> 8)&0xff;
        emi_ble_tx_packet[0] = rf_tx_dma_len&0xff;
        rf_start_stx ((void *)emi_ble_tx_packet,read_reg32(0x140200) + 10);
        while(!(rf_get_irq_status(FLD_RF_IRQ_TX)));
        rf_clr_irq_status(FLD_RF_IRQ_TX);


        delay_ms(2);
        if(pkt_type == 0)
            rf_phy_test_prbs9(&emi_ble_tx_packet[6],37);
    }
    else if(rf_mode == RF_MODE_ZIGBEE_250K)
    {
        rf_data_len = EMI_TX_PKT_PAYLOAD+1;
        rf_tx_dma_len = rf_tx_packet_dma_len(rf_data_len);
        emi_zigbee_tx_packet[4]=EMI_TX_PKT_PAYLOAD+2;
        emi_zigbee_tx_packet[3] = (rf_tx_dma_len >> 24)&0xff;
        emi_zigbee_tx_packet[2] = (rf_tx_dma_len >> 16)&0xff;
        emi_zigbee_tx_packet[1] = (rf_tx_dma_len >> 8)&0xff;
        emi_zigbee_tx_packet[0] = rf_tx_dma_len&0xff;
        rf_start_stx ((void *)emi_zigbee_tx_packet,read_reg32(0x140200) + 10);
        while(!(rf_get_irq_status(FLD_RF_IRQ_TX)));
        rf_clr_irq_status(FLD_RF_IRQ_TX);


        delay_us(625*2);
        if(pkt_type == 0)
            rf_phy_test_prbs9(&emi_zigbee_tx_packet[5],37);
    }
    else if((rf_mode == RF_MODE_PRIVATE_1M) || (rf_mode == RF_MODE_PRIVATE_2M))
    {
        rf_data_len = EMI_TX_PKT_PAYLOAD+1;
        Private_TPLL_tx_packet[4]=EMI_TX_PKT_PAYLOAD;
        rf_tx_dma_len = rf_tx_packet_dma_len(rf_data_len);
        Private_TPLL_tx_packet[3] = (rf_tx_dma_len >> 24)&0xff;
        Private_TPLL_tx_packet[2] = (rf_tx_dma_len >> 16)&0xff;
        Private_TPLL_tx_packet[1] = (rf_tx_dma_len >> 8)&0xff;
        Private_TPLL_tx_packet[0] = rf_tx_dma_len&0xff;
        rf_start_stx((void *)Private_TPLL_tx_packet, read_reg32(0x140200) + 10);
        while(!(rf_get_irq_status(FLD_RF_IRQ_TX)));
        rf_clr_irq_status(FLD_RF_IRQ_TX);


        delay_ms(1);
        if(pkt_type == 0)
            rf_phy_test_prbs9(&Private_TPLL_tx_packet[5],37);
    }
}


/**
 * @brief      This function serves to set the burst mode
 * @param[in]  rf_mode     - mode of RF.
 * @param[in]  power_level - power level of RF.
 * @param[in]  rf_chn      - channel of RF.
 * @param[in]  pkt_type    - The type of data sent.
 * -#0:random
 * -#1:0xf0
 * -#2:0x55
 * @return     none
 */
void rf_emi_tx_burst_setup(rf_mode_e rf_mode,rf_power_level_e power_level,signed char rf_chn,unsigned char pkt_type)
{
    unsigned char i = 0;
    unsigned char tx_data = 0;
    write_reg8(0x10083c,0x10); // print buffer size set
    rf_set_tx_dma(2,128);
    rf_set_chn(rf_chn);
    rf_mode_init();
    switch(rf_mode)
    {
        case RF_MODE_BLE_1M_NO_PN:
            rf_set_ble_1M_NO_PN_mode();
            break;
        case RF_MODE_BLE_2M:
            rf_set_ble_2M_NO_PN_mode();
            break;
        case RF_MODE_LR_S2_500K:
            rf_set_ble_500K_mode();
            break;
        case RF_MODE_LR_S8_125K:
            rf_set_ble_125K_mode();
            break;
        case RF_MODE_ZIGBEE_250K:
            rf_set_zigbee_250K_mode();
            break;
        case RF_MODE_PRIVATE_1M:
            rf_set_pri_1M_mode();
            break;
        case RF_MODE_PRIVATE_2M:
            rf_set_pri_2M_mode();
            break;
        default:break;
    }
    if(rf_mode != RF_MODE_ZIGBEE_250K)
        rf_access_code_comm(EMI_ACCESS_CODE);   //access code

    rf_pn_disable();
    rf_set_power_level (power_level);
    if(pkt_type == 1)
    {
        tx_data = 0x0f;
    }
    else if(pkt_type == 2)
    {
        tx_data = 0x55;
    }
    else if(pkt_type == 3)
    {
        tx_data = 0xaa;
    }
    else if(pkt_type==4){
        tx_data = 0xf0;
    }
    switch(rf_mode)
    {
        case RF_MODE_LR_S2_500K:
        case RF_MODE_LR_S8_125K:
        case RF_MODE_BLE_1M_NO_PN:
        case RF_MODE_BLE_2M:
            emi_ble_tx_packet[4] = pkt_type;//type
            for(i = 0;i < 37;i++)
            {
                emi_ble_tx_packet[6+i]=tx_data;
            }
            break;
        case RF_MODE_ZIGBEE_250K:
            emi_zigbee_tx_packet[5] = pkt_type; // type
            for(i = 0;i < 37;i++)
            {
                emi_zigbee_tx_packet[5 + i] = tx_data;
            }
            break;
        case RF_MODE_PRIVATE_1M:
        case RF_MODE_PRIVATE_2M:
            Private_TPLL_tx_packet[5] = pkt_type; // type
            for(i = 0;i < 37;i++)
            {
                Private_TPLL_tx_packet[5 + i] = tx_data;
            }
            break;

        default:
            break;
    }

}

/**
 * @brief      This function serves to reset baseband
 * @return     none
 */
void rf_emi_reset_baseband(void)
{
    reg_rst3 &= (~FLD_RST3_ZB);               // reset baseband
    reg_rst3 |= (FLD_RST3_ZB);                // clr baseband
}
