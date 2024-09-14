/********************************************************************************************************
 * @file    codec_0581.h
 *
 * @brief   This is the header file for B92
 *
 * @author  Driver Group
 * @date    2023
 *
 * @par     Copyright (c) 2019, Telink Semiconductor (Shanghai) Co., Ltd. ("TELINK")
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
#ifndef _CODEC_0581_H_
#define _CODEC_0581_H_

#include "audio.h"
#include "clock.h"
#include "gpio.h"
#include "spi.h"
#include "stimer.h"
#include "reg_include/register.h"

/**
 * @brief  enum for codec_0581 bool type
 */
typedef enum {
    CODEC_BOOL_FALSE  = 0,
    CODEC_BOOL_TRUE   = 1,
} codec_0581_bool_e;

/**
 * @brief  enum for codec_0581 error code
 */
typedef enum {
    CODEC_0581_OK                       =  0,           /*!< No Error */
    CODEC_0581_ERROR                    = -1,           /*!< General Error  */
    CODEC_0581_NULL_PARAM               = -2,           /*!< Null parameter */
    CODEC_0581_INVALID_PARAM            = -3,           /*!< Invalid parameter passed */
    CODEC_0581_NOT_SUPPORTED            = -4,           /*!< Not supported */
    CODEC_0581_PLL_NOT_LOCKED           = -5,           /*!< PLL is not locked */
}codec_0581_error_e;

/**
 * @brief  enum for ADC channel
 */
typedef enum {
    CODEC_ADC_CHNL_0  = 0,
    CODEC_ADC_CHNL_1  = 1,
    CODEC_ADC_CHNL_2  = 2,
} codec_0581_adc_chnl_e;

/**
 * @brief  enum for ADC sample rate
 */
typedef enum {
    CODEC_ADC_SAMPLE_RATE_12KHz  = 0,
    CODEC_ADC_SAMPLE_RATE_24KHz  = 1,
    CODEC_ADC_SAMPLE_RATE_48KHz  = 2,
    CODEC_ADC_SAMPLE_RATE_96KHz  = 3,
    CODEC_ADC_SAMPLE_RATE_192KHz = 4,
    CODEC_ADC_SAMPLE_RATE_384KHz = 5,
    CODEC_ADC_SAMPLE_RATE_768KHz = 6
} codec_0581_adc_sample_rate_e;

/**
 * @brief  enum for ADC HPF. (HPF -> high pass filter)
 */
typedef enum {
    CODEC_ADC_HPF_DISABLE        = 0,                         /* ADC high pass filter off */
    CODEC_ADC_HPF_CUTOFF_1HZ     = 1,                         /* ADC high pass filter, cut off freq. 1Hz */
    CODEC_ADC_HPF_CUTOFF_4HZ     = 2,                         /* ADC high pass filter, cut off freq. 4Hz */
    CODEC_ADC_HPF_CUTOFF_8HZ     = 3                          /* ADC high pass filter, cut off freq. 5Hz */
} codec_0581_adc_hpf_e;

/**
 * @brief  enum for ADC dither level
 */
typedef enum {
    CODEC_ADC_DITHER_DISABLE    = 0,                          /* dither off */
    CODEC_ADC_DITHER_QUARTE_LSB = 1,                          /* 1/4 lsb */
    CODEC_ADC_DITHER_HALF_LSB   = 2                           /* 1/2 lsb */
} codec_0581_adc_dither_level_e;

/**
 * @brief  enum for ASRCO channel.(ASRCO -> output asynchronous sample rate converter)
 */
typedef enum {
    CODEC_ASRCO_CHNL_0   = 0,
    CODEC_ASRCO_CHNL_1   = 1,
    CODEC_ASRCO_CHNL_2   = 2,
} codec_0581_asrco_chnl_e;

/**
 * @brief  enum for ASRC channel.(ASRC -> asynchronous sample rate converter)
 */
typedef enum {
    CODEC_ASRC_CHNL_ASRCI0   = 0,
    CODEC_ASRC_CHNL_ASRCO0   = 4,
    CODEC_ASRC_CHNL_ASRCO1   = 5,
    CODEC_ASRC_CHNL_ASRCO2   = 6,
} codec_0581_asrc_chnl_e;

/**
 * @brief  enum for ASRCI output and ASRCO input frequency.(ASRCI -> input asynchronous sample rate converter)
 */
typedef enum {
    CODEC_ASRC_FS_12K    = 0,
    CODEC_ASRC_FS_24K    = 1,
    CODEC_ASRC_FS_48K    = 2,
    CODEC_ASRC_FS_96K    = 3,
    CODEC_ASRC_FS_192K   = 4,
} codec_0581_asrc_fs_e;

/**
 * @brief  enum for ASRCO route channel
 */
typedef enum {
    CODEC_ASRCO_ROUTE_ADC0   = 0,
    CODEC_ASRCO_ROUTE_ADC1   = 1,
    CODEC_ASRCO_ROUTE_ADC2   = 2,
    CODEC_ASRCO_ROUTE_EQ     = 3,
    CODEC_ASRCO_ROUTE_FDEC0  = 4,
    CODEC_ASRCO_ROUTE_FDEC1  = 5,
    CODEC_ASRCO_ROUTE_FDEC2  = 6,
    CODEC_ASRCO_ROUTE_FDEC3  = 7,
    CODEC_ASRCO_ROUTE_FDSP0  = 8,
    CODEC_ASRCO_ROUTE_FDSP1  = 9,
    CODEC_ASRCO_ROUTE_FDSP2  = 10,
    CODEC_ASRCO_ROUTE_FDSP3  = 11,
    CODEC_ASRCO_ROUTE_FDSP4  = 12,
    CODEC_ASRCO_ROUTE_FDSP5  = 13,
    CODEC_ASRCO_ROUTE_FDSP6  = 14,
    CODEC_ASRCO_ROUTE_FDSP7  = 15,
    CODEC_ASRCO_ROUTE_FDSP8  = 16,
    CODEC_ASRCO_ROUTE_FDSP9  = 17,
    CODEC_ASRCO_ROUTE_FDSP10 = 18,
    CODEC_ASRCO_ROUTE_FDSP11 = 19,
    CODEC_ASRCO_ROUTE_FDSP12 = 20,
    CODEC_ASRCO_ROUTE_FDSP13 = 21,
    CODEC_ASRCO_ROUTE_FDSP14 = 22,
    CODEC_ASRCO_ROUTE_FDSP15 = 23,
} codec_0581_asrco_route_chnl_e;

/**
 * @brief  enum for PLL source
 */
typedef enum {
    CODEC_CLK_PLL_SOURCE_MCLKIN = 0,
    CODEC_CLK_PLL_SOURCE_FSYNC0 = 1,
    CODEC_CLK_PLL_SOURCE_BCLK0  = 2,
    CODEC_CLK_PLL_SOURCE_RCOSC = 3,
} codec_0581_clk_pll_source_e;

/**
 * @brief  enum for PLL type.
 */
typedef enum {
    CODEC_CLK_PLL_TYPE_INTEGER = 0,
    CODEC_CLK_PLL_TYPE_FRAC    = 1
} codec_0581_clk_pll_type_e;

/**
 * @brief  enum for sync_source.
 */
typedef enum {
    CODEC_CLK_SYNC_SOURCE_LRCLK0   = 0,
    CODEC_CLK_SYNC_SOURCE_LRCLK1   = 1,
    CODEC_CLK_SYNC_SOURCE_ASRCI    = 2,
    CODEC_CLK_SYNC_SOURCE_INTERNAL = 3
} codec_0581_clk_sync_source_e;

/**
 * @brief  enum for DAC sample rate
 */
typedef enum {
    CODEC_DAC_SAMPLE_RATE_12KHz  = 0,
    CODEC_DAC_SAMPLE_RATE_24KHz  = 1,
    CODEC_DAC_SAMPLE_RATE_48KHz  = 2,
    CODEC_DAC_SAMPLE_RATE_96KHz  = 3,
    CODEC_DAC_SAMPLE_RATE_192KHz = 4,
    CODEC_DAC_SAMPLE_RATE_384KHz = 5,
    CODEC_DAC_SAMPLE_RATE_768KHz = 6
} codec_0581_dac_sample_rate_e;

/**
 * @brief  enum for DAC input routing
 */
typedef enum {
    CODEC_DAC_ROUTE_ADC0           = 0,
    CODEC_DAC_ROUTE_ADC1           = 1,
    CODEC_DAC_ROUTE_ADC2           = 2,
    CODEC_DAC_ROUTE_ASRCI0         = 3,
    CODEC_DAC_ROUTE_EQ             = 4,
    CODEC_DAC_ROUTE_FDSP0          = 5,
    CODEC_DAC_ROUTE_FDSP1          = 6,
    CODEC_DAC_ROUTE_FDSP2          = 7,
    CODEC_DAC_ROUTE_FDSP3          = 8,
    CODEC_DAC_ROUTE_FDSP4          = 9,
    CODEC_DAC_ROUTE_FDSP5          = 10,
    CODEC_DAC_ROUTE_FDSP6          = 11,
    CODEC_DAC_ROUTE_FDSP7          = 12,
    CODEC_DAC_ROUTE_FDSP8          = 13,
    CODEC_DAC_ROUTE_FDSP9          = 14,
    CODEC_DAC_ROUTE_FDSP10         = 15,
    CODEC_DAC_ROUTE_FDSP11         = 16,
    CODEC_DAC_ROUTE_FDSP12         = 17,
    CODEC_DAC_ROUTE_FDSP13         = 18,
    CODEC_DAC_ROUTE_FDSP14         = 19,
    CODEC_DAC_ROUTE_FDSP15         = 20,
    CODEC_DAC_ROUTE_SAI0_00_LEFT   = 21,
    CODEC_DAC_ROUTE_SAI0_01_RIGHT  = 22,
    /** attention b92 is not support TDM mode
    CODEC_DAC_ROUTE_SAI0_02        = 23,
    CODEC_DAC_ROUTE_SAI0_03        = 24,
    CODEC_DAC_ROUTE_SAI0_04        = 25,
    CODEC_DAC_ROUTE_SAI0_05        = 26,
    CODEC_DAC_ROUTE_SAI0_06        = 27,
    CODEC_DAC_ROUTE_SAI0_07        = 28,
    CODEC_DAC_ROUTE_SAI0_08        = 29,
    CODEC_DAC_ROUTE_SAI0_09        = 30,
    CODEC_DAC_ROUTE_SAI0_10        = 31,
    CODEC_DAC_ROUTE_SAI0_11        = 32,
    CODEC_DAC_ROUTE_SAI0_12        = 33,
    CODEC_DAC_ROUTE_SAI0_13        = 34,
    CODEC_DAC_ROUTE_SAI0_14        = 35,
    CODEC_DAC_ROUTE_SAI0_15        = 36,
    */
    CODEC_DAC_ROUTE_FINT0          = 37,
    CODEC_DAC_ROUTE_FINT1          = 38
} codec_0581_dac_input_route_e;

/**
 * @brief  enum for EQ bank
 */
typedef enum {
    CODEC_EQ_BANK_0                = 0,
    CODEC_EQ_BANK_1                = 1,
}codec_0581_eq_bank_e;

/**
 * @brief  enum for EQ audio source route
 */
typedef enum {
    CODEC_EQ_ROUTE_SAI0_0_LEFT   = 0,                          /* SAP0 channel0(left) */
    CODEC_EQ_ROUTE_SAI0_1_RIGHT  = 1,                          /* SAP0 channel1(right) */
    /** attention b92 is not support TDM mode
    CODEC_EQ_ROUTE_SAI0_2        = 2,
    CODEC_EQ_ROUTE_SAI0_3        = 3,
    CODEC_EQ_ROUTE_SAI0_4        = 4,
    CODEC_EQ_ROUTE_SAI0_5        = 5,
    CODEC_EQ_ROUTE_SAI0_6        = 6,
    CODEC_EQ_ROUTE_SAI0_7        = 7,
    CODEC_EQ_ROUTE_SAI0_8        = 8,
    CODEC_EQ_ROUTE_SAI0_9        = 9,
    CODEC_EQ_ROUTE_SAI0_10       = 10,
    CODEC_EQ_ROUTE_SAI0_11       = 11,
    CODEC_EQ_ROUTE_SAI0_12       = 12,
    CODEC_EQ_ROUTE_SAI0_13       = 13,
    CODEC_EQ_ROUTE_SAI0_14       = 14,
    CODEC_EQ_ROUTE_SAI0_15       = 15,
    */
    CODEC_EQ_ROUTE_FDSP_0        = 16,                         /* FDSP channel0 */
    CODEC_EQ_ROUTE_FDSP_1        = 17,                         /* FDSP channel1 */
    CODEC_EQ_ROUTE_FDSP_2        = 18,                         /* FDSP channel2 */
    CODEC_EQ_ROUTE_FDSP_3        = 19,                         /* FDSP channel3 */
    CODEC_EQ_ROUTE_FDSP_4        = 20,                         /* FDSP channel4 */
    CODEC_EQ_ROUTE_FDSP_5        = 21,                         /* FDSP channel5 */
    CODEC_EQ_ROUTE_FDSP_6        = 22,                         /* FDSP channel6 */
    CODEC_EQ_ROUTE_FDSP_7        = 23,                         /* FDSP channel7 */
    CODEC_EQ_ROUTE_FDSP_8        = 24,                         /* FDSP channel8 */
    CODEC_EQ_ROUTE_FDSP_9        = 25,                         /* FDSP channel9 */
    CODEC_EQ_ROUTE_FDSP_10       = 26,                         /* FDSP channel10 */
    CODEC_EQ_ROUTE_FDSP_11       = 27,                         /* FDSP channel11 */
    CODEC_EQ_ROUTE_FDSP_12       = 28,                         /* FDSP channel12 */
    CODEC_EQ_ROUTE_FDSP_13       = 29,                         /* FDSP channel13 */
    CODEC_EQ_ROUTE_FDSP_14       = 30,                         /* FDSP channel14 */
    CODEC_EQ_ROUTE_FDSP_15       = 31,                         /* FDSP channel15 */
    CODEC_EQ_ROUTE_ASRCI_0       = 32,                         /* ASRCI channel0 */
    CODEC_EQ_ROUTE_ADC_0         = 33,                         /* ADC channel0 */
    CODEC_EQ_ROUTE_ADC_1         = 34,                         /* ADC channel1 */
    CODEC_EQ_ROUTE_ADC_2         = 35,                         /* ADC channel2 */
} codec_0581_eq_route_e;

/**
 * @brief  enum for FDEC channel.(FDEC -> frequency decimation)
 */
typedef enum {
    CODEC_FDEC_CHNL_0 = 0,
    CODEC_FDEC_CHNL_1 = 1,
    CODEC_FDEC_CHNL_2 = 2,
    CODEC_FDEC_CHNL_3 = 3,
} codec_0581_fdec_chnl_e;

/**
 * @brief  enum for FDEC channel pairs
 */
typedef enum {
    CODEC_FDEC_PAIR_CHNL_0_1 = 0,
    CODEC_FDEC_PAIR_CHNL_2_3 = 1,
} codec_0581_fdec_pair_chnl_e;

/**
 * @brief  enum for FDEC input sample rate
 */
typedef enum {
    CODEC_FDEC_IN_FS_24K     = 1,                            /* input sample rate 24K, start from 1 */
    CODEC_FDEC_IN_FS_48K     = 2,
    CODEC_FDEC_IN_FS_96K     = 3,
    CODEC_FDEC_IN_FS_192K    = 4,
    CODEC_FDEC_IN_FS_384K    = 5,
    CODEC_FDEC_IN_FS_768K    = 6,
} codec_0581_fdec_in_fs_e;

/**
 * @brief  enum for FDEC output sample rate
 */
typedef enum {
    CODEC_FDEC_OUT_FS_12K    = 0,
    CODEC_FDEC_OUT_FS_24K    = 1,
    CODEC_FDEC_OUT_FS_48K    = 2,
    CODEC_FDEC_OUT_FS_96K    = 3,
    CODEC_FDEC_OUT_FS_192K   = 4,
    CODEC_FDEC_OUT_FS_384K   = 5,

    CODEC_FDEC_OUT_FS_8K     = 8,
    CODEC_FDEC_OUT_FS_16K    = 9,
} codec_0581_fdec_out_fs_e;

/**
 * @brief  enum for FDEC route channel
 */
typedef enum {
    CODEC_FDEC_ROUTE_ADC0    = 0,
    CODEC_FDEC_ROUTE_ADC1    = 1,
    CODEC_FDEC_ROUTE_ADC2    = 2,
    CODEC_FDEC_ROUTE_ASRCI0  = 3,
    CODEC_FDEC_ROUTE_EQ      = 4,
    CODEC_FDEC_ROUTE_FDSP0   = 5,
    CODEC_FDEC_ROUTE_FDSP1   = 6,
    CODEC_FDEC_ROUTE_FDSP2   = 7,
    CODEC_FDEC_ROUTE_FDSP3   = 8,
    CODEC_FDEC_ROUTE_FDSP4   = 9,
    CODEC_FDEC_ROUTE_FDSP5   = 10,
    CODEC_FDEC_ROUTE_FDSP6   = 11,
    CODEC_FDEC_ROUTE_FDSP7   = 12,
    CODEC_FDEC_ROUTE_FDSP8   = 13,
    CODEC_FDEC_ROUTE_FDSP9   = 14,
    CODEC_FDEC_ROUTE_FDSP10  = 15,
    CODEC_FDEC_ROUTE_FDSP11  = 16,
    CODEC_FDEC_ROUTE_FDSP12  = 17,
    CODEC_FDEC_ROUTE_FDSP13  = 18,
    CODEC_FDEC_ROUTE_FDSP14  = 19,
    CODEC_FDEC_ROUTE_FDSP15  = 20,
} codec_0581_fdec_route_chnl_e;

/**
 * @brief  enum for FastDSP mode
 */
typedef enum {
    CODEC_FDSP_NORMAL_MODE = 0,
    CODEC_FDSP_TURBO_MODE =  1,
} codec_0581_fdsp_mode_e;

/**
 * @brief  enum for FastDSP parameter bank
 */
typedef enum {
    CODEC_FDSP_PARAM_BANK_A = 0,
    CODEC_FDSP_PARAM_BANK_B = 1,
    CODEC_FDSP_PARAM_BANK_C = 2
} codec_0581_fdsp_param_bank_e;

/**
 * @brief  enum for FastDSP parameter bank switch mode
 */
typedef enum {
    CODEC_FDSP_PARAM_SWITCH_RAMP    = 0,
    CODEC_FDSP_PARAM_SWITCH_INSTANT = 1
} codec_0581_fdsp_param_switch_mode_e;

/**
 * @brief  enum for FastDSP ramp rate
 */
typedef enum {
    CODEC_FDSP_RAMP_RATE_0P02SEC   = 0,                      /* 0.02s ramp */
    CODEC_FDSP_RAMP_RATE_0P04SEC   = 1,                      /* 0.04s ramp */
    CODEC_FDSP_RAMP_RATE_0P06SEC   = 2,                      /* 0.06s ramp */
    CODEC_FDSP_RAMP_RATE_0P08SEC   = 3,                      /* 0.08s ramp */
    CODEC_FDSP_RAMP_RATE_0P1SEC    = 4,                      /* 0.1s  ramp */
    CODEC_FDSP_RAMP_RATE_0P15SEC   = 5,                      /* 0.15s ramp */
    CODEC_FDSP_RAMP_RATE_0P2SEC    = 6,                      /* 0.2s  ramp */
    CODEC_FDSP_RAMP_RATE_0P25SEC   = 7,                      /* 0.25s ramp */
    CODEC_FDSP_RAMP_RATE_0P3SEC    = 8,                      /* 0.3s  ramp */
    CODEC_FDSP_RAMP_RATE_0P5SEC    = 9,                      /* 0.5s  ramp */
    CODEC_FDSP_RAMP_RATE_0P75SEC   = 10,                     /* 0.75s ramp */
    CODEC_FDSP_RAMP_RATE_1SEC      = 11,                     /* 1s    ramp */
    CODEC_FDSP_RAMP_RATE_1P25SEC   = 12,                     /* 1.25s ramp */
    CODEC_FDSP_RAMP_RATE_1P5SEC    = 13,                     /* 1.5s  ramp */
    CODEC_FDSP_RAMP_RATE_1P75SEC   = 14,                     /* 1.75s ramp */
    CODEC_FDSP_RAMP_RATE_2SEC      = 15                      /* 2s    ramp */
} codec_0581_fdsp_ramp_rate_e;

/**
 * @brief  enum for FastDSP copy parameter bank
 */
typedef enum {
    CODEC_FDSP_COPY_PARAM_BANK_A2B = 0,                      /* copy parameter bank A to bank B */
    CODEC_FDSP_COPY_PARAM_BANK_A2C = 1,                      /* copy parameter bank A to bank C */
    CODEC_FDSP_COPY_PARAM_BANK_B2A = 2,                      /* copy parameter bank B to bank A */
    CODEC_FDSP_COPY_PARAM_BANK_B2C = 3,                      /* copy parameter bank B to bank C */
    CODEC_FDSP_COPY_PARAM_BANK_C2A = 4,                      /* copy parameter bank C to bank A */
    CODEC_FDSP_COPY_PARAM_BANK_C2B = 5                       /* copy parameter bank C to bank B */
} codec_0581_fdsp_copy_param_bank_e;

/**
 * @brief  enum for FastDSP rate source
 */
typedef enum {
    CODEC_FDSP_RATE_SRC_ADC0_1     = 0,
    CODEC_FDSP_RATE_SRC_ADC2       = 1,
    CODEC_FDSP_RATE_SRC_EQ         = 5,
    CODEC_FDSP_RATE_SRC_SAI0       = 6,
    CODEC_FDSP_RATE_SRC_FINT0_1    = 10,
    CODEC_FDSP_RATE_SRC_ASRCI      = 14,
    CODEC_FDSP_RATE_SRC_FIXED      = 15
} codec_0581_fdsp_rate_src_e;

/**
 * @brief  enum for FINT channel.(FINT -> frequency interpolation)
 */
typedef enum {
    CODEC_FINT_CHNL_0 = 0,
    CODEC_FINT_CHNL_1 = 1,
} codec_0581_fint_chnl_e;

/**
 * @brief  enum for FINT channel pairs
 */
typedef enum {
    CODEC_FINT_PAIR_CHNL_0_1 = 0,
} codec_0581_fint_pair_chnl_e;

/**
 * @brief  enum for FINT output sample rate
 */
typedef enum {
    CODEC_FINT_OUT_FS_24K    = 1,                            /* input sample rate 24K, start from 1 */
    CODEC_FINT_OUT_FS_48K    = 2,
    CODEC_FINT_OUT_FS_96K    = 3,
    CODEC_FINT_OUT_FS_192K   = 4,
    CODEC_FINT_OUT_FS_384K   = 5,
    CODEC_FINT_OUT_FS_768K   = 6,
} codec_0581_fint_out_fs_e;

/**
 * @brief  enum for FINT input sample rate
 */
typedef enum {
    CODEC_FINT_IN_FS_12K     = 0,
    CODEC_FINT_IN_FS_24K     = 1,
    CODEC_FINT_IN_FS_48K     = 2,
    CODEC_FINT_IN_FS_96K     = 3,
    CODEC_FINT_IN_FS_192K    = 4,
    CODEC_FINT_IN_FS_384K    = 5,
    CODEC_FINT_IN_FS_8K      = 8,
    CODEC_FINT_IN_FS_16K     = 9,
} codec_0581_fint_in_fs_e;

/**
 * @brief  enum for FINT route channel
 */
typedef enum {
    CODEC_FINT_ROUTE_ADC0          = 0,
    CODEC_FINT_ROUTE_ADC1          = 1,
    CODEC_FINT_ROUTE_ADC2          = 2,
    CODEC_FINT_ROUTE_ASRCI0        = 3,
    CODEC_FINT_ROUTE_EQ            = 4,
    CODEC_FINT_ROUTE_FDSP0         = 5,
    CODEC_FINT_ROUTE_FDSP1         = 6,
    CODEC_FINT_ROUTE_FDSP2         = 7,
    CODEC_FINT_ROUTE_FDSP3         = 8,
    CODEC_FINT_ROUTE_FDSP4         = 9,
    CODEC_FINT_ROUTE_FDSP5         = 10,
    CODEC_FINT_ROUTE_FDSP6         = 11,
    CODEC_FINT_ROUTE_FDSP7         = 12,
    CODEC_FINT_ROUTE_FDSP8         = 13,
    CODEC_FINT_ROUTE_FDSP9         = 14,
    CODEC_FINT_ROUTE_FDSP10        = 15,
    CODEC_FINT_ROUTE_FDSP11        = 16,
    CODEC_FINT_ROUTE_FDSP12        = 17,
    CODEC_FINT_ROUTE_FDSP13        = 18,
    CODEC_FINT_ROUTE_FDSP14        = 19,
    CODEC_FINT_ROUTE_FDSP15        = 20,
    CODEC_FINT_ROUTE_SAI0_0_LEFT   = 21,
    CODEC_FINT_ROUTE_SAI0_1_RIGHT  = 22,
    /** attention b92 is not support TDM mode
    CODEC_FINT_ROUTE_SAI0_2        = 23,
    CODEC_FINT_ROUTE_SAI0_3        = 24,
    CODEC_FINT_ROUTE_SAI0_4        = 25,
    CODEC_FINT_ROUTE_SAI0_5        = 26,
    CODEC_FINT_ROUTE_SAI0_6        = 27,
    CODEC_FINT_ROUTE_SAI0_7        = 28,
    CODEC_FINT_ROUTE_SAI0_8        = 29,
    CODEC_FINT_ROUTE_SAI0_9        = 30,
    CODEC_FINT_ROUTE_SAI0_10       = 31,
    CODEC_FINT_ROUTE_SAI0_11       = 32,
    CODEC_FINT_ROUTE_SAI0_12       = 33,
    CODEC_FINT_ROUTE_SAI0_13       = 34,
    CODEC_FINT_ROUTE_SAI0_14       = 35,
    CODEC_FINT_ROUTE_SAI0_15       = 36,
    */
} codec_0581_fint_route_chnl_e;

/**
 * @brief  enum for HPAMP mode.(HPAMP -> headphone amplifier)
 */
typedef enum {
    CODEC_HPAMP_HEADPHONE = 0,
    CODEC_HPAMP_LINE_OUT  = 1
} codec_0581_hpamp_mode_e;

/**
 * @brief  enum for internal interrupt index (connected to IRQ1/2 pin)
 */
typedef enum {
    CODEC_IRQ_INDEX1           = 1,
    CODEC_IRQ_INDEX2           = 2,
} codec_0581_irq_index_e;

/**
 * @brief  enum for internal interrupt (connected to IRQ1/2 pin)
 */
typedef enum {
    CODEC_IRQ_DAC0_CLIP        = 0x00000001,
    CODEC_IRQ_ADC0_CLIP        = 0x00000002,
    CODEC_IRQ_ADC1_CLIP        = 0x00000004,
    CODEC_IRQ_ADC2_CLIP        = 0x00000008,
    CODEC_IRQ_PLL_LOCKED       = 0x00000010,
    CODEC_IRQ_PLL_UNLOCKED     = 0x00000020,
    CODEC_IRQ_AVDD_UVW         = 0x00000040,
    CODEC_IRQ_PRAMP            = 0x00000080,
    CODEC_IRQ_FDSP             = 0x00000100,
    CODEC_IRQ_EQ_CLR_DONE      = 0x00000200,
    CODEC_IRQ_ASRCI_LOCKED     = 0x00000400,
    CODEC_IRQ_ASRCI_UNLOCKED   = 0x00000800,
    CODEC_IRQ_ASRCO_LOCKED     = 0x00001000,
    CODEC_IRQ_ASRCO_UNLOCKED   = 0x00002000,
    CODEC_IRQ_SYNC_LOCKED      = 0x00004000,
    CODEC_IRQ_SYNC_UNLOCKED    = 0x00008000,
    CODEC_IRQ_SPT0_UNLOCKED    = 0x00010000,
    CODEC_IRQ_POWERUP_COMPLETE = 0x00040000,
} codec_0581_irq_src_e;

/**
 * @brief  enum for PGA channel.(PGA -> programmable gain amplifier)
 */
typedef enum{
    CODEC_PGA_CHNL_0             = 0,
    CODEC_PGA_CHNL_1             = 1,
    CODEC_PGA_CHNL_2             = 2,
} codec_0581_pga_chnl_e;

/**
 * @brief  enum for codec_0581 power mode
 */
typedef enum {
    CODEC_PWR_MODE_HIBERNATE      = 0,
    CODEC_PWR_MODE_ACTIVE         = 1,
} codec_0581_pwr_mode_e;

/**
 * @brief  enum for SAP(serial audio port/I2S) control's SAI mode (SPT_SAI_MODE, SPT -> serial port, SAI -> serial audio interface)
 */
typedef enum {
    CODEC_SAP_CTRL_SAI_MODE_STEREO       = 0,            /* sets the serial port in STEREO mode, assume 32-bit words per channel */
    /** attention b92 is not support TDM mode
    CODEC_SAP_CTRL_SAI_MODE_TDM          = 1,
    */
} codec_0581_sap_sai_mode_e;

/**
 * @brief  enum for SAP slot
 */
typedef enum {
    CODEC_SAP_SLOT0_LEFT                 = 0,
    CODEC_SAP_SLOT1_RIGHT                = 1,
    /** attention b92 is not support TDM mode
    CODEC_SAP_SLOT2                      = 2,
    CODEC_SAP_SLOT3                      = 3,
    CODEC_SAP_SLOT4                      = 4,
    CODEC_SAP_SLOT5                      = 5,
    CODEC_SAP_SLOT6                      = 6,
    CODEC_SAP_SLOT7                      = 7,
    CODEC_SAP_SLOT8                      = 8,
    CODEC_SAP_SLOT9                      = 9,
    CODEC_SAP_SLOT10                     = 10,
    CODEC_SAP_SLOT11                     = 11,
    CODEC_SAP_SLOT12                     = 12,
    CODEC_SAP_SLOT13                     = 13,
    CODEC_SAP_SLOT14                     = 14,
    CODEC_SAP_SLOT15                     = 15
    */
} codec_0581_sap_slot_e;

/**
 * @brief  enum for SAP control's data format (SPT_DATA_FORMAT)
 */
typedef enum {
    CODEC_SAP_CTRL_DATA_FORMAT_LJ        = 1,            /* left justified,delay by 0 */
    CODEC_SAP_CTRL_DATA_FORMAT_I2S       = 0,            /* typical I2S mode, delay by 1 */
    CODEC_SAP_CTRL_DATA_FORMAT_DLY8      = 2,            /* delay by 8 */
    CODEC_SAP_CTRL_DATA_FORMAT_DLY12     = 3,            /* delay by 12 */
    CODEC_SAP_CTRL_DATA_FORMAT_DLY16     = 4             /* delay by 16 */
} codec_0581_sap_data_format_e;

/**
 * @brief  enum for SAP control's tri-state enable (SPT_TRI_STATE)
 */
typedef enum {
    CODEC_SAP_CTRL_TRI_STATE_ENABLE      = 1,            /* tri-state enabled  */
    CODEC_SAP_CTRL_TRI_STATE_DISABLE     = 0             /* tri-state disabled */
} codec_0581_sap_tristate_e;

/**
 * @brief  enum for sap mclk output rates
 */
typedef enum {
    CODEC_SAP_MCLK_24576K = 0,
    CODEC_SAP_MCLK_12288K = 1,
    CODEC_SAP_MCLK_6144K  = 2,
    CODEC_SAP_MCLK_3072K  = 3,
    CODEC_SAP_MCLK_1536K  = 4,
    CODEC_SAP_MCLK_768K   = 5,
    CODEC_SAP_MCLK_384K   = 6,
    CODEC_SAP_MCLK_192K   = 7,
} codec_0581_sap_mclk_rate_e;

/**
 * @brief  enum for SAP control's bclk src (SPT_BCLK_SRC)
 */
typedef enum {
    CODEC_SAP_CTRL_BCLK_SRC_SLAVE        = 0,
    CODEC_SAP_CTRL_BCLK_SRC_3072K        = 1,
    CODEC_SAP_CTRL_BCLK_SRC_6144K        = 2,
    CODEC_SAP_CTRL_BCLK_SRC_12288K       = 3,
    CODEC_SAP_CTRL_BCLK_SRC_24576K       = 4
} codec_0581_sap_bclk_src_e;

/**
 * @brief  enum for SAP control's bclk polarity (SPT_BCLK_POL)
 */
typedef enum {
    CODEC_SAP_CTRL_BCLK_POL_NEG          = 0,            /* capture on rising edge  */
    CODEC_SAP_CTRL_BCLK_POL_POS          = 1             /* capture on falling edge */
} codec_0581_sap_bclk_polarity_e;

/**
 * @brief  enum for SAP control's lrclk src (SPT_LRCLK_SRC)
 */
typedef enum {
    CODEC_SAP_CTRL_LRCLK_SRC_SLAVE       = 0,
    CODEC_SAP_CTRL_LRCLK_SRC_48K         = 1,
    CODEC_SAP_CTRL_LRCLK_SRC_96K         = 2,
    CODEC_SAP_CTRL_LRCLK_SRC_192K        = 3,
    CODEC_SAP_CTRL_LRCLK_SRC_12K         = 4,
    CODEC_SAP_CTRL_LRCLK_SRC_24K         = 5,
    CODEC_SAP_CTRL_LRCLK_SRC_384K        = 6,
    CODEC_SAP_CTRL_LRCLK_SRC_768K        = 7,
    CODEC_SAP_CTRL_LRCLK_SRC_8K          = 8,
    CODEC_SAP_CTRL_LRCLK_SRC_16K         = 9
} codec_0581_sap_lrclk_src_e;

/**
 * @brief  enum for SAP control's lrclk polarity (SPT_LRCLK_POL)
 */
typedef enum {
    CODEC_SAP_CTRL_LRCLK_POL_NORM        = 0,            /* normal polarity  */
    CODEC_SAP_CTRL_LRCLK_POL_INVT        = 1             /* inverted polarity */
} codec_0581_sap_lrclk_polarity_e;

/**
 * @brief  enum for SAP output data source
 */
typedef enum {
    CODEC_SAP_OUT_ROUTE_FROM_ADC0,
    CODEC_SAP_OUT_ROUTE_FROM_ADC1,
    CODEC_SAP_OUT_ROUTE_FROM_ADC2,
    CODEC_SAP_OUT_ROUTE_FROM_ASRCO0,
    CODEC_SAP_OUT_ROUTE_FROM_ASRCO1,
    CODEC_SAP_OUT_ROUTE_FROM_ASRCO2,
    CODEC_SAP_OUT_ROUTE_FROM_EQ,
    CODEC_SAP_OUT_ROUTE_FROM_FDEC0,
    CODEC_SAP_OUT_ROUTE_FROM_FDEC1,
    CODEC_SAP_OUT_ROUTE_FROM_FDEC2,
    CODEC_SAP_OUT_ROUTE_FROM_FDEC3,
    CODEC_SAP_OUT_ROUTE_FROM_FDSP0,
    CODEC_SAP_OUT_ROUTE_FROM_FDSP1,
    CODEC_SAP_OUT_ROUTE_FROM_FDSP2,
    CODEC_SAP_OUT_ROUTE_FROM_FDSP3,
    CODEC_SAP_OUT_ROUTE_FROM_FDSP4,
    CODEC_SAP_OUT_ROUTE_FROM_FDSP5,
    CODEC_SAP_OUT_ROUTE_FROM_FDSP6,
    CODEC_SAP_OUT_ROUTE_FROM_FDSP7,
    CODEC_SAP_OUT_ROUTE_FROM_FDSP8,
    CODEC_SAP_OUT_ROUTE_FROM_FDSP9,
    CODEC_SAP_OUT_ROUTE_FROM_FDSP10,
    CODEC_SAP_OUT_ROUTE_FROM_FDSP11,
    CODEC_SAP_OUT_ROUTE_FROM_FDSP12,
    CODEC_SAP_OUT_ROUTE_FROM_FDSP13,
    CODEC_SAP_OUT_ROUTE_FROM_FDSP14,
    CODEC_SAP_OUT_ROUTE_FROM_FDSP15,
} codec_0581_sap_output_route_from_e;

/**
 * @brief  struct for serial audio port configuration
 */
typedef struct {
    codec_0581_sap_sai_mode_e       sai_mode;
    codec_0581_sap_data_format_e    data_format;
    codec_0581_sap_tristate_e       tri_state;
    codec_0581_sap_bclk_src_e       bclk_src;
    codec_0581_sap_bclk_polarity_e  bclk_pol;
    codec_0581_sap_lrclk_src_e      lrclk_src;
    codec_0581_sap_lrclk_polarity_e lrclk_pol;
} codec_0581_sap_config_t;

/**
 * @brief  struct for codec_0581 input adc config
 */
typedef struct {
    codec_0581_adc_chnl_e                adc_chnl;           /* ADC channel codec_0581_adc_chnl_e */
    codec_0581_adc_sample_rate_e         adc_rate;           /* ADC sample rate codec_0581_adc_sample_rate_e */
} codec_0581_input_adc_config_t;

/**
 * @brief  struct for codec_0581 input asrco config
 */
typedef struct {
    codec_0581_asrc_fs_e                 asrco_in_fs;        /* ASRCO input frequency codec_0581_asrc_fs_e */
    codec_0581_asrco_chnl_e              asrco_chnl;         /* ASRCO channel to be configured codec_0581_asrco_chnl_e */
    codec_0581_asrco_route_chnl_e        asrco_route_from;   /* ASRCO input source channel codec_0581_asrco_route_chnl_e */
} codec_0581_input_asrco_config_t;

/**
 * @brief  struct for codec_0581 input fdec config
 */
typedef struct {
    codec_0581_fdec_chnl_e               fdec_chnl;          /* fdec channel to be configured codec_0581_fdec_chnl_e */
    codec_0581_fdec_in_fs_e              fdec_in_fs;         /* fdec input frequency codec_0581_fdec_in_fs_e */
    codec_0581_fdec_out_fs_e             fdec_out_fs;        /* fdec output frequency codec_0581_fdec_out_fs_e */
    codec_0581_fdec_route_chnl_e         fdec_route_from;    /* fdec input source channel codec_0581_fdec_route_chnl_e */
}codec_0581_input_fdec_config_t;

/**
 * @brief  struct for codec_0581 input sap config
 */
typedef struct {
    codec_0581_sap_slot_e                slot_id;            /* the slot id to be configured codec_0581_sap_slot_e, if the i2s channel is mono, it must be CODEC_SAP_SLOT1_RIGHT. */
    codec_0581_sap_output_route_from_e   sap_route_from;     /* where the output data from codec_0581_sap_output_route_from_e */
} codec_0581_input_sap_config_t;

/**
 * @brief  struct for codec_0581 input
 */
typedef struct {
    codec_0581_input_adc_config_t        *adc_config;        /* adc config  codec_0581_input_adc_config_t */
    codec_0581_input_asrco_config_t      *asrco_config;      /* asrco config codec_0581_input_asrco_config_t */
    codec_0581_input_fdec_config_t       *fdec_config;       /* fdec_config codec_0581_input_fdec_config_t */
    codec_0581_input_sap_config_t        *sap_config;        /* sap_config codec_0581_input_sap_config_t */
} codec_0581_input_t;

/**
 * @brief  struct for codec_0581 output asrci config
 */
typedef struct {
    codec_0581_asrc_fs_e                 asrci_out_fs;       /* ASRCI output frequency codec_0581_asrc_fs_e */
} codec_0581_output_asrci_config_t;

/**
 * @brief  struct for codec_0581 output dac config
 */
typedef struct {
    codec_0581_dac_sample_rate_e         dac_rate;           /* DAC sample rate codec_0581_dac_sample_rate_e */
    codec_0581_dac_input_route_e         dac_input;          /* DAC input source channel codec_0581_dac_input_route_e */
} codec_0581_output_dac_config_t;

/**
 * @brief  struct for codec_0581 output fint config
 */
typedef struct {
    codec_0581_fint_chnl_e               fint_chnl;          /* fint channel to be configured codec_0581_fint_chnl_e */
    codec_0581_fint_in_fs_e              fint_in_fs;         /* fint input frequency codec_0581_fint_in_fs_e */
    codec_0581_fint_out_fs_e             fint_out_fs;        /* fint output frequency codec_0581_fint_out_fs_e */
    codec_0581_fint_route_chnl_e         fint_route_from;    /* fint input source channel codec_0581_fint_route_chnl_e */
}codec_0581_output_fint_config_t;

/**
 * @brief  struct for codec_0581 output
 */
typedef struct {
    codec_0581_output_asrci_config_t     *asrci_config;      /* asrci_config codec_0581_output_asrci_config_t */
    codec_0581_output_fint_config_t      *fint_config;       /* fint_config codec_0581_output_fint_config_t */
    codec_0581_output_dac_config_t       *dac_config;        /* dac_config codec_0581_output_dac_config_t */
} codec_0581_output_t;

/**
 * @brief  struct for codec_0581 eq config
 */
typedef struct {
    unsigned int                        *program_addr;      /* eq program data addr */
    unsigned int                        program_len;        /* eq program data length */
    unsigned int                        *param_addr;        /* eq param data addr */
    unsigned int                        param_len;          /* eq param data length */
    codec_0581_eq_bank_e                 bank_id;           /* eq bank id to be selected codec_0581_eq_bank_e */
    codec_0581_eq_route_e                eq_route_from;     /* eq route from codec_0581_eq_route_e */
}codec_0581_eq_config_t;

/**
 * @brief  struct for codec_0581 fdsp config
 */
typedef struct {
    unsigned int                        *program_addr;      /* fdsp program data addr */
    unsigned int                        program_len;        /* fdsp program data length */
    unsigned int                        *param_addr;        /* fdsp param data addr param_len = program_len */
    codec_0581_fdsp_param_bank_e         bank_id;           /* fdsp bank id to be selected codec_0581_fdsp_param_bank_e */
}codec_0581_fdsp_config_t;

/**
 * @brief  struct for codec_0581 i2s init
 */
typedef struct {
    unsigned short                      *sample_rate;       /* sample rate */
    audio_i2s_wl_mode_e                 data_width;         /* i2s data width */
}codec_0581_i2s_init_t;

/**
 * @brief  struct for codec_0581 i2s input
 */
typedef struct {
    void                                *input_data_buf;    /* input data buffer address */
    unsigned int                        input_buf_size;     /* input data buffer size */
    i2s_channel_select_e                i2s_ch_sel;         /* i2s channel select */
    dma_chn_e                           rx_dma_num;         /* rx dma channel select */
}codec_0581_i2s_input_t;

/**
 * @brief  struct for codec_0581 i2s output
 */
typedef struct {
    void                                *output_data_buf;   /* output data buffer address */
    unsigned int                        output_buf_size;    /* output data buffer size */
    dma_chn_e                           tx_dma_num;         /* tx dma channel select */
}codec_0581_i2s_output_t;


/**
 * @brief       This function servers to read codec_0581 register data.
 * @param[in]   reg_addr      external codec register address.
 * @param[out]  reg_data      read data from register.
 * @return      none.
 */
void codec_0581_reg_read(unsigned int reg_addr, unsigned int *reg_data);

/**
 * @brief       This function servers to write codec_0581 register data.
 * @param[in]   reg_addr      external codec register address.
 * @param[in]   reg_data      write data to register.
 * @return      none.
 */
void codec_0581_reg_write(unsigned int reg_addr, unsigned int reg_data);

/**
 * @brief       This function servers to init codec_0581 i2s.
 * @param[in]   i2s_init                codec_0581 i2s init, codec_0581_i2s_init_t.
 * @param[in]   i2s_input               codec_0581 i2s input config, codec_0581_i2s_input_t.
 * @param[in]   i2s_output              codec_0581 i2s output config, codec_0581_i2s_output_t.
 * @return      none
 */
void codec_0581_i2s_init(codec_0581_i2s_init_t *i2s_init, codec_0581_i2s_input_t *i2s_input, codec_0581_i2s_output_t *i2s_output);

/**
 * @brief       This function servers to init codec_0581. include spi port, pmu mode and pll etc.
 * @return      CODEC_0581_OK for success, codec_0581_error_e.
 */
codec_0581_error_e codec_0581_init(void);

/**
 * @brief      This function serves to get register length .
 * @param[in]  first_byte_addr    external codec register addr.
 * @param[out] reg_width          get register width.
 * @return                        0 - get register width success  -1 - err parameter.
 */
char codec_0581_get_addr_length(unsigned int first_byte_addr, unsigned char *reg_width);

/**
 * @brief       This function servers to power on/off codec_0581.
 * @param[in]   power_pin        the power pin selected.
 * @param[in]   enable           CODEC_BOOL_TRUE  - pin output enable and power on
 *                               CODEC_BOOL_FALSE - power off
 * @return      none
 * @attention   active Low Power-Down. All digital and analog circuits are powered down. There is an internal pull-down resistor on
 *              this pin; therefore, the codec is held in power-down mode if its input signal is floating while power is applied to
 *              the supply pins.
 */
void codec_0581_power_on(gpio_pin_e power_pin, codec_0581_bool_e enable);

/**
 * @brief       This function servers to get codec_0581 vendor id, device id and device revision
 * @param[out]  vid        pointer to vendor id value
 * @param[out]  did        pointer to device id value
 * @param[out]  drev       pointer to device revision value
 * @return      none
 */
void codec_0581_get_id(unsigned char *vid, unsigned short *did, unsigned char *drev);

/**
 * @brief       This function servers to reset this codec_0581 in soft way
 * @param[in]   full_reset  CODEC_BOOL_TRUE  for full soft reset, software reset of entire IC
 *                          CODEC_BOOL_FALSE for soft reset,      software reset not including register settings
 * @return      none
 */
void codec_0581_reset(codec_0581_bool_e full_reset);

/**
 * @brief       This function servers to power on/off ADC.
 * @param[in]   adc_channel      ADC channel codec_0581_adc_chnl_e
 * @param[in]   enable           CODEC_BOOL_TRUE - power on, CODEC_BOOL_FALSE - power off
 * @return      none
 */
void codec_0581_adc_enable_power_on(codec_0581_adc_chnl_e adc_channel, codec_0581_bool_e enable);

/**
 * @brief       This function servers to get ADC power on/off status.
 * @param[in]   adc_channel      ADC channel codec_0581_adc_chnl_e
 * @param[out]  power_state      to store the power state which is read back
 * @return      none
 */
void codec_0581_adc_get_power_state(codec_0581_adc_chnl_e adc_channel, unsigned int *power_state);

/**
 * @brief       This function servers to set ADC sample rate
 * @param[in]   adc_channel      ADC channel codec_0581_adc_chnl_e
 * @param[in]   rate             ADC sample rate, codec_0581_adc_sample_rate_e
 * @return      none
 */
void codec_0581_adc_set_sample_rate(codec_0581_adc_chnl_e adc_channel, codec_0581_adc_sample_rate_e rate);

/**
 * @brief       This function servers to get ADC sample rate
 * @param[in]   adc_channel      ADC channel codec_0581_adc_chnl_e
 * @param[out]  rate             to store ADC sample rate which is read back, codec_0581_adc_sample_rate_e
 * @return      none
 */
void codec_0581_adc_get_sample_rate(codec_0581_adc_chnl_e adc_channel, codec_0581_adc_sample_rate_e *rate);

/**
 * @brief       This function servers to set ADC filter
 * @param[in]   adc_channel           ADC channel codec_0581_adc_chnl_e
 * @param[in]   filter                high pass filter cut off frequence, codec_0581_adc_hpf_e
 * @param[in]   higher_order_enable   CODEC_BOOL_TRUE - higher order, CODEC_BOOL_FALSE - lower order
 * @param[in]   high_freq_comp_enable CODEC_BOOL_TRUE - high frequency response compensated, CODEC_BOOL_FALSE - high frequency response not compensated
 * @return      none
 */
void codec_0581_adc_set_filter(codec_0581_adc_chnl_e adc_channel, codec_0581_adc_hpf_e filter, codec_0581_bool_e higher_order_enable, codec_0581_bool_e high_freq_comp_enable);

/**
 * @brief       This function servers to enable or disable ADC chop
 * @param[in]   enable_flag      Enable flag: CODEC_BOOL_TRUE - enable, CODEC_BOOL_FALSE - disable
 * @return      none
 */
void codec_0581_adc_set_chop_enable(codec_0581_bool_e enable_flag);

/**
 * @brief       This function servers to set ADC voice wakeup mode.
 * @param[in]   voicewakeup_enable   CODEC_BOOL_TRUE: enable voice wakeup; CODEC_BOOL_FALSE: disable voice wakeup
 *                                   the ADCx can work in voice wake up mode to further reduce power consumption.
 *                                   during the voice wake up mode, the ADCx sample rate is 8kHz or 16kHz and
 *                                   the ADCx data width is 16 bits (16 MSBs of the ADC output).
 * @return      none
 */
void codec_0581_adc_set_voice_wakeup_mode(codec_0581_bool_e voicewakeup_enable);

/**
 * @brief       This function servers to set adc input mode.
 * @param[in]   adc_channel      ADC channel codec_0581_adc_chnl_e
 * @param[in]   single_enable    ADC input mode, CODEC_BOOL_TRUE - single input, CODEC_BOOL_FALSE - diff input
 * @return      none
 */
void codec_0581_adc_set_input_mode(codec_0581_adc_chnl_e adc_channel, codec_0581_bool_e single_enable);

/**
 * @brief       This function servers to enable/disable ADC mute.
 * @param[in]   adc_channel      ADC channel codec_0581_adc_chnl_e
 * @param[in]   enable           CODEC_BOOL_TRUE - mute, CODEC_BOOL_FALSE - unmute
 * @return      none
 */
void codec_0581_adc_enable_mute(codec_0581_adc_chnl_e adc_channel, codec_0581_bool_e enable);

/**
 * @brief       This function servers to set ADC volume.
 * @param[in]   adc_channel      ADC channel codec_0581_adc_chnl_e
 * @param[in]   vol_db           volume, unit: dB
 * @return      none
 */
void codec_0581_adc_set_volume_db(codec_0581_adc_chnl_e adc_channel, float vol_db);

/**
 * @brief       This function servers to enable dither and set dither offset size.
 * @param[in]   dither           dither level,  codec_0581_adc_dither_level_e
 * @return      none
 */
void codec_0581_adc_set_dither_level(codec_0581_adc_dither_level_e dither);

/**
 * @brief       This function servers to enable or disable ASRC channel.
 * @param[in]   chnl_no          ASRCI/O channel to be configured, codec_0581_asrc_chnl_e
 * @param[in]   enable_flag      enable flag: CODEC_BOOL_TRUE - enable, CODEC_BOOL_FALSE - disable
 * @return      none
 */
void codec_0581_asrc_enable_chnl_power_on(codec_0581_asrc_chnl_e chnl_no, codec_0581_bool_e enable_flag);

/**
 * @brief       This function servers to enable or disable multiple ASRC channels.
 * @param[in]   chnl_bits_set    if the bit is set, the channel will be affected; if the bit is clear, the channel will not be affected
 *                               bit[0]:     ASRCI
 *                               bit[1:3]:   reserved
 *                               bit[4:6]:   ASRCO channel 0-2
 *                               bit[7]:     reserved
 * @param[in]   enable_flag      enable or disable flag:
 *                               CODEC_BOOL_TRUE - enable the channels whose bit is set, CODEC_BOOL_FALSE - disable the channels whose bit is clear
 * @return      none
 */
void codec_0581_asrc_enable_multi_chnls_power_on(unsigned char chnl_bits_set, codec_0581_bool_e enable_flag);

/**
 * @brief       This function servers to select ASRCI input channel.
 * @param[in]   route_from       ASRCI input source channel codec_0581_sap_slot_e
 * @return      none
 */
void codec_0581_asrc_select_asrci_route(codec_0581_sap_slot_e route_from);

/**
 * @brief       This function servers to select ASRCO input channel.
 * @param[in]   asrco_chnl       ASRCO channel to be configured codec_0581_asrco_chnl_e
 * @param[in]   route_from       ASRCO input source channel
 * @return      none
 */
void codec_0581_asrc_select_asrco_route(codec_0581_asrco_chnl_e asrco_chnl, codec_0581_asrco_route_chnl_e route_from);

/**
 * @brief       This function servers to select ASRCI output frequency.
 * @param[in]   out_fs           ASRCI output frequency, codec_0581_asrc_fs_e
 * @return      none
 */
void codec_0581_asrc_select_asrci_fs(codec_0581_asrc_fs_e out_fs);

/**
 * @brief       This function servers to select ASRCO input frequency.
 * @param[in]   in_fs            ASRCO input frequency, codec_0581_asrc_fs_e
 * @return      none
 */
void codec_0581_asrc_select_asrco_fs(codec_0581_asrc_fs_e in_fs);

/**
 * @brief       This function servers to power on/off PLL.
 * @param[in]   enable           CODEC_BOOL_TRUE - power on, CODEC_BOOL_FALSE - power off
 * @return      none
 */
void codec_0581_clk_enable_pll_power_on(codec_0581_bool_e enable);

/**
 * @brief       This function servers to configure PLL, PLL output = (input_clk/prescaler)*multiplier for integer mode, = (input_clk/prescaler)*(multiplier + numerator/denominator) for fractional mode
 * @param[in]   pll_src      codec_0581_clk_pll_source_e
 * @param[in]   type         codec_0581_clk_pll_type_e
 * @param[in]   sync_src     codec_0581_clk_sync_source_e
 * @param[in]   prescaler    pll divider
 * @param[in]   multiplier   pll multiplier
 * @param[in]   numerator    pll numerator
 * @param[in]   denominator  pll denominator
 * @return      none
 */
void codec_0581_clk_config_pll(codec_0581_clk_pll_source_e pll_src, codec_0581_clk_pll_type_e type,
                              codec_0581_clk_sync_source_e sync_src, unsigned char prescaler, unsigned short multiplier, unsigned short numerator, unsigned short denominator);

/**
 * @brief       This function servers to update all PLL configuration settings.
 * @return      none
 */
void codec_0581_clk_update_pll(void);

/**
 * @brief       This function servers to get pll locked status.
 * @param[out]  locked           0 - pll unlocked
 *                               1 - pll locked
 * @return      none
 */
void codec_0581_clk_get_pll_locked_status(unsigned char *locked);

/**
 * @brief       This function servers to get input and output ASRCI lock status.
 * @param[out]  asrci_locked     0 - ASRC currently unlocked  1 - ASRC currently locked
 * @param[out]  asrco_locked     0 - ASRC currently unlocked  1 - ASRC currently locked
 * @return      none
 */
void codec_0581_clk_get_asrc_locked_status(unsigned char *asrci_locked, unsigned char *asrco_locked);

/**
 * @brief       This function servers to enable or disable pll bypass.
 * @param[in]   bypass_pll   enable flag: CODEC_BOOL_TRUE - bypass pll, CODEC_BOOL_FALSE - use pll
 * @return      none
 */
void codec_0581_clk_enable_pll_bypass(codec_0581_bool_e bypass_pll);

/**
 * @brief       This function servers to select SAP MCLK output rate
 * @param[in]   rate             codec_0581_sap_mclk_rate_e
 * @return      none
 */
void codec_0581_clk_set_sap_mclk_out_rate(codec_0581_sap_mclk_rate_e rate);

/**
 * @brief       This function servers to power on/off DAC.
 * @param[in]   enable           CODEC_BOOL_TRUE - power on, CODEC_BOOL_FALSE - power off
 * @return      none
 */
void codec_0581_dac_enable_power_on(codec_0581_bool_e enable);

/**
 * @brief       This function servers to set DAC normal/enhance mode
 * @param[in]   enable           CODEC_BOOL_TRUE - dac enhance mode
 *                               CODEC_BOOL_FALSE - dac normal mode
 * @return      none
 */
void codec_0581_dac_enhanced_mode_enable(codec_0581_bool_e enable);

/**
 * @brief       This function servers to set DAC input sample rate.
 * @param[in]   rate             codec_0581_dac_sample_rate_e
 * @return      none
 */
void codec_0581_dac_set_sample_rate(codec_0581_dac_sample_rate_e rate);

/**
 * @brief       This function servers to enable/disable DAC high frequency response compensated.
 * @param[in]   hpf_enable               CODEC_BOOL_TRUE - ADC high pass filter On, CODEC_BOOL_FALSE - ADC high pass filter off
 * @param[in]   high_freq_comp_enable    CODEC_BOOL_TRUE - high frequency response compensated, CODEC_BOOL_FALSE - high frequency response not compensated
 * @param[in]   interp_enable            CODEC_BOOL_TRUE -  more interpolation filtering : higher delay, CODEC_BOOL_FALSE - Less interpolation filtering : lower delay
 * @return      none
 */
void codec_0581_dac_set_filter(codec_0581_bool_e hpf_enable, codec_0581_bool_e high_freq_comp_enable, codec_0581_bool_e interp_enable);

/**
 * @brief       This function servers to enable/disable low power mode.
 * @param[in]   enable           CODEC_BOOL_TRUE -  enable low power mode, output sample rate 3.072MHz
 *                               CODEC_BOOL_FALSE - disable low power mode, output sample rate 6.144MHz
 * @return      none
 */
void codec_0581_dac_enable_low_power_mode(codec_0581_bool_e enable);

/**
 * @brief       This function servers to enable/disable DAC mute.
 * @param[in]   enable           CODEC_BOOL_TRUE - mute, CODEC_BOOL_FALSE - unmute
 * @return      none
 */
void codec_0581_dac_enable_mute(codec_0581_bool_e enable);

/**
 * @brief       This function servers to set DAC volume.
 * @param[in]   vol_db           volume, unit: dB
 * @return      none
 */
void codec_0581_dac_set_volume_db(float vol_db);

/**
 * @brief       This function servers to set DAC input route.
 * @param[in]   route            codec_0581_dac_input_route_e
 * @return      none
 */
void codec_0581_dac_select_input_route(codec_0581_dac_input_route_e route);

/**
 * @brief       This function servers to set EQ input data source route
 * @param[in]   route            codec_0581_eq_route_e
 * @return      none
 */
void codec_0581_eq_select_route(codec_0581_eq_route_e route);

/**
 * @brief       This function servers to enable/disable EQ
 * @param[in]   enable           CODEC_BOOL_TRUE to run EQ, CODEC_BOOL_FALSE to stop EQ
 * @return      none
 */
void codec_0581_eq_enable_run(codec_0581_bool_e enable);

/**
 * @brief       This function servers to clear param ram
 * @return      none
 */
void codec_0581_eq_clear_param_ram(void);

/**
 * @brief       This function servers to get clear status when clearing param ram
 * @param[out]  clear_done       1 to mean clearing is done
 * @return      none
 */
void codec_0581_eq_get_param_ram_clear_status(unsigned char *clear_done);

/**
 * @brief       This function servers to select param ram
 * @param[in]   bank_id          param bank id codec_0581_eq_bank_e
 * @return      none
 */
void codec_0581_eq_sel_param_ram(codec_0581_eq_bank_e bank_id);

/**
 * @brief       This function servers to enable or disable specific decimator channel.
 * @param[in]   chnl_no          channel to be enabled/disabled codec_0581_fdec_chnl_e
 * @param[in]   enable_flag      enable or disable flag: CODEC_BOOL_TRUE - enable, CODEC_BOOL_FALSE - disable
 * @return      none
 */
void codec_0581_fdec_enable_chnl_power_on(codec_0581_fdec_chnl_e chnl_no, codec_0581_bool_e enable_flag);

/**
 * @brief       This function servers to enable or disable multiple decimator channels
 * @param[in]   chnl_bits_set    if the bit is set, the channel will be affected; if the bit is clear, the channel will not be affected
 *                               bit[0:3]:      fdec channel 0-3
 *                               bit[4:7]:      reserved
 * @param[in]   enable_flag      enable or disable flag:
 *                               CODEC_BOOL_TRUE - enable the channels whose bit is set, CODEC_BOOL_FALSE - disable the channels whose bit is clear
 * @return      none
 */
void codec_0581_fdec_enable_multi_chnls_power_on(unsigned char chnl_bits_set, codec_0581_bool_e enable_flag);

/**
 * @brief       This function servers to select sample frequency of a pair of decimator channels.
 * @param[in]   pair_chnls       channel pair to be set
 * @param[in]   in_fs            input sample rate
 * @param[in]   out_fs           output sample rate
 * @return      none
 */
void codec_0581_fdec_select_pair_chnls_fs(codec_0581_fdec_pair_chnl_e pair_chnls, codec_0581_fdec_in_fs_e in_fs, codec_0581_fdec_out_fs_e out_fs);

/**
 * @brief       This function servers to select the route source of a decimator channel.
 * @param[in]   chnl_no          decimator channel to be configured codec_0581_fdec_chnl_e
 * @param[in]   route_chnl       input/output channel for the decimator channel
 * @return      none
 */
void codec_0581_fdec_select_chnl_route(codec_0581_fdec_chnl_e chnl_no, codec_0581_fdec_route_chnl_e route_chnl);

/**
 * @brief       This function servers to power on/off fast DSP.
 * @param[in]   enable           enable: CODEC_BOOL_TRUE - power on, CODEC_BOOL_FALSE - power off
 * @return      none
 */
void codec_0581_fdsp_enable_power_on(codec_0581_bool_e enable);

/**
 * @brief       This function servers to run/stop fast DSP.
 * @param[in]   enable           enable: CODEC_BOOL_TRUE - run, CODEC_BOOL_FALSE - stop
 * @return      none
 */
void codec_0581_fdsp_enable_run(codec_0581_bool_e enable);

/**
 * @brief       This function servers to select parameter bank.
 * @param[in]   sel              parameter bank index, codec_0581_fdsp_param_bank_e
 * @return      none
 */
void codec_0581_fdsp_select_param_bank(codec_0581_fdsp_param_bank_e sel);

/**
 * @brief       This function servers to set parameter bank switch settings.
 * @param[in]   mode             ramp or instant, codec_0581_fdsp_param_switch_mode_e
 * @param[in]   rate             ramp or instant, codec_0581_fdsp_ramp_rate_e
 * @param[in]   zero             enable: CODEC_BOOL_TRUE - zero state during bank switch, CODEC_BOOL_FALSE - do not zero state during bank switch
 * @param[in]   stop_point       parameter bank switch ramp will stop at lambda/64 of full ramp
 * @return      none
 */
void codec_0581_fdsp_config_bank_switch_settings(codec_0581_fdsp_param_switch_mode_e mode, codec_0581_fdsp_ramp_rate_e rate, codec_0581_bool_e zero, unsigned char stop_point);

/**
 * @brief       This function servers to copy parameter bank.
 * @param[in]   mask             copy mask codec_0581_fdsp_copy_param_bank_e
 * @return                       0 - successful  -1 failed
 */
char codec_0581_fdsp_copy_param_bank(codec_0581_fdsp_copy_param_bank_e mask);

/**
 * @brief       This function servers to set rate source and divider.
 * @param[in]   source           rate source codec_0581_fdsp_rate_src_e
 * @param[in]   div              divider, 16 bits
 * @return      none
 */
void codec_0581_fdsp_set_rate(codec_0581_fdsp_rate_src_e source, unsigned short div);

/**
 * @brief       This function servers to set modulo N.
 * @param[in]   n                N, 6 bits
 * @return      none
 */
void codec_0581_fdsp_set_modulo_n(unsigned char n);

/**
 * @brief       This function servers to load active parameter bank.
 * @param[in]   inst_index       instruction index, 0 ~ 95
 * @param[in]   param            parameter memory 0 ~ 4
 * @return      none
 */
void codec_0581_fdsp_safeload(unsigned char inst_index, unsigned int param[5]);

/**
 * @brief       This function servers to enable or disable specific interpolator channel
 * @param[in]   chnl_no          channel to be enabled/disabled codec_0581_fint_chnl_e
 * @param[in]   enable_flag      enable or disable flag: CODEC_BOOL_TRUE - enable, CODEC_BOOL_FALSE - disable
 * @return      none
 */
void codec_0581_fint_enable_chnl_power_on(codec_0581_fint_chnl_e chnl_no, codec_0581_bool_e enable_flag);

/**
 * @brief       This function servers to enable or disable multiple interpolator channels
 * @param[in]   chnl_bits_set    if the bit is set, the channel will be affected; if the bit is clear, the channel will not be affected
 *                               bit[0:1]:    fint channel 0-1
 *                               bit[2:7]:    reserved
 * @param[in]   enable_flag      enable or disable flag:
 *                               CODEC_BOOL_TRUE - enable the channels whose bit is set, CODEC_BOOL_FALSE - disable the channels whose bit is clear
 * @return      none
 */
void codec_0581_fint_enable_multi_chnls_power_on(unsigned char chnl_bits_set, codec_0581_bool_e enable_flag);

/**
 * @brief       This function servers to select sample frequency of a pair of interpolator channels.
 * @param[in]   pair_chnls       channel pair to be set
 * @param[in]   in_fs            input sample rate
 * @param[in]   out_fs           output sample rate
 * @return      none
 */
void codec_0581_fint_select_pair_chnls_fs(codec_0581_fint_pair_chnl_e pair_chnls, codec_0581_fint_in_fs_e in_fs, codec_0581_fint_out_fs_e out_fs);

/**
 * @brief       This function servers to select the route source of a interpolator channel.
 * @param[in]   chnl_no          interpolator channel to be configured codec_0581_fint_chnl_e
 * @param[in]   route_chnl       input/output channel for the decimator channel
 * @return      none
 */
void codec_0581_fint_select_chnl_route(codec_0581_fint_chnl_e chnl_no, codec_0581_fint_route_chnl_e route_chnl);

/**
 * @brief       This function servers to set headphone output mode.
 * @param[in]   mode             headphone or line-out, codec_0581_hpamp_mode_e
 * @return      none
 */
void codec_0581_hpamp_set_output_mode(codec_0581_hpamp_mode_e mode);

/**
 * @brief       This function servers to clear interrupt request
 * @param[in]   index      codec_0581_irq_index_e
 * @return      none
 */
void codec_0581_int_clr_irq(codec_0581_irq_index_e index);

/**
 * @brief       This function servers to set interrupt request polarity
 * @param[in]   index      codec_0581_irq_index_e
 * @param[in]   pol        0: active low, 1: active high
 * @return      none
 */
void codec_0581_int_set_irq_pol(codec_0581_irq_index_e index, unsigned char pol);

/**
 * @brief       This function servers to enable interrupt request
 * @param[in]   index      codec_0581_irq_index_e
 * @param[in]   irq_src    codec_0581_irq_src_e
 * @param[in]   enable     CODEC_BOOL_TRUE to enable, CODEC_BOOL_FALSE to disable
 * @return      none
 */
void codec_0581_int_enable_irq(codec_0581_irq_index_e index, codec_0581_irq_src_e irq_src, codec_0581_bool_e enable);

/**
 * @brief       This function servers to get interrupt status
 * @param[in]   index      codec_0581_irq_index_e
 * @param[in]   irq_src    codec_0581_irq_src_e
 * @param[out]  status     0 - interrupt was not triggered, 1 - interrupt was triggered
 * @return      none
 */
void codec_0581_int_get_irq_status(codec_0581_irq_index_e index, codec_0581_irq_src_e irq_src, unsigned char *status);

/**
 * @brief       This function servers to power on/off PGA.
 * @param[in]   pga_channel      PGA channel codec_0581_pga_chnl_e
 * @param[in]   enable           CODEC_BOOL_TRUE - power on, CODEC_BOOL_FALSE - power off
 * @return      none
 */
void codec_0581_pga_enable_power_on(codec_0581_pga_chnl_e pga_channel, codec_0581_bool_e enable);

/**
 * @brief       This function servers to set PGA course gain.
 * @param[in]   pga_channel      PGA channel codec_0581_pga_chnl_e
 * @param[in]   gain             gain, output in dB is 0.75 * gain
 * @return      none
 */
void codec_0581_pga_set_gain(codec_0581_pga_chnl_e pga_channel, unsigned char gain);

/**
 * @brief       This function servers to enable/disable gain link.
 * @param[in]   enable           CODEC_BOOL_TRUE - all PGA channels use channel 0 gain value
 *                               CODEC_BOOL_FALSE - each PGA channel uses its respective gain value
 * @return      none
 */
void codec_0581_pga_enable_gain_link(codec_0581_bool_e enable);

/**
 * @brief       This function servers to set input resistance.
 * @param[in]   pga_channel           PGA channel index codec_0581_pga_chnl_e
 * @param[in]   low_resistance_enable CODEC_BOOL_TRUE - PGA channel has low input resistance
 *                                    CODEC_BOOL_FALSE - PGA channel has high input resistance
 * @return      none
 */
void codec_0581_pga_set_input_resistance(codec_0581_pga_chnl_e pga_channel, codec_0581_bool_e low_resistance_enable);

/**
 * @brief       This function servers to set PGA operation mode
 * @param[in]   pga_channel      PGA channel index codec_0581_pga_chnl_e
 * @param[in]   enhanced_enable  CODEC_BOOL_TRUE - PGA channel is in enhanced operation mode
 *                               CODEC_BOOL_FALSE - PGA channel is in normal  performance mode.
 * @return      none
 */
void codec_0581_pga_set_power_mode(codec_0581_pga_chnl_e pga_channel, codec_0581_bool_e enhanced_enable);

/**
 * @brief       This function servers to set codec_0581 power mode.
 * @param[in]   mode             CODEC_PWR_MODE_HIBERNATE - set chip into it's lowest power state
 *                               CODEC_PWR_MODE_ACTIVE    - allows block enabling of PLL, FDSP, and EQ etc
 * @return      none
 */
void codec_0581_pmu_set_power_mode(codec_0581_pwr_mode_e mode);

/**
 * @brief  This function servers enable/disable high power CM startup boost mode
 * @param  enable_flag           CODEC_BOOL_TRUE - CM pin fast charge disabled
 *                               CODEC_BOOL_FALSE - CM pin fast charge enabled setting to false can save some power
 * @return      none
 * @attention  recommended to set CM_STARTUP_OVER = 1 to keep lower power consumption
 */
void codec_0581_pmu_enable_cm_pin_fast_charge(codec_0581_bool_e enable_flag);

/**
 * @brief       This function servers to enable or disable master block power.
 * @param[in]   enable_flag      enable flag: CODEC_BOOL_TRUE  - all blocks that have their respective block enable set
 *                                            CODEC_BOOL_FALSE - all blocks are disabled
 * @return      none
 */
void codec_0581_pmu_enable_master_block(codec_0581_bool_e enable_flag);

/**
 * @brief       This function servers to configure all the control features of SAP
 * @param[in]   config_ptr       pointer to codec_0581_sap_config_t to be used.
 * @return      none
 * @attention   b92 is not support TDM mode
 */
void codec_0581_sap_config_all(codec_0581_sap_config_t *config_ptr);

/**
 * @brief       This function servers to set serial audio interface mode
 * @param[in]   sai_mode         sai mode codec_0581_sap_sai_mode_e.
 * @return      none
 * @attention   b92 is not support TDM mode
 */
void codec_0581_sap_set_sai_mode(codec_0581_sap_sai_mode_e sai_mode);

/**
 * @brief       This function servers to set SAP data format
 * @param[in]   data_format      data format codec_0581_sap_data_format_e.
 * @return      none
 */
void codec_0581_sap_set_data_format(codec_0581_sap_data_format_e data_format);

/**
 * @brief       This function servers to set serial port output tri-state enable or disable
 * @param[in]   tri_state        tri-state codec_0581_sap_tristate_e.
 * @return      none
 */
void codec_0581_sap_set_tri_state(codec_0581_sap_tristate_e tri_state);

/**
 * @brief       This function servers to set SAP bit clock source
 * @param[in]   bclk_src         bit clock source codec_0581_sap_bclk_src_e.
 * @return      none
 */
void codec_0581_sap_set_bclk_src(codec_0581_sap_bclk_src_e bclk_src);

/**
 * @brief       This function servers to set SAP bit clock polarity
 * @param[in]   bclk_pol         bit clock polarity codec_0581_sap_bclk_polarity_e.
 * @return      none
 */
void codec_0581_sap_set_bclk_pol(codec_0581_sap_bclk_polarity_e bclk_pol);

/**
 * @brief       This function servers to set SAP frame (LR) clock source
 * @param[in]   lrclk_src        bit clock source codec_0581_sap_lrclk_src_e.
 * @return      none
 */
void codec_0581_sap_set_lrclk_src(codec_0581_sap_lrclk_src_e lrclk_src);

/**
 * @brief       This function servers to set SAP frame (LR) clock polarity
 * @param[in]   lrclk_pol        frame clock polarity codec_0581_sap_lrclk_polarity_e.
 * @return      none
 */
void codec_0581_sap_set_lrclk_pol(codec_0581_sap_lrclk_polarity_e lrclk_pol);

/**
 * @brief       This function servers to select SAP SPT output slot data source
 * @param[in]   slot_id          the slot id to be configured codec_0581_sap_slot_e
 * @param[in]   route_from       where the output data from. codec_0581_sap_output_route_from_e
 * @return      none
 */
void codec_0581_sap_select_out_route(codec_0581_sap_slot_e slot_id, codec_0581_sap_output_route_from_e route_from);

/**
 * @brief       This function servers to turn on/off SAP input
 * @param[in]   enable_flag      CODEC_BOOL_TRUE - turn on input, CODEC_BOOL_FALSE - turn off input
 * @return      none
 */
void codec_0581_sap_enable_chnl_input(codec_0581_bool_e enable_flag);

/**
 * @brief       This function servers to turn on/off SAP output
 * @param[in]   enable_flag      CODEC_BOOL_TRUE - turn on output, CODEC_BOOL_FALSE - turn off output
 * @return      none
 */
void codec_0581_sap_enable_chnl_output(codec_0581_bool_e enable_flag);

/**
 * @brief This function serves to initialize input route
 * @param[in]  input_config - the relevant configuration codec_0581_input_t struct pointer
 * @return    none
 */
void codec_0581_input_init(codec_0581_input_t *input_config);

/**
 * @brief This function serves to initialize output route
 * @param[in]  output_config - the relevant configuration codec_0581_output_t struct pointer
 * @return    none
 */
void codec_0581_output_init(codec_0581_output_t *output_config);

/**
 * @brief This function serves to initialize eq route and param
 * @param[in]  eq_config - the relevant configuration codec_0581_eq_config_t struct pointer
 * @return    none
 */
void codec_0581_eq_init(codec_0581_eq_config_t *eq_config);

/**
 * @brief This function serves to initialize fdsp route and param
 * @param[in]  fdsp_config - the relevant configuration codec_0581_fdsp_config_t struct pointer
 * @return    none
 */
void codec_0581_fdsp_init(codec_0581_fdsp_config_t *fdsp_config);


#endif /* _CODEC_0581_H_ */

/************************** end of file *****************************/
