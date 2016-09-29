################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/ControlCenterCommt.cpp \
../src/ControlSignal.cpp \
../src/ITC_Server.cpp \
../src/KeepConnectAlive.cpp \
../src/Operation_Get.cpp \
../src/Operation_Set.cpp \
../src/ReportRealStatus.cpp \
../src/XmlRequest.cpp \
../src/main.cpp \
../src/md5.cpp \
../src/oracle.cpp \
../src/publicfunction.cpp \
../src/xml.cpp 

OBJS += \
./src/ControlCenterCommt.o \
./src/ControlSignal.o \
./src/ITC_Server.o \
./src/KeepConnectAlive.o \
./src/Operation_Get.o \
./src/Operation_Set.o \
./src/ReportRealStatus.o \
./src/XmlRequest.o \
./src/main.o \
./src/md5.o \
./src/oracle.o \
./src/publicfunction.o \
./src/xml.o 

CPP_DEPS += \
./src/ControlCenterCommt.d \
./src/ControlSignal.d \
./src/ITC_Server.d \
./src/KeepConnectAlive.d \
./src/Operation_Get.d \
./src/Operation_Set.d \
./src/ReportRealStatus.d \
./src/XmlRequest.d \
./src/main.d \
./src/md5.d \
./src/oracle.d \
./src/publicfunction.d \
./src/xml.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -I/usr/local/include/libxml2 -I/usr/include/oracle/12.1/client64 -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


