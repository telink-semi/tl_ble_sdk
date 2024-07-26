/********************************************************************************************************
 * @file    pm.h
 *
 * @brief   This is the header file for TL721X
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

#include "reg_include/register.h"
#include "compiler.h"
#include "gpio.h"
#include "clock.h"


/**
 * @brief these analog register can store data in deep sleep mode or deep sleep with SRAM retention mode.
 *        Reset these analog registers by watchdog, software reboot (sys_reboot()), RESET Pin, power cycle, 32k watchdog, vbus detect.
 */
/**
 * Customers cannot use analog register 0x35 because driver and chip functions are occupied, details are as follows:
 * [Bit0]: If this bit is 1, it means that reboot or power on has occurred. If this bit is 0, it means that sleep has occurred.
 * [Bit1~7]: These bits are used by the driver and cannot be used by the customer.
 */
#define PM_ANA_REG_WD_CLR_BUF0          0x35 // initial value 0xff.
#define PM_ANA_REG_WD_CLR_BUF1          0x36 // initial value 0x00.
#define PM_ANA_REG_WD_CLR_BUF2          0x37 // initial value 0x00.
#define PM_ANA_REG_WD_CLR_BUF3          0x38 // initial value 0x00.
#define PM_ANA_REG_WD_CLR_BUF4          0x39 // initial value 0x00.

/**
 * @brief analog register below can store information when MCU in deep sleep mode or deep sleep with SRAM retention mode.
 *        Reset these analog registers by power cycle, 32k watchdog, RESET Pin,vbus detect.
 */
/**
 * Customers cannot use analog register 0x3a because driver and chip functions are occupied, details are as follows:
 * [Bit0]: If this bit is 1, it means that reboot has occurred.
 * [Bit1]: If this bit is 1, it means that the software calls the function sys_reboot() when the crystal oscillator does not start up normally.
 * [Bit2]: If this bit is 1, it means that the pm_sleep_wakeup function failed to clear the pm wake flag bit when using the deep wake source, and the software called sys_reboot().
 * [Bit3~7]: These bits are used by the driver and cannot be used by the customer.
 */
#define PM_ANA_REG_POWER_ON_CLR_BUF0    0x3a // initial value 0x00.
#define PM_ANA_REG_POWER_ON_CLR_BUF1    0x3b // initial value 0x00.
#define PM_ANA_REG_POWER_ON_CLR_BUF2    0x3c // initial value 0xff.

/**
 * @brief   gpio wakeup level definition
 */
typedef enum{
    WAKEUP_LEVEL_LOW        = 0,
    WAKEUP_LEVEL_HIGH       = 1,
}pm_gpio_wakeup_level_e;

/**
 * @brief   wakeup tick type definition
 */
typedef enum {
     PM_TICK_STIMER         = 0,    // 24M
     PM_TICK_32K            = 1,
}pm_wakeup_tick_type_e;

/**
 * @brief   sleep mode.
 */
typedef enum {
    //available mode for customer
    SUSPEND_MODE                        = 0x00,
    DEEPSLEEP_MODE                      = 0xf0, //when use deep mode pad wakeup(low or high level), if the high(low) level always in the pad,
                                                //system will not enter sleep and go to below of pm API, will reboot by core_6f = 0x20.
                                                //deep retention also had this issue, but not to reboot.
    DEEPSLEEP_MODE_RET_SRAM_LOW32K      = 0x01, //for boot from sram
    DEEPSLEEP_MODE_RET_SRAM_LOW64K      = 0x03, //for boot from sram
    DEEPSLEEP_MODE_RET_SRAM_LOW128K     = 0x07, //for boot from sram
    DEEPSLEEP_MODE_RET_SRAM_LOW256K     = 0x0f, //for boot from sram
    //not available mode
    DEEPSLEEP_RETENTION_FLAG            = 0x0F,
}pm_sleep_mode_e;

/**
 * @brief   available wake-up source for customer
 */
typedef enum {
     PM_WAKEUP_PAD          = BIT(0),
     PM_WAKEUP_CORE         = BIT(1),
     PM_WAKEUP_TIMER        = BIT(2),
     PM_WAKEUP_COMPARATOR   = BIT(3),   /**<
                                            There are two things to note when using LPC wake up:
                                            1.After the LPC is configured, you need to wait 100 microseconds before go to sleep because the LPC need 1-2 32k tick to calculate the result.
                                              If you enter the sleep function at this time, you may not be able to sleep normally because the data in the result register is abnormal.

                                            2.When entering sleep, keep the input voltage and reference voltage difference must be greater than 30mV, otherwise can not enter sleep normally, crash occurs.
                                          */
//   PM_WAKEUP_SHUTDOWN     = BIT(7),
}pm_sleep_wakeup_src_e;

/**
 * @brief   wakeup status
 */
typedef enum {
    WAKEUP_STATUS_PAD               = FLD_WAKEUP_STATUS_PAD,
    WAKEUP_STATUS_CORE              = FLD_WAKEUP_STATUS_CORE,
    WAKEUP_STATUS_TIMER             = FLD_WAKEUP_STATUS_TIMER,
    WAKEUP_STATUS_COMPARATOR        = FLD_WAKEUP_STATUS_COMPARATOR,
    WAKEUP_STATUS_ALL               = FLD_WAKEUP_STATUS_ALL,
    WAKEUP_STATUS_INUSE_ALL         = FLD_WAKEUP_STATUS_INUSE_ALL,

    STATUS_GPIO_ERR_NO_ENTER_PM     = BIT(8), /**<Bit8 is used to determine whether the wake source is normal.*/
    STATUS_EXCEED_MAX               = BIT(27),
    STATUS_EXCEED_MIN               = BIT(28),
    STATUS_CLEAR_FAIL               = BIT(29),
    STATUS_ENTER_SUSPEND            = BIT(30),
}pm_suspend_wakeup_status_e;

/**
 * @brief   mcu status
 */
typedef enum{
    MCU_STATUS_POWER_ON         = BIT(0), /**<  power on, vbus detect or reset pin */
    //BIT(1) RSVD
    MCU_STATUS_REBOOT_BACK      = BIT(2), /**<  the reboot specific categories,see pm_reboot_event_e:
                                                1.If want to know which reboot it is, call the pm_get_mcu_reboot_status() interface to determine after calling sys_init().
                                                2.If determine whether is 32k watchdog/timer watchdog,can also use the interface wd_32k_get_status()/wd_get_status() to determine.
                                                */
    MCU_STATUS_DEEPRET_BACK     = BIT(3),
    MCU_STATUS_DEEP_BACK        = BIT(4),
}pm_mcu_status;

/**
 * @brief  reboot status
 */
typedef enum{
    SW_SYSTEM_REBOOT            = BIT(0),/**< Clear the watchdog status flag in time, otherwise, the system reboot may be wrongly judged as the watchdog.*/
    HW_TIMER_WATCHDOG_REBOOT    = BIT(1),
    HW_32K_WATCHDOG_REBOOT      = BIT(2),/**< - When the 32k watchdog/timer watchdog status is set to 1, if it is not cleared:
                                              - power cyele/vbus detect/reset pin come back, the status is lost;
                                              - but software reboot(sys_reboot())/deep/deepretation/32k watchdog come back,the status remains;
                                              */
}pm_reboot_event_e;

/**
 * @brief power sel
 * 
 */
typedef enum{
    PM_POWER_UP         = 0,
    PM_POWER_DOWN       = 1,
}pm_power_sel_e;

/**
 * @brief voltage select
 *
 */
typedef enum {
    VDD_BASEBAND    =   0x01,
    VDD_BB_PLL      =   0x02,
    VDD_CORE        =   0x03,
    VDD_RETRAM      =   0x04,
    VDD_ISO         =   0x05,
    VDD_RAM         =   0x06,
}pm_vol_mux_sel_e;

/**
 * @brief Selection of optimization options
 *
 */
typedef enum{
    PM_OPTIMIZE_O2      = 0,
    PM_OPTIMIZE_OS      = 1,
}pm_optimize_sel_e;

/**
 * @brief   early wakeup time
 */
typedef struct {
    unsigned short  suspend_early_wakeup_time_us;   /**< suspend_early_wakeup_time_us = deep_ret_r_delay_us + xtal_stable_time + early_time*/
    unsigned short  deep_ret_early_wakeup_time_us;  /**< deep_ret_early_wakeup_time_us = deep_ret_r_delay_us + early_time*/
    unsigned short  deep_early_wakeup_time_us;      /**< deep_early_wakeup_time_us = suspend_ret_r_delay_us*/
    unsigned short  sleep_min_time_us;              /**< sleep_min_time_us = suspend_early_wakeup_time_us + 200*/
}pm_early_wakeup_time_us_s;

extern volatile pm_early_wakeup_time_us_s g_pm_early_wakeup_time_us;
/**
 * @brief   hardware delay time
 */
typedef struct {
    unsigned short  deep_r_delay_cycle ;            /**< hardware delay time ,deep_ret_r_delay_us = deep_r_delay_cycle * 1/16k */
    unsigned short  suspend_ret_r_delay_cycle ;     /**< hardware delay time ,suspend_ret_r_delay_us = suspend_ret_r_delay_cycle * 1/16k */
    unsigned short  deep_xtal_delay_cycle ;         /**< hardware delay time ,deep_ret_xtal_delay_us = deep_xtal_delay_cycle * 1/16k */
    unsigned short  suspend_ret_xtal_delay_cycle ;  /**< hardware delay time ,suspend_ret_xtal_delay_us = suspend_ret_xtal_delay_cycle * 1/16k */
}pm_r_delay_cycle_s;

extern volatile pm_r_delay_cycle_s g_pm_r_delay_cycle;
/**
 * @brief   deep sleep wakeup status
 */
typedef struct{
    unsigned char is_pad_wakeup;
    unsigned char wakeup_src;   //The pad pin occasionally wakes up abnormally in A0. The core wakeup flag will be incorrectly set in A0.
    unsigned char mcu_status;
    unsigned char rsvd;
}pm_status_info_s;

extern _attribute_aligned_(4) pm_status_info_s g_pm_status_info;
extern _attribute_data_retention_sec_ unsigned char g_pm_vbat_v;


/**
 * @brief       This function serves to get deep retention flag.
 * @return      1 deep retention, 0 deep.
 */
static inline unsigned char pm_get_deep_retention_flag(void)
{
    return !(analog_read_reg8(0x7f) & BIT(0));
}

/**
 * @brief       This function serves to get wakeup source.
 * @return      wakeup source.
 */
static _always_inline pm_wakeup_status_e pm_get_wakeup_src(void)
{
    return ((pm_wakeup_status_e)analog_read_reg8(0x64));
}

/**
 * @brief       This function serves to clear the wakeup bit.
 * @param[in]   status  - the interrupt status that needs to be cleared.
 * @return      none.
 */
static _always_inline void pm_clr_irq_status(pm_wakeup_status_e status)
{
    analog_write_reg8(0x64, status);
}

/**
 * @brief       This function serves to set the wakeup source.
 * @param[in]   wakeup_src  - wake up source select.
 * @return      none.
 */
static inline void pm_set_wakeup_src(pm_sleep_wakeup_src_e wakeup_src)
{
    analog_write_reg8(0x4b, wakeup_src);
}

/**
 * @brief       This function configures a GPIO pin as the wakeup pin.
 * @param[in]   pin - the pins can be set to all GPIO except PB0/PC5 and GPIOG groups.
 * @param[in]   pol - the wakeup polarity of the pad pin(0: low-level wakeup, 1: high-level wakeup).
 * @param[in]   en  - enable or disable the wakeup function for the pan pin(1: enable, 0: disable).
 * @return      none.
 */
void pm_set_gpio_wakeup (gpio_pin_e pin, pm_gpio_wakeup_level_e pol, int en);

/**
 * @brief       This function serves to recover system timer.
 *              The code is placed in the ram code section, in order to shorten the time.
 * @return      none.
 */
_attribute_ram_code_sec_noinline_ void pm_stimer_recover(void);

/**
 * @brief       This function configures pm wakeup time parameter.
 * @param[in]   param - deep/suspend/deep_retention r_delay time.(default value: suspend/deep_ret=3, deep=11)
 * @return      none.
 */
void pm_set_wakeup_time_param(pm_r_delay_cycle_s param);

/**
 * @brief       This function is used in applications where the crystal oscillator is relatively slow to start.
 *              When the start-up time is very slow, you can call this function to avoid restarting caused
 *              by insufficient crystal oscillator time (it is recommended to leave a certain margin when setting).
 * @param[in]   delay_us - The time wait for xtal stable and flash restore to the active working state in the ramcode
 *                          when wakeup from suspend sleep (default value: 200).
 * @param[in]   loopnum - The time for the crystal oscillator to stabilize is approximately: loopnum*40us(default value: loopnum=10).
 * @return      none.
 * @note        Those parameters will be lost after reboot or deep sleep, so it required to be reconfigured.
 */
void pm_set_xtal_stable_timer_param(unsigned int delay_us, unsigned int loopnum);

/**
 * @brief       This function is used to configure data about code runtime updates when os optimization options are used.
 * @param[in]   optimization - Currently selected optimization options. O2 or Os.
 * @return      none.
 * @note        Those parameters will be lost after reboot or deep sleep, so it required to be reconfigured.
 */
void pm_set_cfg_for_os_compile_opt(pm_optimize_sel_e optimization);

/**
 * @brief       This function serves to set baseband/usb/npe power on/off before suspend sleep,If power
 *              on this module,the suspend current will increase;power down this module will save current,
 *              but you need to re-init this module after suspend wakeup.All module is power down default
 *              to save current.
 * @param[in]   value - whether to power on/off the baseband/usb/npe.
 * @param[in]   on_off - select power on or off, 0 - power off; other value - power on.
 * @return      none.
 */
void pm_set_suspend_power_cfg(pm_pd_module_e value, unsigned char on_off);

/**
 * @brief       This function serves to set the working mode of MCU based on 32k crystal,e.g. suspend mode, deep sleep mode, deep sleep with SRAM retention mode and shutdown mode.
 * @param[in]   sleep_mode          - sleep mode type select.
 * @param[in]   wakeup_src          - wake up source select.
 * @param[in]   wakeup_tick_type    - tick type select. Use 32K tick count for long-time sleep or 24M tick count for short-time sleep.
 * @param[in]   wakeup_tick         - The tick value at the time of wake-up.
                                      If the wakeup_tick_type is PM_TICK_STIMER, then wakeup_tick is converted to 24M. The range of tick that can be set is approximately:
                                      current tick value + (18352~0xe0000000), and the corresponding sleep time is approximately: 2ms~234.88s.It cannot go to sleep normally when it exceeds this range.
                                      If the wakeup_tick_type is PM_TICK_32K, then wakeup_tick is converted to 32K. The range of tick that can be set is approximately:
                                      64~0xffffffff, and the corresponding sleep time is approximately: 2ms~37hours.It cannot go to sleep normally when it exceeds this range.
 * @return      indicate whether the cpu is wake up successful.
 */
_attribute_text_sec_ int pm_sleep_wakeup(pm_sleep_mode_e sleep_mode,  pm_sleep_wakeup_src_e wakeup_src, pm_wakeup_tick_type_e wakeup_tick_type, unsigned int  wakeup_tick);

/**
 * @brief       This function serves to get reboot status.
 * @return      reboot enum element of pm_reboot_event_e.
 * @note        -# if return HW_TIMER_WATCHDOG_REBOOT, need call wd_clear_status() to avoid affecting the next detection of the mcu status;
 *              -# if return HW_32K_WATCHDOG_REBOOT,need call wd_32k_clear_status() to avoid affecting the next detection of the mcu status;
 *              -# if return HW_VBUS_DETECT_REBOOT,need to write 1 and 0 for 0x64(bit7) to avoid affecting the next detection of the mcu status;
 *              -# the interface sys_init() must be called before this interface can be invoked;
 */
pm_reboot_event_e pm_get_reboot_event(void);

/**
 * @brief       This function serves to switch digital module power.    
 * @param[in]   module - digital module.
 * @param[in]   power_sel - power up or power down.
 * @return      none.
 */
_attribute_ram_code_sec_noinline_ void pm_set_dig_module_power_switch(pm_pd_module_e module, pm_power_sel_e power_sel);

/**
 * @brief       This function serves to test different voltages from pd3.
 * @param[in]   mux_sel - select different voltages from pd3.
 * @return      none.
 */
void pm_set_probe_vol_to_pd3(pm_vol_mux_sel_e mux_sel);

