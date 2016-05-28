################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/utils/file_utils.c \
../src/utils/filetable.c \
../src/utils/seg.c \
../src/utils/utils.c 

O_SRCS += \
../src/utils/filetable.o \
../src/utils/seg.o 

OBJS += \
./src/utils/file_utils.o \
./src/utils/filetable.o \
./src/utils/seg.o \
./src/utils/utils.o 

C_DEPS += \
./src/utils/file_utils.d \
./src/utils/filetable.d \
./src/utils/seg.d \
./src/utils/utils.d 


# Each subdirectory must supply rules for building sources it contributes
src/utils/%.o: ../src/utils/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


