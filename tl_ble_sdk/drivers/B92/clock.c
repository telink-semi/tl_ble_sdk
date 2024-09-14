/********************************************************************************************************
 * @file    clock.c
 *
 * @brief   This is the source file for B92
 *
 * @author  Driver Group
 * @date    2020
 *
 * @par     Copyright (c) 2020, Telink Semiconductor (Shanghai) Co., Ltd. ("TELINK")
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
#include "error_handler/error_handler.h"

unsigned char rc_24m_power;
unsigned char bbpll_power;

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
_attribute_data_retention_sec_ unsigned char tl_24mrc_cal;
clk_32k_type_e g_clk_32k_src;
/**********************************************************************************************************************
 *                                              local variable                                                     *
 *********************************************************************************************************************/

/**********************************************************************************************************************
 *                                          local function prototype                                               *
 *********************************************************************************************************************/

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
    unsigned char sel_32k   = analog_read_reg8(0x4e)&0x7f;
    unsigned char power_32k = analog_read_reg8(0x05)&0xfc;
    analog_write_reg8(0x4e, sel_32k|(src<<7));
    if(src)
    {
        analog_write_reg8(0x05, power_32k|0x1);//32k xtal
    }
    else
    {
        analog_write_reg8(0x05, power_32k|0x2);//32k rc
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
    int last_32k_tick;
    int curr_32k_tick;
    for(unsigned char i = 0; i< xtal_times; i++)
    {
        //**Note that the clock is 24M crystal oscillator. PCLK is 24MHZ
        //2.set PD0 as pwm output
        unsigned char reg_360 = read_reg8(0x140360);
        write_reg8(0x140360, PWM0);                 //PD0
        unsigned char reg_31e = read_reg8(0x14031e);
        write_reg8(0x14031e, reg_31e & 0xfe);       //PD0
        unsigned char reg_403 = read_reg8(0x140403);
        write_reg8(0x140403, 0x00);                 //pwm0 mode
        unsigned char reg_414 = read_reg8(0x140414);
        write_reg8(0x140414, 0x02);                 //pwm0 cmp
        unsigned char reg_416 = read_reg8(0x140416);
        write_reg8(0x140416, 0x04);                 //pwm0 max
        unsigned char reg_402 = read_reg8(0x140402);
        write_reg8(0x140402, 0xb6);                 //24M/(0xb6 + 1)/4 = 32786, ~=32768
        unsigned short reg_400 = read_reg16(0x140400);
        write_reg16(0x140400, reg_400|0x0100);      //pwm0 en

        //3.wait for PWM wake up Xtal
        delay_ms(100);

        //4.Xtal 32k output
        analog_write_reg8(0x03,0x4f); //<7:6>current select

        //5.Recover PD0 as Xtal pin
        write_reg8(0x140360,reg_360);
        write_reg8(0x14031e,reg_31e);
        write_reg8(0x140403,reg_403);
        write_reg8(0x140414,reg_414);
        write_reg8(0x140416,reg_416);
        write_reg8(0x140402,reg_402);
        write_reg16(0x140400,reg_400);

        last_32k_tick = clock_get_32k_tick();   //clock_get_32k_tick()
        delay_us(305);      //for 32k tick accumulator, tick period: 30.5us, dly 10 ticks
        curr_32k_tick = clock_get_32k_tick();
        if(last_32k_tick != curr_32k_tick)      //clock_get_32k_tick()
        {
            return 1;       //pwm kick 32k pad success
        }
    }
    return 0;
}

/**
 * @brief      This function serves to 24m rc calibration wait..
 * @return     1:busy  0:not busy
 */
static bool clock_24m_rc_cal_busy(void)
{
    return ((analog_read_reg8(0xcf) &BIT(7)) == 0x00);

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
    analog_write_reg8(0xc8, 0x80);


    analog_write_reg8(0x4f, analog_read_reg8(0x4f) | BIT(7) );

    analog_write_reg8(0xc7, 0x0e);
    analog_write_reg8(0xc7, 0x0f);
    wait_condition_fails_or_timeout(clock_24m_rc_cal_busy,g_drv_api_error_timeout_us,drv_timeout_handler,(unsigned int)DRV_API_ERROR_TIMEOUT_RC_24M_CAL);
    unsigned char cap = analog_read_reg8(0xcb);
    analog_write_reg8(0x52, cap);       //write 24m cap into manual register

    analog_write_reg8(0x4f, analog_read_reg8(0x4f) & (~BIT(7)) );

    analog_write_reg8(0xc7, 0x0e);
    tl_24mrc_cal = analog_read_reg8(0x52);
}


/**
 * @brief      This function serves to 32k rc calibration wait.
 * @return     1:busy  0: not busy
 */
static bool clock_32k_rc_cal_busy(void)
{
    return ((analog_read_reg8(0xcf) & BIT(6)) == 0x00);

}

/**
 * @brief     This function performs to select 32K as the system clock source.
 * @return    none.
 */
void clock_cal_32k_rc(void)
{
    analog_write_reg8(0x4f, ((analog_read_reg8(0x4f) & 0x3f) | 0x40));
    analog_write_reg8(0xc6, 0xf6);
    analog_write_reg8(0xc6, 0xf7);
    wait_condition_fails_or_timeout(clock_32k_rc_cal_busy,g_drv_api_error_timeout_us,drv_timeout_handler,(unsigned int)DRV_API_ERROR_TIMEOUT_RC_32K_CAL);
    unsigned char res1 = analog_read_reg8(0xc9);    //read 32k res[13:6]
    analog_write_reg8(0x51, res1);      //write 32k res[13:6] into manual register
    unsigned char res2 = analog_read_reg8(0xca);    //read 32k res[5:0]
    analog_write_reg8(0x4f, (res2 | (analog_read_reg8(0x4f) & 0xc0)));      //write 32k res[5:0] into manual register
    analog_write_reg8(0xc6, 0xf6);
    analog_write_reg8(0x4f, ((analog_read_reg8(0x4f) & 0x3f) | 0x00));//manual on
    rc_24m_power = analog_read_reg8(0x05) & 0x04;
    bbpll_power  = analog_read_reg8(0x06) & 0x01;
}

/**
 * @brief  This function serves to set the 32k tick.
 * @param  tick - the value of to be set to 32k.
 * @return none.
 */
_attribute_ram_code_sec_optimize_o2_ void clock_set_32k_tick(unsigned int tick)
{
    reg_system_ctrl |= FLD_SYSTEM_32K_WR_EN;    //r_32k_wr = 1;
    while(reg_system_st & FLD_SYSTEM_RD_BUSY);
    reg_system_timer_set_32k = tick;

    reg_system_st = FLD_SYSTEM_CMD_SYNC;    //cmd_sync = 1,trig write
    /**
     * This delay time is about 1.38us under the calibrated 24M RC clock.
     * The minimum waiting time here is 3*pclk cycles+3*24M xtal cycles, a total of 0.25us,
     * wait 0.25us before you can use wr_busy signal for judgment, jianzhi suggested that this time to 1us is enough.
     * add by bingyu.li, confirmed by jianzhi.chen 20231115
     */
    core_cclk_delay_tick(sys_clk.cclk);
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
_attribute_ram_code_sec_optimize_o2_ unsigned int clock_get_32k_tick(void)
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
 * @param[in]   pll - pll clock.
 * @param[in]   src - cclk source.
 * @param[in]   cclk_div - the cclk divide from pll.it is useless if src is not PAD_PLL_DIV. cclk max is 96M
 * @param[in]   hclk_div - the hclk divide from cclk.hclk max is 48M.
 * @param[in]   pclk_div - the pclk divide from hclk.pclk max is 48M.if hclk = 1/2 * cclk, the pclk can not be 1/4 of hclk.
 * @param[in]   mspi_clk_div - mspi_clk has two source - pll div and 24M rc. If it is built-in flash, the maximum speed of mspi is 64M.
                               If it is an external flash, the maximum speed of mspi needs to be based on the board test.
                               Because the maximum speed is related to the wiring of the board, and is also affected by temperature and GPIO voltage,
                               the maximum speed needs to be tested at the highest and lowest voltage of the board,
                               and the high and low temperature long-term stability test speed is no problem.
 * @return      none
 * @note        Do not switch the clock during the DMA sending and receiving process;
 *              because during the clock switching process, the system clock will be
 *              suspended for a period of time, which may cause data loss
 */
_attribute_ram_code_sec_noinline_
void clock_init_ram(sys_pll_clk_e pll,
                    sys_clock_src_e src,
                    sys_pll_div_to_cclk_e cclk_div,
                    sys_cclk_div_to_hclk_e hclk_div,
                    sys_hclk_div_to_pclk_e pclk_div,
                    sys_pll_div_to_mspi_clk_e mspi_clk_div)
{
    //ensure mspi is not in busy status before change mspi clock
    mspi_stop_xip();

    //first cclk/mspi_clk switch to 24rc to avoid the risk of hclk/pclk/mspi_clk exceeding its maximum configurable frequency for a short period of time
    //when switching different clock frequencies using this interface.
    write_reg8(0x1401e8, read_reg8(0x1401e8) & 0x8f);               //cclk to 24M rc clock
    write_reg8(0x1401c0, read_reg8(0x1401c0) &(~BIT(6)));           //mspi_clk to 24M rc clock

    //pll clk
    analog_write_reg8(0x80, (analog_read_reg8(0x80) & 0xe0) | ((pll >> 2) & 0x1f));
    analog_write_reg8(0x09, (analog_read_reg8(0x09) & 0xf3) | ((pll&0x03) << 2));
    sys_clk.pll_clk = (pll >> 8);

    //usb clock (192M/4 =48M) pll clock should be the multiple of 48, because USB clock is 48M.
    write_reg8(0x1401fb, sys_clk.pll_clk/48);

    //wait for PLL stable
    analog_write_reg8(0x81, (analog_read_reg8(0x81) | BIT(6)));
    while(BIT(5) != (analog_read_reg8(0x88) & BIT(5)));
    analog_write_reg8(0x81, (analog_read_reg8(0x81) & ~BIT(6)));

    //change mspi clock should be ram code.
    if(RC_24M_TO_MSPI_CLK == mspi_clk_div)
    {
        write_reg8(0x1401c0, read_reg8(0x1401c0) & 0xbf);  //bit6 0
        sys_clk.mspi_clk = 24;
    }
    else
    {
        write_reg8(0x1401c0, (read_reg8(0x1401c0) & 0xc0) | mspi_clk_div );
        write_reg8(0x1401c0, read_reg8(0x1401c0) | BIT(6));  //if the div is odd, should set two times to ensure the correct sequence.
        write_reg8(0x1401c0, read_reg8(0x1401c0) | BIT(6));
        sys_clk.mspi_clk = sys_clk.pll_clk / mspi_clk_div;
    }
    mspi_set_xip_en();

    //hclk and pclk should be set ahead of cclk, ensure the hclk and pclk not exceed the max clk(cclk max 96M, hclk max 48M, pclk max 48M)
    if(CCLK_DIV1_TO_HCLK == hclk_div)
    {
        write_reg8(0x1401d8, read_reg8(0x1401d8) & ~BIT(2));
    }
    else
    {
        write_reg8(0x1401d8, read_reg8(0x1401d8) | BIT(2));
    }

    //pclk can div1/div2/div4 from hclk.
    if(HCLK_DIV1_TO_PCLK == pclk_div)
    {
        write_reg8(0x1401d8, read_reg8(0x1401d8) & 0xfc);
    }
    else
    {
        write_reg8(0x1401d8, (read_reg8(0x1401d8) & 0xfc) | (pclk_div/2));
    }

    //select cclk source(RC24M/PAD24M/PAD_PLL_DIV/PAD_PLL)
    if(PAD_PLL_DIV == src)
    {
        write_reg8(0x1401e8, (read_reg8(0x1401e8) & 0xf0) | cclk_div);
        sys_clk.cclk = sys_clk.pll_clk / cclk_div;
    }
    else if(PAD_PLL == src)
    {
        sys_clk.cclk = sys_clk.pll_clk;
    }
    else
    {
        sys_clk.cclk = 24;
    }
    write_reg8(0x1401e8, (read_reg8(0x1401e8) & 0x8f) | (src << 4));

    //clk record.
    sys_clk.hclk = sys_clk.cclk / hclk_div;
    sys_clk.pclk = sys_clk.hclk / pclk_div;
}

_attribute_text_sec_
void clock_init(sys_pll_clk_e pll,
                sys_clock_src_e src,
                sys_pll_div_to_cclk_e cclk_div,
                sys_cclk_div_to_hclk_e hclk_div,
                sys_hclk_div_to_pclk_e pclk_div,
                sys_pll_div_to_mspi_clk_e mspi_clk_div)
{
    DISABLE_BTB;
    clock_init_ram(pll, src, cclk_div, hclk_div, pclk_div, mspi_clk_div);
    ENABLE_BTB;
}

/**
 * @brief       This function used to configure the frequency of CCLK/HCLK/PCLK when the PLL is 192M.
 *              You need to wait until all the peripherals that use these clocks are idle before you can switch frequencies.
 * @param[in]   cclk_hclk_pclk - frequency of CCLK/HCLK/PCLK.
 * @return      none
 */
void cclk_hclk_pclk_config(pll_div_cclk_hclk_pclk_e div)
{
    unsigned char cclk_div = div>>8;
    unsigned char hclk_div = (div&0x00f0)>>4;
    unsigned char pclk_div = div&0x000f;

    //CCLK max 96M, HCLK max 48M, PCLK max 48M, because the status of cclk is unknown when the interface is invoked,
    //the configuration sequence is as follows: hclk(div2)/pclk(div1) -> Configure cclk -> Configure hclk/pclk.

    //hclk(div2)/pclk(div1)
    write_reg8(0x1401d8, read_reg8(0x1401d8) | BIT(2));
    write_reg8(0x1401d8, read_reg8(0x1401d8) & 0xfc);

    //Configure the CCLK clock frequency.
    write_reg8(0x1401e8, (read_reg8(0x1401e8) & 0xf0) | cclk_div);

    //Configure hclk/pclk
    if(CCLK_DIV1_TO_HCLK == hclk_div)
    {
        write_reg8(0x1401d8, read_reg8(0x1401d8) & ~BIT(2));
    }
    else
    {
        write_reg8(0x1401d8, read_reg8(0x1401d8) | BIT(2));
    }

    //PCLK can div1/div2/div4 from HCLK.
    if(HCLK_DIV1_TO_PCLK == pclk_div)
    {
        write_reg8(0x1401d8, read_reg8(0x1401d8) & 0xfc);
    }
    else
    {
        write_reg8(0x1401d8, (read_reg8(0x1401d8) & 0xfc) | (pclk_div/2));
    }

    sys_clk.cclk = sys_clk.pll_clk / cclk_div;
    sys_clk.hclk = sys_clk.cclk / hclk_div;
    sys_clk.pclk = sys_clk.hclk / pclk_div;
}

#if RC_4M_FUNCTION
/**
 * @brief       This function performs to select 4M or 24M as the system clock .
 * @param[in]   sys_clock_src_sel   -sel 4MHz/24MHz RC mode.
 * @return      none
 */
void sys_clock_src_sel(sys_clock_src_sel_e sys_clock_src_sel)
{
    if(RC_4M == sys_clock_src_sel)
    {
        analog_write_reg8(0x07, analog_read_reg8(0x07) & (~BIT(1)));
                                //<1>:pm_pd_4M_rcosc_1p0v,      default:1,->0: Power up 4MHz RC oscillator.
        analog_write_reg8(0x22, analog_read_reg8(0x22) & (~BIT(4)));
                                //<4>:pm_rc_24M_4M_1p0v,        default:1,->0: 4M mux.
        analog_write_reg8(0x05, analog_read_reg8(0x05) | (BIT(2)));
                                //<2>:24M_rc_pd,        default:0,->1: Power down 24MHz RC oscillator.
        //analog_write_reg8(0x1f    , (analog_read_reg8(0x1f)   &   0x0f )  |   0x00 << 4); //trim cap of 4M rc, default : 1000
            //afe1v_aon_reg31<7:4>=0xff:4MRC output 1.9M   afe1v_aon_reg31<7:4>=0x00:4MRC output 4M
    }
    else
    {
        analog_write_reg8(0x07, analog_read_reg8(0x07) | (BIT(1)));
                                //<1>:pm_pd_4M_rcosc_1p0v,      default:1,->1: Power down 4MHz RC oscillator.
        analog_write_reg8(0x22, analog_read_reg8(0x22) | (BIT(4)));
                                //<4>:pm_rc_24M_4M_1p0v,        default:1,->1: 24M mux.
        analog_write_reg8(0x05, analog_read_reg8(0x05) & (~BIT(2)));
                                //<2>:24M_rc_pd,        default:0,->0: Power up 24MHz RC oscillator.
    }
}
#endif

/**********************************************************************************************************************
 *                                          local function implementation                                             *
 *********************************************************************************************************************/
