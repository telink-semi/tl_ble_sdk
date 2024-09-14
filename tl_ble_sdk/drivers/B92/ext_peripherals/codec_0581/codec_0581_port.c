/********************************************************************************************************
 * @file    codec_0581_port.c
 *
 * @brief   This is the source file for B92
 *
 * @author  Driver Group
 * @date    2023
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
#include "codec_0581.h"

#define CODEC_DUMMY_DATA                (0x00)
#define CODEC_WRITE_CMD                 (0x00)
#define CODEC_READ_CMD                  (0x01)
#define CODEC_WRITE_HEAD                (0x04)
#define CODEC_READ_HEAD                 (0x04)

/**
 * @brief       This function servers to read codec_0581 register data.
 * @param[in]   reg_addr      external codec register address.
 * @param[out]  reg_data      read data from register.
 * @return      none.
 */
void codec_0581_reg_read(unsigned int reg_addr, unsigned int *reg_data)
{
    unsigned char wr_buf[4];
    unsigned char rd_buf[4];
    unsigned char reg_width = 0;

    codec_0581_get_addr_length(reg_addr, &reg_width);

    wr_buf[0] = CODEC_READ_CMD;
    wr_buf[1] = (reg_addr >> 8) & 0xff; /* starting address [15:08] */
    wr_buf[2] = (reg_addr)&0xff;        /* starting address [07:00] */
    wr_buf[3] = CODEC_DUMMY_DATA;

    spi_master_write_read(GSPI_MODULE, wr_buf, CODEC_READ_HEAD, rd_buf, reg_width);

    *reg_data = 0;
    for (unsigned char i = 0; i < reg_width; i += 1)
    {
        *reg_data += rd_buf[i] << (i * 8);
    }
}

/**
 * @brief       This function servers to write codec_0581 register data.
 * @param[in]   reg_addr      external codec register address.
 * @param[in]   reg_data      write data to register.
 * @return      none.
 */
void codec_0581_reg_write(unsigned int reg_addr, unsigned int reg_data)
{
    unsigned char wr_buf[8];
    unsigned char reg_width = 0;

    codec_0581_get_addr_length(reg_addr, &reg_width);

    /* write head */
    wr_buf[0] = CODEC_WRITE_CMD;
    wr_buf[1] = (reg_addr >> 8) & 0xff; /* starting address [15:08] */
    wr_buf[2] = (reg_addr)&0xff;        /* starting address [07:00] */
    wr_buf[3] = CODEC_DUMMY_DATA;

    for (char i = 0; i < reg_width; i += 1) /* payload placed after WR head */
    {
        wr_buf[CODEC_WRITE_HEAD + i] = (reg_data >> (i * 8)) & 0xFF;
    }

    spi_master_write(GSPI_MODULE, wr_buf, reg_width + CODEC_WRITE_HEAD);
}

/**
 * @brief       This function servers to init codec_0581. include spi port, pmu mode and pll etc.
 * @return      CODEC_0581_OK for success, codec_0581_error_e.
 */
codec_0581_error_e codec_0581_init(void)
{
    /* init spi */
    gspi_pin_config_t spi_pin_config = {
        .spi_csn_pin        = GPIO_FC_PE7,
        .spi_clk_pin        = GPIO_FC_PE6,
        .spi_mosi_io0_pin   = GPIO_FC_PF0,
        .spi_miso_io1_pin   = GPIO_FC_PF1,
        .spi_io2_pin        = 0,
        .spi_io3_pin        = 0,
    };

    spi_master_init(GSPI_MODULE, sys_clk.pll_clk * 1000000 / 12000000, SPI_MODE0);
    gspi_set_pin(&spi_pin_config);
    spi_master_config(GSPI_MODULE, SPI_NORMAL);

    /**** set pin high to power on codec_0581 ****/
    codec_0581_power_on(GPIO_PC1, CODEC_BOOL_TRUE);

    /* pull the SS pin low three times, in the meanwhile, toggling the SCLK pin three times, codec can enter SPI mode so read id three times can trigger spi mode */
    unsigned char id1;
    unsigned short id2;
    unsigned char id3;
    for (int i = 0; i < 3; i++)
    {
        codec_0581_get_id(&id1, &id2, &id3);
        if (id1 != 0x41) /* if check id err then delay 5ms for next trigger */
            delay_ms(5);
    }

    /* real get, if failed means spi communication failed */
    codec_0581_get_id(&id1, &id2, &id3);
    if (id1 != 0x41)
    {
        /* read id fail means communication failed */
        return CODEC_0581_ERROR;
    }

    /* set pmu power mode */
    codec_0581_pmu_set_power_mode(CODEC_PWR_MODE_ACTIVE);
    /* set CM_STARTUP_OVER = 1 to keep lower power consumption */
    codec_0581_pmu_enable_cm_pin_fast_charge(CODEC_BOOL_FALSE);
    /* all blocks that have their respective block enable */
    codec_0581_pmu_enable_master_block(CODEC_BOOL_TRUE);
    /* pll power on */
    codec_0581_clk_enable_pll_power_on(CODEC_BOOL_TRUE);
    /* to access FastDSP and EQ memories, the PLL must be locked */
    unsigned char locked = 0;
    unsigned int cnt = 0;
    while ((locked == 0) && (cnt <= 100))
    {
        codec_0581_clk_get_pll_locked_status(&locked);
        delay_ms(1);
        cnt++;
    }

    /* pll is not locked */
    if (cnt > 100)
    {
        return CODEC_0581_PLL_NOT_LOCKED;
    }

    /* pll enable, clk in 12MHz,  mclk = 12MHz / 3 * (6 + 18 / 125) = 24.576MHz */
    codec_0581_clk_config_pll(CODEC_CLK_PLL_SOURCE_MCLKIN, CODEC_CLK_PLL_TYPE_FRAC, CODEC_CLK_SYNC_SOURCE_INTERNAL, 3, 6, 18, 125);

    return CODEC_0581_OK;
}

/**
 * @brief       This function servers to init codec_0581 i2s.
 * @param[in]   i2s_init                codec_0581 i2s init, codec_0581_i2s_init_t.
 * @param[in]   i2s_input               codec_0581 i2s input config, codec_0581_i2s_input_t.
 * @param[in]   i2s_output              codec_0581 i2s output config, codec_0581_i2s_output_t.
 * @return      none
 */
void codec_0581_i2s_init(codec_0581_i2s_init_t *i2s_init, codec_0581_i2s_input_t *i2s_input, codec_0581_i2s_output_t *i2s_output)
{
    /* audio power on */
    audio_power_on();
    /**** set i2s as master mclk = pll_clk(default 192M) * (1 / 16) = 12M ****/
    audio_i2s_set_mclk(GPIO_FC_PC3, 1, sys_clk.pll_clk / 12);

    /* I2S pin */
    i2s_pin_config_t i2s_pin_config = {
        .bclk_pin       = GPIO_FC_PB3,
        .adc_lr_clk_pin = GPIO_FC_PB1,
        .dac_lr_clk_pin = GPIO_FC_PB1,
        .adc_dat_pin    = GPIO_FC_PB2,
        .dac_dat_pin    = GPIO_FC_PB0,
    };

    audio_i2s_config_t audio_i2s_config = {
        .i2s_select = I2S1,
        .i2s_mode = I2S_I2S_MODE,
        .pin_config = &i2s_pin_config,
        .data_width = i2s_init->data_width,
        .master_slave_mode = I2S_AS_MASTER_EN,
        .sample_rate = i2s_init->sample_rate,
    };

    /**** mcu i2s config init ****/
    audio_i2s_config_init(&audio_i2s_config);
    /**** mcu i2s input init ****/
    if (i2s_input)
    {
        audio_i2s_input_output_t audio_i2s_input = {
            .i2s_select             = audio_i2s_config.i2s_select,
            .data_width             = audio_i2s_config.data_width,
            .i2s_ch_sel             = i2s_input->i2s_ch_sel,
            .fifo_chn               = FIFO1,
            .dma_num                = i2s_input->rx_dma_num,
            .data_buf               = i2s_input->input_data_buf,
            .data_buf_size          = i2s_input->input_buf_size,
        };
        /* init input channel and dma */
        audio_i2s_input_init(&audio_i2s_input);
        audio_rx_dma_chain_init(audio_i2s_input.fifo_chn, audio_i2s_input.dma_num, (unsigned short *)audio_i2s_input.data_buf, audio_i2s_input.data_buf_size);
    }

    /**** mcu i2s output init ****/
    if (i2s_output)
    {
        audio_i2s_input_output_t audio_i2s_output = {
            .i2s_select             = audio_i2s_config.i2s_select,
            .data_width             = audio_i2s_config.data_width,
            .i2s_ch_sel             = I2S_MONO,
            .fifo_chn               = FIFO1,
            .dma_num                = i2s_output->tx_dma_num,
            .data_buf               = i2s_output->output_data_buf,
            .data_buf_size          = i2s_output->output_buf_size,
        };
        /* init output channel and dma */
        audio_i2s_output_init(&audio_i2s_output);
        audio_tx_dma_chain_init(audio_i2s_output.fifo_chn, audio_i2s_output.dma_num, (unsigned short *)audio_i2s_output.data_buf, audio_i2s_output.data_buf_size);
    }
}
