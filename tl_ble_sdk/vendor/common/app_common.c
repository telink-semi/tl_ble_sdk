/********************************************************************************************************
 * @file    app_common.c
 *
 * @brief   This is the source file for BLE SDK
 *
 * @author  BLE GROUP
 * @date    06,2020
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
#include "tl_common.h"
#include "drivers.h"
#include "stack/ble/ble.h"

#if defined(TLK_ONLY_BLE_HOST)
#include "stack/pm/pm_sys.h"
#endif

#if PM_DEEPSLEEP_RETENTION_ENABLE
/**
 * @brief       this function is used to set deepsleep retention SRAM size for all application project.
 *              select DEEPSLEEP_MODE_RET_SRAM_** according to retention data size used.
 * @param[in]   none
 * @return      none
 */
void blc_app_setDeepsleepRetentionSramSize(void)
{
    #if  ((MCU_CORE_TYPE == MCU_CORE_TL721X) || (MCU_CORE_TYPE == MCU_CORE_TL321X) || (MCU_CORE_TYPE == MCU_CORE_TL323X))
    __asm__ volatile (
        ".option push\n"
        ".option norelax\n"
    );
    #endif
    extern u32 _RETENTION_RESET_VMA_START;
    u32        deepret_start = (u32)&_RETENTION_RESET_VMA_START;

    extern u32 _RAMCODE_VMA_END;
    u32        deepret_end = (u32)&_RAMCODE_VMA_END;

    u32 deepret_size = deepret_end - deepret_start;
    #if  ((MCU_CORE_TYPE == MCU_CORE_TL721X) || (MCU_CORE_TYPE == MCU_CORE_TL321X)  || (MCU_CORE_TYPE == MCU_CORE_TL323X))
    __asm__ volatile (
        ".option pop\n"
    );
    #endif
    #if (MCU_CORE_TYPE == MCU_CORE_TL322X)      //todo-jk: deepRet size for Condor in not set automatically yet, because D25F should konw N22 usage.
        #if defined(TLK_ONLY_BLE_HOST)          //todo-jk: design dual-core/single-core unified code.
    if (1) {
        blc_pm_setDeepsleepRetentionType(DEEPSLEEP_MODE_RET_SRAM_LOW384K);
        tlkapi_printf(APP_LOG_COMMON_EN, "[APP][COMMON] deep retention size 348K\r\n");
        return;
    }
        #endif
    #endif //#if (MCU_CORE_TYPE == MCU_CORE_TL322X)
    if (deepret_size <= 0x8000) {
        blc_pm_setDeepsleepRetentionType(DEEPSLEEP_MODE_RET_SRAM_LOW32K);
        tlkapi_printf(APP_LOG_COMMON_EN, "[APP][COMMON] deep retention size 32K\r\n");
    }
#if (MCU_CORE_TYPE != MCU_CORE_TL323X)
    else if (deepret_size <= 0x10000) {
        blc_pm_setDeepsleepRetentionType(DEEPSLEEP_MODE_RET_SRAM_LOW64K);
        tlkapi_printf(APP_LOG_COMMON_EN, "[APP][COMMON] deep retention size 64K\r\n");
    }
#endif /* MCU_CORE_TYPE != MCU_CORE_TL323X */
    #if ((MCU_CORE_TYPE == MCU_CORE_B92) || (MCU_CORE_TYPE == MCU_CORE_TL321X) || (MCU_CORE_TYPE == MCU_CORE_TL323X))
    else if (deepret_size <= 0x18000) {
        blc_pm_setDeepsleepRetentionType(DEEPSLEEP_MODE_RET_SRAM_LOW96K);
        tlkapi_printf(APP_LOG_COMMON_EN, "[APP][COMMON] deep retention size 96K\r\n");
    }
    #endif //#if ((MCU_CORE_TYPE == MCU_CORE_B92) || (MCU_CORE_TYPE == MCU_CORE_TL321X)  || (MCU_CORE_TYPE == MCU_CORE_TL323X) )

    #if (MCU_CORE_TYPE == MCU_CORE_TL721X) || (MCU_CORE_TYPE == MCU_CORE_TL322X) || (MCU_CORE_TYPE == MCU_CORE_TL521X)
    else if (deepret_size <= 0x20000) {
        blc_pm_setDeepsleepRetentionType(DEEPSLEEP_MODE_RET_SRAM_LOW128K);
        tlkapi_printf(APP_LOG_COMMON_EN, "[APP][COMMON] deep retention size 128K\r\n");
    }
    #endif //#if (MCU_CORE_TYPE == MCU_CORE_TL721X) || (MCU_CORE_TYPE == MCU_CORE_TL322X)

    #if (MCU_CORE_TYPE == MCU_CORE_TL323X)
    else if (deepret_size <= 0x28000) {
        blc_pm_setDeepsleepRetentionType(DEEPSLEEP_MODE_RET_SRAM_LOW160K);
        tlkapi_printf(APP_LOG_COMMON_EN, "[APP][COMMON] deep retention size 128K\r\n");
    }
    #endif

    #if (MCU_CORE_TYPE == MCU_CORE_TL521X)
    else if (deepret_size <= 0x30000) {
        blc_pm_setDeepsleepRetentionType(DEEPSLEEP_MODE_RET_SRAM_LOW192K);
        tlkapi_printf(APP_LOG_COMMON_EN, "[APP][COMMON] deep retention size 192K\r\n");
    }
    #endif

    #if (MCU_CORE_TYPE == MCU_CORE_TL721X) || (MCU_CORE_TYPE == MCU_CORE_TL322X) || (MCU_CORE_TYPE == MCU_CORE_TL521X)
    else if (deepret_size <= 0x40000) {
        blc_pm_setDeepsleepRetentionType(DEEPSLEEP_MODE_RET_SRAM_LOW256K);
        tlkapi_printf(APP_LOG_COMMON_EN, "[APP][COMMON] deep retention size 256K\r\n");
    }
    #endif //#if (MCU_CORE_TYPE == MCU_CORE_TL721X) || (MCU_CORE_TYPE == MCU_CORE_TL322X)
    #if (MCU_CORE_TYPE == MCU_CORE_TL322X)
    else if (deepret_size <= 0x60000) {
        blc_pm_setDeepsleepRetentionType(DEEPSLEEP_MODE_RET_SRAM_LOW384K);
        tlkapi_printf(APP_LOG_COMMON_EN, "[APP][COMMON] deep retention size 384K\r\n");
    }
    #endif //#if (MCU_CORE_TYPE == MCU_CORE_TL322X)
    else {
        /* retention size too large, overflow. deep retention size setting err*/
        tlkapi_printf(APP_LOG_COMMON_EN, "[APP][COMMON] deep retention size setting err");
    }

}
#endif //#if PM_DEEPSLEEP_RETENTION_ENABLE


/**
 * @brief      This function is check If IRK(local IRK or peer IRK) valid
 * @param[in]  pIrk - pointer to IRK(local IRK or peer IRK)
 * @return     1: IRK valid
 *             0: IRK invalid
 */
int blc_app_isIrkValid(u8 *pIrk)
{
    u8 byte_all_0xFF[16] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    u8 byte_all_0x00[16] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

    /* IRK valid: not 16 0x00, not 16 0xFF */
    if (memcmp(pIrk, byte_all_0xFF, 16) && memcmp(pIrk, byte_all_0x00, 16)) {
        return TRUE;
    } else {
        return FALSE;
    }
}
