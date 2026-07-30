/********************************************************************************************************
 * @file    audio_common.h
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
#ifndef AUDIO_COMMON_H_
#define AUDIO_COMMON_H_

#define TL_AUDIO_MASK_SBC_MODE                    (0x00000001)
#define TL_AUDIO_MASK_MSBC_MODE                   (0x00000002)
#define TL_AUDIO_MASK_ADPCM_MODE                  (0x00000004)
#define TL_AUDIO_MASK_OPUS_MODE                   (0x00000008)

#define TL_AUDIO_MASK_HID_SERVICE_CHANNEL         (0x00000100)
#define TL_AUDIO_MASK_GATT_SERVICE_TELINK_CHANNEL (0x00000200)
#define TL_AUDIO_MASK_GATT_SERVICE_GOOGLE_CHANNEL (0x00000400)
#define TL_AUDIO_MASK_GATT_SERVICE_AMAZON_CHANNEL (0x00000800)

#define TL_AUDIO_MASK_DONGLE_TO_STB               (0x00010000)

#define RCU_PROJECT                               (0x01000000)
#define DONGLE_PROJECT                            (0x02000000)

#define AUDIO_DISABLE                             0

//RCU Audio Mode

#define TL_AUDIO_RCU_ADPCM_GATT_TELINK       (RCU_PROJECT | TL_AUDIO_MASK_ADPCM_MODE | TL_AUDIO_MASK_GATT_SERVICE_TELINK_CHANNEL)

#define TL_AUDIO_RCU_ADPCM_GATT_GOOGLE       (RCU_PROJECT | TL_AUDIO_MASK_ADPCM_MODE | TL_AUDIO_MASK_GATT_SERVICE_GOOGLE_CHANNEL)

#define TL_AUDIO_RCU_ADPCM_HID               (RCU_PROJECT | TL_AUDIO_MASK_ADPCM_MODE | TL_AUDIO_MASK_HID_SERVICE_CHANNEL)

#define TL_AUDIO_RCU_SBC_HID                 (RCU_PROJECT | TL_AUDIO_MASK_SBC_MODE | TL_AUDIO_MASK_HID_SERVICE_CHANNEL)

#define TL_AUDIO_RCU_ADPCM_HID_DONGLE_TO_STB (RCU_PROJECT | TL_AUDIO_MASK_ADPCM_MODE | TL_AUDIO_MASK_HID_SERVICE_CHANNEL | TL_AUDIO_MASK_DONGLE_TO_STB)

#define TL_AUDIO_RCU_SBC_HID_DONGLE_TO_STB   (RCU_PROJECT | TL_AUDIO_MASK_SBC_MODE | TL_AUDIO_MASK_HID_SERVICE_CHANNEL | TL_AUDIO_MASK_DONGLE_TO_STB)

#define TL_AUDIO_RCU_MSBC_HID                (RCU_PROJECT | TL_AUDIO_MASK_MSBC_MODE | TL_AUDIO_MASK_HID_SERVICE_CHANNEL)

#define TL_AUDIO_RCU_OPUS_GATT_AMAZON        (RCU_PROJECT | TL_AUDIO_MASK_OPUS_MODE | TL_AUDIO_MASK_GATT_SERVICE_AMAZON_CHANNEL)

//Dongle Audio Mode

#define TL_AUDIO_DONGLE_ADPCM_GATT_TELINK       (DONGLE_PROJECT | TL_AUDIO_MASK_ADPCM_MODE | TL_AUDIO_MASK_GATT_SERVICE_TELINK_CHANNEL)

#define TL_AUDIO_DONGLE_ADPCM_GATT_GOOGLE       (DONGLE_PROJECT | TL_AUDIO_MASK_ADPCM_MODE | TL_AUDIO_MASK_GATT_SERVICE_GOOGLE_CHANNEL)

#define TL_AUDIO_DONGLE_ADPCM_HID               (DONGLE_PROJECT | TL_AUDIO_MASK_ADPCM_MODE | TL_AUDIO_MASK_HID_SERVICE_CHANNEL)

#define TL_AUDIO_DONGLE_SBC_HID                 (DONGLE_PROJECT | TL_AUDIO_MASK_SBC_MODE | TL_AUDIO_MASK_HID_SERVICE_CHANNEL)

#define TL_AUDIO_DONGLE_ADPCM_HID_DONGLE_TO_STB (DONGLE_PROJECT | TL_AUDIO_MASK_ADPCM_MODE | TL_AUDIO_MASK_HID_SERVICE_CHANNEL | TL_AUDIO_MASK_DONGLE_TO_STB)

#define TL_AUDIO_DONGLE_SBC_HID_DONGLE_TO_STB   (DONGLE_PROJECT | TL_AUDIO_MASK_SBC_MODE | TL_AUDIO_MASK_HID_SERVICE_CHANNEL | TL_AUDIO_MASK_DONGLE_TO_STB)

#define TL_AUDIO_DONGLE_MSBC_HID                (DONGLE_PROJECT | TL_AUDIO_MASK_MSBC_MODE | TL_AUDIO_MASK_HID_SERVICE_CHANNEL)

#define TL_AUDIO_DONGLE_OPUS_GATT_AMAZON        (DONGLE_PROJECT | TL_AUDIO_MASK_OPUS_MODE | TL_AUDIO_MASK_GATT_SERVICE_AMAZON_CHANNEL)


#endif /* AUDIO_COMMON_H_ */
