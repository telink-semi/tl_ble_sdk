# tl\_ble\_sdk README

# SDK Introduction

tl\_ble\_sdk is a software development platform for Bluetooth® Low Energy (BLE) multi-connection IoT applications. Based on Telink's TL721x, TL321x, TL322x, TLSR921x, TLSR922x, TLSR952x series of SoCs, it helps developers quickly build high-performance, low-power BLE products.

The SDK provides a complete software framework, including the BLE protocol stack, hardware drivers, system services, and engineering samples, supporting the full development process from product prototyping to mass production.

tl\_ble\_sdk adopts a multi-role, multi-connection architecture to meet the development needs of diverse Bluetooth LE multi-connection applications; typcial usage scenarios include gaming, wireless human interface devices, smart homes, positioning and tracking, and electronic shelf label.

>- tl\_ble\_sdk by default supports up to 4 Central and 4 Peripheral connections, but does not support free role switching.

**Core Competencies**

| Category | Ability |
| --- | --- |
| Wireless connectivity | BLE protocol stack, multi-role, multi-connection |
| Drives | GPIO, UART, SPI, I²C, USB, Flash, Timer, etc |
| Development frameworks | Bare-metal frameworks, application development frameworks, multi-connection management |
| System services | Power management, storage management, clock management, system initialization |
| Safety | SMP, secure connection, AES-CCM |
| Tools | Debug logs, Flash storage management, Assert |
| OTA | Wireless firmware upgrade |
| Examples | Typical engineering samples |

**Typical Applications**

| Wireless technology | Application areas | Typical products |
| --- | --- | --- |
| Bluetooth® LE | Wireless computer peripherals | Wireless mice, keyboards, game controllers, etc |
| Bluetooth® LE | Wearable devices | Smart wristbands, smartwatches, health monitoring devices, etc |
| Bluetooth® LE | Smart audio | Bluetooth headsets, speakers, AI microphones, etc |
| Bluetooth® LE | Industrial IoT | Temperature and humidity sensors, wireless switches, smart instruments, electronic shelf labels, etc |
| Bluetooth® LE | Positioning services | Bluetooth beacon devices, digital car keys, smart door locks, etc |

**Support Information**

For the complete and accurate list of supported chip series, corresponding development boards and platforms, and detailed SDK versions, please refer to [Release Notes](./doc/tl_ble_sdk_Release_Note.md).

# Documentation and resources 

**Document Navigation**

| Documentation | Description |
| --- | --- |
| [Developer Handbook](https://doc.telink-semi.cn/doc/en/software/res/sdk/ble/tl_ble_sdk_multi_connection_en/tl_ble_sdk_multi_connection_en/) | Detailed explanation of software architecture, warehouse structure, and functional modules |
| [Release Notes](./doc/tl_ble_sdk_Release_Note.md) | Supported platforms, SDK versions, and detailed changes |

**Community and Resources**

| Resources | Description |
| --- | --- |
| [Telink Official Forum](https://forum.telink-semi.cn/) | Technical exchange and supporting |
| [Telink Official Website](https://www.telink-semi.com/) | Product Center and Documentation Center |
| [Github](https://github.com/telink-semi/tl_ble_sdk) / [Gitee](https://gitee.com/telink-semi/tl_ble_sdk.git) | SDK source code repository |

# License

This project adopts the following permits:

**Apache License, Version 2.0**

Licensed under the Apache License, Version 2.0 (the "License");

You may not use this file except in compliance with the License.

You may obtain a copy of the License at:

[http://www.apache.org/licenses/LICENSE-2.0](http://www.apache.org/licenses/LICENSE-2.0)

Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.

See the License for the specific language governing permissions and limitations under the License.
