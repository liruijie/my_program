CC = g++
CFlag = -O0 -g3 
LIBS =-L/usr/lib/oracle/12.1/client64/lib -lpthread -lm -locci -lnsl -lclntsh
INCLUDES =-I/usr/include/oracle/12.1/client64
OBJECTS =     main.o\
        external_query.o\
        occi_opration.o\
        parsing_report.o\
        parsing_report_alalrm.o\
        queue.o\
        signal_check_time.o\
        signal_control_heart.o\
        signal_report.o\
        signal_timeout_check.o\
        web_control_heart.o\
        web_message_recv.o\
        web_online_heart.o 

ITC100:$(OBJECTS)
	$(CC) $(CFlag) -o ITC100 $(OBJECTS) $(INCLUDES) $(LIBS)

main.o:main.cpp 
	$(CC) $(CFlag) -c main.cpp $(INCLUDES) $(LIBS)

ExternPlatform.o:ExternPlatform.cpp 
	$(CC) $(CFlag) -c ExternPlatform.cpp $(INCLUDES) $(LIBS)

external_query.o:external_query.cpp
	$(CC) $(CFlag) -c external_query.cpp $(INCLUDES) $(LIBS)

occi_opration.o:occi_opration.cpp
	$(CC) $(CFlag) -c occi_opration.cpp $(INCLUDES) $(LIBS)

parsing_report.o:parsing_report.cpp
	$(CC) $(CFlag) -c parsing_report.cpp $(INCLUDES) $(LIBS)

parsing_report_alalrm.o:parsing_report_alalrm.cpp
	$(CC) $(CFlag) -c parsing_report_alalrm.cpp $(INCLUDES) $(LIBS)

queue.o:queue.cpp
	$(CC) $(CFlag) -c queue.cpp $(INCLUDES) $(LIBS)

signal_check_time.o:signal_check_time.cpp
	$(CC) $(CFlag) -c signal_check_time.cpp $(INCLUDES) $(LIBS)

signal_control_heart.o:signal_control_heart.cpp
	$(CC) $(CFlag) -c signal_control_heart.cpp $(INCLUDES) $(LIBS)

signal_report.o:signal_report.cpp
	$(CC) $(CFlag) -c signal_report.cpp $(INCLUDES) $(LIBS)

signal_timeout_check.o:signal_timeout_check.cpp
	$(CC) $(CFlag) -c signal_timeout_check.cpp $(INCLUDES) $(LIBS)

web_control_heart.o:web_control_heart.cpp
	$(CC) $(CFlag) -c web_control_heart.cpp $(INCLUDES) $(LIBS)

web_message_recv.o:web_message_recv.cpp
	$(CC) $(CFlag) -c web_message_recv.cpp $(INCLUDES) $(LIBS)

web_online_heart.o:web_online_heart.cpp
	$(CC) $(CFlag) -c web_online_heart.cpp $(INCLUDES) $(LIBS)



.PHONY : clean
clean :
	rm -f  ITC100 $(OBJECTS)
