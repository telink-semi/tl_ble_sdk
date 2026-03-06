/********************************************************************************************************
 * @file    nvm_reg.h
 *
 * @brief   This is the header file for tl322x
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
#pragma once

#include "soc.h"


#define NVM_AHB0_BASE_ADDR           0x00400000

#define reg_nvm_ahb0_rd_data_byte(j) REG_ADDR8(NVM_AHB0_BASE_ADDR + j)

#define reg_nvm_ahb0_rd_data_word(j) REG_ADDR32(NVM_AHB0_BASE_ADDR + j)

#define NVM_AHB1_BASE_ADDR           0x00500000

#define reg_nvm_ahb1_rd_data_byte(j) REG_ADDR8(NVM_AHB1_BASE_ADDR + j)

#define reg_nvm_ahb1_rd_data_word(j) REG_ADDR32(NVM_AHB1_BASE_ADDR + j)


#define NVM_BASE_ADDR               0x80101000

/**
 * BIT[0:7]   data0[7:0] to transmit or received.
 */
#define reg_nvm_wr_rd_data0 REG_ADDR8(NVM_BASE_ADDR + 0x00)

/**
 * BIT[0:7]   data1[7:0] to transmit or received.
 */
#define reg_nvm_wr_rd_data1 REG_ADDR8(NVM_BASE_ADDR + 0x01)

/**
 * BIT[0:7]   data2[7:0] to transmit or received.
 */
#define reg_nvm_wr_rd_data2 REG_ADDR8(NVM_BASE_ADDR + 0x02)

/**
 * BIT[0:7]   data3[7:0] to transmit or received.
 */
#define reg_nvm_wr_rd_data3           REG_ADDR8(NVM_BASE_ADDR + 0x03)

#define reg_nvm_wr_rd_data(j)         REG_ADDR8(NVM_BASE_ADDR + 0x00 + (j))

#define reg_nvm_wr_rd_data_word       REG_ADDR32(NVM_BASE_ADDR + 0x00)

#define reg_nvm_wr_rd_data_adr        (NVM_BASE_ADDR + 0x00)

#define reg_nvm_error_ahbslv_rdata(j) REG_ADDR8(NVM_BASE_ADDR + 0x04 + (j))

#define reg_nvm_error_ahbslv_rd_word  REG_ADDR32(NVM_BASE_ADDR + 0x04)

#define reg_nvm_error_addr_save0      REG_ADDR8(NVM_BASE_ADDR + 0x08)

#define reg_nvm_error_addr_save1      REG_ADDR8(NVM_BASE_ADDR + 0x09)

#define reg_nvm_error_addr_save       REG_ADDR16(NVM_BASE_ADDR + 0x08)

#define reg_nvm_core0_space_start     REG_ADDR8(NVM_BASE_ADDR + 0x0a)

#define reg_nvm_core0_space_size      REG_ADDR8(NVM_BASE_ADDR + 0x0b)

#define reg_nvm_core0_space_offset    REG_ADDR8(NVM_BASE_ADDR + 0x0c)

#define reg_nvm_core1_space_start     REG_ADDR8(NVM_BASE_ADDR + 0x0d)

#define reg_nvm_core1_space_size      REG_ADDR8(NVM_BASE_ADDR + 0x0e)

#define reg_nvm_core1_space_offset    REG_ADDR8(NVM_BASE_ADDR + 0x0f)

#define reg_nvm_par_ain0              REG_ADDR8(NVM_BASE_ADDR + 0x10)

#define reg_nvm_par_ain1              REG_ADDR8(NVM_BASE_ADDR + 0x11)

#define reg_nvm_par_ain               REG_ADDR16(NVM_BASE_ADDR + 0x10)

#define reg_nvm_par_dout_dly          REG_ADDR8(NVM_BASE_ADDR + 0x12)

#define reg_nvm_par_mode              REG_ADDR8(NVM_BASE_ADDR + 0x13)

enum
{
    FLD_NVM_PAR_MODE = BIT_RNG(0, 4),
};

#define reg_nvm_data_len0  REG_ADDR8(NVM_BASE_ADDR + 0x14)

#define reg_nvm_data_len1  REG_ADDR8(NVM_BASE_ADDR + 0x15)

#define reg_nvm_data_len   REG_ADDR16(NVM_BASE_ADDR + 0x14)

#define reg_nvm_wr_rd_reg0 REG_ADDR8(NVM_BASE_ADDR + 0x16)

#define reg_nvm_wr_rd_reg1 REG_ADDR8(NVM_BASE_ADDR + 0x17)

#define reg_nvm_wr_rd_reg  REG_ADDR16(NVM_BASE_ADDR + 0x16)

#define reg_nvm_ctrl       REG_ADDR8(NVM_BASE_ADDR + 0x18)

enum
{
    FLD_NVM_CIPHER_BYPASS        = BIT(0),
    FLD_NVM_AHBSLV_ERR_READY_H_L = BIT(1),
    FLD_NVM_PAR_CLK_GATE_EN      = BIT(2),
    //    FLD_NVM_ERR_IRQ_MASK               = BIT(3),
    //    FLD_NVM_BUSY_TIMEOUT_IRQ_MASK      = BIT(4),
};

typedef enum
{
    FLD_NVM_ERR_IRQ_MASK          = BIT(3),
    FLD_NVM_BUSY_TIMEOUT_IRQ_MASK = BIT(4),
} nvm_irq_mask_e;

#define reg_nvm_clr REG_ADDR8(NVM_BASE_ADDR + 0x1a)

enum
{
    FLD_NVM_TXFIFO_CLR   = BIT(0),
    FLD_NVM_RXFIFO_CLR   = BIT(1),
    FLD_NVM_U0_CACHE_CLR = BIT(2),
    FLD_NVM_U1_CACHE_CLR = BIT(3),
};

#define reg_nvm_err_irq REG_ADDR8(NVM_BASE_ADDR + 0x1b)

typedef enum
{
    FLD_NVM_BUSY_TIMEOUT_IRQ_CLR = BIT(0),
} nvm_irq_status_e;

#define reg_nvm_state REG_ADDR8(NVM_BASE_ADDR + 0x1c)

enum
{
    FLD_NVM_CTRL_BUSY     = BIT(0),
    FLD_NVM_NVM_BUSY     = BIT(1),
    FLD_NVM_CLR_BUSY      = BIT(2),
    FLD_NVM_TX_READY      = BIT(3),
    FLD_NVM_RX_READY      = BIT(4),
    FLD_NVM_RXDMA_CYC     = BIT(5),
    FLD_NVM_READREG_READY = BIT(6),
    FLD_NVM_TXDMA_CYC     = BIT(7),
};

#define reg_nvm_err_state REG_ADDR8(NVM_BASE_ADDR + 0x1d)

typedef enum
{
    FLD_NVM_ERR_R = BIT(0),
    FLD_NVM_OTHER = BIT(1),
} nvm_err_status_e;

#define reg_nvm_busy_time_lim0 REG_ADDR8(NVM_BASE_ADDR + 0x1e)

#define reg_nvm_busy_time_lim1 REG_ADDR8(NVM_BASE_ADDR + 0x1f)

#define reg_nvm_busy_time_lim  REG_ADDR16(NVM_BASE_ADDR + 0x1e)

#define reg_nvm_spi_st         REG_ADDR8(NVM_BASE_ADDR + 0x20)

enum
{
    FLD_NVM_SPI_ENABLE = BIT(0),
    FLD_NVM_SPI_BUSY   = BIT(1),
};

#define reg_nvm_spi_cfg REG_ADDR8(NVM_BASE_ADDR + 0x21)

enum
{
    FLD_NVM_SPI_DIV_CLK_CFG = BIT_RNG(0, 1),
    FLD_NVM_SPI_CNT         = BIT_RNG(2, 6),
    FLD_NVM_SPI_TX_DONE     = BIT(7),
};

#define reg_nvm_spi_wdata   REG_ADDR8(NVM_BASE_ADDR + 0x22)

#define reg_nvm_spi_rdata   REG_ADDR8(NVM_BASE_ADDR + 0x24)

#define reg_nvm_spi_read_st REG_ADDR8(NVM_BASE_ADDR + 0x25)

enum
{
    FLD_NVM_SPI_RDATA_VALID = BIT(0),
    FLD_NVM_SPI_RX_CNT      = BIT_RNG(1, 5),
};
