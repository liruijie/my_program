#include <iostream>
#include <string.h>
#include <occi.h>
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include "signal_report.h"
#include "occi_opration.h"
#include "public_data.h"
#include "web_control_heart.h"
#include "signal_check_time.h"
#include "web_message_recv.h"



using namespace std;
using namespace oracle::occi;

void *function_signal_check_time(void )
{
	int i;

	time_t rawtime;
	struct tm * timeinfo;
	char recv_buf[512];

	int sockfd_recv;

	struct sockaddr_in signal_addr;

#if 1	
	if ((sockfd_recv = socket(PF_INET, SOCK_DGRAM, 0)) < 0)
	{
		perror("fail to socket bind");
		
		//exit(-1);
	}
	   // �����׽���ѡ������ַʹ�ô���  
    	int on=1;  
   	 if((setsockopt(sockfd_recv,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(on)))<0)  
   	 {  
      	  	perror("setsockopt failed");  
       		 exit(EXIT_FAILURE);  
    	} 

#endif
#if 1
	struct timeval tv_out;
       tv_out.tv_sec = 5;//�ȴ�5������Ϊ0������
       tv_out.tv_usec = 0;
	setsockopt(sockfd_recv,SOL_SOCKET,SO_RCVTIMEO,&tv_out, sizeof(tv_out));
#endif

	unsigned char F1_date[21],F1_date_itc100[17];
	/*Уʱ����*/
	F1_date[0] = 0x7e;
	F1_date[1] = 0x00;
	F1_date[2] = 0x13;
	F1_date[3] = 0x00;
	F1_date[4] = 0x01;
	F1_date[5] = 0x00;
	F1_date[6] = 0x00;
	F1_date[7] = 0x00;
	F1_date[8] = 0x00;
	F1_date[9] = 0x20;
	F1_date[10] = 0xf1;
	F1_date[11] = 0x00;
	F1_date[12] = 0x00;
	F1_date[13] = 0x00;
	F1_date[14] = 0x00;
	F1_date[15] = 0x00;
	F1_date[16] = 0x00;
	F1_date[17] = 0x00;
	F1_date[18] = 0x00;
	F1_date[19] = 0xff;
	F1_date[20] = 0x7d;
	
	F1_date_itc100[0] = 0x7e;
	F1_date_itc100[1] = 0x00;
	F1_date_itc100[2] = 0x0f;
	F1_date_itc100[3] = 0x00;
	F1_date_itc100[4] = 0x01;
	F1_date_itc100[5] = 0x00;
	F1_date_itc100[6] = 0x00;
	F1_date_itc100[7] = 0x00;
	F1_date_itc100[8] = 0x00;
	F1_date_itc100[9] = 0x20;
	F1_date_itc100[10] = 0xf1;
	F1_date_itc100[11] = 0x00;
	F1_date_itc100[12] = 0x00;
	F1_date_itc100[13] = 0x00;
	F1_date_itc100[14] = 0x00;
	F1_date_itc100[15] = 0x00;
	F1_date_itc100[16] = 0x7d;

printf("new pthread, file=%s,func=%s,line=%d\n\n",__FILE__,__FUNCTION__,__LINE__);	
	while(1)
	{
		time ( &rawtime );
		timeinfo = localtime ( &rawtime );
		rawtime = rawtime + (8 * 3600);
		/*��ʱ����Уʱ����*/
		for(i = 1; i <signal_number;i++ )
		{
			if(signal_info_data[i].signal_state == 1)
			{
				
				bzero(&signal_addr, sizeof(signal_addr));
				signal_addr.sin_family = PF_INET;

				signal_addr.sin_port = htons(signal_info_data[i].signal_port);
				signal_addr.sin_addr.s_addr = inet_addr(signal_info_data[i].signal_ip);

				if(strcmp(signal_info_data[i].device_type ,"ITC100") == 0)
					{
				F1_date_itc100[7] = (signal_info_data[i].signal_id  >> 8) & 0xff;
				F1_date_itc100[8] = signal_info_data[i].signal_id  & 0xff;
				F1_date_itc100[11]= ((unsigned int)rawtime &0xff000000)>> 24;
				F1_date_itc100[12]= ((unsigned int)rawtime &0x00ff0000)>> 16;
				F1_date_itc100[13]= ((unsigned int)rawtime &0x0000ff00)>> 8;
				F1_date_itc100[14]= ((unsigned int)rawtime &0x000000ff);
				F1_date_itc100[15] =  buf_check_num(F1_date_itc100);;

				printf("check time \n");
				sendto(sockfd_recv, F1_date_itc100, sizeof(F1_date_itc100), 0, (struct sockaddr*)&signal_addr, sizeof(signal_addr));
				}

				else if (strcmp(signal_info_data[i].device_type ,"TC100") == 0)
				{
				F1_date[7] = (signal_info_data[i].signal_id  >> 8) & 0xff;
				F1_date[8] = signal_info_data[i].signal_id  & 0xff;
				F1_date[11] = (1900+timeinfo->tm_year) >> 8;
				F1_date[12] = (1900+timeinfo->tm_year) & 0xff;
				F1_date[13] =  timeinfo->tm_mon + 1;
				F1_date[14] =  timeinfo->tm_mday;
				F1_date[15] =  timeinfo->tm_hour;
				F1_date[16] =  timeinfo->tm_min;
				F1_date[17] =  timeinfo->tm_sec;
				F1_date[18] =  F1_date[18]  +  (0x01 << timeinfo->tm_wday);
				F1_date[19] =  buf_check_num(F1_date);
				sendto(sockfd_recv, F1_date, sizeof(F1_date), 0, (struct sockaddr*)&signal_addr, sizeof(signal_addr));

				}
				
				recvfrom(sockfd_recv, recv_buf, 16, 0, NULL, NULL);
				//printf("check time ok!!!!!!\n");
					
			}
			
		}
		
		sleep(900);
	}
	
}

