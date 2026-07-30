/********************************************************************************************************
 * @file    prf_debug.h
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

/////////////////////////Profile Debug Log Macro Control////////////////////////////

#ifndef PROFILE_DEBUG_LOG_EN
    #define PROFILE_DEBUG_LOG_EN 1
#endif

#ifndef PRF_DBG_COMMON_EN
    #define PRF_DBG_COMMON_EN 1
#endif

extern const unsigned char DBG_PRF_MASK_COMMON_LOG;

#define BLC_PROFILE_DEBUG(en, fmt, ...)                     \
    if (PROFILE_DEBUG_LOG_EN) {                             \
        tlkapi_printf(en, "[PRF]" fmt "\n", ##__VA_ARGS__); \
    }

/////////////////////////Bluetooth Basic Profile Debug Log//////////////////////////
#ifndef PRF_DBG_DIS_EN
    #define PRF_DBG_DIS_EN 1
#endif

#ifndef PRF_DBG_BAS_EN
    #define PRF_DBG_BAS_EN 1
#endif

#ifndef PRF_DBG_SCPS_EN
    #define PRF_DBG_SCPS_EN 1
#endif

#ifndef PRF_DBG_GATTS_EN
#define PRF_DBG_GATTS_EN                                        1
#endif

extern const unsigned char DBG_PRF_MASK_DIS_LOG;
extern const unsigned char DBG_PRF_MASK_BAS_LOG;
extern const unsigned char DBG_PRF_MASK_SCPS_LOG;
extern const unsigned char DBG_PRF_MASK_GATTS_LOG;
/////////////////////////LE Audio Debug Log/////////////////////////////////////////
#ifndef DBG_PRF_AUD_LOG
    #define DBG_PRF_AUD_LOG 1
#endif

#ifndef PRF_DBG_AUDIO_EN
    #define PRF_DBG_AUDIO_EN 1
#endif

#ifndef PRF_DBG_ASCS_EN
    #define PRF_DBG_ASCS_EN 1
#endif

#ifndef PRF_DBG_BASS_EN
    #define PRF_DBG_BASS_EN 1
#endif

#ifndef PRF_DBG_PACS_EN
    #define PRF_DBG_PACS_EN 1
#endif

#ifndef PRF_DBG_BAP_EN
    #define PRF_DBG_BAP_EN 1
#endif

#ifndef PRF_DBG_BCST_EN
    #define PRF_DBG_BCST_EN 1
#endif

#ifndef PRF_DBG_CSIS_EN
    #define PRF_DBG_CSIS_EN 1
#endif

#ifndef PRF_DBG_OTS_EN
    #define PRF_DBG_OTS_EN 1
#endif

#ifndef PRF_DBG_MCS_EN
    #define PRF_DBG_MCS_EN 1
#endif

#ifndef PRF_DBG_TBS_EN
    #define PRF_DBG_TBS_EN 1
#endif

#ifndef PRF_DBG_VCS_EN
    #define PRF_DBG_VCS_EN 1
#endif

#ifndef PRF_DBG_VOCS_EN
    #define PRF_DBG_VOCS_EN 1
#endif

#ifndef PRF_DBG_MICS_EN
    #define PRF_DBG_MICS_EN 1
#endif

#ifndef PRF_DBG_AICS_EN
    #define PRF_DBG_AICS_EN 1
#endif

#ifndef PRF_DBG_CAP_EN
    #define PRF_DBG_CAP_EN 1
#endif

#ifndef PRF_DBG_PBP_EN
    #define PRF_DBG_PBP_EN 1
#endif

#ifndef PRF_DBG_TMAS_EN
    #define PRF_DBG_TMAS_EN 1
#endif

#ifndef PRF_DBG_HAS_EN
    #define PRF_DBG_HAS_EN 1
#endif

#ifndef PRF_DBG_UCP_EN
    #define PRF_DBG_UCP_EN 1
#endif

#ifndef PRF_DBG_STORE_EN
    #define PRF_DBG_STORE_EN 0
#endif

extern const unsigned char DBG_PRF_MASK_AUD_LOG;
extern const unsigned char DBG_PRF_MASK_ASCS_LOG;
extern const unsigned char DBG_PRF_MASK_BASS_LOG;
extern const unsigned char DBG_PRF_MASK_PACS_LOG;
extern const unsigned char DBG_PRF_MASK_BAP_LOG;
extern const unsigned char DBG_PRF_MASK_BCST_LOG;
extern const unsigned char DBG_PRF_MASK_CSIS_LOG;
extern const unsigned char DBG_PRF_MASK_OTS_LOG;
extern const unsigned char DBG_PRF_MASK_MCS_LOG;
extern const unsigned char DBG_PRF_MASK_TBS_LOG;
extern const unsigned char DBG_PRF_MASK_VCS_LOG;
extern const unsigned char DBG_PRF_MASK_VOCS_LOG;
extern const unsigned char DBG_PRF_MASK_MICS_LOG;
extern const unsigned char DBG_PRF_MASK_AICS_LOG;
extern const unsigned char DBG_PRF_MASK_CAP_LOG;
extern const unsigned char DBG_PRF_MASK_PBP_LOG;
extern const unsigned char DBG_PRF_MASK_TMAS_LOG;
extern const unsigned char DBG_PRF_MASK_HAS_LOG;
extern const unsigned char DBG_PRF_MASK_UCP_LOG;
extern const unsigned char DBG_PRF_MASK_STORE_LOG;


/////////////////////////Channel Sounding(HADM) Debug Log////////////////////////////
#ifndef PRF_DBG_RAS_EN
    #define PRF_DBG_RAS_EN 0
#endif

#ifndef PRF_DBG_RAS_DATA_EN
    #define PRF_DBG_RAS_DATA_EN 0
#endif

#ifndef PRF_DBG_RAS_MEM_EN
    #define PRF_DBG_RAS_MEM_EN 0
#endif

extern const unsigned char DBG_PRF_MASK_RAS_LOG;
extern const unsigned char DBG_PRF_MASK_RAS_DATA_LOG;

/////////////////////////Electronic Shelf Label(ESL) Debug Log///////////////////////

#ifndef PRF_DBG_ESLS_EN
    #define PRF_DBG_ESLS_EN 1
#endif

#ifndef PRF_DBG_ESL_ESLP_EN
    #define PRF_DBG_ESL_ESLP_EN 1
#endif

extern const unsigned char DBG_PRF_MASK_ESLS_LOG;
extern const unsigned char DBG_PRF_MASK_ESL_ESLP_LOG;

/////////////////////////HID ISO Debug Log///////////////////////
#ifndef PRF_DBG_HID_EN
    #define PRF_DBG_HID_EN 1
#endif


#ifndef PRF_DBG_HID_ISO_EN
    #define PRF_DBG_HID_ISO_EN 1
#endif

extern const unsigned char DBG_PRF_MASK_HID_LOG;
extern const unsigned char DBG_PRF_MASK_ULL_HID_LOG;

/////////////////////////Test Profile Debug Log////////////////////////////////////
#ifndef PRF_DBG_TEST_PRF_EN
    #define PRF_DBG_TEST_PRF_EN 1
#endif

extern const unsigned char DBG_PRF_MASK_TEST_PRF_LOG;
