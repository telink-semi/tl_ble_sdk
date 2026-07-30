/********************************************************************************************************
 * @file    app.h
 *
 * @brief   This is the header file for 2.4G SDK
 *
 * @author  2.4G GROUP
 * @date    02,2025
 *
 * @par     Copyright (c) 2025, Telink Semiconductor (Shanghai) Co., Ltd. ("TELINK")
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
#ifndef APP_H_
#define APP_H_

#if(FEATURE_TEST_MODE == PER)

#if (APP_FLASH_PROTECTION_ENABLE)
void app_flash_protection_operation(u8 flash_op_evt, u32 op_addr_begin, u32 op_addr_end);
#endif
#if (RF_DEBUG_IO_ENABLE)
void debug_config(void);
#endif


/**
 * @brief       user initialization when MCU power on or wake_up from deepSleep mode
 * @param[in]   none
 * @return      none
 */
void user_init_normal(void);

/**
 * @brief       user initialization when MCU wake_up from deepSleep_retention mode
 * @param[in]   none
 * @return      none
 */
void user_init_deepRetn(void);


/**
 * @brief     BLE main loop
 * @param[in]  none.
 * @return     none.
 */
void main_loop(void);




/**
 * @brief      this function is used to detect if key pressed or released.
 * @param[in]  e - LinkLayer Event type
 * @param[in]  p - data pointer of event
 * @param[in]  n - data length of event
 * @return     none
 */
void proc_keyboard(u8 e, u8 *p, int n);


/**
 * @brief       this function is used to detect if button pressed or released.
 * @param[in]   e - event type when this function is triggered by LinkLayer event
 * @param[in]   p - event callback data pointer for when this function is triggered by LinkLayer event
 * @param[in]   n - event callback data length when this function is triggered by LinkLayer event
 * @return      none
 */
void proc_button(u8 e, u8 *p, int n);


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
void app_flash_protection_operation(u8 flash_op_evt, u32 op_addr_begin, u32 op_addr_end);

#endif /* APP_H_ */

#endif
