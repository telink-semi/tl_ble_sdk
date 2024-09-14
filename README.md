# tl_ble_sdk 

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
* The system clock must be at least 32M.
* Battery Voltage Check is a very important function for mass production. The user must check the battery voltage to prevent abnormal writing or erasing Flash at a low voltage.
* Flash protection is a critical function for mass production. 
	- Flash protection is enabled by default in SDK. User must enable this function on their mass production application. 
	- Users should use the "Unlock" command in the Telink BDT tool for Flash access during the development and debugging phase.
	- Flash protection demonstration in SDK is a reference design based on sample code. Considering that the user's final application may be different from the sample code, 
	for example, the user's final firmware size is bigger, or the user has a different OTA design, or the user needs to store more data in some other area of Flash, 
	all these differences imply that Flash protection reference design in SDK can not be directly used on user's mass production application without any change. 
	User should refer to sample code, understand the principles and methods, and then change and implement a more appropriate mechanism according to their application if needed.
* When the bin size is larger than 256K, please change the OTA startup address using API "blc_ota_setFirmwareSizeAndBootAddress". The API needs to be placed before sys_init().

### Features
* **Chip**
  - Support TLSR951X chip.
  - Support TLSR922X/TLSR952X chip.
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

