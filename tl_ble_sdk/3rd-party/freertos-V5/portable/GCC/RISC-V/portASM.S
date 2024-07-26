/*
 * FreeRTOS Kernel V10.4.2
 * Copyright (C) 2020 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * https://www.FreeRTOS.org
 * https://github.com/FreeRTOS
 *
 * 1 tab == 4 spaces!
 */

/*
 * The FreeRTOS kernel's RISC-V port is split between the the code that is
 * common across all currently supported RISC-V chips (implementations of the
 * RISC-V ISA), and code which tailors the port to a specific RISC-V chip:
 *
 * + The code that is common to all RISC-V chips is implemented in
 *   FreeRTOS\Source\portable\GCC\RISC-V-RV32\portASM.S.  There is only one
 *   portASM.S file because the same file is used no matter which RISC-V chip is
 *   in use.
 *
 * + The code that tailors the kernel's RISC-V port to a specific RISC-V
 *   chip is implemented in freertos_risc_v_chip_specific_extensions.h.  There
 *   is one freertos_risc_v_chip_specific_extensions.h that can be used with any
 *   RISC-V chip that both includes a standard CLINT and does not add to the
 *   base set of RISC-V registers.  There are additional
 *   freertos_risc_v_chip_specific_extensions.h files for RISC-V implementations
 *   that do not include a standard CLINT or do add to the base set of RISC-V
 *   registers.
 *
 * CARE MUST BE TAKEN TO INCLDUE THE CORRECT
 * freertos_risc_v_chip_specific_extensions.h HEADER FILE FOR THE CHIP
 * IN USE.  To include the correct freertos_risc_v_chip_specific_extensions.h
 * header file ensure the path to the correct header file is in the assembler's
 * include path.
 *
 * This freertos_risc_v_chip_specific_extensions.h is for use on RISC-V chips
 * that include a standard CLINT and do not add to the base set of RISC-V
 * registers.
 *
 */
#include "vendor/common/user_config.h"
#if( 1 )

.section .ram_code
 
#if __riscv_xlen == 64
    #define portWORD_SIZE 8
    #define store_x sd
    #define load_x ld
#elif __riscv_xlen == 32
    #define store_x sw
    #define load_x lw
    #define portWORD_SIZE 4
#else
    #error Assembler did not define __riscv_xlen
#endif

#include "freertos_risc_v_chip_specific_extensions.h"

/* Check the freertos_risc_v_chip_specific_extensions.h and/or command line
definitions. */
#if (portasmHAS_CLINT ) && defined( portasmHAS_MTIME )
    #error The portasmHAS_CLINT constant has been deprecated.  Please replace it with portasmHAS_MTIME.  portasmHAS_CLINT and portasmHAS_MTIME cannot both be defined at once.  See https://www.FreeRTOS.org/Using-FreeRTOS-on-RISC-V.html
#endif

#if portasmHAS_CLINT
    #warning The portasmHAS_CLINT constant has been deprecated.  Please replace it with portasmHAS_MTIME and portasmHAS_SIFIVE_CLINT.  For now portasmHAS_MTIME and portasmHAS_SIFIVE_CLINT are derived from portasmHAS_CLINT.  See https://www.FreeRTOS.org/Using-FreeRTOS-on-RISC-V.html
    #define portasmHAS_MTIME portasmHAS_CLINT
    #define portasmHAS_SIFIVE_CLINT portasmHAS_CLINT
#endif

#ifndef portasmHAS_MTIME
    #error freertos_risc_v_chip_specific_extensions.h must define portasmHAS_MTIME to either 1 (MTIME clock present) or 0 (MTIME clock not present).  See https://www.FreeRTOS.org/Using-FreeRTOS-on-RISC-V.html
#endif

#ifndef portasmHANDLE_INTERRUPT
//  #error portasmHANDLE_INTERRUPT must be defined to the function to be called to handle external/peripheral interrupts.  portasmHANDLE_INTERRUPT can be defined on the assembler command line or in the appropriate freertos_risc_v_chip_specific_extensions.h header file.  https://www.FreeRTOS.org/Using-FreeRTOS-on-RISC-V.html
#endif

#ifndef portasmHAS_SIFIVE_CLINT
    #define portasmHAS_SIFIVE_CLINT 0
#endif

/* Only the standard core registers are stored by default.  Any additional
registers must be saved by the portasmSAVE_ADDITIONAL_REGISTERS and
portasmRESTORE_ADDITIONAL_REGISTERS macros - which can be defined in a chip
specific version of freertos_risc_v_chip_specific_extensions.h.  See the notes
at the top of this file. */
#define portCONTEXT_SIZE ( 30 * portWORD_SIZE )

    .global xPortStartFirstTask
    .global trap_entry
    .global pxPortInitialiseStack
    .global freertos_task_switch_handler
    .global xPortYield

    .extern pxCurrentTCB
    .extern ulPortTrapHandler
    .extern vTaskSwitchContext
    .extern xTaskIncrementTick
    .extern Timer_IRQHandler
    .extern pullMachineTimerCompareRegister
    .extern pullNextTime
    .extern uxTimerIncrementsForOneTick /* size_t type so 32-bit on 32-bit core and 64-bits on 64-bit core. */
    .extern xISRStackTop
    .extern portasmHANDLE_INTERRUPT

/*-----------------------------------------------------------*/
.macro portasmSAVE_REGISTERS
    addi    sp, sp, -portCONTEXT_SIZE
    store_x x1,   1 * portWORD_SIZE( sp )
    store_x x5,   2 * portWORD_SIZE( sp )
    store_x x6,   3 * portWORD_SIZE( sp )
    store_x x7,   4 * portWORD_SIZE( sp )
    store_x x8,   5 * portWORD_SIZE( sp )
    store_x x9,   6 * portWORD_SIZE( sp )
    store_x x10,  7 * portWORD_SIZE( sp )
    store_x x11,  8 * portWORD_SIZE( sp )
    store_x x12,  9 * portWORD_SIZE( sp )
    store_x x13, 10 * portWORD_SIZE( sp )
    store_x x14, 11 * portWORD_SIZE( sp )
    store_x x15, 12 * portWORD_SIZE( sp )
    store_x x16, 13 * portWORD_SIZE( sp )
    store_x x17, 14 * portWORD_SIZE( sp )
    store_x x18, 15 * portWORD_SIZE( sp )
    store_x x19, 16 * portWORD_SIZE( sp )
    store_x x20, 17 * portWORD_SIZE( sp )
    store_x x21, 18 * portWORD_SIZE( sp )
    store_x x22, 19 * portWORD_SIZE( sp )
    store_x x23, 20 * portWORD_SIZE( sp )
    store_x x24, 21 * portWORD_SIZE( sp )
    store_x x25, 22 * portWORD_SIZE( sp )
    store_x x26, 23 * portWORD_SIZE( sp )
    store_x x27, 24 * portWORD_SIZE( sp )
    store_x x28, 25 * portWORD_SIZE( sp )
    store_x x29, 26 * portWORD_SIZE( sp )
    store_x x30, 27 * portWORD_SIZE( sp )
    store_x x31, 28 * portWORD_SIZE( sp )

    csrr    t0, mstatus                 /* Required for MPIE bit. */
    store_x t0, 29 * portWORD_SIZE( sp )

    portasmSAVE_ADDITIONAL_REGISTERS    /* Defined in freertos_risc_v_chip_specific_extensions.h to save any registers unique to the RISC-V implementation. */

    store_x  x1, 0 * portWORD_SIZE( sp ) /* save ra as mepc for NONE irq switch in xPortYield function*/

    load_x   t0, pxCurrentTCB           /* Load pxCurrentTCB. */
    store_x  sp, 0( t0 )                /* Write sp to first TCB member. */
.endm

.macro portasmRESTORE_REGISTERS
    load_x   t1, pxCurrentTCB           /* Load pxCurrentTCB. */
    load_x   sp, 0( t1 )                    /* Read sp from first TCB member. */

    /* Load mret with the address of the next instruction in the task to run next. */
    load_x   t0, 0( sp )
    csrw     mepc, t0

    portasmRESTORE_ADDITIONAL_REGISTERS /* Defined in freertos_risc_v_chip_specific_extensions.h to restore any registers unique to the RISC-V implementation. */

    /* Load mstatus with the interrupt enable bits used by the task. */
    load_x  t0, 29 * portWORD_SIZE( sp )
    csrw    mstatus, t0                     /* Required for MPIE bit. */

    load_x  x1,   1 * portWORD_SIZE( sp )
    load_x  x5,   2 * portWORD_SIZE( sp )       /* t0 */
    load_x  x6,   3 * portWORD_SIZE( sp )       /* t1 */
    load_x  x7,   4 * portWORD_SIZE( sp )       /* t2 */
    load_x  x8,   5 * portWORD_SIZE( sp )       /* s0/fp */
    load_x  x9,   6 * portWORD_SIZE( sp )       /* s1 */
    load_x  x10,  7 * portWORD_SIZE( sp )   /* a0 */
    load_x  x11,  8 * portWORD_SIZE( sp )   /* a1 */
    load_x  x12,  9 * portWORD_SIZE( sp )   /* a2 */
    load_x  x13, 10 * portWORD_SIZE( sp )   /* a3 */
    load_x  x14, 11 * portWORD_SIZE( sp )   /* a4 */
    load_x  x15, 12 * portWORD_SIZE( sp )   /* a5 */
    load_x  x16, 13 * portWORD_SIZE( sp )   /* a6 */
    load_x  x17, 14 * portWORD_SIZE( sp )   /* a7 */
    load_x  x18, 15 * portWORD_SIZE( sp )   /* s2 */
    load_x  x19, 16 * portWORD_SIZE( sp )   /* s3 */
    load_x  x20, 17 * portWORD_SIZE( sp )   /* s4 */
    load_x  x21, 18 * portWORD_SIZE( sp )   /* s5 */
    load_x  x22, 19 * portWORD_SIZE( sp )   /* s6 */
    load_x  x23, 20 * portWORD_SIZE( sp )   /* s7 */
    load_x  x24, 21 * portWORD_SIZE( sp )   /* s8 */
    load_x  x25, 22 * portWORD_SIZE( sp )   /* s9 */
    load_x  x26, 23 * portWORD_SIZE( sp )   /* s10 */
    load_x  x27, 24 * portWORD_SIZE( sp )   /* s11 */
    load_x  x28, 25 * portWORD_SIZE( sp )   /* t3 */
    load_x  x29, 26 * portWORD_SIZE( sp )   /* t4 */
    load_x  x30, 27 * portWORD_SIZE( sp )   /* t5 */
    load_x  x31, 28 * portWORD_SIZE( sp )   /* t6 */
    addi    sp, sp, portCONTEXT_SIZE
.endm

/*-----------------------------------------------------------*/
#if 1
    .extern xPortIrqHandler
    .globl vPortTimerUpdate

.align 3
.func
vPortTimerUpdate:
    load_x  t0, pullMachineTimerCompareRegister  /* Load address of compare register into t0. */
    load_x  t1, pullNextTime        /* Load the address of ullNextTime into t1. */

  #if( __riscv_xlen == 32 )
    /* Update the 64-bit mtimer compare match value in two 32-bit writes. */
    li      t4, -1
    lw      t2, 0(t1)               /* Load the low word of ullNextTime into t2. */
    lw      t3, 4(t1)               /* Load the high word of ullNextTime into t3. */
    sw      t4, 0(t0)               /* Low word no smaller than old value to start with - will be overwritten below. */
    sw      t3, 4(t0)               /* Store high word of ullNextTime into compare register.  No smaller than new value. */
    sw      t2, 0(t0)               /* Store low word of ullNextTime into compare register. */
    lw      t0, uxTimerIncrementsForOneTick /* Load the value of ullTimerIncrementForOneTick into t0 (could this be optimized by storing in an array next to pullNextTime?). */
    add     t4, t0, t2              /* Add the low word of ullNextTime to the timer increments for one tick (assumes timer increment for one tick fits in 32-bits). */
    sltu    t5, t4, t2              /* See if the sum of low words overflowed (what about the zero case?). */
    add     t6, t3, t5              /* Add overflow to high word of ullNextTime. */
    sw      t4, 0(t1)               /* Store new low word of ullNextTime. */
    sw      t6, 4(t1)               /* Store new high word of ullNextTime. */
  #endif /* __riscv_xlen == 32 */

  #if( __riscv_xlen == 64 )
    /* Update the 64-bit mtimer compare match value. */
    ld    t2, 0(t1)             /* Load ullNextTime into t2. */
    sd    t2, 0(t0)             /* Store ullNextTime into compare register. */
    ld    t0, uxTimerIncrementsForOneTick  /* Load the value of ullTimerIncrementForOneTick into t0 (could this be optimized by storing in an array next to pullNextTime?). */
    add   t4, t0, t2                /* Add ullNextTime to the timer increments for one tick. */
    sd    t4, 0(t1)             /* Store ullNextTime. */
  #endif /* __riscv_xlen == 64 */
    ret
.endfunc


.align 3            //  2 ^ 3 == 8
.func
trap_entry:
    portasmSAVE_REGISTERS

    csrr    a0, mcause
    csrr    a1, mepc

    store_x a1, 0( sp )      /* save mepc to sp. */

    load_x  sp, xISRStackTop  /* switch sp to irq stack for call functions. */
    jal     portasmHANDLE_INTERRUPT

    portasmRESTORE_REGISTERS
    mret
.endfunc

#else
.align 3            //  2 ^ 3 == 8
.func
trap_entry:
    portasmSAVE_REGISTERS

    csrr a0, mcause
    csrr a1, mepc

test_if_asynchronous:
    srli a2, a0, __riscv_xlen - 1       /* MSB of mcause is 1 if handing an asynchronous interrupt - shift to LSB to clear other bits. */
    beq a2, x0, handle_synchronous      /* Branch past interrupt handing if not asynchronous. */
    store_x a1, 0( sp )                 /* Asynch so save unmodified exception return address. */

handle_asynchronous:

#if( portasmHAS_MTIME != 0 )

    test_if_mtimer:                     /* If there is a CLINT then the mtimer is used to generate the tick interrupt. */

        addi t0, x0, 1

        slli t0, t0, __riscv_xlen - 1   /* LSB is already set, shift into MSB.  Shift 31 on 32-bit or 63 on 64-bit cores. */
        addi t1, t0, 7                  /* 0x8000[]0007 == machine timer interrupt. */
        bne a0, t1, test_if_external_interrupt

        load_x t0, pullMachineTimerCompareRegister  /* Load address of compare register into t0. */
        load_x t1, pullNextTime         /* Load the address of ullNextTime into t1. */

        #if( __riscv_xlen == 32 )

            /* Update the 64-bit mtimer compare match value in two 32-bit writes. */
            li t4, -1
            lw t2, 0(t1)                /* Load the low word of ullNextTime into t2. */
            lw t3, 4(t1)                /* Load the high word of ullNextTime into t3. */
            sw t4, 0(t0)                /* Low word no smaller than old value to start with - will be overwritten below. */
            sw t3, 4(t0)                /* Store high word of ullNextTime into compare register.  No smaller than new value. */
            sw t2, 0(t0)                /* Store low word of ullNextTime into compare register. */
            lw t0, uxTimerIncrementsForOneTick  /* Load the value of ullTimerIncrementForOneTick into t0 (could this be optimized by storing in an array next to pullNextTime?). */
            add t4, t0, t2              /* Add the low word of ullNextTime to the timer increments for one tick (assumes timer increment for one tick fits in 32-bits). */
            sltu t5, t4, t2             /* See if the sum of low words overflowed (what about the zero case?). */
            add t6, t3, t5              /* Add overflow to high word of ullNextTime. */
            sw t4, 0(t1)                /* Store new low word of ullNextTime. */
            sw t6, 4(t1)                /* Store new high word of ullNextTime. */

        #endif /* __riscv_xlen == 32 */

        #if( __riscv_xlen == 64 )

            /* Update the 64-bit mtimer compare match value. */
            ld t2, 0(t1)                /* Load ullNextTime into t2. */
            sd t2, 0(t0)                /* Store ullNextTime into compare register. */
            ld t0, uxTimerIncrementsForOneTick  /* Load the value of ullTimerIncrementForOneTick into t0 (could this be optimized by storing in an array next to pullNextTime?). */
            add t4, t0, t2              /* Add ullNextTime to the timer increments for one tick. */
            sd t4, 0(t1)                /* Store ullNextTime. */

        #endif /* __riscv_xlen == 64 */

        /* If MTIME interrupt is not enabled, 
         * don't switch task and shall branch to "processed_source" .
         */
        csrr    a0, mie
        bbc     a0, 7, processed_source

        lui     s5, 0xe4200             //  check reg_irq_threshold, 0x140200
        load_x  s5, 0(s5)
        bne x0, s5, 1f                  // if  reg_irq_threshold  not zero, then skip interrup
        
        load_x sp, xISRStackTop         /* Switch to ISR stack before function call. */
        jal xTaskIncrementTick
        beqz a0, processed_source       /* Don't switch context if incrementing tick didn't unblock a task. */
        jal vTaskSwitchContext
1:      
        j processed_source

    test_if_external_interrupt:         /* If there is a CLINT and the mtimer interrupt is not pending then check to see if an external interrupt is pending. */
        addi t1, t1, 4                  /* 0x80000007 + 4 = 0x8000000b == Machine external interrupt. */
        bne a0, t1, as_yet_unhandled    /* Something as yet unhandled. */

#endif /* portasmHAS_MTIME */

    load_x sp, xISRStackTop             /* Switch to ISR stack before function call. */
//  jal portasmHANDLE_INTERRUPT         /* Jump to the interrupt handler if there is no CLINT or if there is a CLINT and it has been determined that an external interrupt is pending. */
    j processed_source

handle_synchronous:
    addi a1, a1, 4                      /* Synchronous so updated exception return address to the instruction after the instruction that generated the exeption. */
    store_x a1, 0( sp )                 /* Save updated exception return address. */

test_if_environment_call:
#if 0
    li t0, 11                           /* 11 == environment call. */
    bne a0, t0, is_exception            /* Not an M environment call, so some other exception. */
    load_x sp, xISRStackTop             /* Switch to ISR stack before function call. */
    jal vTaskSwitchContext
    j processed_source
#endif

is_exception:
    csrr t0, mcause                     /* For viewing in the debugger only. */
    csrr t1, mepc                       /* For viewing in the debugger only */
    csrr t2, mstatus
    j is_exception                      /* No other exceptions handled yet. */

as_yet_unhandled:
    csrr t0, mcause                     /* For viewing in the debugger only. */
    j as_yet_unhandled

processed_source:
    portasmRESTORE_REGISTERS
    mret
    .endfunc
#endif

/*-----------------------------------------------------------*/

.align 3            //  2 ^ 3 == 8
.func
xPortYield:
    csrrci  t0, mstatus,8   // disable interrupt, save mstatus to x5 (t0) //   It is OK to change t0, because xPortYield is a normal function
    andi    t0, t0, 8       // mask MIE bit
    slli    t0, t0, 4       // Copy the original MIE to MPIE
    li      t2, 0x800       # MPP -> 1
    or      t0, t0, t2      # MPP -> 1
    li      t1, 0x80
    csrc    mstatus, t1     // Copy the original MIE to MPIE
    csrs    mstatus, t0     // Copy the original MIE to MPIE
    portasmSAVE_REGISTERS
    load_x sp, xISRStackTop             /* Switch to ISR stack before function call. */
    jal vTaskSwitchContext
    portasmRESTORE_REGISTERS
    mret
.endfunc

/*-----------------------------------------------------------*/
.section .text

.align 3            //  2 ^ 3 == 8
.func
xPortStartFirstTask:

#if( portasmHAS_SIFIVE_CLINT != 0 )
    /* If there is a clint then interrupts can branch directly to the FreeRTOS
    trap handler.  Otherwise the interrupt controller will need to be configured
    outside of this file. */
    la t0, trap_entry
    csrw mtvec, t0
#endif /* portasmHAS_CLILNT */
    csrci   mstatus, 8          // disable interrupt
    portasmRESTORE_REGISTERS
    mret
    .endfunc
/*-----------------------------------------------------------*/

/*
 * Unlike other ports pxPortInitialiseStack() is written in assembly code as it
 * needs access to the portasmADDITIONAL_CONTEXT_SIZE constant.  The prototype
 * for the function is as per the other ports:
 * StackType_t *pxPortInitialiseStack( StackType_t *pxTopOfStack, TaskFunction_t pxCode, void *pvParameters );
 *
 * As per the standard RISC-V ABI pxTopcOfStack is passed in in a0, pxCode in
 * a1, and pvParameters in a2.  The new top of stack is passed out in a0.
 *
 * RISC-V maps registers to ABI names as follows (X1 to X31 integer registers
 * for the 'I' profile, X1 to X15 for the 'E' profile, currently I assumed).
 *
 * Register     ABI Name    Description                     Saver
 * x0           zero        Hard-wired zero                 -
 * x1           ra          Return address                  Caller
 * x2           sp          Stack pointer                   Callee
 * x3           gp          Global pointer                  -
 * x4           tp          Thread pointer                  -
 * x5-7         t0-2        Temporaries                     Caller
 * x8           s0/fp       Saved register/Frame pointer    Callee
 * x9           s1          Saved register                  Callee
 * x10-11       a0-1        Function Arguments/return values Caller
 * x12-17       a2-7        Function arguments              Caller
 * x18-27       s2-11       Saved registers                 Callee
 * x28-31       t3-6        Temporaries                     Caller
 *
 * The RISC-V context is saved t FreeRTOS tasks in the following stack frame,
 * where the global and thread pointers are currently assumed to be constant so
 * are not saved:
 *
 * mstatus
 * x31
 * x30
 * x29
 * x28
 * x27
 * x26
 * x25
 * x24
 * x23
 * x22
 * x21
 * x20
 * x19
 * x18
 * x17
 * x16
 * x15
 * x14
 * x13
 * x12
 * x11
 * pvParameters
 * x9
 * x8
 * x7
 * x6
 * x5
 * portTASK_RETURN_ADDRESS
 * [chip specific registers go here]
 * pxCode
 */
.align 3            //  2 ^ 3 == 8
.func
pxPortInitialiseStack:

    csrr t0, mstatus                    /* Obtain current mstatus value. */
    andi t0, t0, ~0x8                   /* Ensure interrupts are disabled when the stack is restored within an ISR.  Required when a task is created after the schedulre has been started, otherwise interrupts would be disabled anyway. */
    addi t1, x0, 0x188                  /* Generate the value 0x1880, which are the MPIE and MPP bits to set in mstatus. */
    slli t1, t1, 4
    or t0, t0, t1                       /* Set MPIE and MPP bits in mstatus value. */

    addi a0, a0, -portWORD_SIZE
    store_x t0, 0(a0)                   /* mstatus onto the stack. */
    addi a0, a0, -(22 * portWORD_SIZE)  /* Space for registers x11-x31. */
    store_x a2, 0(a0)                   /* Task parameters (pvParameters parameter) goes into register X10/a0 on the stack. */
    addi a0, a0, -(6 * portWORD_SIZE)   /* Space for registers x5-x9. */
    store_x x0, 0(a0)                   /* Return address onto the stack, could be portTASK_RETURN_ADDRESS */
    addi t0, x0, portasmADDITIONAL_CONTEXT_SIZE /* The number of chip specific additional registers. */
chip_specific_stack_frame:              /* First add any chip specific registers to the stack frame being created. */
    beq t0, x0, 1f                      /* No more chip specific registers to save. */
    addi a0, a0, -portWORD_SIZE         /* Make space for chip specific register. */
    store_x x0, 0(a0)                   /* Give the chip specific register an initial value of zero. */
    addi t0, t0, -1                     /* Decrement the count of chip specific registers remaining. */
    j chip_specific_stack_frame         /* Until no more chip specific registers. */
1:
    addi a0, a0, -portWORD_SIZE
    store_x a1, 0(a0)                   /* mret value (pxCode parameter) onto the stack. */
    ret
    .endfunc


/*-----------------------------------------------------------*/
    .global vPortSuppressTicksAndSleep
    .extern vPortSuppressTicksAndSleep_i
    .global vPortRestoreActiveTask

    .section .ram_code

.align 3            //  2 ^ 3 == 8
.func
vPortSuppressTicksAndSleep:
    csrrci   x5, mstatus,8              // disable interrupt, save mstatus to x5 (t0)
    portasmSAVE_REGISTERS               // save for vPortRestoreActiveTask

    jal vPortSuppressTicksAndSleep_i

    addi sp, sp, (portasmADDITIONAL_CONTEXT_SIZE * portWORD_SIZE )
    load_x  x1, 1 * portWORD_SIZE( sp )     //  restore ra
    load_x  x5, 2 * portWORD_SIZE( sp )
    addi sp, sp, portCONTEXT_SIZE
    csrw    mstatus, x5                 //  restore mstatus
    ret
.endfunc


.align 3            //  2 ^ 3 == 8
.func
vPortRestoreActiveTask:
    portasmRESTORE_REGISTERS
    csrw    mstatus, x5                 //  restore mstatus
    ret
.endfunc
#if (OS_SEPARATE_STACK_SPACE)
.section .ram_code

#define REG_BYTES   4
#define STORE       sw
#define LOAD        lw

#define CSR_MSTATUS              0x300

.macro IRQ_SAVE_REGISTER

#ifdef __riscv_flen
    addi  sp, sp, -64 * REG_BYTES
#else
    addi  sp, sp, -32 * REG_BYTES
#endif

    STORE x1,   1 * REG_BYTES(sp)
    STORE x7,   7 * REG_BYTES(sp)   //t2
    STORE x10, 10 * REG_BYTES(sp)
    STORE x11, 11 * REG_BYTES(sp)
    STORE x12, 12 * REG_BYTES(sp)
    STORE x13, 13 * REG_BYTES(sp)
    STORE x14, 14 * REG_BYTES(sp)
    STORE x15, 15 * REG_BYTES(sp)
    STORE x16, 16 * REG_BYTES(sp)
    STORE x17, 17 * REG_BYTES(sp)
    STORE x28, 28 * REG_BYTES(sp)   //t3
    STORE x29, 29 * REG_BYTES(sp)   //t4
    STORE x30, 30 * REG_BYTES(sp)   //t5
    STORE x31, 31 * REG_BYTES(sp)   //t6

#ifdef __riscv_flen

/* save FPU necessary registers */
    csrr t0, CSR_MSTATUS
    li   t1, (0x3 << 13)
    and  t2, t0, t1
    bne  t2, t1, irq_skip_fpu_save_caller       /* if mstatus_fs is indicating the "Dirty", save the FPU registers. */

    fsw f0,  32 * REG_BYTES(sp)
    fsw f1,  33 * REG_BYTES(sp)
    fsw f2,  34 * REG_BYTES(sp)
    fsw f3,  35 * REG_BYTES(sp)
    fsw f4,  36 * REG_BYTES(sp)
    fsw f5,  37 * REG_BYTES(sp)
    fsw f6,  38 * REG_BYTES(sp)
    fsw f7,  39 * REG_BYTES(sp)
    fsw f10, 42 * REG_BYTES(sp)
    fsw f11, 43 * REG_BYTES(sp)
    fsw f12, 44 * REG_BYTES(sp)
    fsw f13, 45 * REG_BYTES(sp)
    fsw f14, 46 * REG_BYTES(sp)
    fsw f15, 47 * REG_BYTES(sp)
    fsw f16, 48 * REG_BYTES(sp)
    fsw f17, 49 * REG_BYTES(sp)
    fsw f28, 60 * REG_BYTES(sp)
    fsw f29, 61 * REG_BYTES(sp)
    fsw f30, 62 * REG_BYTES(sp)
    fsw f31, 63 * REG_BYTES(sp)

irq_skip_fpu_save_caller:

#endif
.endm

.macro IRQ_RESTORE_REGISTER

#ifdef __riscv_flen

/* restore FPU necessary registers */
    csrr t2, CSR_MSTATUS
    li   t0, (0x3 << 13)
    and  t1, t2, t0
    bne  t1, t0, irq_skip_fpu_restore_caller    /* if mstatus_fs is indicating the "Dirty", restore the FPU registers. */

    flw f0,  32 * REG_BYTES(sp)
    flw f1,  33 * REG_BYTES(sp)
    flw f2,  34 * REG_BYTES(sp)
    flw f3,  35 * REG_BYTES(sp)
    flw f4,  36 * REG_BYTES(sp)
    flw f5,  37 * REG_BYTES(sp)
    flw f6,  38 * REG_BYTES(sp)
    flw f7,  39 * REG_BYTES(sp)
    flw f10, 42 * REG_BYTES(sp)
    flw f11, 43 * REG_BYTES(sp)
    flw f12, 44 * REG_BYTES(sp)
    flw f13, 45 * REG_BYTES(sp)
    flw f14, 46 * REG_BYTES(sp)
    flw f15, 47 * REG_BYTES(sp)
    flw f16, 48 * REG_BYTES(sp)
    flw f17, 49 * REG_BYTES(sp)
    flw f28, 60 * REG_BYTES(sp)
    flw f29, 61 * REG_BYTES(sp)
    flw f30, 62 * REG_BYTES(sp)
    flw f31, 63 * REG_BYTES(sp)
irq_skip_fpu_restore_caller:

#endif

    LOAD x1,   1 * REG_BYTES(sp)
    LOAD x7,   7 * REG_BYTES(sp)
    LOAD x10, 10 * REG_BYTES(sp)
    LOAD x11, 11 * REG_BYTES(sp)
    LOAD x12, 12 * REG_BYTES(sp)
    LOAD x13, 13 * REG_BYTES(sp)
    LOAD x14, 14 * REG_BYTES(sp)
    LOAD x15, 15 * REG_BYTES(sp)
    LOAD x16, 16 * REG_BYTES(sp)
    LOAD x17, 17 * REG_BYTES(sp)
    LOAD x28, 28 * REG_BYTES(sp)
    LOAD x29, 29 * REG_BYTES(sp)
    LOAD x30, 30 * REG_BYTES(sp)
    LOAD x31, 31 * REG_BYTES(sp)

#ifdef __riscv_flen
    addi  sp, sp, 64 * REG_BYTES
#else
    addi  sp, sp, 32 * REG_BYTES
#endif

.endm

.extern g_plic_switch_sp_flag
.extern tlk_plicISRStackTop
.extern plic_isr
.globl entry_irq_all

.align 3            //  2 ^ 3 == 8
.func
entry_irq_all:
    addi    sp, sp, -3 * REG_BYTES          //change sp
    sw      t0,0(sp)                //save t0
    sw      t1,4(sp)                //save t1
    sw      t2,8(sp)                //save t2
    la      t1,g_plic_switch_sp_flag
    lw      t0,0(t1)                //read flag to t0
    bgtz    t0,switch_sp_end        //if t0 != 0, no switch sp
switch_sp:
    csrrw   x0,mscratch,sp          //save old sp to mscratch
    load_x  sp,tlk_plicISRStackTop  //change sp to _STACK_TOP
switch_sp_end:
    IRQ_SAVE_REGISTER               //save register

    la      t1,g_plic_switch_sp_flag
    lw      t0,0(t1)                //read flag to t0
    li      t2,1
    sll     t2,t2,a1                //t1 = 1<<num
    or      t0,t0,t2                //t0 = t0|t1
    sw      t0,0(t1)                //flag |= bit(n)

    jal     plic_isr

    la      t1,g_plic_switch_sp_flag//read flag addr
    lw      t0,0(t1)                //read flag to t0
    li      t2,1
    lw      a1,44(sp)               //x11 was saved in 11*4(sp)
    sll     t2,t2,a1                //t1 = 1<<num
    xor     t0,t0,t2                //t0 = t0|t1
    sw      t0,0(t1)                //flag |= bit(n)

    IRQ_RESTORE_REGISTER

    la      t1,g_plic_switch_sp_flag
    lw      t0,0(t1)                //read flag to t0
    bgtz    t0,revert_switch_sp_end     //if t0 != 0, no switch sp

revert_switch_sp:
    csrr    t0,mscratch         //get old sp from mscratch
    mv      sp,t0           //change sp to _STACK_TOP
revert_switch_sp_end:
    lw      t0,0(sp)                //save t0
    lw      t1,4(sp)                //save t1
    lw      t2,8(sp)                //save t1
    addi    sp,sp,3 * REG_BYTES     //change sp

    ret
#endif
#endif


