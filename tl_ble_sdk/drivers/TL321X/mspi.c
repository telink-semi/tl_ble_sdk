/********************************************************************************************************
 * @file    mspi.c
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
#include "mspi.h"
#include "clock.h"


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

