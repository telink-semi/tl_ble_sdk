/********************************************************************************************************
 * @file    keyscan_ana.c
 *
 * @brief   This is the source file for tl322x
 *
 * @author  Driver Group
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
#include "keyscan_ana.h"
#include <string.h>
//#include "printf.h" /* remove for clean warnings */

/* sar adc dma chain configuration */
dma_chain_config_t ks_ana_adc_rx_dma_list_config[2];

ks_ana_threshold_t ks_ana_threshold_g;

dma_config_t ks_ana_adc_rx_dma_config[2]= {
    {
        .dst_req_sel= 0,
        .src_req_sel=DMA_REQ_SAR_ADC_RX,
        .dst_addr_ctrl=DMA_ADDR_INCREMENT,
        .src_addr_ctrl=DMA_ADDR_FIX,
        .dstmode=DMA_NORMAL_MODE,
        .srcmode=DMA_HANDSHAKE_MODE,
        .dstwidth=DMA_CTR_WORD_WIDTH,//must word
        .srcwidth=DMA_CTR_WORD_WIDTH,//must word
        .src_burst_size=0,//must 0
        .read_num_en=0,
        .priority=0,
        .write_num_en=0,
        .auto_en=0,//must 0
    },
    {
        .dst_req_sel= 0,
        .src_req_sel=DMA_REQ_SAR_ADC1_RX,
        .dst_addr_ctrl=DMA_ADDR_INCREMENT,
        .src_addr_ctrl=DMA_ADDR_FIX,
        .dstmode=DMA_NORMAL_MODE,
        .srcmode=DMA_HANDSHAKE_MODE,
        .dstwidth=DMA_CTR_WORD_WIDTH,//must word
        .srcwidth=DMA_CTR_WORD_WIDTH,//must word
        .src_burst_size=0,//must 0
        .read_num_en=0,
        .priority=0,
        .write_num_en=0,
        .auto_en=0,//must 0
    }
};
/**
 * @brief     This function serves to configure adc_dma_chn channel.
 * @param[in] adc_chn          - the ADC channel
 * @param[in] dma_chn          - the DMA channel
 * @param[in] data_buffer  - the DMA data_buffer address
 * @param[in] size_byte    - the DMA buffer size
 * @return    none
 */
void ks_ana_receive_dma(adc_num_e adc_chn, dma_chn_e dma_chn, unsigned short *data_buffer, unsigned int size_byte)
{
    dma_set_address(dma_chn, SAR_ADC_FIFO(adc_chn), (unsigned int)data_buffer);
    dma_set_size(dma_chn, size_byte, DMA_WORD_WIDTH);
    dma_chn_en(dma_chn);
}

void ks_ana_set_rx_dma_config(adc_num_e adc_chn, dma_chn_e dma_chn)
{
    dma_config(dma_chn, &ks_ana_adc_rx_dma_config[adc_chn]);
}

void ks_ana_sar_adc_set_dma_chain_llp(adc_num_e adc_chn, dma_chn_e dma_chn, unsigned short *dst_addr, unsigned int size_byte, dma_chain_config_t *head_of_list)
{
    ks_ana_set_rx_dma_config(adc_chn,dma_chn);
    dma_set_address(dma_chn, SAR_ADC_FIFO(adc_chn), (unsigned int)dst_addr);
    dma_set_size(dma_chn, size_byte, DMA_WORD_WIDTH);
    reg_dma_llp(dma_chn) = (unsigned int)(head_of_list);
}

void ks_ana_rx_dma_add_list_element(adc_num_e adc_chn, dma_chn_e chn,  dma_chain_config_t *node, dma_chain_config_t *llpointer, unsigned short *dst_addr, unsigned int size_byte)
{
    node->dma_chain_ctl      = reg_dma_ctrl(chn) | BIT(0);
    node->dma_chain_src_addr = SAR_ADC_FIFO(adc_chn);
    node->dma_chain_dst_addr = (unsigned int)(dst_addr);
    node->dma_chain_data_len = dma_cal_size(size_byte, DMA_WORD_WIDTH);
    node->dma_chain_llp_ptr  = (unsigned int)(llpointer);
}

void ks_ana_rx_dma_chain_init(adc_num_e adc_chn, dma_chn_e adc_dma_chn, unsigned short *dst_addr, unsigned int size_byte)
{
    ks_ana_sar_adc_set_dma_chain_llp(adc_chn,adc_dma_chn, (unsigned short*)dst_addr,size_byte, &ks_ana_adc_rx_dma_list_config[adc_chn]);
    ks_ana_rx_dma_add_list_element(adc_chn, adc_dma_chn, &ks_ana_adc_rx_dma_list_config[adc_chn], &ks_ana_adc_rx_dma_list_config[adc_chn],(unsigned short*)dst_addr, size_byte);
    dma_chn_en(adc_dma_chn);

}

/**
 * @brief      This function serves to init sar_adc for Analog keyscan.
 * @param[in]  ks_ana_mode      - the Analog Keyscan mode
 * @return     none
 */
static void ks_ana_sar_adc_init(ks_ana_mode_e ks_ana_mode)
{
    if (ks_ana_mode == KEYSCAN_1XADC_8K_ONCE_MODE) {//sar_adc0
        adc_init(ADC0, DMA_M_CHN);
        adc_keyscan_sample_init(ADC0);
        
        adc_power_on(ADC0);
    } else {//sar_adc0 and sar_adc1
        adc_init(ADC0, DMA_M_CHN);
        adc_init(ADC1, DMA_M_CHN);

        adc_keyscan_sample_init(ADC0);
        adc_keyscan_sample_init(ADC1);

        adc_power_on(ADC0);
        adc_power_on(ADC1);
    }
}

/**
 * @brief      This function serves to configure keyscan registers for 8K data rate.
 * @param[in]  ks_ana_cfg           - the keyscan register configuration
 * @param[in]  hall_init_time    -set time from keyscan enable to start first scan, time = (16 * hall_init_time) * (1 / clk)
 * @return     none
 */
static void ks_ana_mode_config(ks_ana_mode_e ks_ana_mode, unsigned char hall_init_time)
{
    ks_ana_config_t ks_ana_cfg = {0};
    switch (ks_ana_mode)
    {
    case KEYSCAN_1XADC_8K_ONCE_MODE:
         ks_ana_cfg = (ks_ana_config_t){
            .one_channel_scan_time = 0x26,
            .channel_switch_to_adc_sample_time = {0x01, 0x00},
            .ks_en_to_scan_once_time = 0x3b,
            .ks_en_to_row_power_on_time = {0x00, 0x05, 0x0a, 0x0f, 0x14, 0x19, 0x1e, 0x23},
            .ks_en_to_row_power_dn_time = {0x1f, 0x24, 0x29, 0x2e, 0x32, 0x37, 0x3b, 0x3b}
        };
         break;
    case KEYSCAN_2XADC_8K_ONCE_MODE:
        ks_ana_cfg = (ks_ana_config_t){
            .one_channel_scan_time = 0x15,
            .channel_switch_to_adc_sample_time = {0x00, 0x00},
            .ks_en_to_scan_once_time = 0x2c,
            .ks_en_to_row_power_on_time = {0x00, 0x02, 0x04, 0x06, 0x08, 0x0a, 0x0c, 0x0e},
            .ks_en_to_row_power_dn_time = {0x1a, 0x1d, 0x1f, 0x22, 0x25, 0x27, 0x2a, 0x2b}
        };
        break;
    case KEYSCAN_2XADC_8K_TWICE_M1_MODE:
        ks_ana_cfg = (ks_ana_config_t){
            .one_channel_scan_time = 0x28,
            .channel_switch_to_adc_sample_time = {0x01, 0x01},
            .ks_en_to_scan_once_time = 0x3f,
            .ks_en_to_second_scan_time = 0x2a,
            .ks_en_to_row_power_on_time = {0x00, 0x05, 0x0a, 0x0f, 0x14, 0x19, 0x1e, 0x23},
            .ks_en_to_row_power_dn_time = {0x20, 0x25, 0x2a, 0x2f, 0x34, 0x39, 0x3e, 0x3e}
        };
        reg_ks_a_setl5 = ks_ana_cfg.ks_en_to_second_scan_time - (0x14 - hall_init_time);
        break;
    case KEYSCAN_2XADC_8K_TWICE_M2_MODE:
        ks_ana_cfg = (ks_ana_config_t){
            .one_channel_scan_time = 0x11,
            .channel_switch_to_adc_sample_time = {0x00, 0x00},
            .ks_en_to_scan_once_time = 0x3b,
            .ks_en_to_second_scan_time = 0x27,
            .ks_en_to_row_power_on_time = {0x00, 0x04, 0x09, 0x0d, 0x12, 0x17, 0x1c, 0x20},
            .ks_en_to_row_power_dn_time = {0x1f, 0x24, 0x29, 0x2e, 0x32, 0x37, 0x3a, 0x3a}
        };
        reg_ks_a_setl5 = ks_ana_cfg.ks_en_to_second_scan_time - (0x14 - hall_init_time);
        break;
    default:
        break;
    }
    /**
     * -# hall_init_time is the time from power-on to operational readiness for the hall key.
     * -# Only ks_en_to_scan_once_time/ks_en_to_second_scan_time/ks_en_to_row_power_dn_time need to vary according to the different Hall materials.(Confirmed by jingsong.luo)
     */
    reg_ks_ctrl0 = 0x30;//fix at 0x30, corresponds to ADC 2M sampling rate and cannot be changed
    reg_ks_a_setl0 = ks_ana_cfg.one_channel_scan_time;
    reg_ks_a_setl1 = ks_ana_cfg.channel_switch_to_adc_sample_time[0];
    reg_ks_a_setl2 = ks_ana_cfg.channel_switch_to_adc_sample_time[1];
    reg_ks_a_setl3 = ks_ana_cfg.ks_en_to_scan_once_time - (0x14 - hall_init_time);
    reg_ks_a_setl4 = hall_init_time;
    for (int i = 0; i < 8; i++) {
        reg_ks_pwr_on_setl(i) = ks_ana_cfg.ks_en_to_row_power_on_time[i];
        reg_ks_pwr_dn_setl(i) = ks_ana_cfg.ks_en_to_row_power_dn_time[i] - (0x14 - hall_init_time);
    }

}

/**
 * @brief      This function serves to init keyscan work mode.
 * @param[in]  ks_ana_mode          - the Keyscan mode
 * @param[in]  hall_init_time    -set time from keyscan enable to start first scan, time = (16 * hall_init_time) * (1 / clk)
 * @return     none
 */
static void ks_ana_mode_init(ks_ana_mode_e ks_ana_mode,unsigned char hall_init_time)
{
    reg_clkks_mode = KS_8K_SRC_XTL24M;

    ks_ana_mode_config(ks_ana_mode, hall_init_time);

    if (ks_ana_mode == KEYSCAN_1XADC_8K_ONCE_MODE) {
        reg_ks_a_en0 = FLD_KS_CAPTURE_IO | FLD_KS_A_IE0;                //release IO after capture stop, keyscan interrupt enable for ADC0
        reg_ks_a_en1 = FLD_KS_ADC_EN | FLD_KS_MOD_A | FLD_KS_CLK_A_EN;
    } else if (ks_ana_mode >= KEYSCAN_2XADC_8K_ONCE_MODE) {
        reg_ks_a_en0 = FLD_KS_CAPTURE_IO | FLD_KS_A_IE0 | FLD_KS_A_IE1; //release IO after capture stop, keyscan interrupt enable for ADC0 and ADC1
        reg_ks_a_en1 = FLD_KS_ADC_EN | FLD_KS_ADC1_EN | FLD_KS_MOD_A | FLD_KS_CLK_A_EN;
    }

    if (ks_ana_mode == KEYSCAN_2XADC_8K_TWICE_M1_MODE) {
        reg_ks_a_en2 = FLD_KS_SCAN2_EN | FLD_KS_IRQ_DLY_EN;
    } else if (ks_ana_mode == KEYSCAN_2XADC_8K_TWICE_M2_MODE) {
        reg_ks_a_en2 = FLD_KS_SCAN2_EN_IO | FLD_KS_SETL_TIME_SCAN2_EN | FLD_KS_IRQ_DLY_EN;
    }

    reg_ks_a_en1 |= FLD_KS_A_EN;//keyscan enable
}

/**
 * @brief      This function serves to init keyscan pin.
 * @param[in]  gpio_pins    - the Keyscan pin 
 * @return     none
 */
static void ks_ana_pin_init(ks_ana_gpio_pin_t gpio_pins) 
{
    //set analog switch channel pins
    for (int i = 0; i < ANA_SWITCH_CHANNEL_NUMBER; i++) {
        gpio_function_dis(gpio_pins.ana_switch_channel_pin[i]);
        gpio_set_mux_function((gpio_func_pin_e)gpio_pins.ana_switch_channel_pin[i], KEYS0_IO);
        /* channel pin use hardware pulldown */
        gpio_set_low_level(gpio_pins.ana_switch_channel_pin[i]);
    }

    //set hall rows power pins
    for (int i = 0; i < HALL_ROWS_POWER_NUMBER; i++) {
        gpio_function_dis(gpio_pins.hall_rows_power_pin[i]);
        gpio_set_mux_function((gpio_func_pin_e)gpio_pins.hall_rows_power_pin[i], KEYS0_IO);
        gpio_set_up_down_res(gpio_pins.hall_rows_power_pin[i], GPIO_PIN_PULLDOWN_100K);
        gpio_set_low_level(gpio_pins.hall_rows_power_pin[i]);
    }

    gpio_function_en(gpio_pins.ana_switch_enable_pin);
    gpio_output_en(gpio_pins.ana_switch_enable_pin);
    gpio_set_low_level(gpio_pins.ana_switch_enable_pin);//low level enable

    reg_ks_a_en1 |= (gpio_pins.pin_group_id << 6);//iomux[7:6]
}

/**
 * @brief      This function serves to set keyscan hall threshold.
 * @param[in]  ks_ana_threshold          - the analog keyscan hall threshold
 * @return     none
*/
void ks_ana_set_hall_threshold(ks_ana_threshold_t ks_ana_threshold)
{
    ks_ana_threshold_g.release_threshold = ks_ana_threshold.release_threshold;
    ks_ana_threshold_g.press_threshold   = ks_ana_threshold.press_threshold;
}

/**
 * @brief      This function serves to init keyscan analog module.
 * @param[in]  ks_ana_gpio_pin          - the analog keyscan gpio pins 
 * @param[in]  ks_ana_mode              - the analog keyscan mode
 * @param[in]  hall_init_time    -set time from keyscan enable to start first scan, time = (16 * hall_init_time) * (1 / clk)
 * @param[in]  ks_ana_threshold          - the analog keyscan hall threshold
 * @return     none
 * @note       Please refer to the precautions of ks_ana_gpio_pin_t when setup gpio pins.
 */
void ks_ana_init(ks_ana_gpio_pin_t ks_ana_gpio_pin, ks_ana_mode_e ks_ana_mode, unsigned char hall_init_time, ks_ana_threshold_t ks_ana_threshold)
{
    reg_rst5 |= FLD_RST5_KEY_SCAN;
    reg_clk_en5 |= FLD_CLK5_KEYSCAN_EN;

    ks_ana_pin_init(ks_ana_gpio_pin);//init pin

    ks_ana_sar_adc_init(ks_ana_mode);//init sar_adc

    ks_ana_set_hall_threshold(ks_ana_threshold);//set hall threshold

    ks_ana_mode_init(ks_ana_mode, hall_init_time);//init mode
}

/**
 * @brief      This function serves to disable keyscan analog module.
 * @return     none
 * @note       the disable order: ks_ana_disable-> dma_chn_dis
 */
void ks_ana_disable(void){
    while(!(( ks_ana_get_irq_status() & (FLD_KS_FRM_END_STA | FLD_KS_FRM_END1_STA))==0x3))
    ks_ana_clr_irq_status(FLD_KS_FRM_END_STA | FLD_KS_FRM_END1_STA);
    reg_rst5 &= ~FLD_RST5_KEY_SCAN;
    reg_clk_en5 &= ~FLD_CLK5_KEYSCAN_EN;
    reg_ks_a_en1 &= ~(FLD_KS_A_EN|FLD_KS_ADC_EN | FLD_KS_ADC1_EN | FLD_KS_MOD_A | FLD_KS_CLK_A_EN);//keyscan enable
    adc_power_off(ADC0);
    adc_power_off(ADC1);
}

/**
 * @brief      This function serves to enable keyscan analog module.
 * @return     none
 * @note       the enable order: ks_ana_enable-> ks_ana_rx_dma_chain_init
 */
void ks_ana_enable(void){
     reg_soft_control(ADC0)|=FLD_FIFO_CLR;
     reg_soft_control(ADC1)|=FLD_FIFO_CLR;
     reg_clk_en5 |= FLD_CLK5_KEYSCAN_EN;
     reg_rst5 |= FLD_RST5_KEY_SCAN;
     adc_power_on(ADC0);
     adc_power_on(ADC1);
     reg_ks_a_en1 = FLD_KS_ADC_EN | FLD_KS_ADC1_EN | FLD_KS_MOD_A | FLD_KS_CLK_A_EN;
     reg_ks_a_en1 |= FLD_KS_A_EN;//keyscan enable//keyscan enable
}
/**********************************************************************************************************************
 *                                          local function implementation                                             *
 *********************************************************************************************************************/
