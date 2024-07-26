/********************************************************************************************************
 * @file    app_ui.h
 *
 * @brief   This is the header file for BLE SDK
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
#ifndef APP_UI_H_
#define APP_UI_H_



extern  int central_pairing_enable;
extern  u16 central_unpair_enable;

extern  u16 central_disconnect_connhandle;
extern  int key_not_released;

/**
 * @brief      keyboard task handler
 * @param[in]  e    - event type
 * @param[in]  p    - Pointer point to event parameter.
 * @param[in]  n    - the length of event parameter.
 * @return     none.
 */
void proc_keyboard (u8 e, u8 *p, int n);

/**
 * @brief   BLE Unpair handle for central
 * @param   none.
 * @return  none.
 */
void proc_central_role_unpair(void);

/**
 * @brief      keyboard initialization
 * @param[in]  none
 * @return     none.
 */
void keyboard_init(void);


#endif /* APP_UI_H_ */
