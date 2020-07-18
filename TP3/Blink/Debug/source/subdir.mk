################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../source/App.c \
../source/DAC.c \
../source/FSKV1.c \
../source/SysTick.c \
../source/adc.c \
../source/buffer.c \
../source/comparator.c \
../source/demodulador_FSK.c \
../source/dma.c \
../source/fsk.c \
../source/ftm.c \
../source/gpio.c \
../source/pit.c \
../source/timer.c \
../source/uart.c 

OBJS += \
./source/App.o \
./source/DAC.o \
./source/FSKV1.o \
./source/SysTick.o \
./source/adc.o \
./source/buffer.o \
./source/comparator.o \
./source/demodulador_FSK.o \
./source/dma.o \
./source/fsk.o \
./source/ftm.o \
./source/gpio.o \
./source/pit.o \
./source/timer.o \
./source/uart.o 

C_DEPS += \
./source/App.d \
./source/DAC.d \
./source/FSKV1.d \
./source/SysTick.d \
./source/adc.d \
./source/buffer.d \
./source/comparator.d \
./source/demodulador_FSK.d \
./source/dma.d \
./source/fsk.d \
./source/ftm.d \
./source/gpio.d \
./source/pit.d \
./source/timer.d \
./source/uart.d 


# Each subdirectory must supply rules for building sources it contributes
source/%.o: ../source/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU C Compiler'
	arm-none-eabi-gcc -DCPU_MK64FN1M0VLL12 -D__USE_CMSIS -DDEBUG -I../source -I../ -I../SDK/CMSIS -I../SDK/startup -O0 -fno-common -g3 -Wall -c -ffunction-sections -fdata-sections -ffreestanding -fno-builtin -mcpu=cortex-m4 -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


