/********************************************************************************************************
 * @file    main.c
 *
 * @brief   This is the source file for BLE SDK
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
#include "tl_common.h"
#include "drivers.h"
#include "stack/ble/ble.h"
#include "app.h"


#if (FREERTOS_ENABLE)
    #include "tlk_riscv.h"
    #include <FreeRTOS.h>
    #include <task.h>
    #include "app_freertos.h"
#endif
/**
 * @brief       BLE RF interrupt handler.
 * @param[in]   none
 * @return      none
 */
_attribute_ram_code_ void rf_irq_handler(void)
{
    DBG_CHN14_HIGH;
    blc_sdk_irq_handler();
    DBG_CHN14_LOW;
}
#if (FREERTOS_ENABLE)
PLIC_ISR_REGISTER_OS(rf_irq_handler, IRQ_ZB_RT)
#else
PLIC_ISR_REGISTER(rf_irq_handler, IRQ_ZB_RT)
#endif
/**
 * @brief       System timer interrupt handler.
 * @param[in]   none
 * @return      none
 */
_attribute_ram_code_ void stimer_irq_handler(void)
{
    DBG_CHN15_HIGH;
    blc_sdk_irq_handler();
    DBG_CHN15_LOW;
}
#if (FREERTOS_ENABLE)
PLIC_ISR_REGISTER_OS(stimer_irq_handler, IRQ_SYSTIMER)
#else
PLIC_ISR_REGISTER(stimer_irq_handler, IRQ_SYSTIMER)
#endif

/**
 * @brief      application system initialization
 * @param[in]  none.
 * @return     none.
 */
__INLINE void blc_app_system_init(void)
{
#if (MCU_CORE_TYPE == MCU_CORE_TL521X)
    sys_init(DCDC_1P25_LDO_1P8, VBAT_MAX_VALUE_GREATER_THAN_3V6, INTERNAL_CAP_XTAL24M);
    pm_update_status_info(1);
    gpio_shutdown(GPIO_ALL);
    gpio_set_up_down_res(GPIO_SWS, GPIO_PIN_PULLUP_1M);
    wd_32k_stop();
    wd_stop();
    PLL_144M_CCLK_48M_HCLK_24M_PCLK_12M_MSPI_48M;
#else
    #error "Not Supported Chip!!!"
#endif
}

/**
 * @brief       This is main function
 * @param[in]   none
 * @return      none
 */
_attribute_ram_code_ int main(void)
{
    DBG_CHN0_LOW;

    /* this function must called before "sys_init()" when:
     * (1). For all IC: using 32K RC for power management,
       (2). For B91 only: even no power management */
    blc_pm_select_internal_32k_crystal();

    blc_app_system_init();

    /* detect if MCU is wake_up from deep retention mode */
    int deepRetWakeUp = pm_is_MCU_deepRetentionWakeup(); //MCU deep retention wakeUp

    #if !defined(TLK_ONLY_BLE_HOST)
    /* Place the RF in the N22 initialization. */
    rf_drv_ble_init();
    #endif

    gpio_init(!deepRetWakeUp);

    gpio_function_en(GPIOE_ALL);
    gpio_output_en(GPIOE_ALL); //enable output
    gpio_input_dis(GPIOE_ALL); //disable input

    if (deepRetWakeUp) { //MCU wake_up from deepSleep retention mode
#if (FREERTOS_ENABLE)
        extern void vPortRestoreTick(void);
        vPortRestoreTick();
#endif
        user_init_deepRetn();
    } else { //MCU power_on or wake_up from deepSleep mode
        user_init_normal();
    }

    irq_enable();

#if (FREERTOS_ENABLE)
    app_TaskCreate();

    vTaskStartScheduler();
    while (1);
#else

    while (1) {
        main_loop();

        delay_us(2);
    }
#endif
    return 0;
}
