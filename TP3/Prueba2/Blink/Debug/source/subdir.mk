################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../source/App.c \
../source/Coordinates.c \
../source/DAC.c \
../source/FSK.c \
../source/FXOS8700CQ.c \
../source/MCP25625.c \
../source/SPI.c \
../source/SysTick.c \
../source/adc.c \
../source/buffer.c \
../source/bufferCAN.c \
../source/demodulador_FSK.c \
../source/gpio.c \
../source/i2c.c \
../source/pit.c \
../source/position.c \
../source/timer.c \
../source/uart.c 

OBJS += \
./source/App.o \
./source/Coordinates.o \
./source/DAC.o \
./source/FSK.o \
./source/FXOS8700CQ.o \
./source/MCP25625.o \
./source/SPI.o \
./source/SysTick.o \
./source/adc.o \
./source/buffer.o \
./source/bufferCAN.o \
./source/demodulador_FSK.o \
./source/gpio.o \
./source/i2c.o \
./source/pit.o \
./source/position.o \
./source/timer.o \
./source/uart.o 

C_DEPS += \
./source/App.d \
./source/Coordinates.d \
./source/DAC.d \
./source/FSK.d \
./source/FXOS8700CQ.d \
./source/MCP25625.d \
./source/SPI.d \
./source/SysTick.d \
./source/adc.d \
./source/buffer.d \
./source/bufferCAN.d \
./source/demodulador_FSK.d \
./source/gpio.d \
./source/i2c.d \
./source/pit.d \
./source/position.d \
./source/timer.d \
./source/uart.d 


# Each subdirectory must supply rules for building sources it contributes
source/%.o: ../source/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU C Compiler'
	arm-none-eabi-gcc -DCPU_MK64FN1M0VLL12 -D__USE_CMSIS -DDEBUG -I../source -I../ -I../SDK/CMSIS -I../SDK/startup -O0 -fno-common -g3 -Wall -c -ffunction-sections -fdata-sections -ffreestanding -fno-builtin -mcpu=cortex-m4 -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


