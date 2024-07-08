@echo "*************** start of post build *****************"
@echo "this is post build!!! current configure is :%1"
@if exist output (
@echo output target: output/%1.bin with CRC.
riscv32-elf-objcopy -S -O binary %1.elf  output/%1.bin
..\..\..\tl_check_fw2.exe  output/%1.bin
) else (
@echo output target: %1.bin with CRC.
riscv32-elf-objcopy -S -O binary %1.elf  %1.bin
..\..\..\tl_check_fw2.exe  %1.bin 
)
@echo  "---------------------------  SDK version info ---------------------------"
@grep -E "[$]{3}[a-zA-Z0-9 _.]+[$]{3}" --text -o %1.bin | sed 's/\$//g'
@echo  "---------------------------  SDK version end  ---------------------------"
@echo "**************** end of post build ******************"