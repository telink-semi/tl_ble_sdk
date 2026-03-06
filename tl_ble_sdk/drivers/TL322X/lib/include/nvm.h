/********************************************************************************************************
 * @file    nvm.h
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

#include "compiler.h"
#include "gpio.h"
#include "reg_include/nvm_reg.h"
#include "error_handler/error_handler.h"

typedef enum
{
    NVM_READ_DATA_CMD       = 0x0B,
    NVM_READ2_DATA_CMD      = 0x03,
    NVM_WRITE_ENABLE_CMD    = 0x06,
    NVM_WRITE_DISENB_CMD    = 0x04,
    NVM_WRITE_DATA_CMD      = 0x02,
    NVM_READ_SREG_CMD       = 0x05,
    NVM_WRITE_SREG_CMD      = 0x01,
    NVM_ENTER_OTP_CMD       = 0x07,
    NVM_ENTER_MTP_CMD       = 0x17,
    NVM_WRITE_CC_LOCK_CMD   = 0x0A,
    NVM_READ_CC_LOCK_CMD    = 0x1A,
    NVM_WRITE_CR_LOCK_CMD   = 0x0C,
    NVM_READ_CR_LOCK_CMD    = 0x1C,
    NVM_WRITE_MP_CMD        = 0x0D,
    NVM_READ_MP_CMD         = 0x1D,
    NVM_WRITE_CREG_CMD      = 0x10,
    NVM_READ_CREG_CMD       = 0x11,
    NVM_WRITE_TREG_CMD      = 0x18,
    NVM_READ_TREG_CMD       = 0x19,
    NVM_RUN_MBIST_CMD       = 0x1F,
    NVM_WRITE_RAW_DWORD_CMD = 0x12,
} nvm_command_e;

typedef enum
{
    NVM_MP_LOCK_NONE     = 0x00, //000000h-000000h
    NVM_MP_LOCK_LOW_4K   = 0x01, //000000h-000FFFh
    NVM_MP_LOCK_LOW_8K   = 0x02, //000000h-001FFFh
    NVM_MP_LOCK_LOW_16K  = 0x04, //000000h-003FFFh
    NVM_MP_LOCK_LOW_32K  = 0x08, //000000h-007FFFh
    NVM_MP_LOCK_LOW_64K  = 0x10, //000000h-00FFFFh
    NVM_MP_LOCK_LOW_128K = 0x20, //000000h-01FFFFh
    NVM_MP_LOCK_LOW_256K = 0x40, //000000h-03EFFFh
    NVM_MP_LOCK_LOW_512K = 0x80, //000000h-07FFFFh
    NVM_MP_LOCK_ALL_512K = 0x80, //000000h-07FFFFh
} nvm_mp_lock_block_e;

typedef enum
{
    NVM_CR_LOCK_NONE    = 0x00, //000000h-000000h
    NVM_CR_LOCK_0K_4K   = 0x01, //000000h-000FFFh
    NVM_CR_LOCK_4K_8K   = 0x02, //001000h-001FFFh
    NVM_CR_LOCK_8K_12K  = 0x04, //002000h-002FFFh
    NVM_CR_LOCK_12K_16K = 0x08, //003000h-003FFFh
    NVM_CR_LOCK_16K_20K = 0x10, //004000h-004FFFh
    NVM_CR_LOCK_20K_24K = 0x20, //005000h-005FFFh
    NVM_CR_LOCK_ALL_24K = 0x3F, //000000h-005FFFh
} nvm_cr_lock_block_e;

typedef enum
{
    NVM_CC_LOCK_NONE     = 0x0000, //006000h-000000h
    NVM_CC_LOCK_0_63     = 0x0001, //006000h-00603Fh
    NVM_CC_LOCK_64_127   = 0x0002, //006040h-00607Fh
    NVM_CC_LOCK_128_191  = 0x0004, //006080h-0060BFh
    NVM_CC_LOCK_192_255  = 0x0008, //0060C0h-0060FFh
    NVM_CC_LOCK_256_319  = 0x0010, //006100h-00613Fh
    NVM_CC_LOCK_320_383  = 0x0020, //006140h-00617Fh
    NVM_CC_LOCK_384_447  = 0x0040, //006180h-0061BFh
    NVM_CC_LOCK_448_511  = 0x0080, //0061C0h-0061FFh
    NVM_CC_LOCK_512_575  = 0x0100, //006200h-00623Fh
    NVM_CC_LOCK_576_639  = 0x0200, //006240h-00627Fh
    NVM_CC_LOCK_640_703  = 0x0400, //006280h-0062BFh
    NVM_CC_LOCK_704_767  = 0x0800, //0062C0h-0062FFh
    NVM_CC_LOCK_768_831  = 0x1000, //006300h-00633Fh
    NVM_CC_LOCK_832_895  = 0x2000, //006340h-00637Fh
    NVM_CC_LOCK_896_959  = 0x4000, //006380h-0063BFh
    NVM_CC_LOCK_960_1023 = 0x8000, //0063C0h-0063FFh
    NVM_CC_LOCK_ALL_1024 = 0xFFFF, //006000h-0063FFh
} nvm_cc_lock_block_e;

typedef enum
{
    NVM_ERROR_W = BIT(7), //Write Error
    NVM_ERROR_R = BIT(6), //Read Error
    NVM_ERROR_P = BIT(5), //Protection Write Error
    NVM_ERROR_A = BIT(4), //Write Address Error
    NVM_ERROR_M = BIT(3), //MBIST Error
    NVM_OTP     = BIT(2), //OTP Mode
    NVM_WEL     = BIT(1), //Write Enable Latch
    NVM_BUSY    = BIT(0), //Internal Operation Status
} nvm_status_reg_e;

/**
 * @brief       This function servers to set NVM irq mask.
 * @param[in]   mask - the irq mask.
 * @return      none.
 */
static inline void nvm_set_irq_mask(nvm_irq_mask_e mask)
{
    reg_nvm_ctrl |= mask;
}

/**
 * @brief       This function servers to clear NVM irq mask.
 * @param[in]   mask - the irq mask.
 * @return      none.
 */
static inline void nvm_clr_irq_mask(nvm_irq_mask_e mask)
{
    reg_nvm_ctrl &= (~mask);
}

/**
 * @brief       This function servers to clear NVM irq status.
 * @param[in]   status - the irq status.
 * @return      none.
 */
static inline void nvm_clr_irq_status(nvm_irq_status_e status)
{
    reg_nvm_err_irq = (status);
}

/**
 * @brief       This function servers to get NVM irq status.
 * @param[in]   status - the irq status.
 * @retval      non-zero -  the interrupt occurred.
 * @retval      zero  -  the interrupt did not occur.
 */
static inline unsigned char nvm_get_irq_status(nvm_irq_status_e status)
{
    return (reg_nvm_err_irq & status);
}

/**
 * @brief       This function servers to clear NVM irq status.
 * @param[in]   status - the irq status.
 * @return      none.
 */
static inline void nvm_clr_err_status(nvm_err_status_e status)
{
    reg_nvm_err_state = (status);
}

/**
 * @brief       This function servers to get NVM error status.
 * @param[in]   status - the error status mask.
 * @return      the error status.
 */
static inline unsigned char nvm_get_err_status(nvm_err_status_e status)
{
    return (reg_nvm_err_state & status);
}

/**
 * @brief       This function servers to set NVM address 0.
 * @param[in]   addr - address.
 * @return      none.
 */
_attribute_ram_code_sec_ static inline void nvm_reg_set_addr0(unsigned char addr)
{
    reg_nvm_par_ain0 = addr;
}

/**
 * @brief       This function servers to set NVM address.
 * @param[in]   addr - address.
 * @return      none.
 */
_attribute_ram_code_sec_ static inline void nvm_reg_set_addr(unsigned short addr)
{
    reg_nvm_par_ain = addr;
}

/**
 * @brief       This function servers to set NVM length.
 * @param[in]   len - length.
 * @return      none.
 */
_attribute_ram_code_sec_ static inline void nvm_reg_set_len(unsigned short len)
{
    reg_nvm_data_len = len;
}

/**
 * @brief       This function servers to set NVM commend.
 * @param[in]   cmd - commend.
 * @return      none.
 */
_attribute_ram_code_sec_ static inline void nvm_reg_set_cmd(unsigned char cmd)
{
    reg_nvm_par_mode = cmd;
}

/**
 * @brief       This function servers to read the data in NVM using the AHB mode.
 * @param[in]   addr - start address.
 * @return      data.
 */
_attribute_ram_code_sec_ static inline unsigned int nvm_ahb_read_data(unsigned int addr)
{
    return reg_nvm_ahb0_rd_data_word(addr);
}

/**
 * @brief       This function servers to set bypass cache mode.
 * @param[in]   none.
 * @return      none.
 */
_attribute_ram_code_sec_ static inline void nvm_reg_bypass_cache(void)
{
    reg_nvm_ctrl |= FLD_NVM_CIPHER_BYPASS;
}

/**
 * @brief       This function servers to read the data in NVM using the REG mode.
 * @param[in]   none.
 * @return      none.
 */
_attribute_ram_code_sec_ static inline unsigned int nvm_reg_read_data(void)
{
    return reg_nvm_wr_rd_data_word;
}

/**
 * @brief       This function servers to write data using the REG mode.
 * @param[in]   data - data.
 * @return      none.
 */
_attribute_ram_code_sec_ static inline void nvm_reg_write_data(unsigned int data)
{
    reg_nvm_wr_rd_data_word = data;
}

/**
 * @brief       This function servers to write data to NVM register.
 * @param[in]   data - data.
 * @return      none
 */
_attribute_ram_code_sec_ static inline void nvm_reg_write_reg0(unsigned char data)
{
    reg_nvm_wr_rd_reg0 = data;
}

/**
 * @brief       This function servers to write data to NVM register.
 * @param[in]   data - data.
 * @return      none
 */
_attribute_ram_code_sec_ static inline void nvm_reg_write_reg(unsigned short data)
{
    reg_nvm_wr_rd_reg = data;
}

/**
 * @brief       This function servers to read data from NVM register.
 * @param[in]   none.
 * @return      data.
 */
_attribute_ram_code_sec_ static inline unsigned char nvm_reg_read_reg0(void)
{
    return reg_nvm_wr_rd_reg0;
}

/**
 * @brief       This function servers to read data from NVM register.
 * @param[in]   none.
 * @return      data.
 */
_attribute_ram_code_sec_ static inline unsigned short nvm_reg_read_reg(void)
{
    return reg_nvm_wr_rd_reg;
}

/**
 * @brief       This function servers to clear TX FIFO.
 * @param[in]   none.
 * @return      none.
 */
_attribute_ram_code_sec_ static inline void nvm_reg_txfifo_clr(void)
{
    reg_nvm_clr |= FLD_NVM_TXFIFO_CLR;
    while (reg_nvm_state & BIT(2))
        ;
}

/**
 * @brief       This function servers to get error address.
 * @param[in]   none.
 * @return      error address.
 */
_attribute_ram_code_sec_ static inline unsigned short nvm_get_error_addr(void)
{
    return reg_nvm_error_addr_save;
}

/**
 * @brief       This function servers to get error data.
 * @param[in]   none.
 * @return      error data.
 */
_attribute_ram_code_sec_ static inline unsigned int nvm_get_error_data(void)
{
    return reg_nvm_error_ahbslv_rd_word;
}

/**
 * @brief       This function servers to enable NVM spi IO.
 * @param[in]   none.
 * @return      none.
 */
_attribute_ram_code_sec_ static inline void nvm_spi_io_enable(void)
{
    reg_nvm_ctrl0 = 0x00;
}

/**
 * @brief       This function servers to disable NVM spi IO.
 * @param[in]   none.
 * @return      none.
 */
_attribute_ram_code_sec_ static inline void nvm_spi_io_disable(void)
{
    reg_nvm_ctrl0 = 0x01;
}

/**
 * @brief       This function servers to enable NVM spi mode.
 * @param[in]   none.
 * @return      none.
 */
_attribute_ram_code_sec_ static inline void nvm_spi_enable(void)
{
    reg_nvm_spi_st = FLD_NVM_SPI_ENABLE;
}

/**
 * @brief       This function servers to disable NVM spi mode.
 * @param[in]   none.
 * @return      none.
 */
_attribute_ram_code_sec_ static inline void nvm_spi_disable(void)
{
    reg_nvm_spi_st = 0x00;
}

/**
 * @brief       This function servers to set the NVM configuration using the SPI mode.
 * @param[in]   cfg - the NVM configuration.
 * @return      none.
 */
_attribute_ram_code_sec_ static inline void nvm_spi_set_cfg(unsigned char cfg)
{
    reg_nvm_spi_cfg = cfg;
}

/**
 * @brief       This function servers to write the NVM data using the SPI mode.
 * @param[in]   data - data.
 * @return      none.
 */
_attribute_ram_code_sec_ static inline void nvm_spi_write_data(unsigned char data)
{
    reg_nvm_spi_wdata = data;
}

/**
 * @brief       This function servers to read the NVM data using the SPI mode.
 * @param[in]   none.
 * @return      data.
 */
_attribute_ram_code_sec_ static inline unsigned char nvm_spi_read_data(void)
{
    return reg_nvm_spi_rdata;
}

/**
 * @brief       This function servers to set core0 space.
 * @param[in]   space - space.
 * @return      none
 */
_attribute_ram_code_sec_ static inline void nvm_set_core0_space(unsigned char space)
{
    reg_nvm_core0_space_start  = 0;
    reg_nvm_core0_space_size   = space - 1;
    reg_nvm_core0_space_offset = space;
}

/**
 * @brief       This function servers to set core1 space.
 * @param[in]   space - space.
 * @return      none
 */
_attribute_ram_code_sec_ static inline void nvm_set_core1_space(unsigned char space)
{
    reg_nvm_core1_space_start  = 0;
    reg_nvm_core1_space_size   = space - 1;
    reg_nvm_core1_space_offset = space;
}

/**
 * @brief       This function is used to access the OTP function of the NVM.
 * @param[in]   none.
 * @return      none.
 */
void nvm_enter_otp(void);

/**
 * @brief       This function is used to access the MTP function of the NVM.
 * @param[in]   none.
 * @return      none.
 */
void nvm_enter_mtp(void);

/**
 * @brief       This function is used to enable the NVM write function.
 * @param[in]   none.
 * @return      none.
 */
void nvm_write_enable(void);

/**
 * @brief       This function is used to disable the NVM write function.
 * @param[in]   none.
 * @return      none.
 */
void nvm_write_disable(void);

/**
 * @brief       This function is used to read the registers in the NVM.
 * @param[in]   cmd - Different registers have different commands.
 * @return      none.
 */
unsigned short nvm_read_reg(unsigned char cmd);

/**
 * @brief       This function is used to write the registers in the NVM.
 * @param[in]   cmd - Different registers have different commands.
 * @return      none.
 */
void nvm_write_reg(unsigned char cmd, unsigned short data);

/**
 * @brief       This function is used to read the data in NVM using the AHB0 mode.
 * @param[in]   addr - the start address.
 * @param[in]   len - data length.
 * @param[in]   data - data pointer.
 * @return      none.
 */
void nvm_ahb0_read_word(unsigned int addr, unsigned int len, unsigned int *data);

/**
 * @brief       This function is used to read the data in NVM using the AHB0 mode.
 * @param[in]   addr - the start address.
 * @param[in]   len - data length.
 * @param[in]   data - data pointer.
 * @return      none.
 */
void nvm_ahb0_read_byte(unsigned int addr, unsigned int len, unsigned char *data);

/**
 * @brief       This function is used to read the data in NVM using the AHB1 mode.
 * @param[in]   addr - the start address.
 * @param[in]   len - data length.
 * @param[in]   data - data pointer.
 * @return      none.
 */
void nvm_ahb1_read_word(unsigned int addr, unsigned int len, unsigned int *data);

/**
 * @brief       This function is used to read the data in NVM using the AHB1 mode.
 * @param[in]   addr - the start address.
 * @param[in]   len - data length.
 * @param[in]   data - data pointer.
 * @return      none.
 */
void nvm_ahb1_read_byte(unsigned int addr, unsigned int len, unsigned char *data);

/**
 * @brief       This function is used to read the data in NVM using the REG mode.
 * @param[in]   addr - the start address.
 * @param[in]   len - data length.
 * @param[in]   data - data pointer.
 * @return      none.
 */
void nvm_reg_read(unsigned int addr, unsigned int len, unsigned int *data);

/**
 * @brief       This function is used to write the data to NVM using the REG mode.
 * @param[in]   addr - the start address.
 * @param[in]   len - data length.
 * @param[in]   data - data pointer.
 * @return      none.
 */
void nvm_reg_write(unsigned int addr, unsigned int len, unsigned int *data);


/**
 * @brief       This function is used to read the creg data in NVM.
 * @param[in]   addr - the start address.
 * @return      creg data.
 */
unsigned char nvm_read_creg(unsigned char addr);


/**
 * @brief       This function is used to write the creg data in NVM.
 * @param[in]   addr - the start address.
 * @param[in]   data - creg data.
 * @return      none.
 */
void nvm_write_creg(unsigned char addr, unsigned char data);

