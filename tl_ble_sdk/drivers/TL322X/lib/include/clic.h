/********************************************************************************************************
 * @file    clic.h
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
/** @page CLIC
 *
 *  Introduction
 *  ===============
 *   The core-local interrupt controller (CLIC)
 *
 *  API Reference
 *  ===============
 *  Header File: clic.h
 */
#ifndef __CLIC_H__
#define __CLIC_H__
#include "lib/include/core.h"
#include "compiler.h"
#include "reg_include/register.h"
#if defined(MCU_CORE_TL322X_N22)

/**
 * @brief Declare the interrupt service routine type.
 */
typedef void (*func_clic_isr_t)(void);

/**
 * @brief The global variable is used to indicate whether interrupt nesting is supported.
 * @note
 *        - To avoid interrupt nesting failures caused by sign extension, the unsigned long type is used here.
 */
extern _attribute_data_retention_sec_ volatile unsigned long g_clic_preempt_en;

    /**
 * @defgroup irq_source_index Interrupt source index definition
 * @{
 * @brief Defines the number of each interrupt in the system and its corresponding handler function.
 */
    #define IRQ_MTIMER                 7  /* IRQ_MTIMER machine timer interrupt, - mtime_irq_handler */

    #define IRQ_SYSTIMER               19 /* STIMER_IRQ: system timer interrupt, - stimer_irq_handler */
    #define IRQ_ALG                    20 /* ALGM_IRQ: analog register master interface interrupt, - analog_irq_handler */
    #define IRQ_TIMER1                 21 /* TIMER1_IRQ, - timer1_irq_handler */
    #define IRQ_TIMER0                 22 /* TIMER0_IRQ, - timer0_irq_handler */
    #define IRQ_DMA                    23 /* DMA_IRQ_IRQ, - dma_irq_handler */
    #define IRQ_BMC                    24 /* BMC_IRQ: ahb bus matrix controller interrupt, - bmc_irq_handler */
    #define IRQ_USB1_CTRL_EP_SETUP     25 /* USB1_SETUP_IRQ: USB1 setup interrupt, - usb1_ctrl_ep_setup_irq_handler */
    #define IRQ_USB1_CTRL_EP_DATA      26 /* USB1_DATA_IRQ: USB1 data interrupt, - usb1_ctrl_ep_data_irq_handler */
    #define IRQ_USB1_CTRL_EP_STATUS    27 /* USB1_STATUS_IRQ: USB1 status interrupt, - usb1_ctrl_ep_status_irq_handler */
    #define IRQ_USB1_CTRL_EP_SETINF    28 /* USB1_SETINF_IRQ: USB1 set interface interrupt, - usb1_ctrl_ep_setinf_irq_handler */
    #define IRQ_USB1_ENDPOINT          29 /* USB1_EDP_IRQ: USB1 edp (1-8) interrupt, - usb1_endpoint_irq_handler */
    #define IRQ_TIMER_BB               30 /* TRNG_IRQ, - trng_irq_handler */

    #define IRQ_ZB_RT                  33 /* ZB_BLE_TL_IRQ: BLE (TL) sub-system interrupt, - rf_irq_handler */
    #define IRQ_PWM                    34 /* PWM_IRQ, - pwm_irq_handler */
    #define IRQ_PKE                    35 /* PKE_IRQ, - pke_irq_handler */
    #define IRQ_UART1                  36 /* UART1_IRQ, - uart1_irq_handler */
    #define IRQ_UART0                  37 /* UART0_IRQ, - uart0_irq_handler */
    #define IRQ_DFIFO                  38 /* DFIFO_IRQ: audio dma fifo interrupt, - audio_irq_handler */
    #define IRQ_I2C                    39 /* I2C_IRQ, - i2c_irq_handler */
    #define IRQ_LSPI                   40 /* LSPI_IRQ, - lspi_irq_handler */
    #define IRQ_GSPI                   41 /* GSPI_IRQ, - gspi_irq_handler */
    #define IRQ_USB1_PWDN              42 /* USB1_PWDN_IRQ: USB1 suspend interrupt, - usb1_pwdn_irq_handler */
    #define IRQ_QDEC1                  43 /* IRQ_QDEC1, - qdec1_irq_handler */
    #define IRQ_UART4                  44 /* IRQ_UART4, - uart4_irq_handler */
    #define IRQ_I2C1                   45 /* IRQ_I2C1, -  i2c1_irq_handler */
    #define IRQ_SOFT                   46 /* SOFT_IRQ: software interrupt, - soft_irq_handler */
    #define IRQ_MSPI                   47 /* MSPI_IRQ, - mspi_irq_handler */
    #define IRQ_USB1_RESET             48 /* USB1_RESET_IRQ: USB1 reset interrupt, - usb1_reset_irq_handler */
    #define IRQ_USB1_250US_OR_SOF      49 /* USB1_250US_OR_SOF_IRQ: USB1 250us or SOF interrupt, - usb1_250us_or_sof_irq_handler */
    #define IRQ_IR_LEARN               50 /* IR_LEARN_IRQ - ir_learn_irq_handler */
    #define IRQ_QDEC                   51 /* QDEC_IRQ, - qdec_irq_handler */
    #define IRQ_GPIO_SRC0              52 /* GPIO_GROUP_IRQ[0], - gpio_src0_irq_handler */
    #define IRQ_GPIO_SRC1              53 /* GPIO_GROUP_IRQ[1], - gpio_src1_irq_handler */
    #define IRQ_GPIO_SRC2              54 /* GPIO_GROUP_IRQ[2], - gpio_src2_irq_handler */
    #define IRQ_GPIO_SRC3              55 /* GPIO_GROUP_IRQ[3], - gpio_src3_irq_handler */
    #define IRQ_GPIO_SRC4              56 /* GPIO_GROUP_IRQ[4], - gpio_src4_irq_handler */
    #define IRQ_GPIO_SRC5              57 /* GPIO_GROUP_IRQ[5], - gpio_src5_irq_handler */
    #define IRQ_GPIO_SRC6              58 /* GPIO_GROUP_IRQ[6], - gpio_src6_irq_handler */
    #define IRQ_GPIO_SRC7              59 /* GPIO_GROUP_IRQ[7], - gpio_src7_irq_handler */
    #define IRQ_TRNG                   60 /* TRNG_IRQ, - trng_irq_handler */
    #define IRQ_HASH                   61 /* HASH_IRQ, - hash_irq_handler */
    #define IRQ_PM_LVL                 62 /* PM_LVL_IRQ: PM level interrupt, - pm_level_irq_handler */
    #define IRQ_PM_IRQ                 63 /* PM_MIX_IRQ: PM mixed interrupt, - pm_irq_handler */
    #define IRQ_IRQ_MAILBOX_D25_TO_N22 64 /* D25_TO_D22_IRQ, - mailbox_d25_to_n22_irq_handler */
    #define IRQ_SKE                    65 /* SKE_IRQ, - ske_irq_handler */
    #define IRQ_UART2                  66 /* UART2_IRQ, - uart2_irq_handler */
    #define IRQ_IRQ_KEY_SCAN           67 /* KEY_SCAN_IRQ, - key_scan_irq_handler */
    #define IRQ_IRQ_UART3              68 /* IRQ_UART3, - uart3_irq_handler */
    #define IRQ_SARADC_RX              69 /* SARADC_RX_IRQ: sar adc interrupt, - saradc_rx_irq_handler */
    #define IRQ_RZ                     70 /* RZ_IRQ, - rz_irq_handler */
    #define IRQ_GSPI1                  71 /* IRQ_GSPI1, - gspi1_irq_handler */
    #define IRQ_GSPI2                  72 /* IRQ_GSPI2, - gspi2_irq_handler */
    #define IRQ_GSPI3                  73 /* IRQ_GSPI3, - gspi3_irq_handler */
    #define IRQ_GSPI4                  74 /* IRQ_GSPI4, - gspi4_irq_handler */
    #define IRQ_LIN0                   75 /* IRQ_LIN0,  - lin0_irq_handler */
    #define IRQ_LIN1                   76 /* IRQ_LIN1,  - lin1_irq_handler */
    #define IRQ_CAN0                   77 /* IRQ_CAN0,  - can0_irq_handler */
    #define IRQ_CAN1                   78 /* IRQ_CAN1,  - can1_irq_handler */
    #define IRQ_I3C0                   79 /* IRQ_I3C0,  - i3c0_irq_handler */
    #define IRQ_I3C1                   80 /* IRQ_I3C1,  - i3c1_irq_handler */
    #define IRQ_DMA1                   81 /* IRQ_DMA1,  - dma1_irq_handler */
    #define IRQ_NVM                    82 /* IRQ_NVM,   - nvm_irq_handler */

    /**
 * @}
 */

    /**
 * @brief This macro generates a unique name for an interrupt service routine (ISR) entry point based on the input parameter. \n
 * The generated name is in the form "entry_irq" followed by the value of "irq_num". \n
 * For example, ISR_ENTRY_NAME(19) means entry_irq19.
 * @param[in] irq_num - The number of the interrupt see @ref irq_source_index.
 */
    #define ISR_ENTRY_NAME(irq_num) entry_irq##irq_num

    /**
 * @brief Interrupt service routine register, this macro defines an entry_irq function based on irq_num. \n
 *        The machine attribute was added to the interrupt function, and the compiler saw that it would insert code that modified the protection register, for example:
 *        @code {.c}
 *           _attribute_ram_code_sec_noinline_ entry_irq19(void) __attribute__ ((interrupt ("machine") , aligned(4)));
 *        @endcode
 * @param[in] isr - Interrupt service routine pointer.
 * @param[in] irq_num - The number of the interrupt see @ref irq_source_index.
 * @note
 *      - The same interruption number can only be registered once.
 *      - The code in the interrupt service handler should be as simple as possible, and if you can do it without function calls, you can reduce the number of registers on the stack, \n
 *        thus reducing the code size and speeding up the interrupt response.
 */

    #define CLIC_ISR_REGISTER(isr, irq_num)                                                                                     \
        _attribute_ram_code_sec_noinline_ void ISR_ENTRY_NAME(irq_num)(void) __attribute__((interrupt("machine"), aligned(4))); \
        void                                   ISR_ENTRY_NAME(irq_num)(void)                                                    \
        {                                                                                                                       \
            clic_isr(isr);                                                                                                      \
        }

/**
 * @brief Interrupt source level. The larger the value, the higher the level. After the reset the default level value is 0.
 * @note
 *        - An interrupt occurs only when level is greater than threshold.
 *        - The clic_init function sets the level of all interrupts to 1.
 */
typedef enum
{
    IRQ_PRI_LEV0, /**< 0 indicates that no interrupt is generated. */
    IRQ_PRI_LEV1,
    IRQ_PRI_LEV2,
    IRQ_PRI_LEV3,
} irq_priority_e;

/**
 * @brief Target interrupt priority threshold. The larger the value, the higher the threshold. The default threshold value is 0.
 */
typedef enum
{
    IRQ_PRI_NUM0,
    IRQ_PRI_NUM1,
    IRQ_PRI_NUM2,
    IRQ_PRI_NUM3,
} irq_threshold_e;

/**
 * @brief   This function serves to init CLIC.
 * @return  none
 * @note
 *          - The default value of irq level is set to 1 for all interrupts.
 */
void clic_init(void);

/**
 * @brief      This function serves to select vector mode for the associated interrupt.
 * @param[in]  src - Interrupt source see @ref irq_source_index.
 * @return     none
 * @note       In vector mode, the interrupt service routine needs to be registered with CLIC_ISR_REGISTER().
 */
static inline void clic_interrupt_vector_en(unsigned int src)
{
    BM_SET(reg_clic_attribute(src), FLD_CLIC_SHV);
}

/**
 * @brief      This function serves to set CLIC pending. It provides a way for software to trigger an interrupt without relying on interrupt source from external devices.
 * @param[in]  src - Interrupt source see @ref irq_source_index.
 * @return     none
 */
static inline void clic_set_pending(unsigned int src)
{
    reg_clic_pending(src) = 1;
}

/**
 * @brief      This function serves to enable CLIC interrupt source.
 * @param[in]  src - Interrupt source see @ref irq_source_index.
 * @return     none
 */
static inline void clic_interrupt_enable(unsigned int src)
{
    reg_clic_src(src) = 1;
}

/**
 * @brief      This function serves to disable CLIC interrupt source.
 * @param[in]  src - Interrupt source see @ref irq_source_index.
 * @return     none
 */
static inline void clic_interrupt_disable(unsigned int src)
{
    reg_clic_src(src) = 0;
}

/**
 * @brief       This function serves to set external interrupts preemptive priority level. The priority value 0 is reserved to mean never interrupt.\n
 *              The larger the priority value, the higher the interrupt priority, the default priority value is set to 1 by the software.
 * @param[in]   src      - Interrupt source see @ref irq_source_index.
 * @param[in]   priority - Priority level.
 * @return      none
 */
static inline void clic_set_priority(unsigned int src, irq_priority_e priority)
{
    reg_clic_input_ctl(src) = (reg_clic_input_ctl(src) & (~FLD_CLIC_LEVEL)) | MASK_VAL(FLD_CLIC_LEVEL, priority);
}

/**
 * @brief      This function serves to set priority threshold, only active interrupts with interrupt levels(clic_set_priority()) strictly greater than the threshold will cause interrupt.
 * @param[in]  threshold -  Threshold level.
 * @return     none
 */
static inline void clic_set_threshold(irq_threshold_e threshold)
{
    reg_clic_mth = (reg_clic_mth & (~FLD_CLIC_MTH)) | MASK_VAL(FLD_CLIC_MTH, threshold);
}

/**
 * @brief    This function serves to enable preemptive priority interrupt feature.
 * @return   none
 */
static inline void clic_preempt_feature_en(void)
{
    g_clic_preempt_en = 1;
}

/**
 * @brief    This function serves to disable preemptive priority interrupt feature.
 * @return   none
 */
static inline void clic_preempt_feature_dis(void)
{
    g_clic_preempt_en = 0;
}

/**
 * @brief       This function serves to execute the interrupt service routine, you can call this function when an interrupt occurs.
 * @param[in]   func - Interrupt service routine.
 * @return      none
 */
_attribute_ram_code_sec_ void clic_isr(func_clic_isr_t func);
#endif

#endif
