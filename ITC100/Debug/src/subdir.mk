################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/external_query.cpp \
../src/main.cpp \
../src/occi_opration.cpp \
../src/parsing_report.cpp \
../src/parsing_report_alalrm.cpp \
../src/queue.cpp \
../src/signal_check_time.cpp \
../src/signal_control_heart.cpp \
../src/signal_report.cpp \
../src/signal_timeout_check.cpp \
../src/web_control_heart.cpp \
../src/web_message_recv.cpp \
../src/web_online_heart.cpp 

OBJS += \
./src/external_query.o \
./src/main.o \
./src/occi_opration.o \
./src/parsing_report.o \
./src/parsing_report_alalrm.o \
./src/queue.o \
./src/signal_check_time.o \
./src/signal_control_heart.o \
./src/signal_report.o \
./src/signal_timeout_check.o \
./src/web_control_heart.o \
./src/web_message_recv.o \
./src/web_online_heart.o 

CPP_DEPS += \
./src/external_query.d \
./src/main.d \
./src/occi_opration.d \
./src/parsing_report.d \
./src/parsing_report_alalrm.d \
./src/queue.d \
./src/signal_check_time.d \
./src/signal_control_heart.d \
./src/signal_report.d \
./src/signal_timeout_check.d \
./src/web_control_heart.d \
./src/web_message_recv.d \
./src/web_online_heart.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -I/usr/include/oracle/12.1/client64 -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


