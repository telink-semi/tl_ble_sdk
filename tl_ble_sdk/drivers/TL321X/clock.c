/********************************************************************************************************
 * @file    clock.c
 *
 * @brief   This is the source file for TL321X
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
#include "lib/include/sys.h"
#include "clock.h"
#include "mspi.h"
#include "stimer.h"



/**********************************************************************************************************************
 *                                            local constants                                                       *
 *********************************************************************************************************************/


/**********************************************************************************************************************
 *                                              local macro                                                        *
 *********************************************************************************************************************/


/**********************************************************************************************************************
 *                                             local data type                                                     *
 *********************************************************************************************************************/


/**********************************************************************************************************************
 *                                              global variable                                                       *
 *********************************************************************************************************************/
sys_clk_t sys_clk = {
    .pll_clk = 192,
    .cclk = 24,
    .hclk = 24,
    .pclk = 24,
    .mspi_clk = 24,
};

sys_clk_config_t sys_clk_config = {
    .cclk_cfg = 0x01,
    .hclk_pclk_cfg = 0x00,
    .mspi_clk_cfg = 0x01,
};

_attribute_data_retention_sec_ unsigned char tl_24mrc_cal;
clk_32k_type_e g_clk_32k_src;
/**********************************************************************************************************************
 *                                              local variable                                                     *
 *********************************************************************************************************************/

/**********************************************************************************************************************
 *                                          local function prototype                                               *
 *********************************************************************************************************************/
static unsigned char clock_calculate_div_clk(sys_clock_src_e src, sys_clock_div_e div);

/**********************************************************************************************************************
 *                                         global function implementation                                             *
 *********************************************************************************************************************/


/**
 * @brief       This function serves to set 32k clock source.
 * @param[in]   src - variable of 32k type.
 * @return      none.
 */
void clock_32k_init(clk_32k_type_e src)
{
    unsigned char sel_32k   = analog_read_reg8(areg_aon_0x4e & (~FLD_CLK32K_SEL));
    unsigned char power_32k = analog_read_reg8(areg_aon_0x05) & (~(FLD_32K_RC_PD | FLD_32K_XTAL_PD));
    analog_write_reg8(areg_aon_0x4e, sel_32k | (src << 7));
    if(src)
    {
        analog_write_reg8(areg_aon_0x05, power_32k | FLD_32K_RC_PD);//32k xtal
    }
    else
    {
        analog_write_reg8(areg_aon_0x05, power_32k | FLD_32K_XTAL_PD);//32k rc
    }
    g_clk_32k_src = src;
}

/**
 * @brief       This function serves to kick 32k xtal.
 * @param[in]   xtal_times - kick times.
 * @return      1 success, 0 error.
 */
unsigned char clock_kick_32k_xtal(unsigned char xtal_times)
{
    /*
        TODO:The following kick process needs further validation. It takes about 1.5s for 32k xtal to stabilize, 
        so here temporarily use a delay method instead.
    */
#if 0
    int last_32k_tick;
    int curr_32k_tick;
    for(unsigned char i = 0; i< xtal_times; i++)
    {
        //set PC2 as pwm output
        unsigned char reg_c26 = read_reg8(0x140c26);
        write_reg8(0x140c26, reg_c26 & 0xfb);       //pc_io[2]=0
        unsigned char reg_c82 = read_reg8(0x140c82);
        write_reg8(0x140c82, PWM0);                 //pc[2]_fs=1 sel pwm0

        unsigned char reg_828 = read_reg8(0x140828);
        write_reg8(0x140828, 0x11);                 //cclk_sel=xtl24m, div=1

        unsigned short reg_414 = read_reg16(0x140414);
        write_reg16(0x140414, 375);                 //pwm0_cmp
        unsigned short reg_416 = read_reg16(0x140416);
        write_reg16(0x140416, 750);                 //pwm0_max
        unsigned char reg_401 = read_reg8(0x140401);
        write_reg8(0x140401, 0x01);                 //pwm0 en

        //wait for PWM wake up Xtal
        delay_ms(100);

        //Recover PC2 as Xtal pin
        write_reg8(0x140c26, reg_c26);
        write_reg8(0x140c82, reg_c82);

        write_reg8(0x140828, reg_828);

        write_reg16(0x140414, reg_414);
        write_reg16(0x140416, reg_416);

        write_reg8(0x140401, reg_401);

        last_32k_tick = clock_get_32k_tick();   //clock_get_32k_tick()
        delay_us(305);      //for 32k tick accumulator, tick period: 30.5us, dly 10 ticks
        curr_32k_tick = clock_get_32k_tick();
        if(last_32k_tick != curr_32k_tick)      //clock_get_32k_tick()
        {
            return 1;       //pwm kick 32k pad success
        }
    }
    return 0;
#else
    for(unsigned char i = 0; i< xtal_times; i++)
    {
        delay_ms(150);
    }

    return 1;
#endif
}

/**
 * @brief      This function serves to 24m rc calibration wait..
 * @return     1:busy  0:not busy
 */
static bool clock_24m_rc_cal_busy(void)
{
    return ((analog_read_reg8(areg_0xcf) & FLD_CAL_24M_DONE) == 0);
}

/**
 * @brief     This function performs to select 24M as the system clock source.
 *            24M RC is inaccurate, and it is greatly affected by temperature, if need use it so real-time calibration is required
 *            The 24M RC needs to be calibrated before the pm_sleep_wakeup function,
 *            because this clock will be used to kick 24m xtal start after wake up,
 *            The more accurate this time, the faster the crystal will start.Calibration cycle depends on usage
 * @return    none.
 */
void clock_cal_24m_rc(void)
{
    analog_write_reg8(areg_0xc8, 0x80);//wait 24m rc stable cycles

    analog_write_reg8(areg_aon_0x4f, analog_read_reg8(areg_aon_0x4f) | FLD_RC_24M_CAP_SEL);

    analog_write_reg8(areg_0xc7, FLD_CAL_24M_RC_DISABLE);
    analog_write_reg8(areg_0xc7, FLD_CAL_24M_RC_ENABLE);

    wait_condition_fails_or_timeout(clock_24m_rc_cal_busy,g_drv_api_error_timeout_us,drv_timeout_handler,(unsigned int)DRV_API_ERROR_TIMEOUT_RC_24M_CAL);

    analog_write_reg8(areg_aon_0x52, analog_read_reg8(areg_0xcb));//write 24m cap into manual register

    analog_write_reg8(areg_aon_0x4f, analog_read_reg8(areg_aon_0x4f) & (~FLD_RC_24M_CAP_SEL));

    analog_write_reg8(areg_0xc7, FLD_CAL_24M_RC_DISABLE);
    tl_24mrc_cal = analog_read_reg8(areg_aon_0x52);
}

/**
 * @brief      This function serves to 32k rc calibration wait.
 * @return     1:busy  0: not busy
 */
static bool clock_32k_rc_cal_busy(void)
{
    return ((analog_read_reg8(areg_0xcf) & FLD_CAL_32K_DONE) == 0x00);

}

/**
 * @brief     This function performs to select 32K as the system clock source.
 * @return    none.
 */
void clock_cal_32k_rc(void)
{
    analog_write_reg8(areg_aon_0x4f, analog_read_reg8(areg_aon_0x4f) | FLD_RC_32K_CAP_SEL);

    analog_write_reg8(areg_0xc6, FLD_CAL_32K_RC_DISABLE);
    analog_write_reg8(areg_0xc6, FLD_CAL_32K_RC_ENABLE);
    wait_condition_fails_or_timeout(clock_32k_rc_cal_busy,g_drv_api_error_timeout_us,drv_timeout_handler,(unsigned int)DRV_API_ERROR_TIMEOUT_RC_32K_CAL);

    analog_write_reg8(areg_aon_0x51, analog_read_reg8(areg_0xc9));//write 32k res[13:6] into manual register
    analog_write_reg8(areg_aon_0x4f, (analog_read_reg8(areg_aon_0x4f) & 0xc0) | analog_read_reg8(areg_0xca));//write 32k res[5:0] into manual register
    analog_write_reg8(areg_0xc6, FLD_CAL_32K_RC_DISABLE);
    analog_write_reg8(areg_aon_0x4f, analog_read_reg8(areg_aon_0x4f) & (~FLD_RC_32K_CAP_SEL));//manual on
}

/**
 * @brief  This function serves to set the 32k tick.
 * @param  tick - the value of to be set to 32k.
 * @return none.
 */
_attribute_ram_code_sec_noinline_ void clock_set_32k_tick(unsigned int tick)
{
    reg_system_ctrl |= FLD_SYSTEM_32K_WR_EN;    //r_32k_wr = 1;
    while(reg_system_st & FLD_SYSTEM_RD_BUSY);
    reg_system_timer_set_32k = tick;

    reg_system_st = FLD_SYSTEM_CMD_SYNC;    //cmd_sync = 1,trig write
    /**
     * The minimum waiting time here is 3*pclk cycles+3*24M xtal cycles, a total of 0.25us,
     * wait 0.25us before you can use wr_busy signal for judgment, jianzhi suggested that this time to 1us is enough.
     * add by jilong.liu, confirmed by jianzhi.chen 20240528
     */
    core_cclk_delay_tick(sys_clk.cclk);//1us

    while(reg_system_st & FLD_SYSTEM_CMD_SYNC); //wait wr_busy = 0

}

/**
 * @brief  This function serves to get the 32k tick.
 * @return 32k tick value.
 */
#if 0
/*
 * modify by yi.bao,confirmed by guangjun at 20210105
 * Use digital register way to get 32k tick may read error tick,cause the wakeup time is
 * incorrect with the setting time,the sleep time will very little or very big,will not wakeup on time.
 */
unsigned int clock_get_32k_tick(void)
{
    unsigned int timer_32k_tick;
    reg_system_st = FLD_SYSTEM_CLR_RD_DONE; //clr rd_done
    while((reg_system_st & FLD_SYSTEM_CLR_RD_DONE) != 0);   //wait rd_done = 0;
    reg_system_ctrl &= ~FLD_SYSTEM_32K_WR_EN;   //1:32k write mode; 0:32k read mode
    while((reg_system_st & FLD_SYSTEM_CLR_RD_DONE) == 0);   //wait rd_done = 1;
    timer_32k_tick = reg_system_timer_read_32k;
    reg_system_ctrl |= FLD_SYSTEM_32K_WR_EN;    //1:32k write mode; 0:32k read mode
    return timer_32k_tick;
}
#else
_attribute_ram_code_sec_noinline_ unsigned int clock_get_32k_tick(void)
{
    unsigned int t0 = 0;
    unsigned int t1 = 0;

    //In the system timer auto mode, when writing a tick value to the system tick, if the writing operation overlaps
    //with the 32k rising edge, the writing operation will be unsuccessful. When reading the 32k tick value,
    //first wait for the rising edge to pass to avoid overlap with the subsequent write tick value operation.
    //modify by weihua.zhang, confirmed by jianzhi at 20210126
    t0 = analog_read_reg32(0x60);
    while(1)
    {
        t1 = analog_read_reg32(0x60);
        if((t1-t0) == 1)
        {
            return t1;
        }
        else if(t1-t0)
        {
            t0 = t1;
        }
    }
}
#endif

/**
 * @brief       This function use to select the system clock source.
 * @param[in]   src          - cclk source.
 * @param[in]   cclk_div     - the cclk divide from src.
 * @param[in]   hclk_div     - the hclk divide from cclk.
 * @param[in]   pclk_div     - the pclk divide from hclk.
 * @param[in]   mspi_clk_div - mspi_clk can be divided from pll, rc and xtal. 
 *                             When selecting pll as the clock source, in order to not exceed the maximum frequency, the division coefficient can not less than 4.
 *                             If it is built-in flash, the maximum speed of mspi is 64M.
 *                             If it is an external flash, the maximum speed of mspi needs to be based on the board test.
 *                             Because the maximum speed is related to the wiring of the board, and is also affected by temperature and GPIO voltage,
 *                             the maximum speed needs to be tested at the highest and lowest voltage of the board,
 *                             and the high and low temperature long-term stability test speed is no problem.
 * @return      none
 * @note        Do not switch the clock during the DMA sending and receiving process;
 *              because during the clock switching process, the system clock will be
 *              suspended for a period of time, which may cause data loss
 */
_attribute_ram_code_sec_noinline_
void clock_init(sys_clock_src_e src,
                sys_clock_div_e cclk_div,
                sys_cclk_div_to_hclk_pclk_e hclk_pclk_div,
                sys_clock_div_e mspi_clk_div)
{
    clock_mspi_clk_config(src, mspi_clk_div);

    clock_cclk_hclk_pclk_config(src, cclk_div, hclk_pclk_div);
}

/**
 * @brief       This function use to configure the mspi clock source.
 * @param[in]   src - the mspi clk source
 * @param[in]   div - the mspi clk source divider
 * @return      none.
 */
_attribute_ram_code_sec_noinline_ void clock_mspi_clk_config(sys_clock_src_e src, sys_clock_div_e div)
{
    write_reg8(0x140800, (read_reg8(0x140800) & 0xc0) | src | div);//src:bit[5:4], div:bit[3:0]
    sys_clk.mspi_clk = clock_calculate_div_clk(src, div);
}

/**
 * @brief       This function used to configure the frequency of CCLK/HCLK/PCLK.
 *              You need to wait until all the peripherals that use these clocks are idle before you can switch frequencies.
 * @param[in]   src - clock source.
 * @param[in]   cclk_div - divider of CCLK.
 * @param[in]   hclk_div - divider of HCLK.
 * @param[in]   pclk_div - divider of PCLK.
 * @return      none
 */
_attribute_ram_code_sec_noinline_ void clock_cclk_hclk_pclk_config(sys_clock_src_e src, sys_clock_div_e cclk_div,
                                sys_cclk_div_to_hclk_pclk_e hclk_pclk_div)
{
    //change to 24M rc first.
    write_reg8(0x140828, (read_reg8(0x140828) & 0xc0) | RC_24M | CLK_DIV1);

    //HCLK and PCLK should be set ahead of CCLK, ensure the HCLK and PCLK not exceed the max CCLK(CCLK max 120M, HCLK max 60M, PCLK max 60M)
    write_reg8(0x140818, (read_reg8(0x140818) & 0xf8) | hclk_pclk_div);

    //Configure the CCLK clock frequency.
    write_reg8(0x140828, (read_reg8(0x140828) & 0xc0) | src | cclk_div);//clock source. 0:rc 24m, 1:xtl_24m, 2:pll

    sys_clk.cclk = clock_calculate_div_clk(src, cclk_div);
    sys_clk.hclk = sys_clk.cclk / (1 << (hclk_pclk_div>>2));
    sys_clk.pclk = sys_clk.hclk / (1 << (hclk_pclk_div&0x03));
}

/**
 * @brief       This function use to set all clock to default. 
 * @return      none.
 * @note        After call this, the following clock will set to default source and value:
 *              -----------------------------------------------------------------------
 *              clock source |          clock
 *              -----------------------------------------------------------------------
 *              RC 24M       | CCLK 24M, HCLK 24M, PCLK 24M, MSPI CLK 24M.
 *              -----------------------------------------------------------------------
 */
_attribute_ram_code_sec_noinline_ void clock_set_all_clock_to_default(void)
{
    clock_cclk_hclk_pclk_config(RC_24M, CLK_DIV1, CCLK_DIV1_TO_HCLK_DIV1_TO_PCLK);

    clock_mspi_clk_config(RC_24M, CLK_DIV1);//mspi clk to 24M rc clock, div 1, 24MHz
}

/**
 * @brief       This function use to save all clock configuration for the follow-up restore. 
 * @return      none.
 * @note        This function needs to be used in conjunction with clock_restore_clock_config().
 */
_attribute_ram_code_sec_noinline_ void clock_save_clock_config(void)
{
    sys_clk_config.cclk_cfg = read_reg8(0x140828);

    sys_clk_config.hclk_pclk_cfg = read_reg8(0x140818);

    sys_clk_config.mspi_clk_cfg = read_reg8(0x140800);
}

/**
 * @brief       This function use to restore all previously saved clock configurations.
 * @return      none.
 * @note        This function needs to be used in conjunction with clock_save_clock_config().
 */
_attribute_ram_code_sec_noinline_ void clock_restore_clock_config(void)
{
    clock_cclk_hclk_pclk_config(sys_clk_config.cclk_cfg & BIT_RNG(4, 5),
                                sys_clk_config.cclk_cfg & BIT_RNG(0, 3),
                                sys_clk_config.hclk_pclk_cfg);

    clock_mspi_clk_config(sys_clk_config.mspi_clk_cfg & BIT_RNG(4, 5),      //src
                        sys_clk_config.mspi_clk_cfg & BIT_RNG(0, 3));       //mspiclk_div
}
/**********************************************************************************************************************
 *                                          local function implementation                                             *
 *********************************************************************************************************************/
/**
 * @brief       This function use to configuer baseband pll clk. 
 * @param[in]   pll_clk - the pll clk
 * @return      none.
 */
void clock_bbpll_config(sys_pll_clk_e pll_clk)
{
    analog_write_reg8(areg_aon_0x06, analog_read_reg8(areg_aon_0x06) & (~FLD_PD_BBPLL_LDO));//power up pll

    /* power on bbpll supply switch and select pll clk out */
    analog_write_reg8(0x81, (analog_read_reg8(0x81) & (~FLD_POWER_ON_BBPLL_SUPPLY_SWITCH)) | (pll_clk & 0x40));
    
    sys_clk.pll_clk = (pll_clk >> 8);
}

/**
 * @brief       This function is used to calculate the clock after different clock sources, the unit is MHZ.
 * @param[in]   src - the clock source
 * @param[in]   div - the clock source divider
 * @return      clk.
 */
static unsigned char clock_calculate_div_clk(sys_clock_src_e src, sys_clock_div_e div)
{
    unsigned char clk = 0 ;
    if(BASEBAND_PLL == src ){
        clk = sys_clk.pll_clk / div;
    }else{
        clk = 24 / div;
    }
    return clk;
}
