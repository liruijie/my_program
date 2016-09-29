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
#include <unistd.h>
#include "/home/ITC100/signal_report.h"
#include "/home/ITC100/occi_opration.h"
#include "/home/ITC100/public_data.h"
#include "/home/ITC100/web_control_heart.h"
#include "/home/ITC100/signal_control_heart.h"
#include "/home/ITC100/web_message_recv.h"

using namespace std;
using namespace oracle::occi;

#define BUFF_NUM  512

int flag = 0;
int num_i;

/*************************************************** 
 * 函数名：send_signal_control_heart
 * 功能描述：发送到信号机的控制心跳
 *被访问的表：
 *被修改的表：
* 参数说明：无
*返回值:  无
***************************************************/

void send_signal_control_heart()
{
	pthread_t pth_signal_control_heart[1000];
	

	int i,res;
	
	printf("new pthread,file=%s,func=%s,line=%d\n",__FILE__,__FUNCTION__,__LINE__);	
	
	for(num_i= 1; num_i <signal_number;num_i++ )
	{
		int * zz = &i;

		res = pthread_create(&(pth_signal_control_heart[num_i]),NULL,signal_control_heart,(void*) zz);
		if(res != 0)
		{
			printf("!!!!!!!!!!!!!!!!!! shi bai\n");
			//perror("Fail to create pthread");
			//exit(EXIT_FAILURE);
		}
		while(flag == 0)
		{
			printf("creat signal control heart pthread for signal_id = %d\n",signal_info_data[num_i].signal_id);
			usleep(100000);
		}
		flag = 0;
	}
	printf("\n");
	while(1)
	{
		sleep(1000);
	}
}

void *signal_control_heart(void *arg)
{

//	int sig_num =  *((int *)arg);
	int sig_num  = num_i;

	flag =1;
//	printf("@@@@@@@@@@sig_num = %d\n",sig_num);
	int sockfd_recv;
	unsigned char send_signal_con_heat_buf[14] ;
	struct sockaddr_in signal_addr;
	 struct sockaddr SA;
	socklen_t peerlen;
	
	send_signal_con_heat_buf[0] = 0x7e;
	send_signal_con_heat_buf[1] = 0x00;
	send_signal_con_heat_buf[2] = 0x0c;
	send_signal_con_heat_buf[3] = 0x00;
	send_signal_con_heat_buf[4] = 0x01;
	send_signal_con_heat_buf[5] = 0x00;
	send_signal_con_heat_buf[6] = 0x00;
	send_signal_con_heat_buf[7] = (signal_info_data[sig_num ].signal_id >> 8) ;
	send_signal_con_heat_buf[8] = (signal_info_data[sig_num ].signal_id & 0xff);
	if(strcmp(signal_info_data[sig_num ].device_type,"TC100")== 0)
		send_signal_con_heat_buf[9] = 0x10;
	else if(strcmp(signal_info_data[sig_num ].device_type,"ITC100")== 0)
		send_signal_con_heat_buf[9] = 0x20;
	send_signal_con_heat_buf[10] = 0xF0;
	if(strcmp(signal_info_data[sig_num ].device_type,"TC100")== 0)
		send_signal_con_heat_buf[11] = 0x00;
	else if(strcmp(signal_info_data[sig_num ].device_type,"ITC100")== 0)
		send_signal_con_heat_buf[11] = 0x01;
	send_signal_con_heat_buf[12] = buf_check_num(send_signal_con_heat_buf);
	send_signal_con_heat_buf[13] = 0x7d;

	
	
	if ((sockfd_recv = socket(PF_INET, SOCK_DGRAM, 0)) < 0)
	{
		perror("fail to socket bind");
		//return -1;
		//exit(-1);
	}
	   // 设置套接字选项避免地址使用错误  
    	int on=1;  
   	 if((setsockopt(sockfd_recv,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(on)))<0)  
   	 {  
      	  	perror("setsockopt failed");  
       		 exit(EXIT_FAILURE);  
    	} 


#if 0
	struct timeval tv_out;
       tv_out.tv_sec = 5;//等待5秒设置为0则阻塞
       tv_out.tv_usec = 0;
	setsockopt(sockfd_recv,SOL_SOCKET,SO_RCVTIMEO,&tv_out, sizeof(tv_out));
#endif


//	printf("id = %d , IP = %s  port = %d\n",signal_info_data[sig_num ].signal_id,signal_info_data[sig_num ].signal_ip,signal_info_data[sig_num ].signal_port);
	bzero(&signal_addr, sizeof(signal_addr));
	signal_addr.sin_family = PF_INET;
	
	signal_addr.sin_port = htons(signal_info_data[sig_num ].signal_port);
	signal_addr.sin_addr.s_addr = inet_addr(signal_info_data[sig_num ].signal_ip);

	while(1)
	{
#if 1	
//printf(">> signal = %d,d1 flsg = %d\n",signal_info_data[sig_num].signal_control_flag,signal_info_data[sig_num].control_d1_flag);
		if((signal_info_data[sig_num].signal_control_flag == 1) ||(signal_info_data[sig_num].control_d1_flag == 1))
		{
			sendto(sockfd_recv, send_signal_con_heat_buf, sizeof(send_signal_con_heat_buf), 0, (struct sockaddr*)&signal_addr, sizeof(signal_addr));
			signal_info_data[sig_num].control_heart_flag = 1;
		}
		else if(signal_info_data[sig_num].signal_control_flag == 2)
		{
			signal_info_data[sig_num].signal_control_heart_time --;
	//		printf(" qinwu shengyu shijian  = %d\n",signal_info_data[sig_num].signal_control_heart_time);
			if(signal_info_data[sig_num].signal_control_heart_time > 0)
			{
				sendto(sockfd_recv, send_signal_con_heat_buf, sizeof(send_signal_con_heat_buf), 0, (struct sockaddr*)&signal_addr, sizeof(signal_addr));
			}
			else
			{
				signal_info_data[sig_num].signal_control_heart_time = 0;
			//	signal_info_data[sig_num].signal_control_flag = signal_info_data[sig_num].signal_last_control_flag;
			}
		}
		else if(signal_info_data[sig_num].signal_control_flag == 3)
		{
			sendto(sockfd_recv, send_signal_con_heat_buf, sizeof(send_signal_con_heat_buf), 0, (struct sockaddr*)&signal_addr, sizeof(signal_addr));
		}
			
		//usleep(500000);
		sleep(1);
#endif		
	}
	

	
}


