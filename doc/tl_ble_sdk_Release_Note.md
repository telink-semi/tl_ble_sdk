## V4.0.4.6(PR)

### Version

* SDK Version: tl_ble_sdk V4.0.4.6
* Chip Version: 
    - TLSR921X(B91):           A2
    - TLSR922X/TLSR952X(B92):  A3/A4
    - TL721X:                  A2/A3
    - TL321X:                  A1/A2/A3
    - TL322X:                  A1
    - TL323X:                  A0
* Hardware EVK Version:
    - TLSR921X:                C1T213A20_V1.3
    - TLSR952X:                C1T266A20_V1.3
    - TL721X:                  C1T315A20_V1.2/AIOT_DK1:ML7218D1/ML7218A
    - TL321X:                  C1T331A20_V1.0/C1T335A20_V1.3
    - TL322X:                  C1T382A20_V1.2
    - TL323X:                  C1T388A20_V1.1
* Platform Version: 
    - TLSR921X:                tl_platform_sdk V3.10.0
    - TLSR922X/TLSR952X:       tl_platform_sdk V3.10.0
    - TL721X:                  tl_platform_sdk V3.10.0
    - TL321X:                  tl_platform_sdk V3.10.0
    - TL322X:                  tl_platform_sdk V3.10.0
    - TL323X:                  tl_platform_sdk V3.10.0
* Toolchain Version:
    - TLSR921X:                TL32 ELF MCULIB V5F GCC7.4  (IDE: [TelinkIoTStudio](https://www.telink-semi.com/development-tools))
    - TLSR922X/TLSR952X:       TL32 ELF MCULIB V5F GCC12.2  (IDE: [TelinkIoTStudio](https://www.telink-semi.com/development-tools))
    - TL721X                   TL32 ELF MCULIB V5F GCC12.2  (IDE: [TelinkIoTStudio](https://www.telink-semi.com/development-tools))
    - TL321X:                  TL32 ELF MCULIB V5 GCC12.2  (IDE: [TelinkIoTStudio](https://www.telink-semi.com/development-tools))
    - TL322X:                  TL32 ELF MCULIB V5F GCC12.2  (IDE: [TelinkIoTStudio](https://www.telink-semi.com/development-tools))
    - TL323X:                  TL32 ELF MCULIB V5F GCC12.2  (IDE: [TelinkIoTStudio](https://www.telink-semi.com/development-tools))

### Note

   * N/A


### Bug Fixes

* **Drivers**
  * For TL321X:
    * Adjust the retention ldo voltage from 0.8V to 0.95V:
        * Detailed description: When using deep retention sleep, the device would probabilistically crash after waking up with the default retention ldo setting of 0.8V.
        * After Fix: After raising the voltage, the device can work normally after deep retention wake-up, but electric increased by approximately 0.5 uA.
        * Update Recommendation: An update is mandatory when using deep retention sleep.
  * For TL322X:
    * The issue of sampling value anomalies when using SAR0 and SAR1 together in different configurations has been resolved:
        * Detailed description: When using SAR0 and SAR1 together, different voltage prescale ratios can affect the accuracy of the sampling values.
        * After Fix: Both SAR0 and SAR1 can be used simultaneously, and the sampling values are normal.
        * Update Recommendation: When both SAR0 and SAR1 are used simultaneously, an update must be performed.
    * Fixed minor packet loss issue in BLE coded PHY S2/S8 RX:
        * Detailed Description: During BLE coded PHY S2/S8 communication, when the TX energy is sufficiently high, a small packet loss issue (0.1%–0.2%) still occurs at the RX end.
        * After Fix: Modify RX-related configurations to achieve zero packet loss at the RX end when energy is sufficiently high.
        * Update Recommendation: The update is mandatory when using the BLE Coded PHY S2/S8.
    * Fix the sync error issue of TX during BQB testing for BLE Coded PHY S2/S8:
        * Detailed Description: Synchronization errors are observed on certain test instruments when performing BQB certification testing for BLE Coded PHY S2/S8.
        * After Fix: PA ramp-related configurations were modified, and sync errors no longer occur in Coded PHY S2/S8 TX testing.
        * Update Recommendation: The update is mandatory when using the BLE Coded PHY S2/S8.
    * Implemented the gpio_set_mspi_pin_ie_en and gpio_set_mspi_pin_ie_dis interfaces:
        * Detailed description: This interface has no effect on Mspi pin operations.
        * After Fix: Using this interface, you can enable or disable the MSPI pin and activate or deactivate the MSPI function.
        * Update Recommendation: If this interface is used, it must be updated.
  * For TL321x/TL721x/TL322x/TL323x:
    * The issue of the sleep function being called when the wake-up source remained active and causing a runtime error has been resolved:
        * Detailed description: In the sleep function, the PLL module is turned off until it is awakened and then reopened. If the wake-up source persists and the actual sleep has not occurred, the PLL module will not be reopened, resulting in abnormal clock operation. The logic has been modified; instead of manually turning off the PLL module, it will be automatically turned off only when the device is truly in sleep mode.
        * After Fix: The sleep function can run normally when the wake-up source persists.
        * Update Recommendation: It must be updated.
  * For TLSR921x/TLSR951x/TLSR922x/TLSR952x/TL321x/TL721x:
    * Fixed the issue of incorrect parameter type for the rf_tx_acc_code_pipe_en interface:
        * Detailed Description: The parameter type of the rf_tx_acc_code_pipe_en interface was changed from rf_channel_e to unsigned char. The pipe definition (as bit * positions) in the rf_channel_e enumeration was inconsistent with the actual register configuration logic.
        * After Effect: The pipe value is correctly set after configuring rf_tx_acc_code_pipe_en.
        * Update Recommendation: Mandatory update.

### BREAKING CHANGES

* For TL323X:
    * Adjusted LPD (Low-Voltage Protection) software configuration logic to prevent unintended LPD triggering during chip power-up or wake-up from sleep mode, which could lead to system lock-up and failure to operate normally:
        * Detailed Description: The original configuration sequence of digital and analog modules deviated from the intended design. This could cause the LPD module to be falsely triggered due to abnormal state detection when the supply voltage was below 2.1V during power-up or wake-up from sleep mode, resulting in system lock-up and malfunction. To resolve this issue, the initialization order of digital and analog modules has been restructured, and explicit state flag clearing has been added at critical stages to ensure the LPD module remains in a controlled state during early power-up.
        * After Effect: After optimization, unintended LPD triggering during power-up and wake-up is effectively prevented. The reliability and stability of chip startup have been significantly improved. Meanwhile, the minimum operating voltage has been reduced from 2.1V to 1.9V.
        * Update Recommendation:Firmware update is required.
* For TL322X:
    * Added SAR1 ADC calibration function:
        * Detailed description: Modified the parameters passed to adc_calculate_voltage, removed the adc_set_gpio_calib_vref and adc_set_vbat_calib_vref interfaces, and added the adc_set_sar0_gpio_calib_vref, adc_set_sar0_vbat_calib_vref, and adc_set_sar1_gpio_calib_vref functions.
        * After effect: Supports SAR1 ADC calibration function.
        * Update recommendation: Must update when using SAR1.
* For TL721X:
    * Compensation is implemented inside audio_set_stream0_dig_gain to resolve the inconsistent digital gain between Chip A3 and A2 under the same parameters:
        * Detailed Description: As the digital design of the DMIC on Chip A3 has been updated, the DMIC path of A3 has an additional +15.5 dB gain compared to A2. Therefore, software compensation is applied in the audio_set_stream0_dig_gain interface.
        * After Fix: With the same interface parameters, A2 and A3 achieve similar gain levels. In practice, the gain of A3 is only 1 dB higher than that of A2.
        * Update Recommendation: Must be updated when using DMIC.
* For all chips, synchronized tl_platform_sdk V3.10.0.

### Features

* **BLE general function** 
    - N/A
* **2.4G general function** 
    - N/A
* **Drivers**
    * N/A

* **Others**
    * N/A


### Refactoring

   * N/A

### Performance Improvements

   * N/A

### Known issues

* N/A

### CodeSize

* TLSR921X
    - Compiling acl_central_demo
        - Flash bin size: 105.52 KB
        - IRAM size: 54.29 KB
        - DRAM size: 0.65 KB
    - Compiling acl_connection_demo
        - Flash bin size: 128.02 KB
        - IRAM size: 65.77 KB
        - DRAM size: 0.84 KB
    - Compiling acl_peripheral_demo
        - Flash bin size: 110.83 KB
        - IRAM size: 55.75 KB
        - DRAM size: 0.77 KB
    - Compiling eslp_ap_dmeo
        - Flash bin size: 249.14 KB
        - IRAM size: 93.77 KB
        - DRAM size: 24.02 KB
* TLSR922X/TLSR952X
    - Compiling acl_central_demo
        - Flash bin size: 105.42 KB
        - IRAM size: 54.60 KB
        - DRAM size: 0.70 KB
    - Compiling acl_connection_demo
        - Flash bin size: 127.75 KB
        - IRAM size: 65.86 KB
        - DRAM size: 0.94 KB
    - Compiling acl_peripheral_demo
        - Flash bin size: 111.58 KB
        - IRAM size: 56.10 KB
        - DRAM size: 0.87 KB
    - Compiling eslp_ap_dmeo
        - Flash bin size: 257.20 KB
        - IRAM size: 93.61 KB
        - DRAM size: 24.09 KB

* TL721X
    - Compiling acl_central_demo
        - Flash bin size:  110.96 KB
        - IRAM size: 58.23 KB
        - DRAM size: 0.74 KB
    - Compiling acl_connection_demo
        - Flash bin size: 133.00 KB
        - IRAM size: 69.50 KB
        - DRAM size: 0.98 KB
    - Compiling acl_peripheral_demo
        - Flash bin size: 117.30 KB
        - IRAM size: 59.98 KB
        - DRAM size: 0.91 KB
    - Compiling eslp_ap_dmeo
        - Flash bin size: 268.51 KB
        - IRAM size: 98.24 KB
        - DRAM size: 21.62 KB
    - Compiling eslp_esl_dmeo
        - Flash bin size: 203.16 KB
        - IRAM size: 69.83 KB
        - DRAM size: 5.36 KB

* TL321X
    - Compiling acl_central_demo
        - Flash bin size: 113.16 KB
        - IRAM size: 55.36 KB
        - DRAM size: 0.71 KB
    - Compiling acl_connection_demo
        - Flash bin size: 135.22 KB
        - IRAM size: 66.63 KB
        - DRAM size: 0.96 KB
    - Compiling acl_peripheral_demo
        - Flash bin size:  119.45 KB
        - IRAM size: 57.11 KB
        - DRAM size: 0.88 KB
    - Compiling eslp_ap_dmeo
        - Flash bin size: 258.54 KB
        - IRAM size: 90.60 KB
        - DRAM size: 21.60 KB
    - Compiling eslp_esl_dmeo
        - Flash bin size: 204.53 KB
        - IRAM size: 66.45 KB
        - DRAM size: 5.33 KB
* TL322X
    - Compiling acl_central_demo
        - Flash bin size: 116.69 KB
        - IRAM size: 61.36 KB
        - DRAM size: 0.70 KB
    - Compiling acl_connection_demo
        - Flash bin size: 139.84 KB
        - IRAM size: 73.18 KB
        - DRAM size: 0.68 KB
    - Compiling acl_peripheral_demo
        - Flash bin size:  123.85 KB
        - IRAM size: 63.15 KB
        - DRAM size: 0.61 KB
* TL323X
    - Compiling acl_central_demo
        - Flash bin size: 106.39 KB
        - IRAM size: 55.58 KB
        - DRAM size: 0.66 KB
    - Compiling acl_connection_demo
        - Flash bin size: 128.88 KB
        - IRAM size: 66.35 KB
        - DRAM size: 0.64 KB
    - Compiling acl_peripheral_demo
        - Flash bin size:  112.95 KB
        - IRAM size: 56.83 KB
        - DRAM size: 0.57 KB

**Note:** The above IRAM usage includes the FIFO buffer required for the debug logging functionality. Disabling the `TLKAPI_DEBUG_ENABLE` macro can save approximately 4.76 KB of IRAM.



### 版本

* SDK Version: tl_ble_sdk V4.0.4.6
* Chip Version: 
    - TLSR921X(B91):           A2
    - TLSR922X/TLSR952X(B92):  A3/A4
    - TL721X:                  A2/A3
    - TL321X:                  A1/A2/A3
    - TL322X:                  A1
    - TL323X:                  A0
* Hardware EVK Version:
    - TLSR921X:                C1T213A20_V1.3
    - TLSR952X:                C1T266A20_V1.3
    - TL721X:                  C1T315A20_V1.2/AIOT_DK1:ML7218D1/ML7218A
    - TL321X:                  C1T331A20_V1.0/C1T335A20_V1.3
    - TL322X:                  C1T382A20_V1.2
    - TL323X:                  C1T388A20_V1.1
* Platform Version: 
    - TLSR921X:                tl_platform_sdk V3.10.0
    - TLSR922X/TLSR952X:       tl_platform_sdk V3.10.0
    - TL721X:                  tl_platform_sdk V3.10.0
    - TL321X:                  tl_platform_sdk V3.10.0
    - TL322X:                  tl_platform_sdk V3.10.0
    - TL323X:                  tl_platform_sdk V3.10.0
* Toolchain Version:
    - TLSR921X:                TL32 ELF MCULIB V5F GCC7.4  (IDE: [TelinkIoTStudio](https://www.telink-semi.com/development-tools))
    - TLSR922X/TLSR952X:       TL32 ELF MCULIB V5F GCC12.2  (IDE: [TelinkIoTStudio](https://www.telink-semi.com/development-tools))
    - TL721X                   TL32 ELF MCULIB V5F GCC12.2  (IDE: [TelinkIoTStudio](https://www.telink-semi.com/development-tools))
    - TL321X:                  TL32 ELF MCULIB V5 GCC12.2  (IDE: [TelinkIoTStudio](https://www.telink-semi.com/development-tools))
    - TL322X:                  TL32 ELF MCULIB V5F GCC12.2  (IDE: [TelinkIoTStudio](https://www.telink-semi.com/development-tools))
    - TL323X:                  TL32 ELF MCULIB V5F GCC12.2  (IDE: [TelinkIoTStudio](https://www.telink-semi.com/development-tools))


### Note

   * N/A

### Bug Fixes

* **Drivers**
    * 对于 TL321X：
        * retention ldo电压从0.8V抬到0.95V：
            * 详细描述：使用deep retention睡眠时，用默认的retention ldo挡位0.8V会出现唤醒后低概率性死机。
            * 修复效果：抬高电压后，deep rentention唤醒后能正常工作, 但deep rentention睡眠电流增加约0.5uA。
            * 更新建议：在使用deep retention睡眠时，必须进行更新。
    * 对于 TL322X:
        * 解决了SAR0与SAR1在不同配置下共同使用时采样值异常的问题：
            * 详细描述：在SAR0与SAR1共同使用时，不同的分压系数会影响采样值的正确性。
            * 修复效果：可以同时使用SAR0和SAR1，且采样值正常。
            * 更新建议：同时使用SAR0和SAR1时，必须进行更新。
        * 修复BLE coded PHY S2/S8 RX少量丢包问题:
            * 详细描述：BLE coded PHY S2/S8 通信时当TX能量足够大时，RX端仍存在少量丢包的问题（0.1%~0.2%）。
            * 修复效果：修改RX相关配置，当能量足够大时rx端丢包率为0。
            * 更新建议：使用BLE coded PHY S2/S8时必须更新。
        * 修复BLE coded PHY S2/S8 BQB测试tx时sync error的问题:
            * 详细描述：BLE coded PHY S2/S8 在进行BQB测试时，部分仪器会出现sync error的错误。
            * 修复效果：修改PA ramp相关配置，测试coded PHY S2/S8 tx时未再出现sync error的问题。
            * 更新建议：使用BLE coded PHY S2/S8时必须更新。
        * 修复了gpio_set_mspi_pin_ie_en 和 gpio_set_mspi_pin_ie_dis 接口:
            * 详细描述：该接口对mspi pin操作不生效。
            * 修复效果：使用该接口将mspi pin使能或关闭mspi功能。
            * 更新建议：如果使用了该接口，则必须更新。
    * 对于 TL321x/TL721x/TL322x/TL323x:
        * 解决了在唤醒源一直存在的时候调用睡眠函数，发生运行异常的问题：
            * 详细描述：睡眠函数中关掉了PLL模块，直到唤醒时重新打开PLL，如果唤醒源一直存在，没有真正进睡眠，就不会重新打开PLL，导致时钟异常。修改逻辑，不手动关闭PLL模块，只在真正睡眠时自动关闭PLL模块。
            * 修复效果：在唤醒源一直存在的时候调用睡眠函数运行正常。
            * 更新建议：必须更新。
    * 对于 TLSR921x/TLSR951x/TLSR922x/TLSR952x/TL321x/TL721x:
        * 修复了rf_tx_acc_code_pipe_en 接口参数类型错误问题：
            * 详细描述：rf_tx_acc_code_pipe_en 接口参数类型从 rf_channel_e 改为 unsigned char.rf_channel_e 枚举中对pipe 定义为bit位和实际寄存器设置逻辑不一致。
            * 修复效果：rf_tx_acc_code_pipe_en 设置后pipe值正确。
            * 更新建议：必须更新。
### BREAKING CHANGES
* 对于 TL323X:
    * 调整LPD(低电压保护)软件配置逻辑，防止芯片上电时或睡眠唤醒后误触发LPD，进而引发系统锁死，无法正常工作：
        * 详细描述：数字与模拟模块的配置顺序与预期不符，可能导致芯片在上电低于2.1V和睡眠唤醒过程中，LPD模块因状态判断异常而被误触发，进而引发系统锁死，无法正常工作。为解决该问题，对数字与模拟模块的初始化顺序进行了重构，并在关键阶段增加状态位清零操作，确保LPD模块在上电初期处于可控状态。
        * 修复效果：优化后，有效避免了上电和睡眠唤醒过程中的误触发LPD风险，提升了芯片启动的可靠性与稳定性，同时最低工作电压从2.1V降低到1.9V。
        * 更新建议：必须更新。
* 对于 TL322X:
    * 增加sar1 adc校准功能:
        * 详细描述：修改了adc_calculate_voltage传入参数，删除了adc_set_gpio_calib_vref和adc_set_vbat_calib_vref接口，增加了adc_set_sar0_gpio_calib_vref、adc_set_sar0_vbat_calib_vref、adc_set_sar1_gpio_calib_vref函数。
        * 修复效果：支持sar1 adc校准功能。
        * 更新建议：使用sar1时必须更新。
* 对于 TL721X:
    * 在 audio_set_stream0_dig_gain 接口内部做补偿 ，解决相同参数下，A3芯片和A2芯片数字增益不一致问题：
        * 详细描述：由于A3芯片DMIC 数字设计有更新，dmic path A3 相比A2 有额外+15.5dB增益，因此在audio_set_stream0_dig_gain接口内做了软件补偿。
        * 修复效果：相同的接口参数下，A3和A2 具有相近的增益，实际效果A3的增益仅比A2大1dB。
        * 更新建议：使用dmic 必须更新。
* 对于所有芯片，同步 tl_platform_sdk V3.10.0。

### Features

* **BLE通用功能**
    - N/A
* **2.4G通用功能**
    - N/A
* **Drivers**
    * N/A

* **Others**
    * N/A


### Refactoring

* N/A


### Performance Improvements

   * N/A

### Known issues

* N/A

### CodeSize

* TLSR921X
    - Compiling acl_central_demo
        - Flash bin size: 105.52 KB
        - IRAM size: 54.29 KB
        - DRAM size: 0.65 KB
    - Compiling acl_connection_demo
        - Flash bin size: 128.02 KB
        - IRAM size: 65.77 KB
        - DRAM size: 0.84 KB
    - Compiling acl_peripheral_demo
        - Flash bin size: 110.83 KB
        - IRAM size: 55.75 KB
        - DRAM size: 0.77 KB
    - Compiling eslp_ap_dmeo
        - Flash bin size: 249.14 KB
        - IRAM size: 93.77 KB
        - DRAM size: 24.02 KB
* TLSR922X/TLSR952X
    - Compiling acl_central_demo
        - Flash bin size: 105.42 KB
        - IRAM size: 54.60 KB
        - DRAM size: 0.70 KB
    - Compiling acl_connection_demo
        - Flash bin size: 127.75 KB
        - IRAM size: 65.86 KB
        - DRAM size: 0.94 KB
    - Compiling acl_peripheral_demo
        - Flash bin size: 111.58 KB
        - IRAM size: 56.10 KB
        - DRAM size: 0.87 KB
    - Compiling eslp_ap_dmeo
        - Flash bin size: 257.20 KB
        - IRAM size: 93.61 KB
        - DRAM size: 24.09 KB

* TL721X
    - Compiling acl_central_demo
        - Flash bin size:  110.96 KB
        - IRAM size: 58.23 KB
        - DRAM size: 0.74 KB
    - Compiling acl_connection_demo
        - Flash bin size: 133.00 KB
        - IRAM size: 69.50 KB
        - DRAM size: 0.98 KB
    - Compiling acl_peripheral_demo
        - Flash bin size: 117.30 KB
        - IRAM size: 59.98 KB
        - DRAM size: 0.91 KB
    - Compiling eslp_ap_dmeo
        - Flash bin size: 268.51 KB
        - IRAM size: 98.24 KB
        - DRAM size: 21.62 KB
    - Compiling eslp_esl_dmeo
        - Flash bin size: 203.16 KB
        - IRAM size: 69.83 KB
        - DRAM size: 5.36 KB

* TL321X
    - Compiling acl_central_demo
        - Flash bin size: 113.16 KB
        - IRAM size: 55.36 KB
        - DRAM size: 0.71 KB
    - Compiling acl_connection_demo
        - Flash bin size: 135.22 KB
        - IRAM size: 66.63 KB
        - DRAM size: 0.96 KB
    - Compiling acl_peripheral_demo
        - Flash bin size:  119.45 KB
        - IRAM size: 57.11 KB
        - DRAM size: 0.88 KB
    - Compiling eslp_ap_dmeo
        - Flash bin size: 258.54 KB
        - IRAM size: 90.60 KB
        - DRAM size: 21.60 KB
    - Compiling eslp_esl_dmeo
        - Flash bin size: 204.53 KB
        - IRAM size: 66.45 KB
        - DRAM size: 5.33 KB
* TL322X
    - Compiling acl_central_demo
        - Flash bin size: 116.69 KB
        - IRAM size: 61.36 KB
        - DRAM size: 0.70 KB
    - Compiling acl_connection_demo
        - Flash bin size: 139.84 KB
        - IRAM size: 73.18 KB
        - DRAM size: 0.68 KB
    - Compiling acl_peripheral_demo
        - Flash bin size:  123.85 KB
        - IRAM size: 63.15 KB
        - DRAM size: 0.61 KB
* TL323X
    - Compiling acl_central_demo
        - Flash bin size: 106.39 KB
        - IRAM size: 55.58 KB
        - DRAM size: 0.66 KB
    - Compiling acl_connection_demo
        - Flash bin size: 128.88 KB
        - IRAM size: 66.35 KB
        - DRAM size: 0.64 KB
    - Compiling acl_peripheral_demo
        - Flash bin size:  112.95 KB
        - IRAM size: 56.83 KB
        - DRAM size: 0.57 KB

**Note:** 上述 IRAM 统计包含了打印功能所需的 FIFO，关闭宏 TLKAPI_DEBUG_ENABLE，可以节省约 4.76 KB 的 IRAM。


## V4.0.4.5(PR)

### Version

* SDK Version: tl_ble_sdk V4.0.4.5
* Chip Version: 
    - TLSR921X(B91):           A2
    - TLSR922X/TLSR952X(B92):  A3/A4
    - TL721X:                  A2/A3
    - TL321X:                  A1/A2/A3
    - TL322X:                  A1
    - TL323X:                  A0
* Hardware EVK Version:
    - TLSR921X:                C1T213A20_V1.3
    - TLSR952X:                C1T266A20_V1.3
    - TL721X:                  C1T315A20_V1.2/AIOT_DK1:ML7218D1/ML7218A
    - TL321X:                  C1T331A20_V1.0/C1T335A20_V1.3
    - TL322X:                  C1T382A20_V1.2
    - TL323X:                  C1T388A20_V1.1
* Platform Version: 
    - TLSR921X:                tl_platform_sdk V3.7.0
    - TLSR922X/TLSR952X:       tl_platform_sdk V3.7.0
    - TL721X:                  tl_platform_sdk V3.8.0
    - TL321X:                  tl_platform_sdk V3.8.0
    - TL322X:                  tl_platform_sdk V3.9.0
    - TL323X:                  tl_platform_sdk V3.9.0
* Toolchain Version:
    - TLSR921X:                TL32 ELF MCULIB V5F GCC7.4  (IDE: [TelinkIoTStudio](https://www.telink-semi.com/development-tools))
    - TLSR922X/TLSR952X:       TL32 ELF MCULIB V5F GCC12.2  (IDE: [TelinkIoTStudio](https://www.telink-semi.com/development-tools))
    - TL721X                   TL32 ELF MCULIB V5F GCC12.2  (IDE: [TelinkIoTStudio](https://www.telink-semi.com/development-tools))
    - TL321X:                  TL32 ELF MCULIB V5 GCC12.2  (IDE: [TelinkIoTStudio](https://www.telink-semi.com/development-tools))
    - TL322X:                  TL32 ELF MCULIB V5F GCC12.2  (IDE: [TelinkIoTStudio](https://www.telink-semi.com/development-tools))
    - TL323X:                  TL32 ELF MCULIB V5F GCC12.2  (IDE: [TelinkIoTStudio](https://www.telink-semi.com/development-tools))

### Note

   * N/A


### Bug Fixes

* **Drivers**
    * For TL321X and TL322X, fixed the issue of occasional chip resets caused by the prolonged PLL stabilization time:
        * Detailed Description: Increased the PLL charge pump current to reduce PLL startup time; performed PLL LDO trimming to improve PLL startup success rate.
        * After Fix: The chip stability issue caused by excessively long PLL stabilization time has not recurred during two weeks of stability testing after the fix.
        * Update Recommendation: Mandatory update.
    * For TL322X, fixed overshoot at the end of data packets during RF TX transmission :
        * Detailed description: During RF TX transmission, due to the ramp down time being greater than the end time of the state machine, there may be energy overshoot at the end of the data packet, and the end time of the TX state machine needs to be appropriately delayed.
        * After Fix: There is no energy overshoot at the end of the data packet during RF TX transmission.
        * Update Recommendation: Evaluate if needed.
    * For TL322X, fixed the issue of occasional RF packet reception failure in HP mode:
        * Detailed description: Fixed the issue where RF occasionally fails to receive packets when rf_modem_hp_path(1) is called after rf_mode_init().
        * After Fix: The RF packet reception function works normally, and the issue of failed packet reception has not been reproduced.
        * Update Recommendation: Mandatory update.
    * For TL322X, fixed the issue where ADC did not report sampled data:
        * Detailed Description：The ADC module is not functioning properly and is unable to collect any data.
        * After Fix：The ADC is able to collect data normally.
        * Update Recommendation：When using ADC, updates must be performed.
    * For TL322X, fixed potential abnormal timeout issue in the spi_master_write_read_full_duplex interface:
        * Detailed description:When the length parameter exceeds the chunk_size, data transmission is divided into packets. Between each packet, the FIFO is incorrectly cleared. If the FIFO is empty, the while loop in spi_rxfifo_is_empty continues indefinitely, triggering a timeout.
        * Impact Scope：Using this interface may trigger a timeout.
        * Update Recommendation:Mandatory update.
    * For TL322X, fixed an issue where SPI modules behaved abnormally after waking from suspend when SPI was combined with suspend:
        * Detailed description:
        * After Fix: When the SPI source is set to PLL, the SPI frequency divider loses lock upon wake-up from suspend, causing SPI functionality to become abnormal.
        * Update Recommendation:Mandatory update.
    * For TL322X, fixed the issue where the corresponding interface configurations for rz_set_polarity, rz_set_msb_lsb_mode, rz_set_big_little_endian_mode, rz_set_data_align_mode, rz_set_global_data_mode, rz_set_tx_data_mode, rz_jitter_range_config, and rz_set_fifo_lvl were incorrect:
        - Detailed description: The configurations for the above interface registers were incorrect, causing functional abnormalities.
        - Impact Scope: Configurations are now correct, and functionality is normal.
        - Update recommendation: Mandatory update.

### BREAKING CHANGES

* For TL321X, synchronized tl_platform_sdk V3.8.0 and supported the A3 version chips;
* For TL721X, synchronized tl_platform_sdk V3.8.0;
* For TL322X, synchronized tl_platform_sdk V3.9.0 and supported the A1 version chips.
* For TL323X, synchronized tl_platform_sdk V3.9.0 and supported the A0 version chips.
* Updated the early wake-up time for TL322X and TL323X in deep retention mode.

### Features

* **BLE general function** 
    - N/A
* **2.4G general function** 
    - N/A
* **Drivers**
    * N/A

* **Others**
    * N/A


### Refactoring

   * N/A

### Performance Improvements

   * N/A

### Known issues

* N/A

### CodeSize

* TLSR921X
    - Compiling acl_central_demo
        - Flash bin size: 105.52 KB
        - IRAM size: 54.29 KB
        - DRAM size: 0.65 KB
    - Compiling acl_connection_demo
        - Flash bin size: 128.02 KB
        - IRAM size: 65.77 KB
        - DRAM size: 0.84 KB
    - Compiling acl_peripheral_demo
        - Flash bin size: 110.83 KB
        - IRAM size: 55.75 KB
        - DRAM size: 0.77 KB
    - Compiling eslp_ap_dmeo
        - Flash bin size: 249.14 KB
        - IRAM size: 93.77 KB
        - DRAM size: 24.02 KB
* TLSR922X/TLSR952X
    - Compiling acl_central_demo
        - Flash bin size: 105.42 KB
        - IRAM size: 54.60 KB
        - DRAM size: 0.70 KB
    - Compiling acl_connection_demo
        - Flash bin size: 127.75 KB
        - IRAM size: 65.86 KB
        - DRAM size: 0.94 KB
    - Compiling acl_peripheral_demo
        - Flash bin size: 111.58 KB
        - IRAM size: 56.10 KB
        - DRAM size: 0.87 KB
    - Compiling eslp_ap_dmeo
        - Flash bin size: 257.20 KB
        - IRAM size: 93.61 KB
        - DRAM size: 24.09 KB

* TL721X
    - Compiling acl_central_demo
        - Flash bin size:  110.96 KB
        - IRAM size: 58.23 KB
        - DRAM size: 0.74 KB
    - Compiling acl_connection_demo
        - Flash bin size: 133.00 KB
        - IRAM size: 69.50 KB
        - DRAM size: 0.98 KB
    - Compiling acl_peripheral_demo
        - Flash bin size: 117.30 KB
        - IRAM size: 59.98 KB
        - DRAM size: 0.91 KB
    - Compiling eslp_ap_dmeo
        - Flash bin size: 268.51 KB
        - IRAM size: 98.24 KB
        - DRAM size: 21.62 KB
    - Compiling eslp_esl_dmeo
        - Flash bin size: 203.16 KB
        - IRAM size: 69.83 KB
        - DRAM size: 5.36 KB

* TL321X
    - Compiling acl_central_demo
        - Flash bin size: 113.16 KB
        - IRAM size: 55.36 KB
        - DRAM size: 0.71 KB
    - Compiling acl_connection_demo
        - Flash bin size: 135.22 KB
        - IRAM size: 66.63 KB
        - DRAM size: 0.96 KB
    - Compiling acl_peripheral_demo
        - Flash bin size:  119.45 KB
        - IRAM size: 57.11 KB
        - DRAM size: 0.88 KB
    - Compiling eslp_ap_dmeo
        - Flash bin size: 258.54 KB
        - IRAM size: 90.60 KB
        - DRAM size: 21.60 KB
    - Compiling eslp_esl_dmeo
        - Flash bin size: 204.53 KB
        - IRAM size: 66.45 KB
        - DRAM size: 5.33 KB
* TL322X
    - Compiling acl_central_demo
        - Flash bin size: 116.69 KB
        - IRAM size: 61.36 KB
        - DRAM size: 0.70 KB
    - Compiling acl_connection_demo
        - Flash bin size: 139.84 KB
        - IRAM size: 73.18 KB
        - DRAM size: 0.68 KB
    - Compiling acl_peripheral_demo
        - Flash bin size:  123.85 KB
        - IRAM size: 63.15 KB
        - DRAM size: 0.61 KB
* TL323X
    - Compiling acl_central_demo
        - Flash bin size: 106.45 KB
        - IRAM size: 55.56 KB
        - DRAM size: 0.66 KB
    - Compiling acl_connection_demo
        - Flash bin size: 128.88. KB
        - IRAM size: 66.33 KB
        - DRAM size: 0.64 KB
    - Compiling acl_peripheral_demo
        - Flash bin size:  112.71 KB
        - IRAM size: 56.55 KB
        - DRAM size: 0.57 KB

**Note:** The above IRAM usage includes the FIFO buffer required for the debug logging functionality. Disabling the `TLKAPI_DEBUG_ENABLE` macro can save approximately 4.76 KB of IRAM.



### 版本

* SDK Version: tl_ble_sdk V4.0.4.5
* Chip Version: 
    - TLSR921X(B91):           A2
    - TLSR922X/TLSR952X(B92):  A3/A4
    - TL721X:                  A2/A3
    - TL321X:                  A1/A2/A3
    - TL322X:                  A1
    - TL323X:                  A0
* Hardware EVK Version:
    - TLSR921X:                C1T213A20_V1.3
    - TLSR952X:                C1T266A20_V1.3
    - TL721X:                  C1T315A20_V1.2/AIOT_DK1:ML7218D1/ML7218A
    - TL321X:                  C1T331A20_V1.0/C1T335A20_V1.3
    - TL322X:                  C1T382A20_V1.2
    - TL323X:                  C1T388A20_V1.1
* Platform Version: 
    - TLSR921X:                tl_platform_sdk V3.7.0
    - TLSR922X/TLSR952X:       tl_platform_sdk V3.7.0
    - TL721X:                  tl_platform_sdk V3.8.0
    - TL321X:                  tl_platform_sdk V3.8.0
    - TL322X:                  tl_platform_sdk V3.9.0
    - TL323X:                  tl_platform_sdk V3.9.0
* Toolchain Version:
    - TLSR921X:                TL32 ELF MCULIB V5F GCC7.4  (IDE: [TelinkIoTStudio](https://www.telink-semi.com/development-tools))
    - TLSR922X/TLSR952X:       TL32 ELF MCULIB V5F GCC12.2  (IDE: [TelinkIoTStudio](https://www.telink-semi.com/development-tools))
    - TL721X                   TL32 ELF MCULIB V5F GCC12.2  (IDE: [TelinkIoTStudio](https://www.telink-semi.com/development-tools))
    - TL321X:                  TL32 ELF MCULIB V5 GCC12.2  (IDE: [TelinkIoTStudio](https://www.telink-semi.com/development-tools))
    - TL322X:                  TL32 ELF MCULIB V5F GCC12.2  (IDE: [TelinkIoTStudio](https://www.telink-semi.com/development-tools))
    - TL323X:                  TL32 ELF MCULIB V5F GCC12.2  (IDE: [TelinkIoTStudio](https://www.telink-semi.com/development-tools))


### Note

   * N/A

### Bug Fixes

* **Drivers**
    * 对于 TL321X 和 TL322X，修复了个别芯片偶尔出现因为PLL稳定时间过长导致复位的问题：
        * 详细描述：增加pll的chargepump电流，减小pll启动时间；pll ldo trim，增加pll启动机会。
        * 修复效果：因为PLL稳定时间过长导致复位问题的芯片稳定性测试两周没再复现问题。
        * 更新建议：必须更新。
    * 对于 TL322X，修复RF TX传输期间数据包末尾的能量过冲现象：
        * 详细描述：RF TX 传输期间由于ramp down时间大于状态机结束的时间，会导致数据包末尾有能量过冲现象，需要将tx 状态机结束时间适当延后。
        * 修复效果：RF TX 传输期间数据包末尾的能量无过冲现象。
        * 更新建议：客户自行评估。
    * 对于 TL322X，修复 hp 模式下概率性出现 rf 收不到包的问题：
        * 详细描述：在rf_mode_init()后调用rf_modem_hp_path(1);概率性出现rf收不到包的问题。
        * 修复效果：RF收包功能正常，未复现到收不到包的问题。
        * 更新建议：必须更新。
    * 对于 TL322X，修复了 adc 无采样数据上报的问题：
        * 详细描述：adc 模块无法正常工作，未能采集到任何数据。
        * 修复效果：adc 可以正常采集数据。
        * 更新建议：使用 adc 时必须更新。
    * 对于 TL322X，修复 spi_master_write_read_full_duplex 接口可能异常超时问题：
        * 详细描述：length长度参数大于chunk_size时，会分包收发，每包之间会误清fifo，fifo为空时一直while在spi_rxfifo_is_empty从而触发超时。
        * 修复效果：使用该接口都可能触发超时。
        * 更新建议：必须更新。
    * 对于 TL322X，修复了 spi 结合 suspend 使用，唤醒后 spi 模块功能异常的问题：
        * 详细描述：当 spi 源被设置为 PLL 时，suspend 唤醒后，spi 的分频器失锁，导致 spi 功能异常。
        * 修复效果：spi 从 suspend 唤醒后功能正常。
        * 更新建议：必须更新。
    * 对于 TL322X，修复了 rz_set_polarity/rz_set_msb_lsb_mode/rz_set_big_little_endian_mode/rz_set_data_align_mode/rz_set_global_data_mode/rz_set_tx_data_mode/rz_jitter_range_config/rz_set_fifo_lvl 对应接口配置错误的问题：
        * 详细描述：以上的接口寄存器配置错误，导致功能异常。
        * 修复效果：修复后配置正确，功能正常。
        * 更新建议：必须更新。

### BREAKING CHANGES

* 对于 TL321X，同步 tl_platform_sdk V3.8.0，并添加对 A3 版本芯片的支持；
* 对于 TL721X，同步 tl_platform_sdk V3.8.0；
* 对于 TL322X，同步 tl_platform_sdk V3.9.0，并添加对 A0 版本芯片的支持；
* 对于 TL323X，同步 tl_platform_sdk V3.9.0，并添加对 A1 版本芯片的支持；
* 更新 TL322X 和 TL323X deep retention 模式下的提前唤醒时间。

### Features

* **BLE通用功能**
    - N/A
* **2.4G通用功能**
    - N/A
* **Drivers**
    * N/A

* **Others**
    * N/A


### Refactoring

* N/A


### Performance Improvements

   * N/A

### Known issues

* N/A

### CodeSize

* TLSR921X
    - Compiling acl_central_demo
        - Flash bin size: 105.52 KB
        - IRAM size: 54.29 KB
        - DRAM size: 0.65 KB
    - Compiling acl_connection_demo
        - Flash bin size: 128.02 KB
        - IRAM size: 65.77 KB
        - DRAM size: 0.84 KB
    - Compiling acl_peripheral_demo
        - Flash bin size: 110.83 KB
        - IRAM size: 55.75 KB
        - DRAM size: 0.77 KB
    - Compiling eslp_ap_dmeo
        - Flash bin size: 249.14 KB
        - IRAM size: 93.77 KB
        - DRAM size: 24.02 KB
* TLSR922X/TLSR952X
    - Compiling acl_central_demo
        - Flash bin size: 105.42 KB
        - IRAM size: 54.60 KB
        - DRAM size: 0.70 KB
    - Compiling acl_connection_demo
        - Flash bin size: 127.75 KB
        - IRAM size: 65.86 KB
        - DRAM size: 0.94 KB
    - Compiling acl_peripheral_demo
        - Flash bin size: 111.58 KB
        - IRAM size: 56.10 KB
        - DRAM size: 0.87 KB
    - Compiling eslp_ap_dmeo
        - Flash bin size: 257.20 KB
        - IRAM size: 93.61 KB
        - DRAM size: 24.09 KB

* TL721X
    - Compiling acl_central_demo
        - Flash bin size:  110.21 KB
        - IRAM size: 58.21 KB
        - DRAM size: 0.74 KB
    - Compiling acl_connection_demo
        - Flash bin size: 132.50 KB
        - IRAM size: 69.47 KB
        - DRAM size: 0.98 KB
    - Compiling acl_peripheral_demo
        - Flash bin size: 116.53 KB
        - IRAM size: 59.96 KB
        - DRAM size: 0.91 KB
    - Compiling eslp_ap_dmeo
        - Flash bin size: 268.19 KB
        - IRAM size: 98.24 KB
        - DRAM size: 21.62 KB
    - Compiling eslp_esl_dmeo
        - Flash bin size: 202.61 KB
        - IRAM size: 69.58 KB
        - DRAM size: 5.36 KB

* TL321X
    - Compiling acl_central_demo
        - Flash bin size: 112.09 KB
        - IRAM size: 55.34 KB
        - DRAM size: 0.71 KB
    - Compiling acl_connection_demo
        - Flash bin size: 134.40 KB
        - IRAM size: 66.61 KB
        - DRAM size: 0.96 KB
    - Compiling acl_peripheral_demo
        - Flash bin size:  118.11 KB
        - IRAM size: 56.84 KB
        - DRAM size: 0.88 KB
    - Compiling eslp_ap_dmeo
        - Flash bin size: 258.20 KB
        - IRAM size: 90.60 KB
        - DRAM size: 21.60 KB
    - Compiling eslp_esl_dmeo
        - Flash bin size: 204.21 KB
        - IRAM size: 66.45 KB
        - DRAM size: 5.33 KB
* TL321X
    - Compiling acl_central_demo
        - Flash bin size: 106.20 KB
        - IRAM size: 56.49 KB
        - DRAM size: 0.71 KB
    - Compiling acl_connection_demo
        - Flash bin size: 128.51 KB
        - IRAM size: 67.80 KB
        - DRAM size: 0.95 KB
    - Compiling acl_peripheral_demo
        - Flash bin size:  112.54 KB
        - IRAM size: 58.28 KB
        - DRAM size: 0.88 KB

**Note:** 上述 IRAM 统计包含了打印功能所需的 FIFO，关闭宏 TLKAPI_DEBUG_ENABLE，可以节省约 4.76 KB 的 IRAM。


## V4.0.4.4_Patch_0002(PR)

### Bug Fixes

* **Drivers**
  
  * For TL321X:
  
    * Fixed communication failure caused by RF occasional frequency lock issues:
        * Detailed Description: Due to the LDO trim voltage previously being at a critical threshold, the RF frequency occasionally failed to lock to the intended target frequency.
        * After Fix: After adjusting the LDO trim voltage, frequency locking is normal and RF TX/RX functions are operating correctly.
        * Update Recommendation: Mandatory update.
    * Fixed the issue that different parameter configurations in `rf_rx_performance_mode` caused inaccurate RSSI detection:
        * Detailed Description: Configurations under different parameters of `rf_rx_performance_mode` affect RSSI detection.
        * Impact Scope:  After modification, RSSI calculations are accurate under all parameters of `rf_rx_performance_mode`.
        * Update Recommendation:  Evaluate if needed.
    * Fixed the issue that RX Sensitivity occasionally decreased:
        * Detailed Description: Resolved RX performance degradation caused by probabilistic inaccuracies in certain parameter estimates.
        * After Fix: Modified the valuation algorithm implementation, and the RF performance returned to normal after the modification.
        * Update Recommendation: Mandatory update.
    * Fixed the issue that the RF module occasionally lacks a clock signal, causing TX/RX abnormalities:
        * Detailed Description: Due to incorrect power-up sequence, the RF module may experience a lack of clock input, resulting in the inability to transmit and receive packets normally.
        * After Fix: After modifying the power-up sequence, the RF module clock works normally, and transmit/receive works properly.
        * Update Recommendation: Mandatory update.
  * For TL721X:
      * Fixed the issue of  occasional packet loss in BLE coded PHY S2/S8 RX:
          * Detailed Description: During BLE coded PHY S2/S8 communication, when the TX energy is sufficiently high, a low-probability packet loss issue (0.1%–0.2%) still occurs at the receiver.
          * After Fix: Modified the RX-related configurations to achieve zero packet loss on the receiver when energy is sufficiently high.
          * Update Recommendation:  Evaluate if needed.
      * Fixed the issue where power cycling the ADC module caused inaccurate ADC sampling values:
          * Detailed Description: Power cycling the ADC module causes the ref voltage to drop slightly, leading to progressively inaccurate sampling.
          * Update Recommendation: Mandatory update.
  * For TL322X：
      * Fixed low-probability failure to detect PLL stable flag during power-up：
          * Detailed Description: Rare cases might miss PLL stable flag detection after power-up, potentially causing chip malfunction.
          * After Fix: Optimized PLL configuration to ensure stable flag detection.
          * Update Recommendation: Mandatory update.


### Features

* **Drivers**
    * For TL321X and TL721X, synchronized driver tl_platform_sdk V3.8.0.
    * For TL321X, added support for the A3 version chips.
* **BLE general function**
    * For TL321X/TL721X/TL322X, optimized the `blc_pm_setWfiMask` API to support enabling WFI for reducing power consumption.

### BREAKING CHANGES

   * For TL321X, currently, CCLK only supports up to 48 MHz. When CCLK > 48 MHz is required, to enhance high-frequency robustness, flash power-down protection must be added. Refer to the limitations in `clock.h`. After confirming no impact on your application, please contact Telink FAE support.
   * For TL721X,  all functionality of the PD4 pin has been removed and is not accessible to users. The datasheet has been updated accordingly:
     * In the design of TL7218X_C1T315A20_V1_5 and earlier versions, the PD4 pin originally used for KEY1 had been replaced with PB7 pin to ensure proper operation.
     * The assignments for `TLKAPI_DEBUG_UART_TX_PIN` and `TLKAPI_DEBUG_UART_RX_PIN` had been updated to `GPIO_FC_PB5` and `GPIO_FC_PB4`.
     * The assignments for `TLKAPI_DEBUG_GPIO_PIN` had been updated to `GPIO_PB6`.

### Bug Fixes

* **Drivers**
    * 对于 TL321X：
        * 修复 RF 概率性频率不锁定导致的通信失败问题：
            * 详细描述：由于之前 LDO Trim 电压处于临界值，导致 RF 会概率性出现频率未按照预期锁定到对应的频点。
            * 修复效果：修改 LDO Trim 电压后频率锁定正常，RF TX/RX 功能正常。
            * 更新建议：必须更新。
        * 修复 `rf_rx_performance_mode` 不同参数配置导致 RSSI 检测不准问题：
            * 详细描述：由于 `rf_rx_performance_mode` 不同参数下的配置对 RSSI 的检测产生了影响。
            * 修复效果：修改后 `rf_rx_performance_mode` 所有参数下 RSSI 计算均准确。
            * 更新建议：自行评估。
        * 修复 RX Sensitivity 概率性下降问题：
            * 详细描述：修复 RX 部分参数估计值概率性不准确导致 RX 性能下降。
            * 修复效果：修改估值算法方案，修改后 RF 性能恢复正常。
            * 更新建议：必须更新。
        * 修复 RF 模块概率性无 clock 输入导致 TX/RX 异常问题：
            * 详细描述：由于上电顺序错误，导致 RF 模块概率出现无 clock 输入从而引发无法正常进行收发包。
            * 修复效果：修改上电顺序后，RF 模块 clock 正常，收发功能正常。
            * 更新建议：必须更新。
    * 对于 TL721X：
        * 修复 BLE Coded PHY S2/S8 模式 RX 存在低概率丢包的问题：
            * 详细描述：BLE coded PHY S2/S8 通信时，当 TX 端能量足够大时，RX 端仍存在少量丢包的问题（0.1%~0.2%）。
            * 修复效果：修改 RX 相关配置，当能量足够大时 RX 端丢包率为 0。
            * 更新建议：自行评估。
        * 修复 ADC 模块上下电导致 ADC 采样值不准确的问题：
            * 详细描述：ADC 模块上下电，会导致 `Vref` 下降一点，导致采样越来越不准确。
            * 更新建议：必须更新。
    * 对于 TL322X，修复低概率检测不到PLL稳定标志位的问题：
        * 详细描述：低概率检测不到PLL稳定标志位的问题，这个问题可能会发生预期以外的一次或多次复位甚至不能正常运行。
        * 修复效果：优化PLL配置，确保PLL稳定标志位可以稳定的检测到。
        * 影响范围：个别情况下会检测不到。
        * 更新建议：必须更新，否则会导致运行异常。

### Features

* **Drivers**
    * 对于 TL321X 和 TL721X，同步 driver tl_platform_sdk 3.8.0。
    * 对于 TL321X，增加对 A3 版本芯片的支持。
* **BLE general function**
    * 对于 TL321X/TL721X/TL322X，完善 `blc_pm_setWfiMask` API，使能 WFI 来节省功耗。

### BREAKING CHANGES

   * 对于 TL321X，目前 CCLK 只支持到最高为 48 MHz，当需要支持 CCLK > 48 MHz 时，为了提高高频的鲁棒性，需要添加flash下电保护功能，请参考 `clock.h` 中的限制，确认对应用无影响后，请联系 Telink FAE 支持。
   * 对于 TL721X，删除了PD4引脚的所有功能，与datasheet同步：
     * 在 TL7218X_C1T315A20_V1_5 及之前版本的硬件设计中，原本分配给 KEY1 的 PD4 引脚已更换为 PB7 引脚以确保正常工作。
     * 分配给 `TLKAPI_DEBUG_UART_TX_PIN` 和 `TLKAPI_DEBUG_UART_RX_PIN` 更新为 `GPIO_FC_PB5` 和 `GPIO_FC_PB4`。
     * 分配给 `TLKAPI_DEBUG_GPIO_PIN` 的引脚更新为 `GPIO_PB6`。



## V4.0.4.4_Patch_0001(PR)

### Bug Fixes
* **Drivers**
  * For TL321X, fixed RF power on sequence error, which may potentially cause abnormal operation of RF module.
* **APP**
  * For TL721X, corrected the ADC reference voltage configuration, changing it from ADC_VREF_GPIO_1P2V to ADC_VREF_1P2V to ensure the module works properly.


### Features
* **Drivers**
  * For TL321X, call the efuse_calib_adc_vref() interface in blc_app_loadCustomizedParameters_normal() to calibrate adc gpio mode and vbat mode.
  * For TL721X, call the otp_calib_adc_vref() interface in blc_app_loadCustomizedParameters_normal() to calibrate adc gpio mode and vbat mode.
  
### BREAKING CHANGES
   * N/A

### Bug Fixes
* **Drivers**
  * 对于 TL321X，修复了RF错误的上电顺序，概率性的导致RF 模块工作状态异常。
* **APP**
  * 对于 TL321X，修正 ADC 参考电压配置，由ADC_VREF_GPIO_1P2V改为ADC_VREF_1P2V，确保模块正常工作。
  
### Features
* **Drivers**
  * 对于 TL321X，在blc_app_loadCustomizedParameters_normal()中调用efuse_calib_adc_vref()接口校准adc gpio模式和vbat模式。
  * 对于 TL721X，在blc_app_loadCustomizedParameters_normal()中调用otp_calib_adc_vref()接口校准adc gpio模式和vbat模式。

### BREAKING CHANGES
   * N/A


## V4.0.4.4(PR)

### Version

* SDK Version: tl_ble_sdk V4.0.4.4
* Chip Version: 
    - TLSR921X(B91):           A2
    - TLSR922X/TLSR952X(B92):  A3/A4
    - TL721X:                  A2/A3
    - TL321X:                  A1/A2
    - TL322X:                  A0
* Hardware EVK Version:
    - TLSR921X:                C1T213A20_V1.3
    - TLSR952X:                C1T266A20_V1.3
    - TL721X:                  C1T315A20_V1.2/AIOT_DK1:ML7218D1/ML7218A
    - TL321X:                  C1T331A20_V1.0/C1T335A20_V1.3
    - TL322X:                  C1T382A20_V1.0
* Platform Version: 
    - TLSR921X:                tl_platform_sdk V3.7.0
    - TLSR922X/TLSR952X:       tl_platform_sdk V3.7.0
    - TL721X:                  tl_platform_sdk V3.7.0
    - TL321X:                  tl_platform_sdk V3.7.0
    - TL322X:                  tl_platform_sdk V3.7.0
* Toolchain Version:
    - TLSR921X:                TL32 ELF MCULIB V5F GCC7.4  (IDE: [TelinkIoTStudio](https://www.telink-semi.com/development-tools))
    - TLSR922X/TLSR952X:       TL32 ELF MCULIB V5F GCC12.2  (IDE: [TelinkIoTStudio](https://www.telink-semi.com/development-tools))
    - TL721X                   TL32 ELF MCULIB V5F GCC12.2  (IDE: [TelinkIoTStudio](https://www.telink-semi.com/development-tools))
    - TL321X:                  TL32 ELF MCULIB V5 GCC12.2  (IDE: [TelinkIoTStudio](https://www.telink-semi.com/development-tools))
    - TL322X:                  TL32 ELF MCULIB V5F GCC12.2  (IDE: [TelinkIoTStudio](https://www.telink-semi.com/development-tools))

### Note

   * For TL321X, deleted the clock frequency starting with `PLL_192M_CCLK_96M` to enhance stability.


### Bug Fixes

* **Drivers**
    * For B91, fixed the bug that the UART finite state machine resets but the software read/write pointer is not cleared.
    * For B91/B92/TL721X/TL321X, fixed `adc_calculate_voltage` interface to prevent abnormal output voltage values when sampling voltage is close to 0V.
    * For B92, restricted `dma_set_address` to the `.ram_code` section to prevent the compiler from optimizing it into flash and causing potential crashes.
    * For TL321X, added watchdog clearing operation in the`flash_erase_sector`  API to prevent the watchdog reseting problem caused by flash erase timeout.
    * For B91/B92, consolidated the `.exec.itable` section and `libgcc.a:save-restore.o` into the `.ram_code` section to eliminate system crash risks.
* **FreeRTOS**
    - Modified the method of setting the interrupt switching flag `g_plic_switch_sp_flag` to use increment and decrement operations, avoiding the undefined behavior potentially caused by the original bit-shifting approach.
* **BLE general function**
    * For TL321X/TL721X, fixed an issue that the `blc_ota_setFirmwareSizeAndBootAddress` API could not configure multi-address boot addresses, enabling OTA updates for firmware images of different sizes.
    * For the COC feature, fixed an array out-of-bounds issue caused by abnormal COC configuration.
    * For the extended advertising, fixed the issue of inaccurate timing of sending `AUX_CONNECT_RSP` when receiving `AUX_CONNECT_REQ`.
    * For the HCI feature, extended the HCI MASK from 32-bit to 64-bit to fix incorrect HCI reporting in certain scenarios.
* **2.4G general function**
    * For TL321X, fixed the issue where the write pointer wptr in the DMA becomes abnormal when PTX uses pipe0 to send packets, resulting in the failure to send data packets in the DMA. 
* **Others**
    * For the tlkapi_debug feature, fixed an issue that `tlkDbgCtl.uartSendIsBusy` flag was not cleared when printing long data via UART interface.
    * For the usb_debug feature, fixed usb print fifo overflow issues.
    * For `blc_app_setDeepsleepRetentionSramSize` API, added inline assembly code to disable compiler address expression relaxation, avoiding compilation errors caused by excessive code expansion and oversized jump addresses in certain scenarios.
    * Fixed alignment issues with RF DMA related struct variables to prevent packet errors caused by non-4-byte-aligned DMA addresses in non-Telink toolchain environments.

### BREAKING CHANGES

* All chips synchronized driver tl_platform_sdk V3.7.0:
    - TL721X: A3 version chips supported ;
    - TL322X: A0 version chips supported.
* Updated the early wake-up time for TLSR921X/TLSR922X/TLSR952X(B92)/TL321X/TL721X in deep retention mode.
* For TLSR922X/TLSR952X(B92), deleted the support for 16M flash.

### Features

* **BLE general function** 
    - The BLE version had been upgraded to Core Specification 6.0.
    - PAwR (Periodic Advertising with Responses) supported:
        * Added `PAwR_adv.h` and `PAwR_sync.h`  in the folder `stack/ble/controller/ll/prdadv`.
    - PAST(Periodic Advertising Sync Transfer) supported:
        * Added the new folder `stack/ble/controller/ll/past`, and added `past.h` in this folder.
    - ESL(Electronic Shelf Label) supported:
        * For TLSR921X/TLSR922X/TLSR952X(B92)/TL321X/TL721X, added eslp_ap_demo project for Access Point feature;
        * For TL321X/TL721X, added eslp_esl_demo project for Electronic Shelf Label feature;
        * Added new folder `stack/ble/profile`.
    - For all chips, added ble_controller project, and added new folder `vendor/ble_controller`.
    - For `acl_central_demo`, added LED and matrix keyboard configuration support.
    - For TL322X:
        * Added support for suspend sleep mode; support for deep retention sleep mode will be added in a later version;
        * Support for flash write protection function will be added in a later version;
        * Currently added support for acl_peripheral_demo/acl_central_demo/acl_connection_demo/ble_controller.
* **2.4G general function** 
    - N/A
* **Drivers**
    * For B92:
        * When GPIO voltage is configured to 3.3V, PB0 could be used as ADC detection pin.
    
    * For TL321X：
        * Added API `wd_32k_feed` for Watchdog feeding function;
        * Added API `wd_32k_get_count_ms` for getting the current 32K watchdog counter value(millisecond precision);
        * Added API `adc_get_sample_status_dma` for getting the ADC DMA sampling status.
    * For TLSR921X/TLSR922X/TLSR952X(B92)/TL321X/TL721X, synchronized application folder from tl_platform_sdk to add support for usb cdc and other functionalities.
    * For all chips, added `.rf_certification_cfg` section, this section is required for Driver EMI BQB testing and has no impact on `tl_ble_sdk`.
* **Others**
    * Provided a Handbook based on tl_ble_sdk V4.0.4.4 (currently available in Simplified Chinese only).
    * Added `.gitattributes` file to enforce `lf` line endings for shell scripts.
    * Added `.gitignore` file to prevent Git from tracking build artifacts.


### Refactoring

   * Updated the implementation of the `blc_get_sdk_version` API, changed the output from a numeric/character combination to a string format, and added protocol stack build version info to improve version management and issue tracking.
   * Replaced the post-build script with `tl_check_fw.sh` and added a warning for firmware sizes exceeding 256 KB.
   * Renamed APIs to comply with BLE Specification naming conventions, while maintaining backward compatibility:
     * Renamed API `blc_ll_getCurrentMasterRoleNumber` to `blc_ll_getCurrentCentralRoleNumber`;
     * Renamed API `blc_ll_getCurrentSlaveRoleNumber` to `blc_ll_getCurrentPeripheralRoleNumber`.

* Removed reference code calling `emi_test` in `acl_peripheral_demo`, decoupled `emi_test` from `acl_peripheral_demo`, while retaining a reference implementation of `emi_test` under `vendor/common`.

### Performance Improvements

   * N/A

### Known issues

* N/A

### CodeSize

* TLSR921X
    - Compiling acl_central_demo
        - Flash bin size: 105.52 KB
        - IRAM size: 54.29 KB
        - DRAM size: 0.65 KB
    - Compiling acl_connection_demo
        - Flash bin size: 128.02 KB
        - IRAM size: 65.77 KB
        - DRAM size: 0.84 KB
    - Compiling acl_peripheral_demo
        - Flash bin size: 110.83 KB
        - IRAM size: 55.75 KB
        - DRAM size: 0.77 KB
    - Compiling eslp_ap_dmeo
        - Flash bin size: 249.14 KB
        - IRAM size: 93.77 KB
        - DRAM size: 24.02 KB
* TLSR922X/TLSR952X
    - Compiling acl_central_demo
        - Flash bin size: 105.42 KB
        - IRAM size: 54.60 KB
        - DRAM size: 0.70 KB
    - Compiling acl_connection_demo
        - Flash bin size: 127.75 KB
        - IRAM size: 65.86 KB
        - DRAM size: 0.94 KB
    - Compiling acl_peripheral_demo
        - Flash bin size: 111.58 KB
        - IRAM size: 56.10 KB
        - DRAM size: 0.87 KB
    - Compiling eslp_ap_dmeo
        - Flash bin size: 257.20 KB
        - IRAM size: 93.61 KB
        - DRAM size: 24.09 KB

* TL721X
    - Compiling acl_central_demo
        - Flash bin size:  110.21 KB
        - IRAM size: 58.21 KB
        - DRAM size: 0.74 KB
    - Compiling acl_connection_demo
        - Flash bin size: 132.50 KB
        - IRAM size: 69.47 KB
        - DRAM size: 0.98 KB
    - Compiling acl_peripheral_demo
        - Flash bin size: 116.53 KB
        - IRAM size: 59.96 KB
        - DRAM size: 0.91 KB
    - Compiling eslp_ap_dmeo
        - Flash bin size: 268.19 KB
        - IRAM size: 98.24 KB
        - DRAM size: 21.62 KB
    - Compiling eslp_esl_dmeo
        - Flash bin size: 202.61 KB
        - IRAM size: 69.58 KB
        - DRAM size: 5.36 KB

* TL321X
    - Compiling acl_central_demo
        - Flash bin size: 112.09 KB
        - IRAM size: 55.34 KB
        - DRAM size: 0.71 KB
    - Compiling acl_connection_demo
        - Flash bin size: 134.40 KB
        - IRAM size: 66.61 KB
        - DRAM size: 0.96 KB
    - Compiling acl_peripheral_demo
        - Flash bin size:  118.11 KB
        - IRAM size: 56.84 KB
        - DRAM size: 0.88 KB
    - Compiling eslp_ap_dmeo
        - Flash bin size: 258.20 KB
        - IRAM size: 90.60 KB
        - DRAM size: 21.60 KB
    - Compiling eslp_esl_dmeo
        - Flash bin size: 204.21 KB
        - IRAM size: 66.45 KB
        - DRAM size: 5.33 KB
* TL321X
    - Compiling acl_central_demo
        - Flash bin size: 106.20 KB
        - IRAM size: 56.49 KB
        - DRAM size: 0.71 KB
    - Compiling acl_connection_demo
        - Flash bin size: 128.51 KB
        - IRAM size: 67.80 KB
        - DRAM size: 0.95 KB
    - Compiling acl_peripheral_demo
        - Flash bin size:  112.54 KB
        - IRAM size: 58.28 KB
        - DRAM size: 0.88 KB

**Note:** The above IRAM usage includes the FIFO buffer required for the debug logging functionality. Disabling the `TLKAPI_DEBUG_ENABLE` macro can save approximately 4.76 KB of IRAM.



### 版本

* SDK Version: tl_ble_sdk V4.0.4.4
* Chip Version: 
    - TLSR921X(B91):           A2
    - TLSR922X/TLSR952X(B92):  A3/A4
    - TL721X:                  A2/A3
    - TL321X:                  A1/A2
    - TL322X:                  A0
* Hardware EVK Version:
    - TLSR921X:                C1T213A20_V1.3
    - TLSR952X:                C1T266A20_V1.3
    - TL721X:                  C1T315A20_V1.2/AIOT_DK1:ML7218D1/ML7218A
    - TL321X:                  C1T331A20_V1.0/C1T335A20_V1.3
    - TL322X:                  C1T382A20_V1.0
* Platform Version: 
    - TLSR921X:                tl_platform_sdk V3.7.0
    - TLSR922X/TLSR952X:       tl_platform_sdk V3.7.0
    - TL721X:                  tl_platform_sdk V3.7.0
    - TL321X:                  tl_platform_sdk V3.7.0
    - TL322X:                  tl_platform_sdk V3.7.0
* Toolchain Version:
    - TLSR921X:                TL32 ELF MCULIB V5F GCC7.4  (IDE: [TelinkIoTStudio](https://www.telink-semi.com/development-tools))
    - TLSR922X/TLSR952X:       TL32 ELF MCULIB V5F GCC12.2  (IDE: [TelinkIoTStudio](https://www.telink-semi.com/development-tools))
    - TL721X                   TL32 ELF MCULIB V5F GCC12.2  (IDE: [TelinkIoTStudio](https://www.telink-semi.com/development-tools))
    - TL321X:                  TL32 ELF MCULIB V5 GCC12.2  (IDE: [TelinkIoTStudio](https://www.telink-semi.com/development-tools))
    - TL322X:                  TL32 ELF MCULIB V5F GCC12.2  (IDE: [TelinkIoTStudio](https://www.telink-semi.com/development-tools))


### Note

   * 对于 TL321X，删除 `PLL_192M_CCLK_96M` 开头的时钟频率以提高稳定性。

### Bug Fixes

* **Drivers**
    * 对于 B91，修复 UART 有限状态机复位但是软件读写指针没清零的 bug。
    * 对于 B91/B92/TL721X/TL321X，修复了 `adc_calculate_voltage` 接口，防止采样电压接近 0V 时，输出电压值异常。
    * 对于 B92，限制 `dma_set_address` 存放在 `.ram_code` 段内，避免编译器将该函数优化到 flash 中，解决死机风险。
    * 对于 TL321X，在 API `flash_erase_sector` 中添加清看门狗操作，避免 Flash 擦除超时，导致的看门狗复位问题。
    * 对于 B91/B92，将 `.exec.itable` 段和 `libgcc.a: save-restore.o` 统一放入 `.ram_code` 段，解决死机风险。
* **FreeRTOS**
    - 修改中断切换标志 `g_plic_switch_sp_flag` 的设置方式为自增和自减，避免原本移位方式隐藏的未定义行为。
* **BLE 通用功能**
    * 对于 TL321X/TL721X，修复 `blc_ota_setFirmwareSizeAndBootAddress` API 无法设置多地址启动地址的问题，以支持不同大小固件的 OTA 升级功能。
    * 对于 COC 功能，修复 COC 异常设置导致的数组越界问题。
    * 对于扩展广播功能，修复收到 `AUX_CONNECT_REQ` 包时，发送 `AUX_CONNECT_RSP` 的时间点不准确的问题。
    * 对于 HCI 功能，扩展 HCI MASK 从 32 位到 64 位，修复部分情况下 HCI 上报错误的问题。
* **2.4G通用功能**
    * 对于 TL321X，修复 PTX 使用 pipe0 发包时，写指针 wptr 异常导致的 dma 中的数据包没有发送的问题。
* **Others**
    * 对于 tlkapi_debug 功能，修复 uart 接口打印长数据时，`tlkDbgCtl.uartSendIsBusy` 标志位不清零的问题。
    * 对于 usb_debug 功能，修复 usb 打印溢出问题。
    * 对于 API `blc_app_setDeepsleepRetentionSramSize`，添加内联汇编代码，禁止编译器进行地址表达式松弛化处理，避免部分情况下，代码量增加和代码跳转地址过大导致的编译错误问题。
    * 修复与 RF DMA 相关的结构体变量的对齐问题，避免非 Telink 工具链环境下，可能出现的 DMA 地址非四字节对齐导致的错包问题。

### BREAKING CHANGES

* 所有芯片同步 driver tl_platform_sdk V3.7.0：
    * 添加对 TL721X A3 版本芯片的支持；
    * 添加对 TL322X A0 版本芯片的支持。
* 更新 TLSR921X/TLSR922X/TLSR952X(B92)/TL321X/TL721X deep retention 模式下的提前唤醒时间。
* 对于 TLSR922X/TLSR952X(B92)：删除对 16 M Flash 的支持。

### Features

* **BLE通用功能**
  - BLE 版本已升级至Core 6.0 版本。
  - BLE 支持 PAwR（Periodic Advertising with Responses） 功能：
      * 在文件夹 `stack/ble/controller/ll/prdadv` 中添加了 `PAwR_adv.h` 和 `PAwR_sync.h` 文件。
  - BLE 支持 PAST（Periodic Advertising Sync Transfer）功能：
    * 新增文件夹 `stack/ble/controller/ll/past`，并在该文件夹内添加了 `past.h`。
  - 新增对 ESL 功能的支持：
      * 对于 TLSR921X/TLSR922X/TLSR952X(B92)/TL321X/TL721X，新增 eslp_ap_demo 工程，提供对 Access Point 功能的支持；
      * 对于 TL321X/TL721X，新增 eslp_esl_demo 工程，提供对 Electronic Shelf Label 功能的支持；
      * 新增文件夹 `stack/ble/profile` 文件夹。
  - 对于所有芯片，新增 ble_controller 工程，新增 `vendor/ble_controller` 文件夹。
  - 对于 `acl_central_demo`，添加 LED 和矩阵键盘的配置支持。
  - 对于 TL322X：
      * 支持 Suspend 的睡眠模式，Deep Retention 的睡眠模式会在后续版本中支持；
      * Flash 写保护功能会在后续版本中支持；
      * 当前支持 acl_peripheral_demo/acl_central_demo/acl_connection_demo/ble_controller。
* **2.4G通用功能**
  - N/A
* **Drivers**
    * 对于 B92：
        * GPIO 电压配置为 3.3V 时，支持 PB0 作为 ADC 检测引脚的功能。

    * 对于 TL321X：
        * 添加 API `wd_32k_feed`，用于喂狗功能；
        * 添加 API `wd_32k_get_count_ms`，用于获取当前32k watchdog 计数值（毫秒精度）；
        * 添加 API `adc_get_sample_status_dma`，用于获取 adc dma 采样状态。
    * 对于 TLSR921X/TLSR922X/TLSR952X(B92)/TL321X/TL721X，同步 applicaiton 文件夹，提供对 usb cdc 等功能的支持。

    * 对于所有芯片，添加 `.rf_certification_cfg` 段，该段需配合 Driver EMI BQB 测试使用，在 tl_ble_sdk 上无影响。
* **Others**
    * 提供基于 tl_ble_sdk V4.0.4.4 版本的 Handbook（当前仅支持简体中文版本）。
    * 新增 `.gitattributes` 文件，限定 shell 脚本的换行格式为 `lf`。
    * 新增 `.gitignore`  文件，阻止 git 对编译文件的跟踪。


### Refactoring

* 修改 API `blc_get_sdk_version` 实现，将输出内容从数字、字符组合变为字符串格式，增加协议栈库的生成版本信息，提升版本管理和问题反馈处理效率。
* 修改 post build 的脚本为 `tl_check_fw.sh`，并新增固件超过 256 KB 的提示。
* 修改 API 的名称以符合 BLE Specification 规范，并保持对旧 API 的兼容性：
     * 修改 API `blc_ll_getCurrentMasterRoleNumber` 为 `blc_ll_getCurrentCentralRoleNumber`；
     * 修改 API `blc_ll_getCurrentSlaveRoleNumber` 为 `blc_ll_getCurrentPeripheralRoleNumber`。
* 删除在 `acl_peripheral_demo` 中调用 `emi_test` 的参考代码，将 `emi_test` 和 `acl_peripheral_demo` 功能解耦，保留在 `vendor/common` 下的 `emi_test` 参考实现。


### Performance Improvements

   * N/A

### Known issues

* N/A

### CodeSize

* TLSR921X
    - Compiling acl_central_demo
        - Flash bin size: 105.52 KB
        - IRAM size: 54.29 KB
        - DRAM size: 0.65 KB
    - Compiling acl_connection_demo
        - Flash bin size: 128.02 KB
        - IRAM size: 65.77 KB
        - DRAM size: 0.84 KB
    - Compiling acl_peripheral_demo
        - Flash bin size: 110.83 KB
        - IRAM size: 55.75 KB
        - DRAM size: 0.77 KB
    - Compiling eslp_ap_dmeo
        - Flash bin size: 249.14 KB
        - IRAM size: 93.77 KB
        - DRAM size: 24.02 KB
* TLSR922X/TLSR952X
    - Compiling acl_central_demo
        - Flash bin size: 105.42 KB
        - IRAM size: 54.60 KB
        - DRAM size: 0.70 KB
    - Compiling acl_connection_demo
        - Flash bin size: 127.75 KB
        - IRAM size: 65.86 KB
        - DRAM size: 0.94 KB
    - Compiling acl_peripheral_demo
        - Flash bin size: 111.58 KB
        - IRAM size: 56.10 KB
        - DRAM size: 0.87 KB
    - Compiling eslp_ap_dmeo
        - Flash bin size: 257.20 KB
        - IRAM size: 93.61 KB
        - DRAM size: 24.09 KB

* TL721X
    - Compiling acl_central_demo
        - Flash bin size:  110.21 KB
        - IRAM size: 58.21 KB
        - DRAM size: 0.74 KB
    - Compiling acl_connection_demo
        - Flash bin size: 132.50 KB
        - IRAM size: 69.47 KB
        - DRAM size: 0.98 KB
    - Compiling acl_peripheral_demo
        - Flash bin size: 116.53 KB
        - IRAM size: 59.96 KB
        - DRAM size: 0.91 KB
    - Compiling eslp_ap_dmeo
        - Flash bin size: 268.19 KB
        - IRAM size: 98.24 KB
        - DRAM size: 21.62 KB
    - Compiling eslp_esl_dmeo
        - Flash bin size: 202.61 KB
        - IRAM size: 69.58 KB
        - DRAM size: 5.36 KB

* TL321X
    - Compiling acl_central_demo
        - Flash bin size: 112.09 KB
        - IRAM size: 55.34 KB
        - DRAM size: 0.71 KB
    - Compiling acl_connection_demo
        - Flash bin size: 134.40 KB
        - IRAM size: 66.61 KB
        - DRAM size: 0.96 KB
    - Compiling acl_peripheral_demo
        - Flash bin size:  118.11 KB
        - IRAM size: 56.84 KB
        - DRAM size: 0.88 KB
    - Compiling eslp_ap_dmeo
        - Flash bin size: 258.20 KB
        - IRAM size: 90.60 KB
        - DRAM size: 21.60 KB
    - Compiling eslp_esl_dmeo
        - Flash bin size: 204.21 KB
        - IRAM size: 66.45 KB
        - DRAM size: 5.33 KB
* TL321X
    - Compiling acl_central_demo
        - Flash bin size: 106.20 KB
        - IRAM size: 56.49 KB
        - DRAM size: 0.71 KB
    - Compiling acl_connection_demo
        - Flash bin size: 128.51 KB
        - IRAM size: 67.80 KB
        - DRAM size: 0.95 KB
    - Compiling acl_peripheral_demo
        - Flash bin size:  112.54 KB
        - IRAM size: 58.28 KB
        - DRAM size: 0.88 KB

**Note:** 上述 IRAM 统计包含了打印功能所需的 FIFO，关闭宏 TLKAPI_DEBUG_ENABLE，可以节省约 4.76 KB 的 IRAM。




## V4.0.4.3_Patch_0001(PR)

### Bug Fixes

- calibration
    - (B91/B92/TL721X/TL321X) Modified the logic for determining frequency_offset_value in the user_calib_freq_offset interface to resolve the issue where the frequency offset calibration value for the chip fixture was not taking effect.

### Features

- Drivers
    - N/A

### BREAKING CHANGES

- calibration
    - (B91/B92/TL721X/TL321X) Modified the internal logic for determining frequency_offset_value in the user_calib_freq_offset interface to align with the write logic of the chip fixture's frequency offset calibration value. This ensures proper utilization of the RF frequency offset calibration value stored in the flash, preventing potential RF frequency offset issues. This fix impacts all SDKs and applications that require RF frequency offset calibration.

### Bug Fixes

- calibration
    - (B91/B92/TL721X/TL321X) 修改了user_calib_freq_offset接口中判断frequency_offset_value的逻辑，以解决芯片夹具频偏校准值不生效的问题。

### Features

- Drivers
    - N/A

### BREAKING CHANGES

- calibration
    - (B91/B92/TL721X/TL321X)修改了user_calib_freq_offset接口中判断frequency_offset_value的内部逻辑，使其保证和芯片夹具频偏校准值写入逻辑一致。保证flash中的RF频偏校准值被正确使用，避免产生RF频偏问题。此问题影响所有需要进行RF频偏校准的SDK和应用。

## V4.0.4.3(PR)

### Version

* SDK Version: tl_ble_sdk V4.0.4.3

* Chip Version: 
  
    * TLSR921X(B91):           A2
    * TLSR922X/TLSR952X(B92):  A3/A4
    * TL721X:                  A2
    * TL321X:                  A1/A2
    
* Hardware EVK Version:
    - TLSR921X:                C1T213A20_V1.3
    - TLSR952X:                C1T266A20_V1.3
    - TL721X:                  C1T315A20_V1.2
    - TL321X:                  C1T331A20_V1.0/C1T335A20_V1.3

* Platform Version: 
    - TLSR921X:                tl_platform_sdk V3.4.0
    - TLSR922X/TLSR952X:       tl_platform_sdk V3.4.0
    - TL721X:                  tl_platform_sdk V3.4.0
    - TL321X:                  tl_platform_sdk V3.4.0

* Toolchain Version:
    - TLSR921X:                TL32 ELF MCULIB V5F GCC7.4  (IDE: [TelinkIoTStudio_V2024.8](https://wiki.telink-semi.cn/wiki/IDE-and-Tools/Telink_IoT_Studio/))
    - TLSR922X/TLSR952X:       TL32 ELF MCULIB V5F GCC12.2 (IDE: [TelinkIoTStudio_V2024.8](https://wiki.telink-semi.cn/wiki/IDE-and-Tools/Telink_IoT_Studio/))
    - TL721X                   TL32 ELF MCULIB V5F GCC12.2 (IDE: [TelinkIoTStudio_V2024.8](https://wiki.telink-semi.cn/wiki/IDE-and-Tools/Telink_IoT_Studio/))
    - TL321X:                  TL32 ELF MCULIB V5 GCC12.2 (IDE: [TelinkIoTStudio_V2024.8](https://wiki.telink-semi.cn/wiki/IDE-and-Tools/Telink_IoT_Studio/))

    

### Note
   * N/A

### Bug Fixes
* **Others**
    - The starting address of `_ISTART` for all chips had been adjusted from `0x22` to `0x24` to ensure that the corresponding SRAM region is not impacted by internal operations of the protocol stack.

### BREAKING CHANGES
* All chips synchronized driver tl_platform_sdk V3.4.0:
    - TL321X: supported the A2 version chips.
* Updated the early wake-up time for TLSR921X/TLSR922X/TLSR952X(B92)/TL321X in deep retention mode.
* Code style clean.

### Features
* **BLE general function** 
    - For TLSR922X/TLSR952X (B92)/TL321X/TL721X, added the API `pm_update_status_info` to update the chip wake-up status.
    - Added the API `blc_app_setDeepsleepRetentionSramSize` for automatically configuring the Deep retention Size of the chip.
* **2.4G general function** 
    - For TL321X, added the 2p4g_feature_test, 2p4g_gen_fsk, 2p4g_tpll three demos.
    - 2.4G demo supported TL321X only.
* **Others**
    - N/A

### Refactoring
   * N/A

### Performance Improvements
   * N/A

### Known issues
* N/A

### CodeSize

* TLSR921X
    - Compiling acl_central_demo
        - Flash bin size: 104.0 KB
        - IRAM size: 48.9 KB
        - DRAM size: 0.27 KB
    - Compiling acl_connection_demo
        - Flash bin size: 132.0 KB
        - IRAM size: 63.5 KB
        - DRAM size: 4.3 KB
    - Compiling acl_peripheral_demo
        - Flash bin size: 111.0 KB
        - IRAM size: 50.5 KB
        - DRAM size: 0.26 KB

* TLSR922X/TLSR952X
    - Compiling acl_central_demo
        - Flash bin size: 104.0 KB
        - IRAM size: 54.3 KB
        - DRAM size: 0 KB
    - Compiling acl_connection_demo
        - Flash bin size: 128.0 KB
        - IRAM size: 65.6 KB
        - DRAM size: 0 KB
    - Compiling acl_peripheral_demo
        - Flash bin size: 113.0 KB
        - IRAM size: 56.3 KB
        - DRAM size: 0 KB

* TL721X
    - Compiling acl_central_demo
        - Flash bin size:  109.90 KB
        - IRAM size: 56.39 KB
        - DRAM size: 0.34 KB
    - Compiling acl_connection_demo
        - Flash bin size: 134.20 KB
        - IRAM size: 67.64 KB
        - DRAM size: 0.67 KB
    - Compiling acl_peripheral_demo
        - Flash bin size: 118.0 KB
        - IRAM size: 58.14 KB
        - DRAM size: 0.60 KB

* TL321X
    - Compiling acl_central_demo
        - Flash bin size: 111.50 KB
        - IRAM size: 53.89 KB
        - DRAM size: 0.31 KB
    - Compiling acl_connection_demo
        - Flash bin size: 135.70 KB
        - IRAM size: 65.14 KB
        - DRAM size: 4.67 KB
    - Compiling acl_peripheral_demo
        - Flash bin size:  119.5 KB
        - IRAM size: 55.64 KB
        - DRAM size: 0.57 KB




### 版本

* SDK Version: tl_ble_sdk V4.0.4.3

* Chip Version: 
    - TLSR921X(B91):           A2
    - TLSR922X/TLSR952X(B92):  A3/A4
    - TL721X:                  A2
    - TL321X:                  A1/A2

* Hardware EVK Version:
    - TLSR921X:                C1T213A20_V1.3
    - TLSR952X:                C1T266A20_V1.3
    - TL721X:                  C1T315A20_V1.2
    - TL321X:                  C1T331A20_V1.0/C1T335A20_V1.3

* Platform Version: 
    - TLSR921X:                tl_platform_sdk V3.4.0
    - TLSR922X/TLSR952X:       tl_platform_sdk V3.4.0
    - TL721X:                  tl_platform_sdk V3.4.0
    - TL321X:                  tl_platform_sdk V3.4.0

* Toolchain Version:
    - TLSR921X:                TL32 ELF MCULIB V5F GCC7.4  (IDE: [TelinkIoTStudio_V2024.8](https://wiki.telink-semi.cn/wiki/IDE-and-Tools/Telink_IoT_Studio/))
    - TLSR922X/TLSR952X:       TL32 ELF MCULIB V5F GCC12.2 (IDE: [TelinkIoTStudio_V2024.8](https://wiki.telink-semi.cn/wiki/IDE-and-Tools/Telink_IoT_Studio/))
    - TL721X                   TL32 ELF MCULIB V5F GCC12.2 (IDE: [TelinkIoTStudio_V2024.8](https://wiki.telink-semi.cn/wiki/IDE-and-Tools/Telink_IoT_Studio/))
    - TL321X:                  TL32 ELF MCULIB V5 GCC12.2 (IDE: [TelinkIoTStudio_V2024.8](https://wiki.telink-semi.cn/wiki/IDE-and-Tools/Telink_IoT_Studio/))


### Note
   * N/A

### Bug Fixes
* **Others**
    - 所有芯片的`_ISTART` 的起始地址从 0x22 修改到 0x24 地址，避免相应 SRAM 区域被协议栈内部操作覆盖。

### BREAKING CHANGES 	
* 所有芯片同步 driver tl_platform_sdk V3.4.0：
    - TL321X 支持 A2 芯片。
* 更新 TLSR921X/TLSR922X/TLSR952X(B92)/TL321X deep retention 模式下的提前唤醒时间。
* 代码风格清理。

### Features
* **BLE通用功能**
    - 对于 TLSR922X/TLSR952X(B92)/TL321X/TL721X 调用 API `pm_update_status_info` 用于更新芯片唤醒的状态。
    - 添加 API `blc_app_setDeepsleepRetentionSramSize` 用于自动配置芯片的 Deep retention Size。
* **2.4G通用功能**
    - 对于 TL321X添加了2p4g_feature_test、2p4g_gen_fsk、2p4g_tpll 3个demo。
    - 2.4G demo 只支持 TL321X。
* **Drivers**
    * N/A


### Refactoring
   * N/A

### Performance Improvements
   * N/A

### Known issues
* N/A

### CodeSize
* TLSR921X
    - Compiling acl_central_demo
        - Flash bin size: 104.0 KB
        - IRAM size: 48.9 KB
        - DRAM size: 0.27 KB
    - Compiling acl_connection_demo
        - Flash bin size: 132.0 KB
        - IRAM size: 63.5 KB
        - DRAM size: 4.3 KB
    - Compiling acl_peripheral_demo
        - Flash bin size: 111.0 KB
        - IRAM size: 50.5 KB
        - DRAM size: 0.26 KB

* TLSR922X/TLSR952X
    - Compiling acl_central_demo
        - Flash bin size: 104.0 KB
        - IRAM size: 54.3 KB
        - DRAM size: 0 KB
    - Compiling acl_connection_demo
        - Flash bin size: 128.0 KB
        - IRAM size: 65.6 KB
        - DRAM size: 0 KB
    - Compiling acl_peripheral_demo
        - Flash bin size: 113.0 KB
        - IRAM size: 56.3 KB
        - DRAM size: 0 KB

* TL721X
    - Compiling acl_central_demo
        - Flash bin size:  109.90 KB
        - IRAM size: 56.39 KB
        - DRAM size: 0.34 KB
    - Compiling acl_connection_demo
        - Flash bin size: 134.20 KB
        - IRAM size: 67.64 KB
        - DRAM size: 0.67 KB
    - Compiling acl_peripheral_demo
        - Flash bin size: 118.0 KB
        - IRAM size: 58.14 KB
        - DRAM size: 0.60 KB

* TL321X
    - Compiling acl_central_demo
        - Flash bin size: 111.50 KB
        - IRAM size: 53.89 KB
        - DRAM size: 0.31 KB
    - Compiling acl_connection_demo
        - Flash bin size: 135.70 KB
        - IRAM size: 65.14 KB
        - DRAM size: 4.67 KB
    - Compiling acl_peripheral_demo
        - Flash bin size:  119.5 KB
        - IRAM size: 55.64 KB
        - DRAM size: 0.57 KB



## V4.0.4.2_Patch_0002(PR)

### Bug Fixes

- PM
    - For TL321X, solved the issue that the possible SRAM loss when entering suspend sleep after deep retention sleep.

### Features

- Drivers
    - For TL721X, updated the Driver version to V3.3.0:
        - provided the support for the A2 version chip **(not compatible with the A1 version chip)**;
        - updated the default ADC pin of the battery voltage detection to  `ADC_GPIO_PB1`;
        - updated the ADC configuration in  `battery_check.c`.

### BREAKING CHANGES

- N/A

### Bug Fixes

- PM
    - 对于TL321X，解决了 deep retention 模式唤醒后，再进入 suspend 模式可能造成的 RAM 未完全保持问题。

### Features

- Drivers
    - 对于 TL721X，更新 Driver 版本为 V3.3.0：
        - 提供对 A2 版本芯片的支持，**不兼容对 A1 版本芯片的支持**；
        - 更新默认电池电压检测的 ADC 引脚为 `ADC_GPIO_PB1`；
        - 更新 `battery_check.c` 中 ADC 的配置。

### BREAKING CHANGES

- N/A

## V4.0.4.2_Patch_0001(PR)

### Bug Fixes

- PM
    - For TL321X and TL721X, added calling `pm_update_status_info` in `blc_app_system_init`, and fixed the API `pm_get_mcu_status` returning MCU status error.
    - Fixed insufficient space for default DeepRetention when enabling FreeRtos and deepRetention.
- Others
    - For TL321X, changed Debug IO GPIO_PC2 to GPIO_PC0 to avoid conflict with the external 32K crystal IO.
    - Fixed the exception of the Debug Log sequence number printing function.
    - For TL321X and TL721X, fixed the voltage value offset problem for the battery check Vbat mode.

### Features

- BLE general function
    - Added API `blc_ll_getBleCurrentState` to get the current state of BLE Stack.
    - Added API `blc_ll_isRfBusy` to determine if RF is BUSY.
    - For TLSR921X(B91), added API `pm_get_mcu_status`to get MCU status.
- Others
    - Added error code `SYS_CLK_ERR_NOT_SUPPORTED` to check whether the system clock settings match.
    - For TLSR921X(B91) and TLSR922X/TLSR952X(B92), support obtaining DCOC calibration value through Flash.

### BREAKING CHANGES

- Added `__attribute__((packed))` for struct type variables.

### Bug Fixes

- PM
    - 对于TL321X和TL721X，在`blc_app_system_init`增加调用`pm_update_status_info`，修复API`pm_get_mcu_status`返回状态错误。
    - 修复使能FreeRtos和deepRetention时默认DeepRetention空间不足问题。
- Others
    - 对于TL321X，修改Debug IO GPIO_PC2为GPIO_PC0，避免与32K外部晶振IO冲突。
    - 修复Debug Log序号打印功能异常。
    - 对于TL321X和TL721X，修复低压检测Vbat模式下电压值偏移问题。

### Features

- BLE通用功能
    - 添加API`blc_ll_getBleCurrentState`获取当前BLE Stack状态。
    - 添加API`blc_ll_isRfBusy`判断当前RF是否BUSY。
    - 对于TLSR921X(B91)添加API`pm_get_mcu_status`获取MCU状态。
- Others
    - 添加错误码`SYS_CLK_ERR_NOT_SUPPORTED`检查系统时钟设置是否匹配。
    - 对于TLSR921X(B91)和TLSR922X/TLSR952X(B92)，支持通过Flash获取DCOC校准值。

### BREAKING CHANGES

- 对于struct类型变量添加`__attribute__((packed))`关键字。



## V4.0.4.2(PR)

### Version

* SDK Version: tl_ble_sdk V4.0.4.2
* Chip Version: 
  - TLSR921X(B91): 			A2
  - TLSR922X/TLSR952X(B92): A3/A4
  - TL721X: 				A1
  - TL321X:					A1
* Hardware EVK Version:
  - TLSR921X: 				C1T213A20_V1.3
  - TLSR952X: 				C1T266A20_V1.3
  - TL721X: 				C1T314A20_V1.0
  - TL321X: 				C1T331A20_V1.0/C1T335A20_V1.0
* Platform Version: 
  - TLSR921X: 				tl_platform_sdk V3.2.0
  - TLSR922X/TLSR952X: 		tl_platform_sdk V3.2.0
  - TL721X: 				tl_platform_sdk V3.2.0
  - TL321X: 				tl_platform_sdk V3.2.0
* Toolchain Version:
  - TLSR921X: 				TL32 ELF MCULIB V5F GCC7.4  (IDE: [TelinkIoTStudio_V2024.8](https://wiki.telink-semi.cn/wiki/IDE-and-Tools/Telink_IoT_Studio/))
  - TLSR922X/TLSR952X: 		TL32 ELF MCULIB V5F GCC12.2 (IDE: [TelinkIoTStudio_V2024.8](https://wiki.telink-semi.cn/wiki/IDE-and-Tools/Telink_IoT_Studio/))
  - TL721X: 				TL32 ELF MCULIB V5F GCC12.2 (IDE: [TelinkIoTStudio_V2024.8](https://wiki.telink-semi.cn/wiki/IDE-and-Tools/Telink_IoT_Studio/))
  - TL321X: 				TL32 ELF MCULIB V5  GCC12.2 (IDE: [TelinkIoTStudio_V2024.8](https://wiki.telink-semi.cn/wiki/IDE-and-Tools/Telink_IoT_Studio/)) 

  
### Note
   * TL721X and TL321X do not support USB debug logs currently.

### Bug Fixes
* **BLE general function**
  - Fixed the auxiliary packet broadcast exception in the feature_ext_adv demo. 

### BREAKING CHANGES 	
   * All chips synchronized driver tl_platform_sdk V3.2.0.
     - Supported TL321X A1 chip.
     - Added TL321X power mode parameters in API `sys_init`, and the default configuration is `DCDC_1P25_LDO_1P8`.
     - Updated the early wake-up time in deep retention mode of TLSR921X/TL721X/TL321X.
   * Changed TL721X default clock setting to `PLL_240M_CCLK_48M_HCLK_48M_PCLK_48M_MSPI_48M`.
   * Changed TL321X default clock setting to `PLL_192M_CCLK_48M_HCLK_48M_PCLK_48M_MSPI_48M`.
   * Changed TL721X default power mode to `DCDC_0P94_DCDC_1P8`.

### Features
* **Chip**
  - Added support for the TLSR922X/TLSR952X chip.
* **BLE general function** 
  - Added Secure Boot for TL321X/TL721X.
* **Others**
  - Added sequence number of the debug log to verify its integrity and consistency.
  - Deleted the invalid API `blc_flash_set_4line_read_write`. Users can call the API `ble_flash_4line_enable` to enable Flash 4-line mode.

### Refactoring
   * N/A

### Performance Improvements
   * N/A

### Known issues
   * Currently, for demos that support the ACL central role, the **CCLK** must be at least 48MHz. These demos will support 32MHz **CCLK** in the next version.

### CodeSize
* TLSR921X
  - Compiling acl_central_demo
    - Flash bin size: 99.54 KB
	- IRAM size: 52.51 KB
	- DRAM size: 0.26 KB
  - Compiling acl_connection_demo
	- Flash bin size: 126.82 KB 
	- IRAM size: 66.94 KB
	- DRAM size: 4.35 KB
  - Compiling acl_peripheral_demo
	- Flash bin size: 105.96 KB
	- IRAM size: 54.01 KB
	- DRAM size: 0.24 KB

* TLSR922X/TLSR952X
  - Compiling acl_central_demo
	- Flash bin size: 102.61 KB
	- IRAM size: 55.69 KB
	- DRAM size: 0.31 KB
  - Compiling acl_connection_demo
	- Flash bin size: 129.96 KB 
	- IRAM size: 70.06 KB
	- DRAM size: 4.65 KB
  - Compiling acl_peripheral_demo
	- Flash bin size: 114.25 KB
	- IRAM size: 60.81 KB
	- DRAM size: 3.07 KB 

* TL721X
  - Compiling acl_central_demo
    - Flash bin size: 97.9KB
    - IRAM size: 53.2KB
	- DRAM size: 0KB
  - Compiling acl_connection_demo
    - Flash bin size:  124.8KB
    - IRAM size: 71.6KB
	- DRAM size: 0KB
  - Compiling acl_peripheral_demo
    - Flash bin size:  108.6KB
    - IRAM size: 60.6KB
	- DRAM size: 0KB

* TL321X
  - Compiling acl_central_demo
    - Flash bin size: 97.3KB
    - IRAM size: 51.5KB
    - DRAM size: 0.3KB
  - Compiling acl_connection_demo
    - Flash bin size:124.1KB 
    - IRAM size: 65.3KB
    - DRAM size: 4.6KB
  - Compiling acl_peripheral_demo
    - Flash bin size: 107.8KB
    - IRAM size: 55.8KB
    - DRAM size: 3.0KB 






### 版本

* SDK Version: tl_ble_sdk V4.0.4.2
* Chip Version: 
  - TLSR921X(B91): 			A2
  - TLSR922X/TLSR952X(B92): A3/A4
  - TL721X: 				A1
  - TL321X:					A1
* Hardware EVK Version:
  - TLSR921X: 				C1T213A20_V1.3
  - TLSR952X: 				C1T266A20_V1.3
  - TL721X: 				C1T314A20_V1.0
  - TL321X: 				C1T331A20_V1.0/C1T335A20_V1.0
* Platform Version: 
  - TLSR921X: 				tl_platform_sdk V3.2.0
  - TLSR922X/TLSR952X: 		tl_platform_sdk V3.2.0
  - TL721X: 				tl_platform_sdk V3.2.0
  - TL321X: 				tl_platform_sdk V3.2.0
* Toolchain Version:
  - TLSR921X: 				TL32 ELF MCULIB V5F GCC7.4  (IDE: [TelinkIoTStudio_V2024.8](https://wiki.telink-semi.cn/wiki/IDE-and-Tools/Telink_IoT_Studio/)) 
  - TLSR922X/TLSR952X: 		TL32 ELF MCULIB V5F GCC12.2 (IDE: [TelinkIoTStudio_V2024.8](https://wiki.telink-semi.cn/wiki/IDE-and-Tools/Telink_IoT_Studio/)) 
  - TL721X: 				TL32 ELF MCULIB V5F GCC12.2 (IDE: [TelinkIoTStudio_V2024.8](https://wiki.telink-semi.cn/wiki/IDE-and-Tools/Telink_IoT_Studio/)) 
  - TL321X: 				TL32 ELF MCULIB V5  GCC12.2 (IDE: [TelinkIoTStudio_V2024.8](https://wiki.telink-semi.cn/wiki/IDE-and-Tools/Telink_IoT_Studio/)) 

### Hardware
* TLSR921X
  - C1T213A20_V1.3
* TLSR952X
  - C1T266A20_V1.3
* TL721X
  - C1T314A20_V1.0
* TL321X
  - C1T331A20_V1.0
  - C1T335A20_V1.0
  
### Note
   * TL721X/TL321X当前不支持USB调试log。

### Bug Fixes
* **BLE通用功能**
   - 修复feature_ext_adv demo辅助包广播异常。

### BREAKING CHANGES 	
   * 所有芯片同步driver tl_platform_sdk V3.2.0。
     - TL321X支持A1芯片。
     - TL321X在API`sys_init`中增加供电模式参数，默认配置为`DCDC_1P25_LDO_1P8`。
	 - 更新TLSR921X/TL721X/TL321X deep retention模式下的提前唤醒时间。
   * TL721X 默认时钟设置更改为`PLL_240M_CCLK_48M_HCLK_48M_PCLK_48M_MSPI_48M`。
   * TL321X 默认时钟设置更改为`PLL_192M_CCLK_48M_HCLK_48M_PCLK_48M_MSPI_48M`。
   * TL721X 默认供电模式修改为`DCDC_0P94_DCDC_1P8`。

### Features
* **Chip**
  - 增加对TLSR922X/TLSR952X芯片的支持。
* **BLE通用功能** 
  - TL321X/TL721X增加Secure Boot功能。
* **Others**
  - 增加Debug Log序号打印来验证日志的完整性和一致性。
  - 删除无效API `blc_flash_set_4line_read_write`，用户可调用API `ble_flash_4line_enable`使能4线模式。

### Refactoring
   * N/A

### Performance Improvements
   * N/A

### Known issues
   * 对于支持ACL central功能的demo，系统时钟(**CCLK**)至少需设置为48MHz，下个版本将支持32MHz系统时钟(**CCLK**)。

### CodeSize
* TLSR921X
  - 编译 acl_central_demo
	- Flash bin size: 99.54 KB
	- IRAM size: 52.51 KB
	- DRAM size: 0.26 KB
  - 编译 acl_connection_demo
	- Flash bin size: 126.82 KB 
	- IRAM size: 66.94 KB
	- DRAM size: 4.35 KB
  - 编译 acl_peripheral_demo
	- Flash bin size: 105.96 KB
	- IRAM size: 54.01 KB
	- DRAM size: 0.24 KB

* TLSR922X/TLSR952X
  - 编译 acl_central_demo
	- Flash bin size: 102.61 KB
	- IRAM size: 55.69 KB
	- DRAM size: 0.31 KB
  - 编译 acl_connection_demo
	- Flash bin size: 129.96 KB 
	- IRAM size: 70.06 KB
	- DRAM size: 4.65 KB
  - 编译 acl_peripheral_demo
	- Flash bin size: 114.25 KB
	- IRAM size: 60.81 KB
	- DRAM size: 3.07 KB 

* TL721X
  - 编译 acl_central_demo
    - Flash bin size: 97.9KB
    - IRAM size: 53.2KB
	- DRAM size: 0KB
  - 编译 acl_connection_demo
    - Flash bin size:  124.8KB
    - IRAM size: 71.6KB
	- DRAM size: 0KB
  - 编译 acl_peripheral_demo
    - Flash bin size:  108.6KB
    - IRAM size: 60.6KB
	- DRAM size: 0KB

* TL321X
  - 编译 acl_central_demo
    - Flash bin size: 97.3KB
    - IRAM size: 51.5KB
    - DRAM size: 0.3KB
  - 编译 acl_connection_demo
    - Flash bin size:124.1KB 
    - IRAM size: 65.3KB
    - DRAM size: 4.6KB
  - 编译 acl_peripheral_demo
    - Flash bin size: 107.8KB
    - IRAM size: 55.8KB
    - DRAM size: 3.0KB 






## V4.0.4.1

### Version

* SDK Version: tl_ble_sdk_v4.0.4.1
* Chip Version: 
  - TLSR921X		A3/A4
  - TL721X			A1
  - TL321X			A0
* Driver Version: 
  - TLSR921X		tl_platform_sdk V2.3.0
  - TL721X			tl_platform_sdk V3.1.0
  - TL321X			tl_platform_sdk V3.1.0
* Toolchain Version:
  - TLSR921X		GCC 7 (IDE: Telink RDS)
  - TL721X			GCC 12(IDE: Telink IoT Studio)
  - TL321X			GCC 12(IDE: Telink IoT Studio)

### Hardware
* TLSR921X
  - C1T213A20
* TL721X
  - C1T314A20
* TL321X
  - C1T331A20
  - C1T335A20
  
### Note

* TL721X/TL321X currently does not support USB debug logs.
* When the bin size is larger than 256K, please change the OTA startup address using API "blc_ota_setFirmwareSizeAndBootAddress". The API needs to be placed before sys_init().
* Replaced all tabs with spaces.

### Bug Fixes
* **BLE general function**
   - Fixed [OTA] For TL321X, there is a small probability of failure during OTA for TL321X.
   - Fixed [ACL] When connecting to ACL central, the ACL peripheral device may fail with a low probability.
### BREAKING CHANGES 	
   * N/A

### Features
* **Power Management**
  - Added Power Management for TL721X/TL321X.
* **BLE general function** 
  - Added PHY test for TL321X.
  - Added SMP secure connection for TL321X.
  - Added API "blc_att_enableWriteReqReject" to allow the BLE stack to report the Write Request error code for TL721X/TL321X.
  - Added API "blc_att_enableReadReqReject" to allow the BLE stack to report the Read Request error code for TL721X/TL321X.

### Refactoring
   * N/A

### Performance Improvements
   * N/A

### Known issues
   * N/A

### CodeSize
* TLSR921X
  - Compiling acl_central_demo
    - Flash bin size: 103.0KB
    - IRAM size: 53.0KB
    - DRAM size: 0.4KB
  - Compiling acl_connection_demo
    - Flash bin size:128.0KB 
    - IRAM size: 67.3KB
    - DRAM size: 4.4KB
  - Compiling acl_peripheral_demo
    - Flash bin size: 108.0KB
    - IRAM size: 54.6KB
    - DRAM size: 0.3KB 

* TL721X
  - Compiling acl_central_demo
    - Flash bin size: 99.2KB
    - IRAM size: 52.7KB
  - Compiling acl_connection_demo
    - Flash bin size:  124.0KB
    - IRAM size: 71.2KB
  - Compiling acl_peripheral_demo
    - Flash bin size:  107.9KB
    - IRAM size: 60.0KB

* TL321X
  - Compiling acl_central_demo
    - Flash bin size: 97.8KB
    - IRAM size: 50.5KB
    - DRAM size: 0.3KB
  - Compiling acl_connection_demo
    - Flash bin size:122.4KB 
    - IRAM size: 63.8KB
    - DRAM size: 4.5KB
  - Compiling acl_peripheral_demo
    - Flash bin size: 106.4KB
    - IRAM size: 54.6KB
    - DRAM size: 3.0KB 


### Version

* SDK Version: tl_ble_sdk_v4.0.4.1
* Chip Version: 
  - TLSR921X		A3/A4
  - TL721X			A1
  - TL321X			A0
* Driver Version: 
  - TLSR921X		tl_platform_sdk V2.3.0
  - TL721X			tl_platform_sdk V3.1.0
  - TL321X			tl_platform_sdk V3.1.0
* Toolchain Version:
  - TLSR921X		GCC 7 (IDE: Telink RDS)
  - TL721X			GCC 12(IDE: Telink IoT Studio)
  - TL321X			GCC 12(IDE: Telink IoT Studio)

### Hardware
* TLSR921X
  - C1T213A20
* TL721X
  - C1T314A20
* TL321X
  - C1T331A20
  - C1T335A20
  
### Note

* TL721X/TL321X当前不支持USB调试log。
* 当bin大小大于256K时，请调用API​​ “blc_ota_setFirmwareSizeAndBootAddress”更改OTA启动地址，注意该API需要放在sys_init()之前。
* 已将所有的tab替换为空格。
### Bug Fixes
* **BLE通用功能**
   - Fixed [OTA] 修复了针对TL321X OTA过程中的小概率失败问题。
   - Fixed [ACL] 修复了作为peripheral建立连接时的小概率失败问题。

### BREAKING CHANGES 	
   * N/A

### Features
* **Power Management**
  - 对于TL721X/TL321X增加了功耗管理。
* **BLE通用功能** 
  - 对于TL321X增加了PHY test。
  - 对于TL321X增加了SMP secure connection模式。
  - 对于TL721X/TL321X增加了API “blc_att_enableWriteReqReject”使能协议栈上报执行Write Request时的错误码。
  - 对于TL721X/TL321X增加了API “blc_att_enableReadReqReject”使能协议栈上报执行Read Request时的错误码。

### Refactoring
   * N/A

### Performance Improvements
   * N/A

### Known issues
   * N/A

### CodeSize
* TLSR921X
  - 编译 acl_central_demo
    - Flash bin size: 103.0KB
    - IRAM size: 53.0KB
    - DRAM size: 0.4KB
  - 编译 acl_connection_demo
    - Flash bin size:128.0KB 
    - IRAM size: 67.3KB
    - DRAM size: 4.4KB
  - 编译 acl_peripheral_demo
    - Flash bin size: 108.0KB
    - IRAM size: 54.6KB
    - DRAM size: 0.3KB 

* TL721X
  - 编译 acl_central_demo
    - Flash bin size: 99.2KB
    - IRAM size: 52.7KB
  - 编译 acl_connection_demo
    - Flash bin size:  124.0KB
    - IRAM size: 71.2KB
  - 编译 acl_peripheral_demo
    - Flash bin size:  107.9KB
    - IRAM size: 60.0KB

* TL321X
  - 编译 acl_central_demo
    - Flash bin size: 97.8KB
    - IRAM size: 50.5KB
    - DRAM size: 0.3KB
  - 编译 acl_connection_demo
    - Flash bin size:122.4KB 
    - IRAM size: 63.8KB
    - DRAM size: 4.5KB
  - 编译 acl_peripheral_demo
    - Flash bin size: 106.4KB
    - IRAM size: 54.6KB
    - DRAM size: 3.0KB 



## V4.0.4.0

### Version

* SDK Version: tl_ble_sdk_v4.0.4.0
* Chip Version: 
  - TLSR951X		A3/A4
  - TL721X			A1
  - TL321X			A0
* Driver Version: 
  - TLSR951X		tl_platform_sdk V2.3.0
  - TL721X			tl_platform_sdk V3.0.0
  - TL321X			tl_platform_sdk V3.0.0
* Toolchain Version:
  - TLSR951X		GCC 7 (IDE, Telink RDS)
  - TL721X			GCC 12(IDE, Telink IoT Studio)
  - TL321X			GCC 12(IDE, Telink IoT Studio)

### Hardware
* TLSR951X
  - C1T213A20
* TL721X
  - C1T314A20
* TL321X
  - C1T331A20
  
### Note
* The system clock must be at least 32M.
* Battery Voltage Check is a very important function for mass production. The user must check the battery voltage to prevent abnormal writing or erasing Flash at a low voltage.
* Flash protection is a critical function for mass production. 
	- Flash protection is enabled by default in SDK. User must enable this function on their mass production application. 
	- Users should use the "Unlock" command in the Telink BDT tool for Flash access during the development and debugging phase.
	- Flash protection demonstration in SDK is a reference design based on sample code. Considering that the user's final application may be different from the sample code, 
	for example, the user's final firmware size is bigger, or the user has a different OTA design, or the user needs to store more data in some other area of Flash, 
	all these differences imply that Flash protection reference design in SDK can not be directly used on user's mass production application without any change. 
	User should refer to sample code, understand the principles and methods, and then change and implement a more appropriate mechanism according to their application if needed.
* USB debug log for TL721X/TL321X will be supported in the next version.
* Power Management for TL721X/TL321X will be supported in the next version.
* SMP secure connection for TL321X will be supported in the next version.
* PHY test for TL321X will be supported in the next version.

### Bug Fixes
   * N/A

### BREAKING CHANGES 	
   * N/A

### Features
* **Chip**
  - Support TLSR951X chip.
  - Support TL721X chip.
  - Support TL321X chip.
* **Demo & Library**
  - Provide 3 basic BLE demos and feature test examples.
	- acl_connection_demo supports 4 ACL Central and 4 ACL Peripheral devices with low power suspend mode.
	- acl_central_demo supports 4 ACL Central devices with low power suspend mode.
	- acl_peripheral_demo supports 4 ACL Peripheral devices with low power suspend/deepsleep_retention mode.
	- feature test provides some important function usage examples, e.g. extended ADV and extended Scan.
  - Provided 3 BLE libraries
    - liblt_B91.a/liblt_TL721X.a/liblt_TL321X.a
	  - Support BLE core 5.3 mandatory features and some generally used optional features.
	  - Support up to 4 ACL Central and 4 ACL Peripheral.	

* **FreeRTOS**
  - Support FreeRTOS in acl_peripheral_demo and acl_connection_demo. 
  
* **Application**
  - Flash Protection module and sample.
	- Include flash_prot.h/flash_prot.c in the vendor/common folder as a common module for all applications.
	- Include Flash protection sample code in project acl_central_demo/acl_connection_demo/acl_peripheral_demo. User can refer to the sample code and make sure the Flash protection function is enabled on their application.
	- "app_flash_protection_operation" is a common entrance for all Flash operations (Flash write and erase), Flash locking and unlocking are processed in this function.
	- "blc_appRegisterStackFlashOperationCallback" and "flash_prot_op_cb" are used to process some stack internal Flash operation callback which may need Flash locking and unlocking.
	  - The Telink OTA and SMP module already add callbacks related to blc_flashProt to inform the application layer that they need to write/erase Flash. Then, the application layer processes Flash lock and unlock.
	  - If users do not use Telink OTA, please refer to this design to add similar callback functions in their own OTA code for upper-layer use.
	- Now only firmware is protected in the sample code, system data and user data are not protected.

  - Debug log output, TLKAPI_DEBUG_ENABLE set to 1.
	- real-time log output mode.
	  - APP_REAL_TIME_PRINTF set to 1, TLKAPI_DEBUG_CHANNEL set to TLKAPI_DEBUG_CHANNEL_GSUART, print log with GPIO simulating UART.
	- delayed log with FIFO buffer.
	  - APP_REAL_TIME_PRINTF set to 0.
		- TLKAPI_DEBUG_CHANNEL set to TLKAPI_DEBUG_CHANNEL_UDB, print log with USB.
		- TLKAPI_DEBUG_CHANNEL set to TLKAPI_DEBUG_CHANNEL_UART, print log with UART.
		- TLKAPI_DEBUG_CHANNEL set to TLKAPI_DEBUG_CHANNEL_GSUART, print log with GPIO simulating UART.
### Refactoring
   * N/A

### Performance Improvements
   * N/A


### Known issues
* **General BLE function**
  - When connecting to ACL central, The ACL peripheral device may fail with a low probability. This issue will be fixed in the next version.
  - When the bin size is larger than 256K, please change the OTA startup address using API--blc_ota_setFirmwareSizeAndBootAddress. The API needs to be placed before sys_init().
  - There is a small probability of failure during OTA, this issue will be fixed in the next version.

### CodeSize
* TLSR951X
  - Compiling acl_central_demo
    - Flash bin size: 103.0KB
    - IRAM size: 53.0KB
    - DRAM size: 0.4KB
  - Compiling acl_connection_demo
    - Flash bin size:128.0KB 
    - IRAM size: 62.5KB
    - DRAM size: 4.4KB
  - Compiling acl_peripheral_demo
    - Flash bin size: 108.0KB
    - IRAM size: 50.4KB
    - DRAM size: 0.3KB 

* TL721X
  - Compiling acl_central_demo
    - Flash bin size: 99.2KB
    - IRAM size: 53.0KB
  - Compiling acl_connection_demo
    - Flash bin size:121.0KB 
    - IRAM size: 65.4KB
  - Compiling acl_peripheral_demo
    - Flash bin size: 105.0KB
    - IRAM size: 58.8KB

* TL321X
  - Compiling acl_central_demo
    - Flash bin size: 94.4KB
    - IRAM size: 47.8KB
    - DRAM size: 0.3KB
  - Compiling acl_connection_demo
    - Flash bin size:115.0KB 
    - IRAM size: 53.3KB
    - DRAM size: 4.5KB
  - Compiling acl_peripheral_demo
    - Flash bin size: 99.8KB
    - IRAM size: 50.3KB
    - DRAM size: 3.0KB 
