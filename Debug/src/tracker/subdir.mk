################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/tracker/peertable.c \
../src/tracker/tracker.c 

OBJS += \
./src/tracker/peertable.o \
./src/tracker/tracker.o 

C_DEPS += \
./src/tracker/peertable.d \
./src/tracker/tracker.d 


# Each subdirectory must supply rules for building sources it contributes
src/tracker/%.o: ../src/tracker/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


