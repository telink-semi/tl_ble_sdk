/********************************************************************************************************
 * @file    app_audio.h
 *
 * @brief   This is the header file for BLE SDK
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
#ifndef APP_AUDIO_H_
#define APP_AUDIO_H_

#define APP_AUDIO_BT_OPEN    0x01
#define APP_AUDIO_BT_CLOSE   0x00
#define APP_AUDIO_BT_CONFIG  0x02

extern  unsigned int        key_voice_pressTick;
extern  unsigned char       ui_mic_enable;
extern  unsigned char       key_voice_press;
extern  int                 ui_mtu_size_exchange_req;

/**
 * @brief      the func serve to init dmic
 * @param[in]  none
 * @return     none
 */
void dmic_gpio_reset (void);

/**
 * @brief      the func serve to init amic
 * @param[in]  none
 * @return     none
 */
void amic_gpio_reset (void);

/**
 * @brief      for open the audio and mtu size exchange
 * @param[in]  en   0:close the micphone  1:open the micphone
 * @return     none
 */
void ui_enable_mic (int en);

/**
 * @brief      for open the audio and mtu size exchange
 * @param[in]  none
 * @return     none
 */
void voice_press_proc(void);

/**
 * @brief      audio task in loop for encode and transmit encode data
 * @param[in]  none
 * @return     none
 */
void task_audio (void);

/**
 * @brief      This function serves to Request ConnParamUpdate
 * @param[in]  none
 * @return     none
 */
void blc_checkConnParamUpdate(void);

/**
 * @brief      audio proc in main loop
 * @param[in]  none
 * @return     none
 */
void proc_audio(void);

/**
 * @brief      this function is call back function of audio measurement from server to client
 * @param[in]  p:data pointer.
 * @return     0
 */
int server2client_auido_proc(void* p);

/**
 * @brief      this function is used to check audio state
 * @param[in]  none
 * @return     none
 */
void audio_state_check(void);

/**
 * @brief      this function is used to define what to do when voice key is pressed
 * @param[in]  none
 * @return     none
 */
void key_voice_is_press(void);

/**
 * @brief      this function is used to define what to do when voice key is released
 * @param[in]  none
 * @return     none
 */
void key_voice_is_release(void);

/**
 * @brief      This function is the microphone delay function.
 * @param[in]  delay_time: microphone delay duration, unit is us.
 * @return     none
 */
void audio_proc_delay(u32 delay_time_us);

#endif /* APP_AUDIO_H_ */
