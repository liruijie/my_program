#include <iostream>
#include <string.h>
#include <occi.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/types.h>
#include <pthread.h>
#include "/home/ITC100/signal_report.h"
#include "/home/ITC100/public_data.h"
#include "/home/ITC100/parsing_report.h"
#include "/home/ITC100/parsing_report_alalrm.h"
#include "/home/ITC100/signal_timeout_check.h"
#include "/home/ITC100/occi_opration.h"
#include "/home/ITC100/web_control_heart.h"
#include "/home/ITC100/web_message_recv.h"
#include "/home/ITC100/web_online_heart.h"
#include "/home/ITC100/signal_control_heart.h"
#include "/home/ITC100/signal_check_time.h"
#include "/home/ITC100/external_query.h"



using namespace oracle::occi;

using namespace std;


void *pthread_function_recv(void *arg);
void *pthread_function_parsing_report(void *arg);
void *pthread_function_parsing_report_alarm(void *arg);
void *pthread_function_check_signal_time_out(void *arg);
void *pthread_function_web_control_heart_recv(void *arg);
void *pthread_function_web_online_heart_recv(void *arg);
void *pthread_function_web_message_recv(void *arg);
void *pthread_function_signal_control_heart(void *arg);
void *pthread_function_signal_control_state(void *arg);
void *pthread_function_signal_check_time(void *arg);


void *pthread_external_query(void *arg);

int get_config( char* fileName);
extern int signal_report_timeout;

string oracle_username ;
string oracle_password ;
string oracle_connectstring;


int  main()
{
	int sleep_num = 2;
	
	pthread_t signal_report;
	pthread_t parsing_signal_report;
	pthread_t parsing_signal_report_alarm;
	pthread_t check_signal_time_out;
	pthread_t web_control_heart_recv;
	pthread_t web_online_heart_recv;
	pthread_t web_message_recv;
	pthread_t signal_control_heart;
	pthread_t signal_control_state;
	pthread_t signal_check_time;

	/*外部查询*/
	pthread_t external_query;
	int res;

	char config_file_path[100] ;

	sprintf(config_file_path,"%s", "/home/ITC100/config");
	get_config(config_file_path);

//	printf("oracle_username = %s,oracle_password = %s,oracle_connectstring = %s\n",oracle_username,oracle_password,oracle_connectstring);
	cout <<"name = " << oracle_username     <<"oracle_password = " <<  oracle_password <<   "oracle_connectstring = " << oracle_connectstring << endl;
//	return 0;

	
	res =  occi_open();
	printf(" cooi_open  is  %d\n",res);

	res = occi_singal_Init();
	printf(" occi_singal_Init  is  %d\n",res);
	
	occi_singal_Init_alalrm();

	printf("occi_singal_Init_alalrm\n");
	
	occi_singal_Init_control_heart();

	printf("**********************************\n");
	for(res = 1; res <signal_number;res++)
	{
		//printf("ID = %d   unit_id= %d  l_online_alarm_time = %s ,signal_info_data[i].signal_state = %d   last_time = %d\n",signal_info_data[res].signal_id,signal_info_data[res].unit_id,signal_info_data[res].l_online_alarm_time,signal_info_data[res].signal_state,signal_info_data[res].last_time);	
		printf(" res = %d,ID = %d   unit_id %d\n",res,signal_info_data[res].signal_id,signal_info_data[res].unit_id);	

	}
	printf("**********************************\n");

	//return 0;
	

	/*初始化队列*/
	 sq = queue_create();
	
	printf("init ok !!!!!!!!!!!\n");

	/**************接受来自信号机的数据*******************************************************/
	res = pthread_create(&signal_report,NULL,pthread_function_recv,NULL);
	if(res != 0){
		perror("Fail to create pthread");
		exit(EXIT_FAILURE);
	}

	sleep(sleep_num);

	//处理报警数据和 流量、占有率数据
	res = pthread_create(&parsing_signal_report_alarm,NULL,pthread_function_parsing_report_alarm,NULL);
	if(res != 0){
		perror("Fail to create pthread");
		exit(EXIT_FAILURE);
	}

	sleep(sleep_num);

	res = pthread_create(&check_signal_time_out,NULL,pthread_function_check_signal_time_out,NULL);
	if(res != 0){
		perror("Fail to create pthread");
		exit(EXIT_FAILURE);
	}

	sleep(sleep_num);

	res = pthread_create(&web_control_heart_recv,NULL,pthread_function_web_control_heart_recv,NULL);
	if(res != 0){
		perror("Fail to create pthread");
		exit(EXIT_FAILURE);
	}

	sleep(sleep_num);
#if 1
	res = pthread_create(&web_message_recv,NULL,pthread_function_web_message_recv,NULL);
	if(res != 0){
		perror("Fail to create pthread");
		exit(EXIT_FAILURE);
	}

	sleep(sleep_num);

	res = pthread_create(&web_online_heart_recv,NULL,pthread_function_web_online_heart_recv,NULL);
	if(res != 0){
		perror("Fail to create pthread");
		exit(EXIT_FAILURE);
	}

	sleep(sleep_num);


	res = pthread_create(&signal_control_heart,NULL,pthread_function_signal_control_heart,NULL);
	if(res != 0){
		perror("Fail to create pthread");
		exit(EXIT_FAILURE);
	}
	
#endif

	sleep(sleep_num);
	res = pthread_create(&parsing_signal_report,NULL,pthread_function_parsing_report,NULL);
	if(res != 0){
		perror("@@@@@@@@@@Fail to create pthread !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
		exit(EXIT_FAILURE);
	}

	sleep(10);
	res = pthread_create(&signal_control_state,NULL,pthread_function_signal_control_state,NULL);
	if(res != 0){
		perror("Fail to create pthread");
		exit(EXIT_FAILURE);
	}
#if 1
	sleep(sleep_num);
	res = pthread_create(&signal_check_time,NULL,pthread_function_signal_check_time,NULL);
	if(res != 0){
		perror("Fail to create pthread");
		exit(EXIT_FAILURE);
	}

#endif

sleep(sleep_num);

#if 1
	res = pthread_create(&external_query,NULL,pthread_external_query,NULL);
	if(res != 0){
		perror("Fail to create pthread");
		exit(EXIT_FAILURE);
	}

#endif

 	while(1)
 	{
 		sleep(1000);
 	}
  	return 0;
}

void *pthread_function_recv(void *arg)
{
	receive_signal_report();
	pthread_exit(NULL);
}

void *pthread_function_parsing_report(void *arg)
{
	parsing_signal_report();
	pthread_exit(NULL);
}

void *pthread_function_parsing_report_alarm(void *arg)
{
	parsing_signal_report_alarm();
	pthread_exit(NULL);
}

void *pthread_function_check_signal_time_out(void *arg)
{
	signal_timeout_check();
	pthread_exit(NULL);
}

void *pthread_function_web_control_heart_recv(void *arg)
{
	revice_web_control_heart();
	pthread_exit(NULL);
}
void *pthread_function_web_message_recv(void *arg)
{
	revice_web_message();
	pthread_exit(NULL);
	
}

void *pthread_function_web_online_heart_recv(void *arg)
{
	revice_web_online_heart();
	pthread_exit(NULL);
}

void *pthread_function_signal_control_heart(void *arg)
{
	send_signal_control_heart();
	pthread_exit(NULL);
}

void *pthread_function_signal_control_state(void *arg)
{
		signal_control_state_timeout_check();
		pthread_exit(NULL);
		
}

void *pthread_function_signal_check_time(void *arg)
{
		function_signal_check_time();
		pthread_exit(NULL);
		
}

void *pthread_external_query(void *arg)
{
		int i ;
#if 0
		for(i = 0; i < 100;i++)
		{
			printf("i = %d\n",i);
			sleep(1);
		}
#endif		
			
		function_external_query();
		pthread_exit(NULL);
		
}


#if 1
/*得到oracle的用户名 密码和连接符*/
int get_config(char* fileName)
{
	FILE *fp = fopen(fileName,"r");
 	if( fp == NULL )
  	{
  		printf("!!! Can't open file %s\n",fileName);
    		return 0;
  	}
	char line[64];
  	char tmp[64];

	char aaa[50];

	bzero(aaa,sizeof(aaa));
  	fgets(line,64,fp);
	sscanf(line,"%[^=]=%s",tmp,aaa);
//	printf("%s\n",aaa);
	oracle_username = aaa;

	bzero(aaa,sizeof(aaa));
  	fgets(line,64,fp);
 	sscanf(line,"%[^=]=%s",tmp,aaa);
//	printf("%s\n",aaa);
	oracle_password = aaa;

	bzero(aaa,sizeof(aaa));
  	fgets(line,64,fp);
 	sscanf(line,"%[^=]=%s",tmp,aaa);
//	printf("%s\n",aaa);
	oracle_connectstring = aaa;
	
	bzero(aaa,sizeof(aaa));
  	fgets(line,64,fp);
 	sscanf(line,"%[^=]=%s",tmp,aaa);
	signal_report_timeout = atoi(aaa);
//	printf("%s\n",aaa);
	printf("signal report timeout = %d\n",signal_report_timeout);
	


	

  	fclose(fp);
  	return 0;
}
#endif

