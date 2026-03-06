/********************************************************************************************************
 * @file    spi.c
 *
 * @brief   This is the source file for B92
 *
 * @author  Driver Group
 * @date    2020
 *
 * @par     Copyright (c) 2020, Telink Semiconductor (Shanghai) Co., Ltd. ("TELINK")
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
#include "spi.h"
#include "clock.h"

static unsigned char s_gspi_tx_dma_chn;
static unsigned char s_gspi_master_rx_dma_chn;
static unsigned char s_gspi_slave_rx_dma_chn;
static unsigned char s_lspi_tx_dma_chn;
static unsigned char s_lspi_master_rx_dma_chn;
static unsigned char s_lspi_slave_rx_dma_chn;

spi_timeout_error_t g_spi_timeout_error[2] = {
    {
     .g_spi_error_timeout_us   = 0xffffffff,
     .spi_timeout_handler      = lspi_timeout_handler,
     .g_spi_error_timeout_code = SPI_API_ERROR_TIMEOUT_NONE,
     },
    {
     .g_spi_error_timeout_us   = 0xffffffff,
     .spi_timeout_handler      = gspi_timeout_handler,
     .g_spi_error_timeout_code = SPI_API_ERROR_TIMEOUT_NONE,
     }
};

dma_config_t gspi_tx_dma_config = {
    .dst_req_sel    = DMA_REQ_GSPI_TX,    //tx req
    .src_req_sel    = 0,
    .dst_addr_ctrl  = DMA_ADDR_FIX,
    .src_addr_ctrl  = DMA_ADDR_INCREMENT, //increment
    .dstmode        = DMA_HANDSHAKE_MODE, //handshake
    .srcmode        = DMA_NORMAL_MODE,
    .dstwidth       = DMA_CTR_WORD_WIDTH, //must word
    .srcwidth       = DMA_CTR_WORD_WIDTH, //must word
    .src_burst_size = 0,                  //master tx dma support burst1(1 word).
    .read_num_en    = 0,
    .priority       = 0,
    .write_num_en   = 0,
    .auto_en        = 0, //must 0
};
dma_config_t gspi_master_rx_dma_config = {
    .dst_req_sel    = 0,
    .src_req_sel    = DMA_REQ_GSPI_RX, //rx req
    .dst_addr_ctrl  = DMA_ADDR_INCREMENT,
    .src_addr_ctrl  = DMA_ADDR_FIX,
    .dstmode        = DMA_NORMAL_MODE,
    .srcmode        = DMA_HANDSHAKE_MODE,
    .dstwidth       = DMA_CTR_WORD_WIDTH, //must word
    .srcwidth       = DMA_CTR_WORD_WIDTH, ////must word
    .src_burst_size = 0,                  //master rx dma support burst1(1 word).
    .read_num_en    = 0,
    .priority       = 0,
    .write_num_en   = 0,
    .auto_en        = 0, //must 0
};
dma_config_t gspi_slave_rx_dma_config = {
    .dst_req_sel    = 0,
    .src_req_sel    = DMA_REQ_GSPI_RX, //rx req
    .dst_addr_ctrl  = DMA_ADDR_INCREMENT,
    .src_addr_ctrl  = DMA_ADDR_FIX,
    .dstmode        = DMA_NORMAL_MODE,
    .srcmode        = DMA_HANDSHAKE_MODE,
    .dstwidth       = DMA_CTR_WORD_WIDTH, //must word
    .srcwidth       = DMA_CTR_WORD_WIDTH, ////must word
    .src_burst_size = 0,                  //not support burst.
    .read_num_en    = 0,
    .priority       = 0,
    .write_num_en   = 1,                  //When write_num_en is set to 1, dma will write back the length information at the address 4 bytes before the destination address.
    .auto_en        = 0,                  //must 0
};
dma_config_t lspi_tx_dma_config = {
    .dst_req_sel    = DMA_REQ_LSPI_TX,    //tx req
    .src_req_sel    = 0,
    .dst_addr_ctrl  = DMA_ADDR_FIX,
    .src_addr_ctrl  = DMA_ADDR_INCREMENT, //increment
    .dstmode        = DMA_HANDSHAKE_MODE, //handshake
    .srcmode        = DMA_NORMAL_MODE,
    .dstwidth       = DMA_CTR_WORD_WIDTH, //must word
    .srcwidth       = DMA_CTR_WORD_WIDTH, //must word
    .src_burst_size = 0,                  //master tx dma support burst4/2/1(4/2/1 word).
    .read_num_en    = 0,
    .priority       = 0,
    .write_num_en   = 0,
    .auto_en        = 0, //must 0
};

dma_config_t lspi_master_rx_dma_config = {
    .dst_req_sel    = 0,
    .src_req_sel    = DMA_REQ_LSPI_RX, //rx req
    .dst_addr_ctrl  = DMA_ADDR_INCREMENT,
    .src_addr_ctrl  = DMA_ADDR_FIX,
    .dstmode        = DMA_NORMAL_MODE,
    .srcmode        = DMA_HANDSHAKE_MODE,
    .dstwidth       = DMA_CTR_WORD_WIDTH, //must word
    .srcwidth       = DMA_CTR_WORD_WIDTH, ////must word
    .src_burst_size = 0,                  //master rx dma support burst2/1(2/1 word).
    .read_num_en    = 0,
    .priority       = 0,
    .write_num_en   = 0,
    .auto_en        = 0, //must 0
};
dma_config_t lspi_slave_rx_dma_config = {
    .dst_req_sel    = 0,
    .src_req_sel    = DMA_REQ_LSPI_RX, //rx req
    .dst_addr_ctrl  = DMA_ADDR_INCREMENT,
    .src_addr_ctrl  = DMA_ADDR_FIX,
    .dstmode        = DMA_NORMAL_MODE,
    .srcmode        = DMA_HANDSHAKE_MODE,
    .dstwidth       = DMA_CTR_WORD_WIDTH, //must word
    .srcwidth       = DMA_CTR_WORD_WIDTH, ////must word
    .src_burst_size = 0,                  //not support burst.
    .read_num_en    = 0,
    .priority       = 0,
    .write_num_en   = 1,                  //When write_num_en is set to 1, dma will write back the length information at the address 4 bytes before the destination address
    .auto_en        = 0,                  //must 0
};

/**
 * @brief     This function serves to record the api status.
 * @param[in] spi_api_status - spi_api_error_timeout_code_e.
 * @return    none.
 * @note      This function can be rewritten according to the application scenario.
 */
__attribute__((weak)) void lspi_timeout_handler(unsigned int spi_error_timeout_code)
{
    g_spi_timeout_error[LSPI_MODULE].g_spi_error_timeout_code = spi_error_timeout_code;
}

__attribute__((weak)) void gspi_timeout_handler(unsigned int spi_error_timeout_code)
{
    g_spi_timeout_error[GSPI_MODULE].g_spi_error_timeout_code = spi_error_timeout_code;
}

/**
 * @brief     This function serves to set the spi timeout(us).
 * @param[in] spi_sel    - the spi module.
 * @param[in] timeout_us - the timeout(us).
 * @return    none.
 * @note      The default timeout (g_spi_error_timeout_us) is the larger value.If the timeout exceeds the feed dog time and triggers a watchdog restart,
 *            g_spi_error_timeout_us can be changed to a smaller value via this interface, depending on the application.
 *            The minimum time for g_spi_error_timeout_us is related to the following factors:
 *            1.spi clock rate;
 *            2.cclk clock rate;
 *            3.spi's data line(SPI_SINGLE_MODE, SPI_DUAL_MODE, SPI_QUAD_MODE);
 *            4.maximum interrupt processing time;
 *            5.spi write/read data length (especially SPI_WAIT_BUSY);
 *            Solution in case of timeout exit:
 *            when timeout exits, solution:
 *            reset SPI(as master or slave) module,corresponding api:spi_hw_fsm_reset;
 */
void spi_set_error_timeout(spi_sel_e spi_sel, unsigned int timeout_us)
{
    g_spi_timeout_error[spi_sel].g_spi_error_timeout_us = timeout_us;
}

/**
 * @brief     This function serves to return the spi api error timeout code.
 * @param[in] spi_sel    - the spi module.
 * @return    none.
 */
spi_api_error_timeout_code_e spi_get_error_timeout_code(spi_sel_e spi_sel)
{
    return g_spi_timeout_error[spi_sel].g_spi_error_timeout_code;
}

/**
 * @brief       This function servers to determine whether the txfifo is full.
 * @param[in]   spi_sel     - the spi module.
 * @return      1:Indicates that the txfifo is full. 0:Indicates that the txfifo is not full.
 */
bool spi_txfifo_is_full(spi_sel_e spi_sel)
{
    return reg_spi_txfifo_status(spi_sel) & FLD_SPI_TXF_FULL;
}

/**
 * @brief       This function servers to determine whether the rxfifo is empty.
 * @param[in]   spi_sel     - the spi module.
 * @return      1:Indicates that the rxfifo is empty. 0:Indicates that the rxfifo is not empty.
 */
bool spi_rxfifo_is_empty(spi_sel_e spi_sel)
{
    return reg_spi_rxfifo_status(spi_sel) & FLD_SPI_RXF_EMPTY;
}

/**
 * @brief       This function determines whether tx_fifo can continue to write a word.
 * @param[in]   spi_sel     - the spi module.
 * @return      1: cannot continue to write. 0: can continue to write.
 */
bool spi_txfifo_num_is_word(spi_sel_e spi_sel)
{
    unsigned char tx_fifo_depth = 0;

    (spi_sel == LSPI_MODULE) ? (tx_fifo_depth = 20) : (tx_fifo_depth = 8);

    return (tx_fifo_depth - (reg_spi_txfifo_status(spi_sel) & FLD_SPI_TXF_ENTRIES) < 4);
}

/**
 * @brief       This function determines whether rx_fifo can continue to read a word.
 * @param[in]   spi_sel     - the spi module.
 * @return      1:cannot continue to read. 0:can continue to read.
 */
bool spi_rxfifo_num_is_word(spi_sel_e spi_sel)
{
    return ((reg_spi_rxfifo_status(spi_sel) & FLD_SPI_RXF_ENTRIES) < 4);
}

/**
 * @brief      This function selects  pin  for gspi master or slave mode.
 * @param[in]  pin  - the selected pin.
 * @return     none
 */
void gspi_set_pin_mux(gpio_func_pin_e pin, gpio_func_e function)
{
    if (pin != 0) {
        /**
         * When configuring the mux pin of SPI, pull up the CSN configuration.
         * The chip SPI defaults to the master, and when the slave configures the pin as CSN, since the CSN is floating and in the input state,
         * an end interrupt may be generated by external influences.
         * Added by jiarong.ji on July 22, 2021
         */
        if (GSPI_CSN0_IO == function) {
            gpio_set_up_down_res((gpio_pin_e)pin, GPIO_PIN_PULLUP_10K);
        }
        /**
         * Move the operation of gpio_input_en() to the first.
         * Added by jiarong.ji on July 22, 2021
         */
        gpio_input_en((gpio_pin_e)pin);
        reg_gpio_func_mux(pin) = function;
        gpio_function_dis((gpio_pin_e)pin);
    }
}

/**
 * @brief       This function enable gspi csn pin.
 * @param[in]   pin - the csn pin.
 * @return      none
 */
void gspi_cs_pin_en(gpio_func_pin_e pin)
{
    gspi_set_pin_mux((gpio_func_pin_e)pin, GSPI_CSN0_IO);
}

/**
 * @brief       This function disable gspi csn pin.
 * @param[in]   pin - the csn pin.
 * @return      none
 */
void gspi_cs_pin_dis(gpio_func_pin_e pin)
{
    /**
     * Bug fix: Move the operation of gpio_function_en to the end of the function.
     *      changed by chaofan.20210301.
     */
    gpio_output_en((gpio_pin_e)pin);
    gpio_input_dis((gpio_pin_e)pin);
    gpio_set_high_level((gpio_pin_e)pin);
    gpio_function_en((gpio_pin_e)pin);
}

/**
 * @brief       This function change gspi csn pin.
 * @param[in]   current_csn_pin - the current csn pin.
 * @param[in]   next_csn_pin - the next csn pin.
 * @return      none.
 */
void gspi_change_csn_pin(gpio_func_pin_e current_csn_pin, gpio_func_pin_e next_csn_pin)
{
    gspi_cs_pin_dis(current_csn_pin);
    gspi_cs_pin_en(next_csn_pin);
}

/**
 * @brief      This function selects  pin  for lspi master or slave mode.
 * @param[in]  pin  - the selected pin.
 * @return     none
 */
void lspi_set_pin_mux(gpio_pin_e pin)
{
    if (pin != 0) {
        /**
         * When configuring the mux pin of SPI, pull up the CSN configuration.
         * The chip SPI defaults to the master, and when the slave configures the pin as CSN, since the CSN is floating and in the input state,
         * an end interrupt may be generated by external influences.
         * Added by jiarong.ji on July 22, 2021
         */
        if (pin == (gpio_pin_e)LSPI_CSN_PE0_PIN) {
            gpio_set_up_down_res(pin, GPIO_PIN_PULLUP_10K);
        }
        /**
         * Move the operation of gpio_input_en() to the first.
         * Added by jiarong.ji on July 22, 2021
         */
        gpio_input_en(pin);
        gpio_function_dis(pin);
    }
}

/**
 * @brief       This function servers to set gspi pin.
 * @param[in]   gspi_pin_config - the pointer of pin config struct.
 * @return      none
 */
void gspi_set_pin(gspi_pin_config_t *spi_pin_config)
{
    gspi_set_pin_mux(spi_pin_config->spi_clk_pin, GSPI_CLK_IO);
    gspi_set_pin_mux(spi_pin_config->spi_csn_pin, GSPI_CSN0_IO);
    gspi_set_pin_mux(spi_pin_config->spi_mosi_io0_pin, GSPI_MOSI_IO);
    gspi_set_pin_mux(spi_pin_config->spi_miso_io1_pin, GSPI_MISO_IO);
    gspi_set_pin_mux(spi_pin_config->spi_io2_pin, GSPI_IO2_IO);
    gspi_set_pin_mux(spi_pin_config->spi_io3_pin, GSPI_IO3_IO);
}

/**
 * @brief       This function servers to set gspi pin for XIP.
 * @param[in]   gspi_xip_pin_config - the pointer of pin config struct.
 * @return      none
 */
void gspi_set_xip_pin(gspi_xip_pin_config_t *gspi_xip_pin_config)
{
    gspi_set_pin_mux(gspi_xip_pin_config->gspi_clk_pin, GSPI_CLK_IO);
    gspi_set_pin_mux(gspi_xip_pin_config->gspi_csn0_pin, GSPI_CSN0_IO);
    gspi_set_pin_mux(gspi_xip_pin_config->gspi_csn1_pin, GSPI_CSN1);
    gspi_set_pin_mux(gspi_xip_pin_config->gspi_csn2_pin, GSPI_CSN2);
    gspi_set_pin_mux(gspi_xip_pin_config->gspi_csn3_pin, GSPI_CSN3);
    gspi_set_pin_mux(gspi_xip_pin_config->gspi_mosi_io0_pin, GSPI_MOSI_IO);
    gspi_set_pin_mux(gspi_xip_pin_config->gspi_miso_io1_pin, GSPI_MISO_IO);
    gspi_set_pin_mux(gspi_xip_pin_config->gspi_io2_pin, GSPI_IO2_IO);
    gspi_set_pin_mux(gspi_xip_pin_config->gspi_io3_pin, GSPI_IO3_IO);
}

/**
 * @brief       This function servers to set lspi pin.
 * @param[in]   spi_pin_config - the pointer of pin config struct.
 * @return      none
 */
void lspi_set_pin(lspi_pin_config_t *spi_pin_config)
{
    lspi_set_pin_mux((gpio_pin_e)spi_pin_config->spi_clk_pin);
    lspi_set_pin_mux((gpio_pin_e)spi_pin_config->spi_csn_pin);
    lspi_set_pin_mux((gpio_pin_e)spi_pin_config->spi_mosi_io0_pin);
    lspi_set_pin_mux((gpio_pin_e)spi_pin_config->spi_miso_io1_pin);
    lspi_set_pin_mux((gpio_pin_e)spi_pin_config->spi_io2_pin);
    lspi_set_pin_mux((gpio_pin_e)spi_pin_config->spi_io3_pin);
}

/**
 * @brief   This function set pin for spi slave module.
 *          set PC0 as csn,PC1 as clk,PC2 as mosi_io0,PC3 as miso_io0,
 * @return  none
 */
void spi_slave_set_pin(void)
{
    gpio_input_en(GPIO_PC0 | GPIO_PC1 | GPIO_PC2 | GPIO_PC3);
    gpio_function_dis(GPIO_PC0 | GPIO_PC1 | GPIO_PC2 | GPIO_PC3); //set PC0 as csn,PC1 as clk,PC2 as mosi_io0,PC3 as mmiso_io0,
}

/**
 * @brief       This function configures the clock and working mode for SPI interface.
 * @param[in]   spi_sel     - the spi module.
 * @param[in]   div_clock   - the division factor for SPI module.
 *              spi_clock_out = pll_clk / div_clock
 * @param[in]    mode       - the selected working mode of SPI module.
 *               bit2: CPHA-SPI_CLK Phase,bit3: CPOL-SPI_CLK Polarity
 *              MODE0:  CPOL = 0 , CPHA = 0;
 *              MODE1:  CPOL = 0 , CPHA = 1;
 *              MODE2:  CPOL = 1 , CPHA = 0;
 *              MODE3:  CPOL = 1 , CPHA = 1;
 * @return      none
 */
void spi_master_init(spi_sel_e spi_sel, unsigned short div_clock, spi_mode_type_e mode)
{
    switch (spi_sel) {
    case LSPI_MODULE:
        if (div_clock > 63) { //LSPI clock source select pll_clk when div_clock <= 63,select RC 24M when div_clock > 63.
            reg_lspi_clk_set = ((FLD_LSPI_CLK_MOD & (unsigned char)(div_clock * 24 / (sys_clk.pll_clk))) | FLD_LSPI_DIV_RSTN);
        } else {
            reg_lspi_clk_set = ((FLD_LSPI_CLK_MOD & (unsigned char)div_clock) | FLD_LSPI_DIV_RSTN | FLD_LSPI_DIV_IN_SEL);
        }
        break;
    case GSPI_MODULE:
        if (div_clock > 255) { //GSPI clock source select pll_clk when div_clock <= 255,select RC 24M when div_clock > 255.
            reg_gspi_clk_set = ((FLD_GSPI_CLK_MOD & (unsigned char)(div_clock * 24 / (sys_clk.pll_clk))) | FLD_GSPI_DIV_RSTN);
        } else {
            reg_gspi_clk_set = ((FLD_GSPI_CLK_MOD & (unsigned char)div_clock) | FLD_GSPI_DIV_RSTN | FLD_GSPI_DIV_IN_SEL);
        }
        break;
    }
    reg_spi_ctrl3(spi_sel) |= (FLD_SPI_MASTER_MODE | FLD_SPI_DMATX_SOF_CLRTXF_EN | FLD_SPI_DMARX_EOF_CLRRXF_EN | FLD_SPI_AUTO_HREADY_EN); //master
    reg_spi_ctrl3(spi_sel) = ((reg_spi_ctrl3(spi_sel) & (~FLD_SPI_WORK_MODE)) | (mode << 2));                                             // select SPI mode, support four modes
    spi_rx_tx_irq_trig_cnt(spi_sel);
    spi_xip_dis(spi_sel);
}

/**
 * @brief       This function servers to set the number of bytes to triggered the receive and transmit interrupt.
 * @param[in]   spi_sel - the spi module.
 * @return      none
 */
void spi_rx_tx_irq_trig_cnt(spi_sel_e spi_sel)
{
    switch (spi_sel) {
    case LSPI_MODULE: //lspi with rxfifo deepth = 12 bytes,rx dma support burst 2.
        spi_rx_irq_trig_cnt(spi_sel, 8);
        break;
    case GSPI_MODULE: //gspi with rxfifo deepth = 8 bytes,rx dma not support burst.
        spi_rx_irq_trig_cnt(spi_sel, 4);
        break;
    }
    spi_tx_irq_trig_cnt(spi_sel, 4);
}

/**
 * @brief       This function configures the clock and working mode for SPI interface.
 * @param[in]   spi_sel - the spi module.
 * @param[in]   mode    - the selected working mode of SPI module.
 *               bit2: CPHA-SPI_CLK Phase,bit3: CPOL-SPI_CLK Polarity
 *              MODE0:  CPOL = 0 , CPHA = 0;
 *              MODE1:  CPOL = 0 , CPHA = 1;
 *              MODE2:  CPOL = 1 , CPHA = 0;
 *              MODE3:  CPOL = 1 , CPHA = 1;
 * @return      none
 * @note        spi_clock_in  <= slave'hclk/6
 */
void spi_slave_init(spi_sel_e spi_sel, spi_mode_type_e mode)
{
    switch (spi_sel) {
    case LSPI_MODULE: //set SPI clock as same as hclk.
        reg_lspi_clk_set = ((FLD_LSPI_CLK_MOD & (unsigned char)(sys_clk.pll_clk / sys_clk.hclk)) | FLD_LSPI_DIV_RSTN | FLD_LSPI_DIV_IN_SEL);
        break;
    case GSPI_MODULE:
        reg_gspi_clk_set = ((FLD_GSPI_CLK_MOD & (unsigned char)(sys_clk.pll_clk / sys_clk.hclk)) | FLD_GSPI_DIV_RSTN | FLD_GSPI_DIV_IN_SEL);
        break;
    }
    reg_spi_ctrl3(spi_sel) &= (~FLD_SPI_MASTER_MODE);                                                               //slave
    reg_spi_ctrl3(spi_sel) |= (FLD_SPI_DMATX_SOF_CLRTXF_EN | FLD_SPI_DMARX_EOF_CLRRXF_EN | FLD_SPI_AUTO_HREADY_EN); //slave
    reg_spi_ctrl3(spi_sel) = ((reg_spi_ctrl3(spi_sel) & (~FLD_SPI_WORK_MODE)) | (mode << 2));                       // select SPI mode, support four modes
    spi_rx_irq_trig_cnt(spi_sel, 4);
    spi_tx_irq_trig_cnt(spi_sel, 4);
    spi_xip_dis(spi_sel);
}

/**
 * @brief       This function servers to set dummy cycle cnt.
 * @param[in]   spi_sel     - the spi module.
 * @param[in]   dummy_cnt   - the cnt of dummy clock.
 * @return      none
 */
void spi_set_dummy_cnt(spi_sel_e spi_sel, unsigned char dummy_cnt)
{
    reg_spi_ctrl2(spi_sel) = ((reg_spi_ctrl2(spi_sel) & (~FLD_SPI_DUMMY_CNT)) | ((dummy_cnt - 1) & FLD_SPI_DUMMY_CNT));
    reg_spi_ctrl4(spi_sel) = ((reg_spi_ctrl4(spi_sel) & (~FLD_SPI_DUMMY_CNT_ADD)) | ((dummy_cnt - 1) & FLD_SPI_DUMMY_CNT_ADD));
}

/**
 * @brief       This function servers to set spi transfer mode.
 * @param[in]   spi_sel - the spi module.
 * @param[in]   mode    - transfer mode.
 * @return      none
 */
void spi_set_transmode(spi_sel_e spi_sel, spi_tans_mode_e mode)
{
    reg_spi_ctrl2(spi_sel) = ((reg_spi_ctrl2(spi_sel) & (~FLD_SPI_TRANSMODE)) | ((mode & 0xf) << 4));
}

/**
 * @brief       This function servers to set normal mode.
 * @param[in]   spi_sel     - the spi module.
 * @return      none
 */
void spi_set_normal_mode(spi_sel_e spi_sel)
{
    spi_dual_mode_dis(spi_sel);
    spi_3line_mode_dis(spi_sel);
    spi_quad_mode_dis(spi_sel);
}

/**
 * @brief       This function servers to set dual mode.
 * @param[in]   spi_sel     - the spi module.
 * @return      none
 */
void spi_set_dual_mode(spi_sel_e spi_sel)
{
    spi_dual_mode_en(spi_sel); //quad  precede over dual
    spi_3line_mode_dis(spi_sel);
    spi_quad_mode_dis(spi_sel);
}

/**
 * @brief   This function servers to set quad mode.
 * @return  none
 */
void spi_set_quad_mode(spi_sel_e spi_sel)
{
    spi_quad_mode_en(spi_sel);
    spi_dual_mode_dis(spi_sel);
    spi_3line_mode_dis(spi_sel);
}

/**
 * @brief       This function servers to set 3line mode.
 * @param[in]   spi_sel     - the spi module.
 * @return      none
 */
void spi_set_3line_mode(spi_sel_e spi_sel)
{
    /*must disable dual and quad*/
    spi_3line_mode_en(spi_sel);
    spi_dual_mode_dis(spi_sel);
    spi_quad_mode_dis(spi_sel);
}

/**
 * @brief       This function servers to set spi io  mode.
 * @param[in]   spi_sel - the spi module.
 * @param[in]   mode    - single/dual/quad /3line.
 * @return      none
  */
void spi_set_io_mode(spi_sel_e spi_sel, spi_io_mode_e mode)
{
    switch (mode) {
    case SPI_SINGLE_MODE:
        spi_set_normal_mode(spi_sel);
        break;
    case SPI_DUAL_MODE:
        spi_set_dual_mode(spi_sel);
        break;
    case SPI_QUAD_MODE:
        spi_set_quad_mode(spi_sel);
        break;
    case SPI_3_LINE_MODE:
        spi_set_3line_mode(spi_sel);
        break;
    }
}

/**
 * @brief       This function servers to config normal mode.
 * @param[in]   spi_sel - the spi module.
 * @param[in]   mode    - normal ,mode 3line.
 * @return      none
 */
void spi_master_config(spi_sel_e spi_sel, spi_normal_3line_mode_e mode)
{
    spi_cmd_dis(spi_sel);
    spi_addr_dis(spi_sel);
    spi_set_io_mode(spi_sel, (spi_io_mode_e)mode);
}

/**
 * @brief       This function servers to config spi special mode.
 * @param[in]   spi_sel     - the spi module.
 * @param[in]   config  - the pointer of pin special config struct.
 * @return      none
 */
void spi_master_config_plus(spi_sel_e spi_sel, spi_wr_rd_config_t *config)
{
    spi_set_io_mode(spi_sel, config->spi_io_mode);
    spi_set_addr_len(spi_sel, config->spi_addr_len);
    spi_set_dummy_cnt(spi_sel, config->spi_dummy_cnt);

    if (1 == config->spi_cmd_en) {
        spi_cmd_en(spi_sel);
    } else if (0 == config->spi_cmd_en) {
        spi_cmd_dis(spi_sel);
    }

    if (1 == config->spi_cmd_fmt_en) {
        spi_cmd_fmt_en(spi_sel);
    } else if (0 == config->spi_cmd_fmt_en) {
        spi_cmd_fmt_dis(spi_sel);
    }

    if (1 == config->spi_addr_en) {
        spi_addr_en(spi_sel);
    } else if (0 == config->spi_addr_en) {
        spi_addr_dis(spi_sel);
    }

    if (1 == config->spi_addr_fmt_en) {
        spi_addr_fmt_en(spi_sel);
    } else if (0 == config->spi_addr_fmt_en) {
        spi_addr_fmt_dis(spi_sel);
    }
}

/**
 * @brief       This function servers to send command by spi.
 * @param[in]   spi_sel - the spi module.
 * @param[in]   cmd - command.
 * @return      DRV_API_SUCCESS:command sent successfully. others:fail to send command.
 *              DRV_API_TIMEOUT:timeout exit(solution refer to the note for spi_set_error_timeout).
 */
drv_api_status_e spi_master_send_cmd(spi_sel_e spi_sel, unsigned char cmd)
{
    spi_tx_fifo_clr(spi_sel);
    spi_set_transmode(spi_sel, SPI_MODE_NONE_DATA); //nodata
    spi_set_cmd(spi_sel, cmd);
    if (SPI_WAIT(spi_is_busy, spi_sel, g_spi_timeout_error[spi_sel].g_spi_error_timeout_us, g_spi_timeout_error[spi_sel].spi_timeout_handler, SPI_API_ERROR_TIMEOUT_BUS_BUSY)) {
        return DRV_API_TIMEOUT;
    }
    return DRV_API_SUCCESS;
}

/**
 * @brief       This function servers to write spi fifo.
 * tx_fifo_depth are fixed sizes.
 * lspi with txfifo depth = 20 bytes
 * gspi with txfifo depth = 8 bytes
 * @param[in]   spi_sel - the spi module.
 * @param[in]   data    - the pointer to the data for write.
 * @param[in]   len     - write length.
 * @return      DRV_API_SUCCESS:data write to txfifo successfully. others:data write to txfifo failed.
 *              DRV_API_TIMEOUT:timeout exit(solution refer to the note for spi_set_error_timeout).
                Master timeout will only be affected by their own factors, slave timeout will not only be affected by their own factors,
                will also be affected by the master side of the influence.
 *
 */
drv_api_status_e spi_write(spi_sel_e spi_sel, unsigned char *data, unsigned int len)
{
    unsigned int  word_len   = len >> 2;
    unsigned char single_len = len & 3;
    //When the remaining size in tx_fifo is not less than 4 bytes, the MCU moves the data according to the word length.
    for (unsigned int i = 0; i < word_len; i++) {
        if (SPI_WAIT(spi_txfifo_num_is_word, spi_sel, g_spi_timeout_error[spi_sel].g_spi_error_timeout_us, g_spi_timeout_error[spi_sel].spi_timeout_handler, SPI_API_ERROR_TIMEOUT_TXFIFO_NUM_WORD)) {
            return DRV_API_TIMEOUT;
        }
        reg_spi_wr_rd_data_word(spi_sel) = ((unsigned int *)data)[i];
    }
    //When the remaining size in tx_fifo is less than 4 bytes, the MCU moves the data according to the byte length.
    for (unsigned int i = 0; i < single_len; i++) {
        if (SPI_WAIT(spi_txfifo_is_full, spi_sel, g_spi_timeout_error[spi_sel].g_spi_error_timeout_us, g_spi_timeout_error[spi_sel].spi_timeout_handler, SPI_API_ERROR_TIMEOUT_TXFIFO_FULL)) {
            return DRV_API_TIMEOUT;
        }
        reg_spi_wr_rd_data(spi_sel, i % 4) = data[(word_len * 4) + i];
    }
    return DRV_API_SUCCESS;
}

/**
 * @brief       This function servers to read spi fifo.
 * rx_fifo_depth are fixed sizes.
 * lspi with rxfifo deepth = 12 bytes
 * gspi with rxfifo deepth = 8 bytes
 * @param[in]   spi_sel - the spi module.
 * @param[in]   data    - the pointer to the data for read.
 * @param[in]   len     - write length.
 * @return      DRV_API_SUCCESS:data fetched from rxfifo successfully. others:data fetched from rxfifo failed.
 *              DRV_API_TIMEOUT:timeout exit(solution refer to the note for spi_set_error_timeout).
                Master timeout will only be affected by their own factors, slave timeout will not only be affected by their own factors,
                will also be affected by the master side of the influence.
 */
drv_api_status_e spi_read(spi_sel_e spi_sel, unsigned char *data, unsigned int len)
{
    unsigned int  word_len   = len >> 2;
    unsigned char single_len = len & 3;
    //When the data size in rx_fifo is not less than 4 bytes, the MCU moves the data according to the word length
    for (unsigned int i = 0; i < word_len; i++) {
        if (SPI_WAIT(spi_rxfifo_num_is_word, spi_sel, g_spi_timeout_error[spi_sel].g_spi_error_timeout_us, g_spi_timeout_error[spi_sel].spi_timeout_handler, SPI_API_ERROR_TIMEOUT_RXFIFO_NUM_WORD)) {
            return DRV_API_TIMEOUT;
        }
        ((unsigned int *)data)[i] = reg_spi_wr_rd_data_word(spi_sel);
    }
    //When the data size in rx_fifo is less than 4 bytes, the MCU moves the data according to the word length
    for (unsigned char i = 0; i < single_len; i++) {
        if (SPI_WAIT(spi_rxfifo_is_empty, spi_sel, g_spi_timeout_error[spi_sel].g_spi_error_timeout_us, g_spi_timeout_error[spi_sel].spi_timeout_handler, SPI_API_ERROR_TIMEOUT_RXFIFO_EMPTY)) {
            return DRV_API_TIMEOUT;
        }
        data[(word_len * 4) + i] = reg_spi_wr_rd_data((spi_sel), i % 4);
    }
    return DRV_API_SUCCESS;
}

/**
 * @brief       This function serves to normal write data in normal.
 * @param[in]   spi_sel - the spi module.
 * @param[in]   data    - the pointer to the data for write.
 * @param[in]   len     - write length.
 * @return      DRV_API_SUCCESS:master write data successfully. others:master write data failed.
 *              DRV_API_TIMEOUT:timeout exit(solution refer to the note for spi_set_error_timeout).
 */
drv_api_status_e spi_master_write(spi_sel_e spi_sel, unsigned char *data, unsigned int len)
{
    spi_tx_dma_dis(spi_sel);
    spi_tx_fifo_clr(spi_sel);
    spi_tx_cnt(spi_sel, len);
    spi_set_transmode(spi_sel, SPI_MODE_WRITE_ONLY);
    spi_set_cmd(spi_sel, 0x00); //when  cmd  disable that  will not sent cmd,just trigger spi send .
    spi_write(spi_sel, (unsigned char *)data, len);
    if (SPI_WAIT(spi_is_busy, spi_sel, g_spi_timeout_error[spi_sel].g_spi_error_timeout_us, g_spi_timeout_error[spi_sel].spi_timeout_handler, SPI_API_ERROR_TIMEOUT_BUS_BUSY)) {
        return DRV_API_TIMEOUT;
    }
    return DRV_API_SUCCESS;
}

/**
 * @brief       This function serves to normal write and read data.
 * This interface cannot be used for full duplex.
 * rd_len shouldn't set to 0. Must write first, then read.
 * @param[in]   spi_sel - the spi module.
 * @param[in]   wr_data - the pointer to the data for write.
 * @param[in]   wr_len  - write length.
 * @param[in]   rd_data - the pointer to the data for read.
 * @param[in]   rd_len  - read length.
 * @return      DRV_API_SUCCESS:master write read data successfully. others:master write read data failed.
 *              DRV_API_TIMEOUT:timeout exit(solution refer to the note for spi_set_error_timeout).
 */
drv_api_status_e spi_master_write_read(spi_sel_e spi_sel, unsigned char *wr_data, unsigned int wr_len, unsigned char *rd_data, unsigned int rd_len)
{
    spi_tx_dma_dis(spi_sel);
    spi_rx_dma_dis(spi_sel);
    spi_tx_fifo_clr(spi_sel);
    spi_rx_fifo_clr(spi_sel);
    spi_tx_cnt(spi_sel, wr_len);
    spi_rx_cnt(spi_sel, rd_len);
    spi_set_transmode(spi_sel, SPI_MODE_WRITE_READ);
    spi_set_cmd(spi_sel, 0x00); //when  cmd  disable that  will not sent cmd,just trigger spi send .
    spi_write(spi_sel, (unsigned char *)wr_data, wr_len);
    spi_read(spi_sel, (unsigned char *)rd_data, rd_len);
    if (SPI_WAIT(spi_is_busy, spi_sel, g_spi_timeout_error[spi_sel].g_spi_error_timeout_us, g_spi_timeout_error[spi_sel].spi_timeout_handler, SPI_API_ERROR_TIMEOUT_BUS_BUSY)) {
        return DRV_API_TIMEOUT;
    }
    return DRV_API_SUCCESS;
}

/**
 * @brief       This function serves to single/dual/quad write to the SPI slave.
 * @param[in]   spi_sel     - the spi module.
 * @param[in]   cmd         - cmd one byte will first write.
 * @param[in]   addr        - the address of slave.
 * @param[in]   data        -  pointer to the data need to write.
 * @param[in]   data_len    - length in byte of the data need to write.
 * @param[in]   wr_mode     - write mode.dummy or not dummy.
 * @return      DRV_API_SUCCESS:master write data successfully. others:master write data failed.
 *              DRV_API_TIMEOUT:timeout exit(solution refer to the note for spi_set_error_timeout).
 */
drv_api_status_e spi_master_write_plus(spi_sel_e spi_sel, unsigned char cmd, unsigned int addr, unsigned char *data, unsigned int data_len, spi_wr_tans_mode_e wr_mode)
{
    spi_tx_dma_dis(spi_sel);
    spi_tx_fifo_clr(spi_sel);
    spi_set_address(spi_sel, addr);

    if (data_len == 0) {
        spi_set_transmode(spi_sel, SPI_MODE_NONE_DATA);
    } else {
        spi_set_transmode(spi_sel, (spi_tans_mode_e)wr_mode);
        spi_tx_cnt(spi_sel, data_len);
    }
    spi_set_cmd(spi_sel, cmd);

    if (data_len != 0) {
        spi_write(spi_sel, (unsigned char *)data, data_len);
    }
    if (SPI_WAIT(spi_is_busy, spi_sel, g_spi_timeout_error[spi_sel].g_spi_error_timeout_us, g_spi_timeout_error[spi_sel].spi_timeout_handler, SPI_API_ERROR_TIMEOUT_BUS_BUSY)) {
        return DRV_API_TIMEOUT;
    }
    return DRV_API_SUCCESS;
}

/**
 * @brief       This function serves to normal write data repeatedly.
 * @param[in]   spi_sel     - the spi module.
 * @param[in]   data        - the pointer to the data for write.
 * @param[in]   len         - write length.
 * @param[in]   repeat_time - number of times to write data repeatedly.
 * @return      DRV_API_SUCCESS:master write data successfully. others:master write data failed.
 *              DRV_API_TIMEOUT:timeout exit(solution refer to the note for spi_set_error_timeout).
 */
drv_api_status_e spi_master_write_repeat(spi_sel_e spi_sel, unsigned char *data, unsigned int len, unsigned int repeat_time)
{
    unsigned int i = 0, j = 0, k = 0;
    spi_tx_dma_dis(spi_sel);
    spi_tx_fifo_clr(spi_sel);
    spi_tx_cnt(spi_sel, len * repeat_time);
    spi_set_transmode(spi_sel, SPI_MODE_WRITE_ONLY);
    spi_set_cmd(spi_sel, 0x00); //when  cmd  disable that  will not sent cmd,just trigger spi send .
    for (i = 0; i < repeat_time; i++) {
        for (j = 0; j < len; j++, k++) {
            if (SPI_WAIT(spi_txfifo_is_full, spi_sel, g_spi_timeout_error[spi_sel].g_spi_error_timeout_us, g_spi_timeout_error[spi_sel].spi_timeout_handler, SPI_API_ERROR_TIMEOUT_TXFIFO_FULL)) {
                return DRV_API_TIMEOUT;
            }
            reg_spi_wr_rd_data(spi_sel, k % 4) = data[j];
        }
    }
    if (SPI_WAIT(spi_is_busy, spi_sel, g_spi_timeout_error[spi_sel].g_spi_error_timeout_us, g_spi_timeout_error[spi_sel].spi_timeout_handler, SPI_API_ERROR_TIMEOUT_BUS_BUSY)) {
        return DRV_API_TIMEOUT;
    }
    return DRV_API_SUCCESS;
}

/**
 * @brief       This function serves to single/dual/quad write data to the SPI slave repeatedly.
 * @param[in]   spi_sel     - the spi module.
 * @param[in]   cmd         - cmd one byte will first write.
 * @param[in]   addr        - the address of slave.
 * @param[in]   data        - pointer to the data need to write.
 * @param[in]   data_len    - length in byte of the data need to write.
 * @param[in]   wr_mode     - write mode.dummy or not dummy.
 * @param[in]   repeat_time - number of times to write data repeatedly.
 * @return      DRV_API_SUCCESS:master write data successfully. others:master write data failed.
 *              DRV_API_TIMEOUT:timeout exit(solution refer to the note for spi_set_error_timeout).
 * @attention   Only data would be written repeatedly. the typical sending order is cmd + address + data * repeat_time,
 *              cmd and address would not be repeated.
 */
drv_api_status_e spi_master_write_repeat_plus(spi_sel_e spi_sel, unsigned char cmd, unsigned int addr, unsigned char *data, unsigned int data_len, unsigned int repeat_time, spi_wr_tans_mode_e wr_mode)
{
    unsigned int i = 0, j = 0, k = 0;
    spi_tx_dma_dis(spi_sel);
    spi_tx_fifo_clr(spi_sel);
    spi_set_address(spi_sel, addr);
    spi_tx_cnt(spi_sel, data_len * repeat_time);
    spi_set_transmode(spi_sel, (spi_tans_mode_e)wr_mode);
    spi_set_cmd(spi_sel, cmd);
    for (i = 0; i < repeat_time; i++) {
        for (j = 0; j < data_len; j++, k++) {
            if (SPI_WAIT(spi_txfifo_is_full, spi_sel, g_spi_timeout_error[spi_sel].g_spi_error_timeout_us, g_spi_timeout_error[spi_sel].spi_timeout_handler, SPI_API_ERROR_TIMEOUT_TXFIFO_FULL)) {
                return DRV_API_TIMEOUT;
            }
            reg_spi_wr_rd_data(spi_sel, k % 4) = data[j];
        }
    }
    if (SPI_WAIT(spi_is_busy, spi_sel, g_spi_timeout_error[spi_sel].g_spi_error_timeout_us, g_spi_timeout_error[spi_sel].spi_timeout_handler, SPI_API_ERROR_TIMEOUT_BUS_BUSY)) {
        return DRV_API_TIMEOUT;
    }
    return DRV_API_SUCCESS;
}

/**
 * @brief       This function serves to single/dual/quad  read from the SPI slave.
 * @param[in]   spi_sel     - the spi module.
 * @param[in]   cmd         - cmd one byte will first write.
 * @param[in]   addr        - the address of slave.
 * @param[in]   data        - pointer to the data need to read.
 * @param[in]   data_len    - the length of data.
 * @param[in]   rd_mode     - read mode.dummy or not dummy.
 * @return      DRV_API_SUCCESS:master read data successfully. others:master read data failed.
 *              DRV_API_TIMEOUT:timeout exit(solution refer to the note for spi_set_error_timeout).
 */
drv_api_status_e spi_master_read_plus(spi_sel_e spi_sel, unsigned char cmd, unsigned int addr, unsigned char *data, unsigned int data_len, spi_rd_tans_mode_e rd_mode)
{
    spi_rx_dma_dis(spi_sel);
    spi_rx_fifo_clr(spi_sel);
    spi_set_address(spi_sel, addr);
    spi_set_transmode(spi_sel, (spi_tans_mode_e)rd_mode);
    spi_rx_cnt(spi_sel, data_len);
    spi_set_cmd(spi_sel, cmd);
    spi_read(spi_sel, (unsigned char *)data, data_len);
    if (SPI_WAIT(spi_is_busy, spi_sel, g_spi_timeout_error[spi_sel].g_spi_error_timeout_us, g_spi_timeout_error[spi_sel].spi_timeout_handler, SPI_API_ERROR_TIMEOUT_BUS_BUSY)) {
        return DRV_API_TIMEOUT;
    }
    return DRV_API_SUCCESS;
}

/**
 * @brief       This function serves to write address, then read data from the SPI slave.
 * This interface cannot be used for full duplex.
 * @param[in]   spi_sel     - the spi module.
 * @param[in]   cmd         - cmd one byte will first write.
 * @param[in]   addrs       - pointer to the address of slave.
 * @param[in]   addr_len    - the length of address.
 * @param[in]   data        - the pointer to the data for read.
 * @param[in]   data_len    - read length.
 * @param[in]   wr_mode     - write mode.dummy or not dummy.
 * @return      DRV_API_SUCCESS:master write read data successfully, others:master write read data failed.
 *              DRV_API_TIMEOUT:timeout exit(solution refer to the note for spi_set_error_timeout).
 */
drv_api_status_e spi_master_write_read_plus(spi_sel_e spi_sel, unsigned char cmd, unsigned char *addrs, unsigned int addr_len, unsigned char *data, unsigned int data_len, spi_rd_tans_mode_e wr_mode)
{
    spi_tx_dma_dis(spi_sel);
    spi_rx_dma_dis(spi_sel);
    spi_tx_fifo_clr(spi_sel);
    spi_rx_fifo_clr(spi_sel);
    spi_tx_cnt(spi_sel, addr_len);
    spi_rx_cnt(spi_sel, data_len);
    spi_set_transmode(spi_sel, (spi_tans_mode_e)wr_mode);
    spi_set_cmd(spi_sel, cmd);
    spi_write(spi_sel, (unsigned char *)addrs, addr_len);
    spi_read(spi_sel, (unsigned char *)data, data_len);
    if (SPI_WAIT(spi_is_busy, spi_sel, g_spi_timeout_error[spi_sel].g_spi_error_timeout_us, g_spi_timeout_error[spi_sel].spi_timeout_handler, SPI_API_ERROR_TIMEOUT_BUS_BUSY)) {
        return DRV_API_TIMEOUT;
    }
    return DRV_API_SUCCESS;
}

/**
 * @brief       This function serves to set rx_dma channel and config dma rx default for spi master.
 * @param[in]   spi_sel - the spi module.
 * @param[in]   chn     - dma channel.
 * @return      none
 */
void spi_set_master_rx_dma_config(spi_sel_e spi_sel, dma_chn_e chn)
{
    if (GSPI_MODULE == spi_sel) {
        s_gspi_master_rx_dma_chn = chn;
        dma_config(chn, &gspi_master_rx_dma_config);
    } else if (LSPI_MODULE == spi_sel) {
        s_lspi_master_rx_dma_chn = chn;
        dma_config(chn, &lspi_master_rx_dma_config);
    }
}

/**
 * @brief       This function serves to set rx_dma channel and config dma rx default for spi slave.
 * @param[in]   spi_sel - the spi module.
 * @param[in]   chn     - dma channel.
 * @return      none
 */
void spi_set_slave_rx_dma_config(spi_sel_e spi_sel, dma_chn_e chn)
{
    if (GSPI_MODULE == spi_sel) {
        s_gspi_slave_rx_dma_chn = chn;
        dma_config(chn, &gspi_slave_rx_dma_config);
    } else if (LSPI_MODULE == spi_sel) {
        s_lspi_slave_rx_dma_chn = chn;
        dma_config(chn, &lspi_slave_rx_dma_config);
    }
}

/**
 * @brief       This function serves to set tx_dma channel and config dma tx default.
 * @param[in]   spi_sel - the spi module.
 * @param[in]   chn     - dma channel.
 * @return      none
 */
void spi_set_tx_dma_config(spi_sel_e spi_sel, dma_chn_e chn)
{
    if (GSPI_MODULE == spi_sel) {
        s_gspi_tx_dma_chn = chn;
        dma_config(chn, &gspi_tx_dma_config);
    } else if (LSPI_MODULE == spi_sel) {
        s_lspi_tx_dma_chn = chn;
        dma_config(chn, &lspi_tx_dma_config);
    }
}

/**
 * @brief       this  function set spi dma channel.
 * @param[in]   spi_dma_chn - dma channel.
 * @param[in]   src_addr    - the address of source.
 * @param[in]   dst_addr    - the address of destination.
 * @param[in]   len         - the length of data.
 * */
void spi_set_dma(dma_chn_e spi_dma_chn, unsigned int src_addr, unsigned int dst_addr, unsigned int len)
{
    dma_set_address(spi_dma_chn, src_addr, dst_addr);
    dma_set_size(spi_dma_chn, len, DMA_WORD_WIDTH);
    dma_chn_en(spi_dma_chn);
}

/**
 * @brief       Send an amount of data in DMA mode
 * @param[in]   spi_sel     - to specify the spi module.
 * @param[in]   src_addr    - Pointer to data buffer, it must be 4-bytes aligned address
 * @param[in]   len         - Amount of data to be sent in bytes, range from 1 to 0xFFFFFC
 * @return      none
 * */
void spi_set_tx_dma(spi_sel_e spi_sel, unsigned char *src_addr, unsigned int len)
{
    unsigned char tx_dma_chn;
    if (GSPI_MODULE == spi_sel) {
        tx_dma_chn = s_gspi_tx_dma_chn;
    } else {
        tx_dma_chn = s_lspi_tx_dma_chn;
    }
    spi_tx_dma_en(spi_sel);
    dma_set_address(tx_dma_chn, (unsigned int)src_addr, reg_spi_data_buf_adr(spi_sel));
    dma_set_size(tx_dma_chn, len, DMA_WORD_WIDTH);
    dma_chn_en(tx_dma_chn);
}

/**
 * @brief      Receive an amount of data in DMA mode
 * @param[in]  spi_sel      - to specify the spi module.
 * @param[in]  dst_addr     - Pointer to data buffer, it must be 4-bytes aligned
 * @attention  The first four bytes in the buffer of the received data are the length of the received data.
 *             The actual buffer size that the user needs to set needs to be noted on two points:
 *             -# you need to leave 4bytes of space for the length information.
 *             -# dma is transmitted in accordance with 4bytes, so the length of the buffer needs to be a multiple of 4. Otherwise, there may be an out-of-bounds problem
 *             For example, the actual received data length is 5bytes, the minimum value of the actual buffer size that the user needs to set is 12bytes, and the calculation of 12bytes is explained as follows::
 *             4bytes (length information) + 5bytes (data) + 3bytes (the number of additional bytes to prevent out-of-bounds)
 * @return     none.
 */
void spi_set_rx_dma(spi_sel_e spi_sel, unsigned char *dst_addr)
{
    unsigned char rx_dma_chn;
    if (GSPI_MODULE == spi_sel) {
        rx_dma_chn = s_gspi_slave_rx_dma_chn;
    } else {
        rx_dma_chn = s_lspi_slave_rx_dma_chn;
    }
    spi_rx_dma_en(spi_sel);
    dma_set_address(rx_dma_chn, reg_spi_data_buf_adr(spi_sel), (unsigned int)(dst_addr));
    reg_dma_size(rx_dma_chn) = 0xffffffff;
    dma_chn_en(rx_dma_chn);
}

/**
 * @brief       This function serves to normal write data by dma.
 * @param[in]   spi_sel     - the spi module.
 * @param[in]   src_addr    - the pointer to the data for write.
 * @param[in]   len         - write length.
 * @return      none
 * @note        src_addr : must be aligned by word (4 bytes), otherwise the program will enter an exception.
 */
void spi_master_write_dma(spi_sel_e spi_sel, unsigned char *src_addr, unsigned int len)
{
    unsigned char tx_dma_chn;
    spi_tx_fifo_clr(spi_sel);
    spi_tx_dma_en(spi_sel);
    spi_tx_cnt(spi_sel, len);
    spi_set_transmode(spi_sel, SPI_MODE_WRITE_ONLY);
    if (GSPI_MODULE == spi_sel) {
        tx_dma_chn = s_gspi_tx_dma_chn;
    } else {
        tx_dma_chn = s_lspi_tx_dma_chn;
    }

    spi_set_dma(tx_dma_chn, (unsigned int)src_addr, reg_spi_data_buf_adr(spi_sel), len);
    spi_set_cmd(spi_sel, 0x00);
}

/**
 * @brief       This function serves to normal write cmd and address, then read data by dma.
 * This interface could be used for full duplex.
 * When this interface is used for full-duplex communication, it can only be used on the master side,
 * and the spi_master_config() interface must be called first to disable hardware cmd and hardware address, addr_len is equal to data_len.
 * @param[in]   spi_sel     - the spi module.
 * @param[in]   addr        - the pointer to the cmd and address for write.
 * @param[in]   addr_len    - write length.
 * @param[in]   data        - the pointer to the data for read.
 * @param[in]   data_len    - read length.
 * @return      none
 * @note        addr/data : must be aligned by word (4 bytes), otherwise the program will enter an exception.
 */
void spi_master_write_read_dma(spi_sel_e spi_sel, unsigned char *addr, unsigned int addr_len, unsigned char *data, unsigned int data_len)
{
    unsigned char tx_dma_chn, rx_dma_chn;
    if (GSPI_MODULE == spi_sel) {
        /**
         * Bugfix: Only fix the GSPI RX DMA logic bug, the GSPI must be reset once and the value of the digital register will not be cleared.
         *      changed by pengxiang.hong 20230329.
         */
        spi_hw_fsm_reset(GSPI_MODULE);

        tx_dma_chn = s_gspi_tx_dma_chn;
        rx_dma_chn = s_gspi_master_rx_dma_chn;
    } else {
        tx_dma_chn = s_lspi_tx_dma_chn;
        rx_dma_chn = s_lspi_master_rx_dma_chn;
    }
    spi_tx_fifo_clr(spi_sel);
    spi_rx_fifo_clr(spi_sel);
    spi_tx_dma_en(spi_sel);
    spi_rx_dma_en(spi_sel);
    spi_tx_cnt(spi_sel, addr_len);
    spi_rx_cnt(spi_sel, data_len);
    spi_set_transmode(spi_sel, SPI_MODE_WRITE_READ);

    spi_set_dma(tx_dma_chn, (unsigned int)(addr), reg_spi_data_buf_adr(spi_sel), addr_len);
    spi_set_dma(rx_dma_chn, reg_spi_data_buf_adr(spi_sel), (unsigned int)(data), data_len);
    spi_set_cmd(spi_sel, 0x00); //when  cmd  disable that  will not sent cmd,just trigger spi send .
}

/**
 * @brief       This function serves to single/dual/quad  write to the SPI slave by dma.
 * @param[in]   spi_sel     - the spi module.
 * @param[in]   cmd         - cmd one byte will first write.
 * @param[in]   addr        - the address of slave.
 * @param[in]   data        - pointer to the data need to write.
 * @param[in]   data_len    - length in byte of the data need to write.
 * @param[in]   wr_mode     - write mode.dummy or not dummy.
 * @return      none
 * @note        data : must be aligned by word (4 bytes), otherwise the program will enter an exception.
 */
void spi_master_write_dma_plus(spi_sel_e spi_sel, unsigned char cmd, unsigned int addr, unsigned char *data, unsigned int data_len, spi_wr_tans_mode_e wr_mode)
{
    unsigned char tx_dma_chn;
    spi_tx_fifo_clr(spi_sel);
    spi_tx_dma_en(spi_sel);
    spi_tx_cnt(spi_sel, data_len);
    spi_set_transmode(spi_sel, (spi_tans_mode_e)wr_mode);
    spi_set_address(spi_sel, addr);
    if (GSPI_MODULE == spi_sel) {
        tx_dma_chn = s_gspi_tx_dma_chn;
    } else {
        tx_dma_chn = s_lspi_tx_dma_chn;
    }

    spi_set_dma(tx_dma_chn, (unsigned int)data, reg_spi_data_buf_adr(spi_sel), data_len);
    spi_set_cmd(spi_sel, cmd);
}

/**
 * @brief       This function serves to single/dual/quad  read from the SPI slave by dma.
 * @param[in]   spi_sel     - the spi module.
 * @param[in]   cmd         - cmd one byte will first write.
 * @param[in]   addr        - the address of slave.
 * @param[in]   dst_addr    - pointer to the buffer that will cache the reading out data.
 * @param[in]   data_len    - length in byte of the data need to read.
 * @param[in]   rd_mode     - read mode.dummy or not dummy.
 * @return      none
 * @note        dst_addr : must be aligned by word (4 bytes), otherwise the program will enter an exception.
 */
void spi_master_read_dma_plus(spi_sel_e spi_sel, unsigned char cmd, unsigned int addr, unsigned char *dst_addr, unsigned int data_len, spi_rd_tans_mode_e rd_mode)
{
    unsigned char rx_dma_chn;
    if (GSPI_MODULE == spi_sel) {
        /**
         * Bugfix: Only fix the GSPI RX DMA logic bug, the GSPI must be reset once and the value of the digital register will not be cleared.
         *      changed by pengxiang.hong 20230329.
         */
        spi_hw_fsm_reset(GSPI_MODULE);
        rx_dma_chn = s_gspi_master_rx_dma_chn;
    } else {
        rx_dma_chn = s_lspi_master_rx_dma_chn;
    }
    spi_rx_fifo_clr(spi_sel);
    spi_rx_dma_en(spi_sel);
    spi_set_transmode(spi_sel, (spi_tans_mode_e)rd_mode);
    spi_rx_cnt(spi_sel, data_len);
    spi_set_address(spi_sel, addr);
    spi_set_dma(rx_dma_chn, reg_spi_data_buf_adr(spi_sel), (unsigned int)(dst_addr), data_len);
    spi_set_cmd(spi_sel, cmd);
}

/**
 * @brief       This function serves to single/dual/quad write address and read from the SPI slave by dma.
 * This interface could be used for full duplex.
 * When this interface is used for full-duplex communication, it can only be used on the master side.
 * 1.the spi_master_config() interface must be called first to disable hardware cmd and hardware address
 * 2.must cmd is 0,addr_len is equal to rd_len,rd_mode is SPI_MODE_WRITE_AND_READ.
 * @param[in]   spi_sel     - the spi module.
 * @param[in]   cmd         - cmd one byte will first write.
 * @param[in]   addr        - the pointer to the cmd and address for write.
 * @param[in]   addr_len    - the length of address.
 * @param[in]   rd_data     - pointer to the buffer that will cache the reading out data.
 * @param[in]   rd_len      - length in byte of the data need to read.
 * @param[in]   rd_mode     - read mode.dummy or not dummy.
 * @return      none
 * @note        addr/rd_data : must be aligned by word (4 bytes), otherwise the program will enter an exception.
 */
void spi_master_write_read_dma_plus(spi_sel_e spi_sel, unsigned char cmd, unsigned char *addr, unsigned int addr_len, unsigned char *rd_data, unsigned int rd_len, spi_rd_tans_mode_e rd_mode)
{
    unsigned char tx_dma_chn, rx_dma_chn;
    if (GSPI_MODULE == spi_sel) {
        /**
         * Bugfix: Only fix the GSPI RX DMA logic bug, the GSPI must be reset once and the value of the digital register will not be cleared.
         *      changed by pengxiang.hong 20230329.
         */
        spi_hw_fsm_reset(GSPI_MODULE);
        tx_dma_chn = s_gspi_tx_dma_chn;
        rx_dma_chn = s_gspi_master_rx_dma_chn;
    } else {
        tx_dma_chn = s_lspi_tx_dma_chn;
        rx_dma_chn = s_lspi_master_rx_dma_chn;
    }
    spi_tx_fifo_clr(spi_sel);
    spi_rx_fifo_clr(spi_sel);
    spi_tx_dma_en(spi_sel);
    spi_rx_dma_en(spi_sel);
    spi_tx_cnt(spi_sel, addr_len);
    spi_rx_cnt(spi_sel, rd_len);
    spi_set_transmode(spi_sel, (spi_tans_mode_e)rd_mode);
    spi_set_dma(tx_dma_chn, (unsigned int)(addr), reg_spi_data_buf_adr(spi_sel), addr_len);
    spi_set_dma(rx_dma_chn, reg_spi_data_buf_adr(spi_sel), (unsigned int)(rd_data), rd_len);
    spi_set_cmd(spi_sel, cmd); //when  cmd  disable that  will not sent cmd,just trigger spi send .
}

/**
 * @brief       This function serves to write and read data simultaneously.
 * This interface can only be used by the master.
 * When initializing the master, call the spi_master_config() interface to disable the hardware cmd and hardware address,
 * and then start sending and receiving data.
 * The default chunk size sent and received by this interface is 8byte.
 * @param[in]   spi_sel     - the spi module.
 * @param[in]   write_data  - write data pointer.
 * @param[in]   read_data   - read data pointer.
 * @param[in]   len         - write/read length.
 * @return      none
 */
void spi_master_write_read_full_duplex(spi_sel_e spi_sel, unsigned char *write_data, unsigned char *read_data, unsigned int len)
{
    unsigned int spi_wr_rd_size = 8;
    unsigned int chunk_size     = spi_wr_rd_size;
    spi_tx_dma_dis(spi_sel);
    spi_rx_dma_dis(spi_sel);
    spi_set_transmode(spi_sel, SPI_MODE_WRITE_AND_READ);
    spi_tx_fifo_clr(spi_sel);
    spi_rx_fifo_clr(spi_sel);
    spi_tx_cnt(spi_sel, len);
    spi_rx_cnt(spi_sel, len);
    spi_set_cmd(spi_sel, 0x00); //when  cmd  disable that  will not sent cmd,just trigger spi send .


    for (unsigned int i = 0; i < len; i = i + chunk_size) {
        if (chunk_size > (len - i)) {
            chunk_size = len - i;
        }
        spi_write(spi_sel, write_data + i, chunk_size);
        if (len < spi_wr_rd_size) {
            spi_read(spi_sel, read_data, chunk_size);
        } else if (i == 0) {
            spi_read(spi_sel, read_data, chunk_size - 1);
        } else if ((len - i) > spi_wr_rd_size) {
            spi_read(spi_sel, read_data + i - 1, chunk_size);
        } else {
            spi_read(spi_sel, read_data + i - 1, chunk_size + 1);
        }
    }
}

/**
 * @brief       This function serves to read data in normal.
 * @param[in]   spi_sel     - the spi module.
 * @param[in]   data        - read data pointer.
 * @param[in]   len         - read length.
 * @return      none
 */
void spi_master_read(spi_sel_e spi_sel, unsigned char *data, unsigned int len)
{
    spi_rx_dma_dis(spi_sel);
    spi_rx_fifo_clr(spi_sel);
    spi_rx_cnt(spi_sel, len);
    spi_set_transmode(spi_sel, SPI_MODE_READ_ONLY);
    spi_set_cmd(spi_sel, 0x00); //when  cmd  disable that  will not sent cmd,just trigger spi send .
    spi_read(spi_sel, (unsigned char *)data, len);
}
