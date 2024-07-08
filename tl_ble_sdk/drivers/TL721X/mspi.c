/********************************************************************************************************
 * @file    mspi.c
 *
 * @brief   This is the source file for TL721X
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
#include "mspi.h"
#include "clock.h"

unsigned int g_slave_base_addr[4] = {0x00000000, 0x00000000, 0x00000000, 0x00000000};


/**
 * @brief       This function servers to write mspi fifo.
 * @param[in]   data    - the pointer to the data for write.
 * @param[in]   len     - write length.
 * @return      none
 */
_attribute_ram_code_sec_noinline_  void mspi_write(unsigned char *data, unsigned int len)
{
    for (unsigned int i = 0; i < len; i++)
    {
        //while (reg_mspi_txfifo_status & FLD_MSPI_TXFIFO_FULL);    //do not need software to block,use hardware block mcu.
        reg_mspi_wr_rd_data(i % 4) = data[i];
    }
    mspi_wait();
}

/**
 * @brief       This function servers to read mspi fifo.
 * @param[in]   data    - the pointer to the data for read.
 * @param[in]   len     - write length.
 * @return      none
 */
_attribute_ram_code_sec_noinline_  void mspi_read(unsigned char *data, unsigned int len)
{
    for (unsigned int i = 0; i < len; i++)
    {
//      while (reg_mspi_rxfifo_status & FLD_MSPI_RXFIFO_EMPTY);   //do not need software to block,use hardware block mcu.
        data[i] = reg_mspi_wr_rd_data(i % 4);
    }
    mspi_wait();
}

/**
 * @brief       This function is used to allocate address space to multiple mspi slaves, the chip can allocate address space is 64MB,
 *              can connect up to 4 slave devices,slave0 address space size can not be 0.
 *              The chip's security boot feature supports slave0 only, and only the first 16MB.
 *              If this function is not called, the default chip is, only slave0, and slave0 has an accessible address space of 64MB.
 * @param[in]   slave0_size - the range is 1~4.
 * @param[in]   slave1_size - the range is 0~3.
 * @param[in]   slave2_size - the range is 0~3.
 * @param[in]   slave3_size - the range is 0~3.
 * @return      0: The address is incorrectly assigned. 1: The address is assigned successfully.
 */
unsigned char mspi_slave_device_addr_space_config(mspi_slave_size_e slave0_size, mspi_slave_size_e slave1_size, mspi_slave_size_e slave2_size, mspi_slave_size_e slave3_size)
{
    if((slave0_size + slave1_size + slave2_size + slave3_size) > 4)
    {
        return 0;
    }
    //The middle device can also not allocate the address space, because when the chip actually accesses the address,
    //it will judge according to the following logic, if the following logic is not met, it will not access.
    //  slave0 address[24-25] <= FLD_MSPI_XIP0_END_ADDR.
    //  slave1 address[24-25] > FLD_MSPI_XIP0_END_ADDR && <= FLD_MSPI_XIP1_END_ADDR.
    //  slave2 address[24-25] > FLD_MSPI_XIP1_END_ADDR && <= FLD_MSPI_XIP2_END_ADDR.
    //  slave3 address[24-25] > FLD_MSPI_XIP2_END_ADDR && <= FLD_MSPI_XIP3_END_ADDR.
    //The address space of each slave is:
    //  slave0 space = {0,                    , (xip0_end_addr+1) * 16m}.
    //  slave1 space = {xip0_end_addr+1) * 16m, (xip1_end_addr+1) * 16m}.
    //  slave2 space = {xip1_end_addr+1) * 16m, (xip2_end_addr+1) * 16m}.
    //  slave3 space = {xip2_end_addr+1) * 16m, (xip3_end_addr+1) * 16m}.

    reg_mspi_xip_size_set = (((slave0_size + slave1_size + slave2_size + slave3_size - 1) << 6) & FLD_MSPI_XIP3_END_ADDR)\
            | (((slave0_size + slave1_size + slave2_size - 1) << 4) & FLD_MSPI_XIP2_END_ADDR)\
            | (((slave0_size + slave1_size - 1) << 2) & FLD_MSPI_XIP1_END_ADDR) | ((slave0_size - 1) & FLD_MSPI_XIP0_END_ADDR);

    if(slave1_size == SLAVE_SIZE_NONE){
        g_slave_base_addr[1] = 0x00000000;
    }else{
        g_slave_base_addr[1] = slave0_size << 24;
    }
    if(slave2_size == SLAVE_SIZE_NONE){
        g_slave_base_addr[2] = 0x00000000;
    }else{
        g_slave_base_addr[2] = (slave0_size + slave1_size) << 24;
    }
    if(slave3_size == SLAVE_SIZE_NONE){
        g_slave_base_addr[3] = 0x00000000;
    }else{
        g_slave_base_addr[3] = (slave0_size + slave1_size + slave2_size) << 24;
    }

    return 1;
}

