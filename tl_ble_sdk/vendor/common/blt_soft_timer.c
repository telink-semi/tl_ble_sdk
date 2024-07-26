/********************************************************************************************************
 * @file    blt_soft_timer.c
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
#include "stack/ble/ble.h"
#include "tl_common.h"
#include "blt_soft_timer.h"




#if (BLT_SOFTWARE_TIMER_ENABLE)




_attribute_ble_data_retention_  blt_soft_timer_t    blt_timer;


/**
 * @brief       This function is used to Sort the timers according
 *              to the time of the timed task, so as to trigger the
 *              timers in turn
 * @param[in]   none
 * @return      none
 */
int  blt_soft_timer_sort(void)
{
    if(blt_timer.currentNum < 1 || blt_timer.currentNum > MAX_TIMER_NUM){
        return 0;
    }
    else{
        //BubbleSort
        int n = blt_timer.currentNum;
        u8 temp[sizeof(blt_time_event_t)];

        for(int i=0;i<n-1;i++)
        {
            for(int j=0;j<n-i-1;j++)
            {
                if(TIME_COMPARE_BIG(blt_timer.timer[j].t, blt_timer.timer[j+1].t))
                {
                    //swap
                    memcpy(temp, &blt_timer.timer[j], sizeof(blt_time_event_t));
                    memcpy(&blt_timer.timer[j], &blt_timer.timer[j+1], sizeof(blt_time_event_t));
                    memcpy(&blt_timer.timer[j+1], temp, sizeof(blt_time_event_t));
                }
            }
        }
    }

    return 1;
}



/**
 * @brief       This function is used to add new software timer task
 * @param[in]   func - callback function for software timer task
 * @param[in]   interval_us - the interval for software timer task
 * @return      0 - timer task is full, add fail
 *              1 - create successfully
 */
int blt_soft_timer_add(blt_timer_callback_t func, u32 interval_us)
{
    //int i;
    u32 now = clock_time();

    if(blt_timer.currentNum >= MAX_TIMER_NUM){  //timer full
        return  0;
    }
    else{
        blt_timer.timer[blt_timer.currentNum].cb = func;
        blt_timer.timer[blt_timer.currentNum].interval = interval_us * SYSTEM_TIMER_TICK_1US;
        blt_timer.timer[blt_timer.currentNum].t = now + blt_timer.timer[blt_timer.currentNum].interval;
        blt_timer.currentNum ++;

        blt_soft_timer_sort();

        blc_pm_setAppWakeupLowPower(blt_timer.timer[0].t,  1);

        return  1;
    }
}



/**
 * @brief       Timer tasks are originally ordered. When deleting, it will
 *              be overwritten forward, so the order will not be destroyed
 *              and there is no need to reorder
 * @param[in]   index - the index for some software timer task
 * @return      0 - delete fail
 *              other - delete successfully
 */
int  blt_soft_timer_delete_by_index(u8 index)
{
    if(index >= blt_timer.currentNum){
        return 0;
    }


    for(int i=index; i<blt_timer.currentNum - 1; i++){
        memcpy(&blt_timer.timer[i], &blt_timer.timer[i+1], sizeof(blt_time_event_t));
    }

    blt_timer.currentNum --;

    return 0;
}

/**
 * @brief       This function is used to delete timer tasks
 * @param[in]   func - callback function for software timer task
 * @return      0 - delete fail
 *              1 - delete successfully
 */
int     blt_soft_timer_delete(blt_timer_callback_t func)
{


    for(int i=0; i<blt_timer.currentNum; i++){
        if(blt_timer.timer[i].cb == func){

            blt_soft_timer_delete_by_index(i);

            if(blt_timer.currentNum){ //at least one timer exist
                if(i == 0){  //The most recent timer is deleted, and the time needs to be updated
                    if( (u32)(blt_timer.timer[0].t - clock_time()) < 5000 *  SYSTEM_TIMER_TICK_1MS){
                        blc_pm_setAppWakeupLowPower(blt_timer.timer[0].t,  1);
                    }
                    else{
                        blc_pm_setAppWakeupLowPower(0, 0);  //disable
                    }
                }
            }
            else{
                blc_pm_setAppWakeupLowPower(0, 0);  //disable
            }

            return 1;
        }
    }

    return 0;
}


/**
 * @brief       This function is used to manage software timer tasks
 * @param[in]   type - the type for trigger
 * @return      none
 */
void    blt_soft_timer_process(int type)
{
    if(type == CALLBACK_ENTRY){ //callback trigger

    }

    u32 now = clock_time();
    if(!blt_timer.currentNum){
        blc_pm_setAppWakeupLowPower(0, 0);  //disable
        return;
    }

    if( !blt_is_timer_expired(blt_timer.timer[0].t, now) ){
        return;
    }

    int change_flg = 0;
    int result;
    for(int i=0; i<blt_timer.currentNum; i++){
        if(blt_is_timer_expired(blt_timer.timer[i].t ,now) ){ //timer trigger

            if(blt_timer.timer[i].cb == NULL){

            }
            else{
                result = blt_timer.timer[i].cb();

                if(result < 0){
                    blt_soft_timer_delete_by_index(i);
                }
                else if(result == 0){
                    change_flg = 1;
                    blt_timer.timer[i].t = now + blt_timer.timer[i].interval;
                }
                else{  //set new timer interval
                    change_flg = 1;
                    blt_timer.timer[i].interval = result * SYSTEM_TIMER_TICK_1US;
                    blt_timer.timer[i].t = now + blt_timer.timer[i].interval;
                }
            }
        }
    }


    if(blt_timer.currentNum ){ //timer table not empty
        if(change_flg){
            blt_soft_timer_sort();
        }

        if( (u32)(blt_timer.timer[0].t - now) < 5000 *  SYSTEM_TIMER_TICK_1MS){
            blc_pm_setAppWakeupLowPower(blt_timer.timer[0].t,  1);
        }
        else{
            blc_pm_setAppWakeupLowPower(0, 0);  //disable
        }

    }
    else{
        blc_pm_setAppWakeupLowPower(0, 0);  //disable
    }

}

/**
 * @brief       This function is used to register the call back for pm_appWakeupLowPowerCb
 * @param[in]   none
 * @return      none
 */
void    blt_soft_timer_init(void)
{
    blc_pm_registerAppWakeupLowPowerCb(blt_soft_timer_process);
}


#endif  //end of  BLT_SOFTWARE_TIMER_ENABLE

