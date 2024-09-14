/********************************************************************************************************
 * @file    cstartup_B92.S
 *
 * @brief   This is the boot file for BLE SDK
 *
 * @author  BLE GROUP
 * @date    06,2022
 *
 * @par     Copyright (c) 2022, Telink Semiconductor (Shanghai) Co., Ltd. ("TELINK")
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
#include "user_config.h"

#ifndef __CHANNEL_SOUNDING_EN__
#define __CHANNEL_SOUNDING_EN__     0
#endif

#ifndef __LE_AUDIO_EN__
#define __LE_AUDIO_EN__             0
#endif

 #if 1
 /**
 * @brief   Set "DP_THROUGH_SWIRE_DIS" as "1" to disable the "dp_through_swire" function. Set "DP_THROUGH_SWIRE_DIS" as "0" to enable the "dp_through_swire" function.
 *          Risk description:
 *          The "dp_through_swire" function is enabled by default.When DP and DM pin are used as GPIO function and trigger "dp_through_swire" timing,
 *          there may be a problem of miswriting the chip through swire to cause the chip to crash. Therefore, disable the "dp_through_swire" function in this startup file.(set "DP_THROUGH_SWIRE_DIS" as "1").
 *          If the chip has only dp pin but no sws pin, using the above scheme (set "DP_THROUGH_SWIRE_DIS" as "1") will cause the following problems.
 *          1. During fixture production, burning a firmware program that disables the "dp_through_swire"function into unprogrammed chip will cause the problem that chip can not be burned again due to swire communication failure.
 *          2. Burning a firmware program that disables the "dp_through_swire" function with BDT tool may result in failure of debugging due to swire communication failure.
 *          If developers evaluate that the above scheme (set "DP_THROUGH_SWIRE_DIS" as "1") cannot be used, they can only set "DP_THROUGH_SWIRE_DIS" to "0" to enable the "dp_through_swire" function. However, be careful not to make DP and DM pin trigger "dp_through_swire" timing.
 */
#ifndef DP_THROUGH_SWIRE_DIS
#define DP_THROUGH_SWIRE_DIS      1
#endif

//The default configuration is 128K to ensure it can work properly on all models.
//In actual use, please configure the sram size according to your own model, otherwise the efficiency will be affected.
//the most efficient use is:
//IRAM is used to store code, DRAM is used to store data, if the data-related segments are stored in IRAM, the efficiency will be affected.
#define SRAM_128K      1    //128KSRAM, 128KB-IRAM(96KB-Retention)/0KB-DRAM
#define SRAM_256K      2    //256KSRAM, 256KB-IRAM(96KB-Retention)/0KB-DRAM
#define SRAM_384K      3    //384KSRAM, 256KB-IRAM(96KB-Retention)/128KB-DRAM
#define SRAM_512K      4    //512KSRAM, 256KB-IRAM(96KB-Retention)/256KB-DRAM
#if(__CHANNEL_SOUNDING_EN__)
    #define SRAM_SIZE      SRAM_512K
#elif(__LE_AUDIO_EN__)
    #define SRAM_SIZE      SRAM_512K
#else
    #define SRAM_SIZE      SRAM_128K
#endif

#if (SRAM_SIZE == SRAM_256K)
    .equ __IRAM_2_EN,   1
    .equ __DRAM_1_EN,   0
    .equ __DRAM_2_EN,   0
    .equ __DRAM_DIS,    1
#elif (SRAM_SIZE == SRAM_384K)
    .equ __IRAM_2_EN,   1
    .equ __DRAM_1_EN,   1
    .equ __DRAM_2_EN,   0
    .equ __DRAM_DIS,    0
#elif (SRAM_SIZE == SRAM_512K)
    .equ __IRAM_2_EN,   1
    .equ __DRAM_1_EN,   1
    .equ __DRAM_2_EN,   1
    .equ __DRAM_DIS,    0
#else
    .equ __IRAM_2_EN,   0
    .equ __DRAM_1_EN,   0
    .equ __DRAM_2_EN,   0
    .equ __DRAM_DIS,    1
#endif

#ifdef __BOOT_SWITCH_APP1__
    .equ __BOOT_SWITCH_FLASH_ADDR, 0x200EA000
#else
    .equ __BOOT_SWITCH_FLASH_ADDR, 0x20000000
#endif

    //.equ __DEEPRET_RAM_SIZE,  PM_DEEPSLEEP_RETENTION_SIZE

    .section .vectors, "ax"
    .global     __IRAM_2_EN
    .global     __DRAM_1_EN
    .global     __DRAM_2_EN
    .global     __DRAM_DIS
    .global     __BOOT_SWITCH_FLASH_ADDR
    //.global       __DEEPRET_RAM_SIZE
    //.org  and linker's relaxation (-flto) cannot be used at the same time
    //Pop corresponds to push. Before using .option norelax, use push to save the current .option configuration
    //and then modify .option. After using norelax, use pop to restore
    .option push
    .option norelax
    .org 0x0


    .global _RESET_ENTRY
    .type _RESET_ENTRY,@function

    .align 2
_RESET_ENTRY:
    j        _START

/* used to check flash protection function consistency between the old and new  firmware when OTA upgrading */
#if APP_FLASH_PROTECTION_ENABLE
    .org 0x15    //correspond to "FIRMWARE_FLASH_PROTECTION_FLAG_ADDR" in mcu_boot.h
    .byte (0x5A)
#endif

/* used to check firmware encryption function consistency between the old and new  firmware when OTA upgrading */
#if APP_HW_FIRMWARE_ENCRYPTION_ENABLE
    .org 0x16    //correspond to "FIRMWARE_ENCRYPTION_FLAG_ADDR" in mcu_boot.h
    .byte (0x5A)
#endif

/* used to check secure boot function consistency between the old and new  firmware when OTA upgrading */
#if APP_HW_SECURE_BOOT_ENABLE
    .org 0x17        //correspond to "SECURE_BOOT_FLAG_ADDR" in mcu_boot.h
    .byte (0x5A)
 #endif

    //free the 6th ~ 7th byte to store the crc type of the bin file
    .org 0x18
    .word (BIN_SIZE)

    .org 0x20
    .word ('T'<<24 | 'L'<<16 | 'N'<<8 | 'K')

        .org 0x24
#if 1
    //channel sounding project need to set 1 to enable 2-wire mode locally. other projects not care temporally.
    .word (0x3B0097A9)             //DREAD:   cmd:1x, addr:1x, data:2x, dummy:8
#else
    //.word (0xBB0093B9)             //X2READ:  cmd:1x, addr:2x, data:2x, dummy:4
    //.word (0x6B0097AA)             //QREAD:   cmd:1x, addr:1x, data:4x, dummy:8
    .word (0xEB0095BA)               //X4READ:  cmd:1x, addr:4x, data:4x, dummy:6
#endif

    .option pop
    .align 2

_START:

#if 0
    // add debug, PB4 output 1
    lui     t0, 0x80140
    li      t1, 0xef
    li      t2, 0x10
    sb      t1, 0x30a(t0)        //0x8014030a  PB oen     =  0xef
    sb      t2, 0x30b(t0)        //0x8014030b  PB output  =  0x10
#endif

    /* Initialize global pointer */
    .option push
    .option norelax
    la     gp, __global_pointer$
    .option pop
    
#if DP_THROUGH_SWIRE_DIS
    lui     t0, 0x80100
    addi    t0 , t0 , 0x700
    li      t1, 0x40
    sb      t1, 0x501(t0)  //0x80100c01 -> 0x40  <7>: 0 swire_usb_dis <6>:read-only bit
#endif

    /* Initialize stack pointer */
    la     t0, _STACK_TOP
    mv     sp, t0

#ifdef __nds_execit
    /* Initialize EXEC.IT table */
    la t0, _ITB_BASE_
    csrw uitb, t0
#endif

#ifdef __riscv_flen
    /* Enable FPU */
    li t0, 0x00006000
    csrrs t0, mstatus, t0
    /* Initialize FCSR */
    fscsr zero
#endif

    /* Initial machine trap-vector Base */
    la     t0, __vectors
    csrw   mtvec, t0

    /* Enable vectored external plic interrupt */
    csrsi  mmisc_ctl, 2

    /*vector mode enable bit (VECTORED) of the Feature Enable Register */
    lui     t0, 0xe4000
    li      t1, 0x02
    sw      t1, 0x0(t0)      //(*(volatile unsigned long*)(0xe4000000))= 0x02

    /* Enable I/D-Cache */
    csrr   t0,  mcache_ctl
    ori    t0,  t0,  1  #/I-Cache
    ori    t0,  t0,  2  #/D-Cache
    csrw   mcache_ctl,  t0
    fence.i

    /* Move retention reset from flash to sram */
_RETENTION_RESET_INIT:
    la     t1, _RETENTION_RESET_LMA_START
    la     t2, _RETENTION_RESET_VMA_START
    la     t3, _RETENTION_RESET_VMA_END
_RETENTION_RESET_BEGIN:
    bleu    t3, t2, _RETENTION_DATA_INIT
    lw     t0, 0(t1)
    sw     t0, 0(t2)
    addi   t1, t1, 4
    addi   t2, t2, 4
    j      _RETENTION_RESET_BEGIN

    /* Move retention from flash to sram */
_RETENTION_DATA_INIT:
    la     t1, _RETENTION_DATA_LMA_START
    la     t2, _RETENTION_DATA_VMA_START
    la     t3, _RETENTION_DATA_VMA_END
_RETENTION_DATA_INIT_BEGIN:
    bleu   t3, t2, _RAMCODE_INIT
    lw     t0, 0(t1)
    sw     t0, 0(t2)
    addi   t1, t1, 4
    addi   t2, t2, 4
    j      _RETENTION_DATA_INIT_BEGIN

    /* Move ramcode from flash to sram */
_RAMCODE_INIT:
    la     t1, _RAMCODE_LMA_START
    la     t2, _RAMCODE_VMA_START
    la     t3, _RAMCODE_VMA_END
_RAMCODE_INIT_BEGIN:
    bleu   t3, t2, _DATA_INIT
    lw     t0, 0(t1)
    sw     t0, 0(t2)
    addi   t1, t1, 4
    addi   t2, t2, 4
    j      _RAMCODE_INIT_BEGIN

    /* Move Data from flash to sram */
_DATA_INIT:
    la     t1, _DATA_LMA_START
    la     t2, _DATA_VMA_START
    la     t3, _DATA_VMA_END
_DATA_INIT_BEGIN:
    bleu   t3, t2, _ZERO_BSS
    lw     t0, 0(t1)
    sw     t0, 0(t2)
    addi   t1, t1, 4
    addi   t2, t2, 4
    j      _DATA_INIT_BEGIN

    /* Zero .bss section in sram */
_ZERO_BSS:
    lui    t0, 0
    la     t2, _BSS_VMA_START
    la     t3, _BSS_VMA_END
_ZERO_BSS_BEGIN:
    bleu   t3, t2, _ZERO_AES
    sw     t0, 0(t2)
    addi   t2, t2, 4
    j      _ZERO_BSS_BEGIN

    /* Zero .aes section in sram */
_ZERO_AES:
    lui    t0, 0
    la     t2, _AES_VMA_START
    la     t3, _AES_VMA_END
_ZERO_AES_BEGIN:
    bleu   t3, t2, _ZERO_IRAM_BSS
    sw     t0, 0(t2)
    addi   t2, t2, 4
    j      _ZERO_AES_BEGIN
    
        /* Zero .iram_bss section in sram */
_ZERO_IRAM_BSS:
    lui    t0, 0
    la     t2, _IRAM_BSS_VMA_START
    la     t3, _IRAM_BSS_VMA_END
_ZERO_IRAM_BSS_BEGIN:
    bleu   t3, t2, _FILL_STK
    sw     t0, 0(t2)
    addi   t2, t2, 4
    j      _ZERO_IRAM_BSS_BEGIN

    /* Fill the remaining section in sram */
_FILL_STK:
#if 0
    lui    t0, 0x55555
    addi   t0, t0, 0x555
    la     t2, _BSS_VMA_END
    la     t3, _STACK_TOP
_FILL_STK_BEGIN:
    bleu   t3, t2, _MAIN_FUNC
    sw     t0, 0(t2)
    addi   t2, t2, 4
    j      _FILL_STK_BEGIN
#endif

    /* Jump to the main function */
_MAIN_FUNC:
    nop

    la     t0, main
    jalr   t0

    nop
    nop
    nop
    nop
    nop
_END:
    j    _END


 .section .retention_reset, "ax"
    .option push
    .option norelax
    .global _IRESET_ENTRY
    .type _IRESET_ENTRY,@function

    .align 2
_IRESET_ENTRY:
    /* Decide whether this is an NMI or cold reset */
    j       _ISTART

    .org 0x22
_ISTART:
#if 0
    // add debug, PB4 output 1
    lui     t0, 0x80140
    li      t1, 0xef
    li      t2, 0x10
    sb      t1, 0x30a(t0)        //0x8014030a  PB oen     =  0xef
    sb      t2, 0x30b(t0)        //0x8014030b  PB output  =  0x10
#endif
    /* Initialize global pointer */

    la     gp, __global_pointer$
    .option pop

    /* Initialize stack pointer */
    la     t0, _STACK_TOP
    mv     sp, t0

#ifdef __nds_execit
    /* Initialize EXEC.IT table */
    la t0, _ITB_BASE_
    csrw uitb, t0
#endif

#ifdef __riscv_flen
    /* Enable FPU */
    li t0, 0x00006000
    csrrs t0, mstatus, t0
    /* Initialize FCSR */
    fscsr zero
#endif

    /* Initial machine trap-vector Base */
    la     t0, __vectors
    csrw   mtvec, t0

    /* Enable vectored external plic interrupt */
    csrsi  mmisc_ctl, 2

    /*vector mode enable bit (VECTORED) of the Feature Enable Register */
    lui     t0, 0xe4000
    li      t1, 0x02
    sw      t1, 0x0(t0)      //(*(volatile unsigned long*)(0xe4000000))= 0x02

    /* Enable I/D-Cache */
    csrr   t0,  mcache_ctl
    ori    t0,  t0,  1  #/I-Cache
    ori    t0,  t0,  2  #/D-Cache
    csrw   mcache_ctl,  t0
    fence.i

    /* after retention, digital register will be lost, restore some register values that need to be used */
    la     t0, pm_retention_register_recover
    jalr   t0
    /* flash wakeup */
#if 0
    /*flash poweron need delay about 150us */
    li      t0 , 0
    li      t1 , 2000
_WAIT_FLASH_POWERON:
    addi    t0 , t0 , 1
    bgeu    t1 , t0 , _WAIT_FLASH_POWERON
#endif

    lui     t0, 0xA3FFF
    addi    t0, t0, 0x790
    li      t1, 0x0e
    sb      t1, 0x78c(t0)       //xip_stop      :0xA3FFFF1c = 0x0e
_MSPI_WAIT_1:
    lui     t0, 0xA3FFF
    addi    t0, t0, 0x790
    lb      t2, 0x798(t0)
    li      t3, 0x80
    beq     t3, t2, _MSPI_WAIT_1//read reg_mspi_status FLD_MSPI_BUSY(0xA3FFFF28[bit7])
    lui     t0, 0xA3FFF
    addi    t0, t0, 0x790
    li      t1, 0x06
    sb      t1, 0x78c(t0)       //xip_disable       :0xA3FFFF1c = 0x06
    lui     t0, 0x80140
    li      t1, 0x3f
    sb      t1, 0x331(t0)       //mspi ie enable    :0x80140331 = 0x3f
    lui     t0, 0xA3FFF
    addi    t0, t0, 0x790
    li      t2, 0x80
    sb      t2, 0x778(t0)       //mspi_ctrl1        :0xA3FFFF08 = 0x80
    li      t3, 0xab
    sb      t3, 0x774(t0)       //wakeup_cmd        :0xA3FFFF04 = 0xab
_MSPI_WAIT_2:
    lui     t0, 0xA3FFF
    addi    t0, t0, 0x790
    lb      t2, 0x798(t0)
    li      t3, 0x80
    beq     t3, t2, _MSPI_WAIT_2//read reg_mspi_status FLD_MSPI_BUSY(0xA3FFFF28[bit7])
    lui     t0, 0xA3FFF
    addi    t0, t0, 0x790
    li      t1, 0x0a
    sb      t1, 0x78c(t0)       //xip_enable        :0xA3FFFF1c = 0x0a

    /*flash wakeup need delay about 150us */
    li      t0 , 0
    li      t1 , 2000
_WAIT_FLASH_WAKEUP:
    addi    t0 , t0 , 1
    bgeu    t1 , t0 , _WAIT_FLASH_WAKEUP

_MULTI_ADDRESS_BEGIN:
    lui     t0 , 0xA3FFF
    addi    t0 , t0 , 0x790
    la      t1 , g_pm_mspi_cfg
    lw      t2 , 0(t1)
    sw      t2 , 0x790(t0)      //g_pm_mspi_cfg->0xA3FFFF20

    /* Move Data from flash to sram */
_IDATA_INIT:
    la     t1, _DATA_LMA_START
    la     t2, _DATA_VMA_START
    la     t3, _DATA_VMA_END
_IDATA_INIT_BEGIN:
    bleu   t3, t2, _IZERO_BSS
    lw     t0, 0(t1)
    sw     t0, 0(t2)
    addi   t1, t1, 4
    addi   t2, t2, 4
    j      _IDATA_INIT_BEGIN

    /* Zero .bss section in sram */
_IZERO_BSS:
    lui    t0, 0
    la     t2, _BSS_VMA_START
    la     t3, _BSS_VMA_END
_IZERO_BSS_BEGIN:
    bleu   t3, t2, _IZERO_AES
    sw     t0, 0(t2)
    addi   t2, t2, 4
    j      _IZERO_BSS_BEGIN

    /* Zero .aes section in sram */
_IZERO_AES:
    lui    t0, 0
    la     t2, _AES_VMA_START
    la     t3, _AES_VMA_END
_IZERO_AES_BEGIN:
    bleu   t3, t2, _IFILL_STK
    sw     t0, 0(t2)
    addi   t2, t2, 4
    j      _IZERO_AES_BEGIN

    /* Fill the remaining section in sram */
_IFILL_STK:
#if 0
    lui    t0, 0x55555
    addi   t0, t0, 0x555
    la     t2, _BSS_VMA_END
    la     t3, _STACK_TOP
_IFILL_STK_BEGIN:
    bleu   t3, t2, _IMAIN_FUNC
    sw     t0, 0(t2)
    addi   t2, t2, 4
    j      _IFILL_STK_BEGIN
#endif

    /* Jump to the main function */
_IMAIN_FUNC:
    nop

    la     t0, main
    jalr   t0

    nop
    nop
    nop
    nop
    nop
_IEND:
    j    _IEND


.text
    .global default_irq_entry
    .align 2

default_irq_entry:
1:  j 1b

       .weak trap_handler

 trap_handler:
1:  j 1b

    .macro INTERRUPT num
    .weak entry_irq\num
    .set entry_irq\num, default_irq_entry
    .long entry_irq\num
    .endm

#define VECTOR_NUMINTRS         47

    .section .ram_code, "ax"

    .global __vectors
    .balign 256


__vectors:
    /* Trap vector */
    .long trap_entry

    /* PLIC interrupt vector */
    .altmacro
    .set irqno, 1
    .rept VECTOR_NUMINTRS/*  .rept  .endr  */
    INTERRUPT %irqno
    .set irqno, irqno+1
    .endr
#endif
