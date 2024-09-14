## V3.2.0

### Version

* SDK version: tl_platform_sdk V3.2.0
* Chip version
  * TLSR921x/TLSR951x(B91) (A0/A1/A2), TLSR922x/TLSR952x(B92) (A3/A4), TL721X (A1), TL321X (A1)
* Hardware EVK Version
  * TLSR951x(B91): C1T213A20
  * TLSR952x(B92): C1T266A20
  * TL721X: C1T315A20
  * TL321X: C1T335A20
* Toolchain version
  - TLSR921x/TLSR951x(B91): gcc7(TL32 ELF MCULIB V5F GCC7.4 (riscv32-elf-gcc))
  - TLSR922x/TLSR952x(B92): gcc12(TL32 ELF MCULIB V5F GCC12.2 (riscv32-elf-gcc))
  - TL721x: gcc12(TL32 ELF MCULIB V5F GCC12.2 (riscv32-elf-gcc))
  - TL321x: gcc12(TL32 ELF MCULIB V5 GCC12.2 (riscv32-elf-gcc))

### Note
* (B92/TL721x/TL321x) Update toolchains from V5.3.0 to V5.3.x(http://192.168.48.36/src/driver/tl_platform_src/merge_requests/1175)
* (B91/B92/TL721X/TL321X) Add usb_set_pin() interface, in USB application can choose to enable or disable dp_through_swire function, for compatibility usb_set_pin_en() is defined as usb_set_pin(1) and comments were added to cstartup.S.(http://192.168.48.36/src/driver/tl_platform_src/merge_requests/1249)

### BREAKING CHANGES

* **sys**
  * (TL321X)Add power_mode parameter into sys_init(A0 version only support LDO, A1 version support both LDO and DCDC_LDO) and update sys_init to adapt the chip A1 version.(http://192.168.48.36/src/driver/tl_platform_src/merge_requests/1218)
  * (TL721x/TL321x)In the new version, after calling sys_init(), if you want to get the value of g_pm_status_info.mcu_status or call pm_get_reboot_event() to get related information, you won't be able to. The application layer needs to call pm_update_status_info() to obtain it. The new usage method is as follows:After calling sys_init(), call pm_update_status_info(), and you can get the value of g_pm_status_info.mcu_status and call pm_get_sw_reboot_event to check the reason for the software reboot.Additionally, the reboot classifications have been changed as follows:1.Two new statuses have been added to g_pm_status_info.mcu_status: MCU_HW_REBOOT_TIMER_WATCHDOG and MCU_HW_REBOOT_32K_WATCHDOG.2.The meaning of MCU_STATUS_REBOOT_BACK has changed. Previously, it included both software reboots and watchdog reboots. In the new version, MCU_STATUS_REBOOT_BACK only represents software reboots. If you want to know the specific reason for a software reboot, you can call pm_get_sw_reboot_event to check. The functions pm_get_reboot_event() and pm_get_deep_retention_flag() have been removed.(http://192.168.48.36/src/driver/tl_platform_src/merge_requests/1195,http://192.168.48.36/src/driver/tl_platform_src/merge_requests/1213)
  * (TL721x/TL321x)Previously, the reasons for software reboots could coexist in multiple situations. To save register bits for future expansion, the new version only saves the reason for the most recent software reboot. Therefore, if you want to save all software reboot reasons, the upper application needs to save them after each initialization to prevent losing the history of reboot reasons.(http://192.168.48.36/src/driver/tl_platform_src/merge_requests/1195,http://192.168.48.36/src/driver/tl_platform_src/merge_requests/1213)
  * (TL721x/TL321x)Function name changes:sys_set_power_mode -> pm_set_power_mode, sys_set_vbat_type -> pm_set_vbat_type.(http://192.168.48.36/src/driver/tl_platform_src/merge_requests/1195,http://192.168.48.36/src/driver/tl_platform_src/merge_requests/1213)
* **dma**
  * (B92/TL321x/TL721x) Change the interface for setting the interrupt mode of linked list DMA from dma_set_llp_int_mode to dma_set_llp_irq_mode, and unify the enumeration type of the interrupt mode to dma_llp_irq_mode_e.(http://192.168.48.36/src/driver/tl_platform_src/merge_requests/1229)
* **audio**
  * (B92) Change audio_set_i2s_align_en,audio_set_i2s_align_dis interface name to audio_i2s_align_en,audio_i2s_align_dis.(http://192.168.48.36/src/driver/tl_platform_src/merge_requests/1144).
* **emi**
  * (B91/B92/TL721X/TL321X)Modify the emi. c rf_comtinue_made_run interface to add configurable packet data types and adjust the correspondence between command values and data types from (0: pbrs9 1: 0xf0 2:0x55) to (0: pbrs9 1: 0x0f 2:0x55 3: 0xaa 4:0xf0 5:0x00 6:0xff)(http://192.168.48.36/src/driver/tl_platform_src/merge_requests/1247)
  * (B91/B92/TL721X/TL321X)Renaming the original supported mode of the rf_mode parameter of the functions rf_emi_tx_continue_update_data rf_emi_rx_setup rf_emi_tx_burst_loop and rf_emi_tx_burst_setup RF_MODE_BLE_2M to RF_MODE_BLE_2M_NO_PN (the new name is closer to its mode)(http://192.168.48.36/src/driver/tl_platform_src/merge_requests/1247)
* **adc**
  * (TL721x/TL321x)ADC DMA mode is optimized from round-robin mode to DMA interrupt mode to read data, which improves the efficiency of code running, and at the same time rename adc_get_sample_status_dma()/adc_clr_sample_status_dma()/adc_set_dma_trig_num() to adc_get_irq_status_dma()/adc_clr _irq_status_dma()/adc_set_rx_fifo_trig_cnt(), deleted adc_get_code_dma().(http://192.168.48.36/src/driver/tl_platform_src/merge_requests/1246)

### Features

* **flash**
  * (TL321x)add new flash P25Q40SU/P25Q16SU.(http://192.168.48.36/src/driver/tl_platform_src/merge_requests/1171)
* **Secure_Boot_Demo**
  * (TL721x/TL321x) Add Secure_Boot_Demo, compile the demo to generate runtime descriptors, etc. bin.(http://192.168.48.36/src/driver/tl_platform_src/merge_requests/1184)
* **Toolchain**
  * (B91/B92/TL321x/TL721x):In order to support open source toolchains compilation (if using an open source toolchain, this macro 'STD_GCC' needs to be opened, and currently support Zephyr toolchain compilation), the following modifications are made:
    1. Use assembler language to implement interfaces for reading and writing CSR registers, without using the functions in nds_intrinsic.h (which is a header file in the Andes toolchain);
    2. Add a new file core_reg.h and copy CSR registers from nds_intrinsic.h to core_reg.h.(http://192.168.48.36/src/driver/tl_platform_src/merge_requests/1181)
* **Trap**
  * (B91/B92/TL321x/TL721x)Add new interfaces plic_irqs_preprocess_for_wfi() and plic_irqs_postprocess_for_wfi() to configure the interrupts related to entering and exiting WFI mode, respectively, and provide sample code(http://192.168.48.36/src/driver/tl_platform_src/merge_requests/1186).
* **efuse**
  * (B92):Added efuse_get_chip_status interface to read the status of JTAG, SWS, and boot mode.(http://192.168.48.36/src/driver/tl_platform_src/merge_requests/1223)
  * (TL321x) Added efuse_get_chip_id interface.(http://192.168.48.36/src/driver/tl_platform_src/merge_requests/1075)
* **IR_LEARN_Demo**
  * (TL721x/TL321x) Add IR_LEARN_Demo and driver files.(http://192.168.48.36/src/driver/tl_platform_src/merge_requests/1214).
* **audio**
  * (TL721x) Adding audio related drivers and demos.(http://192.168.48.36/src/driver/tl_platform_src/merge_requests/1144)。
* **USB Demo**
  * (B91/B92/TL321x/TL721x) added print demo into USB_Demo.(http://192.168.48.36/src/driver/tl_platform_src/merge_requests/1230)
* **link**
  * (TL721x/TL321x)Add flash code segment for link and _attribute_flash_code_sec_noinline_ into compiler.h.(http://192.168.48.36/src/driver/tl_platform_src/merge_requests/1243)
  * (B91/B92/TL721X/TL321X)Added the rf_certification_cfg data segment for rf certification related configuration.(http://192.168.48.36/src/driver/tl_platform_src/merge_requests/1175)
* **common**
  * (B91/B92/TL721x/TL321X):To prevent power leakage, add gpio_shutdown(GPIO_ALL) to platform_init() to configure all GPIOs (except SWS and MSPI) to a high resistance state.(http://192.168.48.36/src/driver/tl_platform_src/merge_requests/1226)
* **Jtag**
  * (B92/TL321x/TL721x): Add jtag configuration interface jtag_sdp_set_pin(), two-wire and four-wire enable interfaces jtag_set_pin_en() and sdp_set_pin_en().(http://192.168.48.36/src/driver/tl_platform_src/merge_requests/1226)
* **emi**
  * (B91/B92/TL721X/TL321X)Added rf_phy_test_prbs15() interface for generating prbs15 type data(http://192.168.48.36/src/driver/tl_platform_src/merge_requests/1247)
* **calibration**
  * (TL721X/TL321X)Added calibration_func(),user_calib_freq_offset() calibration interface.(http://192.168.48.36/src/driver/tl_platform_src/merge_requests/1247)
* **rf**
  * (B91/B92/TL721X/TL321X)Add rf_set_power_level_singletone interface, remove rf_set_power_level_index_singletone interface from emi.c(http://192.168.48.36/src/driver/tl_platform_src/merge_requests/1247)
* **S**
  * (B91/B92/TL721X/TL321X)Added the starting address 0x10 related to RF_Certification configuration and defined the starting address of _RF_CCERTIFICATION_CFG_LMA_START(http://192.168.48.36/src/driver/tl_platform_src/merge_requests/1175)
* **adc**
  * (TL721x/TL321x):add adc_start_sample_nodma() , adc_stop_sample_nodma(), adc_get_irq_status(), adc_clr_irq_status() and adc_set_scan_chn_dis().(http://192.168.48.36/src/driver/tl_platform_src/merge_requests/1246)

### Bug Fixes

* **flash**
  * (TL721x)Fix flash default protected area(Changed from unprotected to protect half of the flash area).(http://192.168.48.36/src/driver/tl_platform_src/merge_requests/1188)
* **rf**
  * (TL321x)Fix rf_power_level_index_e enumeration value error problem.(http://192.168.48.36/src/driver/tl_platform_src/merge_requests/1194)
  * (TL721x/TL321x)Due to the removal of reg_bb_dma_rx_rptr, the interface rf_get_rx_packet_addr in rf_common.c has been updated to replace reg_bb_dma_rx_rptr with reg_rf_dma_rx_rptr.(http://192.168.48.36/src/driver/tl_platform_src/merge_requests/1207)
  * (TL721x/TL321x)Fixed the issue of inaccurate RSSI values obtained by RF.(http://192.168.48.36/src/driver/tl_platform_src/merge_requests/1261)
* **pm**
  * (B91/B92/TL721x/TL321x)Fixed an issue where calling function pm_set_vbat_type or sys_init to modify parameter vbat_v failed. (This is only a problem when changing VBAT_MAX_VALUE_LESS_THAN_3V6 to VBAT_MAX_VALUE_GREATER_THAN_3V6)(http://192.168.48.36/src/driver/tl_platform_src/merge_requests/1195,http://192.168.48.36/src/driver/tl_platform_src/merge_requests/1213)
* **dma** 
  * (TL721x/TL321x)Due to invalid or duplicate definitions, the following interfaces have been deleted: In dma_reg.h: reg_dma_rx_wptr, reg_dma_tx_wptr, reg_dma_rx_rptr, reg_dma_tx_rptr. In rf_reg.h: reg_bb_dma_rx_wptr, reg_bb_dma_tx_wptr, reg_bb_dma_rx_rptr, reg_bb_dma_tx_rptr. Users should now use the following register definitions: reg_rf_dma_rx_wptr, reg_rf_dma_rx_rptr, reg_rf_dma_tx_rptr, reg_rf_dma_tx_wptr. (http://192.168.48.36/src/driver/tl_platform_src/merge_requests/1207)(http://192.168.48.36/src/driver/tl_platform_src/merge_requests/1235)
* **spi**
  * (B92/TL721x/TL321x) Change the type of word_len in the spi_write/spi_read interface from unsigned char to unsigned int.(http://192.168.48.36/src/driver/tl_platform_src/merge_requests/1144).
* **gpio**
  * (TL721x/TL321X)Fix the problem of not setting gpio to low level in gpio_init(), preventing that gpio cannot be initialized to low level after calling gpio_init().(http://192.168.48.36/src/driver/tl_platform_src/merge_requests/1226)  
 * **gpio**
  * (B91)Fix the problem of no initialization PF group (MSPI) in gpio_init() to prevent the MSPI port from not being initialized after calling gpio_init().(http://192.168.48.36/src/driver/tl_platform_src/merge_requests/1226)  
* **Jtag**
  * (B91): The jtag two-wire and four-wire enable interfaces jtag_set_pin_en() and sdp_set_pin_en() add analog pull-ups and drop-downs to prevent the jtag hardware from being abnormal when not connected.(http://192.168.48.36/src/driver/tl_platform_src/merge_requests/1226)
* **usb**
  * (TL321X)Fix the problem of setting usbhw_enable_hw_feature() would clear other features exceptionally.(http://192.168.48.36/src/driver/tl_platform_src/merge_requests/1249)
* **adc**
  * (TL721x/TL321x):adc_get_code() is changed from reading analog registers to reading adc rx fifo to prevent repeated fetching of the same code in multiple consecutive calls to adc_get_code().(http://192.168.48.36/src/driver/tl_platform_src/merge_requests/1246)
* **adc**
  * (TL721x/TL321x): Modify the sample cycle configuration for each sampling frequency, and delay 30us after adc_power_on() to wait for adc to stabilize, fix the problem of the first code exception after adc_power_on().(http://192.168.48.36/src/driver/tl_platform_src/merge_requests/1246)
* **flash**
  * (B91/B92/TL721X/TL321X):Add macro in platform_init() not to add protection to the flash of the chip running ram bin to prevent the chip with no flash on board from getting stuck.(http://192.168.48.36/src/driver/tl_platform_src/merge_requests/1255)
* **uart**
  * (TL721X/TL321X):Fixed dma uart rx data loss issue by changing the length parameter passed to uart_receive_dma to the maximum value that can be received.(http://192.168.48.36/src/driver/tl_platform_src/merge_requests/1257)

### Refactoring

* **trap**
  * (B91/B92/TL721X/TL321X) Define the trap_entry function as a weak function so that applications can reimplement it.(http://192.168.48.36/src/driver/tl_platform_src/merge_requests/1212).
* **ADC**
  * (B91/B92/TL721X/TL321X)The ADC temperature detection function is an internal test function, not opened to the public, and the function is disabled by default.(http://192.168.48.36/src/driver/tl_platform_src/merge_requests/1227)
* **pm**
  * (TL721x/TL321x)Encapsulated clock.c, stimer.c, mspi.c, and core.c into library files.(http://192.168.48.36/src/driver/tl_platform_src/merge_requests/1195,http://192.168.48.36/src/driver/tl_platform_src/merge_requests/1213)
  * (TL721X)Modify the PM-related interfaces to use the O2 optimization option.(http://192.168.48.36/src/driver/tl_platform_src/merge_requests/1238)
* **usb**
  * (TL721x/TL321x)Remove unused header file (wchar.h) from usbhw.c. (http://192.168.48.36/src/driver/tl_platform_src/merge_requests/1237)
* **calibration**
  * (B91/B92)Add a range limit of frequency_offset_value to the user_calib_freq_offset interface(http://192.168.48.36/src/driver/tl_platform_src/merge_requests/1247)
* **rf**
  * (TL721x/TL321x)the function rf_clr_dig_logic_state and rf_reset_register_value have been updated with the _attribute_ram_code_sec_noinline_ attribute. The function rf_dma_reset has been updated to static _always_inline.(http://192.168.48.36/src/driver/tl_platform_src/merge_requests/1251)

### Performance Improvements

* **clock**
  * (TL721x/TL321x)To save power, the 24M RC and PLL are turned off separately when not in use.(http://192.168.48.36/src/driver/tl_platform_src/merge_requests/1195,http://192.168.48.36/src/driver/tl_platform_src/merge_requests/1213)
* **S**
  * (TL721x/TL321x)To save power, the GPIO input functionality is turned off during initialization.(http://192.168.48.36/src/driver/tl_platform_src/merge_requests/1195,http://192.168.48.36/src/driver/tl_platform_src/merge_requests/1213)
* **rf**
  * (TL721x)Add configuration related to optimising TX power consumption inside rf_mode_init().(http://192.168.48.36/src/driver/tl_platform_src/merge_requests/1234)
  * (TL321x):Optimise power consumption in tx vant mode, increase vant maximum output power, 
  and update the rf_power_level_list table with the latest driver configuration.
  (http://192.168.48.36/src/driver/tl_platform_src/merge_requests/1213,http://192.168.48.36/src/driver/tl_platform_src/merge_requests/1234)
  * (TL321x): optimise RX sensitivity performance, add rf_rx_performance_mode interface and rf_rx_performance_e to select RX sensitivity configurations, default configuration is RF_RX_LOW_POWER; select RF_RX_HIGH_PERFORMANCE to increase the performance by 1dbm, but it will increase the RX power consumption.(http://192.168.48.36/src/driver/tl_platform_src/merge_requests/1213,http://192.168.48.36/src/driver/tl_platform_src/merge_requests/1234,http://192.168.48.36/src/driver/tl_platform_src/merge_requests/1265)
  * (TL721x/TL321x)Improve BLE500K and BLE125K per floor performance.(http://192.168.48.36/src/driver/tl_platform_src/merge_requests/1232)(http://192.168.48.36/src/driver/tl_platform_src/merge_requests/1260)

### 版本

* SDK版本: tl_platform_sdk V3.2.0
* 芯片版本
  * TLSR921x/TLSR951x(B91) (A0/A1/A2), TLSR922x/TLSR952x(B92) (A3/A4), TL721X (A1), TL321X (A1)
* 硬件评估板版本
  * TLSR951x(B91): C1T213A20
  * TLSR952x(B92): C1T266A20
  * TL721X: C1T315A20
  * TL321X: C1T335A20
* 工具链版本
  - TLSR921x/TLSR951x(B91): gcc7(TL32 ELF MCULIB V5F GCC7.4 (riscv32-elf-gcc))
  - TLSR922x/TLSR952x(B92): gcc12(TL32 ELF MCULIB V5F GCC12.2 (riscv32-elf-gcc))
  - TL721x: gcc12(TL32 ELF MCULIB V5F GCC12.2 (riscv32-elf-gcc))
  - TL321x: gcc12(TL32 ELF MCULIB V5 GCC12.2 (riscv32-elf-gcc))

### Note
* (B92/TL721x/TL321x)编译工具链从V5.3.0升级为V5.3.x。(http://192.168.48.36/src/driver/tl_platform_src/merge_requests/1175)
* (B91/B92/TL721X/TL321X) 新增usb_set_pin()接口，在USB应用中可以选择使能或不使能dp_through_swire功能，为了兼容性usb_set_pin_en()定义为 usb_set_pin(1)，并在cstartup.S 添加相应注释。(http://192.168.48.36/src/driver/tl_platform_src/merge_requests/1249)

### BREAKING CHANGES 

* **sys**
  * (TL321X)sys_init新增power_mode参数（A0只支持LDO模式，A1支持LDO和DCDC_LDO）同时更新sys_init内容以适配A1芯片。(http://192.168.48.36/src/driver/tl_platform_src/merge_requests/1218)
  * (TL721x/TL321x)新的版本调用sys_init()之后，如果想要获取g_pm_status_info.mcu_status的值，或者调用pm_get_reboot_event（）获取相关信息，获取不到了。需要应用层自己调用pm_update_status_info()获取。新的使用方法如下：调用sys_init()后再调用pm_update_status_info()，即可获得g_pm_status_info.mcu_status的值并且可以调用pm_get_sw_reboot_event查看软件重启原因。同时，之前的reboot分类也做了如下变更：1.g_pm_status_info.mcu_status新增两个状态：MCU_HW_REBOOT_TIMER_WATCHDOG 、MCU_HW_REBOOT_32K_WATCHDOG2.MCU_STATUS_REBOOT_BACK的含义和以前不一样了，以前是包含软件重启和watchdog重启。现在的版本MCU_STATUS_REBOOT_BACK仅仅代表软件重启，如果想要知道软件重启的具体原因，可以调用pm_get_sw_reboot_event进行查看。删除了pm_get_reboot_event()/pm_get_deep_retention_flag()函数。(http://192.168.48.36/src/driver/tl_platform_src/merge_requests/1195,http://192.168.48.36/src/driver/tl_platform_src/merge_requests/1213)
  * (TL721x/TL321x)之前的软件reboot原因，可以多种情况共存，为了节省寄存器bit以便将来扩展使用，现在的版本只会保存最新一次软件reboot的原因。所以如果想要保存所有软件reboot的原因的话，上层应用需要每次初始化后自行保存，以防看不到历史的reboot原因。(http://192.168.48.36/src/driver/tl_platform_src/merge_requests/1195,http://192.168.48.36/src/driver/tl_platform_src/merge_requests/1213)
  * (TL721x/TL321x)函数名字变更：sys_set_power_mode->pm_set_power_mode，sys_set_vbat_type->pm_set_vbat_type。(http://192.168.48.36/src/driver/tl_platform_src/merge_requests/1195,http://192.168.48.36/src/driver/tl_platform_src/merge_requests/1213)
* **dma**
  * (B92/TL321x/TL721x)设置链表DMA的中断模式的接口由dma_set_llp_int_mode改成dma_set_llp_irq_mode, 将中断模式的枚举类型统一为dma_llp_irq_mode_e。(http://192.168.48.36/src/driver/tl_platform_src/merge_requests/1229)
* **dma**
  * (B92/TL721x/TL321x) 将dma_set_spi_burst_size接口名称修改为dma_set_burst_size。(http://192.168.48.36/src/driver/tl_platform_src/merge_requests/1144).
* **audio**
  * (B92) 将audio_set_i2s_align_en,audio_set_i2s_align_dis接口名称修改为audio_i2s_align_en,audio_i2s_align_dis。(http://192.168.48.36/src/driver/tl_platform_src/merge_requests/1144).
* **emi**
  * (B91/B92/TL721X/TL321X)修改emi.c rf_continue_mode_run 接口，增加了可配置发包数据类型，调整命令值与数据类型的对应关系，由（0:pbrs9 1:0xf0 2:0x55）调整为（0:pbrs9 1:0x0f 2:0x55 3:0xaa 4:0xf0 5:0x00 6:0xff）(http://192.168.48.36/src/driver/tl_platform_src/merge_requests/1247)
  * (B91/B92/TL721X/TL321X)将 rf_emi_tx_continue_update_data,rf_emi_rx_setup,rf_emi_tx_burst_loop,rf_emi_tx_burst_setup函数的rf_mode参数的原支持模式RF_MODE_BLE_2M更名为RF_MODE_BLE_2M_NO_PN（新名字更贴近其模式）(http://192.168.48.36/src/driver/tl_platform_src/merge_requests/1247)
* **adc**
  * (TL721x/TL321x)ADC DMA模式从轮巡方式优化成DMA中断方式读取数据，提高代码运行效率，同时将adc_get_sample_status_dma()/adc_clr_sample_status_dma()/adc_set_dma_trig_num()改名为adc_get_irq_status_dma()/adc_clr_irq_status_dma()/adc_set_rx_fifo_trig_cnt(),删除了adc_get_code_dma()。(http://192.168.48.36/src/driver/tl_platform_src/merge_requests/1246)

### Features

* **flash**
  * (TL321x)添加新增flash P25Q40SU/P25Q16SU.(http://192.168.48.36/src/driver/tl_platform_src/merge_requests/1171)
* **Secure_Boot_Demo**
  * (TL721x/TL321x) 添加Secure_Boot_Demo, 编译demo生成运行描述符等bin.(http://192.168.48.36/src/driver/tl_platform_src/merge_requests/1184)
* **Toolchain**
  * (B91/B92/TL321x/TL721x):为了支持开源工具链编译(若使用开源工具链，需要打开这个宏'STD_GCC'，目前支持Zephyr工具链编译)，做如下修改：
    1. 使用汇编语言实现CSR寄存器的读写等接口，不使用nds_intrinsic.h内的函数(这是一个在Andes工具链里的头文件)；
    2. 添加一个新文件core_reg.h, 将nds_intrinsic.h里面CSR寄存器拷贝到core_reg.h。(http://192.168.48.36/src/driver/tl_platform_src/merge_requests/1181)
* **Trap**
  * (B91/B92/TL321x/TL721x)新增接口 plic_irqs_preprocess_for_wfi() 和 plic_irqs_postprocess_for_wfi() ，分别用于配置进入和退出 WFI 模式的相关中断并提供示例代码(http://192.168.48.36/src/driver/tl_platform_src/merge_requests/1186)。
* **efuse**
  * (B92):增加efuse_get_chip_status接口，用于读取JTAG、SWS 和启动模式的状态。(http://192.168.48.36/src/driver/tl_platform_src/merge_requests/1223)
  * (TL321x) 增加efuse_get_chip_id接口。(http://192.168.48.36/src/driver/tl_platform_src/merge_requests/1075)
* **IR_LEARN_Demo**
  * (TL721x/TL321x) 新增IR_LEARN_Demo和driver files.(http://192.168.48.36/src/driver/tl_platform_src/merge_requests/1214)。
* **audio**
  * (TL721x) 添加audio相关驱动和demo.(http://192.168.48.36/src/driver/tl_platform_src/merge_requests/1144)。
* **USB Demo**
  * (B91/B92/TL321x/TL721x)在USB_Demo中添加print demo。(http://192.168.48.36/src/driver/tl_platform_src/merge_requests/1230)
* **link**
  * (TL721X/TL321X)link文件中新增flash_code段,complier.h中新增_attribute_flash_code_sec_noinline_(http://192.168.48.36/src/driver/tl_platform_src/merge_requests/1243)
  * (B91/B92/TL721X/TL321X)添加rf_certification_cfg段，用于rf certification的相关配置(http://192.168.48.36/src/driver/tl_platform_src/merge_requests/1175)
* **common**
  * (B91/B92/TL721x/TL321X)为防止漏电，在platform_init()中添加gpio_shutdown(GPIO_ALL)将所有gpio（除SWS和MSPI外）配置为高阻态。(http://192.168.48.36/src/driver/tl_platform_src/merge_requests/1226)
* **Jtag**
  * (B92/TL321x/TL721x)添加jtag配置接口jtag_sdp_set_pin()，两线和四线使能接口jtag_set_pin_en()和sdp_set_pin_en()。(http://192.168.48.36/src/driver/tl_platform_src/merge_requests/1226)
* **emi**
  * (B91/B92/TL721X/TL321X)新增rf_phy_test_prbs15()接口用于生成prbs15类型数据(http://192.168.48.36/src/driver/tl_platform_src/merge_requests/1247)
* **calibration**
  * (TL721X/TL321X)新增calibration_func(),user_calib_freq_offset()校准接口(http://192.168.48.36/src/driver/tl_platform_src/merge_requests/1247)
* **rf**
   * (B91/B92/TL721X/TL321X)新增rf_set_power_level_singletone接口，删除emi.c 中的rf_set_power_level_index_singletone接口(http://192.168.48.36/src/driver/tl_platform_src/merge_requests/1247)
* **S**
  * (B91/B92/TL721X/TL321X)添加RF_Certification配置相关的起始地址 0x10 并定义 RF_Certification 配置的起始地址 _RF_CERTIFICATION_CFG_ADDR_OFFSET(http://192.168.48.36/src/driver/tl_platform_src/merge_requests/1175)
* **adc**
  * (TL721x/TL321x):添加adc_start_sample_nodma(), adc_stop_sample_nodma(), adc_get_irq_status(),adc_clr_irq_status()和adc_set_scan_chn_dis().(http://192.168.48.36/src/driver/tl_platform_src/merge_requests/1246)  
### Bug Fixes

* **flash**
  * (TL721x)修复flash默认保护区域（从无保护改为保护flash的一半区域）。(http://192.168.48.36/src/driver/tl_platform_src/merge_requests/1188)
* **rf**
  * (TL321x)修复 rf_power_level_index_e 枚举值错误问题。(http://192.168.48.36/src/driver/tl_platform_src/merge_requests/1194)
  * (TL721x/TL321x)由于删除了 reg_bb_dma_rx_rptr，在 rf_common.c 中，接口 rf_get_rx_packet_addr 中的 reg_bb_dma_rx_rptr 已替换为 reg_rf_dma_rx_rptr。(http://192.168.48.36/src/driver/tl_platform_src/merge_requests/1207)
  * (TL721x/TL321x)修复了RF获取RSSI值不准的问题(http://192.168.48.36/src/driver/tl_platform_src/merge_requests/1261)
* **pm**
  * (B91/B92/TL721x/TL321x)解决了调用函数pm_set_vbat_type或者sys_init修改参数vbat_v不成功的问题。（仅仅当将VBAT_MAX_VALUE_LESS_THAN_3V6修改为VBAT_MAX_VALUE_GREATER_THAN_3V6会有问题）(http://192.168.48.36/src/driver/tl_platform_src/merge_requests/1195,http://192.168.48.36/src/driver/tl_platform_src/merge_requests/1213)
* **dma** 
  * (TL721x/TL321x)由于定义无效或重复，删除了以下寄存器定义：dma_reg.h中reg_dma_rx_wptr、reg_dma_tx_wptr、reg_dma_rx_rptr、reg_dma_tx_rptr；rf_reg.h中reg_bb_dma_rx_wptr、reg_bb_dma_tx_wptr、reg_bb_dma_rx_rptr、reg_bb_dma_tx_rptr;用户请使用如下寄存器定义：reg_rf_dma_rx_wptr、reg_rf_dma_rx_rptr、reg_rf_dma_tx_rptr、reg_rf_dma_tx_wptr。(http://192.168.48.36/src/driver/tl_platform_src/merge_requests/1207)(http://192.168.48.36/src/driver/tl_platform_src/merge_requests/1235)
* **spi**
  * (B92/TL721x/TL321x) 将spi_write/spi_read接口中的word_len类型从unsigned char修改为unsigned int。(http://192.168.48.36/src/driver/tl_platform_src/merge_requests/1144).
* **gpio**
  * (TL721x/TL321X)修复gpio_init()中无法设置gpio为低电平的问题，防止调用gpio_init()后不能将gpio初始化为低电平。(http://192.168.48.36/src/driver/tl_platform_src/merge_requests/1226)  
 * **gpio**
  * (B91)修复gpio_init()中无法初始化PF组(MSPI)的问题，防止调用gpio_init()后不能初始化MSPI口。(http://192.168.48.36/src/driver/tl_platform_src/merge_requests/1226)  
* **Jtag**
  * (B91)jtag两线和四线使能接口jtag_set_pin_en()和sdp_set_pin_en()添加模拟上下拉，防止jtag硬件在未连接时出现异常。(http://192.168.48.36/src/driver/tl_platform_src/merge_requests/1226)  
* **usb**
  * (TL321X)修复了设置usbhw_enable_hw_feature()时会将其他feature异常清除的问题.(http://192.168.48.36/src/driver/tl_platform_src/merge_requests/1249)
* **adc**
  * (TL721x/TL321x): adc_get_code()由读模拟寄存器方式改为读adc rx fifo方式，防止连续多次调用adc_get_code()时重复取的是同一个code。(http://192.168.48.36/src/driver/tl_platform_src/merge_requests/1246)
* **adc**
  * (TL721x/TL321x):修改各个采样频率的sample cycle配置，且在adc_power_on()后delay 30us 等待adc稳定，修复adc_power_on()后第一个code异常的问题。(http://192.168.48.36/src/driver/tl_platform_src/merge_requests/1246)
* **flash**
  * (B91/B92/TL721X/TL321X):在platform_init()中添加宏不对跑ram bin的芯片的flash加保护，防止无搭载flash的芯片卡死。(http://192.168.48.36/src/driver/tl_platform_src/merge_requests/1255)
* **uart**
  * (TL721X/TL321X):修复了dma uart rx数据丢失问题,将传递给uart_receive_dma的长度参数修改为可以接收到的最大值。(http://192.168.48.36/src/driver/tl_platform_src/merge_requests/1257)

### Refactoring

* **trap**
  * (B91/B92/TL721X/TL321X) 将 trap_entry 函数定义为弱函数，以便应用程序重新可以实现该函数。(http://192.168.48.36/src/driver/tl_platform_src/merge_requests/1212).
* **ADC**
  * (B91/B92/TL721X/TL321X)ADC温度检测功能属于内部测试功能，不对外开放，默认禁用该功能。(http://192.168.48.36/src/driver/tl_platform_src/merge_requests/1227)
* **pm**
  * (TL721x/TL321x)clock.c stimer.c mspi.c core.c 封装成库文件。(http://192.168.48.36/src/driver/tl_platform_src/merge_requests/1195,http://192.168.48.36/src/driver/tl_platform_src/merge_requests/1213)
  * (TL721X)修改PM相关的接口都使用O2优化选项。(http://192.168.48.36/src/driver/tl_platform_src/merge_requests/1238)
* **usb**
  * (TL721x/TL321x)删除 usbhw.c 中未使用的头文件 wchar.h。(http://192.168.48.36/src/driver/tl_platform_src/merge_requests/1237)
* **calibration**
  *  (B91/B92)给user_calib_freq_offset接口添加frequency_offset_value范围限制(http://192.168.48.36/src/driver/tl_platform_src/merge_requests/1247)
* **rf**
  * (TL721x/TL321x)函数rf_clr_dig_logic_state、rf_reset_register_value类型修改为_attribute_ram_code_sec_noinline_；函数rf_dma_reset类型修改为static _always_inline。(http://192.168.48.36/src/driver/tl_platform_src/merge_requests/1251)

### Performance Improvements

* **clock**
  * (TL721x/TL321x)为了节省功耗，在不使用24M RC和PLL的时候，分别关闭24M RC/PLL。(http://192.168.48.36/src/driver/tl_platform_src/merge_requests/1195,http://192.168.48.36/src/driver/tl_platform_src/merge_requests/1213)
* **S**
  * (TL721x/TL321x)为了节省功耗，初始化时关掉gpio输入功能。(http://192.168.48.36/src/driver/tl_platform_src/merge_requests/1195,http://192.168.48.36/src/driver/tl_platform_src/merge_requests/1213)
* **rf**
  * (TL721x)rf_mode_init()内部添加优化TX功耗的相关配置，并根据最新的驱动配置更新rf_power_Level_list表。(http://192.168.48.36/src/driver/tl_platform_src/merge_requests/1234)
  * (TL321x)优化 tx vant 模式下的功耗，提高 vant 最大输出功率，并根据最新的驱动配置更新rf_power_Level_list表(http://192.168.48.36/src/driver/tl_platform_src/merge_requests/1213,http://192.168.48.36/src/driver/tl_platform_src/merge_requests/1234)
  * (TL321x)优化RX sensitivity 性能，添加rf_rx_performance_mode接口及rf_rx_performance_e选择RX灵敏度配置，默认配置为RF_RX_LOW_POWER；选择RF_RX_HIGH_PERFORMANCE性能会提升1dbm,但会增加RX功耗(http://192.168.48.36/src/driver/tl_platform_src/merge_requests/1213,http://192.168.48.36/src/driver/tl_platform_src/merge_requests/1234,http://192.168.48.36/src/driver/tl_platform_src/merge_requests/1265)
  * (TL721x/TL321x)提升 BLE500K 和 BLE125K per floor 性能。(http://192.168.48.36/src/driver/tl_platform_src/merge_requests/1232)(http://192.168.48.36/src/driver/tl_platform_src/merge_requests/1260)
  
---

## V3.1.0

### Version

* SDK version: tl_platform_sdk V3.1.0
* This version of the SDK supports TLSR921x/TLSR951x(A0/A1/A2),TLSR922x/TLSR952x(A3/A4),TL721X(A1),TL321X(A0) chips.
* Toolchain version
  - TLSR921x/TLSR951x: gcc7
  - TLSR922x/TLSR952x: gcc12
  - TL721x: gcc12
  - TL321x: gcc12


### Bug Fixes

* **pm**
  * (TL721x)Solved the problem of exiting the sleep function or causing a reset due to the incorrect position of the clear wake source state operation.Clear the wake source status after setting the wake tick.The process of setting the wake up tick value may generate an intermediate value. If the intermediate value is the same as the 32k tick value, the state of the timer wake up source will be set, causing the exit of the sleep function or a reset.[2b4ddd52](http://192.168.48.36/src/driver/tl_platform_src/merge_requests/1147/commits?commit_id=2b4ddd52a79331bd344548bd5bed1d5f1847f6f8)
  * (B91/B92)Solved the problem that pm_set_wakeup_time_param and pm_set_xtal_stable_timer_param interfaces calculate the early wake time of sleep incorrectly. If you call either of these interfaces, you may not be able to sleep properly.[2f9b7be4](http://192.168.48.36/src/driver/tl_platform_src/merge_requests/1159/commits?commit_id=2f9b7be4ff8e2c0018180a9df7d85d6b4c1f0db3)
* **i2c1_m** 
  * (TLSR922x/TLSR952x/TL721X/TL321X)I2C1_M_WAIT:xxx_timeout_handler error passing parameter.[5fbb0b](http://192.168.48.36/src/driver/tl_platform_src/merge_requests/1162/commits?commit_id=5fbb0bc295c5a7e9e1b976a48dea8dc72333ac61)
* **RF_Demo** 
  * (TL721X/TL321X)Fixed data overflow during header variable assignment in app_pri_generic_mode.c[0d5bc4](http://192.168.48.36/src/driver/tl_platform_src/merge_requests/1160/commits?commit_id=0d5bc466a3c121324dee848c4f245b1080d592ae)
  * (TL721x/TL321x)Fixed rf_set_irq_mask parameter passing error in app_pri_generic_mode.c[0d5bc4](http://192.168.48.36/src/driver/tl_platform_src/merge_requests/1160/commits?commit_id=0d5bc466a3c121324dee848c4f245b1080d592ae)
* **rf** 
  * (TL721X/TL321X)Fixed issue where switching between different RF modes using rf_set_xx_mode could be unsuccessful (in cases where the rf_reset_register_value interface was not called before switching modes)[0d5bc4](http://192.168.48.36/src/driver/tl_platform_src/merge_requests/1160/commits?commit_id=0d5bc466a3c121324dee848c4f245b1080d592ae)
  * (TL321x) Fix the rf_mode_init configuration error to avoid triggering RF DMA and state machine reset during invocation.[373c5d](http://192.168.48.36/src/driver/tl_platform_src/merge_requests/1149/commits?commit_id=373c5dd45fc45afab5b38f5ea572afadcd7428a0)

### BREAKING CHANGES 

* **rf**
  * (TL321x) Close the stimer_bb module in `rf_rode_init`interface; Delete the interfaces `rf_bb_timer_sync_to_stimer_en`, `rf_bb_timer_sync_to_stimer_trig`, `rf_bb_timer_get_tick`and `rf_set_timer_source`.
* **flash**
  * (B91/B92/TL721x/TL321X)Removed flash_read_data()/flash_read_data_decrypt_check(), and changed the default single-wire of flash_read_page() interface in flash.c to default two-wire to prevent the mismatch between the maximum frequency supported by flash single-wire read and the frequency configured by the chip from leading to flash read error. 

### Features

* **TL321X**
  * PKE/PM
* **pm**
  * (TL721x) Added interface pm_set_cfg_for_os_compile_opt(), added enumeration pm_optimize_sel_e, and added code related to os optimization options.
  * (TL721X) In the return value of the pm_sleep_wakeup interface, add two bit definitions, indicating that the stimer tick value is greater than the maximum value and less than the minimum value respectively.
* **sys**
  * (TL721x) Added the DCDC_0P94_LDO_1P8 and DCDC_0P94_DCDC_1P8 modes.
* **pm demo**
  * Added 24M RC calibration and 32K clock recommended usage.
* **core**
  * (TLSR921x/TLSR951x/TLSR922x/TLSR952x/TL721X/TL321X) The core_mie_disable interface adds a return value to return the value of the MIE register before interrupts were disabled.
* **trap**
  * (TLSR921x/TLSR951x/TLSR922x/TLSR952x/TL721X/TL321X) Added plic_all_interrupt_save_and_disable interface to save and disable all interrupt sources of PLIC, plic_all_interrupt_restore interface to restore all PLIC interrupt sources.
* **flash**
  * (B91/B92/TL721x/TL321X)add flash_4line_en() and flash_4line_dis().

### Refactoring

* **USB_Demo**
  * (TLSR921x/TLSR951x/TLSR922x/TLSR952x/TL721X/TL321X)The usb_cdc_tx_data_to_host function supports sending data of arbitrary length in a blocking fashion. usb_cdc_tx_data_to_host_non_block is added as a new non-blocking function, and sample code is provided.
* **rf**
  * (TL721x) Switch stimer_bb to manual mode in `rf_rode_init`, which triggers tick earlier.
* **aes**
  * (TL721x/TL321X)The AES section was deleted because it was not needed.

### Performance Improvements

* N/A

### Note

* N/A

<hr style="border-bottom:2.5px solid rgb(146, 240, 161)">


### 版本

* SDK版本: tl_platform_sdk V3.1.0
* 此版本SDK支持 TLSR921x/TLSR951x(A0/A1/A2),TLSR922x/TLSR952x(A3/A4),TL721X(A1),TL321X(A0) 芯片。
* 工具链版本
  - TLSR921x/TLSR951x: gcc7
  - TLSR922x/TLSR952x: gcc12
  - TL721x: gcc12
  - TL321x: gcc12


### Bug Fixes

* **pm**
  * (TL721X)解决了因为清唤醒源状态操作的位置不对引发的退出睡眠函数或者产生复位的问题。清唤醒源状态的操作需要放在设置唤醒tick之后。设置唤醒tick值的过程可能会产生中间值，如果中间值和32k tick值相同会把timer唤醒源的状态置起来，从而引发退出睡眠函数或者产生复位。[2b4ddd52](http://192.168.48.36/src/driver/tl_platform_src/merge_requests/1147/commits?commit_id=2b4ddd52a79331bd344548bd5bed1d5f1847f6f8)
  * (B91/B92)解决了pm_set_wakeup_time_param和pm_set_xtal_stable_timer_param接口对于睡眠的提前唤醒时间计算错误的问题。如果调用过这两个接口中的任意一个，可能会导致不能正常睡眠。[2f9b7be4](http://192.168.48.36/src/driver/tl_platform_src/merge_requests/1159/commits?commit_id=2f9b7be4ff8e2c0018180a9df7d85d6b4c1f0db3)
* **i2c1_m** 
  * (TLSR922x/TLSR952x/TL721X/TL321X)I2C1_M_WAIT:xxx_timeout_handler传参错误.[5fbb0b](http://192.168.48.36/src/driver/tl_platform_src/merge_requests/1162/commits?commit_id=5fbb0bc295c5a7e9e1b976a48dea8dc72333ac61)
* **RF_Demo** 
  * (TL721X/TL321X)修复了app_pri_generic_mode.c中header变量赋值过程中的数据溢出问题[0d5bc4](http://192.168.48.36/src/driver/tl_platform_src/merge_requests/1160/commits?commit_id=0d5bc466a3c121324dee848c4f245b1080d592ae)
  * (TL721x/TL321x)修复了app_pri_generic_mode.c中rf_set_irq_mask传参错误[0d5bc4](http://192.168.48.36/src/driver/tl_platform_src/merge_requests/1160/commits?commit_id=0d5bc466a3c121324dee848c4f245b1080d592ae)
* **rf**
  * (TL721x/TL321x)修复了使用rf_set_xx_mode切换不同RF模式可能不成功的问题（在切换模式之前没有调用rf_reset_register_value接口的情况）[0d5bc4](http://192.168.48.36/src/driver/tl_platform_src/merge_requests/1160/commits?commit_id=0d5bc466a3c121324dee848c4f245b1080d592ae)
  * (TL321x) 修复了rf_mode_init配置错误，避免调用时触发RF DMA和状态机复位。[373c5d](http://192.168.48.36/src/driver/tl_platform_src/merge_requests/1149/commits?commit_id=373c5dd45fc45afab5b38f5ea572afadcd7428a0)

### BREAKING CHANGES

* **rf**
  * (TL321x) `rf_mode_init`接口中关闭stimer_bb模块；删除`rf_bb_timer_sync_to_stimer_en`、`rf_bb_timer_sync_to_stimer_trig`、`rf_set_timer_source`、`rf_bb_timer_get_tick`接口。
* **flash**
  * (B91/B92/TL721x/TL321X)删除了flash_read_data()/flash_read_data_decrypt_check()，同时将flash.c中的flash_read_page()接口默认单线更改为默认两线，防止flash单线读支持的最大频率和芯片配置的频率不匹配导致flash读错误。

### Features

* **TL321X**
  * PKE/PM
* **pm**
  * (TL721x)添加接口pm_set_cfg_for_os_compile_opt()，添加枚举pm_optimize_sel_e，添加os优化选项相关的代码。
  * (TL721X)在pm_sleep_wakeup接口的返回值中，增加两个bit的定义，分别表示stimer tick值大于最大值和小于最小值。
* **sys**
  * (TL721X)新增DCDC_0P94_LDO_1P8模式和DCDC_0P94_DCDC_1P8模式。
* **pm demo**
  * 添加24M RC校准和32K时钟推荐使用方法。
* **core**
  * (TLSR921x/TLSR951x/TLSR922x/TLSR952x/TL721X/TL321X) core_mie_disable 接口增加一个返回值，用来返回中断禁止前 MIE 寄存器的值。
* **trap**
  * (TLSR921x/TLSR951x/TLSR922x/TLSR952x/TL721X/TL321X) 添加 plic_all_interrupt_save_and_disable 接口保存和禁止 PLIC 所有中断源，plic_all_interrupt_restore 接口恢复所有 PLIC 中断源。
* **flash**
  * (B91/B92/TL721x/TL321X)新增了flash_4line_en()/flash_4line_dis()。

### Refactoring

* **USB_Demo**
  * (TLSR921x/TLSR951x/TLSR922x/TLSR952x/TL721X/TL321X)usb_cdc_tx_data_to_host 函数支持以阻塞的方式发送任意长度数据，新增非阻塞发送函数 usb_cdc_tx_data_to_host_non_block 并提供示例代码。
* **rf**
  * (TL721x) `rf_mode_init`中切换stimer_bb为手动模式，手动模式更早触发tick。
* **aes**
  * (TL721x/TL321X)删掉了AES段，因为不需要这个段。

### Performance Improvements

* N/A

### Note

* N/A

----

  
## V3.0.0

### Version

* SDK version: tl_platform_sdk V3.0.0
* This version of the SDK supports TLSR921x/TLSR951x(A0/A1/A2),TLSR922x/TLSR952x(A3/A4),TL721X(A1),TL321X(A0) chips.
* Toolchain version
  - TLSR921x/TLSR951x: gcc7
  - TLSR922x/TLSR952x: gcc12
  - TL721x: gcc12
  - TL321x: gcc12

### Note
* (TLSR922x/TLSR952x)Update toolchains from V5.1.1 to V5.3.0
  - The compilation options for warnings are uniformly configured as follows: -Wall -Wextra -Wshadow -Wimplicit-fallthrough -Wpointer-arith -Wredundant-decls -Wcast-qual -Wsign-compare -Wunused-parameter -Wunused-variable -Wswitch -Wstrict-prototypes -Wmissing-field-initializers -Wdeprecated-declarations -Wenum-conversion -Wpacked-not-aligned -Waddress-of-packed-member -Wundef
  - The compilation options for compiling, assembling and linking are configured uniformly as follows: -mcpu=d25f -mext-dsp -fomit-frame-pointer -fno-strict-aliasing -std=c99 -fno-jump-tables -fno-fat-lto-objects -fuse-ld=bfd
  - Solve the following file compilation error problem:
      - demo/vendor/PKE_Demo/B91_B92_PKE_Demo_v1.0.1/PKE_Demo/app_ecdsa.c, app_rsa.c
      - demo/vendor/UART_DEMO/app_dma.c
      - demo/vendor/I2C_Demo/app.c, app_dma.c
* Cleared the annotation for the following driver files:
  - chip/B91/drivers: aes.c/aes.h, core.h
  - chip/B92/drivers: aes.c/aes.h, core.h
* (TLSR921x/TLSR951x/TLSR922x/TLSR952x)In order to deal with the portability issues caused by the -fpack-struct compilation option, the following file has been modified by adding the compilation option `__attribute__((packed))` to structures:
    - demo/vendor/USB_Demo/application/usbstd/CDCClassCommon.h
    - demo/vendor/USB_Demo/application/rf_frame.h
    - common/usb_dbg/myudb_usbdesc.h
* (TLSR921x/TLSR951x/TLSR922x/TLSR952x)To make the watchdog coverage more extensive and the code more secure, enable the timer watchdog in the .S file and disable it in the platform_init() function.
* (TLSR922x/TLSR952x)the following modules add a timeout mechanism,clock/analog/mspi/ic/uart/spi/aes/trng,
       for compatibility,the timeout is set to a large value (0xffffffff us) by default, if want to use the timeout mechanism:
       - modify the timeout via the xxx_set_error_timeout interface (refer to the interface notes for a description of the timeout);
       - the application can override xxx_timeout_handler to get exception details as appropriate(for details, refer to the comments of the interface description);
* Cleaned up the Chinese characters in the code, involving the following files:
  - 3rd-party\freertos-V5\include: task.h.
  - chip\B92\drivers\lib\include: sys.h
  - demo\vendor\AUDIO_Demo\AUDIO_V1.1\AUDIO_Demo: app_codec_0581.c
  - demo\vendor\SPI_Demo\SPI_V1.1\SPI_Demo: app_dma_llp.c

### Bug Fixes
* **audio**
  * (TLSR922x/TLSR952x)Fixed the problem of amic and line mono input data anomaly under 44.1k and 48k sample rate, involving interface audio_codec_swap_stream0_data. [3d3f31d9](http://192.168.48.36/src/driver/telink_b91m_platform_src/merge_requests/1034/diffs?commit_id=3d3f31d94888af0f2242fcef7e8107e729ca6776)
  * (TLSR922x/TLSR952x)Solved the problem of inconsistent volume between the left and right channels of 20bit audio data in stereo mode. [716ead90](http://192.168.48.36/src/driver/telink_b91m_platform_src/merge_requests/944/diffs?commit_id=716ead90a4e0184169b082395715481ac358a1bb)
  * (TLSR922x/TLSR952x)Solved the problem of abnormal dac output after dynamically switching the codec dac sampling rate. [d755dae7](http://192.168.48.36/src/driver/telink_b91m_platform_src/merge_requests/1086/diffs?commit_id=d755dae7d81454617c3ddc0290cf07cf4a30b17f)
* **SPI_Demo**
  * (TLSR922x/TLSR952x)Repair SPI_PROTOCOL in spi no dma mode is configured as B91M_SLAVE_PROTOCOL, and the master side sends data in wrong format, modify SPI_MODE_WR_DUMMY_WRITE to SPI_MODE_WR_WRITE_ONLY. [e171fa06](http://192.168.48.36/src/driver/telink_b91m_platform_src/merge_requests/1013/diffs?commit_id=e171fa060b221eac87bef74edadb3e296890bca2)
* **pm**
  * (TLSR922x/TLSR952x)Solved several problems that may occur when the sleep time set by the function pm_sleep_wakeup() is less than 1.62ms: 1. Entered sleep, but the sleep time did not meet expectations, and it can be awakened after about 37 hours. 2. If it happens to be after the pm_clr_all_irq_status() in the processing of pm_sleep_wakeup() and the time is up, it will cause the plic_clr_all_request() to not be cleared, triggering a software reboot. 3. If it happens to be at the moment of sleep and the time was out, it will result in not entering sleep and continuing to execute (when sleep mode is suspend/sleep retention) or software reboot (when sleep mode is deep). [bd44c301](http://192.168.48.36/src/driver/telink_b91m_platform_src/merge_requests/769/diffs?commit_id=bd44c3017fd31dbb3c0808b5c8bbcb6645f38ea2)
  * (TLSR922x/TLSR952x)Solved the problem that may be caused by treating the VBUS flag bit as a wake-up source flag bit in pm_sleep_wakeup(): If jitter occurs on the VBUS pin after the pm_clr_all_irq_status() in the processing of pm_sleep_wakeup(), it will cause the plic_clr_all_request() to not be cleared, triggering a software reboot. [bd44c301](http://192.168.48.36/src/driver/telink_b91m_platform_src/merge_requests/769/diffs?commit_id=bd44c3017fd31dbb3c0808b5c8bbcb6645f38ea2)
  * (TLSR921x/TLSR951x)Solved the crash problem caused by the failure of the crystal oscillator stability flag. The program determines whether to switch from 24M rc to 24M xtal by reading the stable flag of the hardware crystal oscillator. If the crystal used starts slowly and the flag of the crystal oscillator is ready, but the actual crystal oscillator does not start normally, the program will crash after switching to 24M XTAL. [3aa5ca13](http://192.168.48.36/src/driver/telink_b91m_platform_src/merge_requests/936/diffs?commit_id=3aa5ca1363a8805e274dccf03320f584ab75e477)
  * (TLSR921x/TLSR951x/TLSR922x/TLSR952x)Restore the default value of register system_up_32k before sleep, which solves the problem of inaccurate sleep time. If the default value is not restored, the system tick updates every 32k edge. Because the system tick is not updated in a timely manner, the XTAL may be misjudged that it is not stable. If the system tick executes 40us more than once, the reserved waiting time may be exceeded, resulting in inaccurate sleep time. [e8dc8082](http://192.168.48.36/src/driver/telink_b91m_platform_src/merge_requests/1078/diffs?commit_id=e8dc80828f0d9050833b55be48764dcce8733a0a)
* **sys**
  * (TLSR922x/TLSR952x)The A3 chip has a problem (the A4 doesn't) : If the hardware board has a 1.4V dcdc inductor component and uses 1.8V GPIO, it is necessary to set 1P4V to DCDC mode as soon as possible after the chip is powered on, otherwise there is a voltage pulse on vdd1v2 and vddo3. Before moving code operations in the S file, configure the DCDC mode to solve this problem. [875a487e](http://192.168.48.36/src/driver/telink_b91m_platform_src/merge_requests/910/diffs?commit_id=875a487edbb3191372e255369c566f4222d0b74c)
* **USB_Demo**
  * (TLSR922x/TLSR952x)Solved the problem that DMIC0_DMIC1_IN cannot transfer DMIC0 data in USB_MICROPHONE mode. [716ead90](http://192.168.48.36/src/driver/telink_b91m_platform_src/merge_requests/944/diffs?commit_id=716ead90a4e0184169b082395715481ac358a1bb)
 * **link**
   * (TLSR921x/TLSR951x/TLSR922x/TLSR952x)make _BSS_VMA_END four-byte alignment to resolve when using the S file _FILL_STK function,due of the _BSS_VMA_END may not have four-byte alignment, there is an out-of-bounds access beyond _STACK_TOP, resulting in a store access fault. [0e732bce](http://192.168.48.36/src/driver/telink_b91m_platform_src/merge_requests/972/diffs?commit_id=0e732bcea2887804489ad6db251a281f26796218)
* **Timer_Demo**
  * (TLSR921x/TLSR951x/TLSR922x/TLSR952x)Wd_clear() must be executed before each call to wd_start() to avoid abnormal watchdog reset time because the initial count value is not 0. [20a065ec](http://192.168.48.36/src/driver/telink_b91m_platform_src/merge_requests/1000/diffs?commit_id=20a065ece53185549c748bf99d5780029def6180)
* **spi demo** 
  * (TLSR922x/TLSR952x)Fix lspi duplicate call to gspi_reset in interrupt dma_irq_handler. [bfadf96e](http://192.168.48.36/src/driver/telink_b91m_platform_src/merge_requests/1009/diffs?commit_id=bfadf96ed151e483bbbbf0da00d9831c8f25d2ef)
* **qdec_Demo**
  * (TLSR922x/TLSR952x)qdec_get_count_value: If the interface is called continuously (with very short intervals), it is possible that the data read is not updated in time and the result is read from the last time. [ecf387fa](http://192.168.48.36/src/driver/telink_b91m_platform_src/merge_requests/1064/diffs?commit_id=ecf387fab552255396dd584107535693c01a617d)
* **rf**
  * (TLSR921x/TLSR951x/TLSR922x/TLSR952x)Add the missing active state of the reg_rf_ll_2d_sclk register bit<0-2>. [d7ef39ba](http://192.168.48.36/src/driver/telink_b91m_platform_src/merge_requests/1087/diffs?commit_id=d7ef39ba526cdf736cf4ea5b75c79c86366b609c)
* **Coremark_demo**
  * (TLSR921x/TLSR951x/TLSR922x/TLSR952x)Fix the compilation options for the project to ensure that the CPU runs properly. [791247df](http://192.168.48.36/src/driver/telink_b91m_platform_src/merge_requests/894/diffs?commit_id=791247df9dba930661b988a44fc55f3898d3c718)
* **Dhrystone_Demo**
  * (TLSR921x/TLSR951x/TLSR922x/TLSR952x)Fix the compilation options for the project to ensure that the CPU runs properly. [791247df](http://192.168.48.36/src/driver/telink_b91m_platform_src/merge_requests/894/diffs?commit_id=791247df9dba930661b988a44fc55f3898d3c718)
* **flash**
  * (TLSR921x/TLSR951x/TLSR922x/TLSR952x)Fix the problem that multiple calls to flash_write_status_midxxx() result in wrong return value. [0634358a](http://192.168.48.36/src/driver/telink_b91m_platform_src/merge_requests/984/diffs?commit_id=0634358ade7aaedb2939b14c8b4ebb49dfe131ce)
* **RF_Demo**
  * (TLSR921x/TLSR951x/TLSR922x/TLSR952x)Fixed a compilation error caused by undefined TX_DELAY_US in the rf demo. [3fc3d848](http://192.168.48.36/src/driver/telink_b91m_platform_src/merge_requests/904/diffs?commit_id=3fc3d848715b84cdcd763af3613aa4fb99477a86)

### BREAKING CHANGES 
* **sys**
  * (TLSR922x/TLSR952x)Refactor the nop related macro definition to avoid stack overflow in GCC. Simultaneously updated the names of nop related macro definitions to make them more reasonable.
  * (TLSR921x/TLSR951x/TLSR922x/TLSR952x)Add the cap argument to the sys_init function to configure whether the internal capacitor needs to be turned off. If the development board has already welded the capacitor on the outside, then the internal capacitor needs to be turned off; If the development board does not weld the capacitor on the outside, then the internal capacitor needs to be opened. If the corresponding operation is not possible, there may be two capacitors in parallel, or no capacitors. This has a great impact on the starting time and frequency of crystal vibration, which may cause reset and crash.
* **pm/clock**
  * (TLSR921x/TLSR951x/TLSR922x/TLSR952x)Replace the method of obtaining microsecond level delay before the system timer starts working with core_cclk_delay_tick() instead of nop delay.
* **pm**
  * (TLSR921x/TLSR951x)Deleted DCDC_1P4_DCDC_1P8 from the enumeration power_mode_e. When the power supply voltage exceeds 3.8V and the DCDC_1P8 configuration is used, the flash voltage may exceed the power supply range when the LDO is switched to the DCDC.
  * (TLSR921x/TLSR951x)PM_ANA_REG_WD_CLR_BUF0(0x38), PM_ANA_REG_POWER_ON_CLR_BUF0(0x39) register all bits are for driver use only.
  * (TLSR922x/TLSR952x)PM_ANA_REG_WD_CLR_BUF0(0x35), PM_ANA_REG_POWER_ON_CLR_BUF0(0x3a) register all bits are for driver use only.
* **rf**
  * (TLSR921x/TLSR951x/TLSR922x/TLSR952x)The rf_mode_init() function has added an RF Rx DCOC software calibration scheme to solve the problem of poor Rx sensitivity performance in some chips with large DC-offset.
         The execution time of this function will be longer, you can check the function comment for the specific time.
         Due to the use of software DCOC's calibration scheme, it is not necessary and not allowed to use rf_get_dcoc_cal_val and rf_set_dcoc_cal_val for processing in the fast settle function,
         Therefore, the above two interfaces were deleted, and the structure rf_dcoc_cal_t was also deleted;
* **flash**
  * (TLSR921x/TLSR951x/TLSR922x/TLSR952x)Add the return value for the flash_write_status_midxxxxxx,flash_lock_midxxxxxx and flash_unlock_midxxxxxx interface.
* **stimer**
  * (TLSR921x/TLSR951x/TLSR922x/TLSR952x)Delete interface stimer_enable/stimer_disable.
* **USB_Demo**
  * (TLSR921x/TLSR951x/TLSR922x/TLSR952x) Since the -fshort-wchar compilation option was removed, using wide characters to return usb string descriptors no longer works, so use the usbdesc_get_str()/myudb_usbdesc_get_str() function to get the usb string descriptor and remove the associated usb string descriptor constants.
* **flash**
  * (TLSR922x/TLSR952x)Add the return value for the flash_write_status_midxxxxxx,flash_lock_midxxxxxx and flash_unlock_midxxxxxx interface.
* **stimer**
  * (TLSR921x/TLSR951x/TLSR922x/TLSR952x)Delete interface stimer_enable/stimer_disable。
* **USB_Demo**
  * (TLSR921x/TLSR951x/TLSR922x/TLSR952x) Since the -fshort-wchar compilation option was removed, using wide characters to return usb string descriptors no longer works, so use the usbdesc_get_str()/myudb_usbdesc_get_str() function to get the usb string descriptor and remove the associated usb string descriptor constants.
* **rf**
  * (TLSR921x/TLSR951x) For the convenience of users, the fast settle-related interfaces have been re-encapsulated. The new code will affect compatibility. Please refer to rf_demo and app_fast_settle_test.c for usage instructions. (Interfaces rf_get_ldo_trim_val, _set_ldo_trim_val, rf_get_hpmc_cal_val, rf_set_hpmc_cal_val, rf_get_dcoc_cal_val, rf_set_dcoc_cal_val, rf_tx_fast_settle_init, and rf_rx_fast_settle_init have been removed; interfaces rf_fast_settle_config, rf_tx_fast_settle_update_cal_val, and rf_rx_fast_settle_update_cal_val have been added.)
  * (TLSR922x/TLSR952x) For the convenience of users, the fast settle-related interfaces have been re-encapsulated. The new code will affect compatibility. Please refer to rf_demo and app_fast_settle_test.c for usage instructions. (Interfaces rf_rx_fast_settle_dis, rf_get_ldo_trim_val, rf_set_ldo_trim_val, rf_get_hpmc_cal_val, rf_tx_fast_settle_init, and rf_rx_fast_settle_init have been removed; interfaces rf_fast_settle_config, rf_tx_fast_settle_update_cal_val, and rf_rx_fast_settle_update_cal_val have been added.)
* **calibration**
  * (TLSR921x/TLSR951x/TLSR922x/TLSR952x)Deleted the operation on enabling the internal capacitor in the user_read_flash_value_calib (TLSR921x/TLSR951x) /calibration_func(TLSR922x/TLSR952x) interface. The calibration interface will adjust the voltage and capacitance values, so it will be called most of the time, but for 24M crystals using an external capacitor,if the internal capacitor is opened when the calibration interface is called, the two capacitors will be in parallel, affecting the crystal vibration, so this operation is deleted.
* **error_handler**              
  * drv_set_error_timeout: Delete TIMEOUT_US and change it to global variable g_drv_api_error_timeout_us, add the above interfaces for modification to avoid dynamic modification of the lib library due to macro definition.
  * error_code_e: Change the value to drv_api_error_code_e to prevent the same name.
  * get_error_code/timeout_handler: Change it to drv_get_error_code/drv_timeout_handler to prevent the same name.
* **flash_demo**
  * (TLSR921x/TLSR951x/TLSR922x/TLSR952x):In order to realize the bin firmware protection in driver sdk, delete the flash_lock/flash_unlock swtich implementation scheme maintained by flash_demo, for the sake of code simplicity and easy maintenance, add the flash hal layer to realize the scheme with structure, add the hal_flash.c/.h file and add the bin firmware protection function in platform_init. platform_init to add the bin firmware protection function.

### Features
* **TL721X**
  * ADC/Analog/CHACHA20_POLY1305/Debug/Flash/GPIO/HASH/I2C/PKE/PM/PWM/QDEC/RF/SKE/SPI/STIMER/TIMER/TRAP/TRNG/UART/USB
* **TL321X**
  * ADC/Analog/Debug/Flash/GPIO/I2C/PWM/QDEC/RF/SKE/SPI/STIMER/TIMER/TRAP/TRNG/UART/USB
* **alg_reg**
  * (TLSR921x/TLSR951x/TLSR922x/TLSR952x)Delete the following interfaces: analog_write_reg32_dma, analog_read_reg32_dma, analog_write_buff_dma, analog_read_buff_dma, analog_write_addr_data_dma. This is because dma is not used in real applications, and other interfaces already meet the requirements.
  * (TLSR921x/TLSR951x/TLSR922x/TLSR952x)Limit the analog_write_buff interface data length cannot exceed 8 bytes.
* **spi** 
  * (TLSR922x/TLSR952x)Fix the compilation warning problem caused by the mismatch of gspi_change_csn_pin and spi_master_config enumeration parameters, which does not affect the function.
* **rf**  
  * (TLSR921x/TLSR951x/TLSR922x/TLSR952x) Added rf_set_rx_dcoc_cali_by_sw interface to switch rx DCOC software calibration to optimise rf_mode_init execution time(This interface is only available in scenarios where only tx is used, not rx)
  * (TLSR921x/TLSR951x/TLSR922x/TLSR952x) Added the user_calid_rf_rx_dcoc interface to obtain the dcoc calibration values in flash
* **flash**
  * (TLSR921x/TLSR951x/TLSR922x/TLSR952x)Added interfaces: flash_4line_en, flash_4line_dis.
* **USB**
  * (TLSR921x/TLSR951x)Add usbhw_set_ep_addr() interface to set endpoint buffer allocation; usbhw_set_eps_max_size() interface to set the maximum buffer size of the IN endpoints.
  * (TLSR922x/TLSR952x)Add usbhw_set_eps_max_size() interface to set the maximum buffer size of the IN endpoints.
* **SDK VERSION**  
  * (TLSR921x/TLSR951x/TLSR922x/TLSR952x)Added 'tl_Check_Fw.sh' script, print SDK version information after compiling the code.
* **stimer**
  * (TLSR921x/TLSR951x)Added the interface pm_clr_irq_status/pm_set_wakeup_src/stimer_set_32k_track_cnt/stimer_get_32k_track_cnt/stimer_get_tracking_32k_value.
  * (TLSR922x/TLSR952x)Added the interface pm_set_wakeup_src/stimer_set_32k_track_cnt/stimer_get_32k_track_cnt/stimer_get_tracking_32k_value.
* **core**
  * (TLSR921x/TLSR951x/TLSR922x/TLSR952x)Add a new core.c file, which includes the core_cclk_delay_tick interface; Add rdmcycle and core_cclk_delay_tick interfaces.
* **drivers**
  * (TLSR922x/TLSR952x)Add a new folder error_handler, including the error_handler.c/error_handler.h file, which includes the get_error_code and timeout_handler interfaces.
* **sys**
  * (TLSR922x/TLSR952x)Added the interface sys_set_dcdc_1pP4_ldo_2p0 to solve the issue on A3(A4 not have) that if there is a 1.4V dcdc inductor component on the hardware board and a 1.8V GPIO is used will cause a voltage pulse on vdd1v2 and vddo3.
  * (TLSR922x/TLSR952x) Added efuse_get_chip_id interface. Note: Only A3 and later are written as chip id values.
* **flash**
  * (TLSR922x/TLSR952x)Add 1M flash(GD25LQ80E) driver.
  * (TLSR922x/TLSR952x)Add flash_write_config_mid146085() and modify flash_lock_mid146085() and flash_unlock_mid146085() to adapt the second-supply flash P25Q80SU.
* **sys**
  * (TLSR921x/TLSR951x/TLSR922x/TLSR952x)Add crystal_manual_settle() function into sys_init() and pm_sleep_wakeup() in order to quickly start the crystal oscillator.
* **pm**
  * (TLSR921x/TLSR951x)Added pm_set_xtal_stable_timer_param() to set crystal related parameters when the crystal starts to vibrate relatively slowly.
* **flash**
  * (TLSR921x/TLSR951x)add new flash PY25Q128H driver and demo.(note:The flash sector erase time is 240ms maximum.)
  * (TLSR921x/TLSR951x/TLSR922x/TLSR952x)add flash_get_capacity() and check flash capacity demo.
* **flash**
  * (TLSR922x/TLSR952x)add new flash GD25LQ32E/GD25LE32E driver and demo.  
* **core**
  * wait_condition_fails_or_timeout/wait_condition_fails_or_timeout_with_param:Added the above interface to provide uniform timeout handling for all modules;
* **error_handler**              
  * drv_api_status_e:Added enumeration to provide a uniform return state for the new interface;
* **trng**
  * Added timeout mechanism, corresponding new interface trng_hw_reset/trng_set_error_timeout/trng_timeout_handler;
* **aes**
  * Added timeout mechanism, corresponding new interface aes_hw_fsm_reset/aes_set_error_timeout/aes_timeout_handler;
* **i2c**
  * Added timeout mechanism, corresponding new interface i2c_hw_fsm_reset/i2c_set_error_timeout/i2c_get_error_timeout_code/i2c_timeout_handler;
* **i2c1_m**
  * Added timeout mechanism, corresponding new interface i2c1_m_hw_fsm_reset/i2c1_m_set_error_timeout/i2c1_m_get_error_timeout_code/i2c1_m_timeout_handler;
* **uart**
  * Added timeout mechanism, corresponding new interface uart_set_error_timeout/uart_get_error_timeout_code/uart0_timeout_handler/uart1_timeout_handler;
* **spi**
  * Added timeout mechanism, corresponding new interface,spi_set_error_timeout/spi_get_error_timeout_code/lspi_timeout_handler/gspi_timeout_handler/spi_hw_fsm_reset; 
* **EMI_BQB_Demo** 
  * (TLSR921x/TLSR951x/TLSR922x/TLSR952x)Add frequency hopping function for emi_con_tx55(); and emi_con_tx0f();

### Refactoring
* **GPIO_Demo**
  * (TLSR921x/TLSR951x/TLSR922x/TLSR952x)Optimized GPIO_Demo, merged shared interfaces and added comments.
* **gpio**
  * (TLSR921x/TLSR951x/TLSR922x/TLSR952x)Updated gpio_shutdown() comment: gpio_shutdown(GPIO_ALL) is not recommended to be used in applications, if it is used it needs to be called with the appropriate pins enabled after the call depending on the application.
* **aes**
  * (TLSR922x/TLSR952x)Removed declaration of aes_rpa_match() from aes.h.
* **rf**
  * (TLSR921x/TLSR951x/TLSR922x/TLSR952x)The default values for all masks of chip RF are open. For user convenience, It will be turned off in the sys_init function and configured by the user afterwards
* **RF_DEMO** 
  * (TLSR921x/TLSR951x/TLSR922x/TLSR952x)Replace the call to stimer_get_tick() in RF_Demo with rf_stimer_get_tick().
* **AES_Demo**
  * (TLSR921x/TLSR951x/TLSR922x/TLSR952x)Optimized AES_Demo and standardizes the macro definition names for different modes of AES, added AES_ECB,AES_CBC mode, removed AES_RPA mode.
* **USB_Demo**
  * (TLSR921x/TLSR951x/TLSR922x/TLSR952x)CDC endpoints are configurable and default to USB_EDP4_IN and USB_EDP5_OUT.The CDC sends data using the endpoint busy bit instead of the delay_us to determine if the data is sent to completion.
  * (TLSR921x/TLSR951x/TLSR922x/TLSR952x)Use usb_init() instead of usb_init_interrupt() to initialize the USB. (usbhw_set_eps_en() is required to enable the used endpoints if using the usb_init() interface.)
  * (TLSR921x/TLSR951x/TLSR922x/TLSR952x)Optimized mouse demo and keyboard demo, using matrix keys to trigger the hid report.
* **I2C_Demo**
  * (TLSR921x/TLSR951x/TLSR922x/TLSR952x)Optimized I2C_Demo, standardize the differentiation of different patterns and supplement annotations.
* **flash**
  * (TLSR921x/TLSR951x/TLSR922x/TLSR952x)Adjusted the parameter type of flash_write_status_midxxxxxxxx() and added parameter error return value for TLSR922x/TLSR952x.
  * (TLSR922x/TLSR952x)Modify the mspi busy wait code in the flash_send_cmd and flash_is_busy interfaces to call the mspi_wait interface instead.
* **mspi**
  * (TLSR922x/TLSR952x)Modify the mspi_is_busy interface to the mspi_wait interface; Modify the mspi busy waiting code in the mspi_write, mspi_read, and mspi_stop_xip interfaces to call the mspi_wait interface instead.
* **analog** 
  * (TLSR922x/TLSR952x)Add timeout handling in the analog_wait interface.
* **clock**
  * (TLSR922x/TLSR952x)Add timeout handling in clock_cal_24m_rc and clock_cal_32k_rc interface.
* **project**
  * (TLSR921x/TLSR951x/TLSR922x/TLSR952x)Remove the -fshort-wchar compilation option.
* **GPIO_DEMO**
  * (TLSR921x/TLSR951x/TLSR922x/TLSR952x) Set the default GPIO_MODE to GPIO_IRQ; Modify the GPIO_HIGH_RESISTOR mode logic to stop while(1).
* **EMI_BQB_DEMO**
  * (TLSR921x/TLSR951x)Since the dcdc mode has been deleted, the relevant codes of dcdc are masked.
  * (TLSR921x/TLSR951x/TLSR922x/TLSR952x)Adjust the fast settle configuration code in bqb to adapt to the new interface.
  * (TLSR921x/TLSR951x/TLSR922x/TLSR952x) Optimise emi, bqb main.c initialisation part of the code, and also optimise app_emi.c code to adapt to different chips.
* **emi**
  * (TLSR921x/TLSR951x/TLSR922x/TLSR952x) Optimise rf_emi_tx_burst_loop and rf_emi_tx_burst_setup interfaces by replacing the payload len in the interface with the macro EMI_TX_PKT_PAYLOAD
  * (TLSR921x/TLSR951x/TLSR922x/TLSR952x)Move emi.c and emi.h to lib file.
  * (TLSR921x/TLSR951x/TLSR922x/TLSR952x)Standardized comments for emi-related functions.
* **rf**
  * (TLSR921x/TLSR951x/TLSR922x/TLSR952x)Optimize the rf_tx_fast_settle_en、rf_tx_fast_settle_dis、rf_rx_fast_settle_en、rf_rx_fast_settle_dis、rf_set_chn related interface.
  * (TLSR921x/TLSR951x/TLSR922x/TLSR952x)Standardized comments for rf-related functions.
* **RF_Demo**
  * (TLSR921x/TLSR951x/TLSR922x/TLSR952x)add fast settle test demo.
* **trng**
  * (TLSR922x/TLSR952x)trng_init: for timeout mechanism, added a return value in the above interface;
* **aes**
  * (TLSR922x/TLSR952x)aes_encrypt/aes_encrypt_bt_en/aes_decrypt/aes_decrypt_bt_en:for timeout mechanism, added a return value(DRV_API_TIMEOUT) in the above interface;
* **i2c**
  * (TLSR922x/TLSR952x)i2c_master_write/i2c_master_read/i2c_master_write_read:for timeout mechanism, added a return value(DRV_API_TIMEOUT) in the above interface;
* **i2c1_m**
  * (TLSR922x/TLSR952x)aes_encrypt/aes_encrypt_bt_en/aes_decrypt/aes_decrypt_bt_en:for timeout mechanism, added a return value(DRV_API_TIMEOUT) in the above interface;
* **uart**
  * (TLSR921x/TLSR951x/TLSR922x/TLSR952x)uart_reset: To better fit the function, name the interface uart_hw_fsm_reset. For compatibility, use the redefine #define uart_reset uart_hw_fms_reset;
  * (TLSR922x/TLSR952x)uart_send_byte/uart_send_hword/uart_send_word: for timeout mechanism, added a return value in the above interface;
* **spi**
  * (TLSR922x/TLSR952x)spi_master_send_cmd/spi_write/spi_read/spi_master_write/spi_master_write_read/spi_master_write_plus/spi_master_write_repeat/spi_master_write_repeat_plus/spi_master_read_plus/spi_master_write_read_plus: for timeout mechanism, added a return value in the above interface;
* **spi demo** 
  * (TLSR922x/TLSR952x)replace the spi_is_busy() interface in no_dma and dma modes with the timeout mechanism spi interface SPI_WAIT_BUSY, and modify the processing logic that uses the spi_master_get_slave_ready() interface (add a timeout condition and handle it);
* **display demo** 
  * (TLSR922x/TLSR952x)replace lcd_st7796h2,oledmr69330 spi_is_busy() interface with timeout mechanism spi interface SPI_WAIT_BUSY;
* **audio**
  * (TLSR921x/TLSR951x)The audio codec registers have been changed from using the default value directly to writing the default value again to prevent any functional issues caused by the default value being changed in abnormal situations.
* **ALL**
  * (TLSR921x/TLSR951x/TLSR922x/TLSR952x)Replace tabs with spaces in all .c and .h files except the chip directory.
* **GPIO**
  * (TLSR921x/TLSR951x/TLSR922x/TLSR952x)Correct the gpio_get_level() comment.
* **ALL_Demo**
  * (TLSR921x/TLSR951x/TLSR922x/TLSR952x)In order to improve the flexibility of use, the form #if is no longer used to distinguish between different chips, but #if defined.
* **ALG_REG_Demo**
  * (TLSR921x/TLSR951x/TLSR922x/TLSR952x) optimizes the variable definition in analog_write_halfword_test() to ensure that halfword data is read and written consistently.

### Performance Improvements

* **rf**
  * (TLSR921x/TLSR951x)The rf_mode_init() function uses DCOC software calibration to minimize the DC offset of the chip.
         At the same time, turn on the RX secondary filter in BLE S2 and S8 modes to filter out DC offset and noise as much as possible, in order to improve the chip's out of band anti-interference ability (including DC offset).
         These two operations can restore the sensitivity performance of chips with larger DC offset to the normal range.
         But it should be noted that opening the secondary filter will reduce the frequency offset resistance range of the chip to within ± 150kHz.

  * (TLSR922x/TLSR952x)The rf_mode_init() function uses dcoc software calibration to achieve the minimum DC offset for the chip. 
         At the same time, turn on the RX secondary filter in RF modes other than private 250k and 500k to filter out DC offset and noise as much as possible, in order to improve the chip's out of band anti-interference ability (including DC offset). 
         By performing these two operations, the sensitivity performance of chips with larger DC offset can be restored to the normal range, but it is important to note that opening the secondary filter will reduce the frequency offset resistance range of the chip to within ± 150kHz.

### Note
* (TLSR922x/TLSR952x)编译工具链从V5.1.1升级为V5.3.0。
  - 警告的编译选项统一配置为：-Wall -Wextra -Wshadow -Wimplicit-fallthrough -Wpointer-arith -Wredundant-decls -Wcast-qual -Wsign-compare -Wunused-parameter -Wunused-variable -Wswitch -Wstrict-prototypes -Wmissing-field-initializers -Wdeprecated-declarations -Wenum-conversion -Wpacked-not-aligned -Waddress-of-packed-member -Wundef
  - 编译，汇编和链接的编译选项统一配置为：-mcpu=d25f -mext-dsp -fomit-frame-pointer -fno-strict-aliasing -std=c99 -fno-jump-tables -fno-fat-lto-objects -fuse-ld=bfd
  - 解决以下的文件编译报错问题：
      - demo/vendor/PKE_Demo/B91_B92_PKE_Demo_v1.0.1/PKE_Demo/app_ecdsa.c, app_rsa.c
      - demo/vendor/UART_DEMO/app_dma.c
      - demo/vendor/I2C_Demo/app.c, app_dma.c
* 清理了以下驱动文件的注释：
    - chip/B91/drivers: aes.c/aes.h, core.h
    - chip/B92/drivers: aes.c/aes.h, core.h
* (TLSR921x/TLSR951x/TLSR922x/TLSR952x)为了处理编译选项 -fpack-struct 带来的代码移植性问题, 以下文件已经通过向结构体添加编译选项 `__attribute__((packed))` 进行了修改：
    - demo/vendor/USB_Demo/application/usbstd/CDCClassCommon.h
    - demo/vendor/USB_Demo/application/rf_frame.h
    - common/usb_dbg/myudb_usbdesc.h
* (TLSR921x/TLSR951x/TLSR922x/TLSR952x)为了让看门狗覆盖的范围更广，代码更安全，在 .S 文件中使能 timer 看门狗，并在 platform_init() 函数中关闭。
* (TLSR922x/TLSR952x)以下模块添加了超时机制:clock/analog/mspi/ic/uart/spi/aes/trng,为兼容起见,如果要使用超时机制,超时默认设置为较大值 (0xffffffff us):
    - 通过 xxx_set_error_timeout 接口修改超时(有关超时的说明,请参阅接口说明);
    - 应用程序可以重载 xxx_timeout_handler,以获取相应的异常详细信息(详情请参阅接口说明的注释);
* 清理了代码中存在的中文字符，涉及到以下文件：
  - 3rd-party\freertos-V5\include: task.h.
  - chip\B92\drivers\lib\include: sys.h
  - demo\vendor\AUDIO_Demo\AUDIO_V1.1\AUDIO_Demo: app_codec_0581.c
  - demo\vendor\SPI_Demo\SPI_V1.1\SPI_Demo: app_dma_llp.c

### Bug Fixes
* **audio**
  * (TLSR922x/TLSR952x)修复在44.1k和48k采样率下, amic和line单声道输入数据异常问题，涉及接口 audio_codec_swap_stream0_data。 [3d3f31d9](http://192.168.48.36/src/driver/telink_b91m_platform_src/merge_requests/1034/diffs?commit_id=3d3f31d94888af0f2242fcef7e8107e729ca6776)
  * (TLSR922x/TLSR952x)解决了立体声模式时 20bit 音频数据左右声道音量不一致的问题。 [716ead90](http://192.168.48.36/src/driver/telink_b91m_platform_src/merge_requests/944/diffs?commit_id=716ead90a4e0184169b082395715481ac358a1bb)
  * (TLSR922x/TLSR952x)解决了动态切换codec dac采样率后，dac输出异常问题。 [d755dae7](http://192.168.48.36/src/driver/telink_b91m_platform_src/merge_requests/1086/diffs?commit_id=d755dae7d81454617c3ddc0290cf07cf4a30b17f)
* **SPI_Demo**
  * (TLSR922x/TLSR952x)修复spi no dma模式下SPI_PROTOCOL配置为B91M_SLAVE_PROTOCOL，master端发送数据格式错误，将SPI_MODE_WR_DUMMY_WRITE修改成SPI_MODE_WR_WRITE_ONLY。 [e171fa06](http://192.168.48.36/src/driver/telink_b91m_platform_src/merge_requests/1013/diffs?commit_id=e171fa060b221eac87bef74edadb3e296890bca2)
* **pm**
  * (TLSR922x/TLSR952x)解决了当函数pm_sleep_wakeup()设置的睡眠时间小于1.62ms时，可能会引起的以下几种问题： 1.进入了睡眠，但睡眠时间不符合预期，约37小时后可以唤醒。 2.如果正好在pm_sleep_wakeup()函数中的pm_clr_all_irq_status()之后，时间到了，会导致plic_clr_all_request()清除不掉，触发软件reboot。 3.如果正好在睡眠的瞬间，时间到了，会导致不进入睡眠，继续执行（当sleep_mode是suspend/deep retention）或者软件reboot（当sleep_mode是deep）。 [bd44c301](http://192.168.48.36/src/driver/telink_b91m_platform_src/merge_requests/769/diffs?commit_id=bd44c3017fd31dbb3c0808b5c8bbcb6645f38ea2)
  * (TLSR922x/TLSR952x)解决了由于在pm_sleep_wakeup()中把VBUS标志位当成唤醒源标志位一样处理可能会引起的问题：即如果在pm_sleep_wakeup()函数中的pm_clr_all_irq_status()之后，VBUS引脚上产生抖动，会导致plic_clr_all_request()清除不掉，触发软件reboot。 [bd44c301](http://192.168.48.36/src/driver/telink_b91m_platform_src/merge_requests/769/diffs?commit_id=bd44c3017fd31dbb3c0808b5c8bbcb6645f38ea2)
  * (TLSR921x/TLSR951x)解决了因为晶振稳定标志位失效导致的死机问题。程序通过读硬件晶振稳定标志位判断是否从24M rc切换到24M xtal，如果使用的晶体起振较慢，晶振标志位ready但是实际晶振没有正常起震，切换到24M XTAL后，程序就会发生死机问题。 [3aa5ca13](http://192.168.48.36/src/driver/telink_b91m_platform_src/merge_requests/936/diffs?commit_id=3aa5ca1363a8805e274dccf03320f584ab75e477)
  * (TLSR921x/TLSR951x/TLSR922x/TLSR952x)进睡眠前恢复寄存器system_up_32k默认值，解决了睡眠时间不准的问题。如果不恢复默认值，system tick会在每个32k沿更新，由于system tick更新不及时，可能会误判XTAL还没有稳定，多判断一次，多执行40us，可能会超过预留的等待时间，而导致睡眠时间不准确。 [e8dc8082](http://192.168.48.36/src/driver/telink_b91m_platform_src/merge_requests/1078/diffs?commit_id=e8dc80828f0d9050833b55be48764dcce8733a0a)
* **sys** 
  * (TLSR922x/TLSR952x)A3芯片有个issue（A4没有这个问题）：如果硬件板子上有1.4V dcdc inductor component，并且用1.8V GPIO，则需要在芯片上电之后，尽快设置1P4V为DCDC模式，否则在vdd1v2和vddo3上有个电压脉冲。在S文件中搬代码操作前，配置为DCDC模式，解决这个问题。 [875a487e](http://192.168.48.36/src/driver/telink_b91m_platform_src/merge_requests/910/diffs?commit_id=875a487edbb3191372e255369c566f4222d0b74c)
* **USB_Demo**
  * (TLSR922x/TLSR952x)解决了 USB_MICROPHONE 模式下 DMIC0_DMIC1_IN 无法传输 DMIC0 数据的问题。 [716ead90](http://192.168.48.36/src/driver/telink_b91m_platform_src/merge_requests/944/diffs?commit_id=716ead90a4e0184169b082395715481ac358a1bb)
* **link**
   * (TLSR921x/TLSR951x/TLSR922x/TLSR952x)将_BSS_VMA_END四字节对齐，解决使能S文件中的_FILL_STK功能后，由于_BSS_VMA_END可能没有四字节对齐，存在越界访问超出_STACK_TOP，导致store access fault。 [0e732bce](http://192.168.48.36/src/driver/telink_b91m_platform_src/merge_requests/972/diffs?commit_id=0e732bcea2887804489ad6db251a281f26796218)
* **Timer_Demo**
  * (TLSR921x/TLSR951x/TLSR922x/TLSR952x)Wd_clear()需要在每次调用wd_start()之前执行，避免因为初始计数值不为0导致watchdog复位时间异常。 [20a065ec](http://192.168.48.36/src/driver/telink_b91m_platform_src/merge_requests/1000/diffs?commit_id=20a065ece53185549c748bf99d5780029def6180)
* **spi demo** 
   * (TLSR922x/TLSR952x)修复lspi在中断dma_irq_handler中重复调用gspi_reset。 [bfadf96e](http://192.168.48.36/src/driver/telink_b91m_platform_src/merge_requests/1009/diffs?commit_id=bfadf96ed151e483bbbbf0da00d9831c8f25d2ef)
* **qdec_Demo**
  * (TLSR922x/TLSR952x)qdec_get_count_value: If the interface is called continuously (with very short intervals), it is possible that the data read is not updated in time and the result is read from the last time。 [ecf387fa](http://192.168.48.36/src/driver/telink_b91m_platform_src/merge_requests/1064/diffs?commit_id=ecf387fab552255396dd584107535693c01a617d)
* **rf** 
  * (TLSR921x/TLSR951x/TLSR922x/TLSR952x)添加reg_rf_ll_2d_sclk寄存器 bit<0-2>缺失的active状态。 [d7ef39ba](http://192.168.48.36/src/driver/telink_b91m_platform_src/merge_requests/1087/diffs?commit_id=d7ef39ba526cdf736cf4ea5b75c79c86366b609c)
* **Coremark_demo**
  * (TLSR921x/TLSR951x/TLSR922x/TLSR952x)修复工程的编译选项，保证CPU跑分正常。 [791247df](http://192.168.48.36/src/driver/telink_b91m_platform_src/merge_requests/894/diffs?commit_id=791247df9dba930661b988a44fc55f3898d3c718)
* **Dhrystone_Demo**
  * (TLSR921x/TLSR951x/TLSR922x/TLSR952x)修复工程的编译选项，保证CPU跑分正常。 [791247df](http://192.168.48.36/src/driver/telink_b91m_platform_src/merge_requests/894/diffs?commit_id=791247df9dba930661b988a44fc55f3898d3c718)
* **flash**
  * (TLSR921x/TLSR951x/TLSR922x/TLSR952x)修复多次调用flash_write_status_midxxx()导致返回值错误的问题。 [0634358a](http://192.168.48.36/src/driver/telink_b91m_platform_src/merge_requests/984/diffs?commit_id=0634358ade7aaedb2939b14c8b4ebb49dfe131ce)
* **RF_Demo**
  * (TLSR921x/TLSR951x/TLSR922x/TLSR952x)修正了 rf demo 中由于未定义 TX_DELAY_US 而导致的编译报错。 [3fc3d848](http://192.168.48.36/src/driver/telink_b91m_platform_src/merge_requests/904/diffs?commit_id=3fc3d848715b84cdcd763af3613aa4fb99477a86)

### BREAKING CHANGES 
* **sys**
  * (TLSR922x/TLSR952x)重构了nop相关的宏定义避免GCC出现堆栈溢出。同时更新了nop相关宏定义的名字，使其更为合理。
  * (TLSR921x/TLSR951x/TLSR922x/TLSR952x)在sys_init函数中添加cap参数，用于配置是否需要关闭内部电容。如果开发板已经在外部焊接了电容，那么就需要关掉内部电容；如果开发板没有在外部焊接电容，那么就需要打开内部电容。如果不能对应操作的话，就可能会出现两个电容并联，或者没有电容的情况。这样对晶振的起振时间和晶振的频率影响很大，可能会造成复位、死机等情况发生。
* **pm/clock**
  * (TLSR921x/TLSR951x/TLSR922x/TLSR952x)将系统计时器工作前获取微秒级延迟的方法由nop延迟替换为core_cclk_delay_tick()接口.
* **pm**
  * (TLSR921x/TLSR951x)删除了枚举power_mode_e中的DCDC_1P4_DCDC_1P8。因为供电电压超过3.8V的情况下，使用DCDC_1P8配置，在LDO切换到DCDC时，flash的电压有超出供电范围的风险。
  * (TLSR921x/TLSR951x)PM_ANA_REG_WD_CLR_BUF0(0x38)，PM_ANA_REG_POWER_ON_CLR_BUF0(0x39)寄存器的所有bit仅供驱动程序使用。
  * (TLSR922x/TLSR952x)PM_ANA_REG_WD_CLR_BUF0(0x35)，PM_ANA_REG_POWER_ON_CLR_BUF0(0x3a)寄存器的所有bit仅供驱动程序使用。
* **rf**
  * (TLSR921x/TLSR951x/TLSR922x/TLSR952x)rf_mode_init()函数中新增了rf rx dcoc软件校准方案来解决部分DC-offset较大的芯片 rx sensitivity性能差的问题，该函数执行时间会变长，具体时间可以查看函数注释。因为使用了软件dcoc的校准方案，所以在fast settle功能中，不需要并且也不允许再用rf_get_dcoc_cal_val和rf_set_dcoc_cal_val进行处理，固删除了上述两个接口，同时删除了结构体rf_dcoc_cal_t;
* **flash**
  * (TLSR921x/TLSR951x/TLSR922x/TLSR952x)flash_write_status_midxxxxxxxx，flash_lock_midxxxxxx 和 flash_unlock_midxxxxxx接口增加返回值。
* **stimer**
  * (TLSR921x/TLSR951x/TLSR922x/TLSR952x)删除了接口stimer_enable/stimer_disable。
* **USB_Demo**
  * (TLSR921x/TLSR951x/TLSR922x/TLSR952x)由于 -fshort-wchar 编译选项移除，使用宽字符返回 usb 字符串描述符不再适用，所以用 usbdesc_get_str()/myudb_usbdesc_get_str() 函数获取 usb 字符串描述符并删除相关的 usb 字符串描述符常量。
* **flash**
  * (TLSR922x/TLSR952x)flash_write_status_midxxxxxxxx，flash_lock_midxxxxxx 和 flash_unlock_midxxxxxx接口增加返回值。
* **stimer**
  * (TLSR921x/TLSR951x/TLSR922x/TLSR952x)删除了接口stimer_enable/stimer_disable。
* **USB_Demo**
  * (TLSR921x/TLSR951x/TLSR922x/TLSR952x)由于 -fshort-wchar 编译选项移除，使用宽字符返回 usb 字符串描述符不再适用，所以用 usbdesc_get_str()/myudb_usbdesc_get_str() 函数获取 usb 字符串描述符并删除相关的 usb 字符串描述符常量。
* **rf**
  * (TLSR921x/TLSR951x)为了方便用户使用，重新封装了fast settle相关的接口，新的代码会影响兼容性，使用方法请注意参照rf_demo，app_fast_settle_test.c (删除rf_get_ldo_trim_val、_set_ldo_trim_val、rf_get_hpmc_cal_val、rf_set_hpmc_cal_val、rf_get_dcoc_cal_val、rf_set_dcoc_cal_val、rf_tx_fast_settle_init、rf_rx_fast_settle_init接口；新增rf_fast_settle_config、rf_tx_fast_settle_update_cal_val、rf_rx_fast_settle_update_cal_val接口)
  * (TLSR922x/TLSR952x)为了方便用户使用，重新封装了fast settle相关的接口，新的代码会影响兼容性，使用方法请注意参照rf_demo，app_fast_settle_test.c（删除rf_rx_fast_settle_dis、rf_get_ldo_trim_val、rf_set_ldo_trim_val、rf_get_hpmc_cal_val、rf_tx_fast_settle_init、rf_rx_fast_settle_init接口；新增rf_fast_settle_config、rf_tx_fast_settle_update_cal_val、rf_rx_fast_settle_update_cal_val接口)
* **calibration**  
  * (TLSR921x/TLSR951x/TLSR922x/TLSR952x)删掉了user_read_flash_value_calib（B91）/calibration_func(TLSR922x/TLSR952x)接口中对于打开内部电容的操作。校准接口会调整电压和电容值，所以大多数时候都会调用，但是对于24M晶体使用了外部电容的情况下，如果在调用校准接口时打开内部电容，会造成两个电容并联的情况，影响晶振起振，所以删掉了这个操作。
* **error_handler**              
  * drv_set_error_timeout: 将TIMEOUT_US删除,改为全局变量g_drv_api_error_timeout_us,添加以上接口进行修改,避免使用宏定义导致lib库不能动态修改;
  * error_code_e:修改为drv_api_error_code_e,为了防止重名;
  * get_error_code/timeout_handler:修改为drv_get_error_code/drv_timeout_handler,为了防止重名;
* **flash_demo**
  * (TLSR921x/TLSR951x/TLSR922x/TLSR952x):为了driver sdk实现bin固件保护,删除flash_demo维护的flash_lock/flash_unlock swtich实现方案,为代码简洁以及方便维护,改为添加flash hal层以结构体实现方案,添加hal_flash.c/.h文件,并在platform_init中添加bin固件保护功能.

### Features
* **TL721X**
  * ADC/Analog/CHACHA20_POLY1305/Debug/Flash/GPIO/HASH/I2C/PKE/PM/PWM/QDEC/RF/SKE/SPI/STIMER/TIMER/TRAP/TRNG/UART/USB
* **TL321X**
  * ADC/Analog/Debug/Flash/GPIO/I2C/PWM/QDEC/RF/SKE/SPI/STIMER/TIMER/TRAP/TRNG/UART/USB
* **alg_reg** 
  * (TLSR921x/TLSR951x/TLSR922x/TLSR952x)删除如下接口：analog_write_reg32_dma、analog_read_reg32_dma、analog_write_buff_dma、analog_read_buff_dma，analog_write_addr_data_dma。因为实际应用不会这样使用dma，并且其他接口已经可以满足需求。
  * (TLSR921x/TLSR951x/TLSR922x/TLSR952x)限制analog_write_buff接口数据长度不能超过8个字节。
* **spi** 
  * (TLSR922x/TLSR952x)修复gspi_change_csn_pin和spi_master_config枚举参数不匹配导致编译warning问题，不影响功能
* **rf** 
  * (TLSR921x/TLSR951x/TLSR922x/TLSR952x)添加了rf_set_rx_dcoc_cali_by_sw接口开关rx DCOC软件校准，以优化rf_mode_init执行时间（该接口在只用tx，不用rx的场景，才可以使用）
  * (TLSR921x/TLSR951x/TLSR922x/TLSR952x)添加了user_calib_rf_rx_dcoc接口来获取flash中的dcoc校准值
* **flash**
  * (TLSR921x/TLSR951x/TLSR922x/TLSR952x)新增接口：flash_4line_en，flash_4line_dis。
* **USB**
  * (TLSR921x/TLSR951x)新增 usbhw_set_ep_addr() 接口来设置端点 buffer 分配；usbhw_set_eps_max_size() 接口来设置 IN 端点最大 buffer 大小。
  * (TLSR922x/TLSR952x)新增 usbhw_set_eps_max_size() 接口来设置 IN 端点最大 buffer 大小。
* **SDK VERSION**  
  * (TLSR921x/TLSR951x/TLSR922x/TLSR952x)新增'tl_check_fw.sh'脚本,编译完代码后打印sdk版本信息。
* **stimer**
  * (TLSR921x/TLSR951x)增加了接口pm_clr_irq_status/pm_set_wakeup_src/stimer_set_32k_track_cnt/stimer_get_32k_track_cnt/stimer_get_tracking_32k_value。
  * (TLSR922x/TLSR952x)增加了接口pm_set_wakeup_src/stimer_set_32k_track_cnt/stimer_get_32k_track_cnt/stimer_get_tracking_32k_value。
* **core**
  * (TLSR921x/TLSR951x/TLSR922x/TLSR952x)新增core.c文件，文件内包括core_cclk_delay_tick接口；新增rdmcycle、core_cclk_delay_tick接口。
* **drivers**
  * (TLSR922x/TLSR952x)新增文件夹error_handler，包括error_handler.c/error_handler.h文件,文件内包括get_error_code、timeout_handler接口。
* **sys**
  * (TLSR922x/TLSR952x)增加了接口sys_set_dcdc_1pP4_ldo_2p0，以解决A3上（A4没有）如果硬件板上有1.4V dcdc电感器件且使用1.8V GPIO，则会在vdd1v2和vddo3上产生电压脉冲的问题。
  * (TLSR922x/TLSR952x)新增efuse_get_chip_id接口。注：只有A3及以后版本被写入芯片id值。
* **flash**
  * (TLSR922x/TLSR952x)增加1M flash(GD25LQ80E)驱动。
  * (TLSR922x/TLSR952x)添加flash_write_config_mid146085()，并修改flash_lock_mid146085()和flash_unlock_mid146085()以适配二供flash P25Q80SU。
* **sys**
  * (TLSR921x/TLSR951x/TLSR922x/TLSR952x)sys_init和pm_sleep_wakeup函数实现中新增crystal_manual_settle函数以辅助晶振快速起振。
* **pm**
  * (TLSR921x/TLSR951x)新增pm_set_xtal_stable_timer_param()用于在晶体起振相对较慢时调整设置晶体相关参数。
* **flash**
  * (TLSR921x/TLSR951x)添加新增flash PY25Q128H驱动和demo.（注意：该flash sector erase时间最大为240ms。）
  * (TLSR921x/TLSR951x/TLSR922x/TLSR952x)添加flash_get_capacity()和检查flash容量demo。
* **flash**
  * (TLSR922x/TLSR952x)添加GD25LQ32E/GD25LE32E的驱动和demo。
* **core**
  * wait_condition_fails_or_timeout/wait_condition_fails_or_timeout_with_param:新增以上接口,为所有模块提供统一的超时处理;
* **error_handler**              
  * drv_api_status_e:新增枚举,为后面新增接口提供统一的返回状态;
* **trng**
  * 增加了超时机制,相应的新增了接口trng_hw_reset/trng_set_error_timeout/trng_timeout_handler;
* **aes**
  * 增加了超时机制,相应的新增了接口aes_hw_fsm_reset/aes_set_error_timeout/aes_timeout_handler;
* **i2c**
  * 增加了超时机制,相应的新增了接口i2c_hw_fsm_reset/i2c_set_error_timeout/i2c_get_error_timeout_code/i2c_timeout_handler;
* **i2c1_m**
  * 增加了超时机制,相应的新增了接口i2c1_m_hw_fsm_reset/i2c1_m_set_error_timeout/i2c1_m_get_error_timeout_code/i2c1_m_timeout_handler;
* **uart**
  * 增加了超时机制,相应的新增了接口uart_set_error_timeout/uart_get_error_timeout_code/uart0_timeout_handler/uart1_timeout_handler;
* **spi**
  * 增加了超时机制,相应的新增了接口spi_set_error_timeout/spi_get_error_timeout_code/lspi_timeout_handler/gspi_timeout_handler/spi_hw_fsm_reset;
* **EMI_BQB_Demo**
  * (TLSR921x/TLSR951x/TLSR922x/TLSR952x)emi_con_tx55();emi_con_tx0f();增加跳频功能

### Refactoring
* **GPIO_Demo**
  * (TLSR921x/TLSR951x/TLSR922x/TLSR952x)优化了GPIO_Demo，合并共用接口并补充注释。
* **gpio**
  * (TLSR921x/TLSR951x/TLSR922x/TLSR952x)更新了gpio_shutdown()的注释：gpio_shutdown(GPIO_ALL)不建议在应用程序中使用，如果使用需要在调用后根据应用启用相应的引脚。 
* **aes**
  * (TLSR922x/TLSR952x)删除了aes.h中的aes_rpa_match函数声明。
* **rf**
  * (TLSR921x/TLSR951x/TLSR922x/TLSR952x)芯片rf的所有mask的默认值是打开的，为了方便用户使用，sys_init函数中会将其关掉，后面由用户自行配置
* **RF_DEMO**
  * (TLSR921x/TLSR951x/TLSR922x/TLSR952x)将RF_Demo中调用stimer_get_tick()的地方替换为rf_stimer_get_tick()。
* **AES_Demo**
  * (TLSR921x/TLSR951x/TLSR922x/TLSR952x)优化了AES_Demo，规范了AES不同模式的宏定义名称，新增了AES_ECB、AES_CBC模式，删除了AES_RPA模式。
* **USB_Demo**
  * (TLSR921x/TLSR951x/TLSR922x/TLSR952x)CDC 端点可配置，默认为 USB_EDP4_IN 和 USB_EDP5_OUT。CDC 发送数据时使用端点忙位而不是 delay_us 来确定数据是否发送完毕。
  * (TLSR921x/TLSR951x/TLSR922x/TLSR952x)使用 usb_init() 取代 usb_init_interrupt()来初始化 USB。(如果使用 usb_init() 接口，需要使用 usbhw_set_eps_en() 来使能用到的端点。)
  * (TLSR921x/TLSR951x/TLSR922x/TLSR952x)优化mouse demo和keyboard demo, 使用矩阵按键触发上报hid report。
* **I2C_Demo**
  * (TLSR921x/TLSR951x/TLSR922x/TLSR952x)优化了I2C_Demo，规范不同模式区分并补充注释。
* **flash**
  * (TLSR921x/TLSR951x/TLSR922x/TLSR952x)调整了flash_write_status_midxxxxxxxx接口的参数类型，其中B92新增了参数错误返回值。
  * (TLSR922x/TLSR952x)修改flash_send_cmd、flash_is_busy接口中mspi忙碌等待代码，改为调用mspi_wait接口。
* **mspi**
  * (TLSR922x/TLSR952x)将mspi_is_busy接口修改为mspi_wait接口；修改mspi_write、mspi_read、mspi_stop_xip接口中mspi忙碌等待代码，改为调用mspi_wait接口。
* **analog** 
  * (TLSR922x/TLSR952x)analog_wait接口增加超时处理。
* **clock**
  * (TLSR922x/TLSR952x)clock_cal_24m_rc、clock_cal_32k_rc接口增加超时处理。
* **project**
  * (TLSR921x/TLSR951x/TLSR922x/TLSR952x)去除 -fshort-wchar 编译选项。
* **GPIO_DEMO**
  * (TLSR921x/TLSR951x/TLSR922x/TLSR952x)将GPIO_MODE默认值设为GPIO_IRQ;修改GPIO_HIGH_RESISTOR模式逻辑，使程序while(1)停住。
* **EMI_BQB_DEMO**
  * (TLSR921x/TLSR951x)由于已删除dcdc模式，所以注释dcdc相关代码。
  * (TLSR921x/TLSR951x/TLSR922x/TLSR952x)调整bqb中fast settle配置代码，适配新的接口。
  * (TLSR921x/TLSR951x/TLSR922x/TLSR952x)优化emi、bqb main.c初始化部分代码，同时优化app_emi.c代码，以适配不同芯片。
* **emi**
  * (TLSR921x/TLSR951x/TLSR922x/TLSR952x)优化rf_emi_tx_burst_loop和rf_emi_tx_burst_setup接口，将接口中的payload len替换成宏EMI_TX_PKT_PAYLOAD
  * (TLSR921x/TLSR951x/TLSR922x/TLSR952x)将emi.c emi.h移动到lib文件中。
  * (TLSR921x/TLSR951x/TLSR922x/TLSR952x)规范了emi相关函数的注释
* **rf**
  * (TLSR921x/TLSR951x/TLSR922x/TLSR952x)优化rf_tx_fast_settle_en、rf_tx_fast_settle_dis、rf_rx_fast_settle_en、rf_rx_fast_settle_dis、rf_set_chn相关接口。
  * (TLSR921x/TLSR951x/TLSR922x/TLSR952x)规范了rf相关函数的注释.
* **RF_Demo**
  * (TLSR921x/TLSR951x/TLSR922x/TLSR952x)增加fast settle 测试demo。
* **trng**
  * (TLSR922x/TLSR952x)trng_init:为了超时机制,在以上接口中添加了返回值;         
* **aes**
  * (TLSR922x/TLSR952x)aes_encrypt/aes_encrypt_bt_en/aes_decrypt/aes_decrypt_bt_en:为了超时机制,在以上接口中新增了返回值DRV_API_TIMEOUT; 
* **i2c**
  * (TLSR922x/TLSR952x)i2c_master_write/i2c_master_read/i2c_master_write_read:为了超时机制,在以上接口中新增了返回值DRV_API_TIMEOUT;         
* **i2c1_m**
  * (TLSR922x/TLSR952x)i2c1_m_master_write/i2c1_m_master_read/i2c1_m_master_write_read:为了超时机制,在以上接口中新增了返回值DRV_API_TIMEOUT;
* **uart**
  * (TLSR921x/TLSR951x/TLSR922x/TLSR952x)uart_reset:为了更贴合功能，将该接口命名为uart_hw_fsm_reset,为了兼容性,使用重定义#define uart_reset uart_hw_fms_reset;
  * (TLSR922x/TLSR952x)uart_send_byte/uart_send_hword/uart_send_word:为了超时机制,在以上接口中增加了返回值;
* **spi** 
  * (TLSR922x/TLSR952x)spi_master_send_cmd/spi_write/spi_read/spi_master_write/spi_master_write_read/spi_master_write_plus/spi_master_write_repeat/spi_master_write_repeat_plus/spi_master_read_plus/spi_master_write_read_plus:为了超时机制,在以上接口中增加了返回值;
 * **spi demo** 
   * (TLSR922x/TLSR952x)将no_dma和dma模式下的spi_is_busy()接口替换成超时机制spi接口SPI_WAIT_BUSY,并修改使用到spi_master_get_slave_ready()接口的处理逻辑(添加超时情况并进行处理);
 * **display demo** 
   * (TLSR922x/TLSR952x)将lcd_st7796h2,oledmr69330 spi_is_busy()接口替换成超时机制spi接口SPI_WAIT_BUSY;  
* **audio**
  * (TLSR921x/TLSR951x)audio codec 相关寄存器,将原来直接使用默认值改成再写一下默认值,防止异常情况下,默认值被改导致功能出现问题。
* **ALL**
  * (TLSR921x/TLSR951x/TLSR922x/TLSR952x)除 chip 目录外，将其他所有 .c 和 .h 文件中的 tab 替换为 4 个空格。
* **GPIO**
  * (TLSR921x/TLSR951x/TLSR922x/TLSR952x)更正 gpio_get_level() 注释。
* **ALL_Demo**
  * (TLSR921x/TLSR951x/TLSR922x/TLSR952x)为了提高使用的灵活性，区分不同芯片时不再使用#if的形式，改为#if defined。
* **ALG_REG_Demo**
  * (TLSR921x/TLSR951x/TLSR922x/TLSR952x) 优化了analog_write_halfword_test()中的变量定义，确保halfword数据读写一致。

### Performance Improvements

* **rf**
  * (TLSR921x/TLSR951x)rf_mode_init()函数使用dcoc 软件校准来使芯片获得最小的DC-offset,同时在BLE S2 S8模式下打开RX二级滤波器将DC-offset和噪声尽可能滤掉 ，以此来提高芯片的带外抗干扰能力（干扰包括DC-offset）。通过这两个操作可以将DC-offset较大的芯片sensitivity性能恢复到正常范围,但需要注意开二级滤波器会将芯片的抗频偏范围缩小到±150kHz以内。
  * (TLSR922x/TLSR952x)rf_mode_init()函数使用dcoc 软件校准来使芯片获得最小的DC-offset,同时在除了private 250k和500k的RF模式下打开RX二级滤波器，将DC-offset和噪声尽可能滤掉 ，以此来提高芯片的带外抗干扰能力（干扰包括DC-offset）。通过这两个操作可以将DC-offset较大的芯片sensitivity性能恢复到正常范围,但需要注意开二级滤波器会将芯片的抗频偏范围缩小到±150kHz以内。


## V2.3.0

### Version

* SDK version: telink_b91m_driver_sdk V2.3.0.
* This version of the SDK supports B91(A0/A1/A2),B92(A3) chips.

### Note

* To prevent program function exceptions caused by different compiler optimization levels, _always_inline/_attribute_ram_code_sec_optimize_o2_ are added to some functions in the following files. This change is described in this section instead of in the following modules.
  - chip/B92/drivers/lib/include: flash_base.h/plic.h/pm.h/rf.h/sys.h.
  - chip/B92/drivers: analog.h/analog.c/audio.h/audio.c//clock.h/clock.c/core.h/dma.h/gpio.h/mspi.h/stimer.h.
  - common: compiler.h.
  - common/B92: exception.c.
* Cleaned up the Chinese characters in the code, involving the following files:
  - chip/B92/drivers: audio.c/audio.h/i2c.c/i2c.h/spi.h.
  - chip/B91/drivers/lib/include: pm.h.
  - chip/B92/drivers/lib/include: pm.h.
  - demo/vendor/AUDIO_Demo/AUDIO_V1.1/AUDIO_Demo: app_codec_0581.c/app_i2s.c.
  - demo/vendor/DUT_Test: dut_rf_frequency_offset_calib_slave.c.
  - demo/vendor/Flash_Demo: app.c.
  - demo/vendor/OTP_Demo: app.c.
  - demo/vendor/TRAP_Demo: app.c.

### Bug Fixes
* **trap**
  * (B91/B92)plic_set_threshold, core_interrupt_enable, core_interrupt_disable and core_restore_interrupt interfaces, the operation of PLIC registers may not be successful, resulting in the interrupt priority function may be abnormal, add the fence instruction to fix the problem. [c97214d6](http://192.168.48.36/src/driver/telink_b91m_driver_src/merge_requests/617/diffs?commit_id=c97214d640478e3dca1dd1b828b2cb490eab130c)
  * (B91/B92)Fixed an anomaly that caused the PLIC threshold to be set to 0 when the following configurations were enabled at the same time: [e0b8115b](http://192.168.48.36/src/driver/telink_b91m_driver_src/merge_requests/613/diffs?commit_id=e0b8115be1d294c548ba1f56ae2e86abade31d88)
    - plic_set_threshold is set to a non-IRQ_PRI_NUM0 value.
    - plic_preempt_feature_en is called to enable interrupt preemption;
    - flash_plic_preempt_config has preempt_en set to 1.
* **flash**
  * (B92)flash_read_data_decrypt_check/flash_dread_decrypt_check/flash_4read_decrypt_check: After calling the above interface, the prefetch instruction is turned off by mistake, and this version fixes it to turn on. [2242c4e0](http://192.168.48.36/src/driver/telink_b91m_driver_src/merge_requests/620/diffs?commit_id=2242c4e0815369707030eea6e2b20ff128dc64e2)
  * (B91/B92)flash_get_lock_block_midxxxx:get lock protected area error, this version fixes it. [c6395c73](http://192.168.48.36/src/driver/telink_b91m_driver_src/merge_requests/622/diffs?commit_id=c6395c731d56d4419fe0f868e6c08d6c126fcb06)
* **pm**
  * (B92)pm_sleep_wakeup:Fixed a probabilistic crash caused by entering low power mode in an abnormal way:
    - Wait for the MCU to enter wfi mode before going to sleep and then enter low power mode. [d33fba65](http://192.168.48.36/src/driver/telink_b91m_driver_src/merge_requests/580/diffs?commit_id=d33fba652f9848fed81d3d1db8d68af99b8c4896)
  * (B92)Fixed probabilistic crash caused by low retention ldo voltage. [d33fba65](http://192.168.48.36/src/driver/telink_b91m_driver_src/merge_requests/580/diffs?commit_id=d33fba652f9848fed81d3d1db8d68af99b8c4896)
  * (B91/B92)Eliminates the risk that STATUS_GPIO_ERR_NO_ENTER_PM may be set to 1 in the return value of cpu_sleep_wakeup. [e503c4eb](http://192.168.48.36/src/driver/telink_b91m_driver_src/merge_requests/719/diffs?commit_id=e503c4ebe3477ab15acaac67f8e45f95c55180a1) 
  * (B92)Fixed issue where pm_clr_irq_status()could mistakenly clear other interrupt flag bits. [69b24afa](http://192.168.48.36/src/driver/telink_b91m_driver_src/merge_requests/717/diffs?commit_id=69b24afadb7e477214c194d69717e4a1d3249c3f)
* **audio**
  * (B92)Fixed an issue where the audio_set_stream0_dig_gain0() interface setting stream0 digital gain did not take effect. [f34468c2](http://192.168.48.36/src/driver/telink_b91m_driver_src/merge_requests/660/diffs?commit_id=f34468c2bf3e26f98ef05a37bd4919538fd4ce7b)
  * (B92)Fixed an issue where repeated calls to the audio_power_on() interface resulted in audio function exceptions. [2f903fc6](http://192.168.48.36/src/driver/telink_b91m_driver_src/merge_requests/715/diffs?commit_id=2f903fc61a8cb5b8b144c6e1c481513b86992de5)
* **clock**
  * (B91/B92)Fixed a risk that when users use clock_init() to switch between different clock frequencies, hclk/pclk/mspi_clk could exceed its maximum configurable frequency for a short time. [6a0b5cc2](http://192.168.48.36/src/driver/telink_b91m_driver_src/merge_requests/668/diffs?commit_id=6a0b5cc277dc4ba4966ba53ea450a9c92882e67a)
  * (B92)Fixed a risk that when users use cclk_hclk_pclk_config() to switch cclk/hclk/pclk, hclk/pclk could exceed its maximum configurable frequency for a short time. [6a0b5cc2](http://192.168.48.36/src/driver/telink_b91m_driver_src/merge_requests/668/diffs?commit_id=6a0b5cc277dc4ba4966ba53ea450a9c92882e67a)
  * (B92):pll_div_cclk_hclk_pclk_e: cclk/hclk/pclk has a certain multiple relationship, and unsupported relationships are masked. [2a1b07bd](http://192.168.48.36/src/driver/telink_b91m_driver_src/merge_requests/687/diffs?commit_id=2a1b07bd6b20356b160ecdd74df1592cd50669f8)
* **uart**
  * (B91)When dma receives data, solve the problem that rxdone cannot be generated when the send length is greater than the receive length. [a853aaca](http://192.168.48.36/src/driver/telink_b91m_driver_src/merge_requests/691/diffs?commit_id=a853aaca634913b7dbb559283e960febf5479871)
  * (B91)Solve the problem that DMA can't be sent after the program goes to suspend sleep and is awakened up. [b309dfbf](http://192.168.48.36/src/driver/telink_b91m_driver_src/merge_requests/705/diffs?commit_id=b309dfbfd9bcb3c1c96984cea5ad09fa12f2c7f4)
  * (B91)Fixed issue where uart_clr_irq_status() could mistakenly clear other interrupt flag bits. [69b24afa](http://192.168.48.36/src/driver/telink_b91m_driver_src/merge_requests/717/diffs?commit_id=69b24afadb7e477214c194d69717e4a1d3249c3f)
* **rf**
  * (B92)Fixed misconfiguration issue within the rf_set_ble_500K_mode driver interface. [46a18667](http://192.168.48.36/src/driver/telink_b91m_driver_src/merge_requests/695/diffs?commit_id=46a18667e2dccef1c7add17893e66526089f20b6)
  * (B92)Fixed an issue where the rf_start_srx2tx parameter tick did not work resulting in an exception for sending and receiving packets. [f0cdfeb5](http://192.168.48.36/src/driver/telink_b91m_driver_src/merge_requests/764/diffs?commit_id=f0cdfeb5dba0c4df171a0eaca544a3897bcf98ce)
  * (B91)Fixed the issue of incorrect return values for interfaces rf_aoa_aod_hdinfo_offset() and rf_aoa_aod_get_pkt_timestamp(). [04bd6d9c](http://192.168.48.36/src/driver/telink_b91m_driver_src/merge_requests/770/diffs?commit_id=04bd6d9c1472433047ac243eb13353f6976ebbf8)
* **mdec**
  * (B91/B92)Fixed issue where mdec_clr_irq_status() could mistakenly clear other interrupt flag bits. [69b24afa](http://192.168.48.36/src/driver/telink_b91m_driver_src/merge_requests/717/diffs?commit_id=69b24afadb7e477214c194d69717e4a1d3249c3f)
* **i2c**
  * (B92)Fixed issue where i2c_clr_irq_status() could mistakenly clear other interrupt flag bits. [69b24afa](http://192.168.48.36/src/driver/telink_b91m_driver_src/merge_requests/717/diffs?commit_id=69b24afadb7e477214c194d69717e4a1d3249c3f)
* **pwm** 
  * (B92)Fixed issue where pwm_get_irq_status() could return incorrect interrupt status determination and could clear interrupt flag bits by mistake. [5a0e1014](http://192.168.48.36/src/driver/telink_b91m_driver_src/merge_requests/700/diffs?commit_id=5a0e1014e2340b2a9c661ecc2d79c64247cbc48e)
* **gpio**
   * (B91)Fix the problem that the unneeded pin in uart_set_pin()/hspi_set_pin()/pspi_set_pin() is not initialized to NONE_PIN, which leads to abnormal PA0 function. [ef3ff6f1](http://192.168.48.36/src/driver/telink_b91m_driver_src/merge_requests/753/diffs?commit_id=ef3ff6f194fc513802b98afd4263923445555e40)
* **flash**
  * (B91/B92)Fixed an exception issue in flash_write(), flash_write_otp(), flash_write_encrypt() functions that may be triggered by mistakenly setting the parameter len to 0. [291e8752](http://192.168.48.36/src/driver/telink_b91m_driver_src/merge_requests/743/diffs?commit_id=291e8752e69460bc9d3d46e8629b9374d5348dcc)
* **adc**
  * (B92)Fix the problem that efuse_calib_adc_vref() interface didn't do the conversion when taking out offset as a negative number resulting in calibration error. [291e8752](http://192.168.48.36/src/driver/telink_b91m_driver_src/merge_requests/743/diffs?commit_id=291e8752e69460bc9d3d46e8629b9374d5348dcc)
* **USB_Demo**
  * (B91/B92)Fixed a bug in USB enumeration where the device returned too much data when the host request length was less than the actual descriptor length. [4f97a6f1](http://192.168.48.36/src/driver/telink_b91m_driver_src/merge_requests/762/diffs?commit_id=4f97a6f1f2be7b62310bc4061d194c8f9b0b5821)
  * (B91/B92)Fixed a bug where the HID descriptor returned too much data when the host requested an interface-specific descriptor. [4f97a6f1](http://192.168.48.36/src/driver/telink_b91m_driver_src/merge_requests/762/diffs?commit_id=4f97a6f1f2be7b62310bc4061d194c8f9b0b5821)
* **TRAP_Demo**
  * (B91/B92)Fix compile error caused by case mixing in linux environment. [5131b7d0](http://192.168.48.36/src/driver/telink_b91m_driver_src/merge_requests/612/diffs?commit_id=5131b7d09e0eb4e0cbea84c1ac9119da4018ce70)
  * (B91)Fix the issue that WFI mode can't be entered and woken up repeatedly when the global interrupt is turned off. [08c0ddab](http://192.168.48.36/src/driver/telink_b91m_driver_src/merge_requests/595/diffs?commit_id=08c0ddabfb30e08ad2c50d0b7a33d1240ca4f600)
* **I2C_Demo**
  * (B92) When the slave  (I2C_STRETCH_MODE == I2C_STRETCH_DIS),interrupt is incorrectly enabled, sending or receiving messages is abnormal. [9ddf8348](http://192.168.48.36/src/driver/telink_b91m_driver_src/merge_requests/589/diffs?commit_id=9ddf8348849f009574a4d5b7eee3edc6b927992c)
* **SPI_Demo**
  * (B92) spi_master_read_dma_plus()/spi_master_write_read_dma(): indicates whether to use dma_set_irq_mask instead. SPI_END_EN can only represent the completion of spi receiving, not the completion of dma work, to prevent abnormal situations. [7bdb43d4](http://192.168.48.36/src/driver/telink_b91m_driver_src/merge_requests/598/diffs?commit_id=7bdb43d406e33ce6571372b0752b34a5df200d70)
* **UART_DEMO**
  * (B91/B92)When nodma receives data, uart_get_rxfifo_num() is used instead of uart_rx_trig_level to determine how much data is read at one time in the interrupt, which can solve the risk of data not being retrieved in time due to interrupt in the application and reduce the disturbance of fifo Pointers. [a853aaca](http://192.168.48.36/src/driver/telink_b91m_driver_src/merge_requests/691/diffs?commit_id=a853aaca634913b7dbb559283e960febf5479871)

### BREAKING CHANGES 
* **i2c**
  * (B92) Merge clear related interfaces i2c_clr_fifo(i2c_buff_clr_e clr) and i2c_clr_irq_status(i2c_irq_clr_e status) into interface i2c_clr_irq_status(i2c_irq_status_e status). Correspondingly, delete interface i2c_clr_irq_status, delete enumerations i2c_buff_clr_e and i2c_irq_clr_e, and merge them into i2c_irq_status_e.
* **trap**
  * (B91/B92) The core_interrupt_enable() function calls the new interface core_mie_enable to enable MEI, and it no longer enables MTI and MSI.
  * (B91/B92) Since unused interrupt service routines take up too much RAM, add a new PLIC_ISR_REGISTER macro to allow applications to register PLIC interrupt service routines. Accordingly, delete the plic_isr.c file and remove the irq_source_e enumeration and replace it with irq_source_index.
  * (B92) Delete interface mtime_enable()/mtime_disable() and replace it with interface core_mie_enable/core_mie_disable with more comprehensive functionality (including mtime).
  * (B91/B92) The plic_preempt_feature_en() function adds a parameter of type core_preempt_pri_e to configure the interrupt nesting priority at which the MEI can be interrupted by the MSI and the MTI.
* **pm**
  * (B91/B92)pm_sleep_wakeup() function has removed the handling of the interrupt mask for the system timer (turned off before sleep and turned on after suspend wake-up), and now leaves the mask to the user for processing. If the user needs to use the mask, they can call the these two functions to set it: stimer_set_irq_mask（）and stimer_clr_irq_mask().
  * (B91/B92)Use PM_ANA_REG_POWER_ON_CLR_BUF0[bit2] (0x3a[2]) If this bit is 1, it means that the pm_sleep_wakeup function failed to clear the pm wake flag bit when using the deep wake source, and the software called sys_reboot(),The customer cannot change!
  * (B92)Structure type pm_r_delay_cycle_s has two new members: deep_xtal_delay_cycle and suspend_ret_xtal_delay_cycle.
* **IDE**
  * (B91)Re-modified the ide for b91 to TelinkIoTStudio, and the toolchain remains GCC7.

### Features
* **audio**
  * (B91)Add audio_codec_active() and audio_codec_set_micbias() interface to control micbias enable and voltage selection.
  * (B92)Add codec_0581 driver and example code.
* **rf**
  * (B91)Added fast settle related interfaces:
rf_rx_fast_settle_init();rf_tx_fast_settle_init();rf_tx_fast_settle_en();rf_tx_fast_settle_dis();rf_rx_fast_settle_en（）；rf_rx_fast_settle_dis();rf_get_ldo_trim_val();rf_set_ldo_trim_val();
rf_get_hpmc_cal_val(); rf_set_hpmc_cal_val();rf_get_dcoc_cal_val();
rf_set_dcoc_cal_val();
  * (B92)Added fast settle related interfaces:
rf_rx_fast_settle_init();rf_tx_fast_settle_init();rf_tx_fast_settle_en();rf_tx_fast_settle_dis();rf_rx_fast_settle_en（）；rf_rx_fast_settle_dis();rf_get_ldo_trim_val();rf_set_ldo_trim_val();
rf_get_hpmc_cal_val(); rf_set_hpmc_cal_val();rf_get_dcoc_cal_val();
rf_set_dcoc_cal_val();rf_get_rccal_cal_val();rf_set_rccal_cal_val();
rf_dis_hpmc_trim();rf_dis_ldo_trim();rf_dis_dcoc_trim();rf_dis_rccal_trim();
  * (B92)Added interface: rf_set_power_off_singletone();rf_aoa_aod_set_trx_mode();rf_aoa_aod_sample_point_adjust();rf_aoa_rx_ant_switch_point_adjust();rf_aod_tx_ant_switch_point_adjust();rf_aoa_aod_sample_interval_time();rf_aoa_aod_iq_data_mode();
* **trap**
  * (B91)Add machine time interrupt and software interrupt interfaces, and add corresponding examples.
* **pm**
  * (B92)Add pm_get_reboot_event() to determine the status of the current reboot. 
* **pm**
  * (B92)Add pm_set_active_vddo3() to set VDDO3 voltage.
* **gpio**
  * (B92)Add jtag_set_pin_en()/sdp_set_pin_en() interface.

* **TRAP_Demo**
  * (B92)Read the mcause register in the except_handler function.
* **USB_Demo**
  * (B92)Add mic and spk support for codec_0581.
  * (B92)Add WFI (wait for interrupt) and HSP (hardware stack protection) interfaces and add corresponding examples.
* **EMI_BQB_Demo**
  * (B92)emi carrieronly mode adds tone frequency hopping function.
  * (B92)emi tx burst mode adds the function of sending pkt+tone. 
* **I2C_Demo**
  * (B92)Add demo for i2c1_m(master) to communicate with b91m i2c slave.

### Refactoring
* **S**
  * (B92)Adjust the code to reduce the probability of misconfiguring SRAM.
  * (B91)In order to be consistent with B92, and to leave a margin for subsequent flash, the wait time for flash wake-up command to standby mode has been increased to 25us.
* **rf**
  * (B91/B92) Modify rf_set_power_level_index(); function definition to remove _attribute_ram_code_sec_.
  * (B91/B92) Add const keyword to rf_power_level_list table.
* **pm** 
  * (B91)Optimize the pm_sleep_wakeup function: switch the mspi clk to 24M rc clock before sleep, and advance the flash wake position before pll stabilizes.
  * (B91/B92)Tidy up the code and make sure that all functions called in the sleep function are ramcode.
  * (B92)Modify enumeration type pm_suspend_power_cfg_e: Delete the PM_POWER_AUDIO enumeration variable.
* **trap**
  * (B91/B92)The name of irq_source_index has been changed for the sake of multi-chip unification. b91/b92 is compatible with the previous name, and subsequent chips no longer support it.
* **flash**
  * (B91/B92)Changed the comment of parameter "addr" of flash_erase_sector() to: must be 0 or a multiple of 0x1000.
* **sys**
  * (B92)The _ASM_NOP_ definition adds the volatile keyword to prevent compiler optimization. Add the macros CLOCK_DLY_64_CYC and CLOCK_NOP_DLY_xUS that will be used in the program.
* **clock**
  * (B92)clock_set_32k_tick: Optimize the nop notation in the function.
* **spi**
  * (B91/B92)In order to optimize the distribution of memory within the struct, the struct member positions have been adjusted (enum members are placed in the back), without affecting functionality.
* **audio**
  * (B91/B92)In order to optimize the distribution of memory within the struct, the struct member positions have been adjusted (members with defined occupancy lengths and aligned members are placed in front, and enum members are placed in the back), without affecting functionality.
* **ALG_REG_Demo**
  * (B91)Optimize the code structure of demo to enhance reusability.
* **USB_Demo**
  * (B91/B92)Implement configurable SubFrameSize in the mic/spk interface descriptor.
  * (B91/B92)Add __attribute__((packed)) to USB-related structs to avoid functional exceptions when the -fpack-struct compile option is removed.
* **UART_DEMO**
  * (B91/B92)app.c/app_dma.c:add a macro define UART_MODULE_SEL to control which uart is selected.
  * (B92)app_dma_llp.c:1.add a macro define UART_MODULE_SEL to control which uart is selected; 2.to be consistent with B930, dma TC_MASK is used instead of uart UART_RXDONE_MASK.
* **ALL_Demo**
  * (B91/B92)Added common.c and common.h, put the definition of LED and KEY in common.h, defined platform_init interface in common.c, put the public initialization content in platform_init interface, and added the initialization content to set up pull-up for sws to prevent probabilistic crash.
  * (B91/B92)To clear up the warnings caused by warning flag -Wstrict-prototypes, the following demo files has been modified by adding a void parameter to the function without parameters:
    - ADC_Demo, AES_Demo, ALG_REG_Demo, AUDIO_Demo, AXON_Demo, Auto_Test_Demo, CTB_Demo, Charger_Demo, Coremark_Demo, DUT_Demo, Debug_Demo, Dhrystone_Demo, Display_Demo, EMI_BQB_Demo, Flash_Demo, GPIO_Demo, HADM_Demo, I2C_Demo, OTP_Demo, PKE_Demo, QDEC_Demo, RF_Demo, SKE_Demo, SPI_Demo, STimer_Demo, Swire_Demo, TRAP_Demo, Test_Demo, UART_Demo, USB_Demo, s7816_Demo
* **project**
  * (B92)Modified the B92 Project Configuration includes the following contents:
    - On the basis of the original warning flags(-Wall -Wextra -Wshadow -Werror), the following warning flags has been added: -Wimplicit-fallthrough -Wredundant-decls -Wcast-qual -Wsign-compare -Wunused-parameter -Wunused-variable -Wswitch -Wstrict-prototypes -Wmissing-field-initializers -Wpointer-arith -Wdeprecated-declarations -Werror=cpp -Wenum-conversion -Wpacked-not-aligned -Waddress-of-packed-member -Wundef
  - Add preprocessor defined symbols:
    - MCU_CORE_B91=0
    - MCU_CORE_B930=0
    - MCU_CORE_B95=0
    - MCU_CORE_B96=0

* **driver src**
  * (B92)To clear up the warnings caused by warning flag -Wstrict-prototypes, the following src files has been modified by adding a void parameter to the function without parameters:
    - chip/B92/drivers: analog.c/i2c.h/mspi.h/plic_sw.h/s7816.h
    - chip/B92/drivers/lib/include: hadm.h/hadm_drv.h/rf.h/swire.h
    - chip/usb_dbg: myudb.h/myudb_usb.c
  * (B91/B92)To clear up the warnings caused by warning flag -Wcast-qual, the following src files has been modified by adding a const keyword or removing the volatile keyword to the relevant variable:
    - chip/B91/drivers/lib/src/pke: pke.c 
    - chip/B91/drivers/lib/include/pke: pke.h
    - chip/B92/drivers/lib/src/pke: pke.c 
    - chip/B92/drivers/lib/include/pke: pke.h
    - chip/B92/drivers/lib/include/pke: eccp_curve.h
    - chip/B92/drivers/ext_peripherals/codec_0581: codec_0581.h
    - chip/B92/drivers: audio.h
  
### Performance Improvements
* **S**
  * (B92)To save power consumption, the wait time for flash wake up commands to standby mode is shortened when retention wake up.

<hr style="border-bottom:2.5px solid rgb(146, 240, 161)">

### Note

* 为了防止编译优化等级不一样导致的程序功能异常，将如下文件中的部分函数加了_always_inline/_attribute_ram_code_sec_optimize_o2_进行处理，此修改在这里进行整体说明，就不在后面各模块中分别说明了。
  - chip/B92/drivers/lib/include: flash_base.h/plic.h/pm.h/rf.h/sys.h.
  - chip/B92/drivers: analog.h/analog.c/audio.h/audio.c//clock.h/clock.c/core.h/dma.h/gpio.h/mspi.h/stimer.h.
  - common: compiler.h.
  - common/B92: exception.c.
* 清理了代码中存在的中文字符，涉及到以下文件：
  - chip/B92/drivers: audio.c/audio.h/i2c.c/i2c.h/spi.h.
  - chip/B91/drivers/lib/include: pm.h.
  - chip/B92/drivers/lib/include: pm.h.
  - demo/vendor/AUDIO_Demo/AUDIO_V1.1/AUDIO_Demo: app_codec_0581.c/app_i2s.c.
  - demo/vendor/DUT_Test: dut_rf_frequency_offset_calib_slave.c.
  - demo/vendor/Flash_Demo: app.c.
  - demo/vendor/OTP_Demo: app.c.
  - demo/vendor/TRAP_Demo: app.c.

### Bug Fixes
* **trap**
  * (B91/B92)plic_set_threshold, core_interrupt_enable,core_interrupt_disable 和 core_restore_interrupt 接口，操作 PLIC 寄存器可能未成功，导致中断优先级功能可能出现异常，加入fence指令修复该问题。 [c97214d6](http://192.168.48.36/src/driver/telink_b91m_driver_src/merge_requests/617/diffs?commit_id=c97214d640478e3dca1dd1b828b2cb490eab130c)
  * (B91/B92)修复了同时使能如下配置时导致的 PLIC 阈值被设置为 0 的异常问题： [e0b8115b](http://192.168.48.36/src/driver/telink_b91m_driver_src/merge_requests/613/diffs?commit_id=e0b8115be1d294c548ba1f56ae2e86abade31d88)
    - plic_set_threshold 设置为非 IRQ_PRI_NUM0 值;
    - 调用函数 plic_preempt_feature_en 使能了中断抢占功能;
    - flash_plic_preempt_config 的 preempt_en 设置为1。
* **flash**
  * (B92)flash_read_data_decrypt_check/flash_dread_decrypt_check/flash_4read_decrypt_check:调用以上接口后，误将预取指令关闭，这个版本将其修复打开。 [2242c4e0](http://192.168.48.36/src/driver/telink_b91m_driver_src/merge_requests/620/diffs?commit_id=2242c4e0815369707030eea6e2b20ff128dc64e2)
  * (B91/B92)flash_get_lock_block_midxxxx:获取lock保护区域错误，这个版本将其修改。 [c6395c73](http://192.168.48.36/src/driver/telink_b91m_driver_src/merge_requests/622/diffs?commit_id=c6395c731d56d4419fe0f868e6c08d6c126fcb06)
* **pm**
  * (B92)pm_sleep_wakeup:修复了进入低功耗模式的方式不正常而导致的概率性死机问题：
    - 在进睡眠前等待MCU进入wfi模式后再进入低功耗模式。 [d33fba65](http://192.168.48.36/src/driver/telink_b91m_driver_src/merge_requests/580/diffs?commit_id=d33fba652f9848fed81d3d1db8d68af99b8c4896)
  * (B92)修复了因retention ldo电压偏低而导致的概率性死机问题。 [d33fba65](http://192.168.48.36/src/driver/telink_b91m_driver_src/merge_requests/580/diffs?commit_id=d33fba652f9848fed81d3d1db8d68af99b8c4896)
  * (B91/B92)规避了cpu_sleep_wakeup的返回值中STATUS_GPIO_ERR_NO_ENTER_PM 可能会被异常置为1的风险。 [e503c4eb](http://192.168.48.36/src/driver/telink_b91m_driver_src/merge_requests/719/diffs?commit_id=e503c4ebe3477ab15acaac67f8e45f95c55180a1)
  * (B92)修复了pm_clr_irq_status()可能会误清其他中断标志位的问题。 [69b24afa](http://192.168.48.36/src/driver/telink_b91m_driver_src/merge_requests/717/diffs?commit_id=69b24afadb7e477214c194d69717e4a1d3249c3f)
* **audio**
  * (B92)修复了 audio_set_stream0_dig_gain0() 接口设置 stream0 数字增益不生效的问题。 [f34468c2](http://192.168.48.36/src/driver/telink_b91m_driver_src/merge_requests/660/diffs?commit_id=f34468c2bf3e26f98ef05a37bd4919538fd4ce7b)
  * (B92)修复了重复调用 audio_power_on() 接口导致音频功能异常的问题。 [2f903fc6](http://192.168.48.36/src/driver/telink_b91m_driver_src/merge_requests/715/diffs?commit_id=2f903fc61a8cb5b8b144c6e1c481513b86992de5)
* **clock**
  * (B91/B92)修复了用户使用clock_init()切换不同时钟频率时,可能导致hclk/pclk/mspi_clk有短时间超出其最大可设置频率的风险。 [6a0b5cc2](http://192.168.48.36/src/driver/telink_b91m_driver_src/merge_requests/668/diffs?commit_id=6a0b5cc277dc4ba4966ba53ea450a9c92882e67a)
  * (B92)修复了用户使用cclk_hclk_pclk_config()切换cclk/hclk/pclk频率时，可能导致hclk/pclk有短时间超出其最大可设置频率的风险。 [6a0b5cc2](http://192.168.48.36/src/driver/telink_b91m_driver_src/merge_requests/668/diffs?commit_id=6a0b5cc277dc4ba4966ba53ea450a9c92882e67a)
  * (B92):pll_div_cclk_hclk_pclk_e: cclk/hclk/pclk存在一定的倍数关系，将不支持的关系进行屏蔽。 [2a1b07bd](http://192.168.48.36/src/driver/telink_b91m_driver_src/merge_requests/687/diffs?commit_id=2a1b07bd6b20356b160ecdd74df1592cd50669f8)
* **uart**
  * (B91)当dma接收数据时，解决发送长度大于接收长度时rxdone无法产生的问题。 [a853aaca](http://192.168.48.36/src/driver/telink_b91m_driver_src/merge_requests/691/diffs?commit_id=a853aaca634913b7dbb559283e960febf5479871)
  * (B91)解决程序进入睡眠被唤醒后DMA无法发送的问题。 [b309dfbf](http://192.168.48.36/src/driver/telink_b91m_driver_src/merge_requests/705/diffs?commit_id=b309dfbfd9bcb3c1c96984cea5ad09fa12f2c7f4)
  * (B91)修复了uart_clr_irq_status()可能会误清其他中断标志位的问题。 [69b24afa](http://192.168.48.36/src/driver/telink_b91m_driver_src/merge_requests/717/diffs?commit_id=69b24afadb7e477214c194d69717e4a1d3249c3f)
* **rf**
  * (B92)修复了 rf_set_ble_500K_mode驱动接口内配置错误问题。 [46a18667](http://192.168.48.36/src/driver/telink_b91m_driver_src/merge_requests/695/diffs?commit_id=46a18667e2dccef1c7add17893e66526089f20b6)
  * (B92)修复了rf_start_srx2tx参数tick不起作用导致收发包异常的问题。 [f0cdfeb5](http://192.168.48.36/src/driver/telink_b91m_driver_src/merge_requests/764/diffs?commit_id=f0cdfeb5dba0c4df171a0eaca544a3897bcf98ce)
  * (B91)修复了接口rf_aoa_aod_hdinfo_offset()和rf_aoa_aod_get_pkt_timestamp()返回值错误问题。 [04bd6d9c](http://192.168.48.36/src/driver/telink_b91m_driver_src/merge_requests/770/diffs?commit_id=04bd6d9c1472433047ac243eb13353f6976ebbf8)
* **mdec**
  * (B91/B92)修复了mdec_clr_irq_status()可能会误清其他中断标志位的问题。 [69b24afa](http://192.168.48.36/src/driver/telink_b91m_driver_src/merge_requests/717/diffs?commit_id=69b24afadb7e477214c194d69717e4a1d3249c3f)
* **i2c**
  * (B92)修复了i2c_clr_irq_status()可能会误清其他中断标志位的问题。 [69b24afa](http://192.168.48.36/src/driver/telink_b91m_driver_src/merge_requests/717/diffs?commit_id=69b24afadb7e477214c194d69717e4a1d3249c3f)
* **pwm** 
  * (B92)修改了pwm_get_irq_status()可能返回错误的中断状态判断以及会误清中断标志位的问题。 [5a0e1014](http://192.168.48.36/src/driver/telink_b91m_driver_src/merge_requests/700/diffs?commit_id=5a0e1014e2340b2a9c661ecc2d79c64247cbc48e)
* **gpio**
  * (B91)修复uart_set_pin()/hspi_set_pin()/pspi_set_pin()中不需要使用的pin没有被初始化为NONE_PIN而导致PA0功能异常的问题。 [ef3ff6f1](http://192.168.48.36/src/driver/telink_b91m_driver_src/merge_requests/753/diffs?commit_id=ef3ff6f194fc513802b98afd4263923445555e40)
* **flash**
  * (B91/B92)修复了flash_write()、flash_write_otp()、flash_write_encrypt()函数，误将参数len设置为0可能触发的异常问题。 [291e8752](http://192.168.48.36/src/driver/telink_b91m_driver_src/merge_requests/743/diffs?commit_id=291e8752e69460bc9d3d46e8629b9374d5348dcc)
* **adc**
  * (B92)修复efuse_calib_adc_vref()接口取出offset为负数时没做转换导致校准错误的问题。 [291e8752](http://192.168.48.36/src/driver/telink_b91m_driver_src/merge_requests/743/diffs?commit_id=291e8752e69460bc9d3d46e8629b9374d5348dcc)
* **USB_Demo**
  * (B91/B92)修复了 USB 枚举时，host 请求长度小于实际描述符长度时，device 返回过多数据的 bug。 [4f97a6f1](http://192.168.48.36/src/driver/telink_b91m_driver_src/merge_requests/762/diffs?commit_id=4f97a6f1f2be7b62310bc4061d194c8f9b0b5821)
  * (B91/B92)修复了主机请求接口特殊描述符时， HID 描述符返回数据过多的 bug。 [4f97a6f1](http://192.168.48.36/src/driver/telink_b91m_driver_src/merge_requests/762/diffs?commit_id=4f97a6f1f2be7b62310bc4061d194c8f9b0b5821)
* **TRAP_Demo**
  * (B91/B92)修复在 linux 环境下由大小写混用引起的编译错误。 [5131b7d0](http://192.168.48.36/src/driver/telink_b91m_driver_src/merge_requests/612/diffs?commit_id=5131b7d09e0eb4e0cbea84c1ac9119da4018ce70)
  * (B91)修复当全局中断关闭时，WFI 模式无法重复进入与唤醒的问题。 [08c0ddab](http://192.168.48.36/src/driver/telink_b91m_driver_src/merge_requests/595/diffs?commit_id=08c0ddabfb30e08ad2c50d0b7a33d1240ca4f600)
* **I2C_Demo**
  * (B92) 当slave端(I2C_STRETCH_MODE  == I2C_STRETCH_DIS),中断使能错误，导致收发异常。 [9ddf8348](http://192.168.48.36/src/driver/telink_b91m_driver_src/merge_requests/589/diffs?commit_id=9ddf8348849f009574a4d5b7eee3edc6b927992c)
* **SPI_Demo**
  * (B92)spi_master_read_dma_plus()/spi_master_write_read_dma():判断是否结束使用dma_set_irq_mask代替；SPI_END_EN只能代表spi接收完成，不能代表dma是否工作完，防止出现异常情况。 [7bdb43d4](http://192.168.48.36/src/driver/telink_b91m_driver_src/merge_requests/598/diffs?commit_id=7bdb43d406e33ce6571372b0752b34a5df200d70)
* **UART_DEMO**
  * (B91/B92)当nodma接收数据时，使用uart_get_rxfifo_num()替代uart_rx_trig_level判断在中断一次性读取多少数据，解决应用中中断被打断导致数据没有来得及取减少fifo指针紊乱的风险。 [a853aaca](http://192.168.48.36/src/driver/telink_b91m_driver_src/merge_requests/691/diffs?commit_id=a853aaca634913b7dbb559283e960febf5479871)

### BREAKING CHANGES 
* **i2c**
  * (B92)将clear相关的接口i2c_clr_fifo(i2c_buff_clr_e clr)和i2c_clr_irq_status(i2c_irq_clr_e status)合并到接口i2c_clr_irq_status(i2c_irq_status_e status),对应的,删除接口i2c_clr_fifo,删除枚举i2c_buff_clr_e和i2c_irq_clr_e(合并到枚举i2c_irq_status_e)。
* **trap**
  * (B91/B92) core_interrupt_enable() 函数调用新增接口 core_mie_enable 使能 MEI，并且不再使能 MTI 和 MSI。
  * (B91/B92) 由于未使用中断服务函数占用 RAM 过大，所以新增一个 PLIC_ISR_REGISTER 宏让应用程序注册 PLIC 中断服务函数。相应地，删除 plic_isr.c 文件，并移除 irq_source_e 枚举，使用 irq_source_index 代替。
  * (B92) 删除接口 mtime_enable()/mtime_disable()，使用功能更全（包含mtime）的接口core_mie_enable/core_mie_disable 代替。
  * (B91/B92) plic_preempt_feature_en() 函数增加了一个 core_preempt_pri_e 类型的参数，该参数是为了配置 MEI 能否被 MSI 和 MTI 打断的中断嵌套优先级。
* **pm**
  * (B91/B92)pm_sleep_wakeup()函数中去掉了对于system timer中断mask的处理（睡眠之前关掉，suspend唤醒之后打开），现将该mask完全交由用户处理，如果用户需要使用该mask，可以调用这两个函数进行设置:stimer_set_irq_mask()和stimer_clr_irq_mask()。
  * (B91/B92)占用PM_ANA_REG_POWER_ON_CLR_BUF0[bit2]（0x3a[2]）如果该位为1，表示使用深度唤醒源时，pm_sleep_wakeup函数清除pm唤醒标志位失败，软件sys_reboot()，客户不能使用这个bit。
  * (B92)结构体类型pm_r_delay_cycle_s新增两个成员：deep_xtal_delay_cycle和suspend_ret_xtal_delay_cycle。
* **IDE**
  * (B91)重新修改b91的ide为TelinkIoTStudio，工具链仍然为GCC7。

### Features
* **audio**
  * (B91)新增 audio_codec_active() 和 audio_codec_set_micbias() 接口用来控制 micbias 使能和电压选择。
  * (B92)新增 codec_0581 驱动和示例代码。
* **rf**
  * (B91)增加了fast settle相关接口：
rf_rx_fast_settle_init();rf_tx_fast_settle_init();rf_tx_fast_settle_en();rf_tx_fast_settle_dis();rf_rx_fast_settle_en（）；rf_rx_fast_settle_dis();rf_get_ldo_trim_val();rf_set_ldo_trim_val();
rf_get_hpmc_cal_val(); rf_set_hpmc_cal_val();rf_get_dcoc_cal_val();
rf_set_dcoc_cal_val();
  * (B92)增加了fast settle相关接口：rf_rx_fast_settle_init();rf_tx_fast_settle_init();rf_tx_fast_settle_en();rf_tx_fast_settle_dis();rf_rx_fast_settle_en（）；rf_rx_fast_settle_dis();rf_get_ldo_trim_val();rf_set_ldo_trim_val();
rf_get_hpmc_cal_val(); rf_set_hpmc_cal_val();rf_get_dcoc_cal_val();
rf_set_dcoc_cal_val();rf_get_rccal_cal_val();rf_set_rccal_cal_val();
rf_dis_hpmc_trim();rf_dis_ldo_trim();rf_dis_dcoc_trim();rf_dis_rccal_trim();
  * (B92)新增接口：rf_set_power_off_singletone();rf_aoa_aod_set_trx_mode();rf_aoa_aod_sample_point_adjust();rf_aoa_rx_ant_switch_point_adjust();rf_aod_tx_ant_switch_point_adjust();rf_aoa_aod_sample_interval_time();rf_aoa_aod_iq_data_mode();
* **trap**
  * (B91)添加 mtime 和软件中断接口，并添加对应示例。
  * (B92)添加 WFI（等待中断）和 HSP（硬件堆栈保护）接口，并添加对应示例。
* **pm**
  * (B92)添加接口pm_get_reboot_event()判断reboot状态.
* **pm**
  * (B92)添加接口pm_set_active_vddo3()用于设置VDDO3的电压。
* **gpio**
  * (B92)添加了jtag_set_pin_en()/sdp_set_pin_en()接口。
* **TRAP_Demo**
  * (B92)except_handler 函数中读取 mcause 寄存器.
* **USB_Demo**
  * (B92)添加 codec_0581 的 mic 和 spk 功能支持。
* **EMI_BQB_Demo**
  * (B92)emi carrieronly模式新增tone跳频功能。
  * (B92)emi tx burst模式增加发送pkt+tone功能。
* **I2C_Demo**
  * (B92)添加 i2c1_m(master)与 b91m i2c slave通信的demo。

### Refactoring
* **S**
  * (B92)调整代码，减小配置错SRAM的概率。
  * (B91)为了和B92统一，也为了给后续的flash留有余量，flash唤醒命令到standby模式的等待时间增加到25us。
* **rf**
  * (B91/B92)修改rf_set_power_level_index();函数定义,删除_attribute_ram_code_sec_
  * (B91/B92)为rf_power_Level_list表添加const关键字
* **pm** 
  * (B91)优化pm_sleep_wakeup函数：睡眠前将mspi clk切换到24M rc时钟，flash唤醒位置提前到pll稳定之前。
  * (B91/B92)整理代码，确保睡眠函数中所有调用的函数都是ramcode。
  * (B92)修改枚举类型pm_suspend_power_cfg_e : 删除 PM_POWER_AUDIO 枚举变量.
* **trap**
  * (B91/B92)为了多芯片统一，修改了irq_source_index名称，b91/b92 对以前的名称做了兼容，后续芯片不再支持。
* **flash**
  * (B91/B92)修改了flash_erase_sector()的参数“addr”的注释为:必须是0或0x1000的倍数。
* **sys**
  * (B92)_ASM_NOP_定义添加volatile关键字，以防编译器优化。并新增程序中会用到的宏CLOCK_DLY_64_CYC和CLOCK_NOP_DLY_xUS。
* **clock**
  * (B92)clock_set_32k_tick优化函数中的nop写法。
* **spi**
  * (B91/B92)为了优化结构体内存分布，调整了结构体成员位置（将枚举成员放在后面），不影响功能。
* **audio**
  * (B91/B92)为了优化结构体内存分布，调整了结构体成员位置（将占用长度确定的成员和对齐的成员放在前面，枚举成员放在后面），不影响功能。
* **ALG_REG_Demo**
  * (B91)优化Demo的代码结构，增强复用性。
* **USB_Demo**
  * (B91/B92)实现mic/spk接口描述符中的SubFrameSize可配置。
  * (B91/B92)在USB相关结构体添加__attribute__((packed))，避免 -fpack-struct 编译选项去除后导致功能异常。
* **UART_DEMO**
  * (B91/B92)app.c/app_dma.c:添加宏定义UART_MODULE_SEL，通过该宏控制选择哪一路uart.
  * (B92)app_dma_llp.c:1.添加宏定义UART_MODULE_SEL，通过该宏控制选择哪一路uart; 2.为了代码统一，使用dma TC_MASK代替uart UART_RXDONE_MASK.
* **ALL_Demo**
  * (B91/B92)添加了common.c和common.h,将LED、KEY的定义放在common.h,在common.c定义platform_init接口，将公共的初始化内容放platform_init接口中，初始化内容新增了给sws设置上拉，以防概率性死机问题。
  * (B91/B92)为了处理由警告选项-Wstrict-prototypes带来的编译警告, 以下demo文件已经通过向无参函数声明中传入void进行了修改:
    - ADC_Demo, AES_Demo, ALG_REG_Demo, AUDIO_Demo, AXON_Demo, Auto_Test_Demo, CTB_Demo, Charger_Demo, Coremark_Demo, DUT_Demo, Debug_Demo, Dhrystone_Demo, Display_Demo, EMI_BQB_Demo, Flash_Demo, GPIO_Demo, HADM_Demo, I2C_Demo, OTP_Demo, PKE_Demo, QDEC_Demo, RF_Demo, SKE_Demo, SPI_Demo, STimer_Demo, Swire_Demo, TRAP_Demo, Test_Demo, UART_Demo, USB_Demo, s7816_Demo  
* **project**
  * (B92)修改了B92的工程配置，涉及到以下内容:
    - 在原有警告选项的基础上(-Wall -Wextra -Wshadow -Werror), 新增了以下警告选项: - -Wimplicit-fallthrough -Wredundant-decls -Wcast-qual -Wsign-compare -Wunused-parameter -Wunused-variable -Wswitch -Wstrict-prototypes -Wmissing-field-initializers -Wpointer-arith -Wdeprecated-declarations -Werror=cpp -Wenum-conversion -Wpacked-not-aligned -Waddress-of-packed-member -Wundef
    - 新增了预处理器符号定义:
      - MCU_CORE_B91=0
      - MCU_CORE_B930=0
      - MCU_CORE_B95=0
      - MCU_CORE_B96=0
* **driver src**
  * (B92)为了处理由警告选项-Wstrict-prototypes带来的编译警告, 以下源文件已经通过向无参函数声明中传入void进行了修改:
    - chip/B92/drivers: analog.c/i2c.h/mspi.h/plic_sw.h/s7816.h
    - chip/B92/drivers/lib/include: hadm.h/hadm_drv.h/rf.h/swire.h
    - chip/usb_dbg: myudb.h/myudb_usb.c
  * (B91/B92)为了处理由警告选项-Wcast-qual带来的编译警告, 以下源文件已经通过向相关变量添加const关键字或删除volatile关键字进行了修改:
    - chip/B91/drivers/lib/src/pke: pke.c 
    - chip/B91/drivers/lib/include/pke: pke.h
    - chip/B92/drivers/lib/src/pke: pke.c 
    - chip/B92/drivers/lib/include/pke: pke.h
    - chip/B92/drivers/lib/include/pke: eccp_curve.h
    - chip/B92/drivers/ext_peripherals/codec_0581: codec_0581.h
    - chip/B92/drivers: audio.h

### Performance Improvements
* **S**
  * (B92)为了节省功耗，在retention唤醒时，缩短了flash唤醒命令到standby模式的等待时间。

<hr style="border-bottom:2.5px solid rgb(146, 240, 161)">

## V2.2.0

### Version

* SDK version: telink_b91m_driver_sdk V2.2.0.
* This version of the SDK supports B91(A0/A1/A2),B92(A1/A2) chips.
* The default configuration of LEDs and KEYs match the following hardware revisions:
*   B91	 	C1T213A20_V1_3_20200727
*   B92	 	C1T266A20_V1_3_20220722

### Note

* telink_b91m_driver_sdk v2.0, this version corrects spelling errors and resolves compilation warnings, so there are some CHANGES that affect compatibility, please read the BREAKING CHANGES section for more information that affects compatibility.

### BREAKING CHANGES

* **watchdog**
  * (B92)charger_clear_vbus_detect_status()/charger_get_vbus_detect_status() were renamed wd_turn_off_vbus_timer()/usb_get_vbus_detect_status().
* **audio**
  * (B92)audio_set_codec_stream0_path()/audio_set_codec_stream1_path()/audio_codec_swap_stream0_data()/audio_codec_swap_stream1_d ata() has all new sampling rate parameters.
  * (B91)The structure aduio_i2s_clk_config_t change to audio_i2s_clk_config_t.
  * (B92)The interface aduio_i2s_set_mclk change to audio_i2s_set_mclk.
* **sys**
  * (B92)gpio_voltage_e added to sys_init function to configure the gpio voltage type(GPIO_VOLTAGE_3V3/GPIO_VOLTAGE_1V8).
  * (B92)Modify 1P2 to 1P4 in the enumeration type power_mode_e.
* **i2c** 
  * (B91)The register reg_i2c_slave_strech_en change it to reg_i2c_slave_stretch_en.
  * (B92)The interface i2c_slave_strech_en/i2c_slave_strech_dis/i2c_master_strech_en/i2c_master_strech_dis change them to i2c_slave_stretch_en/i2c_slave_stretch_dis/i2c_master_stretch_en/i2c_master_stretch_dis separately.
  * (B92)The enumeration member FLD_I2C_MASTER_STRECH_EN/FLD_I2C_SLAVE_STRECH_EN change them FLD_I2C_MASTER_STRETCH_EN/FLD_I2C_SLAVE_STRETCH_EN separately.
* **spi**
  * (B91)The enumeration spi_nomal_3line_mode_e and the enumeration member SPI_NOMAL change them spi_normal_3line_mode_e and SPI_NORMAL(the spi_master_config interface is associated with this enumeration).
  * (B92)Remove the spi_pin_config_t structure，add gspi_pin_config_t and lspi_pin_config_t,
     change the parameter of gspi_set_pin function to gspi_pin_config_t pointer type, and change the parameter of lspi_set_pin function to gspi_pin_config_t pointer type.
* **rf**
   * (B91)(B92)The enumeration member FLD_RF_R_NOACK_RECNT_EN/FLD_RF_R_CMD_SCHDULE_EN change them FLD_RF_R_NOACK_RETRY_CNT_EN/FLD_RF_R_CMD_SCHEDULE_EN separately.
   * (B91)(B92)The register reg_rf_modem_sync_thre_ble change to reg_rf_modem_sync_thres_ble.
   * (B92)The rf_set_channel_fpga frequency setting function in FPGA verification phase was deleted and the rf_set_chn interface was used instead.
   * (B92)Updated rf energy meters for the latest supply voltage configurations.
   * (B91/B92)Removed the const keyword from the rf_power_Level_list table.
* **flash**
  * (B92)The interface flash_quad_page_pragram_encrypt change to flash_quad_page_program_encrypt.
* **usb**
  * (B91)(B92)The register reg_usb_ep8_send_thre change to reg_usb_ep8_send_thres.
* **calibration**
  * (B92)The calibration value interface user_read_flash_value_calib() is changed to calibration_func().
* **pm**  
  * (B92)Occupies PM_ANA_REG_POWER_ON_CLR_BUF0[bit1] (0x3a[1]) to mark whether restart caused by abnormal start-up has occurred, the customer cannot use this bit.
* **uart**
  * (B92)uart_get_rx_buff_cnt is changed to uart_get_rxfifo_rem_cnt.
  * (B92)Delete the uart_rts_auto_mode/uart_rts_manual_mode interface.
* **ide**
  * (B91)The IDE for B91 is changed to TelinkIoTStudio, and the toolchain remains GCC7.

  
### Bug Fixes

* **gpio**
  * (B92)The digital Pull-up resistor of PF [5:0] and PG [5:0] is not available, and these pins are prohibited in the interface of gpio_set_pullup_res_30k(). [ad91c3e5](http://192.168.48.36/src/driver/telink_b91m_driver_src/merge_requests/566/diffs?commit_id=ad91c3e57719db168a95846f8022a0c7c9bf8ec5)
* **rf**
  * (B92)Fixed the issue that the RSSI value did not match the actual received energy. [435541cf](http://192.168.48.36/src/driver/telink_b91m_driver_src/merge_requests/444/diffs?commit_id=435541cf3d9776581b57883579b54cb4421f08fd)
  *	(B91/B92)Fixed the problem of incorrect setting of ctrim at some frequency points of rf_set_chn, and reduced the tx energy difference at different frequency points. [8ead48a7](http://192.168.48.36/src/driver/telink_b91m_driver_src/merge_requests/541/diffs?commit_id=8ead48a7e05d90ee0cd64d68ef475f8275974b9f)
  *	(B91/B92)The SETTLE_Time setting error caused by 4bit settle_time not taking effect when the rf_set_tx_settle_time interface is set, the SETTLE_time setting range is extended to 0xfff. [8ead48a7](http://192.168.48.36/src/driver/telink_b91m_driver_src/merge_requests/541/diffs?commit_id=8ead48a7e05d90ee0cd64d68ef475f8275974b9f)
* **emi**
  * (B91/B92)Fixed inconsistency between rx packet length and dma fifo size causing FIFO data overflow to overwrite other RAM information. [95a0c226](http://192.168.48.36/src/driver/telink_b91m_driver_src/merge_requests/450/diffs?commit_id=95a0c22606473f33a8f5032cf0eeb7660bd94a16)
* **audio**
  * (B92)The channel data exchange problem is solved when the sampling rate of DMIC is 44.1k and 48k. [2928be92](http://192.168.48.36/src/driver/telink_b91m_driver_src/merge_requests/453/diffs?commit_id=2928be92d4f9dbd3b52f2633ae13356b3e100c43) 
* **spi**
  * (B92)Fixed the slave gpsi rx dma receiving the first data error. [03fdb77b](http://192.168.48.36/src/driver/telink_b91m_driver_src/merge_requests/452/diffs?commit_id=03fdb77bfb4840573ff6bad797a1deea9ebb32ed)
* **mspi**
  * (B92)The related function is modified with ramcode, to avoid the risk that the mspi-related function may be compiled into the text segment and cause crash. [7ce63726](http://192.168.48.36/src/driver/telink_b91m_driver_src/merge_requests/465/diffs?commit_id=7ce63726d14f7135e8efd37d7b183a6e52577bd2)
* **sys**
  * (B92)sys_reboot: the definition of sys_reboot is changed to sys.c, and the sys_reboot_ram interface is added. the processing of sys_reboot is stored in ram to avoid incorrect operations on flash when sys_reboot is called. [7ce63726](http://192.168.48.36/src/driver/telink_b91m_driver_src/merge_requests/465/diffs?commit_id=7ce63726d14f7135e8efd37d7b183a6e52577bd2)
  * (B91/B92)The sys_reboot interface was rewritten to avoid crash due to prefetch and interruption. [1cd3cc0e](http://192.168.48.36/src/driver/telink_b91m_driver_src/merge_requests/479/diffs?commit_id=1cd3cc0e1c14780aad3f729d8db6917ba44c66c9)
  * (B92)The power supply of sram is about 1.2V. Adjusting the voltage to the ram in different states can be maintained at 1.2V. [1cd3cc0e](http://192.168.48.36/src/driver/telink_b91m_driver_src/merge_requests/479/diffs?commit_id=1cd3cc0e1c14780aad3f729d8db6917ba44c66c9)
  * (B92)Solve the problem that the crystal oscillator stability flag fails to cause the crash. If the start-up is abnormal, it will restart. Use PM_ANA_REG_POWER_ON_CLR_BUF0[bit1] to check whether the restart caused by the abnormal start-up has occurred. (It has been implemented in the driver layer, and does not require special calls from the application layer.) [406f518d](http://192.168.48.36/src/driver/telink_b91m_driver_src/merge_requests/540/diffs?commit_id=406f518d43f134209d4dc9c81bc40523208a93ad)
  * (B92)The clock sources for configuring cclk, hclk, pclk, and mspiclk during initialization are all from 24M rc. [2b85e90e](http://192.168.48.36/src/driver/telink_b91m_driver_src/merge_requests/551/diffs?commit_id=2b85e90e6f30097f0f400212951fca7712666936)
  * (B92)The risk of digital core voltages greater than the maximum operating voltage of 1.32V is avoided. The digital core voltage is configured at 1.2V. [cb5916dc](http://192.168.48.36/src/driver/telink_b91m_driver_src/merge_requests/573/diffs?commit_id=cb5916dc4b5503b19cc92744e4a7471c3b88205c)
* **clock**
  * (B91)clock_init: added clock_init_ram interface to store the processing of clock_init in the ram to avoid possible machine crashes caused by improper operations during clock configuration. [7ce63726](http://192.168.48.36/src/driver/telink_b91m_driver_src/merge_requests/465/diffs?commit_id=7ce63726d14f7135e8efd37d7b183a6e52577bd2)
* **uart**
  * (B92)uart_receive_dma interface directly uses UART0 instead of parameter uart_num, modify it. [f7cebf4e](http://192.168.48.36/src/driver/telink_b91m_driver_src/merge_requests/516/diffs?commit_id=f7cebf4e152151ca130c1c3861eff0f856985b10)
* **gpio**
  * (B92)Modify the gpio_set_src_irq() configuration to resolved the problem that the group src level interrupt could not work properly.[e9581b0a](http://192.168.48.36/src/driver/telink_b91m_driver_src/merge_requests/511/commits?commit_id=e9581b0a098b9230269a5389a98a82c9e4a90bb3)
* **dma** 
  * (B92/B91) Fix the problem that if the buff parameter in audio_rx/tx_dma_chain_init interface is in IRAM area(DRAM is not affected), then calling audio_get_rx_dma_wptr/audio_get_tx_dma_rptr interface will cause error.  Reason: The return value of convert_ram_addr_cpu2bus is written to the dma-related register,The value of the register is read out as input parameter of convert_ram_addr_bus2cpu,it will be calculated incorrectly. [cfcb91ef](http://192.168.48.36/src/driver/telink_b91m_driver_src/merge_requests/543/diffs?commit_id=cfcb91ef95289d327a78c014a401705c53d7118a)
* **pwm**
  * (B91)In the pwm_clr_irq_status interface, fix the problem that the FLD_PWM0_IR_FIFO_IRQ interrupt flag bit is not cleared correctly. [e0dfb211](http://192.168.48.36/src/driver/telink_b91m_driver_src/merge_requests/532/diffs?commit_id=e0dfb21137f0e1396618ac246e10626d941093e0)
* **pm**
  * (B92)Solved the problem of high current in suspend sleep caused by leakage caused by the failure to connect the up and down pull of sws. [eb6c0261](http://192.168.48.36/src/driver/telink_b91m_driver_src/merge_requests/466/diffs?commit_id=eb6c0261101c513f228a1a9dbf02dc9832f088d6)
* **I2C_Demo**
  * (B92)The received packet data is abnormal due to a code logic error. [106435f3](http://192.168.48.36/src/driver/telink_b91m_driver_src/merge_requests/443/diffs?commit_id=106435f316711e1b27c9b7a095969ec2e1fca7aa)
* **USB_Demo**
  * (B92)Fixed the problem of not turn off the 8s vbus timer after unplugging and reinserting the vbus while the power is on(Vbat supply). [b36d04e1](http://192.168.48.36/src/driver/telink_b91m_driver_src/merge_requests/451/diffs?commit_id=b36d04e10302fd0eb09c30d9a5e41fb1bc533096)
  * (B91/B92)Fixed an issue that may enter an unexpected endpoint interrupt due to the endpoint interrupt mask being fully turned on by default.[2a22e068](http://192.168.48.36/src/driver/telink_b91m_driver_src/merge_requests/454/diffs?commit_id=2a22e0684b524a11c22db86fe932164d56642ed6)
  * (B91)Fixed mic_demo MIC_DMA_CHN undefined and spk_demo #ifdef #endif not used in pairs bug. [a2efbbb2](http://192.168.48.36/src/driver/telink_b91m_driver_src/merge_requests/498/diffs?commit_id=a2efbbb2853f979e84fc2dc0464ff0b55c827cbb)
### Features

* **usb**
  * (B92)Add set config and get config support when disable auto config.
* **flash**
  * (B92)Add 2M flash(GD25LQ16E).
* **pm**
  * (B92)Add the function of clearing the pm interrupt flag bit.
* **audio**
  * (B91)Added audio adc/dac 24k sampling rate configuration option to the audio_set_i2s_clock() function interface..
* **rf**
  * (B91/B92) Added interfaces rf_ldot_ldo_rxtxlf_bypass_en(), rf_ldot_ldo_rxtxlf_bypass_dis(), rf_set_rxpara() related to improving the sensitivity performance of the emi/bqb program at 24M multiplier frequency points.
  * (B92) Update the energy meter to support energy setting when the GPIO output voltage is 1.8V.
* **aes**
  * (B91/B92) Added interfaces to prevent conflicting AES encryption and decryption - aes_encrypt_bt_en and aes_decrypt_bt_en.
* **clock**
  * (B91/B92)Added the configuration of CCLK to 96M. 
  * (B92)When the PLL is 192M, add the function interface cclk_hclk_pclk_config() for adjusting the CCLK/HCLK/PCLK frequency and the corresponding enumeration pll_div_cclk_hclk_pclk_e. 
* **flash**
  * (B91/B92)In each flash, add the interface flash_get_lock_block_midxxx to return which area is currently locked in flash (also in midxxx_lock_block_e, Note When flash write protection is used, only the midxxx_lock_block_e enumeration value can be selected, so that the values returned by the flash_get_lock_block_midxxx interface are in midxxx_lock_block_e).
  * (B91/B92)In flash.h, the enumeration flash_mid_e is added,can use the enumeration value instead of flash_mid to increase readability.
  * (B91/B92)flash.h, add FLASH_SIZE_16M to flash_capacity_e.
* **efuse**
  * (B92)Add the interface efuse_calib_adc_vref() to read adc calibration values from efuse.
* **adc**
  * (B92)Add adc calibration value configuration interfaces adc_set_gpio_calib_vref()、 adc_set_gpio_two_point_calib_offset()、adc_set_vbat_calib_vref()和adc_set_vbat_two_point_calib_offset(), and optimize some comments.
* **all**
  * (B92)Add calibration_func() for all demo.
* **i2c**
  * (B91)Add interfaces related to slave/matser stretch:i2c_slave_stretch_en/i2c_slave_stretch_dis/i2c_slave_manual_stretch_en/i2c_slave_manual_stretch_clr/i2c_master_stretch_en/i2c_master_stretch_dis.
  * (B91)In i2c_master_init, the master stretch function is enabled by default (i2c_master_stretch_en), no matter whether the slave is stretch or not, the master side will not be abnormal.
  * (B92)In i2c_master_init, the master stretch function is enabled by default (i2c_master_stretch_en), no matter whether the slave is stretch or not, the master side will not be abnormal.
  * (B92)In i2c_master_init, the i2c_master_detect_nack_en function is turned on by default,no matter whether the slave sends an nack, the master is normal.
* **PM_Demo**
  * (B92)Add PM demo.
* **USB_Demo**
  * (B91)Add msc demo based on tinyusb.
* **EMI_BQB_Demo**
  * (B91)Added EMI_Demo initial configuration function and PA configuration method.
  * (B92)Added EMI_BQB_Demo, and support RF current test function.
* **Flash_Demo**
  * (B91/B92)Maintain flash_lock/flash_unlock interfaces(to determine whether the flash is locked or unlocked), and list all flash interfaces to avoid omissions at the application layer due to subsequent flash additions.
* **TRAP_Demo**
  * (B92)Add machine timer interrupt, software interrupt and add corresponding demo to TRAP_Demo.

### Refactoring

* **aes**
  * (B91/B92)Update the comment of the function interface aes_set_em_base_addr.
* **dma**
  * (B91/B92)Add a note for peripherals using dma: the src and dst addresses of dma must be four-byte aligned,otherwise the program will enter an exception.
  * (B91/B92)Add comments about peripherals used in conjunction with dma.
* **rf**
  * (B91/B92)Set the rf_set_chn function to the _attribute_ram_code_sec_noinline_ type according to the application requirements.
  * (B91/B92)Simplified the implementation of rf_update_internal_cap.
  * (B91/B92)Define rf_set_power_level_index in ram.
* **all**
* (B91/B92)Normalizes the naming of content related to private schemas.
* (B92)Clear all warning errors from DriverSDK.
* **adc**
  * (B91/B92)Add comments to g_adc_vref and corrected some interface comments.
* **gpio**
* (B92)Peripheral some functions pin is not used and not set to GPIO_NONE_PIN, resulting in pin set to PA0 by mistake, modify GPIO_NONE_PIN to 0 to optimize the problem.
* **audio**
* (B92)Audio_set_codec_stream0/1_path interface internal optimization.
* **project**
  * (B92)Add related warning options in Telink IoT Studio: -Wall,-Wextra,-Wshadow,-Werror, and clear related warnings and errors.Clear coding warnings, 
  * (B92)Add functions, jump configurations for various data structures.
  * (B91/B92)Remove compilation options: -Wno-gnu-zero-variadic-macro-arguments(gcc7 and gcc10 no longer support this option).
* **uart**
  *  (B92)uart_get_dma_rev_data_len:simplify the function logic while ensuring the function remains unchanged.
* **adc**
  * (B91)Adjust the position of adc_set_gpio_two_point_calib_offset(), and optimize some comments.
* **gpio/i2c/spi/stimer/pm**
  * (B91/B92):The gpio.h,i2c.h,spi.h,stimer.h add stdbool header files,modified for pm_wakeup_status_e pm_get_wakeup_src function returns. 
* **keyscan/stimer/watchdog/aes/irlearn/adc/lpc/ctb/charger/pwm/timer/gpio**
  * (B91/B92)Correct spelling errors.
* **pm**
  * (B92)Add comments on some gpio pins that cannot be used as a pm wake source.
* **PLIC_Demo**
  * (B91/B92)PLIC_demo merge to TRAP_Demo and delete PLIC_Demo.


### Performance Improvements

* **rf**
  * (B92)Optimize the sensitivity of BLE2M, Private 2M, zigbee, hybee mode.
* **sys**
  * (B92)Adjust each ldo inside the chip to set it to normal voltage gear.
  * (B92)Adjust the VDDO3 output to 1.8V or 3.3V.
* **pm**
  * (B91)Modify the pm_sleep_wakeup function to make suspend sleep time more accurate during long sleep.
* **adc**
  * (B91) Stricter determination of illegal values for ADC calibration values.
* **EMI_BQB_Demo**
  * (B91/B92) Use rf_ldot_ldo_rxtxlf_bypass_en(), rf_ldot_ldo_rxtxlf_bypass_dis(), rf_set_rxpara()
interfaces to improve the sensitivity performance of emi/bqb programs at 24M multiplier frequency points.
### Note

* N/A

<hr style="border-bottom:2.5px solid rgb(146, 240, 161)">


### 版本

* SDK版本: telink_b91m_driver_sdk V2.2.0。
* 此版本SDK支持 B91(A0/A1/A2),B92(A1/A2) 芯片。
* LED和KEY的默认配置匹配以下硬件版本:
*   B91	 	C1T213A20_V1_3_20200727
*   B92	 	C1T266A20_V1_3_20220722

### Note

* telink_b91m_driver_sdk V2.2.0,该版本会纠正拼写错误并解决编译警告，因此会有一些影响兼容性的修改，影响兼容性的部分，请详细阅读BREAKING CHANGES部分。

### BREAKING CHANGES
* **watchdog**
  * (B92)charger_clear_vbus_detect_status()/charger_get_vbus_detect_status()分别改名为wd_turn_off_vbus_timer()/usb_get_vbus_detect_status()。
* **audio**
  * (B92)audio_set_codec_stream0_path()/audio_set_codec_stream1_path()/audio_codec_swap_stream0_data()/audio_codec_swap_stream1_data()全部新增加了采样率的形参。
  * (B91)结构体aduio_i2s_clk_config_t改为audio_i2s_clk_config_t。
  * (B92)接口aduio_i2s_set_mclk改为audio_i2s_set_mclk。
* **sys**
  * (B92)sys_init函数新增gpio_voltage_e参数，用于配置gpio电压类型（GPIO_VOLTAGE_3V3/GPIO_VOLTAGE_1V8）。
  * (B92)将枚举类型power_mode_e中的1P2修改为1P4
* **i2c** 
  * (B91)寄存器reg_i2c_slave_strech_en改为reg_i2c_slave_stretch_en。
  * (B92)接口i2c_slave_strech_en/i2c_slave_strech_dis/i2c_master_strech_en/i2c_master_strech_dis分别改为 i2c_slave_stretch_en/i2c_slave_stretch_dis/i2c_master_stretch_en/i2c_master_stretch_dis。
  * (B92)枚举成员FLD_I2C_MASTER_STRECH_EN/FLD_I2C_SLAVE_STRECH_EN分别改为FLD_I2C_MASTER_STRETCH_EN/FLD_I2C_SLAVE_STRETCH_EN。
* **spi**
  * (B91)枚举名spi_nomal_3line_mode_e以及枚举成员SPI_NOMAL分别改为spi_normal_3line_mode_e和SPI_NORMAL(spi_master_config接口和该枚举有关)。
  * (B92)去除spi_pin_config_t结构体,新增gspi_pin_config_t，lspi_pin_config_t，
    将gspi_set_pin函数形参改为gspi_pin_config_t指针类型，将lspi_set_pin函数形参改为gspi_pin_config_t指针类型。  
* **rf**
   * (B91)(B92)枚举成员FLD_RF_R_NOACK_RECNT_EN/FLD_RF_R_CMD_SCHDULE_EN分别改为FLD_RF_R_NOACK_RETRY_CNT_EN/FLD_RF_R_CMD_SCHEDULE_EN。
   * (B91)(B92)寄存器reg_rf_modem_sync_thre_ble改为reg_rf_modem_sync_thres_ble。
   * (B92)删除FPGA验证阶段的频点设置函数rf_set_channel_fpga，改为使用rf_set_chn接口。
   * (B92)针对最新的供电电压配置，更新rf能量表。
   * (B91/B92)删除了rf_power_Level_list表的const关键字。
* **flash**
  * (B92)接口flash_quad_page_pragram_encrypt改为flash_quad_page_program_encrypt。
* **usb**
  * (B91)(B92)寄存器reg_usb_ep8_send_thre改为reg_usb_ep8_send_thres。
* **calibration**
  * (B92)校准值接口user_read_flash_value_calib()改为calibration_func()。
* **pm**
  * (B92)占用PM_ANA_REG_POWER_ON_CLR_BUF0[bit1]（0x3a[1]）标志是否发生过起振异常导致的重启，客户不能使用这个bit。 
* **uart**
  * (B92)uart_get_rx_buff_cnt改为uart_get_rxfifo_rem_cnt。
  * (B92)(B92)删除接口uart_rts_auto_mode/uart_rts_manual_mode。
* **ide**
  * (B91)B91的IDE更改为TelinkIoTStudio，工具链仍然为GCC7。
  

### Bug Fixes

* **gpio**
  * (B92)PF[5:0]和PG[5:0]的数字上拉电阻不可用，这些引脚在接口gpio_set_pullup_res_30k()中被禁止。 [ad91c3e5](http://192.168.48.36/src/driver/telink_b91m_driver_src/merge_requests/566/diffs?commit_id=ad91c3e57719db168a95846f8022a0c7c9bf8ec5)

* **rf**
  * (B92)修复了rssi值与实际接收能量不符的问题。[435541cf](http://192.168.48.36/src/driver/telink_b91m_driver_src/merge_requests/444/diffs?commit_id=435541cf3d9776581b57883579b54cb4421f08fd)
  * (B91/B92)修复了rf_set_chn部分频点ctrim设置错误的问题，减小不同频点上tx能量差异。[8ead48a7](http://192.168.48.36/src/driver/telink_b91m_driver_src/merge_requests/541/diffs?commit_id=8ead48a7e05d90ee0cd64d68ef475f8275974b9f)
  * (B91/B92)解决rf_set_tx_settle_time接口设置settle时间时高4bit未生效导致settle time设置错误的问题，将settle时间的设置范围扩展到0xfff。[8ead48a7](http://192.168.48.36/src/driver/telink_b91m_driver_src/merge_requests/541/diffs?commit_id=8ead48a7e05d90ee0cd64d68ef475f8275974b9f)
* **emi**
  * (B91/B92)修复了rx包长度和dma fifo大小之间不一致导致FIFO数据溢出覆盖其他RAM信息。[95a0c226](http://192.168.48.36/src/driver/telink_b91m_driver_src/merge_requests/450/diffs?commit_id=95a0c22606473f33a8f5032cf0eeb7660bd94a16)
* **audio**
  * (B92)解决了DMIC在采样率为44.1k,48k时通道数据相互调换的问题。[2928be92](http://192.168.48.36/src/driver/telink_b91m_driver_src/merge_requests/453/diffs?commit_id=2928be92d4f9dbd3b52f2633ae13356b3e100c43) 
* **spi**
  * (B92)修复slave端 gpsi rx dma接收第一笔数据会出错问题。[03fdb77b](http://192.168.48.36/src/driver/telink_b91m_driver_src/merge_requests/452/diffs?commit_id=03fdb77bfb4840573ff6bad797a1deea9ebb32ed)
* **mspi**
  * (B92)mspi相关的函数增加ramcode修饰，避免了mspi相关函数可能编译到text段，造成死机的风险。[7ce63726](http://192.168.48.36/src/driver/telink_b91m_driver_src/merge_requests/465/diffs?commit_id=7ce63726d14f7135e8efd37d7b183a6e52577bd2)
* **sys**
  * (B92)sys_reboot:将sys_reboot的定义改到sys.c中，并新增sys_reboot_ram接口，将sys_reboot的处理放在ram中，避免调用sys_reboot时引发对flash错误操作。[7ce63726](http://192.168.48.36/src/driver/telink_b91m_driver_src/merge_requests/465/diffs?commit_id=7ce63726d14f7135e8efd37d7b183a6e52577bd2)
  * (B91/B92)重新写了sys_reboot接口，避免了因预取址和中断等因素引发死机的隐患。[1cd3cc0e](http://192.168.48.36/src/driver/telink_b91m_driver_src/merge_requests/479/diffs?commit_id=1cd3cc0e1c14780aad3f729d8db6917ba44c66c9)
  * (B92)sram供电要求1.2V左右。调整不同状态下给ram供电的电压都可以保持在1.2V。[1cd3cc0e](http://192.168.48.36/src/driver/telink_b91m_driver_src/merge_requests/479/diffs?commit_id=1cd3cc0e1c14780aad3f729d8db6917ba44c66c9)
  * (B92)解决晶振稳定标志位失灵导致死机的问题。 起振异常则重启，占用PM_ANA_REG_POWER_ON_CLR_BUF0[bit1]查询是否发生过起振异常导致的重启。（已在驱动层实现，不需要应用层特殊调用。）[406f518d](http://192.168.48.36/src/driver/telink_b91m_driver_src/merge_requests/540/diffs?commit_id=406f518d43f134209d4dc9c81bc40523208a93ad)
  * (B92)初始化时配置cclk/hclk/pclk/mspiclk的时钟源都来源于24M rc。[2b85e90e](http://192.168.48.36/src/driver/telink_b91m_driver_src/merge_requests/551/diffs?commit_id=2b85e90e6f30097f0f400212951fca7712666936)
  * (B92)避免了digital core电压大于最高工作电压1.32V的可能带来的风险。digital core电压配置为1.2V。[cb5916dc](http://192.168.48.36/src/driver/telink_b91m_driver_src/merge_requests/573/diffs?commit_id=cb5916dc4b5503b19cc92744e4a7471c3b88205c)
* **clock**
  * (B91)clock_init:新增clock_init_ram接口，将clock_init的处理放在ram中，避免配置clock时不当操作引起死机隐患。[7ce63726](http://192.168.48.36/src/driver/telink_b91m_driver_src/merge_requests/465/diffs?commit_id=7ce63726d14f7135e8efd37d7b183a6e52577bd2)
* **uart**
  * (B92)uart_receive_dma接口中有直接使用UART0,而不是参数uart_num，将其修改。[f7cebf4e](http://192.168.48.36/src/driver/telink_b91m_driver_src/merge_requests/516/diffs?commit_id=f7cebf4e152151ca130c1c3861eff0f856985b10)
* **gpio**
  * (B92)修改gpio_set_src_irq()配置，解决了group src level中断无法正常工作的问题。[e9581b0a](http://192.168.48.36/src/driver/telink_b91m_driver_src/merge_requests/511/commits?commit_id=e9581b0a098b9230269a5389a98a82c9e4a90bb3)
* **dma** 
  * (B92/B91)修复audio_rx/tx_dma_chain_init接口中传入的buff参数分配在IRAM时(DRAM不受影响)，再调用audio_get_rx_dma_wptr/audio_get_tx_dma_rptr接口会出错问题，原因：先调用convert_ram_addr_cpu2bus，其返回值写到dma相关寄存器后，再读出寄存器的值输入到convert_ram_addr_bus2cpu 其计算结果会出错。[cfcb91ef](http://192.168.48.36/src/driver/telink_b91m_driver_src/merge_requests/543/diffs?commit_id=cfcb91ef95289d327a78c014a401705c53d7118a)
* **pwm**
  * (B91)在pwm_clr_irq_status接口中，修复FLD_PWM0_IR_FIFO_IRQ中断标志位不能正确清除问题。[e0dfb211](http://192.168.48.36/src/driver/telink_b91m_driver_src/merge_requests/532/diffs?commit_id=e0dfb21137f0e1396618ac246e10626d941093e0)
* **pm**
  * (B92)解决了因sws未接模拟上下拉而漏电导致的suspend睡眠电流大的问题。[eb6c0261](http://192.168.48.36/src/driver/telink_b91m_driver_src/merge_requests/466/diffs?commit_id=eb6c0261101c513f228a1a9dbf02dc9832f088d6)
* **I2C_Demo**
  * (B92)代码逻辑错误导致收包数据异常。[106435f3](http://192.168.48.36/src/driver/telink_b91m_driver_src/merge_requests/443/diffs?commit_id=106435f316711e1b27c9b7a095969ec2e1fca7aa)
* **USB_Demo**
  * (B92)修复了在不断电的情况下(Vbat供电),重复拔插Vbus，vbus 8s复位定时器无法再次清除的问题。[b36d04e1](http://192.168.48.36/src/driver/telink_b91m_driver_src/merge_requests/451/diffs?commit_id=b36d04e10302fd0eb09c30d9a5e41fb1bc533096)
  * (B91/B92)修复端点中断mask默认全部打开，可能导致进入非预期端点中断问题。[2a22e068](http://192.168.48.36/src/driver/telink_b91m_driver_src/merge_requests/454/diffs?commit_id=2a22e0684b524a11c22db86fe932164d56642ed6)
  * (B91)修复了 mic_demo MIC_DMA_CHN 未定义和 spk_demo #ifdef #endif未成对使用的错误。[a2efbbb2](http://192.168.48.36/src/driver/telink_b91m_driver_src/merge_requests/498/diffs?commit_id=a2efbbb2853f979e84fc2dc0464ff0b55c827cbb)
  

### Features

* **usb**
  * (B92)USB 手动配置模式下，支持获取配置命令和设置配置命令。  
* **flash**
  * (B92)增加2M flash(GD25LQ16E)。
* **pm**
  * (B92)增加清除pm中断标志位的函数接口。
* **audio**
  * (B91)在audio_set_i2s_clock()函数接口里增加audio adc/dac 24k采样率配置选项。
* **rf**
  * (B91/B92)添加了提升emi/bqb程序在24M倍数频点的sensitivity性能的相关接口rf_ldot_ldo_rxtxlf_bypass_en()、rf_ldot_ldo_rxtxlf_bypass_dis()、rf_set_rxpara()。
  * (B92)更新能量表以支持GPIO 输出电压为1.8V时的能量设置。
* **aes**
  * (B91/B92)新增防止冲突的AES加密和解密的接口-aes_encrypt_bt_en和aes_decrypt_bt_en。
* **clock**
  * (B91/B92)新增把CCLK为96M的配置。
  * (B92)PLL为192M时，增加调节CCLK/HCLK/PCLK频率的函数接口cclk_hclk_pclk_config() 和对应的枚举pll_div_cclk_hclk_pclk_e。  
* **flash**
  * (B91/B92)在每款flash中，添加接口flash_get_lock_block_midxxx，返回flash当前锁的是哪一块区域（同时在midxxx_lock_block_e中，添加注释说明flash写保护时只能选择。midxxx_lock_block_e的枚举值，使得flash_get_lock_block_midxxx接口返回的值都在midxxx_lock_block_e中）。
  * (B91/B92)flash.h中，新增枚举flash_mid_e，可以使用枚举值代替直接使用flash_mid，增加可读性。
  * (B91/B92)flash.h中，在flash_capacity_e中添加FLASH_SIZE_16M。
* **efuse**
  * (B92)新增从efuse读取adc校准值接口efuse_calib_adc_vref()。
* **adc**
  * (B92)新增adc校准值配置接口adc_set_gpio_calib_vref()、 adc_set_gpio_two_point_calib_offset()、adc_set_vbat_calib_vref()和adc_set_vbat_two_point_calib_offset()，并优化了部分注释。
* **all**
  * (B92)为所有demo添加calibration_func()功能。
* **i2c**
  * (B91) 新增slave/matser stretch功能相关接口 ：i2c_slave_stretch_en/i2c_slave_stretch_dis/i2c_slave_manual_stretch_en/i2c_slave_manual_stretch_clr/i2c_master_stretch_en/i2c_master_stretch_dis。
  * (B91) 在i2c_master_init中，默认将master stretch功能打开(i2c_master_stretch_en),无论对方slave是否stretch,master端都不会异常。
  * (B92) 在i2c_master_init中，默认将master stretch功能打开(i2c_master_stretch_en),无论对方slave是否stretch,master端都不会异常。
  * (B92) 在i2c_master_init中，默认打开i2c_master_detect_nack_en功能,无论对方slave是否发送nack,master端都不会异常。
* **PM_Demo**
  * (B92)添加 PM demo. 
* **USB_Demo**
  * (B91)基于 tinyusb 添加 U 盘 Demo。
* **EMI_BQB_Demo**
  * (B91)增加了EMI_Demo初始配置功能和PA配置方法。
  * (B92)增加了EMI_BQB_Demo,并支持RF电流测试功能。
* **Flash_Demo**
  * (B91/B92)给应用层维护两个接口flash_lock/flash_unlock（判断flash是否上锁还是解锁），将所有的flash列举出来，避免由于后续新增flash，应用层有遗漏。
* **TRAP_Demo**
  * (B92)添加 mtime 中断, 软件中断并在 TRAP_Demo 中添加相应的示例.

### Refactoring

* **aes**
  * (B91/B92)更新函数接口aes_set_em_base_addr的注释。
* **dma**
  * (B91/B92)添加使用dma的外设注释: dma的src和dst地址必须四字节对齐，否则程序进入异常。
  * (B91/B92)补充外设与dma结合使用的相关注释。
* **rf**
  * (B91/B92)根据应用需求将rf_set_chn函数设置为_attribute_ram_code_sec_noinline_类型。
  * (B91/B92)简化rf_update_internal_cap内部分实现。
  * (B91/B92)将rf_set_power_level_index定义在ram中。 
* **all**
  * (B91/B92)规范化与私有模式相关内容的命名。 
  * (B92)清除DriverSDK所有警告报错。
* **adc**
* (B91/B92)对g_adc_vref添加注释，并修正部分接口注释。
* **gpio**
* (B92)外设某些功能pin不用且未设为GPIO_NONE_PIN,导致pin误设置到PA0，修改GPIO_NONE_PIN为0优化该问题。
* **audio**
* (B92)audio_set_codec_stream0/1_path接口内部优化。
* **project**
  * (B92)工程配置下添加如下编译选项：-Wall,-Wextra,-Wshadow,-Werror，并清除警告报错。
  * (B92)解决特殊文件不能跳转问题。
  * (B91/B92)去除编译选项：-Wno-gnu-zero-variadic-macro-arguments(gcc7和gcc10已经不支持该选项)。
* **uart** 
  * (B92)uart_get_dma_rev_data_len:保证功能不变的情况下，简化功能逻辑。
* **adc**
  * (B91)调整adc_set_gpio_two_point_calib_offset()的位置，并优化了部分注释。
* **gpio/i2c/spi/stimer/pm**
  * (B91/B92)将gpio.h,i2c.h,spi.h,stimer.h添加stdbool头文件，pm_get_wakeup_src函数返回修改为pm_wakeup_status_e。
* **keyscan/stimer/watchdog/aes/irlearn/adc/lpc/ctb/charger/pwm/timer/gpio**
  * (B91/B92)纠正拼写错误。
* **pm**
  * (B92)添加注释说明不能用作pm唤醒源的一些gpio引脚
* **PLIC_Demo**
  * (B91/B92)将 PLIC_Demo 合并到 TRAP_Demo 并删除 PLIC_Demo。

### Performance Improvements

* **rf**
  * (B92)优化 BLE2M，Private 2M，zigbee，hybee模式的sensitivity。
* **sys**
  * (B92)调节芯片内部各个ldo，使其设置成正常电压档位。
  * (B92)调节VDDO3输出1.8V或者3.3V。
* **pm**
  * (B91)修改pm_sleep_wakeup函数，使长睡眠时suspend睡眠时间更准确。
* **adc**
  * (B91) 加严了ADC校准值的非法值判断。
* **EMI_BQB_Demo**
  * (B91/B92)使用rf_ldot_ldo_rxtxlf_bypass_en()、rf_ldot_ldo_rxtxlf_bypass_dis()、rf_set_rxpara()
接口，提升emi/bqb程序在24M倍数频点的sensitivity性能。


<hr style="border-bottom:2.5px solid rgb(146, 240, 161)">


## V2.1.0-Beta

### Version

* SDK version: telink_b91m_driver_sdk V2.1.0-Beta.
* This version of the SDK supports B91(A0,A1,A2),B92(A1) chips.
* B92(added) supports USB/AUDIO/I2C/s7816//UART/QDEC/PWM/Timer/Stimer/PLIC/GPIO/SPI/PKE/AES/ANALOG/TRNG
* The default configuration of LEDs and KEYs match the following hardware revisions:
*   B91	 	C1T213A20_V1_3_20200727
*   B92	 	C1T266A20_V1_3_20220722

### BREAKING CHANGES

* **flash** 
  * (B91)Modify midxxxx_otp_block_e and midxxxx_lock_otp_e element name to change the unit K to B to avoid misleading OTP size.
* **watchdog**
  * (B91)The wd_clear interface only clears the watchdog count, but not the reset state of the watchdog.

### Bug Fixes

* **EMI_Demo**
  * (B91)Fixed the problem that the carrier signal cannot be amplified by the PA module under certain PA pin combinations.[e01a08e2](http://192.168.48.36/src/driver/telink_b91m_driver_src/commit/e01a08e240e16476a87cb7bdf618a77c18a331a5)
* **PM**
  * (B91)Solved the probabilistic crash problem caused by abnormal flash instruction fetch after suspend wakeup (must appear under certain software logic).[c3e12945](http://192.168.48.36/src/driver/telink_b91m_driver_src/commit/c3e12945fa99b7a73d98a0af96360e330c9b9ac9)
  * (B91)Solved the problem that the sleep time is incorrect due to a small probability that the total interruption time of the shutdown is not timely.[e66aca1a](http://192.168.48.36/src/driver/telink_b91m_driver_src/commit/e66aca1a1c3fa87d80bba50828437bc767c16088)
* **pwm**
  * (B91)pwm_clr_irq_status: when the pwm_clr_irq_status function is used to clear some pwm interrupts, other pwm interrupts may be cleared by mistake.[c796ae51](http://192.168.48.36/src/driver/telink_b91m_driver_src/commit/c796ae51712642ba0c17c2846a30b1669d54843d)
  * (B91)pwm_clr_irq_mask: the function of this interface is to clear mask. this interface does not take effect in the old version and has been modified in the new version.[c796ae51](http://192.168.48.36/src/driver/telink_b91m_driver_src/commit/c796ae51712642ba0c17c2846a30b1669d54843d)
* **dma**
  * (B91)struct dma_chain_config_t variable member added volatile modified: the interface pwm_set_tx_dma_add_list_element/audio_rx_dma_chain_init/audio_tx_dma_chain_init chain configuration is affected by compilation optimization, may cause the pwm/audio module can not work properly(In the use of the chain function scenario).[c796ae51](http://192.168.48.36/src/driver/telink_b91m_driver_src/commit/c796ae51712642ba0c17c2846a30b1669d54843d)
* **swire**
  * (B91)swire_master_read:fixed an issue with older versions of the swire_master_read interface that could get stuck when the slave did not respond.[a44b6ca6](http://192.168.48.36/src/driver/telink_b91m_driver_src/commit/a44b6ca6429a58d863099af8199720079a014c88)
* **uart**
  * (B91)uart_reset：in no_dma, the old version needs to call uart_clr_tx_done before setting the uart interrupt to clear the tx_done interrupt state, otherwise the call of uart_reset will cause tx_done interrupt by mistake, the new version changes the interface of uart_reset. there is no need to call uart_clr_tx_done in no_dma.[cd80a1b7](http://192.168.48.36/src/driver/telink_b91m_driver_src/commit/cd80a1b79975bcbe1ff91660f1c230a7fa1544d9)
* **lpc**
  * (B91)solves the problem that the LPC POWER DOWN configuration is incorrect.[74c85d0d](http://192.168.48.36/src/driver/telink_b91m_driver_src/commit/74c85d0d8a9f95a6152fd9910d086402ed5d5693)
* **rf**
  * (B91)Turn on the continue mode to reduce the probability of missynchronization and solve the packet loss problem caused by missynchronization[6108aba3](http://192.168.48.36/src/driver/telink_b91m_driver_src/commit/6108aba3868a18322396e74c25106bb2d631d121)
  * (B91)Fix per floor issue.[6108aba3](http://192.168.48.36/src/driver/telink_b91m_driver_src/commit/6108aba3868a18322396e74c25106bb2d631d121)
  
### Features

* **PLIC_Demo**
  * (B91)add WFI(Wait-For-Interrupt Mode) demo
* **GPIO_Demo**
  * (B91)Added the function of triggering interrupts by using the buttons that come with the development board.
* **vendor/common**
  * (B91)add 4M Flash Calibration.
* **flash**
  * (B91)add 2M flash(P25Q16SU)/4M flash(P25Q32SH).
  * (B91)delete 4M flash (P25Q32SH) and add 4M wide press flash (P25Q32SU).
* **RF**
  * (B91)Added PTA related interfaces and demos.
* **swire**
  * (B91)swire_read_set_timeout_tick/swire_read_get_timeout_tick: the default value of 300,000 stimer ticks is set for abnormal read timeout in swire_master_read. can be configured according to the read timing.
* **dp_through_swire**   
  * (B91)Add "DP_THROUGH_SWIRE_DIS" setting to enable and disable "dp_through_swire" function in startup file. 
* **watchdog**
  * (B91)Add the interface for the watchdog to clear the reset state - wd_clear_status.
  * (B91)Add the interface for reading the reset status of the watchdog -wd_get_status.
* **version** 
  * (B91)Add version section for SDK name and version info.
* **uart**
  * (B91)uart_set_pin: the original version set the uart tx/rx function, will be set at the same time, but the application will have a separate use of uart rx or tx function, add enum can choose to separate rx or tx function.
* **uart(demo)**
  * (B91)Add uart and pm combined with the sample code.
* **emi**
  * (B91)Added relevant processing for frequency offset calibration of 4M flash.
* **bqb**
  * (B91)use the uart_send_byte() function replaces bqb_uart_send_Byte() function.
  * (B91)Added relevant processing for frequency offset calibration of 4M flash.
 * **DUT_Demo** 
  * (B91)Users can develop fixture test programs according to their own needs. 
  
### Refactoring

* **plic**
  * (B91)adjusted the location of the plic_isr.c file
* **ALL_Demo**
  * (B91)All demos use the led and key of the telink development board by default, and delete the useless definitions of SW1 and SW2.
* **BQB_Demo**
  * (B91)Modified the writing method of obtaining the size of the flash capacity, which is consistent with the description of the flash capacity in flash.h.
* **EMI_Demo**
  * (B91)Adjusted the interface read_calibration_flash for reading calibration parameters.
* **Vendor/common/calibration**
  * (B91)Adjust user_calib_adc_vref and user_calib_freq_offset to be externally callable interfaces.
* **Swire**
  * (B91)Removed Swire_Demo and Swire.h files.

### Performance Improvements

* N/A

### Note

* (B92) There is a problem with PB0,A1 does not work  
* (B92) watchdoge for 32K is turned on by default and can be turned off by calling wd_32k_stop()  
* (B92) Vbus power supply (not Vbat), there will be muc reset after 8s, USB demo call charger_clear_vbus_detect_status() can be turned off 

<hr style="border-bottom:2.5px solid rgb(146, 240, 161)">


### 版本

* SDK版本: telink_b91m_driver_sdk V2.1.0-Beta。
* 此版本SDK支持 B91(A0,A1,A2),B92(A1) 芯片。
* B92(新增) 支持 USB/AUDIO/I2C/s7816//UART/QDEC/PWM/Timer/Stimer/PLIC/GPIO/SPI/PKE/AES/ANALOG/TRNG
* LED和KEY的默认配置匹配以下硬件版本:
*   B91	 	C1T213A20_V1_3_20200727
*   B92	 	C1T266A20_V1_3_20220722

### BREAKING CHANGES

* **flash**
  * (B91)修改modify midxxxx_otp_block_e 和 midxxxx_lock_otp_e元素名,将单位K改为B,避免误导OTP的大小。
* **watchdog**
  * (B91)wd_clear接口只清看门狗计数，不清看门狗的复位状态。

### Bug Fixes

* **EMI_Demo**
  * (B91)修复了某些PA pin脚组合下，Carrier信号不能被PA模块放大的问题。[e01a08e2](http://192.168.48.36/src/driver/telink_b91m_driver_src/commit/e01a08e240e16476a87cb7bdf618a77c18a331a5)
* **PM**
  * (B91)解决了suspend唤醒后，flash取指异常导致的概率性的死机问题（某种软件逻辑下必现）。[c3e12945](http://192.168.48.36/src/driver/telink_b91m_driver_src/commit/c3e12945fa99b7a73d98a0af96360e330c9b9ac9)
  * (B91)解决了因关总中断时间不及时，小概率地导致的睡眠时间不对的问题。[e66aca1a](http://192.168.48.36/src/driver/telink_b91m_driver_src/commit/e66aca1a1c3fa87d80bba50828437bc767c16088)
* **pwm**
  * (B91)pwm_clr_irq_status:在使用函数pwm_clr_irq_status清除pwm某个中断状态时，有可能会误清除其他pwm中断。[c796ae51](http://192.168.48.36/src/driver/telink_b91m_driver_src/commit/c796ae51712642ba0c17c2846a30b1669d54843d)
  * (B91)pwm_clr_irq_mask: 该接口功能是清mask，旧版本调用该接口不能生效，新版本已修正。[c796ae51](http://192.168.48.36/src/driver/telink_b91m_driver_src/commit/c796ae51712642ba0c17c2846a30b1669d54843d)
* **dma**
  * (B91)结构体dma_chain_config_t变量成员添加volatile修改：由于编译优化影响接口pwm_set_tx_dma_add_list_element/audio_rx_dma_chain_init/audio_tx_dma_chain_init链表的配置，可能导致pwm/audio模块不能正常工作（在使用了链表功能的场景下）。[c796ae51](http://192.168.48.36/src/driver/telink_b91m_driver_src/commit/c796ae51712642ba0c17c2846a30b1669d54843d)
* **swire**
  * (B91)swire_master_read接口：修复了旧版本，当slave无响应时，可能会卡死在swire_master_read接口的问题。[a44b6ca6](http://192.168.48.36/src/driver/telink_b91m_driver_src/commit/a44b6ca6429a58d863099af8199720079a014c88)
* **uart**
  * (B91)uart_reset接口：no_dma下，旧版本需要在设置uart中断之前调用uart_clr_tx_done清除tx_done中断状态，否则调用uart_reset导致误触发tx_done中断，新版本对uart_reset接口进行修改，无需在no_dma下调用uart_clr_tx_done.[cd80a1b7](http://192.168.48.36/src/driver/telink_b91m_driver_src/commit/cd80a1b79975bcbe1ff91660f1c230a7fa1544d9)
* **lpc** 
  * (B91)解决了LPC POWER DOWN配置错误的问题。[74c85d0d](http://192.168.48.36/src/driver/telink_b91m_driver_src/commit/74c85d0d8a9f95a6152fd9910d086402ed5d5693)
* **rf**
  * (B91)使能continue mode降低误同步概率，解决误同步造成的丢包问题。[6108aba3](http://192.168.48.36/src/driver/telink_b91m_driver_src/commit/6108aba3868a18322396e74c25106bb2d631d121)
  * (B91)修复per floor问题。[6108aba3](http://192.168.48.36/src/driver/telink_b91m_driver_src/commit/6108aba3868a18322396e74c25106bb2d631d121)
  
### Features

* **PLIC_Demo**
  * (B91)添加了WFI(Wait-For-Interrupt Mode) demo
* **GPIO_Demo**
  * (B91)新增使用开发板自带的按键来触发中断的功能。 
* **vendor/common**
  * (B91)添加4M flash calibration相关处理。
* **flash**
  * (B91)增加2M flash(P25Q16SU)/4M flash(P25Q32SH)。
  * (B91)删掉了4M flash(P25Q32SH)，添加了4M 宽压flash(P25Q32SU)。
* **RF**
  * (B91)增加PTA相关接口及demo。
* **swire**
  * (B91)swire_read_set_timeout_tick/swire_read_get_timeout_tick:在swire_master_read中的读异常超时时间默认设置300000个stimer tick值，改为可以根据读时序进行配置。
* **dp_through_swire**  
  * (B91)启动文件中新增"DP_THROUGH_SWIRE_DIS" 配置用于打开或关闭"dp_through_swire"功能。 
* **watchdog**
  * (B91)增加看门狗清复位状态的接口-wd_clear_status。
  * (B91)增加读看门狗复位状态的接口-wd_get_status。
* **version** 
  * (B91)新增version段用于存放SDK name和version信息。
* **uart**
  * (B91)uart_set_pin:原有的版本设置uart tx/rx功能时，会同时进行设置，但应用中会有单独使用uart的rx或者tx功能，新增枚举可以选择单独设置rx或者tx功能。
* **uart(demo)**
  * (B91)添加uart和pm结合示例代码。
* **emi**
  * (B91)增加了对4M flash 频偏校准的相关处理。
* **bqb**
  * (B91)使用uart_send_byte()函数代替了bqb_uart_send_byte()函数。
  * (B91)增加了对4M flash 频偏校准的相关处理。
* **DUT_Demo** 
  * (B91)用户可根据自身需要开发夹具测试程序。
  
### Refactoring

* **plic**
  * (B91)调整了plic_isr.c文件的位置。
* **ALL_Demo**
  * (B91)所有的demo默认使用telink开发板的led和key，同时删掉了无用的SW1、SW2的定义。
* **BQB_Demo**
  * (B91)修改了获取flash容量大小的写法，与flash.h中关于flash容量的描述保持一致。
* **EMI_Demo**
  * (B91)调整了读校准参数的接口read_calibration_flash。
* **Vendor/common/calibration**
  * (B91)调整user_calib_adc_vref和user_calib_freq_offset为可被外部调用的接口。
* **Swire**
  * (B91)删除了Swire_Demo和Swire.h文件。

### Performance Improvements

* N/A

### Note

* (B92) PB0有问题,A1不能用
* (B92) 32K的watchdoge默认打开，调用wd_32k_stop()可以将其关掉
* (B92) Vbus供电(不是Vbat),8s后会有muc reset ，USB demo中调用charger_clear_vbus_detect_status()可以关掉

<hr style="border-bottom:2.5px solid rgb(146, 240, 161)">


## V2.0.0

### Version

* SDK version: telink_b91m_driver_sdk V2.0.0.
* This version of the SDK supports B91 A0, A1 and A2 chips.

### BREAKING CHANGES

* **project**
    * Modified the folder structure, using virtual folders in the project and linking to local folders through relative paths.
* **pke**
    * B91 adds the cases where the curves are 160, 192 and 224.
    * pke_get_irq_status, pke_clr_irq_status, pke_set_irq_mask, pke_clr_irq_mask, pke_set_microcode, pke_set_exe_cfg, pke_opr_start, pke_check_rt_code, pke_set_operand_width_pre, pke_set_operand_width_pre_mont, pke_calc_pre_width and pke_mont_calc_pre
* **swire**
    * A new parameter is added to the swire_sync interface and swire_set_swm_en interface to set swm gpio.
* **uart**
    * The uart_set_dma_rx_timeout interface is changed to uart_set_rx_timeout.
* **dma**
    * dma_mask is turned on by default, and it is cleared in sys_init, and there is no need to clear it again in the upper application.
* **i2c**
    * Before the i2c_master_write_read function, there is no stop between sending data and receiving data by default. After modification, you can choose whether there is a stop by using the interface i2c_master_send_stop.
* **pwm**
    * Before pwm_start can only open one at the same time by default, redefine the enumeration type, after modification, you can open multiple at the same time.
* **flash**
    * In order to be compatible with different Flash models, the Flash driver structure has been adjusted, and some Flash interface calling methods have been modified.
    * In order to be compatible with different types of Flash, flash_read_status and flash_write_status are not provided for external use. If you need to use the corresponding interface, you need to select the interface under the flash directory according to the mid (for example: flash_write_status_midxxxxxx).
    * The first parameter flash_mid of the interface int flash_read_mid_uid_with_check( unsigned int *flash_mid ,unsigned char *flash_uid) requires 4 bytes of space, and the second parameter requires 16 bytes of space.
    * Interface flash_lock, flash_unlock, flash_read_otp, flash_write_otp, flash_erase_otp, flash_lock_otp are replaced by flash_lock_midxxxxxx, flash_unlock_midxxxxxx, flash_read_otp_midxxxxxx, flash_write_otp_midxxxxxx, flash_miderase_otxxxxxx, flash_miderase_otxxx, xxx
    * Delete unused function interfaces: flash_erase_page, flash_erase_32kblock, flash_erase_64kblock, flash_erase_chip, flash_deep_powerdown, flash_release_deep_powerdown.
* **s7816**
    * Modify the meaning of the s7816_set_clk parameter, and change the parameter passed in to represent the frequency division number.
* **rf**
    * The previous function interface rf_set_tx_settle_time, rf_set_rx_settle_time set the time value to the parameter + 1, now the set value is consistent with the parameter.
* **pm**
    * pm_set_suspend_power() add a argument to select power down or power on usb/npe/baseband module before suspend.if power down,th sleep current will decrease,but need to re-init the module after wake up.three module all power down default.

### Bug Fixes

* **freertos**
    * Fixed the problem that the system would crash after being interrupted by a task while reading and writing the flash. Because const variables also access flash.
* **swire**
    * Modify the driving time of the swire_sync function interface.
    * Modify the swire_set_slave_id interface to prevent writing 0 first and then assigning the value when configuring the register. When the configuration is 0, abnormal signals may be generated and some risks may be generated.
* **flash**
    * Solve the problem that the unlock interface of flash will reset the four-wire configuration.
* **aes**
    * Modify the pointer of the aes data address to the offset address based on I-RAM minus the offset address of embase to I-RAM.
* **clock**
    * Modify the value of the corresponding position in the sys_pll_clk_e enumeration to the correct value of 60.
* **audio**
    * In mono mode, fixed the problem that the analog input (amic or line-in) does not work when switching channels.
    * When the channel is switched, the adc channel (adc1, adc2) of the corresponding codec also needs to be switched.
* **rf**
    * Fix the definition error of FLD_RF_IRQ_ALL in the enumeration of rf interrupt in B91.
    * Fixed the problem that the function rf_set_access_code_len only takes effect when the access code length is set to 4.
    * Fixed the function rf_set_rffe_pin that may cause the set pin to be in an unstable state during the setting process.
* **uart**
    * Fixed the problem caused by shared variables when UART0 and UART1 are working at the same time.
    * Modify the uart_set_pin interface and adjust the configuration sequence of gpio multiplexing function to avoid the generation of a signal from high to low to high when the uart pin multiplexing function is configured by the old version of the code.
    * Fixed the problem of incorrect calculation of dma receiving length in uart dma mode.
    * Modify the uart_reset interface to solve the problem that when uart is reset, the tx_done mask has been opened, and the tx_done interrupt will continue to enter.
    * Modify the uart_set_rx_timeout/uart_init interface to prevent writing 0 first and then assigning values ​​when configuring certain registers. When the configuration is 0, abnormal signals may be generated and some risks may be generated.
* **i2c**
    * Modify the internal implementation mechanism of the i2c_slave_read function, add the fifo software pointer, record the current fifo position, and avoid the slave always fetching data from the first address of the fifo when reading data, which may cause the software pointer and the hardware pointer to not correspond.
    * Modify the i2c_set_pin interface and adjust the configuration sequence of gpio multiplexing functions to avoid the generation of a signal from high to low to high when the i2c pin multiplexing function is configured by the old version of the code.
    * Modify the implementation method of the i2c_rx_irq_trig_cnt interface to avoid writing 0 first and then assigning the value when configuring the register, because when the configuration is 0, an error interrupt will be generated.
* **gpio**
    * Solved the problem of false interruption caused by incorrect configuration sequence when the rising edge triggers the interrupt when combined with the pull-up resistor.
    * After configuring IO as peripheral functions (such as I2C / SPI, etc.),then call the gpio_shutdown function to make the GPIO enter the shutdown mode. At this time, the function does not work.
* **spi**
    * Fixed the problem that the mixed use of SPI DMA mode and non-DMA mode will cause abnormal communication in DMA mode.
    * Fixed the problem that the SPI slave was powered on first and the CSN was floating, which caused an end interrupt.
    * The operation of clearing 0 and then writing the corresponding value is modified to a write to avoid possible abnormalities caused by the state when the register is written to 0.
    * The hspi_set_pin_mux() interface and pspi_set_pin_mux() interface modified part of the code sequence.
* **S7816**
    * Modify the s7816_set_clk interface to prevent writing 0 first and then assigning the value when configuring the register. When the configuration is 0, abnormal signals may be generated and some risks may be generated.
* **charger**
    * Fixed the problem that the charger repeatedly switched on and off during the USB charging process.
* **stimer**
    * Solve the problem that the stimer_disable interface cannot stop the stimer because the auto/manual mode is not distinguished.
* **dma**
    * The operation of clearing the reg_dma_ctrl register to 0 and then writing the target value is modified as a write to avoid abnormalities caused by writing 0 to the register.
    * Fixed the problem that any dma channel cannot be written to the dma register again when the transmission has not finished. Need to disable dma before performing the corresponding operation.
* **pm**
    * Fixed the problem that the program freezes during the suspend wakeup process when cclk is configured to 96M and pclk is configured to cclk's quarter-frequency 24M.
    * In DCDC_1P4_DCDC_1P8 mode, set the correct voltage level to avoid the risk of low voltage level of 2.8V which was used in the previous version. ( the voltage of some chips may be lower than the 2.5V required by the codec, which will affect the performance of the codec)
* **pwm(demo)**
    * In app_pwm_ir_dma.c, in pwm_ir_fifo_dma mode, the array subscript set 0 for each interrupt,otherwise, multiple interrupts will result in buff out-of-bounds access.
 * **plic** 
    * fix the problem that the plic_set_threshold() may not take effect.  
* **spi**
	* Solved the problem that sometimes compilation optimization will store the table of switch case statement in rodata segment. Encoding options add -fno-jump-tables.
* **sws_usb**
	* fix the issue that dp dm use as GPIO may cause unexpected error.firmware will disable swire through usb function,and you need to call usb_set_pin_en() function to enable it.

### Refactoring

* **spi**
    * Move spi_tx_dma_en and spi_rx_dma_en operations from demo to interface.
* **plic**
    * To facilitate application, move all interrupt handling functions to vendor/common/plic_isr.c.
* **gpio**
    * Turn on the jtag function in the gpio_init() function.
* **project**
    * The suffix of Debug file is uniformly revised to elf.
* **rf**
    * Normalizes the private schema keyword name.
    * Modify the internal implementation of rf_set_rx_timeout.
    * Separate the rx settle time setting from the rf_start_srx function and use it as the rf_set_rx_settle_time function.
* **audio**
    * Refactored audio_set_i2s_clock() function.
* **dma**
    * Refactored the function of setting DMA source address and destination address to make it compatible with addresses other than sram.
* **emi**
    * Delete mode3 in continue mode, which is debug mode. Does not affect the use, the function has been verified to be normal.
* **pke**
    * Adjust the structure of pke_demo (package the algorithm calculation part into a library), and adjust the structure of the files to be sealed into a library.
* **adc**
    * Modify the interface adc_set_ref_voltage and delete the repeated operation of setting the default reference voltage.
* **plic**
    * adjust core_interrupt_disable and core_restore_interrupt interfaces.
* **pwm(demo)**
    * In app_pwm_ir_dma.c, in pwm_chain_dma mode, modify the source addresses pointed to by nodes 1 and 2 so that the output waveform starts from buff0 and then buff1 alternately outputs.
* **license**
    * update license of related files from TELINK_BSD to TELINK_APACHE


### Features

* **emi**
    * Add test 0xaa and 0xf0 two data packets in tx burst mode, and control these two modes through bdt.
* **audio**
    * Added 192K sampling rate.
* **rf**
    * Added setting rf tx/rx mi modulation interface: rf_set_tx_modulation_index and rf_set_rx_modulation_index.
* **plic_demo**
    * Add the hardware stack protection (HSP) demo in the PLIC demo.
* **spi**
    * Add interfaces pspi_csn_pin_def_e pspi_change_csn_pin() and hspi_csn_pin_def_e hspi_change_csn_pin() for switching hardware CSN in multi-slave mode.
    * Add interface spi_master_write_read_full_duplex(), which is used for SPI full-duplex communication in MCU data transmission mode.
    * Add interface spi_master_read() for master read-only data.
    * Add macro definition SPI_SLAVE_READY in Demo. 1 means the slave_ready bit is used. The master will first ask if the slave is ready, and the slave will set this bit to 1 during initialization.
    * Add the spi_master_get_slave_ready() interface in the Demo, and the master sends instructions to read the ready status of the slave.
* **flash**
    * Add a four-wire read interface, a two-wire read interface, and a four-wire write interface.
* **adc**
    * Add the interface adc_get_gpio_calib_vref to obtain the reference voltage for single-point calibration.
    * Increase the value ADC_SAMPLE_FREQ_192K of the enumeration variable adc_sample_freq_e to select the ADC sampling rate of 192Khz.
    * Add interface adc_set_gpio_two_point_calib_offset to set the offset of two-point calibration.
* **vendor/common**
    * Added interface user_read_flash_value_calib for chip calibration (including ADC calibration and RF frequency offset calibration).
* **EMI_BQB_Demo**
    * Added the macro CLOSE_INTERNAL_CAP_EN to choose whether to close the internal capacitor.
    * Removed some unnecessary codes: Code for writing frequency offset value in EMI initialization;Temporary PA setting Code.
* **spi(demo)**
    * Added macro definition SPI_SLAVE_READY_TEST to test slave_ready function.

### Performance Improvements
* **IDE**
     * Add the compile command make -j${NUMBER_OF_PROCESSORS} to speed up the compilation.
* **plic**
     * Add the always_inline modification of the function plic_isr to reduce the size of the compiled bin file.
<hr style="border-bottom:2.5px solid rgb(146, 240, 161)">


### 版本

* SDK版本: telink_b91m_driver_sdk V2.0.0。
* 此版本SDK支持B91 A0、A1和A2芯片。

### BREAKING CHANGES

* **project**
    * 修改了文件夹结构，工程中采用虚拟文件夹，并通过相对路径链接到本地文件夹。
* **pke**
    * B91增加了曲线为160，192和224的情况。
    * pke_get_irq_status、pke_clr_irq_status、pke_set_irq_mask、pke_clr_irq_mask、pke_set_microcode、pke_set_exe_cfg、pke_opr_start、pke_check_rt_code、pke_set_operand_width和pke_calc_pre_mont、pke_pre_calc_mont等接口删除。
* **swire**
    * swire_sync接口、swire_set_swm_en接口新添加一个参数，设置swm gpio。
* **uart**
    * uart_set_dma_rx_timeout接口改为uart_set_rx_timeout。
* **dma**
    * dma_mask默认是打开的，在sys_init中统一进行了清除，在上层的应用中不需要再次清除。
* **i2c**
    * i2c_master_write_read函数之前默认发送数据与接收数据中间是没有stop，修改之后，通过使用接口i2c_master_send_stop，可以选择是否有stop。
* **pwm**
    * pwm_start之前默认只能同时打开一个，重新定义枚举类型，修改之后，可以同时打开多个。
* **flash**
    * 为兼容不同的Flash型号，Flash驱动结构做了调整，修改了部分Flash接口调用方式。
    * 为兼容不同型号的Flash,flash_read_status、flash_write_status不提供给外部使用，需要使用对应接口时，需要根据mid去选择flash目录下的接口(例如：flash_write_status_midxxxxxx)。
    * 接口int flash_read_mid_uid_with_check( unsigned int *flash_mid ,unsigned char *flash_uid)的第一个参数flash_mid需要4个字节空间，第二个参数需要16byte空间。
    * 接口flash_lock、flash_unlock、flash_read_otp、flash_write_otp、flash_erase_otp、flash_lock_otp由flash_lock_midxxxxxx、flash_unlock_midxxxxxx、flash_read_otp_midxxxxxx、flash_write_otp_midxxxxxx、flash_erase_otp_midxxxxxx、flash_lock_otp_midxxxxxx替代。
    * 删除不用函数接口：flash_erase_page、flash_erase_32kblock、flash_erase_64kblock、flash_erase_chip、flash_deep_powerdown、flash_release_deep_powerdown。
* **s7816**
    * 修改s7816_set_clk参数含义，改为传入的参数就代表分频数。
* **rf**
    * 之前函数接口rf_set_tx_settle_time，rf_set_rx_settle_time设置时间值为参数 + 1，现在改为设置值与参数保持一致。
* **pm**
    * pm_set_suspend_power()接口添加一个参数用于选择进入suspend前是power down还是power on usb/npe/baseband。关闭模块时睡眠电流会减小，但是唤醒后需要重新初始化对应模块。默认全关。

### Bug Fixes

* **freertos**
    * 修复了在读写flash，被任务中断后会死机的问题。因为const 变量也访问 flash。
* **swire**
    * 修改swire_sync函数接口的驱动时间。
    * 修改swire_set_slave_id接口，防止配置寄存器时先写0，后赋值，由于配置为0时，可能产生异常信号导致一些风险。
* **flash**
    * 解决了flash的解锁接口会把四线配置复位的问题。
* **aes**
    * 修改aes数据地址的指针为基于I-RAM的偏移地址减去embase对I-RAM的偏移地址。
* **clock**
    * 修改sys_pll_clk_e枚举中对应位置的值为正确的60。
* **audio**
    * mono模式下，修复了模拟输入（amic或line-in）切换通道时不起作用问题。
	* 通道切换时，对应的codec的adc通道（adc1,adc2）也需作切换。
* **rf**
    * 修复B91中rf中断的枚举中关于FLD_RF_IRQ_ALL的定义错误。
    * 修复了函数rf_set_access_code_len只有在设置access code长度为4的时候才生效的问题。
    * 修复了函数rf_set_rffe_pin可能会导致设置的pin脚在设置过程中处于不定态。
* **uart**
    * 修复了UART0和UART1同时工作时，由于共享变量造成的问题。
    * 修改uart_set_pin接口，调整gpio复用功能配置顺序,避免旧版本代码写法配置uart pin脚复用功能时，产生一个由高到低再到高的信号。
    * 修复了uart dma模式下，计算dma接收长度错误的问题。
    * 修改uart_reset接口，解决当reset uart时，tx_done mask已经打开，会一直进入tx_done中断的问题。
    * 修改uart_set_rx_timeout/uart_init接口，防止在配置某些寄存器时先写0，后赋值，由于配置为0时，可能产生异常信号导致一些风险。
* **i2c**
    * 修改i2c_slave_read函数的内部实现机制，添加fifo软件指针，记录当前fifo的位置，避免slave读取数据时，总是从fifo的首地址取数据，可能导致软件指针与硬件指针没有对应。
    * 修改i2c_set_pin接口，调整gpio复用功能配置顺序,避免旧版本代码写法配置i2c pin脚复用功能时，产生一个由高到低再到高的信号。
    * 修改i2c_rx_irq_trig_cnt接口的实现方法，避免配置寄存器时先写0，后赋值，由于配置为0时，产生一个误中断。
* **gpio**
    * 解决了当配合上拉电阻，上升沿触发中断，由于配置顺序不对，导致误中断的问题。
    * 将IO配置为外设功能（比如I2c/SPI等）后，再调用gpio_shutdown函数让该GPIO进入shutdown模式，此时该函数不起作用。
* **spi**
    * 修复了SPI DMA模式和非DMA模式混用会造成DMA模式通信异常的问题。
    * 修复了SPI slave先上电，CSN浮空导致产生end中断的问题。
    * 先清0再写相应值的操作，修改为一次写入，避免寄存器写0时状态可能造成异常。
    * hspi_set_pin_mux()接口和pspi_set_pin_mux()接口修改了部分代码顺序。
* **S7816**
    * 修改s7816_set_clk接口，防止配置寄存器时先写0，后赋值，由于配置为0时，可能产生异常信号导致一些风险。
* **charger**
    * 修复了USB在充电过程中，charger反复开关的问题。
* **stimer**
    * 解决因为没有区分auto/manual模式而导致stimer_disable接口不能停止stimer的问题。
* **dma**
    * reg_dma_ctrl寄存器先清0再写目标值的操作，修改成一次写入，避免寄存器写0造成异常。
    * 修复了任一dma通道在传输还未结束时，不能再次写入dma寄存器的问题，需要先disable dma再进行相应的操作。
* **pm**
    * 修复了cclk配置为96M，pclk配置为cclk的四分频24M时，程序在suspend唤醒过程中，程序卡死的问题。
    * DCDC_1P4_DCDC_1P8模式下，设置正确的电压档位，规避之前版本使用电压低档位，可能造成2.8V电压偏低的风险。（某些芯片该电压可能会低于codec要求的2.5V，会影响codec性能）
* **pwm(demo)**
    * 在app_pwm_ir_dma.c中，模式PWM_IR_FIFO_DMA下，每次进中断需将数组下标归0，否则多次中断，导致buff越界访问;
* **plic** 
  * 修复 plic_set_threshold() 可能不生效的问题。
* **spi** 
  * 解决了有时编译优化会将switch case语句的表存放在rodata段的问题。编码选项添加-fno-jump-tables。
* **sws_usb** 
  * 解决当dp dm做GPIO使用时可能会导致异常错误的问题。firmware默认会disable swire through usb功能，当需要使用该功能时需要调用usb_set_pin_en接口。

### Refactoring
* **spi**
    * 将spi_tx_dma_en和spi_rx_dma_en操作从demo中移到了接口中。
* **plic**
    * 为了方便应用，将所有的中断处理函数移动到vendor/common/plic_isr.c。
* **gpio**
    * 在gpio_init()函数中打开jtag功能。
* **project**
    * Debug文件后缀统一修改为elf。
* **rf**
    * 规范化private模式关键字名称。
    * 修改rf_set_rx_timeout内部实现方式。
    * 将rx settle时间设置从rf_start_srx函数中分离出来作为 rf_set_rx_settle_time函数使用。
* **audio**
    * 重构audio_set_i2s_clock()函数。
* **dma**
    * 重构了设置dma源地址、目的地址函数，使其可以兼容除sram以外的其他地址。
* **emi**
    * 删除continue模式下的mode3，该模式为debug模式。不影响使用，已验证功能正常。
* **pke**
    * 将pke_demo结构进行调整（算法运算部分封装成库），对要封成库的文件进行结构调整。
* **adc**
    * 修改接口adc_set_ref_voltage，删除设置默认参考电压的重复操作。
* **plic**
    * 调整core_interrupt_disable和core_restore_interrupt接口。
* **pwm(demo)**
    * 在app_pwm_ir_dma.c中，模式PWM_CHAIN_DMA下，修改节点1和节点2指向的源地址，使得输出的波形从buff0开始再到buff1交替输出;
* **license**
    * 将相关文件的许可证从 TELINK_BSD 更新为 TELINK_APACHE


### Features
* **emi**
    * tx burst模式下添加测试0xaa和0xf0两种数据包，通过bdt控制这两种模式。
* **audio**
    * 增加了192K采样率。
* **rf**
    * 新增设置rf tx/rx mi调制接口：rf_set_tx_modulation_index和rf_set_rx_modulation_index。
* **plic_demo**
    * 在PLIC demo里添加硬件栈保护（HSP）demo。
* **spi**
    * 添加接口pspi_csn_pin_def_e pspi_change_csn_pin()和hspi_csn_pin_def_e hspi_change_csn_pin()，用于多slave模式下切换硬件CSN。
    * 添加接口spi_master_write_read_full_duplex(),用于MCU传输数据方式的SPI全双工通信。
    * 添加接口spi_master_read(),用于master只读数据。
    * 在Demo中添加宏定义SPI_SLAVE_READY。1表示使用slave_ready位，master会先询问slave是否准备好，slave会在初始化时将该位设置为1。
    * 在Demo中添加spi_master_get_slave_ready()接口，master发送指令读取slave的ready状态。
* **flash**
    * 增加四线读接口，增加两线读接口，增加四线写接口。
* **adc**
    * 增加接口adc_get_gpio_calib_vref，获取单点校准的参考电压。
    * 增加枚举变量adc_sample_freq_e的值ADC_SAMPLE_FREQ_192K，用于选择ADC采样率192Khz。
    * 增加接口adc_set_gpio_two_point_calib_offset，设置两点校准的offset。
* **vendor/common**
    * 增加接口user_read_flash_value_calib，用于芯片校准(包括ADC校准和RF频偏校准)。
* **EMI_BQB_Demo**
    * 增加了宏CLOSE_INTERNAL_CAP_EN，用来选择是否关闭内部电容。
    * 去除了一些不必要的代码：EMI初始化中写频偏值的代码；用于PA设置的临时代码。
* **spi(demo)**
    * 增加宏定义SPI_SLAVE_READY_TEST，用于测试slave_ready功能。
	
### Performance Improvements
* **IDE**
    * 添加编译命令make -j${NUMBER_OF_PROCESSORS}，增快编译速度。
* **plic**
    * 添加函数plic_isr的always_inline修饰，减少编译的bin文件大小。
	
---
## V1.5.0-Beta

### Version

* SDK version : telink_b91_driver_sdk v1.5.0-Beta.
* This version sdk support B91 A0，A1 and A2 chip.

### Bug Fixes

* **usb**
  * Fix the problem that usb_spk could not work when only USB_SPEAKER_ENABLE is turned on in USB_SPK Demo.
* **audio**
  * Correct spelling errors in "audio_buff" in AUDIO Demo.
  * Correct the data type definition error in the app_sin_data.h file in AUDIO_Demo.
* **spi**
  * Interface hspi_change_csn_pin()/pspi_change_csn_pin() have logic error, would configure the unused csn pin, the error has been fixed.And hspi_change_csn_pin()/pspi_change_csn_pin() has been deleted from the driver and moved to SPI_Demo.
  * Interface spi_slave_set_pin() have a pin configuration bug, the bug has been fixed.
  * Fixed the comment error of the hspi address length of the hspi_set_addr_len function.
* **rf**
  * Fixed the following problem: When using the function rf_clr_irq_status () to clear a certain rf interrupt status, other rf interrupts may  be cleared by mistake.
* **uart** 
  * add parameter of uart_num in function "uart_get_dma_rev_data_len() to distinguish UART0 and UART1".

### Features

* **freertos**
  * add freertos Demo

### Known issues

* N/A

### BREAKING CHANGES

* **flash**
  * In order to be compatible with other sdk, the flash_read_mid function used to pass in a buf pointer, and instead returns an unsigned int variable.
  * Mid is changed from 3byte to 4byte, and the highest byte is used to distinguish flashes with the same mid but different functions.

<hr style="border-bottom:2.5px solid rgb(146, 240, 161)">


### 版本

* SDK版本: telink_b91_driver_sdk v1.5.0-Beta。
* 此版本SDK支持B91 A0、A1和A2芯片。

### Bug Fixes

* **usb**
  * 修复在USB_SPK Demo中仅打开USB_SPEAKER_ENABLE时，usb_spk无法工作的问题。
* **audio**
  * 更正在AUDIO_DEMO中“audio_buff”中的拼写错误。
  * 更正在AUDIO_DEMO中app_sin_data.h文件中数据类型定义不符的错误。
* **spi**
  * Hspi_change_csn_pin()/Pspi_change_csn_pin() 接口有逻辑错误，会配置没有用到的csn引脚，该问题已经修复。并且Hspi_change_csn_pin()/Pspi_change_csn_pin()接口已经从Drvier中删除并移到了SPI_Demo中。
  * spi_slave_set_pin()接口引脚配置有错误，该问题已经修复。
  * 修复了hspi_set_addr_len函数的hspi地址长度的注释错误。
* **rf**
  * 修复了如下问题：在使用函数rf_clr_irq_status ()清除rf某个中断状态时，有可能会误清除其他rf中断。
* **uart**
  * 函数uart_get_dma_rev_data_len()增加参数uart_num区分UART0和UART1。

### Features

* **freertos**
  * 增加 freertos Demo

### Known issues

* N/A

### BREAKING CHANGES

* **flash**
  * 为了兼容其他sdk，flash_read_mid函数以前是传入一个buf指针，改为返回unsigned int型变量。
  * mid由3byte改为4byte,最高byte用于区分mid相同但是功能存在差异的flash。

---
## V1.4.0-Beta

### Version
* SDK version : telink_b91_driver_sdk v1.4.0-Beta.
* This version sdk support B91 A0，A1 and A2 chip.

### Bug Fixes

* **pm**
  * solve the problem that pragrem can not run when VBAT power supply is greater than 4.1V,and sys_int configure power mode as DCDC mode.
  * During the stability test, the program freezes after running for a period of time.
* **spi** 
  * spi enum SPI_MODE1、SPI_MODE2 definition as error value, fixed as correct value.
* **uart**
  * when close the irq in dma mode,the receive buffer maybe overflow if dma receive data continuous.

### Features

* **pwm**
  * add function interface "pwm_32k_chn_dis".
* **audio** 
  * Add the interface for selecting DAC output channel (audio_set_output_chn).
* **usb**
  * Add software to handle set_configuration control transmission.
  * Support usb cdc to send wMaxPacketSize (64bytes) length data.
* **uart** 
  * Add function uart_get_dma_rev_data_len() in DMA mode.
    

### Refactoring

* **spi** 
  * adjusted the initial process of LED in SPI_Demo.
* **audio**
  * Adjust the calls of i2c (control codec) and pwm (provide external clock) interfaces in audio applications.
* **usb/audio**
  * set the sampling rate of the DAC to 16k by default,In the mic_spk demo.

### Known issues

* N/A
### BREAKING CHANGES
* **audio**
  * Modify the audio_i2s_config function parameters, add audio_i2s_invert_config structure parameters to select i2s_lr_clk_invert and i2s_data_invert.

<hr style="border-bottom:2.5px solid rgb(146, 240, 161)">

### 版本

* SDK版本: telink_b91_driver_sdk v1.4.0-Beta。
* 此版本SDK支持B91 A0、A1和A2芯片。

### Bug Fixes

* **pm**
  * 解决VBAT供电高于4.1V且sys_int配置供电模式为DCDC时程序不能运行的问题。
  * 稳定性测试时，运行一段时间程序卡死的问题。
* **spi**
  * spi 的SPI_MODE1、SPI_MODE2枚举值定义错了，修复为了正确值。
* **uart**
  * DMA模式下关闭总中断时，如果UART仍在接收数据，有可能导致接收数组溢出。

### Features

* **pwm**
  * 添加函数接口pwm_32k_chn_dis。
* **audio**
  * 增加invert I2S CLK接口（audio_invert_i2s_lr_clk）。
* **usb**
  * 添加软件处理set_configuration控制传输。
  * 支持USB CDC发送wMaxPacketSize（64bytes）长度数据。
* **uart**
  * DMA模式增加函数uart_get_dma_rev_data_len()。


### Refactoring

* **spi** 
  * 调整了SPI_Demo中LED的初始化过程。
* **audio**
  * 调整AUDIO应用中i2c（控制codec）和pwm(提供外部时钟)接口的调用。
* **usb/audio**
  * 在mic_spk demo中将DAC的采样率默认设为16k。

### Known issues

* N/A

### BREAKING CHANGES

* **audio**
  * 修改audio_i2s_config function函数参数，新增audio_i2s_invert_config 结构体形参，设置i2s_lr_clk_invert 和i2s_data_invert。


---
## V1.3.1-Beta

### Version
* SDK version : telink_b91_driver_sdk V1.3.1-Beta.
* This version sdk support B91 A0，A1 and A2 chip.

### Bug Fixes
* **pm**:

 * Use digital register way to get 32k tick may read error tick,cause the wakeup time is incorrect with the setting time,the sleep time will very little or very big,will not wakeup ontime.

  + The previous configuration will cause the voltage on the VDD1V8 pin to be generally high during normal operation, about 3.0V~3.1V (the reason is that the previous configuration will cause the two power supplies to be powered at the same time), which may cause abnormal phenomena. The version is modified to the correct configuration. In the default configuration, the voltage on the VDD1V8 pin will be about 2.8V.

* **USB**:
  + The header file "stdDescriptors.h" is not case sensitive when included, so it is uniformly changed to "StdDescriptors.h"

* **AUDIO**:
      + In the function audio_set_dmic_pin(), fix the problem of possible errors in setting the dmic pin

### Known issues
* N/A
### BREAKING CHANGES
* **pwm**
   + Detailed description: add 32k clock calibration,the interface function(pwm_32k_chn_en) changes" =" to |=，to prevent reset other channels.

<hr style="border-bottom:2.5px solid rgb(146, 240, 161)">

### 版本

* SDK版本: telink_b91_driver_sdk V1.3.1-Beta。
* 此版本SDK支持B91 A0、A1和A2芯片。

### Bug Fixes

* **pm**:
  * 使用数字方式获取32k tick出错会导致设置的唤醒tick与预期不符，从而导致唤醒时间与设置的不相符，睡眠时间可能过小或者过大，可能会表现为不唤醒。
 + 之前的配置会导致正常工作时，VDD1V8引脚上的电压会普遍偏高，大概为3.0V~3.1V（原因是之前的配置会导致两个电源会同时供电），可能会造成异常现象，此版本修改为正确配置，默认配置下，VDD1V8引脚上的电压会是2.8V左右。

* **USB**:
    + 头文件"stdDescriptors.h",include时没有区分大小写，统一改成"StdDescriptors.h"


* **AUDIO**:
      + 在函数audio_set_dmic_pin()中，修复设置dmic pin可能出错问题

### Known issues
* N/A

### BREAKING CHANGES
* **pwm**
 + 添加32k时钟源校准，接口函数pwm_32k_chn_en修改，=改为|=，防止重置其他通道.

---

### Version



## V1.3.0-Beta

### Version
* SDK version : telink_b91_driver_sdk V1.3.0-Beta.
* This version sdk support B91 A0，A1 and A2 chip.

### Bug Fixes

* **pm**:
  * Solved the problem that the reboot state cannot be read after the reboot comes back, and the deep state is read only. The solution is to add a status "MCU_STATUS_REBOOT_DEEP_BACK" to the enumeration type "pm_mcu_status".
  * Solved the problem that the timer could not wake up after waking up several times when the function clock_cal_32k_rc() was not called.
  * Solved the problem that when the flash is configured with four wires, it cannot work normally after waking up from suspend mode.
* **clock**: 
  * Solved the problem of invalid kick with PD0 port when 32k xtal starts to vibrate.
* **adc**: 
  * Fix the sort bug of function adc_sort_and_get_average_code() in ADC_Demo.
  * Fix the problem that the adc_get_temperature()/adc_get_voltage() function in ADC_Demo may not get data for the first time.

### Features

* **swire**
  * add swire driver and demo.
* **cproject**: 
  * delete B91 keyword.
* **adc**
  * Added the 1/4 pre_scale parameter enum of the adc_set_scale_factor() function, and deleted the 1, 1/2, 1/8 pre_scale parameter enum.
* **pm** 
  * change the variable name g_pm_multi_addr ->g_pm_mspi_cfg.
    
  * Improved the problem of incorrect LED configuration during the suspend sleep current test in pm_demo.
* **audio** 
  * add audio_set_codec_wnf api.
  * add input_mode_select at line_in.
  * set all gain to 0db default in audio.c.
* **flash** 
  * add flash_set_xip_config function.
* **usb** 
  * add host speaker stop playback detection function in spk demo.
* **pwm**
  * add function interface pwm_32k_chn_update_duty_cycle.


### Known issues
* N/A
### BREAKING CHANGES
* **s7816**
  * modify the s7816_set_time(),delete the atr wait time in the function of coldreset and warmreset.

<hr style="border-bottom:2.5px solid rgb(146, 240, 161)">

### 版本

* SDK版本: telink_b91_driver_sdk V1.3.0-Beta。
* 此版本SDK支持B91 A0、A1和A2芯片。

### Bug Fixes

* **pm**:
  * 解决了reboot回来后不能读到reboot状态，只读到deep状态的问题。解决方案是在枚举类型"pm_mcu_status"中新增一种状态"MCU_STATUS_REBOOT_DEEP_BACK"。 
  * 解决了不调用函数clock_cal_32k_rc()时，timer唤醒几次后不能再唤醒的问题。
  * 解决了设置flash四线时，suspend唤醒后flash不能正常工作的问题。
* **clock**:
  * 解决了32k xtal起振时用PD0口kick无效的问题。
* **adc**:
  * 修复了ADC_Demo中的adc_sort_and_get_average_code()函数的排序问题。
  * 修复了ADC_Demo中的adc_get_temperature()/adc_get_voltage()函数第一次可能会采不到数据的问题。

### Features

* **swire**
  * 增加swire的驱动和demo。
* **cproject**:
  * 删除关键字B91。
* **adc**
  * 添加了adc_set_scale_factor（）函数的1/4 pre_scale参数枚举，并删除了1、1 / 2、1 / 8 pre_scale参数枚举。


* **pm**

  * 更改变量名称g_pm_multi_addr-> g_pm_mspi_cfg。
  * 改进了在pm_demo中的suspend睡眠电流测试过程中，指示灯配置错误的问题。

* **audio** 
  * 添加接口audio_set_codec_wnf。
  * 在line_in添加input_mode_select。
  * 在audio.c中将所有增益设置为0db默认值。

* **flash** 

  * 添加函数flash_set_xip_config 。

* **usb** 

  * 在spk_demo中添加主机扬声器停止播放的检测功能。

* **pwm**

  * 添加功能接口pwm_32k_chn_update_duty_cycle。




### Known issues
* N/A

### BREAKING CHANGES
* **s7816**

  * 修改函数s7816_set_time()，在coldreset和warmreset函数中删除atr等待时间。


---
## V1.2.0-Beta

### Version
* SDK version : telink_b91_driver_sdk V1.2.0-Beta.
* This version sdk support B91 A0 and A1 chip.

### Bug Fixes

* **uart** 

  * Fix the problem that the configuration function uart\_receive_dma cannot continue to receive data after UART suspend or reset UART

### Features
* **sys**
  * Modify the member name of the parameter enumeration vbat\_type\_e variable in the sys\_init function
* **watchdog**
	* Simplify the parameters of the wd\_set\_interval\_ms function, and no longer retain the parameter tick\_per\_ms
* **timer**
   * In the timer\_set\_mode function, the initial tick and capture tick are no longer configured and are encapsulated into two interfaces (timer\_set\_init\_tick, timer\_set\_cap_tick)
* **gpio** 
  * Modify default macro definition in gpio_default.h 
  * The corresponding enable function gpio\_irq\_en,gpio\_gpio2risc0\_irq\_en,gpio\_gpio2risc1\_irq\_en is no longer called in the gpio\_set\_irq,gpio\_set\_gpio2risc0\_irq,gpio\_set\_gpio2risc1\_irq function, which is flexibly called in the application layer
* **plic**
  * Delete the parameter of the function plic\_interrupt\_claim
* **other**  
  * Optimize the include structure of the header file, add Include paths in the project configuration options

### Known issues
* **32k xtal** 
  - 32k xtal can't be used on the A1 version chip
* **gpio** 
  - GPIO_PD0 can't be used on the A1 version chip
* **other**
  - Note (A1 version chip): Use 2.8V voltage to power the chip. When the power supply voltage is higher than 2.8V, the program may not run.
### BREAKING CHANGES
* N/A

<hr style="border-bottom:2.5px solid rgb(146, 240, 161)">

### Bug Fixes

* **uart** 

  * 修复UART suspend起来或reset UART后，配置函数uart\_receive\_dma不能继续接收数据问题

### 版本
* SDK版本: telink_b91_driver_sdk v1.2.0-Beta.
* 此版本SDK支持B91 A0和A1芯片.
### Features
* **sys**
  * 在sys\_init函数中修改枚举vbat\_type\_e类型的成员名称。
* **watchdog**
	* 简化wd\_set\_interval_ms函数的参数,参数period\_ms为间隔时间，不再保留参数tick\_per\_ms
* **timer**
  * 在timer\_set\_mode函数中,不再配置初始tick和capture tick，分别封装成两个接口（timer\_set\_init_tick,timer\_set\_cap\_tick）
* **gpio** 
  * 修改gpio_default.h默认宏定义
  * 在gpio\_set\_irq,gpio\_set\_gpio2risc0_irq/gpio\_set\_gpio2risc1\_irq函数中不再调用对应使能函数gpio\_irq\_en,gpio\_gpio2risc0\_irq\_en,gpio\_gpio2risc1\_irq\_en,在应用层里灵活调用
* **plic**
  * 删去函数plic\_interrupt\_claim的参数
* **other**  
  * 优化头文件包含结构，在工程配置选项里添加Include paths
### Known issues
* **32k xtal** 
   - 32k xtal 在A1版本芯片上不能使用。
* **gpio** 
  - GPIO_PD0在A1版本芯片上不能使用
* **其他**
  - 注意事项（A1版本芯片）：使用2.8V电压给芯片供电，供电电压高于2.8V时，程序有可能会跑不起来。

### BREAKING CHANGES
* N/A

---
## V1.1.0-Beta
### Version

* SDK version : telink_b91_driver_sdk v1.1.0-Beta.
* This version sdk support B91 A0 and A1 chip.

### Bug Fixes

* **pm**

  * Solved the problem of not being able to wake up when the sleep time is less than 950us.

  * Solved the problem that the sleep function cannot be awakened when the sleep function is called immediately within 500us after power on.

* **mdec**: 

  * Solved the problem of first interruption after initialization.

* **clock**: 

  * Solve the problem that the program sometimes stops in the clock initialization function (clock_init).

* **lpc**: 

  * Solved the problem of abnormal operation of lpc.

* **flash**: 

  * Remove the declaration of the function flash_read_mid_uid_with_check_ram. Added declaration of function flash_read_mid_uid_with_check.

  * Solved the problem of not being able to cross pages when writing flash across pages.

* **bqb:** 

  * fix the issue that could not send packets through instructions in initialization state.

* **rf:** 

  * Fixed an abnormal function of rf_set_rffe_pin function.

  * Modify the function rf_set_rx_dma and rf_set_tx_dma to adapt to sending and receiving long packets(up to 255 byte)

  * Modified the usage example of rx　interrupt. 

  * fix the issue that rf_set_pipe_access_code can not set 5-byte access code.

  * Fix the register setting error in the rf_set_preamble_len function.

  * Modify the access code of S2 and S8 mode to manual trigger.

* **emi** 

  * Fix the problem of zigbee mode 2480Mhz band edge can not pass the spec.
  * Fixed single carrier frequency offset in emi.

* **.s** 

  * Modified cstartup_b91_flash.S and cstartup_b91_ram.S file to reserve space. 

* **reboot&clock**

  * fix the issue that reboot will crash when hclk is half of cclk in pll_clk and pad24M_clk,need add a 

* **analog** 

  * fix a compile error in analog demo

* **spi** 

  * Simplify structure and macro definition naming, optimize SPI slave demo structure，fix compilation error

* **uart** 

  * Fix the issue that the mask might be overwritten by the set of bit width when the uart-init.

### Features
* **sys**

  * Add parameters vbat_v in the sys_init function.

* **sys_time**

  * Change CLOCK_16M_SYS_TIMER_CLK_XXX to SYSTEM_TIMER_TICK_XX.

* **pm** 

  * Increase the convergence calibration function of sleep time.

  * Increase USB wake-up function.

  * Increase the suspend sleep function.
  * add long sleep wakeup api.

* **bqb** 

  * add a macro to switch new/old setting ways to configure uart/cap/flash.

* **license** 

  * change the author to "Driver Group" and modify the license template to meet the requiements CI checking.

* **rf** 

  * Add ANT mode to rf demo.

  * update the function rf_set_chn and rf_set_ble_chn to support out-of-band frequency point setting

  * In order to facilitate the development of SDK, the DMA setting function related to RF is divided into several functions with single function, and added to the driver.

* **AES** 

  * optimize AES_set_mode apiuart:add Single line transceiver function

* **uart**

  * add UART-TXDONE/RXDONE interrupt in DMA mode in the chip version of A1 .

  * data of variable length is acceptable in uart-dma mode in the chip version of A1, the length of receive-date can be work out.

* **s7816**

  * add s7816 demo, can communicate with the equipment that support IS07816 protocol.

* **gpio**

  * add 30k-digital pull-up。

* **plic**

  * Optimized interrupt handling function structure in PLIC

* **i2c**

  * add function that the master sends a stop after detecting a NACK in the ID phase，then stops reading and writing。

  * Add tx_done interrupt in dma demo

* **audio**

  * In MONO mode, the input channel (left/right) can be selected

  * Add input channel acquisition mode selection (single-ended/differential)

  * Add configuration for ADC and DAC to work at different sampling rates（ADC=16K，DAC=48K）

* **spi**

  * Add 3line mode and master to multiple-slave function

* **adc**

  * Add ADC module  in the chip version of A1 which support DMA sampling mode and manual sampling mode, support Vbat\GPIO\Temperature sensor sampling

### Known issues
* **32k xtal** 
  - 32k xtal can't be used on the A1 version chip
* **gpio** 
  - GPIO_PD0 can't be used on the A1 version chip
* **other**
  - Note (A1 version chip): Use 2.8V voltage to power the chip. When the power supply voltage is higher than 2.8V, the program may not run.
### BREAKING CHANGES
* N/A

<hr style="border-bottom:2.5px solid rgb(146, 240, 161)">

### Bug Fixes

* **pm**  

  * 解决了睡眠时间小于950us时，不能唤醒的问题。 

  * 解决了上电500us内立即调用睡眠函数时无法唤醒的问题。

* **mdec**  

  * 解决了初始化后先进一次中断的问题。

* **clock**  

  * 解决了程序有时会停在时钟初始化函数（clock_init）中的问题。

* **lpc**  

  * 解决了lpc工作异常的问题。

* **flash**  

  * 去掉函数flash_read_mid_uid_with_check_ram的声明。新增函数flash_read_mid_uid_with_check的声明。

  * 解决了跨页写flash时，不能跨页的问题。

* **bqb** 

  * 修复了在初始化状态下，不能通过指令发包的问题。

* **rf** 

  * 修复了函数rf_set_rffe_pin功能异常 。

  * 修改函数参数rf_set_rx_dma ,rf_set_tx_dma以适应长包（最大长度255byte）

  * 修改rx interrupt使用示例

  * 修复了rf_set_pipe_access_code不能设置5byte长度的access code

  * 修复了rf_set_preamble_len函数中的寄存器设置错误

  *  修改S2,S8模式access code为手动触发

* **emi** 

  * 修复zigbee 2480Mhz边带不能过spec的问题
  * 修复单通频偏的问题

* **.s** 

  * 更新s文件以预留空间

* **reboot&clock** 

  * 修复当hclk等于1/2cclk时reboot出现宕机的问题。

* **analog** 

  * 修复analog_demo中的错误

* **spi** 

  * 精简结构体和宏定义命名，优化SPI slave demo结构，修复编译错误

* **uart** 

  * 修复UART初始化设置位宽时可能会覆盖掉mask的问题

### 版本
* SDK版本: telink_b91_driver_sdk v1.0.0-Beta.
* 此版本SDK支持B91 A0和A1芯片.
### Features
* **sys**

  * 在sys_init函数中添加参数vbat_v。

* **sys_time**

  * 将CLOCK_16M_SYS_TIMER_CLK_XXX更改为SYSTEM_TIMER_TICK_XX。

* **pm**

  * 增加睡眠时间的收敛校准功能。

  * 增加USB唤醒功能。 

  * 增加suspend睡眠功能。    
  * 新增长睡眠接口。

* **bqb** 

  * 增加了一个宏，用来选择新的或旧的方式来配置串口、CAP值以及适配的Flash大小。

* **license** 

  * 将License的作者统一改成了“Driver Group”以及统一调整了License的模板。    

* **rf** 

  * 添加ANT模式到rf demo中.

  * 更新设置channel的函数rf_set_chn和rf_set_ble_chn以支持带外频点设置

  * 为了方便SDK的开发将DMA设置函数拆分成几个功能单一的函数并添加到驱动中。

* **AES** 

  * 优化AES_set_mode接口uart:新增单线收发模式

* **uart**

  * A1版本芯片新增DMA模式UART-TXDONE/RXDONE中断。

  * A1版本芯片UART-DMA模式可以接收不定长的数据，并能计算出接收数据的长度

* **s7816**

  * 新增s7816模块，可与支持ISO7816协议的设备进行单线半双工通信。

* **gpio**

  * 新增30k数字上拉。

* **plic**

  * 在PLIC中优化了中断处理函数结构

* **i2c**

  * 增加了master在发ID阶段检测到NACK后发stop，并停止读写动作

  * 在dma demo中增加了tx_done中断。

* **audio**

  * MONO模式下，可选择输入通道（左/右)，默认只有左通道输出；

  * 增加了输入通道采集模式选择（单端/差分）

  * 新增ADC和DAC在不同采样率工作的配置（ADC=16K，DAC=48K）。

* **spi**

  * 新增3line模式和master对多slave功能。

* **adc**

  * A1版本芯片新增ADC模块，支持DMA采样模式和手动采样模式，支持Vbat\GPIO\Temperature sensor采样


### Known issues
* **32k xtal** 
   - 32k xtal 在A1版本芯片上不能使用。
* **gpio** 
  - GPIO_PD0在A1版本芯片上不能使用
* **其他**
  - 注意事项（A1版本芯片）：使用2.8V电压给芯片供电，供电电压高于2.8V时，程序有可能会跑不起来。

### BREAKING CHANGES
* N/A

---
## V1.0.0-Beta
### Version
* SDK version : telink_b91_driver_sdk v1.0.0-Beta.
* This version sdk support B91 A0 chip.
### Features
* AES/AUDIO/FLASH/LPC/MDEC/PKE/PM/PWM/RF/STIMER/TIMER/TRNG
* Interface:GPIO/I2C/SPI/UART/USB
### Known issues
* **pm** 
  - The suspend mode can't be used on the A0 version chip
  - In order to avoid the problem of abnormal overwrite sram in the A0 chip, a special treatment is added to the sys_init function 
    (if it is the A0 chip and it is the first power-on, it will enter deep, wake up after 100ms, and the program will be executed normally after wake-up)
* **gpio** 
  - GPIO_PD can't be used on the A0 version chip
* **uart** 
  - It is recommended to use the non-DMA mode. In the A0 version, the function of the DMA mode is defective. When receiving, the interrupt can be generated normally 
    only when the received data length is known.
* **clock**
  - The use of clock_init function needs to pay attention to two points:
   - CCLK_DIV2_TO_HCLK and HCLK_DIV4_TO_PCLK can’t be set at the same time
   - If the pm or reboot function is used, the parameter hclk_div cannot be set to CCLK_DIV2_TO_HCLK
* **other**
  - Precautions for the use of the development board (A0 version chip):
  If USB power supply, you need to unplug the USBVBUS-VBUS jumper (that is, you can't use +5V directly to the Vbus of the chip), otherwise it will cause USB/Audio 
  instability.
### BREAKING CHANGES
* N/A

<hr style="border-bottom:2.5px solid rgb(146, 240, 161)">

### 版本
* SDK版本: telink_b91_driver_sdk v1.0.0-Beta.
* 此版本SDK支持B91 A0芯片.
### Features
* AES/AUDIO/FLASH/LPC/MDEC/PKE/PM/PWM/RF/STIMER/TIMER/TRNG
* Interface:GPIO/I2C/SPI/UART/USB
### Known issues
* **pm** 
   - suspend模式在A0版本芯片上不可以使用
   - 为了规避A0芯片存在的异常改写sram问题，在sys_init函数中加了一段特殊处理（如果是A0芯片，并且是第一次上电，则会进入deep，100ms后唤醒，唤醒后会正常执行程序）
* **gpio** 
  - GPIO_PD在A0版本芯片上不能使用
* **uart** 
  - 建议使用非DMA模式，在A0版本，DMA模式的功能有缺陷，接收时，只有已知接收数据长度时，才能正常的产生中断。
* **clock**
  - clock_init函数的使用需要注意一下两点:
   - CCLK_DIV2_TO_HCLK和HCLK_DIV4_TO_PCLK不能同时设置 
   - 如果有使用pm或者reboot功能，则参数hclk_div不能设置为CCLK_DIV2_TO_HCLK
* **其他**
  - 开发板使用注意事项（A0版本芯片）：如果USB供电时，需要拔掉USBVBUS-VBUS跳帽（即不能用+5V直接给芯片的Vbus），否则会导致USB/Audio不稳定

### BREAKING CHANGES
* N/A