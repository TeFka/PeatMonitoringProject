################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (10.3-2021.10)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/Depth/DepthMeasuring.c 

OBJS += \
./Core/Src/Depth/DepthMeasuring.o 

C_DEPS += \
./Core/Src/Depth/DepthMeasuring.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/Depth/%.o Core/Src/Depth/%.su: ../Core/Src/Depth/%.c Core/Src/Depth/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32L412xx -c -I../Core/Inc -I../Drivers/STM32L4xx_HAL_Driver/Inc -I../Drivers/STM32L4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32L4xx/Include -I../Drivers/CMSIS/Include -I../FATFS/Target -I../FATFS/App -I../Middlewares/Third_Party/FatFs/src -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Src-2f-Depth

clean-Core-2f-Src-2f-Depth:
	-$(RM) ./Core/Src/Depth/DepthMeasuring.d ./Core/Src/Depth/DepthMeasuring.o ./Core/Src/Depth/DepthMeasuring.su

.PHONY: clean-Core-2f-Src-2f-Depth

