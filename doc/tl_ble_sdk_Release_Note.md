## V4.0.4.0

### Version

* SDK Version: tl_ble_sdk_v4.0.4.0
* Chip Version: 
  - TLSR951X		A3/A4
  - TL721X			A1
  - TL321X			A0
* Driver Version: tl_platform_sdk V3.0.0
* Toolchain Version:
  - TLSR951X		GCC 7
  - TL721X			GCC 12
  - TL321X			GCC 12

### Hardware
* TLSR951X
  - BOARD_B91_EVK_C1T213A20
* TL721X
  - BOARD_TL721X_EVK_C1T314A20
* TL321X
  - BOARD_TL321X_EVK_C1T331A20
  
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

* TL321X
 

