/********************************************************************************************************
 * @file     tlk_risc.h
 *
 * @brief    This is the header file for BLE SDK
 *
 * @author   BLE GROUP
 * @date         11,2022
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
 *******************************************************************************************************/

#ifndef _RISC_V_TLK_RISCV_H_
#define _RISC_V_TLK_RISCV_H_

#include "types.h"
#include "vendor/common/user_config.h"
#include "driver.h"
#include "ext_driver/ext_pm.h"
#include "compatibility_pack/cmpt.h"
#include "stack/ble/os_sup/os_sup.h"

#if FREERTOS_ENABLE && BLE_APP_PM_ENABLE
    #define OS_PM_EN   1
#endif

#if FREERTOS_ENABLE
    #define OS_EN      1
#endif

#ifndef OS_EN
    #define OS_EN      0
#endif

#ifndef OS_PM_EN
    #define OS_PM_EN   0
#endif

#define MSTATUS_UIE             0x00000001
#define MSTATUS_SIE             0x00000002
#define MSTATUS_HIE             0x00000004
#define MSTATUS_MIE             0x00000008
#define MSTATUS_UPIE            0x00000010
#define MSTATUS_SPIE            0x00000020
#define MSTATUS_HPIE            0x00000040
#define MSTATUS_MPIE            0x00000080
#define MSTATUS_SPP             0x00000100
#define MSTATUS_HPP             0x00000600
#define MSTATUS_MPP             0x00001800
#define MSTATUS_FS              0x00006000
#define MSTATUS_XS              0x00018000
#define MSTATUS_MPRV            0x00020000
#define MSTATUS_PUM             0x00040000
#define MSTATUS_MXR             0x00080000
#define MSTATUS_VM              0x1F000000
#define MSTATUS32_SD            0x80000000

#if __riscv_xlen == 64
#define MCAUSE_INT              0x8000000000000000UL
#define MCAUSE_CAUSE            0x7FFFFFFFFFFFFFFFUL
#else
#define MCAUSE_INT              0x80000000UL
#define MCAUSE_CAUSE            0x7FFFFFFFUL
#endif

#define IRQ_S_SOFT              1
#define IRQ_H_SOFT              2
#define IRQ_M_SOFT              3
#define IRQ_S_TIMER             5
#define IRQ_H_TIMER             6
#define IRQ_M_TIMER             7
#define IRQ_S_EXT               9
#define IRQ_H_EXT               10
#define IRQ_M_EXT               11
#define IRQ_COP                 12
#define IRQ_HOST                13

#define MIP_SSIP                (1 << IRQ_S_SOFT)
#define MIP_HSIP                (1 << IRQ_H_SOFT)
#define MIP_MSIP                (1 << IRQ_M_SOFT)
#define MIP_STIP                (1 << IRQ_S_TIMER)
#define MIP_HTIP                (1 << IRQ_H_TIMER)
#define MIP_MTIP                (1 << IRQ_M_TIMER)
#define MIP_SEIP                (1 << IRQ_S_EXT)
#define MIP_HEIP                (1 << IRQ_H_EXT)
#define MIP_MEIP                (1 << IRQ_M_EXT)


/** @{
 *  Software interrupt
 */
#define PLIC_SW_BASE         0xE6400000
#define PLIC_SW_EN          *(volatile uint32_t*)(PLIC_SW_BASE + 0x2000)
#define PLIC_SW_TRIG        *(volatile uint32_t*)(PLIC_SW_BASE + 0x1000)

typedef struct{
    volatile uint32_t claim_complete;
}plic_sw_cmpl_t;

#define plic_sw_cmpl   ((plic_sw_cmpl_t *)(PLIC_SW_BASE + 0x200004))

static inline void soft_irq_enable(void)
{
    //__asm volatile("csrs mie (1<<11)|(1<<7)(1<<3)");
    //__asm volatile("csrs mstatus (1<<3)");
    PLIC_SW_EN = (1<<1); //BIT(1);
}

static inline  void soft_irq_trigger(void)
{
    PLIC_SW_TRIG = (1<<1);//BIT(1);
}

static inline void soft_irq_complete(void)
{
    plic_sw_cmpl->claim_complete;            //read claim
    plic_sw_cmpl->claim_complete = 1;        //complete
}
/* @} */

static inline unsigned char mtimer_is_irq_enable(void)
{
    uint32_t mie = 0;
    __asm volatile ("csrr %0, mie" :"=r"(mie));
    return (mie & BIT(7)) ? 1:0;
}
/* @ } */

#if (OS_SEPARATE_STACK_SPACE)
#ifndef configISR_PLIC_STACK_SIZE
#define configISR_PLIC_STACK_SIZE   1024
#endif
extern void entry_irq_all(func_isr_t func, unsigned int src);

#define PLIC_ISR_REGISTER_OS(isr, irq_num)                                                                                                     \
__attribute__((section(".ram_code")))  __attribute__((noinline)) __attribute__((naked)) void ISR_ENTRY_NAME(irq_num)(void) __attribute__ ((aligned(4)));                    \
void ISR_ENTRY_NAME(irq_num)(void) {                                                                                                        \
__asm__("c.addi sp,-16");                                                                                                               \
__asm__("c.swsp ra,0(sp)");                                                                                                             \
__asm__("c.swsp a0,4(sp)");                                                                                                             \
__asm__("c.swsp a1,8(sp)");                                                                                                             \
entry_irq_all(isr,irq_num);                                                                                                             \
__asm__("c.lwsp ra,0(sp)");                                                                                                             \
__asm__("c.lwsp a0,4(sp)");                                                                                                             \
__asm__("c.lwsp a1,8(sp)");                                                                                                             \
__asm__("c.addi sp,16");                                                                                                                \
__asm__("mret");                                                                                                                        \
}
#endif //#if (OS_SEPARATE_STACK_SPACE)

#endif /* _TLK_RISCV_H_ */
