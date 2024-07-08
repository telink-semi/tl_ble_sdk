/********************************************************************************************************
 * @file    analog.c
 *
 * @brief   This is the source file for TL321X
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
#include "lib/include/plic.h"
#include "analog.h"
#include "compiler.h"
#include "stimer.h"
/**********************************************************************************************************************
 *                                            local constants                                                       *
 *********************************************************************************************************************/


/**********************************************************************************************************************
 *                                              local macro                                                        *
 *********************************************************************************************************************/


/**********************************************************************************************************************
 *                                             local data type                                                     *
 *********************************************************************************************************************/


/**********************************************************************************************************************
 *                                              global variable                                                       *
 *********************************************************************************************************************/


/**********************************************************************************************************************
 *                                              local variable                                                     *
 *********************************************************************************************************************/
/**********************************************************************************************************************
 *                                          local function prototype                                               *
 *********************************************************************************************************************/

/**
 * @brief      This function serves to judge whether analog is busy.
 * @return     0: not busy  1:busy
 */
_attribute_ram_code_sec_noinline_ bool analog_busy(void)
{
    return reg_ana_ctrl & FLD_ANA_BUSY;
}

/**
 * @brief      This function serves to judge whether analog Tx buffer is empty.
 * @return     0:not empty      1: empty
 */
_attribute_ram_code_sec_noinline_ bool analog_txbuf_no_empty(void)
{
/**
    Because the logic of the current chip hardware to write the first data is:
    regardless of whether there is data in the FIFO, the write operation will be performed immediately.
    In order to prevent incorrect data from being emitted due to the slow filling of the FIFO,
    we need to determine whether the FIFO is non-empty before triggering the write action.
*/
    return (!(reg_ana_buf_cnt & FLD_ANA_TX_BUFCNT));
}

/**********************************************************************************************************************
 *                                         global function implementation                                             *
 *********************************************************************************************************************/





/**
 * @brief      This function serves to analog register read by byte.
 * @param[in]  addr - address need to be read.
 * @return     the result of read.
 */
_attribute_ram_code_sec_noinline_ unsigned char analog_read_reg8(unsigned char addr){
    unsigned int r=core_interrupt_disable();
    reg_ana_addr = addr;
    reg_ana_len=0x1;
    reg_ana_ctrl = FLD_ANA_CYC;
    analog_wait();
    unsigned char data= reg_ana_data(0);
    core_restore_interrupt(r);
    return data;

}

/**
 * @brief      This function serves to analog register write by byte.
 * @param[in]  addr - address need to be write.
 * @param[in]  data - the value need to be write.
 * @return     none.
 */
_attribute_ram_code_sec_noinline_ void analog_write_reg8(unsigned char addr, unsigned char data){
    unsigned int r=core_interrupt_disable();
    reg_ana_len = 1;
    reg_ana_addr = addr;
    reg_ana_data(0) = data;
    analog_wait_txbuf_no_empty();
    reg_ana_ctrl = (FLD_ANA_CYC | FLD_ANA_RW);
    analog_wait();
    reg_ana_ctrl =0x00;
    core_restore_interrupt(r);
}

/**
 * @brief      This function serves to analog register write by halfword.
 * @param[in]  addr - address need to be write.
 * @param[in]  data - the value need to be write.
 * @return     none.
 */
_attribute_ram_code_sec_noinline_ void analog_write_reg16(unsigned char addr, unsigned short data)
{
    unsigned int r=core_interrupt_disable();
    reg_ana_len=2;
    reg_ana_addr = addr;
    reg_ana_addr_data16 = data;
    analog_wait_txbuf_no_empty();
    reg_ana_ctrl = (FLD_ANA_CYC | FLD_ANA_RW);
    analog_wait();
    reg_ana_ctrl =0x00;
    core_restore_interrupt(r);
}

/**
 * @brief      This function serves to analog register read by halfword.
 * @param[in]  addr - address need to be read.
 * @return     the result of read.
 */
_attribute_ram_code_sec_noinline_ unsigned short analog_read_reg16(unsigned char addr)
{
    unsigned int r=core_interrupt_disable();
    reg_ana_len=2;
    reg_ana_addr = addr;
    reg_ana_ctrl = FLD_ANA_CYC;
    analog_wait();
    unsigned short data=reg_ana_addr_data16;
    core_restore_interrupt(r);
    return data;
}



/**
 * @brief      This function serves to analog register read by word.
 * @param[in]  addr - address need to be read.
 * @return     the result of read.
 */
_attribute_ram_code_sec_noinline_ unsigned int analog_read_reg32(unsigned char addr)
{
    unsigned int r=core_interrupt_disable();
    reg_ana_len = 4;
    reg_ana_addr = addr;
    reg_ana_ctrl = FLD_ANA_CYC;
    analog_wait();
    unsigned int data=reg_ana_addr_data32;
    core_restore_interrupt(r);
    return data;
}


/**
 * @brief      This function serves to analog register write by word.
 * @param[in]  addr - address need to be write.
 * @param[in]  data - the value need to be write.
 * @return     none.
 */
_attribute_ram_code_sec_noinline_ void analog_write_reg32(unsigned char addr, unsigned int data)
{
    unsigned int r=core_interrupt_disable();
    reg_ana_len = 4;
    reg_ana_addr = addr;
    reg_ana_addr_data32 = data;
    analog_wait_txbuf_no_empty();
    reg_ana_ctrl = (FLD_ANA_CYC | FLD_ANA_RW);
    analog_wait();
    reg_ana_ctrl =0x00;
    core_restore_interrupt(r);
}

/**
 * @brief      This function write buffer to analog register.
 * @param[in]  addr - address need to be write.
 * @param[in]  *buff - the buffer need to be write.
 * @param[in]  len - the length of buffer(The data length cannot be greater than 8).
 * @return     none.
 */
_attribute_ram_code_sec_noinline_ void analog_write_buff(unsigned char addr, unsigned char *buff, unsigned char len)
{
    if(len > 8){
        return;
    }
    unsigned int r=core_interrupt_disable();

    reg_ana_len = len;
    reg_ana_addr = addr;
    reg_ana_data(0) = *(buff++);
    analog_wait_txbuf_no_empty();
    reg_ana_ctrl = (FLD_ANA_CYC | FLD_ANA_RW);
    for(unsigned int i=1; i<len; i++)
    {
        reg_ana_data(i % 4) = *(buff++);
    }

    analog_wait();
    reg_ana_ctrl =0x00;
    core_restore_interrupt(r);
}

/**
 * @brief      This function read data from analog register to buffer.
 * @param[in]  addr - address need to be read from.
 * @param[in]  *buff - the buffer need to be put data.
 * @param[in]  len - the length of read data.
 * @return     none.
 */
_attribute_ram_code_sec_noinline_ void analog_read_buff(unsigned char addr, unsigned char *buff, unsigned char len)
{
    unsigned int r=core_interrupt_disable();

    reg_ana_len = len;
    reg_ana_addr = addr;
    reg_ana_ctrl = FLD_ANA_CYC;
    for(unsigned int i=0; i<len; i++)
    {
        while(!(reg_ana_buf_cnt & FLD_ANA_RX_BUFCNT));
        *(buff++) = reg_ana_data(i % 4);
    }

    analog_wait();
    reg_ana_ctrl =0x00;
    core_restore_interrupt(r);
}

