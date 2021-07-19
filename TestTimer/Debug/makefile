################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include src/subdir.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 

# All Target
all: LAB3v1.axf

# Tool invocations
LAB3v1.axf: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: MCU Linker'
	arm-none-eabi-gcc -nostdlib -L"C:\Users\pedro\Desktop\LEECT\Semestre 5\SE1\Workplace\LEETC_SE1\Debug" -L"C:\Users\pedro\Desktop\LEECT\Semestre 5\SE1\Workplace\CMSIS_CORE_LPC17xx\Debug" -Xlinker -Map="LAB3v1.map" -Xlinker --cref -Xlinker --gc-sections -Xlinker -print-memory-usage -mcpu=cortex-m3 -mthumb -T "LAB3v1_Debug.ld" -o "LAB3v1.axf" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '
	$(MAKE) --no-print-directory post-build

# Other Targets
clean:
	-$(RM) $(EXECUTABLES)$(OBJS)$(C_DEPS) LAB3v1.axf
	-@echo ' '

post-build:
	-@echo 'Performing post-build steps'
	-arm-none-eabi-size "LAB3v1.axf"; # arm-none-eabi-objcopy -v -O binary "LAB3v1.axf" "LAB3v1.bin" ; # checksum -p LPC1769 -d "LAB3v1.bin";
	-@echo ' '

.PHONY: all clean dependents post-build

-include ../makefile.targets
