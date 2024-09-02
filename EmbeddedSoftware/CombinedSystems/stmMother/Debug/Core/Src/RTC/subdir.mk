################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (10.3-2021.10)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/RTC/RTCOperation.c 

OBJS += \
./Core/Src/RTC/RTCOperation.o 

C_DEPS += \
./Core/Src/RTC/RTCOperation.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/RTC/%.o Core/Src/RTC/%.su: ../Core/Src/RTC/%.c Core/Src/RTC/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32L412xx -c -I../Core/Inc -I../Drivers/STM32L4xx_HAL_Driver/Inc -I../Drivers/STM32L4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32L4xx/Include -I../Drivers/CMSIS/Include -I../FATFS/Target -I../FATFS/App -I../Middlewares/Third_Party/FatFs/src -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Src-2f-RTC

clean-Core-2f-Src-2f-RTC:
	-$(RM) ./Core/Src/RTC/RTCOperation.d ./Core/Src/RTC/RTCOperation.o ./Core/Src/RTC/RTCOperation.su

.PHONY: clean-Core-2f-Src-2f-RTC

