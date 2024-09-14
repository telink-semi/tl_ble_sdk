/********************************************************************************************************
 * @file    audio.c
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
#include "audio.h"
#include "clock.h"
#include "pwm.h"
#include "stimer.h"
#include "lib/include/pm.h"


unsigned char audio_codec_rate[AUDIO_RATE_SIZE] = { 0x06,/*8k*/                 //12Mhz/1500=8K
                                                    0x17,/*8.0214k*/            //12Mhz/1496=8.0214K
                                                    0x19,/*11.0259k*/           //12Mhz/1088=11.0259K
                                                    0x08,/*12k*/                //12Mhz/1000=12K
                                                    0x0a,/*16k*/                //12Mhz/750=16K
                                                    0x1b,/*22.0588k*/           //12Mhz/544=22.0588K
                                                    0x1c,/*24k*/                //12Mhz/500=24K
                                                    0x0c,/*32k*/                //12Mhz/375=32K
                                                    0x11,/*44.118k*/            //12Mhz/272=44.118K
                                                    0x00/*48k*/};               //12Mhz/250=48K

unsigned char audio_rx_dma_chn;
unsigned char audio_tx_dma_chn;
unsigned char audio_rx_fifo_chn;
unsigned char audio_tx_fifo_chn;

/**
 * @brief Audio power flag.
 *      - 0 indicates audio power down.
 *      - 1 indicates audio power on.
 */
static unsigned char audio_powered_flag = 0;

dma_chain_config_t g_audio_tx_dma_list_cfg[2];
dma_chain_config_t g_audio_rx_dma_list_cfg[2];

audio_i2s_invert_config_t audio_i2s_invert_config[2]=
{
   {
    .i2s_lr_clk_invert_select=I2S_LR_CLK_INVERT_DIS,
    .i2s_data_invert_select=DATA_INVERT_DIS,
   },
   {
    .i2s_lr_clk_invert_select=I2S_LR_CLK_INVERT_DIS,
    .i2s_data_invert_select=DATA_INVERT_DIS,
   },
};
dma_config_t audio_dma_rx_config[2]=
{
    {
        .dst_req_sel= 0,
        .src_req_sel=DMA_REQ_AUDIO0_RX,//l_rx req
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
        .src_req_sel=DMA_REQ_AUDIO1_RX,// r_rx req
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


dma_config_t audio_dma_tx_config[2]=
{
    {
        .dst_req_sel= DMA_REQ_AUDIO0_TX,//tx req
        .src_req_sel=0,
        .dst_addr_ctrl=DMA_ADDR_FIX,
        .src_addr_ctrl=DMA_ADDR_INCREMENT,//increment
        .dstmode=DMA_HANDSHAKE_MODE,//handshake
        .srcmode=DMA_NORMAL_MODE,
        .dstwidth=DMA_CTR_WORD_WIDTH,//must word
        .srcwidth=DMA_CTR_WORD_WIDTH,//must word
        .src_burst_size=0,//must 0
        .read_num_en=0,
        .priority=0,
        .write_num_en=0,
        .auto_en=0,//must 0
    },
    {
        .dst_req_sel= DMA_REQ_AUDIO1_TX,//tx req
        .src_req_sel=0,
        .dst_addr_ctrl=DMA_ADDR_FIX,
        .src_addr_ctrl=DMA_ADDR_INCREMENT,//increment
        .dstmode=DMA_HANDSHAKE_MODE,//handshake
        .srcmode=DMA_NORMAL_MODE,
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
 * @brief     This function configures stream0 dmic pin.
 * @param[in] dmic0_data - the data of  dmic pin
 * @param[in] dmic0_clk1 - the clk1 of dmic pin
 * @param[in] dmic0_clk2 - the clk2 of dmic pin,if need not set clk2,please set GPIO_NONE_PIN
 * @return    none
 */
void audio_set_stream0_dmic_pin(gpio_func_pin_e dmic0_data,gpio_func_pin_e dmic0_clk1,gpio_func_pin_e dmic0_clk2)
{
    gpio_input_en((gpio_pin_e)dmic0_data);
    gpio_set_mux_function(dmic0_data,DMIC0_DAT_I);
    gpio_function_dis((gpio_pin_e)dmic0_data);
    gpio_set_mux_function(dmic0_clk1,DMIC0_CLK);
    gpio_function_dis((gpio_pin_e)dmic0_clk1);
    if( dmic0_clk2 != GPIO_NONE_PIN)
    {
        gpio_set_mux_function(dmic0_clk2,DMIC0_CLK);
        gpio_function_dis((gpio_pin_e)dmic0_clk2);
    }

}

/**
 * @brief     This function configures stream1 dmic pin.
 * @param[in] dmic1_data - the data of dmic  pin
 * @param[in] dmic1_clk1 - the clk1 of dmic pin
 * @param[in] dmic1_clk2 - the clk2 of dmic pin,if need not set clk2,please set GPIO_NONE_PIN
 * @return    none
 */
void audio_set_stream1_dmic_pin(gpio_func_pin_e dmic1_data,gpio_func_pin_e dmic1_clk1,gpio_func_pin_e dmic1_clk2)
{
    gpio_input_en((gpio_pin_e)dmic1_data);
    gpio_set_mux_function(dmic1_data,DMIC1_DAT_I);
    gpio_function_dis((gpio_pin_e)dmic1_data);
    gpio_set_mux_function(dmic1_clk1,DMIC1_CLK);
    gpio_function_dis((gpio_pin_e)dmic1_clk1);
    if( dmic1_clk2 != GPIO_NONE_PIN)
    {
        gpio_set_mux_function(dmic1_clk2,DMIC1_CLK);
        gpio_function_dis((gpio_pin_e)dmic1_clk2);
    }
}

/**
 * @brief     This function serves to enable rx_dma channel.
 * @param[in] chn   - dma channel
 * @return    none
 */
 void audio_rx_dma_en(dma_chn_e chn)
{
     dma_chn_en(chn);
}

 /**
  * @brief     This function serves to disable rx_dma channel.
  * @param[in] chn   - dma channel
  * @return    none
  */
  void audio_rx_dma_dis(dma_chn_e chn)
 {
     dma_chn_dis(chn);
 }

 /**
  * @brief     This function serves to enable tx_dma channel.
  * @param[in] chn   - dma channel
  * @return    none
  */
 void audio_tx_dma_en(dma_chn_e chn)
{
     dma_chn_en(chn);
}

 /**
  * @brief     This function serves to disable dis_dma channel.
  * @param[in] chn   - dma channel
  * @return    none
  */
 void audio_tx_dma_dis(dma_chn_e chn)
{
     dma_chn_dis(chn);
}

/**
 * @brief     This function serves to config  rx_dma channel.
 * @param[in] chn          - dma channel
 * @param[in] dst_addr     - Pointer to data buffer, it must be 4-bytes aligned address.
 *                           and the actual buffer size defined by the user needs to be not smaller than the data_len, otherwise there may be an out-of-bounds problem.
 * @param[in] data_len     - Length of DMA in bytes, it must be set to a multiple of 4. The maximum value that can be set is 0x10000. 
 * @param[in] head_of_list - the head address of dma llp.
 * @return    none
 */
void audio_rx_dma_config(dma_chn_e chn,unsigned short *dst_addr,unsigned int data_len,dma_chain_config_t *head_of_list)
{
    audio_rx_dma_chn=chn;
    audio_set_rx_buff_len(audio_rx_fifo_chn,data_len);
    dma_config(chn,&audio_dma_rx_config[audio_rx_fifo_chn]);
    dma_set_address( chn,REG_AUDIO_FIFO_ADDR(audio_rx_fifo_chn),(unsigned int)(dst_addr));
    dma_set_size(chn,data_len,DMA_WORD_WIDTH);
    reg_dma_llp(chn)=(unsigned int)convert_ram_addr_cpu2bus(head_of_list);
}

/**
 * @brief     This function serves to set rx dma chain transfer
 * @param[in] config_addr - the head of list of llp_pointer.
 * @param[in] llpointer   - the next element of llp_pointer.
 * @param[in] dst_addr    - Pointer to data buffer, it must be 4-bytes aligned address and the actual buffer size defined by the user needs to
 *                          be not smaller than the data_len, otherwise there may be an out-of-bounds problem.
 * @param[in] data_len    - Length of DMA in bytes, it must be set to a multiple of 4. The maximum value that can be set is 0x10000.
 * @return    none
 */
void audio_rx_dma_add_list_element(dma_chain_config_t *config_addr,dma_chain_config_t *llpointer ,unsigned short * dst_addr,unsigned int data_len)
{
    config_addr->dma_chain_ctl=reg_dma_ctrl(audio_rx_dma_chn)|BIT(0);
    config_addr->dma_chain_src_addr=REG_AUDIO_FIFO_ADDR(audio_rx_fifo_chn);
    config_addr->dma_chain_dst_addr=(unsigned int)convert_ram_addr_cpu2bus(dst_addr);
    config_addr->dma_chain_data_len=dma_cal_size(data_len,4);
    config_addr->dma_chain_llp_ptr=(unsigned int)convert_ram_addr_cpu2bus(llpointer);
}

/**
 * @brief     This function serves to set audio rx dma chain transfer.
 * @param[in] rx_fifo_chn - rx fifo select.
 * @param[in] chn         - dma channel
 * @param[in] in_buff     - Pointer to data buffer, it must be 4-bytes aligned address and the actual buffer size defined by the user needs to
 *                          be not smaller than the data_len, otherwise there may be an out-of-bounds problem.
 * @param[in] buff_size   - Length of DMA in bytes, it must be set to a multiple of 4. The maximum value that can be set is 0x10000.
 * @return    none
 */
void audio_rx_dma_chain_init (audio_fifo_chn_e rx_fifo_chn,dma_chn_e chn,unsigned short * in_buff,unsigned int buff_size)
{
    audio_rx_fifo_chn=rx_fifo_chn;
    audio_rx_dma_config(chn,(unsigned short*)in_buff, buff_size, &g_audio_rx_dma_list_cfg[rx_fifo_chn]);
    audio_rx_dma_add_list_element(&g_audio_rx_dma_list_cfg[rx_fifo_chn], &g_audio_rx_dma_list_cfg[rx_fifo_chn],(unsigned short*)in_buff, buff_size);
}

/**
 * @brief     This function serves to config  tx_dma channel.
 * @param[in] chn          - dma channel
 * @param[in] src_addr     - Pointer to data buffer, it must be 4-bytes aligned address.
 * @param[in] data_len     - Length of DMA in bytes, range from 1 to 0x10000.
 * @param[in] head_of_list - the head address of dma llp.
 * @return    none
 */
void audio_tx_dma_config(dma_chn_e chn,unsigned short * src_addr, unsigned int data_len,dma_chain_config_t * head_of_list)
{
     audio_tx_dma_chn=chn;
     audio_set_tx_buff_len(audio_tx_fifo_chn,data_len);
     dma_config(chn,&audio_dma_tx_config[audio_tx_fifo_chn]);
     dma_set_address(chn,(unsigned int)(src_addr),REG_AUDIO_FIFO_ADDR(audio_tx_fifo_chn));
     dma_set_size(chn,data_len,DMA_WORD_WIDTH);
     reg_dma_llp(chn)=(unsigned int)convert_ram_addr_cpu2bus(head_of_list);
}

/**
 * @brief     This function serves to set tx dma chain transfer
 * @param[in] config_addr - the head of list of llp_pointer.
 * @param[in] llpointer   - the next element of llp_pointer.
 * @param[in] src_addr    - Pointer to data buffer, it must be 4-bytes aligned address.
 * @param[in] data_len    - Length of DMA in bytes, range from 1 to 0x10000.
 * @return    none
 */
void audio_tx_dma_add_list_element(dma_chain_config_t *config_addr,dma_chain_config_t *llpointer ,unsigned short * src_addr,unsigned int data_len)
{
    config_addr->dma_chain_ctl=reg_dma_ctrl(audio_tx_dma_chn)|BIT(0);
    config_addr->dma_chain_src_addr=(unsigned int)convert_ram_addr_cpu2bus(src_addr);
    config_addr->dma_chain_dst_addr=REG_AUDIO_FIFO_ADDR(audio_tx_fifo_chn);
    config_addr->dma_chain_data_len=dma_cal_size(data_len,4);
    config_addr->dma_chain_llp_ptr=(unsigned int)convert_ram_addr_cpu2bus(llpointer);
}

/**
 * @brief     This function serves to initialize audio tx dma chain transfer.
 * @param[in] tx_fifo_chn - tx fifo select.
 * @param[in] chn         - dma channel
 * @param[in] out_buff    - Pointer to data buffer, it must be 4-bytes aligned address.
 * @param[in] buff_size   - Length of DMA in bytes, range from 1 to 0x10000.
 * @return    none
 */
void audio_tx_dma_chain_init (audio_fifo_chn_e tx_fifo_chn,dma_chn_e chn,unsigned short * out_buff,unsigned int buff_size)
{
    audio_tx_fifo_chn=tx_fifo_chn;
    audio_tx_dma_config(chn, (unsigned short*)out_buff, buff_size, &g_audio_tx_dma_list_cfg[tx_fifo_chn]);
    audio_tx_dma_add_list_element(&g_audio_tx_dma_list_cfg[tx_fifo_chn], &g_audio_tx_dma_list_cfg[tx_fifo_chn], (unsigned short*)out_buff, buff_size);
}

/**
 *  @brief      This function serves to set stream0 sample_rate.
 *  @param[in]  rate - sample_rate
 *  @return     none
 */
void audio_set_codec_stream0_sample_rate(audio_sample_rate_e rate)
{
    reg_codec_clk_ctr1= (reg_codec_clk_ctr1&(~FLD_AUDIO_CODEC_DEC0_CLK_SR))|(audio_codec_rate[rate]<<1);
}

/**
 *  @brief      This function serves to set stream1 sample_rate.
 *  @param[in]  rate- sample_rate
 *  @return     none
 */
void audio_set_codec_stream1_sample_rate(audio_sample_rate_e rate)
{
    reg_codec_dec1_ctr1 = audio_codec_rate[rate];
}

/**
 *  @brief      This function serves to set dac sample_rate.
 *  @param[in]  rate - sample_rate
 *  @return     none
 */
void audio_set_codec_dac_sample_rate(audio_sample_rate_e rate)
{
    reg_codec_clk_ctr2 &= (~FLD_AUDIO_CODEC_INT_EN);
    reg_codec_clk_ctr2 = (reg_codec_clk_ctr2&(~FLD_AUDIO_CODEC_INT_CLK_SR))|FLD_AUDIO_CODEC_INT_EN |((audio_codec_rate[rate])<<3);
}

/**
 *  @brief      This function serves to set stream0 input mode.
 *  @param[in]  input_mode - 0:line_in or amic 1:dmic
 *  @return     none
 */
void audio_set_codec_stream0_input_mode(audio_stream0_input_mode_e input_mode)
{
/**
  * Enabling FLD_AUDIO_CODEC_R_NEG bit is valid for both stream0 and stream1.
  * If this bit is 0 and the DMIC sampling rate is 44.1k and 48k,the left and right channel data will be exchanged.
**/
   reg_codec_mic_ctr =(reg_codec_mic_ctr&(~FLD_AUDIO_CODEC_MIC_SEL))|FLD_AUDIO_CODEC_R_NEG|(input_mode<<3);
}

/**
 *  @brief      This function serves to  swap stream0 data .
 *  @param[in]  en - DATA_INVERT_DIS/DATA_INVERT_EN
 *  @return     none
 */
 void audio_swap_stream0_data(audio_data_invert_e en)
{
     if(en)
     {
         reg_codec_dsm_ctr1 =reg_codec_dsm_ctr1|FLD_AUDIO_CODEC_DEC0_SWAP;
     }
     else
     {
         reg_codec_dsm_ctr1=reg_codec_dsm_ctr1&(~FLD_AUDIO_CODEC_DEC0_SWAP);
     }
}

 /**
  *     @brief      This function serves to set stream0 input path.
  *     @param[in]  che_en  - channel selection
  *     @param[in]  rate    - select channel sample rate
  *     @return     none
  */
 void audio_set_codec_stream0_path(codec_stream0_input_src_e che_en,audio_sample_rate_e rate)

 {
    /* at 44.1k, 48k sampling rate, the DMCI channel will be switched (amic, line_in does not affect),
     * and the switch here restores the original channel correspondence
     */

    if(rate == AUDIO_44P1K ||rate == AUDIO_48K)
    {

        if(che_en == DMIC_STREAM0_MONO_L)
         {
            che_en = DMIC_STREAM0_MONO_R;
         }
        else if(che_en == DMIC_STREAM0_MONO_R)
         {
            che_en = DMIC_STREAM0_MONO_L;
         }
    }
    reg_codec_mic_ctr =(reg_codec_mic_ctr&(~(FLD_AUDIO_CODEC_DEC0_CH0_EN|FLD_AUDIO_CODEC_DEC0_CH1_EN)))\
                                          | MASK_VAL( FLD_AUDIO_CODEC_DEC0_CH0_EN, (che_en&CHANNEL_LEFT)==CHANNEL_LEFT ? 1:0,\
                                                    FLD_AUDIO_CODEC_DEC0_CH1_EN, (che_en&CHANNEL_RIGHT)==CHANNEL_RIGHT ? 1:0);
    reg_codec_clk_ctr2 |=FLD_AUDIO_CODEC_DEC0_EN;
 }

/**
 *  @brief      This function serves to  swap stream1 data.
 *  @param[in]  en - DATA_INVERT_DIS/DATA_INVERT_EN
 *  @return     none
 */
 void audio_swap_stream1_data(audio_data_invert_e en)
{
     if(en)
     {
         reg_codec_dec1_ctr0 =reg_codec_dec1_ctr0|FLD_AUDIO_CODEC_DEC1_SWAP;
     }
     else
     {
         reg_codec_dec1_ctr0=reg_codec_dec1_ctr0&(~FLD_AUDIO_CODEC_DEC1_SWAP);
     }
}

 /**
   *    @brief      This function serves to set stream1 input path.
   *    @param[in]  che_en  - channel selection
   *    @param[in]  rate    - select channel sample rate
   *    @return     none
   */
 void audio_set_codec_stream1_path(codec_stream1_input_src_e che_en,audio_sample_rate_e rate)
 {
     /* at 44.1k, 48k sampling rate, the DMCI channel will be switched (amic, line_in does not affect),
     * and the switch here restores the original channel correspondence
     */

    if(rate == AUDIO_44P1K ||rate == AUDIO_48K)
    {

        if(che_en == DMIC_STREAM1_MONO_L)
         {
            che_en = DMIC_STREAM1_MONO_R;
         }
        else if(che_en == DMIC_STREAM1_MONO_R)
         {
            che_en = DMIC_STREAM1_MONO_L;
         }
    }
    reg_codec_dec1_ctr0 =(reg_codec_dec1_ctr0&(~(FLD_AUDIO_CODEC_DEC1_CH0_EN|FLD_AUDIO_CODEC_DEC1_CH1_EN)))\
                                                      |MASK_VAL(FLD_AUDIO_CODEC_DEC1_EN,1,FLD_AUDIO_CODEC_CLK1_EN,1,\
                                                      FLD_AUDIO_CODEC_DEC1_CH0_EN, (che_en&CHANNEL_LEFT)==CHANNEL_LEFT ? 1:0,\
                                                      FLD_AUDIO_CODEC_DEC1_CH1_EN, (che_en&CHANNEL_RIGHT)==CHANNEL_RIGHT ? 1:0);
 }

/**
 * @brief      This function serves to set codec stream0 input data bit width mode for fifo0 or fifo1 .
 * @param[in]  fifo_chn - fifo0 or fifo1
 * @param[in]  ain_mode - fifo input data bit width selection
 * @return    none
 */
void audio_set_stream0_fifo_input_mode(audio_fifo_chn_e fifo_chn,audio_codec_in_mode_e ain_mode)
{
    if ((ain_mode == CODEC_BIT_16_STEREO) || (ain_mode == CODEC_BIT_20_STEREO))
    {
         reg_audio_dec0_sel = (reg_audio_dec0_sel & (~(FLD_AUDIO_DEC0_AINL_COME | FLD_AUDIO_DEC0_AINR_COME))) |
                                   MASK_VAL(FLD_AUDIO_DEC0_AINL_COME, ain_mode, FLD_AUDIO_DEC0_AINR_COME, ain_mode);
    }
    else
    {
        reg_audio_dec0_sel = (fifo_chn== FIFO0)?((reg_audio_dec0_sel&(~FLD_AUDIO_DEC0_AINL_COME))|(ain_mode)):((reg_audio_dec0_sel&(~FLD_AUDIO_DEC0_AINR_COME))|(ain_mode<<2));
    }
}

/**
 * @brief      This function serves to set codec stream0 input data bit width for fifo0 or fifo1 .
 * @param[in]  fifo_chn   - fifo0 or fifo1
 * @param[in]  source     - audio input source select.
 * @param[in]  data_width - input data bit width selection
 * @return    none
 */
void audio_codec_set_stream0_fifo_input_mode(audio_fifo_chn_e fifo_chn,codec_stream0_input_src_e source, audio_codec_wl_mode_e data_width)
{
    unsigned char  ain_mode=0;
    if( (source&CHANNEL_STEREO)== CHANNEL_STEREO)
    {
        ain_mode= (data_width == CODEC_BIT_16_DATA) ? CODEC_BIT_16_STEREO :CODEC_BIT_20_STEREO;
    }
    else
    {
        ain_mode= (data_width == CODEC_BIT_16_DATA) ? CODEC_BIT_16_MONO :CODEC_BIT_20_MONO;
    }
    audio_set_stream0_fifo_input_mode(fifo_chn,ain_mode);
}

/**
 * @brief      This function serves to set codec stream1 input data bit width for fifo0 or fifo1 .
 * @param[in]  fifo_chn - fifo0 or fifo1
 * @param[in]  ain_mode - fifo input data bit width selection
 * @return    none
 */
void audio_set_stream1_fifo_input_mode(audio_fifo_chn_e fifo_chn,audio_codec_in_mode_e ain_mode)
{
    if ((ain_mode == CODEC_BIT_16_STEREO) || (ain_mode == CODEC_BIT_20_STEREO) || (ain_mode == CODEC_BIT_16_STEREO_STREAM0_STREAM1))
    {
         reg_audio_dec1_sel = (reg_audio_dec1_sel & (~(FLD_AUDIO_DEC1_AINL_COME | FLD_AUDIO_DEC1_AINR_COME))) |
                                   MASK_VAL(FLD_AUDIO_DEC1_AINL_COME, ain_mode, FLD_AUDIO_DEC1_AINR_COME, ain_mode);
    }
    else
    {
        reg_audio_dec1_sel = (fifo_chn== FIFO0)?((reg_audio_dec1_sel&(~FLD_AUDIO_DEC1_AINL_COME))|(ain_mode)):(reg_audio_dec1_sel&(~FLD_AUDIO_DEC1_AINR_COME))|(ain_mode<<4);
    }
}

/**
 * @brief      This function serves to set codec stream1 input data bit width for fifo0 or fifo1 .
 * @param[in]  fifo_chn   - fifo0 or fifo1
 * @param[in]  source     - audio input source select.
 * @param[in]  data_width - input data bit width selection
 * @return    none
 */
void audio_codec_set_stream1_fifo_input_mode(audio_fifo_chn_e fifo_chn,codec_stream1_input_src_e source, audio_codec_wl_mode_e data_width)
{
    unsigned char  ain_mode=0;
    if(source == DMIC_STREAM0_STREAM1_STEREO)
    {
        ain_mode = CODEC_BIT_16_STEREO_STREAM0_STREAM1;
    }
    else
    {
      if((source&CHANNEL_STEREO)== CHANNEL_STEREO)
        {
            ain_mode= (data_width == CODEC_BIT_16_DATA) ? CODEC_BIT_16_STEREO :CODEC_BIT_20_STEREO;
        }
        else
        {
            ain_mode= (data_width == CODEC_BIT_16_DATA) ? CODEC_BIT_16_MONO :CODEC_BIT_20_MONO;
        }
    }
    audio_set_stream1_fifo_input_mode(fifo_chn,ain_mode);
}

/**
 *  @brief      This function serves to  swap data .
 *  @param[in]  en - DATA_INVERT_DIS/DATA_INVERT_EN
 *  @return     none
 */
 void audio_swap_dac_data(audio_data_invert_e en)
{
     if(en)
     {
         reg_codec_dsm_ctr1 =reg_codec_dsm_ctr1|FLD_AUDIO_CODEC_INT_SWAP;
     }
     else
     {
         reg_codec_dsm_ctr1=reg_codec_dsm_ctr1&(~FLD_AUDIO_CODEC_INT_SWAP);
     }
}

/**
 * @brief     This function serves to set codec dac data output data bit width and fifo path.
 * @param[in] int_aout_mode - fifo output source select
 * @return    none
 */
void audio_set_dac_fifo_output_mode(audio_out_mode_e int_aout_mode)
{
    reg_audio_dec0_sel = (reg_audio_dec0_sel&(~FLD_AUDIO_INT_AOUT_COME))|(int_aout_mode<<4);
}

/**
 * @brief      This function serves to set codec dac data bit width
 * @param[in]  fifo_chn     - fifo select
 * @param[in]  source       - audio input source select.
 * @param[in]  wdata_width  - output data bit width selection
 * @return    none
 */
void audio_set_dac_fifo_output_mode_config(audio_fifo_chn_e fifo_chn,audio_dac_output_src_e  source,audio_codec_wl_mode_e data_width)
{
    unsigned char int_aout_mode=0;
    if( (source&CODEC_DAC_STEREO)== CODEC_DAC_STEREO)
    {
        int_aout_mode=(fifo_chn == FIFO0) ? ((data_width==CODEC_BIT_16_DATA) ? BIT_16_STEREO_FIFO0 :BIT_20_OR_24_STEREO_FIFO0) : ((data_width == CODEC_BIT_16_DATA) ? BIT_16_STEREO_FIFO1 :BIT_20_OR_24_STEREO_FIFO1);
    }
    else
    {
        int_aout_mode=(fifo_chn == FIFO0) ? ((data_width==CODEC_BIT_16_DATA) ? BIT_16_MONO_FIFO0 :BIT_20_OR_24_MONO_FIFO0) : ((data_width == CODEC_BIT_16_DATA) ? BIT_16_MONO_FIFO1 :BIT_20_OR_24_MONO_FIFO1);
    }
    audio_set_dac_fifo_output_mode(int_aout_mode);
}

/**
 * @brief     This function serves to set i2s data input data bit width mode.
 * @param[in] fifo_chn      - fifo select
 * @param[in] i2s_select    - i2s channel select
 * @param[in] i2s_ain_mode  - fifo i2s input mode select
 * @return    none
 */
void audio_set_i2s_fifo_input_mode(audio_fifo_chn_e fifo_chn,audio_i2s_select_e i2s_select,audio_i2s_in_mode_e i2s_ain_mode)
{
    reg_audio_i2s_tune(i2s_select) = (fifo_chn== FIFO0)?((reg_audio_i2s_tune(i2s_select)&(~FLD_AUDIO_I2S_AIN0_COME))|(i2s_ain_mode)):((reg_audio_i2s_tune(i2s_select)&(~FLD_AUDIO_I2S_AIN1_COME))|(i2s_ain_mode<<2));
}

/**
 * @brief     This function serves to set i2s data output data bit width mode.
 * @param[in] i2s_select      - i2s channel select
 * @param[in] i2s_aout_mode   - fifo i2s output mode select
 * @return    none
 */
void audio_set_i2s_fifo_output_mode(audio_i2s_select_e i2s_select,audio_out_mode_e i2s_aout_mode)
{
    reg_audio_i2s_tune(i2s_select) = (reg_audio_i2s_tune(i2s_select)&(~FLD_AUDIO_I2S_AOUT_COME))|(i2s_aout_mode<<4);
}

/**
 * @brief     This function serves to set fifo input data path.
 * @param[in] fifo_chn - fifo select
 * @param[in] ain_sel  - fifo input source select
 * @return    none
 */
void audio_data_fifo_input_path_sel (audio_fifo_chn_e fifo_chn,audio_mux_ain_e ain_sel)
{
    reg_audio_sel(fifo_chn) = (reg_audio_sel(fifo_chn)&(~FLD_AUDIO_AIN_SEL))|(ain_sel);
}

/**
 * @brief     This function serves to set fifo output data path .
 * @param[in] fifo_chn - fifo select
 * @param[in] aout_sel - fifo output source select
 * @return    none
 */
void audio_data_fifo_output_path_sel (audio_fifo_chn_e fifo_chn,audio_mux_aout_e aout_sel)
{
    reg_audio_sel(fifo_chn) =  (reg_audio_sel(fifo_chn)&(~FLD_AUDIO_AOUT_SEL))|(aout_sel<<3);
}

/**
 * @brief     This function serves to power on audio.
 * @return    none
 * @attention Must power on before reading and writing the audio related register.
 */
void audio_power_on(void)
{
    /*The power-on sequence that must be followed
     * 1.power down audio power
     * 2.wait audio power stabilization
     * 3.switch on audio power switch
     * 4.power on audio power
     * 5.wait audio power stabilization
     * */
    if (0 == audio_powered_flag) /* repeated power-up can cause abnormal audio functions. */
    {
        audio_powered_flag = 1;
        analog_write_reg8(0x7d,analog_read_reg8(0x7d)|(BIT(2)));   //1.audio power down  ,     1:power down 0:power on
        delay_us(6);                                               //2.wait audio power stabilization
        analog_write_reg8(0x1e,analog_read_reg8(0x1e) & (~BIT(5)));//3.power switch default 1, 1:switch off 0:switch on
        analog_write_reg8(0x7d,analog_read_reg8(0x7d) & (~BIT(2)));//4.audio power on          1:power down 0:power on
        delay_us(6);                                               //5.wait audio power stabilization
    }
}

/**
 * @brief      This function serves to power down all audio power.
 * @return     none
 * @attention  1.audio digital registers are lost and need to be reinitialized
 *             2.If you only need to turn off adc or dac power, please configure audio_codec_adc_power_down()/audio_codec_dac_power_down().
 */
_attribute_ram_code_sec_optimize_o2_ void audio_power_down(void)
{
    /*The power-down sequence that must be followed
     * 1.power down audio power
     * 2.switch off audio power switch
     * */
    analog_write_reg8(0x7d,analog_read_reg8(0x7d) |BIT(2));//audio power        default 1, 1:power down 0:power on
    analog_write_reg8(0x1e,analog_read_reg8(0x1e) |BIT(5));//codec power switch default 1, 1:switch off 0:switch on
    audio_codec_adc_power_down();
    audio_codec_dac_power_down();
    audio_powered_flag = 0;
}

/**
 * @brief     This function serves to initial audio.
 * @return    none
 */
void audio_codec_init(void)
{
    audio_power_on();
    audio_set_codec_clk(1,16);//mclk 192/16=12M for internal codec
    reg_codec_clk_ctr1 |=FLD_AUDIO_CODEC_CLK_USB|FLD_AUDIO_CODEC_CLK0_EN;// usb mode and codec clk enable
}

/**
 * @brief     This function serves to set codec_adc clock.
 * @return    none
 */
void audio_codec_set_adc_clock(void)
{
    reg_codec_adc_1m_clk_ctr|= FLD_AUDIO_CODEC_CLK_1M_SEL|FLD_AUDIO_CODEC_CLK_1M_EN;//set 1M clk  for adc
}

/**
 * @brief      This function serves to power on codec_adc.
 * @param[in]  ch_en                - adc chn enable
 * @param[in]  micbias_power_en     - micbias power enable
 * @return    none
 */
void  audio_codec_adc_power_on(audio_chn_sel_e ch_en,power_switch_e micbias_power_en)
{
    audio_codec_set_micbias(micbias_power_en,MICBIAS_NORMAL_1V6_MODE);
    analog_write_reg8(0x1a,analog_read_reg8(0x1a) & (~BIT(0)));//pd_LDO_adc=0(power on)

    reg_codec_ana_reg1=(reg_codec_ana_reg1&(~(FLD_AUDIO_PD_PGABUF|FLD_AUDIO_PD_PGABOOST|FLD_AUDIO_PD_INPPGA)))|MASK_VAL(FLD_AUDIO_PD_PGABUF,((ch_en&CHANNEL_LEFT)==CHANNEL_LEFT) ? 0:1,\
                                                                FLD_AUDIO_PD_PGABOOST,((ch_en&CHANNEL_LEFT)==CHANNEL_LEFT) ? 0:1,\
                                                                FLD_AUDIO_PD_INPPGA,  ((ch_en&CHANNEL_LEFT)==CHANNEL_LEFT) ? 0:1);
    reg_codec_ana_reg2=(reg_codec_ana_reg2&(~(FLD_AUDIO_PD_ASDM)))|MASK_VAL(FLD_AUDIO_PD_ASDM, ((ch_en&CHANNEL_LEFT)==CHANNEL_LEFT) ? 0:1);


    reg_codec_ana_reg12=(reg_codec_ana_reg12&(~(FLD_AUDIO_PD_PGABUF|FLD_AUDIO_PD_PGABOOST|FLD_AUDIO_PD_INPPGA)))|MASK_VAL(FLD_AUDIO_PD_PGABUF, ((ch_en&CHANNEL_RIGHT)==CHANNEL_RIGHT) ? 0:1,\
                                                             FLD_AUDIO_PD_PGABOOST,((ch_en&CHANNEL_RIGHT)==CHANNEL_RIGHT) ? 0:1,\
                                                             FLD_AUDIO_PD_INPPGA,  ((ch_en&CHANNEL_RIGHT)==CHANNEL_RIGHT) ? 0:1);
    reg_codec_ana_reg13=(reg_codec_ana_reg13&(~(FLD_AUDIO_PD_ASDM)))|MASK_VAL(FLD_AUDIO_PD_ASDM,((ch_en&CHANNEL_RIGHT)==CHANNEL_RIGHT) ? 0:1);
}

/**
 * @brief     This function serves to power down codec_adc.
 * @return    none
 */
_attribute_ram_code_sec_optimize_o2_ void audio_codec_adc_power_down(void)
{
    analog_write_reg8(0x1a,analog_read_reg8(0x1a)|BIT(0));//pd_LDO_adc=0(power off)
    audio_codec_set_micbias(POWER_DOWN,MICBIAS_NORMAL_1V6_MODE);
}

/**
 * @brief      This function serves to  select dac/drv lbias current.
 * @param[in]  lbias_curr - current  select
 * @return     none
 */
void audio_codec_set_output_lbias_curr(codec_lbias_curr_sel_e  lbias_curr)
{
    reg_codec_ana_reg4= (lbias_curr&0x03)<<5;
    reg_codec_ana_reg5= ((lbias_curr&BIT(2)) == BIT(2))? 1:0;
}

/**
 * @brief      This function serves to dac initialize.
 * @param[in]  work_mode - dac work mode
 * @return     none
 */
void audio_codec_dac_init(codec_dac_mode_e work_mode)
{
     /*CIC compensation +0.6dB, default 0*/
    reg_codec_pga_l_2|= FLD_AUDIO_CIC_COMP_EN;
    /*BIT[4]dsm sigma-delta modulator g coefficient select , BIT[5] sigma-delta modulator quantizer select  default 1*/
    reg_codec_pga_l_2&= ~(BIT_RNG(4,5));
    if(HP_MODE == work_mode )
    {
        audio_codec_set_output_lbias_curr(CURR_5P0_UA);
    }
    else if(NML_MODE == work_mode )
    {
        audio_codec_set_output_lbias_curr(CURR_3P8_UA);
    }
    else if(LP_MODE == work_mode )
    {
        audio_codec_set_output_lbias_curr(CURR_3P07_UA);
    }

    /* FLD_AUDIO_INT_VREF must be 1(Bypass the Vref_buf LPF filter)*/
    reg_codec_ana_reg6=MASK_VAL(FLD_AUDIO_HP_VREF_BUF,1,\
                                FLD_AUDIO_INT_VREF,  1,\
                                FLD_AUDIO_LP_VREF_BUF,0,\
                                FLD_AUDIO_LP_L_DAC,  1,\
                                FLD_AUDIO_LP_R_DAC,  1,\
                                FLD_AUDIO_LP_L_DRV,  0,\
                                FLD_AUDIO_LP_R_DRV,  0);

    reg_codec_ana_reg7=(reg_codec_ana_reg7&(~(FLD_AUDIO_HP_CTRL_DRV_L|FLD_AUDIO_HP_CTRL_DRV_R)))|MASK_VAL(FLD_AUDIO_HP_CTRL_DRV_L,1,\
                        FLD_AUDIO_HP_CTRL_DRV_R,1);
}

/**
 * @brief      This function serves to power on codec_dac.
 * @param[in]  ch_en - dac chn enable
 * @return     none
 */
void audio_codec_dac_power_on(audio_chn_sel_e ch_en)
{
    analog_write_reg8(0x1a,analog_read_reg8(0x1a) & (~BIT(1))); //pd_LDO_dac=0(power on)
    analog_write_reg8(0x1a,analog_read_reg8(0x1a) & (~BIT(2))); //pd_LDO_pa =0(on))

    reg_codec_ana_reg3=(reg_codec_ana_reg3&(~(FLD_AUDIO_PD_PA_L|FLD_AUDIO_PD_PA_R|FLD_AUDIO_PD_DAC_L|FLD_AUDIO_PD_DAC_R)))|MASK_VAL(FLD_AUDIO_PD_PA_L, ((ch_en&CHANNEL_LEFT)==CHANNEL_LEFT)? 0:1,\
                                                                  FLD_AUDIO_PD_PA_R, ((ch_en&CHANNEL_RIGHT)==CHANNEL_RIGHT) ? 0:1,\
                                                                  FLD_AUDIO_PD_DAC_L,((ch_en&CHANNEL_LEFT)==CHANNEL_LEFT )? 0:1,\
                                                                  FLD_AUDIO_PD_DAC_R,((ch_en&CHANNEL_RIGHT)==CHANNEL_RIGHT) ? 0:1);
    /*FLD_AUDIO_FRC_ON_DRL must be 1*/
    reg_codec_ana_reg7=(reg_codec_ana_reg7&(~(FLD_AUDIO_FRC_ON_DRVL|FLD_AUDIO_FRC_ON_DRVR)))|MASK_VAL(FLD_AUDIO_FRC_ON_DRVL,((ch_en&CHANNEL_LEFT))==CHANNEL_LEFT ? 1:0,\
                                FLD_AUDIO_FRC_ON_DRVR,((ch_en&CHANNEL_RIGHT)==CHANNEL_RIGHT) ? 1:0);
}

/**
 * @brief     This function serves to power down codec_dac.
 * @return    none
 */
_attribute_ram_code_sec_optimize_o2_ void audio_codec_dac_power_down(void)
{
    reg_codec_ana_reg3 |= (BIT_RNG(0,1));                   //pd_PA L/R =1(off)
    analog_write_reg8(0x1a,analog_read_reg8(0x1a)|BIT(2));  //pd_LDO_pa =0(off))
    reg_codec_ana_reg3 |= (BIT_RNG(2,3));                   //pd_DAC L/R=1(off)
    analog_write_reg8(0x1a,analog_read_reg8(0x1a)|BIT(1));  //pd_LDO_dac=1((off))
}

/**
 * @brief      This function serves to set amic micbias.
 * @param[in]  en               - POWER_DOWN or POWER_ON
 * @param[in]  micbias_mode     - micbias output mode
 * @return     none
 */
_attribute_ram_code_sec_optimize_o2_ void audio_codec_set_micbias(power_switch_e en ,micbias_work_mode_e  micbias_mode)
{
    if(en)
    {
        if(micbias_mode == MICBIAS_AVD_3V3_MODE){
            analog_write_reg8(0x26,(analog_read_reg8(0x26)&(~((BIT(0)|BIT(1)|BIT(4)))))|MASK_VAL(BIT(0),1,BIT(1),0,BIT(4),1));
        }
        else if(micbias_mode == MICBIAS_NORMAL_1V6_MODE){
            analog_write_reg8(0x26,(analog_read_reg8(0x26)&(~((BIT(0)|BIT(1)|BIT(4)))))|MASK_VAL(BIT(0),0,BIT(1),0,BIT(4),0));
        }
        else if(micbias_mode == MICBIAS_NORMAL_1V8_MODE){
            analog_write_reg8(0x26,(analog_read_reg8(0x26)&(~((BIT(0)|BIT(1)|BIT(4)))))|MASK_VAL(BIT(0),1,BIT(1),0,BIT(4),0));
        }
    }
    else
    {
        analog_write_reg8(0x26,analog_read_reg8(0x26) |BIT(1));
    }
      /*0x26<0>:mic bias voltage select,                     default:1, 0:micbias voltage=1.6V,       1:micbias voltage=1.8V
            <1>:pd_micbias                                   default:1, 0:micbias up                  1:micbias down
            <4>:bypass micbias, micbias out= IO supply 3.3V, default:0, 0:micbias normal working mode 1:micbias bypass(VAD mode)
      */
}

/**
 * @brief This function serves to configure audio stream0 input.
 * @param[in]  source          - codec input source select.
 * @param[in]  rate            - select channel sample rate
 * @return    none
 */
void audio_codec_stream0_input_config(codec_stream0_input_src_e source,audio_sample_rate_e rate)
{
    if(!(source&BIT(3)))
    {
        audio_codec_set_adc_clock();
        audio_codec_adc_power_on(source&3,source>>2);
    }
    audio_set_codec_stream0_input_mode(source>>3);
    audio_set_codec_stream0_path(source,rate);
    audio_set_codec_stream0_sample_rate(rate);

     /*Initialization sets all input stream gain to 0
      * 1.because some gains are not 0 default
      * 2.for performance test
     */
    audio_set_adc_pga_l_gain(CODEC_IN_GAIN_0_DB);
    audio_set_adc_pga_r_gain(CODEC_IN_GAIN_0_DB);
    audio_set_stream0_dig_gain0(CODEC_IN_D_GAIN0_0_DB);
    audio_set_stream0_dig_gain1(CODEC_IN_D_GAIN1_0_DB);
}

/**
 * @brief This function serves to configure audio stream1 input.
 * @param[in]  source          - audio input source select.
 * @param[in]  rate            - select channel sample rate
 * @return    none
 */
void audio_codec_stream1_input_config(codec_stream1_input_src_e source,audio_sample_rate_e rate)
{
    audio_set_codec_stream1_path(source,rate);
    audio_set_codec_stream1_sample_rate(rate);
}

/**
 * @brief This function serves to configure audio stream0 channel select swap.
 * @param[in]  fifo_chn        - select channel fifo
 * @param[in]  source          - audio stream0 input source select.
 * @param[in]  rate            - select channel sample rate
 * @return    none
 * @note
 * Condition 1 (default)
 * (1) audio_swap_stream0_data(DATA_INVERT_DIS)
 * (2)mono:fifo0->ch0_l on
 *         fifo1->ch1_r on
 * Condition 2
 * (1) audio_swap_stream0_data(DATA_INVERT_EN)
 * (2) mono:fifo0->ch1_r on
 *          fifo1->ch0_l on
 *
 *
 */
void audio_codec_swap_stream0_data(audio_fifo_chn_e fifo_chn,codec_stream0_input_src_e source,audio_sample_rate_e rate)
{
    /*at the sampling rate of 44.1k and 48k, two dmic data exchanges occur in the mono and stereo modes
     * so at the sampling rate of 44.1k and 48k, software is required to exchange the two dmic
     *  data so as to restore the original data of the two dmics.
     *
     * sample rate = AUDIO_44P1K or AUDIO_48K,fifo = fifo0
     * (1) audio_swap_stream0_data(DATA_INVERT_EN)
     * (2) mono:fifo0->ch1_r on
     *          fifo1->ch0_l on
     * sample rate = AUDIO_44P1K or AUDIO_48K,fifo = fifo1
     * (1) audio_swap_stream0_data(DATA_INVERT_DIS)
     * (2) mono:fifo0->ch0_l on
     *          fifo1->ch1_r on
     */

    codec_stream0_input_src_e input_src;
    input_src = source;

    switch(source)
 {
    case LINE_STREAM0_MONO_L:
    case AMIC_STREAM0_MONO_L:
    case DMIC_STREAM0_MONO_L:if((rate == AUDIO_44P1K || rate == AUDIO_48K)&&(input_src == DMIC_STREAM0_MONO_L)){
                                (fifo_chn == FIFO1)?(audio_swap_stream0_data(DATA_INVERT_DIS)):(audio_swap_stream0_data(DATA_INVERT_EN));
                             }
                             else{
                                 (fifo_chn == FIFO1)?(audio_swap_stream0_data(DATA_INVERT_EN)):(audio_swap_stream0_data(DATA_INVERT_DIS));
                             }
                             break;
    case LINE_STREAM0_MONO_R:
    case AMIC_STREAM0_MONO_R:
    case DMIC_STREAM0_MONO_R:if((rate == AUDIO_44P1K || rate == AUDIO_48K)&&(input_src == DMIC_STREAM0_MONO_R)){
                               (fifo_chn == FIFO0)?(audio_swap_stream0_data(DATA_INVERT_DIS)):(audio_swap_stream0_data(DATA_INVERT_EN));
                             }
                             else{
                                (fifo_chn == FIFO0)?(audio_swap_stream0_data(DATA_INVERT_EN)):(audio_swap_stream0_data(DATA_INVERT_DIS));
                             }
                             break;
    case DMIC_STREAM0_STEREO:if(rate == AUDIO_44P1K || rate == AUDIO_48K){
                                audio_swap_stream0_data(DATA_INVERT_EN);
                             }
                             break;
    default:break;
 }
}

/**
 * @brief This function serves to configure audio stream1 channel select swap.
 * @param[in]  fifo_chn        - select channel fifo
 * @param[in]  source          - audio stream1 input source select.
 * @param[in]  rate            - select channel sample rate
 * @return    none
 * @note
 * Condition 1 (default)
 * (1) audio_swap_stream1_data(DATA_INVERT_DIS)
 * (2)mono:fifo0->ch0_l on
 *         fifo1->ch1_r on
 * Condition 2
 * (1) audio_swap_stream1_data(DATA_INVERT_EN)
 * (2) mono:fifo0->ch1_r on
 *          fifo1->ch0_l on
 *
 *
 */
void audio_codec_swap_stream1_data(audio_fifo_chn_e fifo_chn,codec_stream1_input_src_e source,audio_sample_rate_e rate)
{
    /*at the sampling rate of 44.1k and 48k, two dmic data exchanges occur in the mono and stereo modes
     * so at the sampling rate of 44.1k and 48k, software is required to exchange the two dmic
     *  data so as to restore the original data of the two dmics.
     *
     * sample rate = AUDIO_44P1K or AUDIO_48K,fifo = fifo0
     * (1) audio_swap_stream0_data(DATA_INVERT_EN)
     * (2) mono:fifo0->ch1_r on
     *          fifo1->ch0_l on
     * sample rate = AUDIO_44P1K or AUDIO_48K,fifo = fifo1
     * (1) audio_swap_stream0_data(DATA_INVERT_DIS)
     * (2) mono:fifo0->ch0_l on
     *          fifo1->ch1_r on
     */
    switch(source)
 {
    case DMIC_STREAM1_MONO_L:if(rate == AUDIO_44P1K || rate == AUDIO_48K){
                               (fifo_chn == FIFO1)?(audio_swap_stream1_data(DATA_INVERT_DIS)):(audio_swap_stream1_data(DATA_INVERT_EN));
                             }
                             else{
                               (fifo_chn == FIFO1)?(audio_swap_stream1_data(DATA_INVERT_EN)):(audio_swap_stream1_data(DATA_INVERT_DIS));
                             }
                             break;
    case DMIC_STREAM1_MONO_R:if(rate == AUDIO_44P1K || rate == AUDIO_48K){
                               (fifo_chn == FIFO0)?(audio_swap_stream1_data(DATA_INVERT_DIS)):(audio_swap_stream1_data(DATA_INVERT_EN));
                             }
                            else{
                               (fifo_chn == FIFO0)?(audio_swap_stream1_data(DATA_INVERT_EN)):(audio_swap_stream1_data(DATA_INVERT_DIS));
                             }
                            break;
    case DMIC_STREAM1_STEREO:if(rate == AUDIO_44P1K || rate == AUDIO_48K){
                             audio_swap_stream1_data(DATA_INVERT_EN);
                             }
                            break;
    default:break;
 }
}

/**
 * @brief This function serves to configure audio stream0 fifo input.
 * @param[in]  fifo_chn        - select channel fifo
 * @param[in]  source          - codec input source
 * @param[in]  data_width      - fifo data bit width.
 * @return    none
 */
void audio_codec_stream0_fifo_input_config(audio_fifo_chn_e fifo_chn,codec_stream0_input_src_e source,audio_codec_wl_mode_e data_width)
{
    audio_codec_set_stream0_fifo_input_mode(fifo_chn,source,data_width);
    audio_data_fifo_input_path_sel(fifo_chn,CODEC_STREAM0_IN_FIFO);
}

/**
 * @brief This function serves to configure audio stream1 fifo input.
 * @param[in]  fifo_chn        - select channel fifo
 * @param[in]  source          - codec input source
 * @param[in]  data_width      - fifo data bit width.
 * @return    none
 */
void audio_codec_stream1_fifo_input_config(audio_fifo_chn_e fifo_chn,codec_stream1_input_src_e source,audio_codec_wl_mode_e data_width)
{
    audio_codec_set_stream1_fifo_input_mode(fifo_chn,source,data_width);
    audio_data_fifo_input_path_sel(fifo_chn,CODEC_STREAM1_IN_FIFO);
}

/**
 * @brief This function serves to initialize audio stream0 input.
 * @param[in]  audio_codec_input     - input configuration data structure pointer.
 * @return    none
 */
void audio_codec_stream0_input_init(audio_codec_stream0_input_t* audio_codec_input)
{
    audio_codec_stream0_input_config(audio_codec_input->input_src,audio_codec_input->sample_rate);
    audio_codec_swap_stream0_data(audio_codec_input->fifo_num,audio_codec_input->input_src,audio_codec_input->sample_rate);
    audio_codec_stream0_fifo_input_config(audio_codec_input->fifo_num,audio_codec_input->input_src,audio_codec_input->data_width);
}

/**
 * @brief This function serves to initialize audio stream1 input.
 * @param[in]  audio_codec_input     - input configuration data structure pointer.
 * @return    none
 */
void audio_codec_stream1_input_init(audio_codec_stream1_input_t* audio_codec_input)
{
    audio_codec_stream1_input_config(audio_codec_input->input_src,audio_codec_input->sample_rate);
    audio_codec_swap_stream1_data(audio_codec_input->fifo_num,audio_codec_input->input_src,audio_codec_input->sample_rate);
    audio_codec_stream1_fifo_input_config(audio_codec_input->fifo_num,audio_codec_input->input_src,audio_codec_input->data_width);
}

/**
 * @brief This function serves to configure audio stream output.
 * @param[in]  chn             - audio output source select.
 * @param[in]  rate            - select channel sample rate
 * @param[in]  dac_mode        - select channel dac mode
 * @return    none
 */
void audio_codec_stream_output_config(audio_dac_output_src_e chn,audio_sample_rate_e rate,codec_dac_mode_e dac_mode)
{

    audio_codec_dac_power_on((audio_chn_sel_e)chn);
    audio_codec_dac_init(dac_mode);
    audio_set_codec_dac_sample_rate(rate);

     /*Initialization sets all dac gain to 0
      * 1.because some gains are not 0 default
      * 2.for performance test
     */
    audio_set_dac_pga_l_gain(CODEC_OUT_GAIN_0_DB);
    audio_set_dac_pga_r_gain(CODEC_OUT_GAIN_0_DB);
    audio_set_dac_l_gain(CODEC_OUT_D_GAIN_0_DB);
    audio_set_dac_r_gain(CODEC_OUT_D_GAIN_0_DB);
}

/**
 * @brief This function serves to configure audio stream fifo output.
 * @param[in]  fifo_chn         - select channel fifo
 * @param[in]  chn              - audio output source select.
 * @param[in]  data_width       - fifo data bit width.
 * @return    none
 */
void audio_codec_stream_fifo_output_config(audio_fifo_chn_e fifo_chn,audio_dac_output_src_e chn,audio_codec_wl_mode_e data_width)
{
    audio_set_dac_fifo_output_mode_config(fifo_chn,chn,data_width);
    audio_data_fifo_output_path_sel(fifo_chn,CODEC_INT_OUT_FIFO);
}

/**
 * @brief This function serves to initialize audio stream output.
 * @param[in]  audio_codec_output   - output configuration data structure pointer.
 * @return    none
 * @note   When mono channel, The hardware will output the data of mono channel on the dac L and R at the same time.
 *         If only need one channel output, in order to save power, you can turn off any channel
 * For example:audio_codec_output->output_src= CODEC_DAC_MONO_L or CODEC_DAC_MONO_R select mono channel
 */
void audio_codec_stream_output_init(audio_codec_output_t *audio_codec_output)
{
    audio_codec_stream_output_config(audio_codec_output->output_src,audio_codec_output->sample_rate,audio_codec_output->mode);
    audio_codec_stream_fifo_output_config(audio_codec_output->fifo_num,audio_codec_output->output_src,audio_codec_output->data_width);
}

/**
 * @brief     This function configures i2s mclk pin for external codec,mclk=192M*(div_numerator/div_denominator):192M*(1/16)=12M.
 * @param[in] mclk_pin -mclk output pin.
 * @param[in] div_numerator
 * @param[in] div_denominator
 * @return    none
 * @attention If need to use internal codec at the same time, mclk must be set to 12M.
 */
void audio_i2s_set_mclk(gpio_func_pin_e mclk_pin,unsigned short div_numerator,unsigned short div_denominator)
{
    audio_set_codec_clk(div_numerator,div_denominator);
    gpio_set_probe_clk_function(mclk_pin,PROBE_CODEC_MCLK);
}

/**
 * @brief     This function configures i2s pin.
 * @param[in] i2s_selecti2s - channel select
 * @param[in] config        - i2s config pin struct
 * @return    none
 */
void i2s_set_pin(audio_i2s_select_e i2s_select,i2s_pin_config_t *config)
{
    gpio_input_en((gpio_pin_e)config->bclk_pin);
    gpio_set_mux_function(config->bclk_pin,I2S0_BCK_IO+(i2s_select)*6);
    gpio_function_dis((gpio_pin_e)config->bclk_pin);

    if(config->adc_lr_clk_pin!=GPIO_NONE_PIN)
    {
        gpio_input_en((gpio_pin_e)config->adc_lr_clk_pin);
        gpio_set_mux_function(config->adc_lr_clk_pin,I2S0_LR_IN_IO+(i2s_select)*6);
        gpio_function_dis((gpio_pin_e)config->adc_lr_clk_pin);
    }
    if(config->dac_lr_clk_pin!=GPIO_NONE_PIN)
    {
        gpio_input_en((gpio_pin_e)config->dac_lr_clk_pin);
        gpio_set_mux_function(config->dac_lr_clk_pin,I2S0_LR_OUT_IO+(i2s_select)*6);
        gpio_function_dis((gpio_pin_e)config->dac_lr_clk_pin);
    }
    if(config->adc_dat_pin!=GPIO_NONE_PIN)
    {
        gpio_input_en((gpio_pin_e)config->adc_dat_pin);
        gpio_set_mux_function(config->adc_dat_pin,I2S0_DAT_IN_I+(i2s_select)*6);
        gpio_function_dis((gpio_pin_e)config->adc_dat_pin);
    }
    if(config->dac_dat_pin!=GPIO_NONE_PIN)
    {
        gpio_input_en((gpio_pin_e)config->dac_dat_pin);
        gpio_set_mux_function(config->dac_dat_pin,I2S0_DAT_OUT+(i2s_select)*6);
        gpio_function_dis((gpio_pin_e)config->dac_dat_pin);
    }
}

/**
 * @brief     This function set i2s input .
 * @param[in] fifo_chn         - fifo channel selection
 * @param[in] i2s_select       - channel select
 * @param[in] data_mode        - i2s data bit with
 * @param[in] ch               - i2s channel selection
 * @return    none
 */
void audio_set_i2s_input_chn_wl(audio_fifo_chn_e fifo_chn,audio_i2s_select_e i2s_select,audio_i2s_wl_mode_e data_mode,i2s_channel_select_e ch)
{
    unsigned char i2s_mode;
    i2s_mode=(ch == I2S_MONO) ? ((data_mode==I2S_BIT_16_DATA) ? I2S_BIT_16_MONO :I2S_BIT_20_OR_24_MONO) : ( (data_mode == I2S_BIT_16_DATA) ? I2S_BIT_16_STEREO :I2S_BIT_20_OR_24_STEREO);
    audio_set_i2s_fifo_input_mode(fifo_chn,i2s_select,i2s_mode);
}

/**
 * @brief     This function set i2s output .
 * @param[in] fifo_chn         - fifo channel selection
 * @param[in] i2s_select       - channel select
 * @param[in] data_mode        - i2s data bit with
 * @param[in] ch               - i2s channel selection
 * @return    none
 */
void audio_set_i2s_output_chn_wl(audio_fifo_chn_e fifo_chn,audio_i2s_select_e i2s_select,audio_i2s_wl_mode_e data_mode,i2s_channel_select_e ch)
{
    unsigned char i2s_mode;
    if((data_mode == I2S_BIT_16_DATA_FIFO0_AND_FIFO1)&&(ch == I2S_STEREO)){
        i2s_mode = BIT_16_STEREO_FIFO0_AND_FIFO1;
    }
    else if((data_mode == I2S_BIT_20_OR_24_DATA_FIFO0_AND_FIFO1)&&(ch == I2S_STEREO)){
        i2s_mode = BIT_20_OR_24_STEREO_FIFO0_AND_FIFO1;
    }
    else{
        if(fifo_chn == FIFO0){
          i2s_mode=(ch == I2S_MONO) ? ((data_mode==I2S_BIT_16_DATA) ? BIT_16_MONO_FIFO0 :BIT_20_OR_24_MONO_FIFO0) : ( (data_mode == I2S_BIT_16_DATA) ? BIT_16_STEREO_FIFO0 :BIT_20_OR_24_STEREO_FIFO0);
        }
        else{
          i2s_mode=(ch == I2S_MONO) ? ((data_mode==I2S_BIT_16_DATA) ? BIT_16_MONO_FIFO1 :BIT_20_OR_24_MONO_FIFO1) : ( (data_mode == I2S_BIT_16_DATA) ? BIT_16_STEREO_FIFO1 :BIT_20_OR_24_STEREO_FIFO1);
        }
    }
    audio_set_i2s_fifo_output_mode(i2s_select,i2s_mode);
}

/**
 * @brief     This function serves to config i2s0 interface, word length, and m/s.
 * @param[in] i2s_sel      - i2s channel select
 * @param[in] i2s_format   - interface protocol
 * @param[in] wl           - audio data word length
 * @param[in] m_s          - select i2s as master or slave
 * @param[in] i2s_config_t - the ptr of i2s_config_t that configure i2s lr_clk phase and lr_clk swap.
 *  i2s_config_t->i2s_lr_clk_invert_select-lr_clk phase control(in RJ,LJ or i2s modes),in i2s mode(opposite phasing in  RJ,LJ mode), 0=right channel data when lr_clk high ,1=right channel data when lr_clk low.
 *                                                                                     in DSP mode(in DSP mode only), DSP mode A/B select,0=DSP mode A ,1=DSP mode B.
 *            i2s_config_t->i2s_data_invert_select - 0=left channel data left,1=right channel data left.
 * but data output channel will be inverted,you can also set i2s_config_t->i2s_data_invert_select=1 to recovery it.
 * @return    none
 */
void audio_i2s_config(audio_i2s_select_e i2s_sel,i2s_mode_select_e i2s_format,audio_i2s_wl_mode_e wl, i2s_m_s_mode_e m_s , audio_i2s_invert_config_t * i2s_config_t)
{
    reg_i2s_cfg1(i2s_sel) = MASK_VAL(FLD_AUDIO_I2S_FORMAT,     i2s_format,\
                                     FLD_AUDIO_I2S_WL,         wl,\
                                     FLD_AUDIO_I2S_LRP,        i2s_config_t->i2s_lr_clk_invert_select,\
                                     FLD_AUDIO_I2S_LRSWAP,     i2s_config_t->i2s_data_invert_select,\
                                     FLD_AUDIO_I2S_ADC_DCI_MS, m_s,\
                                     FLD_AUDIO_I2S_DAC_DCI_MS, m_s);
}

/**
 * @brief     This function serves to set sampling rate when i2s as master.
 * @param[in] i2s_select                    - i2s channel select
 * @param[in] i2s_clk_config                 i2s_clk_config[2]   i2s_clk_config[3]-->lrclk_adc(sampling rate)
                                                  ||                  ||
 *  pll(192M default)------->div---->i2s_clk--->2*div(div=0,bypass)--->blck----->div
 *                           ||                                       ||
 *           i2s_clk_config[0]/i2s_clk_config[1]                 i2s_clk_config[4]-->lrclk_dac (sampling rate)
 *
 For example:sampling rate=16K, i2s_clk_config[5]={ 8,125,6,64,64}, sampling rate=192M*(8/125)/(2*6)/64=16K

 * @return    none
 * @attention The default is from pll 192M(default). If the pll is changed, the clk will be changed accordingly.
 */
_attribute_ram_code_sec_noinline_ void audio_set_i2s_clock (audio_i2s_select_e i2s_select,unsigned short *i2s_clk_config)
{
    audio_set_i2s_clk(i2s_select,i2s_clk_config[0],i2s_clk_config[1]);
    audio_set_i2s_bclk(i2s_select,i2s_clk_config[2]);
    audio_set_i2s_lrclk(i2s_select,i2s_clk_config[3],i2s_clk_config[4]);
}

/**
 * @brief This function serves to initialize  configuration i2s.
 * @param[in]  i2s_config - the relevant configuration struct pointer
 * @return    none
 */
void audio_i2s_config_init(audio_i2s_config_t* i2s_config)
{
    i2s_set_pin(i2s_config->i2s_select,i2s_config->pin_config);
    if(i2s_config->master_slave_mode == I2S_AS_MASTER_EN)
    {
        audio_set_i2s_clock(i2s_config->i2s_select,i2s_config->sample_rate);
    }
    audio_i2s_config(i2s_config->i2s_select,i2s_config->i2s_mode,i2s_config->data_width,i2s_config->master_slave_mode,&audio_i2s_invert_config[i2s_config->i2s_select]);
    audio_clk_i2s_en(i2s_config->i2s_select);
}

/**
 * @brief This function serves to initialize  input i2s.
 * @param[in] audio_i2s_input - the relevant input struct pointer
 * @return    none
 */
void audio_i2s_input_init(audio_i2s_input_output_t* audio_i2s_input)
{
    audio_set_i2s_input_chn_wl(audio_i2s_input->fifo_chn,audio_i2s_input->i2s_select,audio_i2s_input->data_width,audio_i2s_input->i2s_ch_sel);
    audio_data_fifo_input_path_sel(audio_i2s_input->fifo_chn,(audio_i2s_input->i2s_select == I2S0) ? (I2S0_IN_FIFO) : (I2S1_IN_FIFO));
}

/**
 * @brief This function serves to initialize  output i2s.
 * @param[in]  audio_i2s_output - the relevant output struct pointer
 * @return    none
 */
void audio_i2s_output_init(audio_i2s_input_output_t* audio_i2s_output)
{
    audio_set_i2s_output_chn_wl(audio_i2s_output->fifo_chn,audio_i2s_output->i2s_select,audio_i2s_output->data_width,audio_i2s_output->i2s_ch_sel);

    if((audio_i2s_output->data_width == I2S_BIT_16_DATA_FIFO0_AND_FIFO1)||(audio_i2s_output->data_width == I2S_BIT_20_OR_24_DATA_FIFO0_AND_FIFO1))
    {
        audio_data_fifo_output_path_sel(audio_i2s_output->fifo_chn,I2S0_OUT_FIFO);
    }
    else
    {
        audio_data_fifo_output_path_sel(audio_i2s_output->fifo_chn,(audio_i2s_output->i2s_select == I2S0) ? (I2S0_OUT_FIFO) : (I2S1_OUT_FIFO));
    }
}

/**
 * @brief This function serves to enable the i2s align function.
 * @return    none
 */
void  audio_i2s_align_en(void)
{
    /*for sync mode, select iis0 for the sync clock source*/
    reg_align_ctrl |=FLD_I2S_ALIGN_EN|FLD_I2S_CLK_SEL|FLD_I2S_ALIGN_CTRL;
}

/**
 * @brief This function serves to disable the i2s align function.
 * @return    none
 */
void  audio_i2s_align_dis(void)
{
    reg_align_ctrl &= ~FLD_I2S_CLK_SEL;
    reg_usb_audio_ctrl |=FLD_I2S_ALIGN_MASK;
    reg_align_ctrl &= ~FLD_I2S_ALIGN_EN;
}

/**
 * @brief This function serves to sets the align data bit width .(i2s sync mode 16bit:{i2s1_r[15:0], i2s1_l[15:0]}
 *                                                                                    {i2s0_r[15:0], i2s0_l[15:0]}
 *                                                               (i2s sync mode 20bit:{12{i2s1_l[19]},i2s1_l[19:0]}
 *                                                                                    {12{i2s1_r[19]},i2s1_r[19:0]}
 *                                                                                    {12{i2s0_l[19]},i2s0_l[19:0]}
 *                                                                                    {12{i2s0_r[19]},i2s0_r[19:0]}
 *                                                               (i2s sync mode 24bit:{8{i2s1_l[23]}, i2s1_l[23:0]}
 *                                                                                    {8{i2s1_r[23]}, i2s1_r[23:0]}
 *                                                                                    {8{i2s0_l[23]}, i2s0_l[23:0]}
 *                                                                                    {8{i2s0_r[23]}, i2s0_r[23:0]}
 * @param[in]  align_data_width - align data bit widths
 * @return    none
 */
void audio_set_i2s_align_data_width(audio_i2s_align_data_width_e align_data_width)
{
    reg_audio_is1_ctrl= (reg_audio_is1_ctrl&(~(I2S_ALIGN_BIT_16_DATA|I2S_ALIGN_BIT_20_24_DATA)))|align_data_width;
    audio_set_i2s_fifo_output_mode(I2S1,BIT_16_STEREO_FIFO0);  //Circumvent hardware design issues and set the data format for output data when using the synchronization function
}
