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
all: Car_Runner.axf

# Tool invocations
Car_Runner.axf: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: MCU Linker'
	arm-none-eabi-gcc -nostdlib -L"C:\Users\pedro\Desktop\Projetos\Car-Runner\LEETC_SE1\Debug" -L"C:\Users\pedro\Desktop\Projetos\Car-Runner\CMSIS_CORE_LPC17xx\Debug" -Xlinker -Map="Car_Runner.map" -Xlinker --cref -Xlinker --gc-sections -Xlinker -print-memory-usage -mcpu=cortex-m3 -mthumb -T "Car_Runner_Debug.ld" -o "Car_Runner.axf" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '
	$(MAKE) --no-print-directory post-build

# Other Targets
clean:
	-$(RM) $(EXECUTABLES)$(OBJS)$(C_DEPS) Car_Runner.axf
	-@echo ' '

post-build:
	-@echo 'Performing post-build steps'
	-arm-none-eabi-size "Car_Runner.axf"; # arm-none-eabi-objcopy -v -O binary "Car_Runner.axf" "Car_Runner.bin" ; # checksum -p LPC1769 -d "Car_Runner.bin";
	-@echo ' '

.PHONY: all clean dependents post-build

-include ../makefile.targets
