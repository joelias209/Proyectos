################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (9-2020-q2-update)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
D:/Users/Joel/Documents/STM32CubeIDE/Common\ FreeRTOS_Projects/ThirdParty/FreeRTOS/portable/GCC/ARM_CM4F/port.c 

OBJS += \
./ThirdParty/FreeRTOS/portable/GCC/ARM_CM4F/port.o 

C_DEPS += \
./ThirdParty/FreeRTOS/portable/GCC/ARM_CM4F/port.d 


# Each subdirectory must supply rules for building sources it contributes
ThirdParty/FreeRTOS/portable/GCC/ARM_CM4F/port.o: D:/Users/Joel/Documents/STM32CubeIDE/Common\ FreeRTOS_Projects/ThirdParty/FreeRTOS/portable/GCC/ARM_CM4F/port.c ThirdParty/FreeRTOS/portable/GCC/ARM_CM4F/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F446xx -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I"D:/Users/Joel/Documents/STM32CubeIDE/Common FreeRTOS_Projects/ThirdParty/FreeRTOS/include" -I"D:/Users/Joel/Documents/STM32CubeIDE/Common FreeRTOS_Projects/ThirdParty/FreeRTOS/portable/GCC/ARM_CM4F" -I"D:/Users/Joel/Documents/STM32CubeIDE/Common FreeRTOS_Projects/ThirdParty/SEGGER/Config" -I"D:/Users/Joel/Documents/STM32CubeIDE/Common FreeRTOS_Projects/ThirdParty/SEGGER/OS" -I"D:/Users/Joel/Documents/STM32CubeIDE/Common FreeRTOS_Projects/ThirdParty/SEGGER/SEGGER" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"ThirdParty/FreeRTOS/portable/GCC/ARM_CM4F/port.d" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

