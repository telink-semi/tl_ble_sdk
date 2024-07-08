/********************************************************************************************************
 * @file    i2c.c
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
#include "i2c.h"

static unsigned char i2c_dma_tx_chn;
static unsigned char i2c_dma_rx_chn;

dma_config_t i2c_tx_dma_config={
    .dst_req_sel= DMA_REQ_I2C_TX,//tx req
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
    };
dma_config_t i2c_rx_dma_config={
    .dst_req_sel= DMA_REQ_AUDIO0_TX,
    .src_req_sel=DMA_REQ_I2C_RX,
    .dst_addr_ctrl=DMA_ADDR_INCREMENT,
    .src_addr_ctrl=DMA_ADDR_FIX,
    .dstmode=DMA_NORMAL_MODE,
    .srcmode=DMA_HANDSHAKE_MODE,
    .dstwidth=DMA_CTR_WORD_WIDTH,//must word
    .srcwidth=DMA_CTR_WORD_WIDTH,////must word
    .src_burst_size=0,
    .read_num_en=0,
    .priority=0,
    .write_num_en=0,
    .auto_en=0,//must 0
};



/*
 * This parameter is 0x20 by default, that is, each write or read API opens the stop command.
 * if g_i2c_stop_en=0x00,it means every write or read API will disable stop command.
 */
unsigned char g_i2c_stop_en=0x20;


/**
 * @brief      The function of this interface is equivalent to that after the user finishes calling the write or read interface, the stop signal is not sent,
 *                and then the write or read command is executed again. The driver defaults that every write or read API will send a stop command at the end
 * @param[in]  en - Input parameters.Decide whether to disable the stop function after each write or read interface
 * @return     none
 */
void i2c_master_send_stop(unsigned char en)
{
    if(en==1)
    {
        g_i2c_stop_en=0x20;
    }else{
        g_i2c_stop_en=0x00;
    }

}


/**
 * @brief      This function selects a pin port for I2C interface.
 * @param[in]  sda_pin - the pin port selected as I2C sda pin port.
 * @param[in]  scl_pin - the pin port selected as I2C scl pin port.
 * @return     none
 */
void i2c_set_pin(i2c_sda_pin_e sda_pin,i2c_scl_pin_e scl_pin)
{

    //When the pad is configured with mux input and a pull-up resistor is required, gpio_input_en needs to be placed before gpio_function_dis,
    //otherwise first set gpio_input_disable and then call the mux function interface,the mux pad will misread the short low-level timing.confirmed by minghai.20210709.
    gpio_input_en(sda_pin);//enable sda input
    gpio_input_en(scl_pin);//enable scl input
    gpio_set_up_down_res(sda_pin, GPIO_PIN_PULLUP_10K);
    gpio_set_up_down_res(scl_pin, GPIO_PIN_PULLUP_10K);

    unsigned char val = 0;
    unsigned char mask = 0xff;

    //enable gpio as i2c sda function.
    if(sda_pin == I2C_GPIO_SDA_B3)
    {
        mask= (unsigned char)~(BIT(7)|BIT(6));
        val = BIT(6);
    }
    else if(sda_pin == I2C_GPIO_SDA_C2)
    {
        mask = (unsigned char)~(BIT(5)|BIT(4));
        val = 0;
    }
    else if(sda_pin == I2C_GPIO_SDA_E2)
    {
        mask = (unsigned char)~(BIT(5)|BIT(4));
        val = 0;
    }
    else if(sda_pin == I2C_GPIO_SDA_E3)
    {
        mask = (unsigned char)~(BIT(7)|BIT(6));
        val = 0;
    }
    reg_gpio_func_mux(sda_pin)=(reg_gpio_func_mux(sda_pin)& mask)|val;
    //enable gpio as i2c scl function.
    if(scl_pin == I2C_GPIO_SCL_B2)
    {
        mask= (unsigned char)~(BIT(5)|BIT(4));
        val = BIT(4);
    }
    else if(scl_pin == I2C_GPIO_SCL_C1)
    {
        mask = (unsigned char)~(BIT(3)|BIT(2));
        val = 0;
    }
    else if(scl_pin == I2C_GPIO_SCL_E0)
    {
        mask = (unsigned char)~(BIT(1)|BIT(0));
        val = 0;
    }
    else if(scl_pin == I2C_GPIO_SCL_E1)
    {
        mask = (unsigned char)~(BIT(3)|BIT(2));
        val = 0;
    }
    reg_gpio_func_mux(scl_pin)=(reg_gpio_func_mux(scl_pin)& mask)|val;

    //disable sda_pin and scl_pin gpio function.
    gpio_function_dis(scl_pin);
    gpio_function_dis(sda_pin);
}

/**
 * @brief      This function serves to enable i2c master function.
 * @return     none.
 */
void i2c_master_init(void)
{
    reg_i2c_sct0  |=  FLD_I2C_MASTER;       //i2c master enable.
    i2c_master_stretch_en();
}



/**
 * @brief      This function serves to set the i2c clock frequency.The i2c clock is consistent with the pclk.
 * @param[in]  clock - the division factor of I2C clock,the i2c frequency can meet 1M, and the maximum limit is not confirmed,
 *             I2C frequency = pclk/ (4*DivClock).
 * @return     none
 */
void i2c_set_master_clk(unsigned char clock)
{

    //i2c frequency = system_clock/(4*clock)
    reg_i2c_sp = clock;

    //set enable flag.
    reg_clk_en0 |= FLD_CLK0_I2C_EN;
}


/**
 * @brief      This function serves to enable slave mode.
 * @param[in]  id - the id of slave device.it contains write or read bit,the laster bit is write or read bit.
 *                       ID|0x01 indicate read. ID&0xfe indicate write.
 * @return     none
 */
void i2c_slave_init(unsigned char id)
{
    reg_i2c_sct0 &= (~FLD_I2C_MASTER); //enable slave mode.

    reg_i2c_id      = id;                   //default eagle slave ID is 0x5a
}


/**
 *  @brief      The function of this API is to ensure that the data can be successfully sent out.
 *              can choose whether to send stop,if i2c stop is not configured, the next time a start signal is sent, it will be a restart signal,
 *              but if a nack exception is received after sending out the id, during exception handling, a stop signal will be sent.
 *  @param[in]  id - to set the slave ID.
 *  @param[in]  data - The data to be sent, The first three bytes can be set as the RAM address of the slave.
 *  @param[in]  len - This length is the total length, including both the length of the slave RAM address and the length of the data to be sent.
 *  @return     0 : the master receive NACK after sending out the id and then send stop.  1: the master sent the data successfully,(master does not detect NACK in data phase)
 */
unsigned char  i2c_master_write(unsigned char id, unsigned char *data, unsigned char len)
{
    BM_SET(reg_i2c_status,FLD_I2C_TX_CLR);//clear index
    //set i2c master write.
    reg_i2c_data_buf(0)=id & (~FLD_I2C_WRITE_READ_BIT); //BIT(0):R:High  W:Low;
    reg_i2c_sct1 = (FLD_I2C_LS_ADDR | FLD_I2C_LS_START);
    while(i2c_master_busy());
    if(reg_i2c_mst&FLD_I2C_ACK_IN)
    {
        reg_i2c_sct1 = (FLD_I2C_LS_STOP);
        while(i2c_master_busy());
        return 0;
    }
    reg_i2c_len   =  len;
    //write data
    unsigned int cnt = 0;
    while(cnt<len)
    {
        if(i2c_get_tx_buf_cnt()<8)
        {
            reg_i2c_data_buf(cnt % 4) = data[cnt];    //write data
            cnt++;
            if(cnt==1)
            {
                reg_i2c_sct1 = ( FLD_I2C_LS_DATAW|g_i2c_stop_en ); //launch stop cycle
            }
        }
    }

    while(i2c_master_busy());
    return 1;
}


/**
 * @brief      This function serves to read a packet of data from the specified address of slave device.
 *             can choose whether to send stop,if i2c stop is not configured, the next time a start signal is sent, it will be a restart signal,
 *             but if a nack exception is received after sending out the id, during exception handling, a stop signal will be sent.
 * @param[in]  id - to set the slave ID.
 * @param[in]  data - Store the read data
 * @param[in]  len - The total length of the data read back.
 * @return     0 : the master receive NACK after sending out the id and then send stop.  1: the master receive the data successfully.
 */
unsigned char  i2c_master_read(unsigned char id, unsigned char *data, unsigned char len)
{
    //set i2c master read.
    BM_SET(reg_i2c_status,FLD_I2C_RX_CLR);//clear index
    reg_i2c_sct0  |=  FLD_I2C_RNCK_EN;       //i2c rnck enable.
    reg_i2c_data_buf(0)=(id | FLD_I2C_WRITE_READ_BIT); //BIT(0):R:High  W:Low;
    reg_i2c_sct1 = (FLD_I2C_LS_ADDR | FLD_I2C_LS_START);
    while(i2c_master_busy());
    if(reg_i2c_mst&FLD_I2C_ACK_IN)
    {
        reg_i2c_sct1 = (FLD_I2C_LS_STOP);
        while(i2c_master_busy());
        return 0;
    }
    reg_i2c_sct1 = ( FLD_I2C_LS_DATAR | FLD_I2C_LS_ID_R | g_i2c_stop_en);
    reg_i2c_len   =  len;
    unsigned int cnt = 0;
    while(cnt<len)
    {
        if(i2c_get_rx_buf_cnt()>0)
        {
            data[cnt] = reg_i2c_data_buf(cnt % 4);
            cnt++;
        }
    }
    while(i2c_master_busy());
    return 1;
}


/**
 * @brief      This function serves to write data and restart read data.
 *             can choose whether to send stop or not,If i2c stop is not configured, the next time a start signal is sent, it will be a restart signal,
 *             but if a nack exception signal is received after sending out the id, during exception handling, a stop signal will be sent.
 * @param[in]  id - to set the slave ID.
 * @param[in]  wr_data - The data to be sent, The first three bytes can be set as the RAM address of the slave.
 * @param[in]  wr_len -  This length is the total length, including both the length of the slave RAM address and the length of the data to be sent.
 * @param[in]  rd_data - Store the read data
 * @param[in]  rd_len -  The total length of the data read back.
 * @return     0 : the master receive NACK after sending out the id and then send stop.  1: the master receive the data successfully.
 */
unsigned char i2c_master_write_read(unsigned char id, unsigned char *wr_data, unsigned char wr_len, unsigned char *rd_data, unsigned char rd_len)
{
    //set i2c master write.
    if(!i2c_master_write(id,wr_data,wr_len)){
        return 0;
    }
    //i2c_master_write_read: the master between writing and reading,it will be a restart signal,and after reading and writing, a stop signal is sent,
    //in order to after write and read, a stop signal is sent,so need to enable stop during read.
    unsigned char i2c_stop_en = g_i2c_stop_en;
    if(0x00 == i2c_stop_en){
        i2c_master_send_stop(1);
    }
    //set i2c master read.
    if(!i2c_master_read(id,rd_data,rd_len)){
        if(0x00 == i2c_stop_en){
            i2c_master_send_stop(0);
        }
        return 0;
    }
    //since the configuration state of stop is changed in this interface,
    //the previous configuration needs to be restored either after the function reads or when an exception occurs during the read process.
    if(0x00 == i2c_stop_en){
        i2c_master_send_stop(0);
    }
    return 1;
}


/**
 * @brief      The function of this API is just to write data to the i2c tx_fifo by DMA.
 *             can choose whether to send stop,if i2c stop is not configured, the next time a start signal is sent, it will be a restart signal,
 *             but in B91,in dma mode,there is no way to detect the abnormal phenomenon,so there is no corresponding exception handling method in case of abnormal phenomenon.
 * @param[in]  id - to set the slave ID.
 * @param[in]  data - The data to be sent, The first three bytes represent the RAM address of the slave.
 * @param[in]  len - This length is the total length, including both the length of the slave RAM address and the length of the data to be sent,
 *                   the maximum transmission length of i2c is 0xff bytes, so dont'n over this length.
 * @return     none.
 * @note       data: must be aligned by word (4 bytes), otherwise the program will enter an exception.
 */
void i2c_master_write_dma(unsigned char id, unsigned char *data, unsigned char len)
{

    //set id.
    reg_i2c_id = (id & (~FLD_I2C_WRITE_READ_BIT)); //BIT(0):R:High  W:Low

    dma_set_size(i2c_dma_tx_chn,len,DMA_WORD_WIDTH);
    dma_set_address(i2c_dma_tx_chn,(unsigned int)data,reg_i2c_data_buf0_addr);
    dma_chn_en(i2c_dma_tx_chn);

    reg_i2c_len   =  len;
    reg_i2c_sct1 = (FLD_I2C_LS_ID|FLD_I2C_LS_START|FLD_I2C_LS_DATAW |g_i2c_stop_en);

}

/**
 * @brief      This function serves to read a packet of data from the specified address of slave device.
 * @param[in]  id - to set the slave ID.
 * @param[in]  rx_data - Store the read data.
 * @param[in]  len - The total length of the data read back,the maximum transmission length of i2c is 0xFF bytes, so dont'n over this length.
 * @return     none.
 * @note       data: must be aligned by word (4 bytes), otherwise the program will enter an exception.
 */
void i2c_master_read_dma(unsigned char id, unsigned char *rx_data, unsigned char len)
{

    reg_i2c_sct0  |=  FLD_I2C_RNCK_EN;       //i2c rnck enable

    //set i2c master read.
    reg_i2c_id = (id | FLD_I2C_WRITE_READ_BIT); //BIT(0):R:High  W:Low

    dma_set_size(i2c_dma_rx_chn,len,DMA_WORD_WIDTH);
    dma_set_address(i2c_dma_rx_chn,reg_i2c_data_buf0_addr,(unsigned int)rx_data);
    dma_chn_en(i2c_dma_rx_chn);

    reg_i2c_len   =  len;
    reg_i2c_sct1 = ( FLD_I2C_LS_ID | FLD_I2C_LS_DATAR | FLD_I2C_LS_START | FLD_I2C_LS_ID_R | g_i2c_stop_en);

}

/**
 * @brief      This function serves to send a packet of data to master device.It will trigger after the master sends the read sequence.
 * @param[in]  data - the pointer of tx_buff.
 * @param[in]  len - The total length of the data,the maximum transmission length of DMA is 0xFFFFFC bytes, so dont'n over this length.
 * @return     none.
 * @note       data: must be aligned by word (4 bytes), otherwise the program will enter an exception.
 */
void i2c_slave_set_tx_dma( unsigned char *data, unsigned char len)
{
    dma_set_address(i2c_dma_tx_chn,(unsigned int)data,reg_i2c_data_buf0_addr);
    dma_set_size(i2c_dma_tx_chn,len,DMA_WORD_WIDTH);
    dma_chn_en(i2c_dma_tx_chn);
}


/**
 * @brief      This function serves to receive a packet of data from master device,It will trigger after the master sends the write sequence.
 * @param[in]  data - This parameter is the first address of the received data buffer, which must be 4 bytes aligned, otherwise the program will enter an exception.
 *                    and the actual buffer size defined by the user needs to be not smaller than the len, otherwise there may be an out-of-bounds problem.
 * @param[in]  len  - This parameter is used to set the size of the received dma and must be set to a multiple of 4. The maximum value that can be set is 0xFFFFFC.
 * @return        none
 */
void i2c_slave_set_rx_dma(unsigned char *data, unsigned char len)
{
    dma_set_address(i2c_dma_rx_chn,reg_i2c_data_buf0_addr,(unsigned int)data);
    dma_set_size(i2c_dma_rx_chn,len,DMA_WORD_WIDTH);
    dma_chn_en(i2c_dma_rx_chn);
}
//logs the current read position in the fifo.
unsigned char i2c_slave_rx_index = 0;
/**
 * @brief     This function serves to receive data .
 * @param[in]  data - the data need read.
 * @param[in]  len - The total length of the data
 * @return    none
 */
void i2c_slave_read(unsigned char* data , unsigned char len )
{
    unsigned int cnt = 0;
    while(cnt<len)
    {
        if(i2c_get_rx_buf_cnt()>0)
        {
        data[cnt] = reg_i2c_data_buf(i2c_slave_rx_index);
        i2c_slave_rx_index++;
        i2c_slave_rx_index &= 0x03 ;
        cnt++;
        }
    }
}

/**
 * @brief     This function serves to receive i2c data by byte with not DMA method.
 * @param[in]  data - the data need send.
 * @param[in]  len - The total length of the data.
 * @return    none
 */
void i2c_slave_write(unsigned char* data , unsigned char len)
{
    i2c_clr_fifo(I2C_TX_BUFF_CLR);
    unsigned int cnt = 0;
    while(cnt<len)
    {
        if(i2c_get_tx_buf_cnt()<8)
        {
            reg_i2c_data_buf(cnt % 4) = data[cnt];
            cnt++;
        }
    }
}


/**
 * @brief     This function serves to set i2c tx_dma channel and config dma tx default.
 * @param[in] chn: dma channel.
 * @return    none
 * @note      In the case that the DMA transfer is not completed(bit 0 of reg_dma_ctr0(chn): 1-the transmission has not been completed,0-the transmission is completed), re-calling the DMA-related functions may cause problems.
 *            If you must do this, you must perform the following sequence:
 *            1. dma_chn_dis(i2c_dma_tx_chn) 2.i2c_reset() 3.i2c_master_write_dma()/i2c_slave_set_tx_dma()
 */
void i2c_set_tx_dma_config(dma_chn_e chn)
{
    i2c_dma_tx_chn = chn;
    dma_config(chn, &i2c_tx_dma_config);
}

/**
 * @brief     This function serves to set i2c rx_dma channel and config dma rx default.
 * @param[in] chn: dma channel.
 * @return    none
 * @note      In the case that the DMA transfer is not completed(bit 0 of reg_dma_ctr0(chn): 1-the transmission has not been completed,0-the transmission is completed), re-calling the DMA-related functions may cause problems.
 *            If you must do this, you must perform the following sequence:
 *            1. dma_chn_dis(i2c_dma_tx_chn) 2.i2c_reset() 3.i2c_master_read_dma()/i2c_slave_set_rx_dma()
 */
void i2c_set_rx_dma_config(dma_chn_e chn)
{
    i2c_dma_rx_chn = chn;
    dma_config(chn, &i2c_rx_dma_config);
}








