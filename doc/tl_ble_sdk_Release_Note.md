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
   * Changed TL721X default system clock setting to 48MHz.
   * Changed TL321X default system clock setting to 48MHz.
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
   
### Flash
* TLSR921X
  - P25Q80U
  - P25Q16SU
* TLSR952X
  - P25Q80U
  - P25Q16SU
* TL721X
  - P25Q16SU
* TL321X
  - P25Q80U
  - P25Q16SU

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
   * TL721X 默认系统时钟设置更改为48MHz。
   * TL321X 默认系统时钟设置更改为48MHz。
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

### Flash
* TLSR921X
  - P25Q80U
  - P25Q16SU
* TLSR952X
  - P25Q80U
  - P25Q16SU
* TL721X
  - P25Q16SU
* TL321X
  - P25Q80U
  - P25Q16SU

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
   
### Flash
* TLSR921X
  - P25Q80U
  - P25Q16SU
* TL721X
  - P25Q16SU
* TL321X
  - P25Q80U
  - P25Q16SU

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

### Flash
* TLSR921X
  - P25Q80U
  - P25Q16SU
* TL721X
  - P25Q16SU
* TL321X
  - P25Q80U
  - P25Q16SU

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

### Flash
* TLSR951X
  - P25Q80U
  - P25Q16SU
* TL721X
  - P25Q16SU
* TL321X
  - P25Q80U

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
