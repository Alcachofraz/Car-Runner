################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/adxl.c \
../src/button.c \
../src/eeprom.c \
../src/esp.c \
../src/flash.c \
../src/i2c.c \
../src/lcd.c \
../src/led.c \
../src/network.c \
../src/rtc.c \
../src/spi.c \
../src/transport.c \
../src/uart.c \
../src/wait.c 

OBJS += \
./src/adxl.o \
./src/button.o \
./src/eeprom.o \
./src/esp.o \
./src/flash.o \
./src/i2c.o \
./src/lcd.o \
./src/led.o \
./src/network.o \
./src/rtc.o \
./src/spi.o \
./src/transport.o \
./src/uart.o \
./src/wait.o 

C_DEPS += \
./src/adxl.d \
./src/button.d \
./src/eeprom.d \
./src/esp.d \
./src/flash.d \
./src/i2c.d \
./src/lcd.d \
./src/led.d \
./src/network.d \
./src/rtc.d \
./src/spi.d \
./src/transport.d \
./src/uart.d \
./src/wait.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU C Compiler'
	arm-none-eabi-gcc -DDEBUG -DFREERTOS -D__CODE_RED -DCORE_M3 -D__USE_CMSIS=CMSIS_CORE_LPC17xx -D__LPC17XX__ -D__NEWLIB__ -I"C:\Users\pedro\Desktop\Projetos\G9_CAR_RUNNER_RTOS\MQTTPacket\inc" -I"C:\Users\pedro\Desktop\Projetos\G9_CAR_RUNNER_RTOS\LEETC_SE1\inc" -I"C:\Users\pedro\Desktop\Projetos\G9_CAR_RUNNER_RTOS\FreeRTOS-Kernel\include" -I"C:\Users\pedro\Desktop\Projetos\G9_CAR_RUNNER_RTOS\FreeRTOS-Kernel\src\portable" -I"C:\Users\pedro\Desktop\Projetos\G9_CAR_RUNNER_RTOS\CMSIS_CORE_LPC17xx\inc" -O0 -fno-common -g3 -Wall -c -fmessage-length=0 -fno-builtin -ffunction-sections -fdata-sections -fmerge-constants -fmacro-prefix-map="../$(@D)/"=. -mcpu=cortex-m3 -mthumb -D__NEWLIB__ -fstack-usage -specs=nano.specs -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


