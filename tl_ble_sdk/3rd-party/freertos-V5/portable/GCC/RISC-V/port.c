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

/*-----------------------------------------------------------
 * Implementation of functions defined in portable.h for the RISC-V RV32 port.
 *----------------------------------------------------------*/

/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "portmacro.h"

/* Standard includes. */
#include <string.h>
#include "compiler.h"
#include "types.h"
#include "drivers.h"
#include <stdio.h>
#include "vendor/common/user_config.h"
#include "tlk_riscv.h"
#include "stack/ble/controller/ll/ll_pm.h"


/* Let the user override the pre-loading of the initial LR with the address of
prvTaskExitError() in case it messes up unwinding of the stack in the
debugger. */
#ifdef configTASK_RETURN_ADDRESS
    #define portTASK_RETURN_ADDRESS    configTASK_RETURN_ADDRESS
#else
    #define portTASK_RETURN_ADDRESS    prvTaskExitError
#endif

/* The stack used by interrupt service routines.  Set configISR_STACK_SIZE_WORDS
to use a statically allocated array as the interrupt stack.  Alternative leave
configISR_STACK_SIZE_WORDS undefined and update the linker script so that a
linker variable names __freertos_irq_stack_top has the same value as the top
of the stack used by main.  Using the linker script method will repurpose the
stack that was used by main before the scheduler was started for use as the
interrupt stack after the scheduler has started. */
#ifdef configISR_STACK_SIZE_WORDS
    PRIVILEGED_DATA static __attribute__ ((aligned(16))) StackType_t xISRStack[ configISR_STACK_SIZE_WORDS ] = { 0 };
    PRIVILEGED_DATA StackType_t xISRStackTop = ( StackType_t ) &( xISRStack[ configISR_STACK_SIZE_WORDS & ~portBYTE_ALIGNMENT_MASK ] );

    /* Don't use 0xa5 as the stack fill bytes as that is used by the kernerl for
    the task stacks, and so will legitimately appear in many positions within
    the ISR stack. */
    #define portISR_STACK_FILL_BYTE    0xee
#else
    extern const uint32_t __freertos_irq_stack_top[];
    PRIVILEGED_DATA StackType_t xISRStackTop = ( StackType_t ) __freertos_irq_stack_top;
#endif

/*
 * Setup the timer to generate the tick interrupts.  The implementation in this
 * file is weak to allow application writers to change the timer used to
 * generate the tick interrupt.
 */
void vPortSetupTimerInterrupt( void ) __attribute__(( weak ));

/*-----------------------------------------------------------*/

/* Used to program the machine timer compare register. */
PRIVILEGED_DATA uint64_t ullNextTime = 0ULL;
PRIVILEGED_DATA uint64_t *pullNextTime = &ullNextTime;
PRIVILEGED_DATA size_t uxTimerIncrementsForOneTick = ( size_t ) ( ( configCPU_CLOCK_HZ ) / ( configTICK_RATE_HZ ) ); /* Assumes increment won't go over 32-bits. */
PRIVILEGED_DATA uint32_t ullMachineTimerCompareRegisterBase;
PRIVILEGED_DATA volatile uint64_t * pullMachineTimerCompareRegister = NULL;

/* Set configCHECK_FOR_STACK_OVERFLOW to 3 to add ISR stack checking to task
stack checking.  A problem in the ISR stack will trigger an assert, not call the
stack overflow hook function (because the stack overflow hook is specific to a
task stack, not the ISR stack). */
#if defined( configISR_STACK_SIZE_WORDS ) && ( configCHECK_FOR_STACK_OVERFLOW > 2 )
    #warning This path not tested, or even compiled yet.

    static const uint8_t ucExpectedStackBytes[] = {
                                    portISR_STACK_FILL_BYTE, portISR_STACK_FILL_BYTE, portISR_STACK_FILL_BYTE, portISR_STACK_FILL_BYTE,        \
                                    portISR_STACK_FILL_BYTE, portISR_STACK_FILL_BYTE, portISR_STACK_FILL_BYTE, portISR_STACK_FILL_BYTE,        \
                                    portISR_STACK_FILL_BYTE, portISR_STACK_FILL_BYTE, portISR_STACK_FILL_BYTE, portISR_STACK_FILL_BYTE,        \
                                    portISR_STACK_FILL_BYTE, portISR_STACK_FILL_BYTE, portISR_STACK_FILL_BYTE, portISR_STACK_FILL_BYTE,        \
                                    portISR_STACK_FILL_BYTE, portISR_STACK_FILL_BYTE, portISR_STACK_FILL_BYTE, portISR_STACK_FILL_BYTE };    \

    #define portCHECK_ISR_STACK() configASSERT( ( memcmp( ( void * ) xISRStack, ( void * ) ucExpectedStackBytes, sizeof( ucExpectedStackBytes ) ) == 0 ) )
#else
    /* Define the function away. */
    #define portCHECK_ISR_STACK()
#endif /* configCHECK_FOR_STACK_OVERFLOW > 2 */

/*-----------------------------------------------------------*/
/**
 * brief: RTOS task clock configuration function
 */
RAM_CODE
void vPortSetupTimerInterrupt( void )
{
    if (g_clk_32k_src == CLK_32K_RC)
    {
         uxTimerIncrementsForOneTick = ( size_t ) ( ( 32000 ) / ( configTICK_RATE_HZ ) ); /* Assumes increment won't go over 32-bits. */
    }
    else
    {
         uxTimerIncrementsForOneTick = ( size_t ) ( ( 32768 ) / ( configTICK_RATE_HZ ) ); /* Assumes increment won't go over 32-bits. */
    }
    pullMachineTimerCompareRegister  = &reg_mtimecmp;
    ullNextTime = mtime_get_value();
    ullNextTime += ( uint64_t ) uxTimerIncrementsForOneTick;
    *pullMachineTimerCompareRegister = ullNextTime;
        /* Prepare the time to use after the next tick interrupt. */
    ullNextTime += ( uint64_t ) uxTimerIncrementsForOneTick;
}

/*-----------------------------------------------------------*/

BaseType_t xPortStartScheduler( void )
{
extern void xPortStartFirstTask( void );

    #if( configASSERT_DEFINED == 1 )
    {
        volatile uint32_t mtvec = 0;

        /* Check the least significant two bits of mtvec are 00 - indicating
        single vector mode. */
        __asm volatile( "csrr %0, mtvec" : "=r"( mtvec ) );
//        configASSERT( ( mtvec & 0x03UL ) == 0 );

        /* Check alignment of the interrupt stack - which is the same as the
        stack that was being used by main() prior to the scheduler being
        started. */
        configASSERT( ( xISRStackTop & portBYTE_ALIGNMENT_MASK ) == 0 );

        #ifdef configISR_STACK_SIZE_WORDS
        {
            memset( ( void * ) xISRStack, portISR_STACK_FILL_BYTE, sizeof( xISRStack ) );
        }
        #endif     /* configISR_STACK_SIZE_WORDS */
    }
    #endif /* configASSERT_DEFINED */

    /* If there is a CLINT then it is ok to use the default implementation
    in this file, otherwise vPortSetupTimerInterrupt() must be implemented to
    configure whichever clock is to be used to generate the tick interrupt. */
    vPortSetupTimerInterrupt();
    
    /* Enable mtime and external interrupts.  1<<7 for timer interrupt, 1<<11
    for external interrupt.  _RB_ What happens here when mtime is not present as
    with pulpino? */
    __asm volatile( "csrs mie, %0" :: "r"(0x880) );


    xPortStartFirstTask();

    /* Should not get here as after calling xPortStartFirstTask() only tasks
    should be executing. */
    return pdFAIL;
}
/*-----------------------------------------------------------*/

void vPortEndScheduler( void )
{
    /* Not implemented. */
    printf("vPortEndScheduler \r\n");
    for( ;; );
}

void vAssertCalled( const char * pcFile, unsigned long ulLine ){
    ( void ) pcFile; ( void ) ulLine; 
    printf("assert fail: %s, %ld\r\n", pcFile, ulLine);
    for( ;; );
}

void vApplicationStackOverflowHook( TaskHandle_t pxTask, char *pcTaskName )
{
    ( void ) pcTaskName;
    ( void ) pxTask;

    /* Run time stack overflow checking is performed if
    configconfigCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2.  This hook
    function is called if a stack overflow is detected. */
    printf("vApplicationStackOverflowHook pcTaskName= %s\r\n",pcTaskName);
    for( ;; );
}
/*-----------------------------------------------------------*/

void vApplicationMallocFailedHook( void )
{
    /* Called if a call to pvPortMalloc() fails because there is insufficient
    free memory available in the FreeRTOS heap.  pvPortMalloc() is called
    internally by FreeRTOS API functions that create tasks, queues or
    semaphores. */
    printf("vApplicationMallocFailedHook \r\n");
    for( ;; );
}

/**
 ***********************************************************************************
 *
 * Telink Specific
 *
 ***********************************************************************************
 */
#include "tlk_riscv.h"

#if (OS_SEPARATE_STACK_SPACE) //BLE SDK use
_attribute_data_retention_sec_ volatile unsigned int  g_plic_switch_sp_flag=0;
 static __attribute__ ((aligned(16))) unsigned long plicISRStack[ configISR_PLIC_STACK_SIZE ] = { 0 };
_attribute_data_retention_sec_ unsigned long tlk_plicISRStackTop = ( unsigned long ) &( plicISRStack[ configISR_PLIC_STACK_SIZE & ~0x000f ] );
#endif

#if OS_PM_EN
extern void vPortRestoreActiveTask(void);
extern int blc_pm_handler(void);
#endif


#define SYSTICK_TO_OSTICK               (SYSTEM_TIMER_TICK_1S / configTICK_RATE_HZ)

#define OS_TICK_TO_SYSTIMER_TICK(x)    ((x) * (SYSTEM_TIMER_TICK_1S /configTICK_RATE_HZ))
#define SYSTIMER_TICK_TO_OS_TICK(x)    (((x) + SYSTICK_TO_OSTICK/2) / SYSTICK_TO_OSTICK)


PRIVILEGED_DATA static uint32_t tickBeforeSleep;
RAM_CODE
 void vPortRestoreTick(void)
{
#if ( configUSE_TICKLESS_IDLE != 0 )
    uint32_t t = (uint32_t)(stimer_get_tick() - tickBeforeSleep);
    if(t < ((uint32_t)0xffffffff) / 2){
        //vTaskStepTick((t + SYSTICK_TO_OSTICK/2) / SYSTICK_TO_OSTICK);
        vTaskStepTick(SYSTIMER_TICK_TO_OS_TICK(t));
    }
#endif    
}



RAM_CODE
void vPortSuppressTicksAndSleep_i(uint32_t xExpectedIdleTime)
{
    (void)xExpectedIdleTime;
#if OS_PM_EN
    //DBG_CHN5_LOW;

    uint32_t stimer_tick = tickBeforeSleep = stimer_get_tick();

    if(xExpectedIdleTime > SYSTIMER_TICK_TO_OS_TICK(0xFFFFFFFF>>1)){
        xExpectedIdleTime = SYSTIMER_TICK_TO_OS_TICK(0xFFFFFFFF>>1);
    }
    uint32_t os_wakeup_tick = stimer_tick + OS_TICK_TO_SYSTIMER_TICK(xExpectedIdleTime);

    uint32_t res = 1;
    uint32_t t = 0;
    /* If a context switch is pending then abandon the low power entry as the
    context switch might have been pended by an external interrupt that    requires
    processing. */
    eSleepModeStatus eSleepAction = eTaskConfirmSleepModeStatus();
    if( eSleepAction == eAbortSleep )
    {
        //todo
    }
    else
    {
        if(blc_isBleSchedulerBusy()){
            configPRE_SLEEP_PROCESSING( xExpectedIdleTime );
            blc_pm_setAppWakeupLowPower(os_wakeup_tick, 1);
            res = blc_pm_handler(); ////0:SLEEP; 1: NO SLEEP
            blc_pm_setAppWakeupLowPower(0, 0);
        }
        else {/* you need to enable it if BLE is NOT used */
            configPRE_SLEEP_PROCESSING( xExpectedIdleTime );
            extern int blc_pm_OShandler(uint32_t expect_time);
            if(!blc_pm_OShandler(OS_TICK_TO_SYSTIMER_TICK(xExpectedIdleTime)))
            {
                res= 0;
            }
        }
        if(res == 0)
        {
            /* Restore OS count */
            t = (uint32_t)(stimer_get_tick() - stimer_tick);
            vTaskStepTick(SYSTIMER_TICK_TO_OS_TICK(t));
            /* Allow the application to define some post sleep processing.  This is
            the standard configPOST_SLEEP_PROCESSING() macro, as described on the
            FreeRTOS.org website. */
            configPOST_SLEEP_PROCESSING( SYSTIMER_TICK_TO_OS_TICK(t));
        }
    }
    //DBG_CHN5_HIGH;
#endif
}

void vPortRestoreTask(void)
{
#if OS_PM_EN
    __asm volatile( "csrci     mstatus,8");

    core_mie_enable(FLD_MIE_MTIE | FLD_MIE_MSIE);
    vPortSetupTimerInterrupt();     //    reset the timer compare register to prevent irq triggered immediately

    // to reset IDLE task stack
    vPortRestoreActiveTask();
#endif
}

/**
 *******************************************************************************
 * IRQ
 *******************************************************************************
 */
volatile uint32_t A_mcause      = 0;    //Debug use
volatile uint32_t A_mdcause     = 0;    //Debug use
volatile uint32_t A_mepc        = 0;    //Debug use
volatile uint32_t A_mtval       = 0;    //Debug use
volatile uint32_t A_mstatus     = 0;    //Debug use
volatile uint32_t A_trap_cnt    = 0;    //Debug use

volatile uint32_t A_mtime_cnt     = 0;    //Debug use
volatile uint32_t A_mswi_cnt     = 0;    //Debug use

#if OS_EN
RAM_CODE
void mtime_handler(void)
{
    A_mtime_cnt++; //Debug use
    /* Update mtimecmp */
    extern void vPortTimerUpdate(void);
    vPortTimerUpdate();

    if(xTaskIncrementTick() != 0){
        vTaskSwitchContext();     //RTOS task scheduling
    }
}


/* Unused: Use "xPortYield()" instead. "xPortYield()" is efficient. */
RAM_CODE
void mswi_handler(void)
{
    A_mswi_cnt++;//Debug use

    vTaskSwitchContext();    //RTOS task scheduling
}
#endif

RAM_CODE  void except_handler(void)
{
    A_mcause  = read_csr(NDS_MCAUSE);
    A_mepc    = read_csr(NDS_MEPC);
    A_mdcause = read_csr(NDS_MDCAUSE);
    A_mtval   = read_csr(NDS_MTVAL);
    A_mstatus = read_csr(NDS_MSTATUS);
    printf("A_mcause =%08lX\r\n",  A_mcause);
    printf("A_mepc   =%08lX\r\n",    A_mepc);
    printf("A_mdcause=%08lX\r\n", A_mdcause);
    printf("A_mtval  =%08lX\r\n",   A_mtval);
    printf("A_mstatus=%08lX\r\n", A_mstatus);
    while(1)
    {
        for(volatile uint32_t i = 0; i < 0xffff; i++)
        {
            A_trap_cnt++;
            __asm__("nop");
        }
        /*It is recommended that this function be enabled for mass-produced products*/
        //sys_reboot();
    }
}
#ifndef tracePort_IrqHandler_BEGIN
#define tracePort_IrqHandler_BEGIN()
#endif
#ifndef tracePort_IrqHandler_END
#define tracePort_IrqHandler_END()
#endif
RAM_CODE
void xPortIrqHandler(uint32_t mcause, uint32_t mepc)
{
    (void) mepc;
    (void) mcause;
    tracePort_IrqHandler_BEGIN();
#if OS_EN
    if(mcause == (MCAUSE_INT + IRQ_M_TIMER))
    {
        /* Machine timer interrupt */
        mtime_handler();
    }
    else if(mcause == (MCAUSE_INT + IRQ_M_SOFT))
    {
        /* Machine SWI interrupt */
        mswi_handler();

        /* Machine SWI is connected to PLIC_SW source 1 */
        soft_irq_complete();
    }
    else
    {
       except_handler();
    }
    tracePort_IrqHandler_END();
#else
     except_handler();
#endif
}
