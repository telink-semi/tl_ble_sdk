/********************************************************************************************************
 * @file    core.c
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
#include "core.h"
/**********************************************************************************************************************
 *                                         global function implementation                                             *
 *********************************************************************************************************************/
/**
 * @brief       This function performs to set delay time by cclk tick.
 * @param[in]   core_cclk_tick - Number of ticks in cclk
 * @return      none
 */
_attribute_ram_code_sec_noinline_ void core_cclk_delay_tick(unsigned long long core_cclk_tick)
{
    unsigned long long start = rdmcycle();
    while (rdmcycle() - start < core_cclk_tick) {}
}



/**
 * @brief       provides a unified timeout interface(for internal calls only).
 * @param[in]   condition - function pointer, timeout judgment condition, must be bool xxx(void).
 * @param[in]   timeout_us - timeout(us).
 * @param[in]   func -  function pointer, timeout exit handle,must be void xxx(unsigned int).
 * @param[in]   err_code - xxx_api_error_code_e/drv_api_error_code_e.
 * @return      1: timeout(the upper layer interface uses drv_api_status_e(DRV_API_TIMEOUT)) 0: pass
 * @note        the rules for parameter are as follows (for internal purposes only):
 *              wait_condition_fails_or_timeout_with_param:
 *              1.If the peripheral module has only one timeout judgment, can call this interface directly;
 *              2.If the peripheral module has multiple calls, for the code simplicity, can be redefined by macros (refer to I2C_WAIT);
 *              3.If the function is called by another .c file or by itself .h,
 *                define it in the.h file and state that it is for internal use (refer to analog.h),otherwise put it in.c;
 *              condition:
 *              1.What is the type of the function according to the type of the upper layer interface;
 *              2.For the new conditional interface,if the function is called by another .c file or by itself .h,
 *                define it in the.c file,declare in.h and state that it is for internal use (refer to analog.h),otherwise put it in.c;
 *              3.For the original interface in.h,if the application is likely to call, determine whether to add timeout precautions;
 *              timeout_us:
 *              1.If determine that the timeout is caused by own interface(1.must reboot 2.reset is solved,
 *                but it is called in many places, and function calls are too nested, and most functions have no return type),
 *                use g_drv_api_error_timeout_us,and the corresponding set interface drv_set_error_timeout;
 *              2.If the timeout may be caused by an external source or own unexpected exceptions need to be reset,
 *                define the timeout global variable in the corresponding module(g_xxx_error_timeout_us),
 *                and the corresponding interface xxx_set_error_timeout:
 *                - If timeout may be related to the other side of the communication,
 *                  add comment minimum time limit and solution(refer to i2c);
 *                - If the timeout may be caused own unexpected exceptions need to be reset,
 *                  the timeout solution has been added to the parameter func,just note the minimum time limit(refer to aes);
 *              func:
 *              1.If determine that the timeout is caused by own interface(1.must reboot 2.reset is solved, but it is called
 *                in many places, and function calls are too nested, and most functions have no return type), use timeout_handler;
 *              2.If the timeout may be caused by an external source or own unexpected exceptions need to be reset,
 *                define the timeout_handler interface in the corresponding module (xxx_timeout_handler):
 *                - If timeout may be related to the other side of the communication,
 *                  the driver only needs to record err_code using global variable g_xxx_error_timeout_code(refer to i2c);
 *                - If the timeout may be caused own unexpected exceptions need to be reset(one wrong reasons),
 *                  add the timeout solution to the interface,and (void)err_code(refer to trng);
 *                - If the timeout may be caused own unexpected exceptions need to be reset(many wrong reasons),
 *                  add the timeout solution to the interface,and (void)err_code(refer to aes);
 *              err_code:
 *              1.If determine that the timeout is caused by own interface(1.must reboot 2.reset is solved,
 *                but it is called in many places, and function calls are too nested, and most functions have no return type),
 *                use drv_api_error_code_e(self-add);
 *              2.If the timeout may be caused by an external source or own unexpected exceptions need to be reset:
 *                 - If timeout may be related to the other side of the communication,
 *                   define the enumeration xxx_api_error_timeout_code_e, the global variable g_xxx_error_timeout_code,
 *                   and the interface xxx_get_error_timeout_code(refer to i2c);
 *                 - If the timeout may be caused own unexpected exceptions need to be reset(one wrong reasons),
 *                   err_code parameter transfer is 0,no need to define the enumeration xxx_api_error_timeout_code_e,
 *                   the global variable g_xxx_error_timeout_code, and the interface xxx_get_error_timeout_code(refer to trng);
 *                 - If the timeout may be caused own unexpected exceptions need to be reset(many wrong reasons),
 *                   only define the enumeration xxx_api_error_timeout_code_e, no need to define the global variable
 *                   g_xxx_error_timeout_code, and the interface xxx_get_error_timeout_code(refer to aes);
 */
_attribute_ram_code_sec_noinline_  unsigned int wait_condition_fails_or_timeout(condition_fp condition,unsigned int timeout_us,timeout_handler_fp func,unsigned int err_code)
{
     unsigned long long start = rdmcycle();
//Refer to the i2c_master_write/i2c_master_read judgment logic when the peripheral sends or receives data and the judgment condition needs to be compared with fifo cnt.
     while(condition()){
        if(core_cclk_time_exceed(start,timeout_us)){
            func(err_code);
            return  1;
        }
     }
     return  0;
}

/**
 * @brief       provides a unified timeout interface(condition with parameter)(for internal calls only).
 * @param[in]   condition - function pointer, timeout judgment condition, must be bool xxx(unsigned int).
 * @param[in]   cdt_param  - condition parameter.
 * @param[in]   timeout_us - timeout(us).
 * @param[in]   func -  function pointer, timeout exit handle,must be void xxx(unsigned int).
 * @param[in]   err_code - xxx_api_error_code_e/drv_api_error_code_e.
 * @return      1: timeout(the upper layer interface uses drv_api_status_e(DRV_API_TIMEOUT)) 0: pass
 * @note        the rules for parameter are as follows (for internal purposes only):
 *              wait_condition_fails_or_timeout_with_param:
 *              1.If the peripheral module has only one timeout judgment, can call this interface directly;
 *              2.If the peripheral module has multiple calls, for the code simplicity, can be redefined by macros (refer to I2C_WAIT);
 *              3.If the function is called by another .c file or by itself .h,
 *                define it in the.h file and state that it is for internal use (refer to analog.h),otherwise put it in.c;
 *              condition:
 *              1.What is the type of the function according to the type of the upper layer interface;
 *              2.For the new conditional interface,if the function is called by another .c file or by itself .h,
 *                define it in the.c file,declare in.h and state that it is for internal use (refer to analog.h),otherwise put it in.c;
 *              3.For the original interface in.h,if the application is likely to call, determine whether to add timeout precautions;
 *              timeout_us:
 *              1.If determine that the timeout is caused by own interface(1.must reboot 2.reset is solved,
 *                but it is called in many places, and function calls are too nested, and most functions have no return type),
 *                use g_drv_api_error_timeout_us,and the corresponding set interface drv_set_error_timeout;
 *              2.If the timeout may be caused by an external source or own unexpected exceptions need to be reset,
 *                define the timeout global variable in the corresponding module(g_xxx_error_timeout_us),
 *                and the corresponding interface xxx_set_error_timeout:
 *                - If timeout may be related to the other side of the communication,
 *                  add comment minimum time limit and solution(refer to i2c);
 *                - If the timeout may be caused own unexpected exceptions need to be reset,
 *                  the timeout solution has been added to the parameter func,just note the minimum time limit(refer to aes);
 *              func:
 *              1.If determine that the timeout is caused by own interface(1.must reboot 2.reset is solved, but it is called
 *                in many places, and function calls are too nested, and most functions have no return type), use timeout_handler;
 *              2.If the timeout may be caused by an external source or own unexpected exceptions need to be reset,
 *                define the timeout_handler interface in the corresponding module (xxx_timeout_handler):
 *                - If timeout may be related to the other side of the communication,
 *                  the driver only needs to record err_code using global variable g_xxx_error_timeout_code(refer to i2c);
 *                - If the timeout may be caused own unexpected exceptions need to be reset(one wrong reasons),
 *                  add the timeout solution to the interface,and (void)err_code(refer to trng);
 *                - If the timeout may be caused own unexpected exceptions need to be reset(many wrong reasons),
 *                  add the timeout solution to the interface,and (void)err_code(refer to aes);
 *              err_code:
 *              1.If determine that the timeout is caused by own interface(1.must reboot 2.reset is solved,
 *                but it is called in many places, and function calls are too nested, and most functions have no return type),
 *                use drv_api_error_code_e(self-add);
 *              2.If the timeout may be caused by an external source or own unexpected exceptions need to be reset:
 *                 - If timeout may be related to the other side of the communication,
 *                   define the enumeration xxx_api_error_timeout_code_e, the global variable g_xxx_error_timeout_code,
 *                   and the interface xxx_get_error_timeout_code(refer to i2c);
 *                 - If the timeout may be caused own unexpected exceptions need to be reset(one wrong reasons),
 *                   err_code parameter transfer is 0,no need to define the enumeration xxx_api_error_timeout_code_e,
 *                   the global variable g_xxx_error_timeout_code, and the interface xxx_get_error_timeout_code(refer to trng);
 *                 - If the timeout may be caused own unexpected exceptions need to be reset(many wrong reasons),
 *                   only define the enumeration xxx_api_error_timeout_code_e, no need to define the global variable
 *                   g_xxx_error_timeout_code, and the interface xxx_get_error_timeout_code(refer to aes);
 */
_attribute_ram_code_sec_noinline_ unsigned int wait_condition_fails_or_timeout_with_param(condition_fp_with_param condition,unsigned int cdt_param,unsigned int timeout_us,timeout_handler_fp func,unsigned int  err_code){
         unsigned long long start = rdmcycle();
         while(condition(cdt_param)){
            if(core_cclk_time_exceed(start,timeout_us)){
                func(err_code);
                return  1;
            }
         }
         return  0;
}
