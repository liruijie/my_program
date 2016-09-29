#include "ExternPlatform.h"

#include <iostream>
#include <string.h>
#include <occi.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>

#include "occi_opration.h"
#include "public_data.h"
#include "web_message_recv.h"
#include "signal_report.h"



#define API_Port      10010 
extern int signal_report_timeout;
extern char transmitIP[20];
int sockfd_tj;
Statement *st_tj;
ResultSet *rs_tj;
Environment *env_tj;
Connection *conn_parsing_tj;
struct sockaddr_in myaddr,Remote_Addr;

void TianJin_API()
{
	
	//struct sockaddr SA;
	char recv_buf[100];
	int res;
	printf("new pthread, file=%s,func=%s,line=%d\n\n",__FILE__,__FUNCTION__,__LINE__);
	if ((sockfd_tj = socket(PF_INET, SOCK_DGRAM, 0)) < 0)
	{
		perror("fail to socket");
		exit(-1);
	}
	   // �����׽���ѡ������ַʹ�ô���  
    	int on=1;  
   	 if((setsockopt(sockfd_tj,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(on)))<0)  
   	 {  
      	  	perror("setsockopt failed");  
       		 exit(EXIT_FAILURE);  
    	} 

	
	bzero(&myaddr, sizeof(myaddr));
	myaddr.sin_family = PF_INET;
	myaddr.sin_port = htons(API_Port);
	myaddr.sin_addr.s_addr = INADDR_ANY;
	bzero(&Remote_Addr, sizeof(Remote_Addr));
	Remote_Addr.sin_family = PF_INET;
	Remote_Addr.sin_port = htons(10020);
	Remote_Addr.sin_addr.s_addr = inet_addr(transmitIP);

	if ( bind( sockfd_tj, (struct sockaddr *)&myaddr, sizeof(myaddr) ) < 0 )
	{
		perror("fail to bind");
		exit(-1);
	}
	/*�������ݿ�*/
		
	env_tj = Environment::createEnvironment(Environment::OBJECT);  
	
	string username = oracle_username;  
	string userpass = oracle_password;  
	string connstr = oracle_connectstring; 

	while(1)
	{
		try
		{
			conn_parsing_tj	= env_tj->createConnection(username, userpass, connstr);  
		    	 if(conn_parsing_tj == NULL)  
		    	{  
		  	  cout << "access oracle failed..." << endl;  
		   	 }  
		    
			st_tj = conn_parsing_tj->createStatement(); 
			st_tj->setAutoCommit (TRUE);
			break;
		}
		catch (SQLException &sqlExcp)
	   	{
	   		env_tj->terminateConnection(conn_parsing_tj);  
			Environment::terminateEnvironment(env_tj);
		      printf("parsing_signal_report_alarm   crart oracle falil!!\n");
	   	}
		sleep(1);
	}
	pthread_t TJ_report_data;
	res = pthread_create(&TJ_report_data,NULL,Report_Data,NULL);
	if(res != 0){
		perror("Fail to create pthread");
		exit(EXIT_FAILURE);
	}

	socklen_t Addr_Len = sizeof(Remote_Addr);
	while(1)
	{
		struct timeval 	timeVal;
		fd_set  fdSet;
		int ret;

		timeVal.tv_sec = signal_report_timeout;
		timeVal.tv_usec = 0;

		FD_ZERO(&fdSet);
		FD_SET(sockfd_tj, &fdSet);

		ret = select((int) sockfd_tj + 1, &fdSet,NULL, NULL, &timeVal);
		if(ret == 0)                     //��ʱ
		{
			
			//printf("API Port timeout\n");
		
		}
		if(ret)                //������
		{
			int num = recvfrom(sockfd_tj, recv_buf, 512, 0, (struct sockaddr *)&Remote_Addr, &Addr_Len);
/*
			for(i=0 ; i<num ; i++)
			{
				printf("0x%02X  ",recv_buf[i]);
			}
			printf("\n");       
 */
			if(num < 14)
			{
				printf("chang du cuo wu,%d\n",num);
				continue;
			}
			handle_receive_data(recv_buf);
			
		}
	}
}


int handle_receive_data( char *rcv_buf)
{
	int ret;
	unsigned char SetReply[17]={0x7E,0x00,0x0F,0x00,0x01,0xFF,0xFF,0x00,0x00,0x40,0xCC,0x10,0x00,0x00,0x00,0x00,0x7D};

	if( check_buf( (unsigned char *)rcv_buf) == -1 )
	{
		return 0;
	}
	switch((unsigned char )rcv_buf[10])
	{
		case 0x2B:
				{
					Handle_Data_0x2B(rcv_buf);
					break;
				}
		case 0xCA:
				{
					Handle_Data_0xCA(rcv_buf);
					break;
				}
		case 0xCC:
				{
					SetReply[7] = rcv_buf[7];
					SetReply[8] = rcv_buf[8];
					ret = UpdateSignalParameter((((unsigned char)rcv_buf[7]<<8)+(unsigned char)rcv_buf[8]));
					SetReply[12] = ret;
					SetReply[15]= buf_check_num(SetReply);
					sendto(sockfd_tj, SetReply, 17, 0, (struct sockaddr*)&Remote_Addr, sizeof(Remote_Addr));
					break;
				}
		default:
			break;
	}
	return 1;
}


void Handle_Data_0x2B(char *rcv_buf)
{
	char sql[1000];
	unsigned char reply_buf[100];
	int signal_id  = rcv_buf[7]*256 + rcv_buf[8];
	int signal_num = check_signal_id(signal_id);
	reply_buf[0] = 0x7E;
	reply_buf[1] = 0x00;
	reply_buf[2] = 0x0D;
	reply_buf[3] = rcv_buf[3];
	reply_buf[4] = rcv_buf[4];
	reply_buf[5] = rcv_buf[5];
	reply_buf[6] = rcv_buf[6];
	reply_buf[7] = rcv_buf[7];
	reply_buf[8] = rcv_buf[8];
	reply_buf[9] = 0x30;
	reply_buf[10] = 0x2B;
	reply_buf[11] = rcv_buf[11];             //���ݱ��
	reply_buf[12] = 0x00;                    //����״̬  ��ʼ��Ϊ0x00
	reply_buf[13] = 0x00;      				//У��    ��ʼ��Ϊ0x00
	reply_buf[14] = 0x7D;
	
	if(signal_num == -1)
	{
		printf("signal_id is invalid");
		reply_buf[12] = 0x01;    
		reply_buf[13] = buf_check_num(reply_buf);
		sendto(sockfd_tj, reply_buf, 15, 0, (struct sockaddr*)&Remote_Addr, sizeof(Remote_Addr));
		return ;
	}
	if(rcv_buf[11] == 0)     //��һ����Ϣ����
	{
	
		
		sprintf(sql,"update ITC_CFG_PLAN set plan_record = %d,phase_diff_time  = %d where signal_id = %d and plan_id = %d",rcv_buf[12],rcv_buf[13]*256+rcv_buf[14],signal_id,signal_info_data[signal_num].cur_plan);
		try
		{
			st_tj->setSQL(sql);
			st_tj->executeUpdate();
		}
		catch (SQLException& ex)
		{
			
			//return ;
		}
       //����ʱ���
		if(rcv_buf[12] == 0)        //��Э��  ����Ϊ��ʱ��ģʽ
		{
			sprintf(sql,"update itc_cfg_time_period t set t.run_mode=33 where t.signal_id=%d and t.plan_id= %d",signal_id,signal_info_data[signal_num].cur_plan);
		}
		else       //����ΪЭ��ģʽ
		{
			sprintf(sql,"update itc_cfg_time_period t set t.run_mode=35 where t.signal_id=%d and t.plan_id= %d",signal_id,signal_info_data[signal_num].cur_plan);
		}
		try
		{
			st_tj->setSQL(sql);
			st_tj->executeUpdate();
		}
		catch (SQLException& ex)
		{
			
			//return ;
		}
	}
	else                   //��������������
	{
		if(rcv_buf[11]<17)          //��֤������ȷ��
		{
			int stage_time = rcv_buf[13]*256+rcv_buf[14];
			if(stage_time != 0)         //�׶�ʱ������Ч��ֵ
			{
				sprintf(sql,"update itc_cfg_plan_chain set stage_time = %d where signal_id = %d and plan_id = %d and stage_order = %d",rcv_buf[13]*256+rcv_buf[14],signal_id,signal_info_data[signal_num].cur_plan,rcv_buf[12]); 
				try
				{
					st_tj->setSQL(sql);
					st_tj->executeUpdate();
				}
				catch (SQLException& ex)
				{	
					//return -1;
				}
			}
			if(rcv_buf[11] == 16)     //���һ�����ñ��� ��ʼ�·�����
			{
				reply_buf[12] = UpdateSignalParameter(signal_id);
			}
		}
	}
	
	reply_buf[13] = buf_check_num(reply_buf);
	sendto(sockfd_tj, reply_buf, 15, 0, (struct sockaddr*)&Remote_Addr, sizeof(Remote_Addr));
	
}
void Handle_Data_0xCA(char *rcv_buf)
{
	unsigned char reply_buf[100];
	int signal_id  = rcv_buf[7]*256 + rcv_buf[8];
	int signal_num = check_signal_id(signal_id);
	reply_buf[0] = 0x7E;
	reply_buf[1] = 0x00;
	reply_buf[2] = 0x0D;
	reply_buf[3] = rcv_buf[3];
	reply_buf[4] = rcv_buf[4];
	reply_buf[5] = rcv_buf[5];
	reply_buf[6] = rcv_buf[6];
	reply_buf[7] = rcv_buf[7];
	reply_buf[8] = rcv_buf[8];
	reply_buf[9] = 0x30;
	reply_buf[10] = 0xCA;
	reply_buf[11] = rcv_buf[11];			 //���ݱ��
	reply_buf[12] = 0x00;					 //����״̬  ��ʼ��Ϊ0x00
	reply_buf[13] = 0x00;					//У��	  ��ʼ��Ϊ0x00
	reply_buf[14] = 0x7D;

	
	if(signal_num == -1)
	{
		printf("signal_id is invalid");
		reply_buf[12] = 0x01;    
		reply_buf[13] = buf_check_num(reply_buf);
		sendto(sockfd_tj, reply_buf, 15, 0, (struct sockaddr*)&Remote_Addr, sizeof(Remote_Addr));
		return ;
	}
	signal_info_data[signal_num].TJ_API_Report_Real_Data_Flag = rcv_buf[12];
	reply_buf[13] = buf_check_num(reply_buf);
	sendto(sockfd_tj, reply_buf, 15, 0, (struct sockaddr*)&Remote_Addr, sizeof(Remote_Addr));
	
	
}
void *Report_Data(void *arg)
{
	int i;
	while(1)
	{
		for(i=0;i<signal_number;i++)
		{
			if( (signal_info_data[i].signal_state == 1))// && (signal_info_data[i].TJ_API_Report_Real_Data_Flag == 1 ))          //�źŻ����� �����ϱ��ı�־λ������
			{
				//printf("report id = %d ,data length = %d",signal_info_data[i].signal_id,sizeof(signal_info_data[i].Real_Data_Backup));
				sendto(sockfd_tj, signal_info_data[i].Real_Data_Backup, signal_info_data[i].Real_Data_Len, 0, (struct sockaddr*)&Remote_Addr, sizeof(Remote_Addr));
				if(signal_info_data[i].fault_flag == true)
				{
					sendto(sockfd_tj, signal_info_data[i].fault_backup, signal_info_data[i].fault_data_len, 0, (struct sockaddr*)&Remote_Addr, sizeof(Remote_Addr));
					signal_info_data[i].fault_flag = false;
				}
				if(signal_info_data[i].volume_flag == true)
				{
					sendto(sockfd_tj, signal_info_data[i].volume_backup, signal_info_data[i].volume_data_len , 0, (struct sockaddr*)&Remote_Addr, sizeof(Remote_Addr));
					signal_info_data[i].volume_flag = false;
				}
				if(signal_info_data[i].occu_flag == true)
				{
					sendto(sockfd_tj, signal_info_data[i].occu_backup, signal_info_data[i].occu_data_len , 0, (struct sockaddr*)&Remote_Addr, sizeof(Remote_Addr));
					signal_info_data[i].occu_flag = false;
				}
			}
		}
		usleep(800*1000);
	}
	return (void *)0;
}

int UpdateSignalParameter(int SignalID)
{
	int i,ret;
	char data_buf[20];
	memset(data_buf,0,20);
	data_buf[7] = (SignalID & 0XFF00)>>8;
	data_buf[8] = (SignalID & 0xFF);
	data_buf[9] = 0x20;
	for(i=1;i<=18;i++)
	{
		switch(i)
		{
			case 1:
				data_buf[10] = 0x01;
				break;
			case 2:
				data_buf[10] = 0x20;
				break;
			case 3:
				data_buf[10] = 0x21;
				break;
			case 4:
				data_buf[10] = 0x22;
				break;
			case 5:
				data_buf[10] = 0x23;
				break;
			case 6:
				data_buf[10] = 0x25;
				break;
			case 7:
				data_buf[10] = 0x26;
				break;
			case 8:
				data_buf[10] = 0x27;
				break;
			case 9:
				data_buf[10] = 0x28;
				break;
			case 10:
				data_buf[10] = 0x29;
				break;
			case 11:
				data_buf[10] = 0x2A;
				break;
			case 12:
				data_buf[10] = 0x2B;
				break;
			case 13:
				data_buf[10] = 0x2C;
				break;
			case 14:
				data_buf[10] = 0x2D;
				break;
			case 15:
				data_buf[10] = 0x2E;
				break;
			case 16:
				data_buf[10] = 0x2F;
				break;
			case 17:
				data_buf[10] = 0x30;
				break;
			case 18:
				data_buf[10] = 0x31;
				break;
				/*
			case 19:
				data_buf[10] = 0x01;
				break;
			case 20:
				data_buf[10] = 0x01;
				break;
			case 21:
				data_buf[10] = 0x01;
				break;
				*/
			default:
			{
				//memset(data_buf,0,20);
				return 0x01;
			}
		}
		ret = revice_web_message_opretion((unsigned char *)data_buf);
		if(ret == -1 ||ret == 10 )
		{
			/*shi bai*/
			return 0x01;
		}
	}
	return 0x00;
}
















