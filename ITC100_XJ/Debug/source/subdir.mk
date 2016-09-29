################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../source/ExternPlatform.cpp \
../source/external_query.cpp \
../source/main.cpp \
../source/occi_opration.cpp \
../source/parsing_report.cpp \
../source/parsing_report_alalrm.cpp \
../source/queue.cpp \
../source/signal_check_time.cpp \
../source/signal_control_heart.cpp \
../source/signal_report.cpp \
../source/signal_timeout_check.cpp \
../source/web_control_heart.cpp \
../source/web_message_recv.cpp \
../source/web_online_heart.cpp 

OBJS += \
./source/ExternPlatform.o \
./source/external_query.o \
./source/main.o \
./source/occi_opration.o \
./source/parsing_report.o \
./source/parsing_report_alalrm.o \
./source/queue.o \
./source/signal_check_time.o \
./source/signal_control_heart.o \
./source/signal_report.o \
./source/signal_timeout_check.o \
./source/web_control_heart.o \
./source/web_message_recv.o \
./source/web_online_heart.o 

CPP_DEPS += \
./source/ExternPlatform.d \
./source/external_query.d \
./source/main.d \
./source/occi_opration.d \
./source/parsing_report.d \
./source/parsing_report_alalrm.d \
./source/queue.d \
./source/signal_check_time.d \
./source/signal_control_heart.d \
./source/signal_report.d \
./source/signal_timeout_check.d \
./source/web_control_heart.d \
./source/web_message_recv.d \
./source/web_online_heart.d 


# Each subdirectory must supply rules for building sources it contributes
source/%.o: ../source/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -I/usr/include/oracle/12.1/client64 -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


