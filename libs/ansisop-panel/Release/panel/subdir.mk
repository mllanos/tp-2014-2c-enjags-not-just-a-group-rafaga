################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../panel/cpu.c \
../panel/kernel.c \
../panel/panel.c 

OBJS += \
./panel/cpu.o \
./panel/kernel.o \
./panel/panel.o 

C_DEPS += \
./panel/cpu.d \
./panel/kernel.d \
./panel/panel.d 


# Each subdirectory must supply rules for building sources it contributes
panel/%.o: ../panel/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O3 -Wall -c -fmessage-length=0 -fPIC -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


