## V4.0.4.2_Patch_0002(PR)

### Bug Fixes

- **PM**
    - For TL321X, solved the issue that the possible SRAM loss when entering suspend sleep after deep retention sleep.

### Features

* **Drivers**
    * For TL721X, updated the Driver version to V3.3.0:
        * provided the support for the A2 version chip **(not compatible with the A1 version chip)**;
        * updated the default ADC pin of the battery voltage detection to  `ADC_GPIO_PB1`;
        * updated the ADC configuration in  `battery_check.c`;
        * added the support for three flash (`P25Q80SU` / `GD25LE80E` / `GD25LE16E`).

### BREAKING CHANGES

- N/A 



### Bug Fixes

- **PM**
    - 对于TL321X，解决了 deep retention 模式唤醒后，再进入 suspend 模式可能造成的 RAM 未完全保持问题。

### Features

* **Drivers**
    - 对于 TL721X，更新 Driver 版本为 V3.3.0：
        - 提供对 A2 版本芯片的支持，**不兼容对 A1 版本芯片的支持**；
        - 更新默认电池电压检测的 ADC 引脚为 `ADC_GPIO_PB1`；
        - 更新 `battery_check.c` 中 ADC 的配置；
        - 添加了对 `P25Q80SU`，`GD25LE80E`，`GD25LE16E` 三款 Flash 的支持.


### BREAKING CHANGES

- N/A 



## V4.0.4.2_Patch_0001(PR)


### Bug Fixes
* **PM**
   - For TL321X and TL721X, added calling `pm_update_status_info` in `blc_app_system_init`, and fixed the API `pm_get_mcu_status` returning MCU status error.
   - Fixed insufficient space for default DeepRetention when enabling FreeRtos and deepRetention.
* **Others** 
   - For TL321X, changed Debug IO GPIO_PC2 to GPIO_PC0 to avoid conflict with the external 32K crystal IO.
   - Fixed the exception of the Debug Log sequence number printing function.
   - For TL321X and TL721X, fixed the voltage value offset problem for the battery check Vbat mode.

### Features
* **BLE general function** 
  - Added API `blc_ll_getBleCurrentState` to get the current state of BLE Stack.
  - Added API `blc_ll_isRfBusy` to determine if RF is BUSY.
  - For TLSR921X(B91), added API `pm_get_mcu_status`to get MCU status.
* **Others**
  - Added error code `SYS_CLK_ERR_NOT_SUPPORTED` to check whether the system clock settings match.
  - For TLSR921X(B91) and TLSR922X/TLSR952X(B92), support obtaining DCOC calibration value through Flash.

### BREAKING CHANGES  
  * Added `__attribute__((packed))` for struct type variables.


### Bug Fixes
* **PM**
   - 对于TL321X和TL721X，在`blc_app_system_init`增加调用`pm_update_status_info`，修复API`pm_get_mcu_status`返回状态错误。
   - 修复使能FreeRtos和deepRetention时默认DeepRetention空间不足问题。
* **Others** 
   - 对于TL321X，修改Debug IO GPIO_PC2为GPIO_PC0，避免与32K外部晶振IO冲突。
   - 修复Debug Log序号打印功能异常。
   - 对于TL321X和TL721X，修复低压检测Vbat模式下电压值偏移问题。
   
### Features
* **BLE通用功能** 
  - 添加API`blc_ll_getBleCurrentState`获取当前BLE Stack状态。
  - 添加API`blc_ll_isRfBusy`判断当前RF是否BUSY。
  - 对于TLSR921X(B91)添加API`pm_get_mcu_status`获取MCU状态。
* **Others**
  - 添加错误码`SYS_CLK_ERR_NOT_SUPPORTED`检查系统时钟设置是否匹配。
  - 对于TLSR921X(B91)和TLSR922X/TLSR952X(B92)，支持通过Flash获取DCOC校准值。
  
### BREAKING CHANGES 
  * 对于struct类型变量添加`__attribute__((packed))`关键字。
