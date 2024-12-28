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
#include "app_config.h"
#include "app.h"

#if (FEATURE_TEST_MODE == TEST_GATT_API)


/**
 * @brief       BLE RF interrupt handler.
 * @param[in]   none
 * @return      none
 */
_attribute_ram_code_ void rf_irq_handler(void)
{
    DBG_CHN14_HIGH;

    blc_sdk_irq_handler ();

    DBG_CHN14_LOW;
}
PLIC_ISR_REGISTER(rf_irq_handler, IRQ_ZB_RT)
/**
 * @brief       System timer interrupt handler.
 * @param[in]   none
 * @return      none
 */
_attribute_ram_code_ void stimer_irq_handler(void)
{
    DBG_CHN15_HIGH;

    blc_sdk_irq_handler ();

    DBG_CHN15_LOW;
}
PLIC_ISR_REGISTER(stimer_irq_handler, IRQ_SYSTIMER)


/**
 * @brief      application system initialization
 * @param[in]  none.
 * @return     none.
 */
__INLINE void blc_app_system_init(void)
{
    #if (MCU_CORE_TYPE == MCU_CORE_B91)
        sys_init(DCDC_1P4_LDO_1P8, VBAT_MAX_VALUE_GREATER_THAN_3V6,INTERNAL_CAP_XTAL24M);
        CCLK_48M_HCLK_48M_PCLK_24M;
    #elif (MCU_CORE_TYPE == MCU_CORE_B92)
        sys_init(DCDC_1P4_LDO_2P0, VBAT_MAX_VALUE_GREATER_THAN_3V6, GPIO_VOLTAGE_3V3, INTERNAL_CAP_XTAL24M);
        wd_32k_stop();
        CCLK_48M_HCLK_48M_PCLK_24M;
    #elif (MCU_CORE_TYPE == MCU_CORE_TL721X)
        sys_init(DCDC_0P94_DCDC_1P8,VBAT_MAX_VALUE_GREATER_THAN_3V6,INTERNAL_CAP_XTAL24M);
        pm_update_status_info(1);
        gpio_set_up_down_res(GPIO_SWS, GPIO_PIN_PULLUP_1M);
        wd_32k_stop();
        wd_stop();
        PLL_240M_CCLK_48M_HCLK_48M_PCLK_48M_MSPI_48M;
    #elif(MCU_CORE_TYPE == MCU_CORE_TL321X)
        sys_init(DCDC_1P25_LDO_1P8, VBAT_MAX_VALUE_GREATER_THAN_3V6, INTERNAL_CAP_XTAL24M);
        pm_update_status_info(1);
        gpio_set_up_down_res(GPIO_SWS, GPIO_PIN_PULLUP_1M);
        wd_32k_stop();
        wd_stop();
        PLL_192M_CCLK_48M_HCLK_24M_PCLK_24M_MSPI_48M;
    #endif
}


/**
 * @brief       This is main function
 * @param[in]   none
 * @return      none
 */
_attribute_ram_code_ int main(void)
{
    /* this function must called before "sys_init()" when:
     * (1). For all IC: using 32K RC for power management,
       (2). For B91 only: even no power management */
    blc_pm_select_internal_32k_crystal();

    blc_app_system_init();

    /* detect if MCU is wake_up from deep retention mode */
    int deepRetWakeUp = pm_is_MCU_deepRetentionWakeup();  //MCU deep retention wakeUp


    rf_drv_ble_init();

    gpio_init(!deepRetWakeUp);

    if( deepRetWakeUp ){ //MCU wake_up from deepSleep retention mode
        user_init_deepRetn ();
    }
    else{ //MCU power_on or wake_up from deepSleep mode
        user_init_normal();
    }


    irq_enable();

    while(1)
    {
        main_loop ();
    }
    return 0;
}

#endif //end of (FEATURE_TEST_MODE == ...)
