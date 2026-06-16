/********************************************************************************************************
 * @file    cstartup_TL322X_nvm.S
 *
 * @brief   This is the boot file for tl322x
 *
 * @author  Driver Group
 * @date    2025
 *
 * @par     Copyright (c) 2025, Telink Semiconductor (Shanghai) Co., Ltd. ("TELINK")
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
#ifdef N22_MCU_STARTUP_NVM

/* If using Andes toolchain, these two Macro are defined in it's toolchain */
/* If using std gcc toolchain, these two core registers(not standard risc-v core registers)
    should be defined here. */
#ifdef STD_GCC
#define mmisc_ctl  0x7D0
#endif

//D25 ILM, D25 DLM, N22 ILM, N22 DLM made into software sharing, The driver is configured to 384k by default, can choose the corresponding size according to the chip specification.
//SRAM_384K: the total size is 384k (32k*12), a total of 12 copies of 32k;
//SRAM_256K: the total size is 256k (32k*8), a total of 8 copies of 32k;
//which can be divided to d25f/n22, limit: d25f ILM at least 32K.
//Note: d25f/cstartup_flash.S, d25f/cstartup_ram.S, n22/cstartup_flash.S, n22/cstartup_ram.S configurations are consistent and need to be modified together.
#define  SRAM_384K      0
#define  SRAM_256K      1
#define  SRAM_SIZE     SRAM_384K

#if(SRAM_SIZE  ==   SRAM_384K)
    .equ __D25F_IRAM_SIZE,   2 //64KB
    .equ __D25F_DRAM_SIZE,   3 //96KB
    .equ __N22_IRAM_SIZE,    5 //160KB
    .equ __N22_DRAM_SIZE,    2 //64KB
#elif(SRAM_SIZE  ==   SRAM_256K)
    .equ __D25F_IRAM_SIZE,   3 //96KB
    .equ __D25F_DRAM_SIZE,   3 //96KB
    .equ __N22_IRAM_SIZE,    1 //32KB
    .equ __N22_DRAM_SIZE,    1 //32KB
#endif

    .equ __D25F_N22_RAM_REG,    ((__N22_DRAM_SIZE+__N22_IRAM_SIZE)<<12)|(__N22_DRAM_SIZE<<8)|(__D25F_DRAM_SIZE)

    .section .vectors, "ax"

    .global     __D25F_IRAM_SIZE
    .global     __D25F_DRAM_SIZE
    .global     __N22_IRAM_SIZE
    .global     __N22_DRAM_SIZE
    .global     __D25F_N22_RAM_REG

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

    .org 0x18
    .word (BIN_SIZE)

    .org 0x20
    .word ('T'<<24 | 'L'<<16 | 'N'<<8 | 'K')

    .org 0x24
/* set bit[22] to 1, enable timeout:
 * when xip reaches a certain time without fetching instructions, cs will not be pulled down, saving power consumption.
 */
      .word (0x3B4097A9)           //DREAD:   cmd:1x, addr:1x, data:2x, dummy:8
//    .word (0xEB4095BA)           //X4READ:  cmd:1x, addr:4x, data:4x, dummy:6
//    .word (0xEB4493BA)           //X4READ:  cmd:1x, addr:4x, data:4x, token:0x00, dummy:4

    .option pop
    .align 2

_START:

    /*Initialize D25 & N22 ILM DLM Size */
   // lui    t0 , 0x80141
   // li     t1 , __D25F_N22_RAM_REG
   // sh     t1 , -2036(t0)              //write16(0x8014080c,0x2105)

    /* Initialize global pointer */
    .option push
    .option norelax
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

    /* Initial machine non-vectored trap Base */
    la      t0, trap_entry
    ori     t0, t0, 3
    csrw    mtvec, t0

    /* Initial machine trap-vector Base */
    la      t0, __vectors
    csrw    mtvt, t0
#if 0
    /* Enable vectored external plic interrupt */
    csrsi   mmisc_ctl, 2

    /*vector mode enable bit (VECTORED) of the Feature Enable Register */
    lui     t0, 0xe4000
    li      t1, 0x02
    sw      t1, 0x0(t0)      //(*(volatile unsigned long*)(0xe4000000))= 0x02
#endif

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
    bleu   t3, t2, _RETENTION_DATA_INIT
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
    bleu   t3, t2, _FILL_STK
    sw     t0, 0(t2)
    addi   t2, t2, 4
    j      _ZERO_BSS_BEGIN

    /* Fill the remaining section in sram */
_FILL_STK:
#if 0
    lui    t0, 0x55555
    addi   t0, t0, 0x555
    la     t2, _BSS_VMA_END    //_BSS_VMA_END  must be 4-byte aligned
    la     t3, _STACK_TOP      //_STACK_TOP    must be 4-byte aligned
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

    .align 2
    .org 0x22

_ISTART:
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

    /* Initial machine non-vectored trap Base */
    la      t0, trap_entry
    ori     t0, t0, 3
    csrw    mtvec, t0

    /* Initial machine trap-vector Base */
    la      t0, __vectors
    csrw    mtvt, t0

    /* Enable vectored external plic interrupt */
    csrsi   mmisc_ctl, 2

    /*vector mode enable bit (VECTORED) of the Feature Enable Register */
    lui     t0, 0xe4000
    li      t1, 0x02
    sw      t1, 0x0(t0)      //(*(volatile unsigned long*)(0xe4000000))= 0x02

    /* Enable I/D-Cache */
    csrr    t0,  mcache_ctl
    ori     t0,  t0,  1  #/I-Cache
    ori     t0,  t0,  2  #/D-Cache
    csrw    mcache_ctl,  t0
    fence.i

    /* Zero .bss section in sram */
_IZERO_BSS:
    lui    t0, 0
    la     t2, _BSS_VMA_START
    la     t3, _BSS_VMA_END
_IZERO_BSS_BEGIN:
    bleu   t3, t2, _IFILL_STK
    sw     t0, 0(t2)
    addi   t2, t2, 4
    j      _IZERO_BSS_BEGIN

    /* Fill the remaining section in sram */
_IFILL_STK:
#if 0
    lui    t0, 0x55555
    addi   t0, t0, 0x555
    la     t2, _BSS_VMA_END     //_BSS_VMA_END  must be 4-byte aligned
    la     t3, _STACK_TOP       //_STACK_TOP    must be 4-byte aligned
_IFILL_STK_BEGIN:
    bleu   t3, t2, _IDATA_INIT
    sw     t0, 0(t2)
    addi   t2, t2, 4
    j      _IFILL_STK_BEGIN
#endif

    /* Move Data from flash to sram */
//Read the flash as late as possible to lengthen the time from the wake up to the operation of the flash.
_IDATA_INIT:
    la     t1, _DATA_LMA_START
    la     t2, _DATA_VMA_START
    la     t3, _DATA_VMA_END
_IDATA_INIT_BEGIN:
    bleu   t3, t2, _IMAIN_FUNC
    lw     t0, 0(t1)
    sw     t0, 0(t2)
    addi   t1, t1, 4
    addi   t2, t2, 4
    j      _IDATA_INIT_BEGIN

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

#define VECTOR_NUMINTRS         82 /* 64+18  1~18 reserved */

 .section .ram_code, "ax"
    .global __vectors
    .balign 512                    /*2^(ceiling(log2(N)+2),N=82*/


__vectors:
    /* Trap vector */
    .long trap_entry

    /* CLIC interrupt vector */
    .altmacro
    .set irqno, 1
    .rept VECTOR_NUMINTRS/*  .rept  .endr  */
    INTERRUPT %irqno
    .set irqno, irqno+1
    .endr

    .weak trap_entry
    .end
#endif
