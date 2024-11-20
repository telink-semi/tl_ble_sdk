/********************************************************************************************************
 * @file    app_freertos.c
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
#include "app_freertos.h"
#include "drivers.h"
#include "stack/ble/ble.h"
#include "app.h"
#include "app_config.h"

#if(FREERTOS_ENABLE)
#include <FreeRTOS.h>

#include <timers.h>
#include "semphr.h"
#include "stack/ble/os_sup/os_sup.h"


_attribute_ble_data_retention_ static TaskHandle_t hBleTask = NULL;  //Handle for the BLE task

_attribute_ble_data_retention_ static SemaphoreHandle_t xBleSendDataMutex = NULL;  //xBleSendDataMutex (lock) to ensure thread-safe access to BLE data sending operations


/**
 * @brief        vPreSleepProcessing
 * @param[in]    uxExpectedIdleTime
 * @return      none
 */
void vPreSleepProcessing( unsigned long uxExpectedIdleTime )
{
    /* Avoid compiler warnings about the unused parameter. */
    (void)uxExpectedIdleTime;
#if (BLE_APP_PM_ENABLE)
    void app_process_power_management(void);
    app_process_power_management();
#endif

}


/**
 * @brief        vPostSleepProcessing
 * @param[in]    uxExpectedIdleTime
 * @return      none
 */
void vPostSleepProcessing( unsigned long uxExpectedIdleTime )
{
    //reserved
}
/**
 * @brief        vApplicationIdleHook
 * @param[in]    none
 * @return      none
 */
void vApplicationIdleHook( void )
{
    /* Doesn't do anything yet. */

    ////////////////////////////////////// UI entry /////////////////////////////////
    #if (BATT_CHECK_ENABLE)
    traceAPP_BAT_Task_BEGIN();
    /*The frequency of low battery detect is controlled by the variable lowBattDet_tick, which is executed every
     500ms in the demo. Users can modify this time according to their needs.*/
    extern u32    lowBattDet_tick;
    if(battery_get_detect_enable() && clock_time_exceed(lowBattDet_tick, 500000) ){
        lowBattDet_tick = clock_time();
        user_battery_power_check(BAT_DEEP_THRESHOLD_MV);
    }
    traceAPP_BAT_Task_END();
    #endif

    #if (TLKAPI_DEBUG_ENABLE)
        tlkapi_debug_handler();
    #endif
}


#if UI_LED_ENABLE
/**
 * @brief        This function is the LED task
 * @param[in]    none
 * @return      none
 */
_attribute_ble_data_retention_ static TaskHandle_t hLedTask = NULL;
static void led_task(void *pvParameters)
{
    (void)pvParameters;
    while(1)
    {
        gpio_toggle(GPIO_LED_GREEN);
        traceAPP_LED_Task_Toggle();
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}


/**
 * @brief        This function is to create LED task
 * @param[in]    none
 * @return      none
 */
void app_ledTaskCreate(void)
{
     BaseType_t ret;
     ret =  xTaskCreate( led_task, "tLed", 512,   (void*)0, (tskIDLE_PRIORITY+1), &hLedTask );
     configASSERT( ret == pdPASS );
}
#endif  //#if UI_LED_ENABLE


/**
 * @brief        This function is used to send a semaphore and can be called inside an interrupt.
 * @param[in]    none.
 * @return        none.
 */
_attribute_ram_code_
void os_give_sem_from_isr(void)
{
    if(hBleTask == NULL)
        return;
    BaseType_t pxHigherPriorityTaskWoken = pdFALSE;
    vTaskNotifyGiveFromISR(hBleTask, &pxHigherPriorityTaskWoken);
}


/**
 * @brief        This function is to send a semaphore
 * @param[in]    none.
 * @return        none.
 */
_attribute_ram_code_
void os_give_sem(void)
{
    if(hBleTask == NULL)
        return;
    xTaskNotifyGive(hBleTask);

}


/**
 * @brief        Acquires a mutex semaphore.
 * @param[in]    none
 * @return      none
 */
_attribute_ram_code_
void os_take_mutex_sem(void)
{
    traceAPP_MUTEX_Task_BEGIN();
    if( xSemaphoreTake( xBleSendDataMutex, portMAX_DELAY ) != pdPASS )
    {
         printf("xSemaphoreTake pdFALSE\r\n");
    }
}


/**
 * @brief        Releases a mutex semaphore.
 * @param[in]    none
 * @return      none
 */
_attribute_ram_code_
void os_give_mutex_sem(void)
{
    traceAPP_MUTEX_Task_END();
    if( xSemaphoreGive( xBleSendDataMutex ) != pdPASS )
    {
        printf("xSemaphoreGive pdFALSE\r\n");
    }
}

/**
 * @brief        This function is the BLE task
 * @param[in]    none
 * @return      none
 */
static void ble_task( void *pvParameters )
{
    (void)pvParameters;

    while(1)
    {
        ulTaskNotifyTake(pdTRUE,  portMAX_DELAY);

        traceAPP_BLE_Task_BEGIN();

        blc_sdk_main_loop();

        traceAPP_BLE_Task_END();
        //debug
        //uxTaskGetStackHighWaterMark(NULL);
    }
}

/**
 * @brief        This function is to create BLE task
 * @param[in]    none
 * @return      none
 */
void app_BleTaskCreate(void)
{
     BaseType_t ret;
     blc_ll_registerGiveSemCb(os_give_sem_from_isr, os_give_sem); /* Register semaphore to ble module */

     blc_ll_registerMutexSemCb(os_take_mutex_sem, os_give_mutex_sem);

     xBleSendDataMutex = xSemaphoreCreateMutex();

     configASSERT( xBleSendDataMutex );

     ret =  xTaskCreate( ble_task, "tble", 1024, (void*)0, (tskIDLE_PRIORITY+2), &hBleTask );

     configASSERT( ret == pdPASS );
}


#if ( ( configUSE_TRACE_FACILITY == 1 ) && ( configUSE_STATS_FORMATTING_FUNCTIONS > 0 ) && ( configSUPPORT_DYNAMIC_ALLOCATION == 1 ) )
_attribute_ble_data_retention_ static TaskHandle_t hCpuTask = NULL;
/**
 * @brief        This function is a task used by debug to output the RTOS stack status
 * @param[in]    none
 * @return      none
 */
static void cpu_task( void *pvParameters )
{
    char pWriteBuffer[512];
    for(;;)
     {
        vTaskDelay(pdMS_TO_TICKS(10000));
        vTaskList((char *)&pWriteBuffer);
        printf("task_name   task_state priority stack tasK_num\n");
        printf("%s",pWriteBuffer);
    }
    vTaskDelete(NULL);
     return;
}
#endif //#if ( ( configUSE_TRACE_FACILITY == 1 ) && ( configUSE_STATS_FORMATTING_FUNCTIONS > 0 ) && ( configSUPPORT_DYNAMIC_ALLOCATION == 1 ) )


/**
 * @brief        This function is used to send a semaphore and can be called inside an interrupt.
 * @param[in]    none.
 * @return        none.
 */
void app_TaskCreate(void)
{
    /*If you wake up from deepretion mode, you don't need to create task again.
     *Only enable interrupt and resume general registers need to be opened */
    if(pm_is_MCU_deepRetentionWakeup()){
        configPOST_SLEEP_PROCESSING(0);
        extern void vPortRestoreTask(void);
        vPortRestoreTask();
    }

    /* Enable OS support */
    blc_setOsSupEnable(1);

#if (BLE_APP_PM_ENABLE)
    blc_ll_enOsPowerManagement_module();
    #if (PM_DEEPSLEEP_RETENTION_ENABLE)
        #if (MCU_CORE_TYPE == MCU_CORE_B91)
            #error "B91 deep retention 64K SRAM not enough !!!"
        #elif (MCU_CORE_TYPE == MCU_CORE_B92)
            blc_pm_setDeepsleepRetentionType(DEEPSLEEP_MODE_RET_SRAM_LOW96K);
        #elif (MCU_CORE_TYPE == MCU_CORE_TL721X)
            blc_pm_setDeepsleepRetentionType(DEEPSLEEP_MODE_RET_SRAM_LOW128K);
        #elif(MCU_CORE_TYPE == MCU_CORE_TL321X)
            blc_pm_setDeepsleepRetentionType(DEEPSLEEP_MODE_RET_SRAM_LOW96K);
        #else
            #error "This chip doesn't support deep retention !!!"
        #endif
    #endif  //(PM_DEEPSLEEP_RETENTION_ENABLE)
#endif //#if (BLE_APP_PM_ENABLE)

#if UI_LED_ENABLE
    app_ledTaskCreate();
#endif //#if UI_LED_ENABLE

    app_BleTaskCreate();
    os_give_sem(); /* !!! important */

#if ( ( configUSE_TRACE_FACILITY == 1 ) && ( configUSE_STATS_FORMATTING_FUNCTIONS > 0 ) && ( configSUPPORT_DYNAMIC_ALLOCATION == 1 ) )
      xTaskCreate( cpu_task, "cpu_task", configMINIMAL_STACK_SIZE*4, (void*)0, (tskIDLE_PRIORITY), &hCpuTask );
#endif //#if ( ( configUSE_TRACE_FACILITY == 1 ) && ( configUSE_STATS_FORMATTING_FUNCTIONS > 0 ) && ( configSUPPORT_DYNAMIC_ALLOCATION == 1 ) )

     printf("app Task Create complete\r\n");
}

#endif   //#if(FREERTOS_ENABLE)
