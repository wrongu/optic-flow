################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../driver.cpp \
../features.cpp \
../functors.cpp \
../math_helpers.cpp \
../of.cpp \
../segmentation.cpp 

OBJS += \
./driver.o \
./features.o \
./functors.o \
./math_helpers.o \
./of.o \
./segmentation.o 

CPP_DEPS += \
./driver.d \
./features.d \
./functors.d \
./math_helpers.d \
./of.d \
./segmentation.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -I/usr/local/include/opencv -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


