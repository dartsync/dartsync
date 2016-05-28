################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/peer/filemonitor.c \
../src/peer/p2p.c \
../src/peer/peer.c \
../src/peer/peer_peertable.c 

O_SRCS += \
../src/peer/filemonitor.o \
../src/peer/p2p.o \
../src/peer/peer.o \
../src/peer/peer_peertable.o 

OBJS += \
./src/peer/filemonitor.o \
./src/peer/p2p.o \
./src/peer/peer.o \
./src/peer/peer_peertable.o 

C_DEPS += \
./src/peer/filemonitor.d \
./src/peer/p2p.d \
./src/peer/peer.d \
./src/peer/peer_peertable.d 


# Each subdirectory must supply rules for building sources it contributes
src/peer/%.o: ../src/peer/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


