/********************************************************************************************************
 * @file    main.c
 *
 * @brief   This is the source file for 2.4G SDK
 *
 * @author  2.4G Group
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
#include "app_config.h"
#include "tl_common.h"
#include "drivers.h"
extern void user_init_deepRetn(void);
extern void user_init_normal(void);
extern void main_loop(void);

#if (RF_DEBUG_IO_ENABLE)
//debug_io
#if (MCU_CORE_TYPE == MCU_CORE_TL323X)
_attribute_ram_code_sec_ void rf_dbg_bb_set_pin(gpio_pin_e pin)
{
    unsigned char mask = 0x3F;//bit5~bit0
    unsigned char val = 0x39;//function select DBG_BB
    //note:  setting pad the function  must before  setting no_gpio function, cause it will lead to uart transmit extra one byte data at begin.(confirmed by minghai&sunpeng)
    reg_gpio_func_mux(pin) = (reg_gpio_func_mux(pin) & mask) | val;
    gpio_function_dis(pin);
}

_attribute_ram_code_sec_ void rf_enable_debug_IO(void)
{
    REG_ADDR8(0x170003) |= BIT(5);//default 0x0

    REG_ADDR8(0x140d01) &= 0x0F;        // 140379[7:4] = 0
    REG_ADDR8(0x140d01) |= BIT(0);        // 140379[0] = 1; if not set, B2/B3 not work.
    REG_ADDR8(0x140d00) |= BIT(1);        // 140378[1] = 1
    REG_ADDR8(0x140d00) |= BIT(2);        // 140378[2] = 1
    REG_ADDR8(0x140d00) &= ~BIT(5);    // 140378[5] = 0

    rf_dbg_bb_set_pin(GPIO_PA0); //tx_en
    rf_dbg_bb_set_pin(GPIO_PA1); //tx_on
    rf_dbg_bb_set_pin(GPIO_PA2); //rx_en
    rf_dbg_bb_set_pin(GPIO_PA3); //clk_bb
    rf_dbg_bb_set_pin(GPIO_PA4); //hit_sync

    rf_dbg_bb_set_pin(GPIO_PB0); //sclk
    rf_dbg_bb_set_pin(GPIO_PB1); //tx_data
    rf_dbg_bb_set_pin(GPIO_PB2); //rx_data_vld
    rf_dbg_bb_set_pin(GPIO_PB3); //rx_data
}
#else
_attribute_ram_code_sec_ void dbg_io_set_pin(gpio_pin_e pin)
{
    unsigned char mask = 0x3F; //bit5~bit0
    unsigned char val  = 0x4d; //function select DBG_BB
    //note:  setting pad the function  must before  setting no_gpio function, cause it will lead to uart transmit extra one byte data at begin.(confirmed by minghai&sunpeng)
    reg_gpio_func_mux(pin) = (reg_gpio_func_mux(pin) & mask) | val;
    gpio_function_dis(pin);
}

_attribute_ram_code_sec_ void rf_enable_debug_IO(void)
{
    REG_ADDR8(0x170003) |= BIT(5);  //default 0x0

    REG_ADDR8(0x140ca1) &= 0x0F;    // 140379[7:4] = 0
    REG_ADDR8(0x140ca1) |= BIT(0);  // 140379[0] = 1; if not set, B2/B3 not work.
    REG_ADDR8(0x140ca0) |= BIT(1);  // 140378[1] = 1
    REG_ADDR8(0x140ca0) |= BIT(2);  // 140378[2] = 1
    REG_ADDR8(0x140ca0) &= ~BIT(5); // 140378[5] = 0

    dbg_io_set_pin(GPIO_PA0);       //tx_en
    dbg_io_set_pin(GPIO_PA1);       //tx_on
    dbg_io_set_pin(GPIO_PA2);       //rx_en
    dbg_io_set_pin(GPIO_PA3);       //clk_bb
    dbg_io_set_pin(GPIO_PA4);       //hit_sync

//    dbg_io_set_pin(GPIO_PB0);       //sclk
    dbg_io_set_pin(GPIO_PB1);       //tx_data
    dbg_io_set_pin(GPIO_PB2);       //rx_data_vld
    dbg_io_set_pin(GPIO_PB3);       //rx_data
}
#endif // (MCU_CORE_TYPE)
#endif

#if (APP_FLASH_PROTECTION_ENABLE)

/**
 * @brief      flash protection operation, including all locking & unlocking for application
 *             handle all flash write & erase action for this demo code. use should add more more if they have more flash operation.
 * @param[in]  flash_op_evt - flash operation event, including application layer action and stack layer action event(OTA write & erase)
 *             attention 1: if you have more flash write or erase action, you should should add more type and process them
 *             attention 2: for "end" event, no need to pay attention on op_addr_begin & op_addr_end, we set them to 0 for
 *                          stack event, such as stack OTA write new firmware end event
 * @param[in]  op_addr_begin - operating flash address range begin value
 * @param[in]  op_addr_end - operating flash address range end value
 *             attention that, we use: [op_addr_begin, op_addr_end)
 *             e.g. if we write flash sector from 0x10000 to 0x20000, actual operating flash address is 0x10000 ~ 0x1FFFF
 *                  but we use [0x10000, 0x20000):  op_addr_begin = 0x10000, op_addr_end = 0x20000
 * @return     none
 */
_attribute_data_retention_ u16 flash_lockBlock_cmd;

void app_flash_protection_operation(u8 flash_op_evt, u32 op_addr_begin, u32 op_addr_end)
{
    (void)op_addr_begin;
    (void)op_addr_end;
    if (flash_op_evt == FLASH_OP_EVT_APP_INITIALIZATION) {
        /* ignore "op addr_begin" and "op addr_end" for initialization event
         * must call "flash protection_init" first, will choose correct flash protection relative API according to current internal flash type in MCU */
        flash_protection_init();

        /* just sample code here, protect all flash area for old firmware and OTA new firmware.
         * user can change this design if have other consideration */
        u32 app_lockBlock = FLASH_LOCK_FW_LOW_512K; //just demo value, user can change this value according to application

        flash_lockBlock_cmd = flash_change_app_lock_block_to_flash_lock_block(app_lockBlock);

        if (blc_flashProt.init_err) {
            tlkapi_printf(APP_FLASH_PROT_LOG_EN, "[FLASH][PROT] flash protection initialization error!!!\n");
        }

        tlkapi_printf(APP_FLASH_PROT_LOG_EN, "[FLASH][PROT] initialization, lock flash\n");
        flash_lock(flash_lockBlock_cmd);
    }
    /* add more flash protection operation for your application if needed */
}
#endif

/**
 * @brief      application system initialization
 * @param[in]  none.
 * @return     none.
 */
__INLINE void blc_app_system_init(void)
{
#if (MCU_CORE_TYPE == MCU_CORE_B91)
    sys_init(DCDC_1P4_LDO_1P8, VBAT_MAX_VALUE_GREATER_THAN_3V6, INTERNAL_CAP_XTAL24M);
    gpio_set_up_down_res(GPIO_SWS, GPIO_PIN_PULLUP_1M);
    wd_stop();
    CCLK_32M_HCLK_32M_PCLK_16M;
#elif (MCU_CORE_TYPE == MCU_CORE_B92)
    sys_init(DCDC_1P4_LDO_2P0, VBAT_MAX_VALUE_GREATER_THAN_3V6, GPIO_VOLTAGE_3V3, INTERNAL_CAP_XTAL24M);
    pm_update_status_info(1);
    gpio_set_up_down_res(GPIO_SWS, GPIO_PIN_PULLUP_1M);
    wd_32k_stop();
    CCLK_32M_HCLK_32M_PCLK_16M;
#elif (MCU_CORE_TYPE == MCU_CORE_TL721X)
    sys_init(DCDC_0P94_DCDC_1P8, VBAT_MAX_VALUE_GREATER_THAN_3V6, INTERNAL_CAP_XTAL24M);
    pm_update_status_info(1);
    gpio_set_up_down_res(GPIO_SWS, GPIO_PIN_PULLUP_1M);
    wd_32k_stop();
    wd_stop();
    PLL_240M_CCLK_48M_HCLK_48M_PCLK_48M_MSPI_48M;
#elif (MCU_CORE_TYPE == MCU_CORE_TL321X)
    sys_init(DCDC_1P25_LDO_1P8, VBAT_MAX_VALUE_GREATER_THAN_3V6, INTERNAL_CAP_XTAL24M);
    pm_update_status_info(1);
    gpio_set_up_down_res(GPIO_SWS, GPIO_PIN_PULLUP_1M);
    wd_32k_stop();
    wd_stop();
    PLL_192M_CCLK_48M_HCLK_24M_PCLK_24M_MSPI_48M;
#elif (MCU_CORE_TYPE == MCU_CORE_TL322X)
    sys_init(DCDC_1P25_LDO_1P8, VBAT_MAX_VALUE_GREATER_THAN_3V6, INTERNAL_CAP_XTAL24M);
    pm_update_status_info(1);
    gpio_set_up_down_res(GPIO_SWS, GPIO_PIN_PULLUP_1M);
    wd_32k_stop();
    wd_stop();
    PLL_192M_D25F_64M_HCLK_N22_32M_PCLK_32M_MSPI_48M;

    #if !defined(TLK_ONLY_BLE_HOST)
        pm_set_dig_module_power_switch(FLD_PD_ZB_EN, PM_POWER_UP); //Temporarily placed, wait for the driver to confirm afterwards
        rf_n22_dig_init();
    #endif
#elif (MCU_CORE_TYPE == MCU_CORE_TL323X)
    sys_init(DCDC_1P25_LDO_1P8, VBAT_MAX_VALUE_GREATER_THAN_3V6, INTERNAL_CAP_XTAL24M);
    pm_update_status_info(1);
    gpio_set_up_down_res(GPIO_SWS, GPIO_PIN_PULLUP_1M);
    wd_32k_stop();
    wd_stop();
    PLL_192M_CCLK_48M_HCLK_24M_PCLK_12M_MSPI_48M;
#elif (MCU_CORE_TYPE == MCU_CORE_TL521X)
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
    blc_app_system_init();

    /* detect if MCU is wake_up from deep retention mode */
    int deepRetWakeUp = pm_is_MCU_deepRetentionWakeup(); //MCU deep retention wakeUp

    gpio_init(!deepRetWakeUp);

#if (APP_FLASH_PROTECTION_ENABLE)
    app_flash_protection_operation(FLASH_OP_EVT_APP_INITIALIZATION, 0, 0);
    blc_appRegisterStackFlashOperationCallback(app_flash_protection_operation); //register flash operation callback for stack
#endif

    gpio_set_gpio_en(GPIO_PB4 | GPIO_PB5 | GPIO_PB6 | GPIO_PB7);
    gpio_set_output_en(GPIO_PB4 | GPIO_PB5 | GPIO_PB6 | GPIO_PB7, 1);
    gpio_write(GPIO_PB4 | GPIO_PB5 | GPIO_PB6 | GPIO_PB7, 0);

    if (deepRetWakeUp) {                                                        //MCU wake_up from deepSleep retention mode
        user_init_deepRetn();
    } else {                                                                    //MCU power_on or wake_up from deepSleep mode
        user_init_normal();
    }

#if (RF_DEBUG_IO_ENABLE)
    rf_enable_debug_IO();
#endif

    while (1) {
        main_loop();
    }
    return 0;
}
