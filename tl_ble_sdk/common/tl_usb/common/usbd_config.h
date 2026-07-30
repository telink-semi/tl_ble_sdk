/********************************************************************************************************
 * @file    usbd_config.h
 *
 * @brief   This is the header file for Telink RISC-V MCU
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
#ifndef __USBD_CONFIG_H__
#define __USBD_CONFIG_H__

#ifndef USBD_EP0_MPS
    #define USBD_EP0_MPS 64
#endif

#ifndef USBD_SUPPORT_DRIVER_MAX
    #define USBD_SUPPORT_DRIVER_MAX 16
#endif

#ifndef USBD_SUPPORT_INTERFACE_MAX
    #define USBD_SUPPORT_INTERFACE_MAX 16
#endif

#ifndef USBD_SUPPORT_ENDPOINT_MAX
    #define USBD_SUPPORT_ENDPOINT_MAX 8 /* Excluding endpoints 0. */
#endif

#ifndef USBD_ENDPOINT_DIRECTION
    #define USBD_ENDPOINT_DIRECTION 2 /* 1: one direction only, 2: bidirectional endpoints. */
#endif

#endif
