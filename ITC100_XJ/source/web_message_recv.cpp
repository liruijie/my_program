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
#include <fcntl.h>
#include "signal_report.h"
#include "occi_opration.h"
#include "public_data.h"
#include "web_control_heart.h"
#include "web_message_recv.h"


using namespace std;
using namespace oracle::occi;

#define BUFF_NUM  512


int signal_report_timeout;

/*���Ʊ���*/	
unsigned char F1_date[21];
unsigned char F2_date[15];
unsigned char F3_date[14];
unsigned char C0_date[14];
unsigned char C1_date[14];
unsigned char C2_date[15];
unsigned char C3_date[15];
unsigned char C4_date[16];
unsigned char C5_date[16];
unsigned char C6_date[85];


/*���ñ���*/
unsigned char set_01_date[164 + 1];
unsigned char set_02_date[24];
unsigned char set_11_date[8][21];
unsigned char set_12_date[8][21];
unsigned char set_13_date[8][21];
unsigned char set_03_date[16][184+5];
unsigned char set_04_date[64][174];
unsigned char set_05_date[128][213];
unsigned char set_06_date[128][336];
unsigned char set_07_date[128][160];
unsigned char set_08_date[270];
unsigned char set_09_date[64][78];
unsigned char set_0a_date[28];
unsigned char set_0b_date[26];
unsigned char set_0c_date[26];
unsigned char set_14_date[24][87];
unsigned char set_0d_date[128][97];
unsigned char set_15_date[24][31];
unsigned char set_0e_date[128][97];
unsigned char set_0f_date[104][23];
unsigned char set_10_date[270];


struct plan_config
{
	unsigned char plan_week[2];
	unsigned char plan_record;
	unsigned char xianweicha_time[2];
	unsigned char plan_num;
	
	unsigned char plan_config_chin_id[16];
	unsigned char plan_config_chin_time[32];
	unsigned char plan_config_chin_type[16];
	unsigned char plan_config_chin_pede_green[16];
	unsigned char plan_config_chin_pede_green_flash[16];
	unsigned char plan_config_chin_pede_green_flashQ[16];
	unsigned char plan_config_chin_pede_red[16];
	
	/*�����ж�Ӧ�Ĵӱ��еķ�����*/
	int plan_config_id;
	int flag;
};
struct  sjbsdsz_info
{
	unsigned char time_num;
	unsigned char time_all_infomation[192];
};
struct diaodu_info
{
	unsigned char Ddb_type;
	unsigned char Ddb_priority;
	unsigned char Week_ddb_type;
	unsigned char Date_ddb_type_r[4];
	unsigned char Date_ddb_type_y[2];
	unsigned char New_ddb_type_n;
	unsigned char New_ddb_type_y[2];
	unsigned char New_ddb_type_r[4];
	unsigned char Time_table_number_1;
	
	
};
struct guduyueshu
{
	unsigned char date[64];
};

struct shield_stage_info
{
	unsigned char stage_id;
	unsigned char stage1_id[4];
	unsigned char stage1_type[4];
	unsigned char stage1_occ_detector[4][13];
	unsigned char stage1_occ_value[4];
	unsigned char stage1_occ_check[4];
	unsigned char stage1_occ_long[4];
#if 0
	unsigned char stage2_id;
	unsigned char stage2_type;
	unsigned char stage2_occ_detector[13];
	unsigned char stage2_occ_value;
	unsigned char stage2_occ_check;
	unsigned char stage2_occ_long;

	unsigned char stage3_id;
	unsigned char stage3_type;
	unsigned char stage3_occ_detector[13];
	unsigned char stage3_occ_value;
	unsigned char stage3_occ_check;
	unsigned char stage3_occ_long;

	unsigned char stage4_id;
	unsigned char stage4_type;
	unsigned char stage4_occ_detector[13];
	unsigned char stage4_occ_value;
	unsigned char stage4_occ_check;
	unsigned char stage4_occ_long;
#endif	
};

struct diaodu_info_and_time
{
	unsigned char Ddb_type;
	unsigned char Ddb_priority;
	unsigned char Week_ddb_type;
	unsigned char Date_ddb_type_r[4];
	unsigned char Date_ddb_type_y[2];

	unsigned char time_num;

	unsigned char Time_table_number_1;
	unsigned char Hour[24];
	unsigned char Minute[24];
	unsigned char SHIELD_ID[24];
	
};

struct vary_mark_info
{
	unsigned char chink_id[4];
	unsigned char chink_type[4];
	unsigned char chink_red[4];
	unsigned char chink_green[4];
	

};

struct diaodu_info_and_time_0e
{
	unsigned char Ddb_type;
	unsigned char Ddb_priority;
	unsigned char Week_ddb_type;
	unsigned char Date_ddb_type_r[4];
	unsigned char Date_ddb_type_y[2];

	unsigned char time_num;

	unsigned char Time_table_number_1;
	unsigned char Hour[24];
	unsigned char Minute[24];
	unsigned char VARY_MARK_ID[24];
	
};

struct  detector_info
{
	unsigned char Detector_type;
	unsigned char Flow_week[2];
	unsigned char Occ_week[2];
	unsigned char Warning_week[2];
	int  direction;

};


pthread_t recv_message[5000];
int recv_message_flag;

int sockfd;
struct web_info web_info_date;
socklen_t peerlen;

char sql[10000];
Statement *st;
ResultSet *rs ;
Environment *env;
Connection *conn_parsing ;

/*���ݿ����ӱ�־ 0 ���� -1 �쳣*/
int flag_sql_error;







void *pthread_function_revice_web_message_opration(void *arg);

/*�������ݿ����� �����������*/
void *pthread_function_flag_sql_error(void *arg);




/*************************************************** 
 * ��������revice_web_message
 * ��������������web�����ĸ��ֲ���
 *�����ʵı�
 *���޸ĵı�
* ����˵�����յ�����Ϣ
*����ֵ:  ��
***************************************************/
void revice_web_message()
{
	//printf("jin ru  jieshou  web xiancheng @@@@@@@@@@@\n");
	pthread_t flag_sql;
		
	recv_message_flag = 0;
	/*����UDP������*/

printf("new pthread, file=%s,func=%s,line=%d\n\n",__FILE__,__FUNCTION__,__LINE__);	
	struct sockaddr_in myaddr;

	
	if ((sockfd = socket(PF_INET, SOCK_DGRAM, 0)) < 0)
	{
		perror("fail to socket");
		exit(-1);
	}
	   // �����׽���ѡ������ַʹ�ô���  
    	int on=1;  
   	 if((setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(on)))<0)  
   	 {  
      	  	perror("setsockopt failed");  
       		 exit(EXIT_FAILURE);  
    	} 
//printf("file=%s,func=%s,line=%d\n",__FILE__,__FUNCTION__,__LINE__);		
	bzero(&myaddr, sizeof(myaddr));
	myaddr.sin_family = PF_INET;
	myaddr.sin_port = htons(10001);
	myaddr.sin_addr.s_addr = INADDR_ANY;
	//int bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
	if ( bind( sockfd, (struct sockaddr *)&myaddr, sizeof(myaddr) ) < 0 )
	{
		perror("fail to bind");
		exit(-1);
	}
//printf("file=%s,func=%s,line=%d\n",__FILE__,__FUNCTION__,__LINE__); 


	peerlen = sizeof(web_info_date.peeraddr);
#if 1
	/*�������ݿ�*/
		
	env = Environment::createEnvironment(Environment::OBJECT);  
//	string username = "atc";  
//    	string userpass = "atc";  
//    	string connstr = "192.168.1.24:1521/oracle"; 

	string username = oracle_username;  
    	string userpass = oracle_password;  
    	string connstr = oracle_connectstring; 

	

//printf("file=%s,func=%s,line=%d\n",__FILE__,__FUNCTION__,__LINE__);	
	
	while(1)
	{
		try
		{
			conn_parsing = env->createConnection(username, userpass, connstr); 
			 if(conn_parsing == NULL)  
		    {  
		  	  cout << "access oracle failed..." << endl;  
		   	 }  
		    
			st = conn_parsing->createStatement(); 
			st->setAutoCommit (TRUE);
			break;
		}
		catch (SQLException &sqlExcp)
	    {
			 env->terminateConnection(conn_parsing);  
			Environment::terminateEnvironment(env);  
			printf("creat init occi_singal_Init_control_heart  oracle fail!\n");
	    }
		sleep(1);
		
	}

	flag_sql_error = 0;
//printf("file=%s,func=%s,line=%d\n",__FILE__,__FUNCTION__,__LINE__);	

#endif
	int res = pthread_create(&flag_sql,NULL,pthread_function_flag_sql_error,NULL);
	if(res != 0)
	{
		perror("Fail to create pthread");
		exit(EXIT_FAILURE);
	}
	//printf("file=%s,line=%d\n",__FILE__,__LINE__);

	all_send_signal_message_info();
	
//printf("file=%s,func=%s,line=%d\n",__FILE__,__FUNCTION__,__LINE__); 

	while(1)
	{
		
	//	bzero(web_info_date.recv_buf,sizeof(web_info_date.recv_buf));
		int num = recvfrom(sockfd, web_info_date.recv_buf, BUFF_NUM, 0, (struct sockaddr *)&web_info_date.peeraddr, &peerlen);
	//	sendto(sockfd, web_info_date.recv_buf, BUFF_NUM, 0, (struct sockaddr *)&web_info_date.peeraddr, peerlen);

		printf("shoudao yi tiao shuju  from  web!!!!!!!!!!! \n");
	//	printf("recv from [%s:%d] : \n", inet_ntoa(web_info_date.peeraddr.sin_addr), ntohs(web_info_date.peeraddr.sin_port));
		printf("revice_web_message9 ==  %x 10 ==%x\n",web_info_date.recv_buf[9],web_info_date.recv_buf[10]);
		printf("revice_web_message recv_buf[13] =  %x \n",web_info_date.recv_buf[13]);

		int temp;
		for(temp = 0;temp< num; temp++)
			printf("%02x ",web_info_date.recv_buf[temp]);
		printf("\n");
		
		if(check_buf_web(web_info_date.recv_buf) == 0)
		{	
			try
			{
				int res = pthread_create(&recv_message[recv_message_flag],NULL,pthread_function_revice_web_message_opration,NULL);
				if(res != 0)
				{
					perror("Fail to create pthread");
					//exit(EXIT_FAILURE);
				}
				recv_message_flag++;
				
				if(recv_message_flag >= 4999)
				{
					recv_message_flag = 0;
				}
			}
			catch( int)
			{
				perror("Fail to create pthread");
			}
		}

		
	}
}

/*************************************************** 
 * ��������pthread_function_revice_web_message_opration
 * ��������������web�����ĸ��ֲ����̺߳���
 *�����ʵı�
 *���޸ĵı�
* ����˵�����յ�����Ϣ
*����ֵ:  ��
***************************************************/
void *pthread_function_revice_web_message_opration(void *arg)
{
			
	unsigned char l_buf[512] ;
	struct sockaddr_in  peeraddr;	
	int i ;
	for(i = 0; i < web_info_date.recv_buf[1] * 256 + web_info_date.recv_buf[2] + 2;i++)
	{
		l_buf[i] =  web_info_date.recv_buf[i];
	}
	//sprintf((char*)l_buf,"%s",(char*)web_info_date.recv_buf);
	//strcpy((char*)recv_buf,(char*)web_info_date.recv_buf);

	//memcpy(peeraddr,web_info_date.peeraddr,sizeof(web_info_date.peeraddr));
	peeraddr = web_info_date.peeraddr;

	
//	printf("pthread_function_revice_web_message_opration  web_info_date.recv_buf %x %x\n",web_info_date.recv_buf[9],web_info_date.recv_buf[10]);
//	printf("pthread_function_revice_web_message_opration  l_buf %x %x\n",l_buf[9],l_buf[10]);
	
	int res =  revice_web_message_opretion( l_buf);
	//int res ;
	printf("res = %d\n",res);
	l_buf[9] = 0x30;
	if(res == -1 ||res == 10 )
	{
		/*ʧ��*/
		l_buf[13] = 0x01;
	}
	else 
	{	
		/*�ɹ�*/
		l_buf[13] = 0x00;
	}
	l_buf[14] = buf_check_num(l_buf);
	//printf("recv from [%s:%d] : \n", inet_ntoa(web_info_date.peeraddr.sin_addr), ntohs(web_info_date.peeraddr.sin_port));
	sendto(sockfd, l_buf, 22, 0, (struct sockaddr *)&peeraddr, peerlen);

	/*��������*/
	if(res == 2)
	{
		sleep(3);
		system("/home/ITC100/restart.sh");
	}
	/*sql ������д���*/
	else if(res == 10 )
	{
		flag_sql_error = -1;
	}
	
	pthread_detach (pthread_self());
	pthread_exit(NULL);
}


/*************************************************** 
 * ��������revice_web_message_opretion
 * ��������������web�����ĸ��ֲ������ݴ���
 *�����ʵı�
 *���޸ĵı�
* ����˵�����յ�����Ϣ
*����ֵ:  ��
***************************************************/
int revice_web_message_opretion(   unsigned  char* buf )
{
	int sockfd_recv;

	struct sockaddr_in signal_addr;

	time_t rawtime;
	struct tm * timeinfo;
	//int sockfd_recv;
	unsigned char recv_buf[BUFF_NUM] ;
	//struct sockaddr_in signal_addr;


	int signal_num;
	unsigned int i ,j;
	int res;
	int lin_res_num;
	char lin_buf[100];
	char *result = NULL;
#if 1	
	if ((sockfd_recv = socket(PF_INET, SOCK_DGRAM, 0)) < 0)
	{
		perror("fail to socket bind");
		return -1;
		//exit(-1);
	}
	   // �����׽���ѡ������ַʹ�ô���  
    	int on=1;  
   	 if((setsockopt(sockfd_recv,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(on)))<0)  
   	 {  
      	  	perror("setsockopt failed");  
       		 exit(EXIT_FAILURE);  
    	} 
	//����ͨѶģʽΪ�첽ģʽ
	int set_flags = fcntl(sockfd_recv, F_GETFL);
	if( (set_flags<0)||((fcntl(sockfd_recv, F_SETFL, set_flags |O_ASYNC))<0) )
	{
		perror("fcntl\n");
	}

#endif
#if 1
	struct timeval tv_out;
       tv_out.tv_sec = 5;//�ȴ�5������Ϊ0������
       tv_out.tv_usec = 0;
	setsockopt(sockfd_recv,SOL_SOCKET,SO_RCVTIMEO,&tv_out, sizeof(tv_out));
#endif

	signal_num =  check_signal_id( buf[7] * 256 + buf[8]);

	printf("id = %d , IP = %s  port = %d\n",signal_info_data[signal_num].signal_id,signal_info_data[signal_num].signal_ip,signal_info_data[signal_num].signal_port);
	bzero(&signal_addr, sizeof(signal_addr));
	signal_addr.sin_family = PF_INET;
	
	signal_addr.sin_port = htons(signal_info_data[signal_num].signal_port);
	signal_addr.sin_addr.s_addr = inet_addr(signal_info_data[signal_num].signal_ip);
	
	//signal_addr.sin_port = htons(8888);
	//signal_addr.sin_addr.s_addr = inet_addr("192.168.1.53");

	time ( &rawtime );
	timeinfo = localtime ( &rawtime );

	int sugnal_num = check_signal_id(buf[7] * 256 + buf[8]);
	try
	{
		/**********************/
		
		/*���Ʊ���*/
		if(buf[9] == 0x50)
		{
			
			/*Уʱ*/
			if(buf[10] == 0xf1)
			{
				if(strcmp(signal_info_data[sugnal_num].device_type,"TC100") == 0)
				{
					
					unsigned char F1_date_lin[21];

					for(i = 0; i < sizeof(F1_date_lin);i++)
					{
						F1_date_lin[i] = F1_date[i];
					}
					
					F1_date_lin[7] = buf[7];
					F1_date_lin[8] = buf[8];
					F1_date_lin[11] = (1900+timeinfo->tm_year) >> 8;
					F1_date_lin[12] = (1900+timeinfo->tm_year) & 0xff;
					F1_date_lin[13] =  timeinfo->tm_mon + 1;
					F1_date_lin[14] =  timeinfo->tm_mday;
					F1_date_lin[15] =  timeinfo->tm_hour;
					F1_date_lin[16] =  timeinfo->tm_min;
					F1_date_lin[17] =  timeinfo->tm_sec;
					F1_date_lin[18] =  F1_date_lin[18]  +  (0x01 << timeinfo->tm_wday);
					F1_date_lin[19] =  buf_check_num(F1_date_lin);

					sendto(sockfd_recv, F1_date_lin, sizeof(F1_date_lin), 0, (struct sockaddr*)&signal_addr, sizeof(signal_addr));

				}
				else if(strcmp(signal_info_data[sugnal_num].device_type,"ITC100") == 0)
				{
					time_t rawtime_t = rawtime + (8 * 3600);
					unsigned char F1_date_lin[17];
					bzero(F1_date_lin,17);
					F1_date_lin[0]=0x7e;F1_date_lin[2]= 0x0f;F1_date_lin[4]=0x01;
					F1_date_lin[7]=buf[7];F1_date_lin[8] =buf[8];
					F1_date_lin[9]=0x20;F1_date_lin[10] =0xf1;

					F1_date_lin[11]= ((unsigned int)rawtime_t &0xff000000)>> 24;
					F1_date_lin[12]= ((unsigned int)rawtime_t &0x00ff0000)>> 16;
					F1_date_lin[13]= ((unsigned int)rawtime_t &0x0000ff00)>> 8;
					F1_date_lin[14]= ((unsigned int)rawtime_t &0x000000ff);

					F1_date_lin[15] =  buf_check_num(F1_date_lin);
					F1_date_lin[16] = 0x7d;
				
				      sendto(sockfd_recv, F1_date_lin, sizeof(F1_date_lin), 0, (struct sockaddr*)&signal_addr, sizeof(signal_addr));
				}

#if 0			
				printf("sendto  F1_date : ");
				int x_y;
				for(x_y = 0;x_y <20;x_y ++)
				{
					printf("%02x",F1_date_lin[x_y]);
				}
				printf("\n");
#endif			
			}
			/*ȡ������*/
			else if(buf[10] == 0xf2)
			{
			#if 0
				if(buf[13] ==0xc6)
				{
					/*ȡ���˲�(Ҳ����D1�ı���)*/
					
				}
			#endif
				unsigned char F2_date_lin[15];

				for(i = 0; i < sizeof(F2_date_lin);i++)
				{
					F2_date_lin[i] = F2_date[i];
				}
				F2_date_lin[7] = buf[7];
				F2_date_lin[8] = buf[8];
				F2_date_lin[11] = buf[11];
				F2_date_lin[12] = buf[13];
				F2_date_lin[13] = buf_check_num(F2_date_lin);
				sendto(sockfd_recv, F2_date_lin, sizeof(F2_date_lin), 0, (struct sockaddr*)&signal_addr, sizeof(signal_addr));

				printf("sendto  F2_date !!!!!!!!!!\n ");
			}
			/*����*/
			else if(buf[10] == 0xf3)
			{
				unsigned char F3_date_lin[14];

				for(i = 0; i < sizeof(F3_date_lin);i++)
				{
					F3_date_lin[i] = F3_date[i];
				}
				
				F3_date_lin[7] = buf[7];
				F3_date_lin[8] = buf[8];
				F3_date_lin[11] = buf[13];
				F3_date_lin[12] = buf_check_num(F3_date_lin);
				sendto(sockfd_recv, F3_date_lin, sizeof(F3_date_lin), 0, (struct sockaddr*)&signal_addr, sizeof(signal_addr));

				printf("sendto  F3_date !!!!!!!!!!\n ");
			}
			/*�ֶ�����*/
			else if(buf[10] == 0xc0)
			{
				if(strcmp(signal_info_data[sugnal_num].device_type,"TC100") == 0)
					{
				unsigned char C0_date_lin[14];

				for(i = 0; i < sizeof(C0_date_lin);i++)
				{
					C0_date_lin[i] = C0_date[i];
				}
					
				C0_date_lin[7] = buf[7];
				C0_date_lin[8] = buf[8];
				C0_date_lin[11] = buf[13];
				C0_date_lin[12] = buf_check_num(C0_date_lin);

				printf("ip  an  port  [%s:%d] :\n", inet_ntoa(signal_addr.sin_addr), ntohs(signal_addr.sin_port));
				sendto(sockfd_recv, C0_date_lin, sizeof(C0_date_lin), 0, (struct sockaddr*)&signal_addr, sizeof(signal_addr));


				unsigned int wxw;
				printf("fasong de baowen :");
				for(wxw = 0; wxw <sizeof(C0_date_lin);wxw++)
				{
					printf("%02x ",C0_date_lin[wxw]);
					
				}
					}
				else if(strcmp(signal_info_data[sugnal_num].device_type,"ITC100") == 0)
				{
					unsigned char C0_date_lin[15];

				bzero(C0_date_lin,15);
					C0_date_lin[0]=0x7e;C0_date_lin[2]= 0x0d;C0_date_lin[4]=0x01;
					C0_date_lin[7]=buf[7];C0_date_lin[8] =buf[8];
					C0_date_lin[9]=0x20;C0_date_lin[10] =0xc0;
					
				C0_date_lin[11] = buf[11];C0_date_lin[12] =  buf[13];
				
				C0_date_lin[13] = buf_check_num(C0_date_lin);
				C0_date_lin[14] = 0x7d;

				printf("ip  an  port  [%s:%d] :\n", inet_ntoa(signal_addr.sin_addr), ntohs(signal_addr.sin_port));
				sendto(sockfd_recv, C0_date_lin, sizeof(C0_date_lin), 0, (struct sockaddr*)&signal_addr, sizeof(signal_addr));


				unsigned int wxw;
				printf("fasong de baowen :");
				for(wxw = 0; wxw <sizeof(C0_date_lin);wxw++)
				{
					printf("%02x ",C0_date_lin[wxw]);
					
				}
				}
				printf("sendto  C0_date !!!!!!!!!!\n ");
			}
			/*��������*/
			else if(buf[10] == 0xc1)
			{
			if(strcmp(signal_info_data[sugnal_num].device_type,"TC100") == 0)
				{
				unsigned char C1_date_lin[14];

				for(i = 0; i < sizeof(C1_date_lin);i++)
				{
					C1_date_lin[i] = C1_date[i];
				}
				
				C1_date_lin[7] = buf[7];
				C1_date_lin[8] = buf[8];
				C1_date_lin[11] = buf[13];
				C1_date_lin[12] = buf_check_num(C1_date_lin);
				sendto(sockfd_recv, C1_date_lin, sizeof(C1_date_lin), 0, (struct sockaddr*)&signal_addr, sizeof(signal_addr));
				}

			else if (strcmp(signal_info_data[sugnal_num].device_type,"ITC100") == 0)
				{
					unsigned char C1_date_lin[15];
					bzero(C1_date_lin,15);
					C1_date_lin[0]=0x7e;C1_date_lin[2]= 0x0d;C1_date_lin[4]=0x01;
					C1_date_lin[7]=buf[7];C1_date_lin[8] =buf[8];
					C1_date_lin[9]=0x20;C1_date_lin[10] =0xc1;
					C1_date_lin[11]=buf[11];C1_date_lin[12]=buf[13];
				C1_date_lin[13] = buf_check_num(C1_date_lin);
				C1_date_lin[14] = 0x7d;
				sendto(sockfd_recv, C1_date_lin, sizeof(C1_date_lin), 0, (struct sockaddr*)&signal_addr, sizeof(signal_addr));
				}
				printf("sendto  C1_date !!!!!!!!!!\n ");
			}
			/*ָ������*/
			else if(buf[10] == 0xc2)
			{
				unsigned char C2_date_lin[15];

				for(i = 0; i < sizeof(C2_date_lin);i++)
				{
					C2_date_lin[i] = C2_date[i];
				}
				
				C2_date_lin[7] = buf[7];
				C2_date_lin[8] = buf[8];
				C2_date_lin[11] = buf[11];
				C2_date_lin[12] = buf[13];
				C2_date_lin[13] = buf_check_num(C2_date_lin);
				int www;
				printf("send c0  buf:");
				for(www = 0; www<15;www ++)
				{
					printf("%02x",C2_date_lin[www]);
				}
				printf("sendto  C2_date !!!!!!!!!!\n ");
				sendto(sockfd_recv, C2_date_lin, sizeof(C2_date_lin), 0, (struct sockaddr*)&signal_addr, sizeof(signal_addr));

				
			}
			/*ָ��ģʽ*/
			else if(buf[10] == 0xc3)
			{
				unsigned char C3_date_lin[15];

				for(i = 0; i < sizeof(C3_date_lin);i++)
				{
					C3_date_lin[i] = C3_date[i];
				}
				C3_date_lin[7] = buf[7];
				C3_date_lin[8] = buf[8];
				C3_date_lin[11] = buf[11];
				C3_date_lin[12] = buf[13];
				C3_date_lin[13] = buf_check_num(C3_date_lin);
				sendto(sockfd_recv, C3_date_lin, sizeof(C3_date_lin), 0, (struct sockaddr*)&signal_addr, sizeof(signal_addr));

				printf("sendto  C3_date !!!!!!!!!!\n ");
			}
			/*ָ������*/
			else if(buf[10] == 0xc4)
			{
				unsigned char C4_date_lin[16];

				for(i = 0; i < sizeof(C4_date_lin);i++)
				{
					C4_date_lin[i] = C4_date[i];
				}
				
				C4_date_lin[7] = buf[7];
				C4_date_lin[8] = buf[8];
				C4_date_lin[11] = buf[11];
				C4_date_lin[12] = buf[12];
				C4_date_lin[13] = buf[13];
				C4_date_lin[14] = buf_check_num(C4_date_lin);
				sendto(sockfd_recv, C4_date_lin, sizeof(C4_date_lin), 0, (struct sockaddr*)&signal_addr, sizeof(signal_addr));

				printf("sendto  C4_date !!!!!!!!!!\n ");
			}
			/*ָ����λ��*/
			else if(buf[10] == 0xc5)
			{
				unsigned char C5_date_lin[16];

				for(i = 0; i < sizeof(C5_date_lin);i++)
				{
					C5_date_lin[i] = C5_date[i];
				}
			
				C5_date_lin[7] = buf[7];
				C5_date_lin[8] = buf[8];
				C5_date_lin[11] = buf[11];
				C5_date_lin[12] = buf[12];
				C5_date_lin[13] = buf[13];
				C5_date_lin[14] = buf_check_num(C5_date_lin);
				sendto(sockfd_recv, C5_date_lin, sizeof(C5_date_lin), 0, (struct sockaddr*)&signal_addr, sizeof(signal_addr));

				printf("sendto  C5_date !!!!!!!!!!\n ");
			}
			/*�·���ʱ�����˲�*/
			else if(buf[10] == 0xd1)
			{
				//int sugnal_num = check_signal_id(buf[7] * 256 + buf[8]);
				
				signal_info_data[sugnal_num].control_heart_flag =0;
				signal_info_data[sugnal_num].control_d1_flag = 1;
				
				while(signal_info_data[sugnal_num].control_heart_flag == 0)
					sleep(1);
				
				if(strcmp(signal_info_data[sugnal_num].device_type,"TC100") == 0)
					{
						unsigned char C6_date_lin[85];

						for(i = 0; i < sizeof(C6_date_lin);i++)
						{
							C6_date_lin[i] = C6_date[i];
						}
			
						int i_l;
				
				
				
						int gro_id = buf[12] * 256 + buf[13];
						sprintf(sql,"select clink_id,  steptime,plan_id from GRO_ROADINFO where gro_id = %d and  unit_id = %d",gro_id,signal_info_data[sugnal_num].unit_id);

						printf("%s\n",sql);
				 		st->setSQL(sql);
				 		rs = st->executeQuery();
				 
						if(rs->next() != 0)
						{
							C6_date_lin[7] = buf[7];
							C6_date_lin[8] = buf[8];
							C6_date_lin[15] = rs->getInt(1) & 0xff;
							C6_date_lin[16] = (rs->getInt(2) * 10) >> 8;
							C6_date_lin[17] = (rs->getInt(2) * 10) & 0xff;
							C6_date_lin[12] = rs->getInt(3);
						}
						else
						{
							close(sockfd_recv);
							return -1;
						}

				
						sprintf(sql,"select stage_time,  stage_id ,stage_type,stage_order from gro_plan_config where gro_id = %d and  signal_id = %d",gro_id,buf[7] * 256 + buf[8]);
						printf("%s\n",sql);
						st->setSQL(sql);
						rs = st->executeQuery();

						int num_select = 0;
						int all_stage_time = 0;

						/*�������λ*/
						for(i_l = 19 ; i_l <83;i_l++)
						{
							C6_date_lin[i_l] = 0;
						}
				
						while(rs->next())
						{
							C6_date_lin[19 + rs->getInt(4) - 1] = rs->getInt(2) & 0xff;
							C6_date_lin[35 + (rs->getInt(4) - 1) * 2] =  (rs->getInt(1) * 10)>> 8;
							C6_date_lin[35 + (rs->getInt(4) - 1) * 2+ 1] =  (rs->getInt(1)*10) & 0xff;
							C6_date_lin[67 + rs->getInt(4) - 1] = rs->getInt(3) & 0xff;
							all_stage_time += rs->getInt(1);
							num_select++;
						}
						C6_date_lin[13] = (all_stage_time * 10) >> 8;
						C6_date_lin[14] = (all_stage_time * 10) & 0xff;
				
						C6_date_lin[18]  = num_select;
						C6_date_lin[83] = buf_check_num(C6_date_lin);

						int ll_i;
						for(ll_i = 0; ll_i < 85; ll_i++)
						{
							signal_info_data[sugnal_num].last_C6_date_lin[ll_i] = C6_date_lin[ll_i];
						}

				
						sendto(sockfd_recv, C6_date_lin, sizeof(C6_date_lin), 0, (struct sockaddr*)&signal_addr, sizeof(signal_addr));
						unsigned int mm;
						printf("send c6 :");
						for(mm = 0; mm < sizeof(C6_date_lin);mm++)
						{
							printf("%02x",C6_date_lin[mm]);
						}
						printf("\n");
						printf("sendto  C6_date !!!!!!!!!!\n ");
					}
				if(strcmp(signal_info_data[sugnal_num].device_type,"ITC100") == 0)
				{
					static int gro_num = 128;
					gro_num = (gro_num+1)%256;
					if(gro_num == 0)
						gro_num = 129;
					//printf(">>> gro num = %d\n",gro_num);
					unsigned char C6_date_lin[166];

					bzero(C6_date_lin,166);

					C6_date_lin[0] =0x7e;C6_date_lin[2] =0xa4;C6_date_lin[4] =0x01;
					C6_date_lin[7] =buf[7];C6_date_lin[8] =buf[8];
					C6_date_lin[9] =0x20;C6_date_lin[10] =0xc6;C6_date_lin[12] =gro_num;
					C6_date_lin[165] =0x7d;

					int gro_id = buf[12] * 256 + buf[13];
					sprintf(sql,"select clink_id,  steptime,plan_id,listunitid,times from GRO_ROADINFO where gro_id = %d and  unit_id = %d",gro_id,signal_info_data[sugnal_num].unit_id);

					printf("%s\n",sql);
				 	st->setSQL(sql);
				 	rs = st->executeQuery();

					if(rs->next() != 0)
					{
							C6_date_lin[11] = rs->getInt(4);
							//C6_date_lin[12] = rs->getInt(3); // ��������Լ����壬129-255��ÿ�μ�1
							C6_date_lin[13] = rs->getInt(4);
							C6_date_lin[14] = (rs->getInt(5) *10) >> 8 ;
							C6_date_lin[15] = (rs->getInt(5)*10) & 0xff;
							
							C6_date_lin[16] = rs->getInt(1) & 0xff; //Э�����
							C6_date_lin[17] = (rs->getInt(2) * 10) >> 8; // ��λ��
							C6_date_lin[18] = (rs->getInt(2) * 10) & 0xff;
							
					}
					else
					{
						close(sockfd_recv);
						return -1;
					}

					int num_select = 0;

					sprintf(sql,"select stage_time,  stage_id ,stage_type,stage_order,green_time*10,green_flash_time*10,green_flash_qtime*10,red_time*10 from gro_plan_config where gro_id = %d and  signal_id = %d",gro_id,buf[7] * 256 + buf[8]);
					printf("%s\n",sql);
					st->setSQL(sql);
					rs = st->executeQuery();

					while(rs->next())
					{
						C6_date_lin[20+ rs->getInt(4) - 1] = rs->getInt(2) & 0xff;
						C6_date_lin[36 + (rs->getInt(4) - 1) * 2] =  (rs->getInt(1) * 10)>> 8;
						C6_date_lin[36 + (rs->getInt(4) - 1) * 2+ 1] =  (rs->getInt(1)*10) & 0xff;
						C6_date_lin[68 + rs->getInt(4) - 1] = rs->getInt(3) & 0xff;
						num_select++;

						C6_date_lin[84 +( rs->getInt(4) - 1)*5] = rs->getInt(5) /256;
						C6_date_lin[85 +( rs->getInt(4) - 1)*5] = rs->getInt(5) %256;
						C6_date_lin[86 +( rs->getInt(4) - 1)*5] = rs->getInt(6) ;
						C6_date_lin[87 +( rs->getInt(4) - 1)*5] = rs->getInt(7) ;
						C6_date_lin[88 +( rs->getInt(4) - 1)*5] = rs->getInt(8) ;

					}
				
					C6_date_lin[19]  = num_select;
					C6_date_lin[164] = buf_check_num(C6_date_lin);

					int ll_i;
					for(ll_i = 0; ll_i < 166; ll_i++)
					{
						signal_info_data[sugnal_num].last_C6_date_lin[ll_i] = C6_date_lin[ll_i];
					}

					sendto(sockfd_recv, C6_date_lin, sizeof(C6_date_lin), 0, (struct sockaddr*)&signal_addr, sizeof(signal_addr));
				}
				
			}
			else if(buf[10] == 0xc6)
			{
				/*�·�����*/
				
			}
			else if(buf[10] == 0xc7)
			{
				/*�·�����*/

				signal_info_data[sugnal_num].signal_control_heart_time = buf[12] *256 + buf[13];
				
				unsigned char C0_date_lin[14];

				for(i = 0; i < sizeof(C0_date_lin);i++)
				{
					C0_date_lin[i] = C0_date[i];
				}
					
				C0_date_lin[7] = buf[7];
				C0_date_lin[8] = buf[8];
				C0_date_lin[11] = buf[15] + 0x80;
				C0_date_lin[12] = buf_check_num(C0_date_lin);

				printf("ip  an  port  [%s:%d] :\n", inet_ntoa(signal_addr.sin_addr), ntohs(signal_addr.sin_port));
				if(strcmp(signal_info_data[sugnal_num].device_type,"TC100") == 0)
				{
					sendto(sockfd_recv, C0_date_lin, sizeof(C0_date_lin), 0, (struct sockaddr*)&signal_addr, sizeof(signal_addr));

					unsigned int wxw;
					printf("fasong de baowen :");
					for(wxw = 0; wxw <sizeof(C0_date_lin);wxw++)
					{
						printf("%02x ",C0_date_lin[wxw]);
					
					}
					printf("\n");
				}
				else if(strcmp(signal_info_data[sugnal_num].device_type,"ITC100") == 0)
				{
					unsigned char C0_date_lin_itc[15];

					bzero(C0_date_lin_itc,15);
					C0_date_lin_itc[0]=0x7e;C0_date_lin_itc[2]= 0x0d;C0_date_lin_itc[4]=0x01;
					C0_date_lin_itc[7]=buf[7];C0_date_lin_itc[8] =buf[8];
					C0_date_lin_itc[9]=0x20;C0_date_lin_itc[10] =0xc0;
					
				C0_date_lin_itc[11] = buf[11];C0_date_lin_itc[12] =  buf[15];
				
				C0_date_lin_itc[13] = buf_check_num(C0_date_lin_itc);
				C0_date_lin_itc[14] = 0x7d;

				printf("ip  an  port  [%s:%d] :\n", inet_ntoa(signal_addr.sin_addr), ntohs(signal_addr.sin_port));
				sendto(sockfd_recv, C0_date_lin_itc, sizeof(C0_date_lin_itc), 0, (struct sockaddr*)&signal_addr, sizeof(signal_addr));

				}
				
				printf("sendto  C7_date shijian = %d  jieduan = %d \n ",buf[12]*256 + buf[13],buf[14]*256 + buf[15]);
				
			}
			else if(buf[10] == 0xff)
			{
				/*����*/
				return 2;
			}
			else
			{
				close(sockfd_recv);
				return -1;
			}
				
			res = recvfrom(sockfd_recv, recv_buf, 16, 0, NULL, NULL);
#if 1
			int ww;
			printf("\n RECV  BUF IS :");
			for(ww = 0; ww <res;ww++ )
			{
				printf("%02x",recv_buf[ww]);
			}
			printf("\n");		
			printf("recvfrom  date !!!!!!!!!!\n ");
#endif		
			if(res <= 0)
			{
				close(sockfd_recv);
				return -1;
			}
			else if( buf[10] == recv_buf[10] || ( buf[10] == 0xc7 && recv_buf[10] == 0xc0) || ( buf[10] == 0xd1 && recv_buf[10] == 0xc6))
			{

				/*����ȡ��*/
				if(buf[10] == 0xf2)
				{
					if(buf[13] ==0xc6)
					{
						/*ȡ���˲�(Ҳ����D1�ı���)*/
						signal_info_data[sugnal_num].signal_control_flag = 0;
						sprintf(sql,"update CONTROL_CMD set control_flag = 0 where signal_id = %d ",buf[7] * 256 + buf[8]);
						printf("%s\n",sql);
						st->setSQL(sql);
						st->executeUpdate();

						signal_info_data[sugnal_num].signal_last_control_flag= 0;
						signal_info_data[sugnal_num].control_d1_flag = 0;
					}
				}

				/*����*/
				else if(buf[10] == 0xc7 && recv_buf[10] == 0xc0  )
				{
					/*����ȡ���ɹ�*/
					if(recv_buf[11] == 0x00) 
					{
						if(signal_info_data[sugnal_num].signal_last_control_flag == 0)
						{
							signal_info_data[sugnal_num].signal_control_flag = 0;
							sprintf(sql,"update CONTROL_CMD set control_flag = 0 where signal_id = %d ",buf[7] * 256 + buf[8]);
							printf("%s\n",sql);
							st->setSQL(sql);
							st->executeUpdate();
							
						
							
						}
						else if(signal_info_data[sugnal_num].signal_last_control_flag == 3)
						{
							signal_info_data[sugnal_num].signal_control_flag = 3;
							sprintf(sql,"update CONTROL_CMD set control_flag = 3 where signal_id = %d ",buf[7] * 256 + buf[8]);
							printf("%s\n",sql);
							st->setSQL(sql);
							st->executeUpdate();

							/*����һ���˲�����*/
							sendto(sockfd_recv, signal_info_data[signal_num].last_C6_date_lin, sizeof(signal_info_data[signal_num].last_C6_date_lin) , 0, (struct sockaddr*)&signal_addr, sizeof(signal_addr));

							 int num = recvfrom(sockfd_recv, recv_buf, BUFF_NUM, 0, NULL, NULL);
							 if(num <= 0)
							 {
							 	printf(" zai ci  lv bo shibai \n");
							 }
							 else
							 {
							 	printf(" zai ci  lv bo chenggong \n");
							 }

							
						}
							
					}
					/*�������óɹ�*/
					else 
					{
						signal_info_data[sugnal_num].signal_control_flag = 2;
						sprintf(sql,"update CONTROL_CMD set control_flag = 2 where signal_id = %d ",buf[7] * 256 + buf[8]);
						printf("%s\n",sql);
						st->setSQL(sql);
						st->executeUpdate();

					}
				}
				/*�˲�*/
				else if(buf[10] == 0xd1 && recv_buf[10] == 0xc6)
				{
					/*�ǿ���״̬���ܱ���˲�*/
					if(signal_info_data[sugnal_num].signal_control_flag == 0 || signal_info_data[sugnal_num].signal_control_flag == 3|| signal_info_data[sugnal_num].signal_control_flag == 1)
					{
						/*�յ��·���ʱ����Ҳ�����˲�������״̬�ĳ�3*/
						 signal_info_data[sugnal_num].signal_control_flag = 3;
						sprintf(sql,"update CONTROL_CMD set control_flag = 3 where signal_id = %d ",buf[7] * 256 + buf[8]);
						printf("%s\n",sql);
						st->setSQL(sql);
						st->executeUpdate();
						
						/*�յ��·���ʱ����Ҳ�����˲�����һ�ο���״̬�ĳ�3��������������0*/
						signal_info_data[sugnal_num].signal_last_control_flag = 3;
						
						//strcpy(signal_info_data[sugnal_num].last_C6_date_lin,C6_date_lin);
					}
					else
					{
						/*����������״̬�������˲�*/
						close(sockfd_recv);
						return -1;
					}
				}
				
				close(sockfd_recv);
				return 0;
			}
			else
			{
				close(sockfd_recv);
				return -1;
			}
		
		}
		/*���ñ���*/
		else if(buf[9] == 0x20)
		{
			/*������Ϣ����*/
			if(buf[10] == 0x01)
			{					
				if(strcmp(signal_info_data[sugnal_num].device_type,"TC100") == 0)
					{
				unsigned char set_01_date_lin[164 + 1];

				for(i = 0; i < sizeof(set_01_date_lin);i++)
				{
					set_01_date_lin[i] = set_01_date[i];
				}
				
				set_01_date_lin[7] = buf[7];
				set_01_date_lin[8] = buf[8];
				sprintf(sql,"select signal_ip,signal_netmask,signal_gateway,signal_port,swj_ip,swj_port,nvl(CONFIG_FAULT_YELLOWFLASH,0),nvl(GREEN_CONFLICT_YELLOWFLASH,0), nvl(REDGREEN_CONFLICT_YELLOWFLASH,0),nvl(LAMP_FAILURE_YELLOWFLASH,0)  from  SIGNAL_CONFIG_INFO where signal_id = %d" ,buf[7] * 256 + buf[8]);
				printf("%s\n",sql);
				
				st->setSQL(sql);
				rs = st->executeQuery();
				if(rs->next() == 0)
				{
					close(sockfd_recv);
					return -1;
				}
				else
				{
					//const char delims[] = ",";
					
					

					/*�źŻ�IP*/
					sprintf(lin_buf,"%s",rs->getString(1).c_str());
					result = strtok( lin_buf, "." );
					i = 0;
					while( result != NULL ) 
					{
					    	
						lin_res_num= atoi(result);
						//printf( "result is \"%s\"\n", result );
						//printf( "lin_res_num is \"%d\"\n", lin_res_num);
						set_01_date_lin[12 + i] = lin_res_num;
					   	result = strtok( NULL, "." );
						i++;
						if(i >= 4)
						{
							break;
						}
					}

					/*�źŻ�����*/
					bzero(lin_buf,sizeof(lin_buf));
					sprintf(lin_buf,"%s",rs->getString(2).c_str());
					result = strtok( lin_buf, "." );
					i = 0;
					while( result != NULL ) 
					{
					    	
						lin_res_num= atoi(result);
						//printf( "result is \"%s\"\n", result );
						//printf( "lin_res_num is \"%d\"\n", lin_res_num);
						set_01_date_lin[16 + i] = lin_res_num;
					   	result = strtok( NULL, "." );
						i++;
						if(i >= 4)
						{
							break;
						}
					}

					/*�źŻ�����*/
					bzero(lin_buf,sizeof(lin_buf));
					sprintf(lin_buf,"%s",rs->getString(3).c_str());
					result = strtok( lin_buf, "." );
					i = 0;
					while( result != NULL ) 
					{
					    	
						lin_res_num= atoi(result);
						//printf( "result is \"%s\"\n", result );
						//printf( "lin_res_num is \"%d\"\n", lin_res_num);
						set_01_date_lin[20 + i] = lin_res_num;
					   	result = strtok( NULL, "." );
						i++;
						if(i >= 4)
						{
							break;
						}
					}
				
					/*�źŻ��˿�*/
					set_01_date_lin[24] = rs->getInt(4) >> 8;
					set_01_date_lin[25] = rs->getInt(4) & 0xff;

					
					/*��λ��IP*/
					bzero(lin_buf,sizeof(lin_buf));
					sprintf(lin_buf,"%s",rs->getString(5).c_str());
					result = strtok( lin_buf, "." );
					i = 0;
					while( result != NULL ) 
					{
					    	
						lin_res_num= atoi(result);
						//printf( "result is \"%s\"\n", result );
						//printf( "lin_res_num is \"%d\"\n", lin_res_num);
						set_01_date_lin[26 + i] = lin_res_num;
					   	result = strtok( NULL, "." );
						i++;
						if(i >= 4)
						{
							break;
						}
					}

					/*��λ���˿�*/
					set_01_date_lin[30] = rs->getInt(6) >> 8;
					set_01_date_lin[31] = rs->getInt(6) & 0xff;

					/*��λ��ID*/
					set_01_date_lin[32] = (buf[7] * 256 + buf[8]) >> 8;
					set_01_date_lin[33] = (buf[7] * 256 + buf[8])& 0xff;

					if(rs->getInt(7) == 0 )
					{
						set_01_date_lin[34] = (set_01_date_lin[34] & 0xf7);
					}
					else
					{
						set_01_date_lin[34] = (set_01_date_lin[34] & 0xf7) +0x08;
					}

					if(rs->getInt(8) == 0 )
					{
						set_01_date_lin[34] = (set_01_date_lin[34] & 0xfb);
					}
					else
					{
						set_01_date_lin[34] = (set_01_date_lin[34] & 0xfb) +0x04;
					}

					if(rs->getInt(9) == 0 )
					{
						set_01_date_lin[34] = (set_01_date_lin[34] & 0xfd);
					}
					else
					{
						set_01_date_lin[34] = (set_01_date_lin[34] & 0xfd) +0x02;
					}

					
					if(rs->getInt(10) == 0 )
					{
						set_01_date_lin[34] = (set_01_date_lin[34] & 0xfe);
					}
					else
					{
						set_01_date_lin[34] = (set_01_date_lin[34] & 0xfe) +0x01;
					}
					
					set_01_date_lin[162 + 1] = buf_check_num(set_01_date_lin);
					
					sendto(sockfd_recv, set_01_date_lin, sizeof(set_01_date_lin), 0, (struct sockaddr*)&signal_addr, sizeof(signal_addr));
					//printf("sendto  C6_date !!!!!!!!!!\n ");
/*
					res = recvfrom(sockfd_recv, recv_buf, 512, 0, NULL, NULL);
					printf("recvfrom  date !!!!!!!!!!\n ");
					if(res <= 0)
					{
						close(sockfd_recv);
						return -1;
					}
					else
					{
						close(sockfd_recv);
						return 0;
					}
*/
					struct timeval 	timeVal;
					fd_set  fdSet;
					int ret;

					timeVal.tv_sec = signal_report_timeout;
					timeVal.tv_usec = 0;

					FD_ZERO(&fdSet);
					FD_SET(sockfd_recv, &fdSet);

					ret = select((int) sockfd_recv + 1, &fdSet,NULL, NULL, &timeVal);
					if(ret == 0)                     //��ʱ
					{
						printf("signal no reply::jiang ce qi\n");
						close(sockfd_recv);
						return -1;
					}
					if(ret)                //������
					{
						res = recvfrom(sockfd_recv, recv_buf, 512, 0, NULL, NULL);
						if(res == 0)
						{
							printf("select timeout::ji chu shu ju \n");
							close(sockfd_recv);
							return -1;
						}
						else
						{
							printf("recvfrom signal_id = %d,jichu shuju report!!!!!!!!!!\n ",(recv_buf[7]*256+recv_buf[8]));
							//printf("xieyi lei xing = %#2X,shujubianma = %#2X, baowen zhuangtai =%#2X \n",recv_buf[9],recv_buf[10],recv_buf[12]);
							
						}
					}								

					
				}
				}
				else if(strcmp(signal_info_data[sugnal_num].device_type,"ITC100") == 0)
				{ //0x06,0x07,0x09,0x0a
					unsigned char set_05_date_lin[100],set_06_date_lin[26],set_07_date_lin[21],set_08_date_lin[20],set_09_date_lin[16],set_0a_date_lin[15];
					bzero(set_05_date_lin,100);
					bzero(set_06_date_lin,26);
					bzero(set_07_date_lin,21);
					bzero(set_07_date_lin,20);
					bzero(set_09_date_lin,16);
					bzero(set_0a_date_lin,15);
					sprintf(sql,"select signal_ip,signal_netmask,signal_gateway,swj_ip,swj_port,swj_type,signal_id,cotrol_conut,unit_name_ascii from  SIGNAL_CONFIG_INFO where signal_id = %d" ,buf[7] * 256 + buf[8]);
					printf("%s\n",sql);
				
					st->setSQL(sql);
					rs = st->executeQuery();
					if(rs->next() == 0)
					{
						close(sockfd_recv);
						return -1;
					}
					else 
					{
						/*�źŻ�IP*/
					sprintf(lin_buf,"%s",rs->getString(1).c_str());
					result = strtok( lin_buf, "." );
					i = 0;
					while( result != NULL ) 
					{
					    	
						lin_res_num= atoi(result);
						//printf( "result is \"%s\"\n", result );
						//printf( "lin_res_num is \"%d\"\n", lin_res_num);
						set_06_date_lin[12 + i] = lin_res_num;
					   	result = strtok( NULL, "." );
						i++;
						if(i >= 4)
						{
							break;
						}
					}

					/*�źŻ�����*/
					bzero(lin_buf,sizeof(lin_buf));
					sprintf(lin_buf,"%s",rs->getString(2).c_str());
					result = strtok( lin_buf, "." );
					i = 0;
					while( result != NULL ) 
					{
					    	
						lin_res_num= atoi(result);
						//printf( "result is \"%s\"\n", result );
						//printf( "lin_res_num is \"%d\"\n", lin_res_num);
						set_06_date_lin[16 + i] = lin_res_num;
					   	result = strtok( NULL, "." );
						i++;
						if(i >= 4)
						{
							break;
						}
					}

					/*�źŻ�����*/
					bzero(lin_buf,sizeof(lin_buf));
					sprintf(lin_buf,"%s",rs->getString(3).c_str());
					result = strtok( lin_buf, "." );
					i = 0;
					while( result != NULL ) 
					{
					    	
						lin_res_num= atoi(result);
						//printf( "result is \"%s\"\n", result );
						//printf( "lin_res_num is \"%d\"\n", lin_res_num);
						set_06_date_lin[20 + i] = lin_res_num;
					   	result = strtok( NULL, "." );
						i++;
						if(i >= 4)
						{
							break;
						}
					}

					/*��λ��IP*/
					bzero(lin_buf,sizeof(lin_buf));
					sprintf(lin_buf,"%s",rs->getString(4).c_str());
					result = strtok( lin_buf, "." );
					i = 0;
					while( result != NULL ) 
					{
					    	
						lin_res_num= atoi(result);
						//printf( "result is \"%s\"\n", result );
						//printf( "lin_res_num is \"%d\"\n", lin_res_num);
						set_07_date_lin[12 + i] = lin_res_num;
					   	result = strtok( NULL, "." );
						i++;
						if(i >= 4)
						{
							break;
						}
					}

					/*��λ���˿�*/
					set_07_date_lin[16] = rs->getInt(5) >> 8;
					set_07_date_lin[17] = rs->getInt(5) & 0xff;

					/*��λ������*/
					set_07_date_lin[18] = atoi(rs->getString(6).c_str());

					/*�źŻ�ID*/
					set_09_date_lin[12] =(rs->getInt(7) ) /256;
					set_09_date_lin[13] = (rs->getInt(7)) %256;

					/*�źŻ�����·������*/
					//set_0a_date_lin[12] =(rs->getInt(8) ) /256;
					//set_0a_date_lin[13] = (rs->getInt(8)) %256;
					set_0a_date_lin[12] =rs->getInt(8);


					/*�źŻ���������*/
					char temp_name[50];
					bzero(lin_buf,sizeof(lin_buf));
					sprintf(lin_buf,"%s",rs->getString(9).c_str());
					result = strtok( lin_buf, "," );
					int count_name = 0;
					while( result != NULL ) 
					{
					    	
						sscanf(result,"%x",&lin_res_num);
						//printf( "result is \"%s\"\n", result );
						//printf( "lin_res_num is \"%d\"\n", lin_res_num);
						temp_name[count_name] = lin_res_num;
					   	result = strtok( NULL, "," );
						count_name++;
					}
					set_05_date_lin[0]=0x7e;set_05_date_lin[1]=(count_name+14-2)/256;set_05_date_lin[2]=(count_name+14-2)%256;
					set_05_date_lin[4]=0x01;set_05_date_lin[7]=buf[7];set_05_date_lin[8]=buf[8];
					set_05_date_lin[9]=0x20;set_05_date_lin[10]=0x05;
					for(i = 0;i<count_name; i ++)
						set_05_date_lin[12+i] = temp_name[i];
					set_05_date_lin[count_name + 12] = buf_check_num(set_05_date_lin);
					set_05_date_lin[count_name + 13] = 0x7d;
					
					
					set_06_date_lin[0]=0x7e;set_07_date_lin[0]=0x7e;set_09_date_lin[0]=0x7e;set_0a_date_lin[0]=0x7e;
					set_06_date_lin[2]=0x18;set_07_date_lin[2]=0x13;set_09_date_lin[2]=0x0e;set_0a_date_lin[2]=0x0d;
					set_06_date_lin[4]=0x01;set_07_date_lin[4]=0x01;set_09_date_lin[4]=0x01;set_0a_date_lin[4]=0x01;
					set_06_date_lin[7]=buf[7];set_07_date_lin[7]=buf[7];set_09_date_lin[7]=buf[7];set_0a_date_lin[7]=buf[7];
					set_06_date_lin[8]=buf[8];set_07_date_lin[8]=buf[8];set_09_date_lin[8]=buf[8];set_0a_date_lin[8]=buf[8];
					set_06_date_lin[9]=0x20;set_07_date_lin[9]=0x20;set_09_date_lin[9]=0x20;set_0a_date_lin[9]=0x20;
					set_06_date_lin[10]=0x06;set_07_date_lin[10]=0x07;set_09_date_lin[10]=0x09;set_0a_date_lin[10]=0x0a;
					set_06_date_lin[24]=buf_check_num(set_06_date_lin);
					set_07_date_lin[19]=buf_check_num(set_07_date_lin);
					set_09_date_lin[14]=buf_check_num(set_09_date_lin);
					set_0a_date_lin[13]=buf_check_num(set_0a_date_lin);
					set_06_date_lin[25]=0x7d;set_07_date_lin[20]=0x7d;set_09_date_lin[15]=0x7d;set_0a_date_lin[14]=0x7d;

					set_08_date_lin[0]=0x7e;set_08_date_lin[2]=0x12;set_08_date_lin[4]=0x01;
					set_08_date_lin[7]=buf[7];set_08_date_lin[8]=buf[8];set_08_date_lin[9]=0x20;
					set_08_date_lin[10]=0x08;set_08_date_lin[12]=0xb6;set_08_date_lin[13]=0xab;
					set_08_date_lin[14]=0xb0;set_08_date_lin[15]=0xcb;set_08_date_lin[16]=0xc7;
					set_08_date_lin[17]=0xf8;set_08_date_lin[19]=0x7d;
					set_08_date_lin[18]=buf_check_num(set_08_date_lin);

					sendto(sockfd_recv, set_05_date_lin, (count_name+14), 0, (struct sockaddr*)&signal_addr, sizeof(signal_addr));
/*
					res = recvfrom(sockfd_recv, recv_buf, 512, 0, NULL, NULL);
					printf("recvfrom  date 05!!!!!!!!!!\n ");
					if(res <= 0)
					{
						close(sockfd_recv);
						return -1;
					}
	*/
					struct timeval 	timeVal;
					fd_set  fdSet;
					int ret;

					timeVal.tv_sec = signal_report_timeout;
					timeVal.tv_usec = 0;

					FD_ZERO(&fdSet);
					FD_SET(sockfd_recv, &fdSet);

					ret = select((int) sockfd_recv + 1, &fdSet,NULL, NULL, &timeVal);
					if(ret == 0)                     //��ʱ
					{
						printf("signal no reply::jiang ce qi\n");
						close(sockfd_recv);
						return -1;
					}
					if(ret)                //������
					{
						res = recvfrom(sockfd_recv, recv_buf, 512, 0, NULL, NULL);
						if(res == 0)
						{
							printf("select timeout::jiang ce qi\n");
							close(sockfd_recv);
							return -1;
						}
						else
						{
							printf("recvfrom signal_id = %d,0x05 report!!!!!!!!!!\n ",(recv_buf[7]*256+recv_buf[8]));
							printf("xieyi lei xing = %#2X,shujubianma = %#2X, baowen zhuangtai =%#2X \n",recv_buf[9],recv_buf[10],recv_buf[12]);
							
						}
					}								
					


	//0x06
					sendto(sockfd_recv, set_06_date_lin, sizeof(set_06_date_lin), 0, (struct sockaddr*)&signal_addr, sizeof(signal_addr));
					/*
					res = recvfrom(sockfd_recv, recv_buf, 512, 0, NULL, NULL);
					printf("recvfrom  date 06!!!!!!!!!!\n ");
					if(res <= 0)
					{
						close(sockfd_recv);
						return -1;
					}
					*/
					FD_ZERO(&fdSet);
					FD_SET(sockfd_recv, &fdSet);

					ret = select((int) sockfd_recv + 1, &fdSet,NULL, NULL, &timeVal);
					if(ret == 0)                     //��ʱ
					{
						printf("signal no reply::IP\n");
						close(sockfd_recv);
						return -1;
					}
					if(ret)                //������
					{
						res = recvfrom(sockfd_recv, recv_buf, 512, 0, NULL, NULL);
						if(res == 0)
						{
							printf("select timeout::IP\n");
							close(sockfd_recv);
							return -1;
						}
						else
						{
							//close(sockfd_recv);
							printf("recvfrom signal_id = %d,0x06 report!!!!!!!!!!\n ",(recv_buf[7]*256+recv_buf[8]));
							printf("xieyi lei xing = %#2X,shujubianma = %#2X, baowen zhuangtai =%#2X \n",recv_buf[9],recv_buf[10],recv_buf[12]);
							
						}
					}	
//0x07				
					sendto(sockfd_recv, set_07_date_lin, sizeof(set_07_date_lin), 0, (struct sockaddr*)&signal_addr, sizeof(signal_addr));
				/*	res = recvfrom(sockfd_recv, recv_buf, 512, 0, NULL, NULL);
					printf("recvfrom  date 07 !!!!!!!!!!\n ");
					if(res <= 0)
					{
						close(sockfd_recv);
						return -1;
					}
				*/
					FD_ZERO(&fdSet);
					FD_SET(sockfd_recv, &fdSet);

					ret = select((int) sockfd_recv + 1, &fdSet,NULL, NULL, &timeVal);
					if(ret == 0)                     //��ʱ
					{
						printf("signal no reply::IP\n");
						close(sockfd_recv);
						return -1;
					}
					if(ret)                //������
					{
						res = recvfrom(sockfd_recv, recv_buf, 512, 0, NULL, NULL);
						if(res == 0)
						{
							printf("select timeout::IP\n");
							close(sockfd_recv);
							return -1;
						}
						else
						{
							//close(sockfd_recv);
							printf("recvfrom signal_id = %d,0x07 report!!!!!!!!!!\n ",(recv_buf[7]*256+recv_buf[8]));
							printf("xieyi lei xing = %#2X,shujubianma = %#2X, baowen zhuangtai =%#2X \n",recv_buf[9],recv_buf[10],recv_buf[12]);
							
						}
					}				


//���͹̶����źŻ�ʱ����Ϣ
					
					sendto(sockfd_recv, set_08_date_lin, sizeof(set_08_date_lin), 0, (struct sockaddr*)&signal_addr, sizeof(signal_addr));
					/*
					res = recvfrom(sockfd_recv, recv_buf, 512, 0, NULL, NULL);
					printf("recvfrom  date 08 !!!!!!!!!!\n ");
					if(res <= 0)
					{
						close(sockfd_recv);
						return -1;
					}
					*/
					FD_ZERO(&fdSet);
					FD_SET(sockfd_recv, &fdSet);

					ret = select((int) sockfd_recv + 1, &fdSet,NULL, NULL, &timeVal);
					if(ret == 0)                     //��ʱ
					{
						printf("signal no reply::shiqu\n");
						close(sockfd_recv);
						return -1;
					}
					if(ret)                //������
					{
						res = recvfrom(sockfd_recv, recv_buf, 512, 0, NULL, NULL);
						if(res == 0)
						{
							printf("select timeout::shiqu\n");
							close(sockfd_recv);
							return -1;
						}
						else
						{
							//close(sockfd_recv);
							printf("recvfrom signal_id = %d,0x08 report!!!!!!!!!!\n ",(recv_buf[7]*256+recv_buf[8]));
							printf("xie yi lei xing = %#2X,shujubianma = %#2X, baowen zhuangtai =%#2X \n",recv_buf[9],recv_buf[10],recv_buf[12]);
							
						}
					}

//0x09			
					sendto(sockfd_recv, set_09_date_lin, sizeof(set_09_date_lin), 0, (struct sockaddr*)&signal_addr, sizeof(signal_addr));
		/*			res = recvfrom(sockfd_recv, recv_buf, 512, 0, NULL, NULL);
					printf("recvfrom  date 09 !!!!!!!!!!\n ");
					if(res <= 0)
					{
						close(sockfd_recv);
						return -1;
					}
			*/
					FD_ZERO(&fdSet);
					FD_SET(sockfd_recv, &fdSet);

					ret = select((int) sockfd_recv + 1, &fdSet,NULL, NULL, &timeVal);
					if(ret == 0)                     //��ʱ
					{
						printf("signal no reply::IP\n");
						close(sockfd_recv);
						return -1;
					}
					if(ret)                //������
					{
						res = recvfrom(sockfd_recv, recv_buf, 512, 0, NULL, NULL);
						if(res == 0)
						{
							printf("select timeout::IP\n");
							close(sockfd_recv);
							return -1;
						}
						else
						{
							//close(sockfd_recv);
							printf("recvfrom signal_id = %d,0x06 report!!!!!!!!!!\n ",(recv_buf[7]*256+recv_buf[8]));
							printf("xieyi lei xing = %#2X,shujubianma = %#2X, baowen zhuangtai =%#2X \n",recv_buf[9],recv_buf[10],recv_buf[12]);
							
						}
					}			
//0x0A		
					sendto(sockfd_recv, set_0a_date_lin, sizeof(set_0a_date_lin), 0, (struct sockaddr*)&signal_addr, sizeof(signal_addr));
/*					
					res = recvfrom(sockfd_recv, recv_buf, 512, 0, NULL, NULL);
					printf("recvfrom  date 0a !!!!!!!!!!\n ");
					if(res <= 0)
					{
						close(sockfd_recv);
						return -1;
					}
					
					else
					{
						close(sockfd_recv);
						return 0;
					}
				*/	
					FD_ZERO(&fdSet);
					FD_SET(sockfd_recv, &fdSet);

					ret = select((int) sockfd_recv + 1, &fdSet,NULL, NULL, &timeVal);
					if(ret == 0)                     //��ʱ
					{
						printf("signal no reply::IP\n");
						close(sockfd_recv);
						return -1;
					}
					if(ret)                //������
					{
						res = recvfrom(sockfd_recv, recv_buf, 512, 0, NULL, NULL);
						if(res == 0)
						{
							printf("select timeout::IP\n");
							close(sockfd_recv);
							return -1;
						}
						else
						{
							close(sockfd_recv);
							printf("recvfrom signal_id = %d,0x06 report!!!!!!!!!!\n ",(recv_buf[7]*256+recv_buf[8]));
							printf("xieyi lei xing = %#2X,shujubianma = %#2X, baowen zhuangtai =%#2X \n",recv_buf[9],recv_buf[10],recv_buf[12]);
							return 0;
						}     
					}			
					}
				
				}

			}
			/*����������,tc100*/
			else if(buf[10] == 0x02)
			{	
				unsigned char set_02_date_lin[24];

				for(i = 0; i < sizeof(set_02_date_lin);i++)
				{
					set_02_date_lin[i] = set_02_date[i];
				}
				
				set_02_date_lin[7] = buf[7];
				set_02_date_lin[8] = buf[8];
				
				
				sprintf(sql,"select san_zq,san_zkb,kuaisan_zq,kuaisan_zkb,PULSE_WIDTH_TIME  from  SANPINLV  where signal_id = %d" ,buf[7] * 256 + buf[8]);
			
				printf("%s\n",sql);
				
				st->setSQL(sql);
				rs = st->executeQuery();
				if(rs->next() == 0)
				{
					close(sockfd_recv);
					return -1;
				}
#if 0
				/*������*/
				set_02_date_lin[12] = (rs->getInt(1) * 10) >> 8;
				set_02_date_lin[13] = (rs->getInt(1) * 10)& 0xff;

				/*������*/
				set_02_date_lin[14] = ((rs->getInt(2) )* 10) >> 8;
				set_02_date_lin[15] = ((rs->getInt(2) )* 10) & 0xff;

				/*������*/
				set_02_date_lin[16] = ((rs->getInt(3) )* 10) >> 8;
				set_02_date_lin[17] = ((rs->getInt(3) )* 10) & 0xff;

				/*������*/
				set_02_date_lin[18] = ((rs->getInt(4) )* 10) >> 8;
				set_02_date_lin[19] = ((rs->getInt(4) )* 10) & 0xff;

				/*���嵹��ʱʱ��*/
				set_02_date_lin[20] = ((rs->getInt(5) )* 10) >> 8;
				set_02_date_lin[21] = ((rs->getInt(5) )* 10) & 0xff;
#endif

#if 1
				/*������*/
				set_02_date_lin[12] = (rs->getInt(1) ) >> 8;
				set_02_date_lin[13] = (rs->getInt(1) )& 0xff;

				/*������*/
				set_02_date_lin[14] = ((rs->getInt(2) )) >> 8;
				set_02_date_lin[15] = ((rs->getInt(2) )) & 0xff;

				/*������*/
				set_02_date_lin[16] = ((rs->getInt(3) )) >> 8;
				set_02_date_lin[17] = ((rs->getInt(3) )) & 0xff;

				/*������*/
				set_02_date_lin[18] = ((rs->getInt(4) )) >> 8;
				set_02_date_lin[19] = ((rs->getInt(4) )) & 0xff;

				/*���嵹��ʱʱ��*/
				set_02_date_lin[20] = ((rs->getInt(5) )) >> 8;
				set_02_date_lin[21] = ((rs->getInt(5) )) & 0xff;
#endif

				set_02_date_lin[22] = buf_check_num(set_02_date_lin);

				sendto(sockfd_recv, set_02_date_lin, sizeof(set_02_date_lin), 0, (struct sockaddr*)&signal_addr, sizeof(signal_addr));
/*
				res = recvfrom(sockfd_recv, recv_buf, 512, 0, NULL, NULL);
				printf("recvfrom  date !!!!!!!!!!\n ");
				if(res <= 0)
				{
					close(sockfd_recv);
					return -1;
				}
				else
				{
					close(sockfd_recv);
					return 0;
				}
*/
				struct timeval 	timeVal;
					fd_set  fdSet;
					int ret;

					timeVal.tv_sec = signal_report_timeout;
					timeVal.tv_usec = 0;

					FD_ZERO(&fdSet);
					FD_SET(sockfd_recv, &fdSet);

					ret = select((int) sockfd_recv + 1, &fdSet,NULL, NULL, &timeVal);
					if(ret == 0)                     //��ʱ
					{
						close(sockfd_recv);
						return -1;
					}
					if(ret)
					{
						res = recvfrom(sockfd_recv, recv_buf, 512, 0, NULL, NULL);
						if(res <= 0)
						{
							close(sockfd_recv);
							return -1;
						}
						else
						{
							printf("recvfrom signal_id = %d,tc100 san pin lv report!!!!!!!!!!\n ",(recv_buf[7]*256+recv_buf[8]));
							close(sockfd_recv);
							return 0;
						}
					}
			
			}
			//itc100 ����Ƶ��
			else if(buf[10] == 0x22)
			{
				unsigned char set_22_date_lin[18];

				bzero(set_22_date_lin,sizeof(set_22_date_lin));

				set_22_date_lin[0] = 0x7e;set_22_date_lin[2]=0x10;
				set_22_date_lin[4]=0x01;
				set_22_date_lin[7] = buf[7];
				set_22_date_lin[8] = buf[8];
				set_22_date_lin[9] = 0x20;
				set_22_date_lin[10] = 0x22;
				sprintf(sql,"select cycle,duty_ratio,fast_cycle,fast_duty_ratio  from  ITC_CFG_FLASH  where signal_id = %d" ,buf[7] * 256 + buf[8]);
			
				printf("%s\n",sql);
				
				st->setSQL(sql);
				rs = st->executeQuery();
				if(rs->next() == 0)
				{
					close(sockfd_recv);
					return -1;
				}
				
				set_22_date_lin[12] = rs->getInt(1) ;
				set_22_date_lin[13] = rs->getInt(2) ;
				set_22_date_lin[14] = rs->getInt(3) ;
				set_22_date_lin[15] = rs->getInt(4);


				set_22_date_lin[16] = buf_check_num(set_22_date_lin);
				set_22_date_lin[17] = 0x7D;

				sendto(sockfd_recv, set_22_date_lin, sizeof(set_22_date_lin), 0, (struct sockaddr*)&signal_addr, sizeof(signal_addr));
/*
				res = recvfrom(sockfd_recv, recv_buf, 512, 0, NULL, NULL);
				printf("recvfrom  date !!!!!!!!!!\n ");
				if(res <= 0)
				{
					close(sockfd_recv);
					return -1;
				}
				else
				{
					close(sockfd_recv);
					return 0;
				}
*/
				struct timeval 	timeVal;
					fd_set  fdSet;
					int ret;

					timeVal.tv_sec = signal_report_timeout;
					timeVal.tv_usec = 0;

					FD_ZERO(&fdSet);
					FD_SET(sockfd_recv, &fdSet);

					ret = select((int) sockfd_recv + 1, &fdSet,NULL, NULL, &timeVal);
					if(ret == 0)                     //��ʱ
					{
						close(sockfd_recv);
						return -1;
					}
					if(ret)
					{
						res = recvfrom(sockfd_recv, recv_buf, 512, 0, NULL, NULL);
						if(res <= 0)
						{
							close(sockfd_recv);
							return -1;
						}
						else
						{
							printf("recvfrom signal_id = %d,itc100 san pin lv  report!!!!!!!!!!\n ",(recv_buf[7]*256+recv_buf[8]));
							close(sockfd_recv);
							return 0;
						}
					}				
			}
			/*���鿪�����ͣ�tc100*/
			else if(buf[10] == 0x11)
			{
				unsigned char set_11_date_lin[8][21];

				for(i = 0; i < 8;i++)
				{
					for(j = 0; j < 21;j++)
					{
						set_11_date_lin[i][j] = set_11_date[i][j];
					}
					
				}
				
				for(i = 0;i < 8;i++)
				{
					set_11_date_lin[i][7] = buf[7];
					set_11_date_lin[i][8] = buf[8];

					set_11_date_lin[i][12] = i+ 1;
					sprintf(sql,"select kaijidengse1,kaijishijian1,kaijidengse2,kaijishijian2, kaijidengse3,kaijishijian3  from  KAIJIDENGSE  where signal_id = %d and clink_id = %d" ,buf[7] * 256 + buf[8] ,i+1);
				//	printf("%s\n",sql);
					st->setSQL(sql);
					rs = st->executeQuery();
					if(rs->next() == 0)
					{
						set_11_date_lin[i][13] = 0;
						set_11_date_lin[i][14] = 0;
						set_11_date_lin[i][15] = 0;
						set_11_date_lin[i][16] = 0;
						set_11_date_lin[i][17] = 0;
						set_11_date_lin[i][18] = 0;
					}
					else
					{
						set_11_date_lin[i][13] = rs->getInt(1);
						set_11_date_lin[i][14] = rs->getInt(2)* 10;
						set_11_date_lin[i][15] = rs->getInt(3);
						set_11_date_lin[i][16] = rs->getInt(4) * 10;
						set_11_date_lin[i][17] = rs->getInt(5);
						set_11_date_lin[i][18] = rs->getInt(6) * 10;
					}

					set_11_date_lin[i][19] = buf_check_num(set_11_date_lin[i]);

#if 0
					int www;
					printf("send buf:");
					for(www = 0; www <21;www++)
						printf( "%02x", set_11_date_lin[i][www]);
					printf("\n");

#endif

					sendto(sockfd_recv, set_11_date_lin[i], sizeof(set_11_date_lin[i]), 0, (struct sockaddr*)&signal_addr, sizeof(signal_addr));
/*
					res = recvfrom(sockfd_recv, recv_buf, 512, 0, NULL, NULL);
					printf("recvfrom  date !!!!!!!!!!\n ");
					if(res <= 0)
					{
						close(sockfd_recv);
						return -1;
					}
					else
					{
						
					}
*/
					struct timeval 	timeVal;
					fd_set  fdSet;
					int ret;

					timeVal.tv_sec = signal_report_timeout;
					timeVal.tv_usec = 0;

					FD_ZERO(&fdSet);
					FD_SET(sockfd_recv, &fdSet);

					ret = select((int) sockfd_recv + 1, &fdSet,NULL, NULL, &timeVal);
					if(ret == 0)                     //��ʱ
					{
						close(sockfd_recv);
						return -1;
					}
					if(ret)
					{
						res = recvfrom(sockfd_recv, recv_buf, 512, 0, NULL, NULL);
						if(res <= 0)
						{
							close(sockfd_recv);
							return -1;
						}
						else
						{
							printf("recvfrom signal_id = %d,tc100 kai ji lei xing report!!!!!!!!!!\n ",(recv_buf[7]*256+recv_buf[8]));
						}
					}
					
				}
				close(sockfd_recv);
				return 0;
				
			}
			//itc100 ,���鿪������
			else if(buf[10] == 0x21)
			{
				unsigned char itcSet_21_data[206];
				bzero(itcSet_21_data,206);

				itcSet_21_data[0]= 0x7E;itcSet_21_data[2]=0XCC;//204
				itcSet_21_data[4]= 0X01;//version
				itcSet_21_data[7]=buf[7];itcSet_21_data[8]=buf[8];
				itcSet_21_data[9]=0x20;itcSet_21_data[10]=0x21;
				itcSet_21_data[205] = 0x7d;

				sprintf(sql,"select phase_id,color1,time1,color2,time2, color3,time3  from  ITC_CFG_STARTUP_COLOR  where signal_id = %d order by phase_id" ,buf[7] * 256 + buf[8]);

				st->setSQL(sql);
				rs = st->executeQuery();

				while(rs->next())
				{
					itcSet_21_data[12 + (rs->getInt(1)-1) *6] = rs->getInt(2);
					itcSet_21_data[13 + (rs->getInt(1)-1) *6] = rs->getInt(3);
					itcSet_21_data[14 + (rs->getInt(1)-1) *6] = rs->getInt(4);
					itcSet_21_data[15 + (rs->getInt(1)-1) *6] = rs->getInt(5);
					itcSet_21_data[16 + (rs->getInt(1)-1) *6] = rs->getInt(6);
					itcSet_21_data[17 + (rs->getInt(1)-1) *6] = rs->getInt(7);
				}

				itcSet_21_data[204] = buf_check_num(itcSet_21_data);

				sendto(sockfd_recv, itcSet_21_data, sizeof(itcSet_21_data), 0, (struct sockaddr*)&signal_addr, sizeof(signal_addr));
/*
					res = recvfrom(sockfd_recv, recv_buf, 512, 0, NULL, NULL);
					printf("recvfrom  date !!!!!!!!!!\n ");
					if(res <= 0)
					{
						close(sockfd_recv);
						return -1;
					}
					else
					{
						close(sockfd_recv);
						return 0;
					}
*/
					struct timeval 	timeVal;
					fd_set  fdSet;
					int ret;

					timeVal.tv_sec = signal_report_timeout;
					timeVal.tv_usec = 0;

					FD_ZERO(&fdSet);
					FD_SET(sockfd_recv, &fdSet);

					ret = select((int) sockfd_recv + 1, &fdSet,NULL, NULL, &timeVal);
					if(ret == 0)                     //��ʱ
					{
						close(sockfd_recv);
						return -1;
					}
					if(ret)
					{
						res = recvfrom(sockfd_recv, recv_buf, 512, 0, NULL, NULL);
						if(res <= 0)
						{
							close(sockfd_recv);
							return -1;
						}
						else
						{
							printf("recvfrom signal_id = %d,itc100 kai ji lei xing report!!!!!!!!!!\n ",(recv_buf[7]*256+recv_buf[8]));
							close(sockfd_recv);
							return 0;
						}
					}
					
			}
			/*���������*/
			else if(buf[10] == 0x12)
			{
				unsigned char set_12_date_lin[8][21];

				for(i = 0; i < 8;i++)
				{
					for(j = 0; j < 21;j++)
					{
						set_12_date_lin[i][j] = set_12_date[i][j];
					}
					
				}
			
				for(i = 0;i < 8;i++)
				{
					set_12_date_lin[i][7] = buf[7];
					set_12_date_lin[i][8] = buf[8];
					set_12_date_lin[i][12] = i+ 1;
					
					sprintf(sql,"select red_green_1,red_green_time1, red_green_2,red_green_time2, red_green_3,red_green_time3  from  GUODUDENGSE  where signal_id = %d and clink_id = %d" ,buf[7] * 256 + buf[8] ,i+1);
				//	printf("%s\n",sql);
					st->setSQL(sql);
					rs = st->executeQuery();
					if(rs->next() == 0)
					{
						set_12_date_lin[i][13] = 0;
						set_12_date_lin[i][14] = 0;
						set_12_date_lin[i][15] = 0;
						set_12_date_lin[i][16] = 0;
						set_12_date_lin[i][17] = 0;
						set_12_date_lin[i][18] = 0;
					}
					else
					{
						set_12_date_lin[i][13] = rs->getInt(1);
						set_12_date_lin[i][14] = rs->getInt(2)* 10;
						set_12_date_lin[i][15] = rs->getInt(3);
						set_12_date_lin[i][16] = rs->getInt(4) * 10;
						set_12_date_lin[i][17] = rs->getInt(5);
						set_12_date_lin[i][18] = rs->getInt(6) * 10;
					}

					set_12_date_lin[i][19] = buf_check_num(set_12_date_lin[i]);

					sendto(sockfd_recv, set_12_date_lin[i], sizeof(set_12_date_lin[i]), 0, (struct sockaddr*)&signal_addr, sizeof(signal_addr));
#if 0
					int www;
					printf("send buf:");
					for(www = 0; www <21;www++)
						printf( "%02x", set_12_date_lin[i][www]);
					printf("\n");

#endif
/*

					res = recvfrom(sockfd_recv, recv_buf, 512, 0, NULL, NULL);
					printf("recvfrom  date !!!!!!!!!!\n ");
					if(res <= 0)
					{
						close(sockfd_recv);
						return -1;
					}
					else
					{
						
					}

		*/
					struct timeval 	timeVal;
					fd_set  fdSet;
					int ret;

					timeVal.tv_sec = signal_report_timeout;
					timeVal.tv_usec = 0;

					FD_ZERO(&fdSet);
					FD_SET(sockfd_recv, &fdSet);

					ret = select((int) sockfd_recv + 1, &fdSet,NULL, NULL, &timeVal);
					if(ret == 0)                     //��ʱ
					{
						close(sockfd_recv);
						return -1;
					}
					if(ret)
					{
						res = recvfrom(sockfd_recv, recv_buf, 512, 0, NULL, NULL);
						if(res <= 0)
						{
							close(sockfd_recv);
							return -1;
						}
						else
						{
							printf("recvfrom signal_id = %d,red2green report  %d !!!!!!!!!\n ",(recv_buf[7]*256+recv_buf[8]),i);
							
						}
					}			
				}
				close(sockfd_recv);
				return 0;
				
			}
			/*�̱������*/
			else if(buf[10] == 0x13)
			{

				unsigned char set_13_date_lin[8][21];

				for(i = 0; i < 8;i++)
				{
					for(j = 0; j < 21;j++)
					{
						set_13_date_lin[i][j] = set_13_date[i][j];
					}
					
				}
					
				for(i = 0;i < 8;i++)
				{
					set_13_date_lin[i][7] = buf[7];
					set_13_date_lin[i][8] = buf[8];
					
					set_13_date_lin[i][12] = i+ 1;
					sprintf(sql,"select green_red_1,green_red_time1, green_red_2,green_red_time2, green_red_3,green_red_time3  from  GUODUDENGSE  where signal_id = %d and clink_id = %d" ,buf[7] * 256 + buf[8] ,i+1);
				//	printf("%s\n",sql);
					st->setSQL(sql);
					rs = st->executeQuery();
					if(rs->next() == 0)
					{
						set_13_date_lin[i][13] = 0;
						set_13_date_lin[i][14] = 0;
						set_13_date_lin[i][15] = 0;
						set_13_date_lin[i][16] = 0;
						set_13_date_lin[i][17] = 0;
						set_13_date_lin[i][18] = 0;
					}
					else
					{
						set_13_date_lin[i][13] = rs->getInt(1);
						set_13_date_lin[i][14] = rs->getInt(2)* 10;
						set_13_date_lin[i][15] = rs->getInt(3);
						set_13_date_lin[i][16] = rs->getInt(4) * 10;
						set_13_date_lin[i][17] = rs->getInt(5);
						set_13_date_lin[i][18] = rs->getInt(6) * 10;
					}

					set_13_date_lin[i][19] = buf_check_num(set_13_date_lin[i]);

					sendto(sockfd_recv, set_13_date_lin[i], sizeof(set_13_date_lin[i]), 0, (struct sockaddr*)&signal_addr, sizeof(signal_addr));
#if 0
					int www;
					printf("send buf:");
					for(www = 0; www <21;www++)
						printf( "%02x", set_13_date_lin[i][www]);
					printf("\n");

#endif
/*
					res = recvfrom(sockfd_recv, recv_buf, 512, 0, NULL, NULL);
					printf("recvfrom  date !!!!!!!!!!\n ");
					if(res <= 0)
					{
						close(sockfd_recv);
						return -1;
					}
					else
					{
						
					}

	*/
					struct timeval 	timeVal;
					fd_set  fdSet;
					int ret;

					timeVal.tv_sec = signal_report_timeout;
					timeVal.tv_usec = 0;

					FD_ZERO(&fdSet);
					FD_SET(sockfd_recv, &fdSet);

					ret = select((int) sockfd_recv + 1, &fdSet,NULL, NULL, &timeVal);
					if(ret == 0)                     //��ʱ
					{
						close(sockfd_recv);
						return -1;
					}
					if(ret)
					{
						res = recvfrom(sockfd_recv, recv_buf, 512, 0, NULL, NULL);
						if(res <= 0)
						{
							close(sockfd_recv);
							return -1;
						}
						else
						{
							printf("recvfrom signal_id = %d,green2red report %d !!!!!!!!!!\n ",(recv_buf[7]*256+recv_buf[8]),i);
							
						}
					}				
				}
				close(sockfd_recv);
				return 0;
				
			}
			//itc100,���ɵ�ɫ
			else if(buf[10] == 0x20)
			{
				unsigned char itcSet_20_data[398];
				bzero(itcSet_20_data,398);

				itcSet_20_data[0]= 0x7E;
				itcSet_20_data[1]=0X01;
				itcSet_20_data[2]=0X8C;//204
				itcSet_20_data[4]= 0X01;//version
				itcSet_20_data[7]=buf[7];
				itcSet_20_data[8]=buf[8];
				itcSet_20_data[9]=0x20;
				itcSet_20_data[10]=0x20;
				itcSet_20_data[397] = 0x7d;
#if 1
				sprintf(sql,"select phase_id,red_green_1,red_green_time1,red_green_2,red_green_time2, red_green_3,red_green_time3  from  ITC_CFG_TRANS_COLOR  where signal_id = %d order by phase_id" ,buf[7] * 256 + buf[8]);

				st->setSQL(sql);
				rs = st->executeQuery();

				while(rs->next())
				{
					itcSet_20_data[12 +(rs->getInt(1)-1) *6] = rs->getInt(2);
					itcSet_20_data[13 + (rs->getInt(1)-1) *6] = rs->getInt(3);
					itcSet_20_data[14 + (rs->getInt(1)-1) *6] = rs->getInt(4);
					itcSet_20_data[15 + (rs->getInt(1)-1) *6] = rs->getInt(5);
					itcSet_20_data[16 + (rs->getInt(1)-1) *6] = rs->getInt(6);
					itcSet_20_data[17 + (rs->getInt(1)-1) *6] = rs->getInt(7);
					
				}

				sprintf(sql,"select phase_id,green_red_1,green_red_time1,green_red_2,green_red_time2, green_red_3,green_red_time3  from  ITC_CFG_TRANS_COLOR  where signal_id = %d order by phase_id" ,buf[7] * 256 + buf[8]);

				st->setSQL(sql);
				rs = st->executeQuery();

				while(rs->next())
				{
					itcSet_20_data[204 + (rs->getInt(1)-1) *6] = rs->getInt(2);
					itcSet_20_data[205 + (rs->getInt(1)-1) *6] = rs->getInt(3);
					itcSet_20_data[206 + (rs->getInt(1)-1) *6] = rs->getInt(4);
					itcSet_20_data[207 + (rs->getInt(1)-1) *6] = rs->getInt(5);
					itcSet_20_data[208 + (rs->getInt(1)-1) *6] = rs->getInt(6);
					itcSet_20_data[209 + (rs->getInt(1)-1) *6] = rs->getInt(7);
				}
#endif


				itcSet_20_data[396] = buf_check_num(itcSet_20_data);

				sendto(sockfd_recv, itcSet_20_data, sizeof(itcSet_20_data), 0, (struct sockaddr*)&signal_addr, sizeof(signal_addr));
				printf("send guo du deng se\n");
				unsigned int count;
				for(count = 0;count < sizeof(itcSet_20_data);count ++)
					printf("%2x ",itcSet_20_data[count]);
				printf("\n");
/*
					res = recvfrom(sockfd_recv, recv_buf, 512, 0, NULL, NULL);
					printf("recvfrom  date !!!!!!!!!!\n ");
					if(res <= 0)
					{
						close(sockfd_recv);
						return -1;
					}
					else
					{
						close(sockfd_recv);
						return 0;
					}
*/
					struct timeval 	timeVal;
					fd_set  fdSet;
					int ret;

					timeVal.tv_sec = signal_report_timeout;
					timeVal.tv_usec = 0;

					FD_ZERO(&fdSet);
					FD_SET(sockfd_recv, &fdSet);

					ret = select((int) sockfd_recv + 1, &fdSet,NULL, NULL, &timeVal);
					if(ret == 0)                     //��ʱ
					{
						close(sockfd_recv);
						return -1;
					}
					if(ret)
					{
						res = recvfrom(sockfd_recv, recv_buf, 512, 0, NULL, NULL);
						if(res <= 0)
						{
							close(sockfd_recv);
							return -1;
						}
						else
						{
							printf("recvfrom signal_id = %d,itc100 guo du deng se report!!!!!!!!!!\n ",(recv_buf[7]*256+recv_buf[8]));
							close(sockfd_recv);
							return 0;
						}
					}
					
			}
			/*��λ��������,tc100*/
			else if(buf[10] == 0x03)
			{
				unsigned char set_03_date_lin[16][184+5];

				for(i = 0; i < 16;i++)
				{
					for(j = 0; j < (184+5);j++)
					{
						set_03_date_lin[i][j] = set_03_date[i][j];
					}
					
				}
				
				for(i = 0;i < 16;i++)
				{
					set_03_date_lin[i][7] = buf[7];
					set_03_date_lin[i][8] = buf[8];

					set_03_date_lin[i][12] = i+ 1;
					
					sprintf(sql,"select CLINK_TYPE ,GREEN_MIN,GREEN_MAX,GREEN_LONG,CLINK_APPLY, CLINK_LONG,FOLLOW_CLINK_ID,LATE_GREEN_TIME,LONG_GREEN_TIME,SIGNAL_START_TYPE,RED_GREEN_TYPE, GREEN_RED_TYPE,PULSE_WIDTH_TIME,PEDESTRIAN_EMPTY_FLAG,PEDESTRIAN_EMPTY_TIME  from  CLINK_INFO  where signal_id = %d and clink_id = %d" ,buf[7] * 256 + buf[8] ,i+1);
				//	printf("%s\n",sql);
					st->setSQL(sql);
					rs = st->executeQuery();
					if(rs->next() == 0)
					{
						for(j = 13;j<55+5;j++)
						{
							set_03_date_lin[i][j] = 0;
						}
						
					}
					else
					{
						set_03_date_lin[i][13] = rs->getInt(1);
						set_03_date_lin[i][14] = (rs->getInt(2)) >> 8;
						set_03_date_lin[i][15] = (rs->getInt(2))  & 0xff;
						set_03_date_lin[i][16] = (rs->getInt(3)) >> 8;
						set_03_date_lin[i][17] = (rs->getInt(3))  & 0xff;
						set_03_date_lin[i][18] = (rs->getInt(4)) >> 8;
						set_03_date_lin[i][19] = (rs->getInt(4))  & 0xff;

						/*��λ����������*/
						bzero(lin_buf,sizeof(lin_buf));
						sprintf(lin_buf,"%s",rs->getString(5).c_str());
						result = strtok( lin_buf, "," );

						for(j = 0;j <13;j++)
						{
							set_03_date_lin[i][20 + j] = 0;
						}
						
						while( result != NULL ) 
						{
						  
							lin_res_num= atoi(result);
							  if(i == 1)
						    	{
						    		printf("!!!!!!!!!!!!!!%d",lin_res_num);
						    	}
							if(lin_res_num>=1  &&  lin_res_num<=8)
							{
								set_03_date_lin[i][20] += (0x80 >> (lin_res_num - 1));
							}
							else if(lin_res_num>=9  &&  lin_res_num<=16)
							{
								if(lin_res_num % 8 == 0)
								{
									set_03_date_lin[i][21] += (0x80 >> (7));
								}
								else
								{
									set_03_date_lin[i][21] += (0x80 >> (lin_res_num % 8 - 1));
								}
								
							}
							else if(lin_res_num>=17  &&  lin_res_num<=24)
							{
								if(lin_res_num % 8 == 0)
								{
									set_03_date_lin[i][22] += (0x80 >> (7));
								}
								else
								{
									set_03_date_lin[i][22] += (0x80 >> (lin_res_num % 8 - 1));
								}
							}
							else if(lin_res_num>=25 &&  lin_res_num<=32)
							{
								if(lin_res_num % 8 == 0)
								{
									set_03_date_lin[i][23] += (0x80 >> (7));
								}
								else
								{
									set_03_date_lin[i][23] += (0x80 >> (lin_res_num % 8 - 1));
								}
							}
							else if(lin_res_num>=33  &&  lin_res_num<=40)
							{
								if(lin_res_num % 8 == 0)
								{
									set_03_date_lin[i][24] += (0x80 >> (7));
								}
								else
								{
									set_03_date_lin[i][24] += (0x80 >> (lin_res_num % 8 - 1));
								}
							}
							else if(lin_res_num>=41  &&  lin_res_num<=48)
							{
								if(lin_res_num % 8 == 0)
								{
									set_03_date_lin[i][25] += (0x80 >> (7));
								}
								else
								{
									set_03_date_lin[i][26] += (0x80 >> (lin_res_num % 8 - 1));
								}
							}
							else if(lin_res_num>=49  &&  lin_res_num<=56)
							{
								if(lin_res_num % 8 == 0)
								{
									set_03_date_lin[i][26] += (0x80 >> (7));
								}
								else
								{
									set_03_date_lin[i][26] += (0x80 >> (lin_res_num % 8 - 1));
								}
							}
							else if(lin_res_num>=57  &&  lin_res_num<=64)
							{
								if(lin_res_num % 8 == 0)
								{
									set_03_date_lin[i][27] += (0x80 >> (7));
								}
								else
								{
									set_03_date_lin[i][27] += (0x80 >> (lin_res_num % 8 - 1));
								}
							}
							else if(lin_res_num>=65  &&  lin_res_num<=72)
							{
								if(lin_res_num % 8 == 0)
								{
									set_03_date_lin[i][28] += (0x80 >> (7));
								}
								else
								{
									set_03_date_lin[i][28] += (0x80 >> (lin_res_num % 8 - 1));
								}
							}
							else if(lin_res_num>=73  &&  lin_res_num<=80)
							{
								if(lin_res_num % 8 == 0)
								{
									set_03_date_lin[i][29] += (0x80 >> (7));
								}
								else
								{
									set_03_date_lin[i][29] += (0x80 >> (lin_res_num % 8 - 1));
								}
							}
							else if(lin_res_num>=81  &&  lin_res_num<=88)
							{
								if(lin_res_num % 8 == 0)
								{
									set_03_date_lin[i][30] += (0x80 >> (7));
								}
								else
								{
									set_03_date_lin[i][30] += (0x80 >> (lin_res_num % 8 - 1));
								}
							}
							else if(lin_res_num>=89  &&  lin_res_num<=96)
							{
								if(lin_res_num % 8 == 0)
								{
									set_03_date_lin[i][31] += (0x80 >> (7));
								}
								else
								{
									set_03_date_lin[i][31] += (0x80 >> (lin_res_num % 8 - 1));
								}
							}
							else if(lin_res_num>=97  &&  lin_res_num<=104)
							{
								if(lin_res_num % 8 == 0)
								{
									set_03_date_lin[i][32] += (0x80 >> (7));
								}
								else
								{
									set_03_date_lin[i][32] += (0x80 >> (lin_res_num % 8 - 1));
								}
							}
							
						   	result = strtok( NULL, "," );
						}


						/*��λ�ӳ��������*/
						bzero(lin_buf,sizeof(lin_buf));
						sprintf(lin_buf,"%s",rs->getString(6).c_str());
						result = strtok( lin_buf, "," );

						for(j = 0;j <13;j++)
						{
							set_03_date_lin[i][33 + j] = 0;
						}
						
						while( result != NULL ) 
						{
						    	
							lin_res_num= atoi(result);
							if(lin_res_num>=1  &&  lin_res_num<=8)
							{
								if(lin_res_num % 8 == 0)
								{
									set_03_date_lin[i][33] += (0x80 >> (7));
								}
								else
								{
									set_03_date_lin[i][33] += (0x80 >> (lin_res_num % 8 - 1));
								}
							}
							else if(lin_res_num>=9  &&  lin_res_num<=16)
							{
								if(lin_res_num % 8 == 0)
								{
									set_03_date_lin[i][34] += (0x80 >> (7));
								}
								else
								{
									set_03_date_lin[i][34] += (0x80 >> (lin_res_num % 8 - 1));
								}
							}
							else if(lin_res_num>=17  &&  lin_res_num<=24)
							{
								if(lin_res_num % 8 == 0)
								{
									set_03_date_lin[i][35] += (0x80 >> (7));
								}
								else
								{
									set_03_date_lin[i][35] += (0x80 >> (lin_res_num % 8 - 1));
								}
							}
							else if(lin_res_num>=25 &&  lin_res_num<=32)
							{
								if(lin_res_num % 8 == 0)
								{
									set_03_date_lin[i][36] += (0x80 >> (7));
								}
								else
								{
									set_03_date_lin[i][36] += (0x80 >> (lin_res_num % 8 - 1));
								}
							}
							else if(lin_res_num>=33  &&  lin_res_num<=40)
							{
								if(lin_res_num % 8 == 0)
								{
									set_03_date_lin[i][37] += (0x80 >> (7));
								}
								else
								{
									set_03_date_lin[i][37] += (0x80 >> (lin_res_num % 8 - 1));
								}
							}
							else if(lin_res_num>=41  &&  lin_res_num<=48)
							{
								if(lin_res_num % 8 == 0)
								{
									set_03_date_lin[i][38] += (0x80 >> (7));
								}
								else
								{
									set_03_date_lin[i][38] += (0x80 >>(lin_res_num % 8 - 1));
								}
							}
							else if(lin_res_num>=49  &&  lin_res_num<=56)
							{
								if(lin_res_num % 8 == 0)
								{
									set_03_date_lin[i][39] += (0x80 >> (7));
								}
								else
								{
									set_03_date_lin[i][39] += (0x80 >> (lin_res_num % 8 - 1));
								}
							}
							else if(lin_res_num>=57  &&  lin_res_num<=64)
							{
								if(lin_res_num % 8 == 0)
								{
									set_03_date_lin[i][40] += (0x80 >> (7));
								}
								else
								{
									set_03_date_lin[i][40] += (0x80 >> (lin_res_num % 8 - 1));
								}
							}
							else if(lin_res_num>=65  &&  lin_res_num<=72)
							{
								if(lin_res_num % 8 == 0)
								{
									set_03_date_lin[i][41] += (0x80 >> (7));
								}
								else
								{
									set_03_date_lin[i][41] += (0x80 >> (lin_res_num % 8 - 1));
								}
							}
							else if(lin_res_num>=73  &&  lin_res_num<=80)
							{
								if(lin_res_num % 8 == 0)
								{
									set_03_date_lin[i][42] += (0x80 >> (7));
								}
								else
								{
									set_03_date_lin[i][42] += (0x80 >> (lin_res_num % 8 - 1));
								}
							}
							else if(lin_res_num>=81  &&  lin_res_num<=88)
							{
								if(lin_res_num % 8 == 0)
								{
									set_03_date_lin[i][43] += (0x80 >> (7));
								}
								else
								{
									set_03_date_lin[i][43] += (0x80 >> (lin_res_num % 8 - 1));
								}
							}
							else if(lin_res_num>=89  &&  lin_res_num<=96)
							{
								if(lin_res_num % 8 == 0)
								{
									set_03_date_lin[i][44] += (0x80 >> (7));
								}
								else
								{
									set_03_date_lin[i][44] += (0x80 >> (lin_res_num % 8 - 1));
								}
							}
							else if(lin_res_num>=97  &&  lin_res_num<=104)
							{
								if(lin_res_num % 8 == 0)
								{
									set_03_date_lin[i][45] += (0x80 >> (7));
								}
								else
								{
									set_03_date_lin[i][45] += (0x80 >> (lin_res_num % 8 - 1));
								}
							}
							
						   	result = strtok( NULL, "," );
						}

						set_03_date_lin[i][46] =  rs->getInt(7);
						set_03_date_lin[i][47] =  (rs->getInt(8) ) >> 8;
						set_03_date_lin[i][48] =  (rs->getInt(8) ) & 0xff;
						set_03_date_lin[i][49] =  (rs->getInt(9) ) >> 8;
						set_03_date_lin[i][50] =  (rs->getInt(9) ) & 0xff;
							
						set_03_date_lin[i][51 +5] =  rs->getInt(10);
						set_03_date_lin[i][52+ 5] =  rs->getInt(11);
						set_03_date_lin[i][53 + 5] =  rs->getInt(12);

						set_03_date_lin[i][51] =  (rs->getInt(13) ) >> 8;
						set_03_date_lin[i][52] =  (rs->getInt(13) ) & 0xff;
						set_03_date_lin[i][53] =  (rs->getInt(14) ) ;
						set_03_date_lin[i][54] =  (rs->getInt(15) ) >> 8;
						set_03_date_lin[i][55] =  (rs->getInt(15) ) & 0xff;

						
						
					}
					set_03_date_lin[i][182+5] = buf_check_num(set_03_date_lin[i]);

					sendto(sockfd_recv, set_03_date_lin[i], sizeof(set_03_date_lin[i]), 0, (struct sockaddr*)&signal_addr, sizeof(signal_addr));
/*
					res = recvfrom(sockfd_recv, recv_buf, 512, 0, NULL, NULL);
					printf("recvfrom  date !!!!!!!!!!\n ");
					if(res <= 0)
					{
						close(sockfd_recv);
						return -1;
					}
					else
					{	
						
						
					}
	*/
					struct timeval 	timeVal;
					fd_set  fdSet;
					int ret;

					timeVal.tv_sec = signal_report_timeout;
					timeVal.tv_usec = 0;

					FD_ZERO(&fdSet);
					FD_SET(sockfd_recv, &fdSet);

					ret = select((int) sockfd_recv + 1, &fdSet,NULL, NULL, &timeVal);
					if(ret == 0)                     //��ʱ
					{
						close(sockfd_recv);
						return -1;
					}
					if(ret)
					{
						res = recvfrom(sockfd_recv, recv_buf, 512, 0, NULL, NULL);
						if(res <= 0)
						{
							close(sockfd_recv);
							return -1;
						}
						else
						{
							printf("recvfrom signal_id = %d,TC100 xiang wei she zhi %d!!!!!!!!!!\n ",(recv_buf[7]*256+recv_buf[8]),i);
							
						}
					}		
				}
				close(sockfd_recv);
				return 0;
				
			}

			/*ITC100�� ��λ����*/
			else if(buf[10] == 0x23)
				{
				unsigned char set_23_date_lin[32][34],set_24_date_lin[46];
				
				bzero(set_24_date_lin,46);
				
				set_24_date_lin[0]= 0x7e;set_24_date_lin[2] = 0x2c;
				set_24_date_lin[4]=0x01;
				set_24_date_lin[7] = buf[7];
				set_24_date_lin[8] = buf[8];
				set_24_date_lin[9] = 0x20;set_24_date_lin[10]=0x24;set_24_date_lin[45] = 0x7d;
					
				for(i = 0;i < 32;i++)
				{
					bzero(set_23_date_lin[i],34);
					set_23_date_lin[i][0]= 0x7e;set_23_date_lin[i][2] = 0x20;
					set_23_date_lin[i][4]=0x01;
					set_23_date_lin[i][7] = buf[7];
					set_23_date_lin[i][8] = buf[8];
					set_23_date_lin[i][9] = 0x20;set_23_date_lin[i][10]=0x23;

					set_23_date_lin[i][11] = i;
					set_23_date_lin[i][12] = i+ 1;

					set_23_date_lin[i][33] = 0x7d;
					
					sprintf(sql,"select PHASE_TYPE ,GREEN_MIN,GREEN_MAX,GREEN_LONG,DETECTOR_APPLY, DETECTOR_LONG  from  ITC_CFG_PHASE  where signal_id = %d and phase_id = %d" ,buf[7] * 256 + buf[8] ,i+1);
				//	printf("%s\n",sql);
					st->setSQL(sql);
					rs = st->executeQuery();
					if(rs->next() != 0)
					{
						set_23_date_lin[i][13] = rs->getInt(1);
						if(set_23_date_lin[i][13] == 3) //������λ
							set_24_date_lin[12+i] = 0x01;
						else
							set_24_date_lin[12+i] = 0x00;
						
						if(set_23_date_lin[i][13] >= 0x01 && set_23_date_lin[i][13] <= 0x05)
							set_23_date_lin[i][14] = 0X01;//����
						else
							set_23_date_lin[i][14] = 0X00;//δ����
							
						set_23_date_lin[i][15] = (rs->getInt(2)) >> 8;
						set_23_date_lin[i][16] = (rs->getInt(2))  & 0xff;//��С��
						set_23_date_lin[i][17] = (rs->getInt(3)) >> 8;
						set_23_date_lin[i][18] = (rs->getInt(3))  & 0xff;//�����
						set_23_date_lin[i][19] = set_23_date_lin[i][17];
						set_23_date_lin[i][20] = set_23_date_lin[i][18];
						set_23_date_lin[i][21] = (rs->getInt(4)) >> 8;
						set_23_date_lin[i][22] = (rs->getInt(4))  & 0xff;

						/*��λ����������*/
						bzero(lin_buf,sizeof(lin_buf));
						sprintf(lin_buf,"%s",rs->getString(5).c_str());
						result = strtok( lin_buf, "," );

						for(j = 0;j <4;j++)
						{
							set_23_date_lin[i][23 + j] = 0;
						}
						
						while( result != NULL ) 
						{
						  
							lin_res_num= atoi(result);

							if(lin_res_num>=1  &&  lin_res_num<=8)
							{
								set_23_date_lin[i][23] += (0x80 >> (lin_res_num - 1));
							}
							else if(lin_res_num>=9  &&  lin_res_num<=16)
							{
								if(lin_res_num % 8 == 0)
								{
									set_23_date_lin[i][24] += (0x80 >> (7));
								}
								else
								{
									set_23_date_lin[i][24] += (0x80 >> (lin_res_num % 8 - 1));
								}
								
							}
							else if(lin_res_num>=17  &&  lin_res_num<=24)
							{
								if(lin_res_num % 8 == 0)
								{
									set_23_date_lin[i][25] += (0x80 >> (7));
								}
								else
								{
									set_23_date_lin[i][25] += (0x80 >> (lin_res_num % 8 - 1));
								}
							}
							else if(lin_res_num>=25 &&  lin_res_num<=32)
							{
								if(lin_res_num % 8 == 0)
								{
									set_23_date_lin[i][26] += (0x80 >> (7));
								}
								else
								{
									set_23_date_lin[i][26] += (0x80 >> (lin_res_num % 8 - 1));
								}
							}
							
																	
							
						   	result = strtok( NULL, "," );
						}


						/*��λ�ӳ��������*/
						bzero(lin_buf,sizeof(lin_buf));
						sprintf(lin_buf,"%s",rs->getString(6).c_str());
						result = strtok( lin_buf, "," );

						for(j = 0;j <4;j++)
						{
							set_23_date_lin[i][27 + j] = 0;
						}
						
						while( result != NULL ) 
						{
						    	
							lin_res_num= atoi(result);
							if(lin_res_num>=1  &&  lin_res_num<=8)
							{
								if(lin_res_num % 8 == 0)
								{
									set_23_date_lin[i][27] += (0x80 >> (7));
								}
								else
								{
									set_23_date_lin[i][27] += (0x80 >> (lin_res_num % 8 - 1));
								}
							}
							else if(lin_res_num>=9  &&  lin_res_num<=16)
							{
								if(lin_res_num % 8 == 0)
								{
									set_23_date_lin[i][28] += (0x80 >> (7));
								}
								else
								{
									set_23_date_lin[i][28] += (0x80 >> (lin_res_num % 8 - 1));
								}
							}
							else if(lin_res_num>=17  &&  lin_res_num<=24)
							{
								if(lin_res_num % 8 == 0)
								{
									set_23_date_lin[i][29] += (0x80 >> (7));
								}
								else
								{
									set_23_date_lin[i][29] += (0x80 >> (lin_res_num % 8 - 1));
								}
							}
							else if(lin_res_num>=25 &&  lin_res_num<=32)
							{
								if(lin_res_num % 8 == 0)
								{
									set_23_date_lin[i][30] += (0x80 >> (7));
								}
								else
								{
									set_23_date_lin[i][30] += (0x80 >> (lin_res_num % 8 - 1));
								}
							}
							
						   	result = strtok( NULL, "," );
						}

						set_23_date_lin[i][31] =  0x00;
						
						
					}
					
					
					set_23_date_lin[i][32] = buf_check_num(set_23_date_lin[i]);
					sendto(sockfd_recv, set_23_date_lin[i], sizeof(set_23_date_lin[i]), 0, (struct sockaddr*)&signal_addr, sizeof(signal_addr));

					/*
					res = recvfrom(sockfd_recv, recv_buf, 512, 0, NULL, NULL);
					printf("recvfrom signal_id = %d,xiang wei %d report!!!!!!!!!!\n ",(recv_buf[6]<<8+recv_buf[7]),i);
					if(res <= 0)
					{
						close(sockfd_recv);
						return -1;
					}
					*/
					struct timeval 	timeVal;
					fd_set  fdSet;
					int ret;

					timeVal.tv_sec = signal_report_timeout;
					timeVal.tv_usec = 0;

					FD_ZERO(&fdSet);
					FD_SET(sockfd_recv, &fdSet);

					ret = select((int) sockfd_recv + 1, &fdSet,NULL, NULL, &timeVal);
					if(ret == 0)                     //��ʱ
					{
						close(sockfd_recv);
						return -1;
					}
					if(ret)
					{
						res = recvfrom(sockfd_recv, recv_buf, 512, 0, NULL, NULL);
						if(res <= 0)
						{
							close(sockfd_recv);
							return -1;
						}
						printf("recvfrom signal_id = %d,ITC100 xiang wei %d report!!!!!!!!!!\n ",(recv_buf[7]*256+recv_buf[8]),i);
					}
			

					
			
				}

				set_24_date_lin[44] = buf_check_num(set_24_date_lin);
					sendto(sockfd_recv, set_24_date_lin, sizeof(set_24_date_lin), 0, (struct sockaddr*)&signal_addr, sizeof(signal_addr));
/*
					res = recvfrom(sockfd_recv, recv_buf, 512, 0, NULL, NULL);
					printf("recvfrom  date 24!!!!!!!!!!\n ");
					if(res <= 0)
					{
						close(sockfd_recv);
						return -1;
					}
	*/
					struct timeval 	timeVal;
					fd_set  fdSet;
					int ret;

					timeVal.tv_sec = signal_report_timeout;
					timeVal.tv_usec = 0;

					FD_ZERO(&fdSet);
					FD_SET(sockfd_recv, &fdSet);

					ret = select((int) sockfd_recv + 1, &fdSet,NULL, NULL, &timeVal);
					if(ret == 0)                     //��ʱ
					{
						close(sockfd_recv);
						return -1;
					}
					if(ret)
					{
						res = recvfrom(sockfd_recv, recv_buf, 512, 0, NULL, NULL);
						if(res <= 0)
						{
							close(sockfd_recv);
							return -1;
						}
						else
						{
							printf("recvfrom signal_id = %d,0X24 report!!!!!!!!!!\n ",(recv_buf[7]*256+recv_buf[8]));
							close(sockfd_recv);
							return 0;
						}
					}				
				
				
			}

			
			/*TC100�׶���������*/
			else if(buf[10] == 0x04)
			{
				unsigned char set_04_date_lin[64][174];

				for(i = 0; i < 64;i++)
				{
					for(j = 0; j < 174;j++)
					{
						set_04_date_lin[i][j] = set_04_date[i][j];
					}
					
				}
				
				for(i = 0; i < 64 ;i++)
				{
					set_04_date_lin[i][7] = buf[7];
					set_04_date_lin[i][8] = buf[8];
					set_04_date_lin[i][12] = i + 1;
					
					sprintf(sql,"select Stage_clink, App_detector,Extend_Detector, Occ_auto,Buchang_auto,Stat_time_auto  from  STAGE_INFO  where signal_id = %d and Stage_id= %d" ,buf[7] * 256 + buf[8] ,i+1);
					//printf("%s\n",sql);
					st->setSQL(sql);
					rs = st->executeQuery();
					if(rs->next() == 0)
					{
						for(j = 13;j<44;j++)
						{
							set_04_date_lin[i][j] = 0;
						}
						
					}
					else
					{
						/*��λ����������*/
						bzero(lin_buf,sizeof(lin_buf));
						sprintf(lin_buf,"%s",rs->getString(1).c_str());
						result = strtok( lin_buf, "," );

						set_04_date_lin[i][13] = 0;
						set_04_date_lin[i][14] = 0;
						while( result != NULL ) 
						{
							lin_res_num= atoi(result);
			
							if(lin_res_num>=1  &&  lin_res_num<=8)
							{
								if(lin_res_num % 8 == 0)
								{
									set_04_date_lin[i][13] += (0x80 >> (7));
								}
								else
								{
									set_04_date_lin[i][13] += (0x80 >> (lin_res_num % 8 - 1));
								}
							}
							else if(lin_res_num>=9  &&  lin_res_num<=16)
							{
								if(lin_res_num % 8 == 0)
								{
									set_04_date_lin[i][14] += (0x80 >> (7));
								}
								else
								{
									set_04_date_lin[i][14] += (0x80 >> (lin_res_num % 8 - 1));
								}
							}
							
							result = strtok( NULL, "," );
						}

						/*�׶�����������*/
						bzero(lin_buf,sizeof(lin_buf));
						sprintf(lin_buf,"%s",rs->getString(2).c_str());
						result = strtok( lin_buf, "," );

						for(j = 0;j <13;j++)
						{
							set_04_date_lin[i][15 + j] = 0;
						}
						
						while( result != NULL ) 
						{
						    	
							lin_res_num= atoi(result);
							if(lin_res_num>=1  &&  lin_res_num<=8)
							{
								if(lin_res_num % 8 == 0)
								{
									set_04_date_lin[i][15] += (0x80 >> (7));
								}
								else
								{
									set_04_date_lin[i][15] += (0x80 >>(lin_res_num % 8 - 1));
								}
							}
							else if(lin_res_num>=9  &&  lin_res_num<=16)
							{
								if(lin_res_num % 8 == 0)
								{
									set_04_date_lin[i][16] += (0x80 >> (7));
								}
								else
								{
									set_04_date_lin[i][16] += (0x80 >> (lin_res_num % 8 - 1));
								}
							}
							else if(lin_res_num>=17  &&  lin_res_num<=24)
							{
								if(lin_res_num % 8 == 0)
								{
									set_04_date_lin[i][17] += (0x80 >> (7));
								}
								else
								{
									set_04_date_lin[i][17] += (0x80 >>(lin_res_num % 8 - 1));
								}
							}
							else if(lin_res_num>=25 &&  lin_res_num<=32)
							{
								if(lin_res_num % 8 == 0)
								{
									set_04_date_lin[i][18] += (0x80 >> (7));
								}
								else
								{
									set_04_date_lin[i][18] += (0x80 >> (lin_res_num % 8 - 1));
								}
							}
							else if(lin_res_num>=33  &&  lin_res_num<=40)
							{
								if(lin_res_num % 8 == 0)
								{
									set_04_date_lin[i][19] += (0x80 >> (7));
								}
								else
								{
									set_04_date_lin[i][19] += (0x80 >> (lin_res_num % 8 - 1));
								}
							}
							else if(lin_res_num>=41  &&  lin_res_num<=48)
							{
								if(lin_res_num % 8 == 0)
								{
									set_04_date_lin[i][20] += (0x80 >> (7));
								}
								else
								{
									set_04_date_lin[i][20] += (0x80 >> (lin_res_num % 8 - 1));
								}
							}
							else if(lin_res_num>=49  &&  lin_res_num<=56)
							{
								if(lin_res_num % 8 == 0)
								{
									set_04_date_lin[i][21] += (0x80 >> (7));
								}
								else
								{
									set_04_date_lin[i][21] += (0x80 >> (lin_res_num % 8 - 1));
								}
							}
							else if(lin_res_num>=57  &&  lin_res_num<=64)
							{
								if(lin_res_num % 8 == 0)
								{
									set_04_date_lin[i][22] += (0x80 >> (7));
								}
								else
								{
									set_04_date_lin[i][22] += (0x80 >> (lin_res_num % 8 - 1));
								}
							}
							else if(lin_res_num>=65  &&  lin_res_num<=72)
							{
								if(lin_res_num % 8 == 0)
								{
									set_04_date_lin[i][23] += (0x80 >> (7));
								}
								else
								{
									set_04_date_lin[i][23] += (0x80 >> (lin_res_num % 8 - 1));
								}
							}
							else if(lin_res_num>=73  &&  lin_res_num<=80)
							{
								if(lin_res_num % 8 == 0)
								{
									set_04_date_lin[i][24] += (0x80 >> (7));
								}
								else
								{
									set_04_date_lin[i][24] += (0x80 >> (lin_res_num % 8 - 1));
								}
							}
							else if(lin_res_num>=81  &&  lin_res_num<=88)
							{
								if(lin_res_num % 8 == 0)
								{
									set_04_date_lin[i][25] += (0x80 >> (7));
								}
								else
								{
									set_04_date_lin[i][25] += (0x80 >> (lin_res_num % 8 - 1));
								}
							}
							else if(lin_res_num>=89  &&  lin_res_num<=96)
							{
								if(lin_res_num % 8 == 0)
								{
									set_04_date_lin[i][26] += (0x80 >> (7));
								}
								else
								{
									set_04_date_lin[i][26] += (0x80 >> (lin_res_num % 8 - 1));
								}
							}
							else if(lin_res_num>=97  &&  lin_res_num<=104)
							{
								if(lin_res_num % 8 == 0)
								{
									set_04_date_lin[i][27] += (0x80 >> (7));
								}
								else
								{
									set_04_date_lin[i][27] += (0x80 >> (lin_res_num % 8 - 1));
								}
							}
							
						   	result = strtok( NULL, "," );
						}

						
						/*�׶��ӳ��������*/
						bzero(lin_buf,sizeof(lin_buf));
						sprintf(lin_buf,"%s",rs->getString(3).c_str());
						result = strtok( lin_buf, "," );

						for(j = 0;j <13;j++)
						{
							set_04_date_lin[i][28 + j] = 0;
						}
						
						while( result != NULL ) 
						{
						    	
							lin_res_num= atoi(result);
							if(lin_res_num>=1  &&  lin_res_num<=8)
							{
								if(lin_res_num % 8 == 0)
								{
									set_04_date_lin[i][28] += (0x80 >> (7));
								}
								else
								{
									set_04_date_lin[i][28] += (0x80 >> (lin_res_num % 8 - 1));
								}
							}
							else if(lin_res_num>=9  &&  lin_res_num<=16)
							{
								if(lin_res_num % 8 == 0)
								{
									set_04_date_lin[i][29] += (0x80 >> (7));
								}
								else
								{
									set_04_date_lin[i][29] += (0x80 >> (lin_res_num % 8 - 1));
								}
							}
							else if(lin_res_num>=17  &&  lin_res_num<=24)
							{
								if(lin_res_num % 8 == 0)
								{
									set_04_date_lin[i][30] += (0x80 >> (7));
								}
								else
								{
									set_04_date_lin[i][30] += (0x80 >> (lin_res_num % 8 - 1));
								}
							}
							else if(lin_res_num>=25 &&  lin_res_num<=32)
							{
								if(lin_res_num % 8 == 0)
								{
									set_04_date_lin[i][31] += (0x80 >> (7));
								}
								else
								{
									set_04_date_lin[i][31] += (0x80 >> (lin_res_num % 8 - 1));
								}
							}
							else if(lin_res_num>=33  &&  lin_res_num<=40)
							{
								if(lin_res_num % 8 == 0)
								{
									set_04_date_lin[i][32] += (0x80 >> (7));
								}
								else
								{
									set_04_date_lin[i][32] += (0x80 >> (lin_res_num % 8 - 1));
								}
							}
							else if(lin_res_num>=41  &&  lin_res_num<=48)
							{
								if(lin_res_num % 8 == 0)
								{
									set_04_date_lin[i][33] += (0x80 >> (7));
								}
								else
								{
									set_04_date_lin[i][33] += (0x80 >> (lin_res_num % 8 - 1));
								}
							}
							else if(lin_res_num>=49  &&  lin_res_num<=56)
							{
								if(lin_res_num % 8 == 0)
								{
									set_04_date_lin[i][34] += (0x80 >> (7));
								}
								else
								{
									set_04_date_lin[i][34] += (0x80 >>(lin_res_num % 8 - 1));
								}
							}
							else if(lin_res_num>=57  &&  lin_res_num<=64)
							{
								if(lin_res_num % 8 == 0)
								{
									set_04_date_lin[i][35] += (0x80 >> (7));
								}
								else
								{
									set_04_date_lin[i][35] += (0x80 >> (lin_res_num % 8 - 1));
								}
							}
							else if(lin_res_num>=65  &&  lin_res_num<=72)
							{
								if(lin_res_num % 8 == 0)
								{
									set_04_date_lin[i][36] += (0x80 >> (7));
								}
								else
								{
									set_04_date_lin[i][36] += (0x80 >> (lin_res_num % 8 - 1));
								}
							}
							else if(lin_res_num>=73  &&  lin_res_num<=80)
							{
								if(lin_res_num % 8 == 0)
								{
									set_04_date_lin[i][37] += (0x80 >> (7));
								}
								else
								{
									set_04_date_lin[i][37] += (0x80 >> (lin_res_num % 8 - 1));
								}
							}
							else if(lin_res_num>=81  &&  lin_res_num<=88)
							{
								if(lin_res_num % 8 == 0)
								{
									set_04_date_lin[i][38] += (0x80 >>(7));
								}
								else
								{
									set_04_date_lin[i][38] += (0x80 >> (lin_res_num % 8 - 1));
								}
							}
							else if(lin_res_num>=89  &&  lin_res_num<=96)
							{
								if(lin_res_num % 8 == 0)
								{
									set_04_date_lin[i][39] += (0x80 >> (7));
								}
								else
								{
									set_04_date_lin[i][39] += (0x80 >> (lin_res_num % 8 - 1));
								}
							}
							else if(lin_res_num>=97  &&  lin_res_num<=104)
							{
								if(lin_res_num % 8 == 0)
								{
									set_04_date_lin[i][40] += (0x80 >> (7));
								}
								else
								{
									set_04_date_lin[i][40] += (0x80 >> (lin_res_num % 8 - 1));
								}
							}
							
						   	result = strtok( NULL, "," );
						}

						set_04_date_lin[i][41] = rs->getInt(4);
						set_04_date_lin[i][42] = rs->getInt(5);
						set_04_date_lin[i][43] = rs->getInt(6) * 10;
						
					}

				
					set_04_date_lin[i][172] = buf_check_num(set_04_date_lin[i]);

					sendto(sockfd_recv, set_04_date_lin[i], sizeof(set_04_date_lin[i]), 0, (struct sockaddr*)&signal_addr, sizeof(signal_addr));
/*
					res = recvfrom(sockfd_recv, recv_buf, 512, 0, NULL, NULL);
					printf("recvfrom  date !!!!!!!!!!\n ");
					if(res <= 0)
					{
						close(sockfd_recv);
						return -1;
					}
					else
					{	
						
						
					}
		*/
					struct timeval 	timeVal;
					fd_set  fdSet;
					int ret;

					timeVal.tv_sec = signal_report_timeout;
					timeVal.tv_usec = 0;

					FD_ZERO(&fdSet);
					FD_SET(sockfd_recv, &fdSet);

					ret = select((int) sockfd_recv + 1, &fdSet,NULL, NULL, &timeVal);
					if(ret == 0)                     //��ʱ
					{
						close(sockfd_recv);
						return -1;
					}
					if(ret)
					{
						res = recvfrom(sockfd_recv, recv_buf, 512, 0, NULL, NULL);
					//	int iii=0;
						//for(;iii<res;iii++)
							//printf("\n %2X\n",recv_buf[iii]);
						if(res <= 0)
						{
							close(sockfd_recv);
							return -1;
						}
						printf("recvfrom signal_id = %d,jieduan %d report!!!!!!!!!!\n ",(recv_buf[7]*256+recv_buf[8]),i);
					}			
				}

				close(sockfd_recv);
				return 0;	
			}

			//itc100,�׶�����
			else if(buf[10] == 0x27)
				{
				unsigned char set_27_date_lin[64][31];

				for(i = 0; i < 64 ;i++)
				{
					bzero(set_27_date_lin[i],31);
					set_27_date_lin[i][0]=0x7e;set_27_date_lin[i][2]=0x1d;set_27_date_lin[i][4]=0x01;
					set_27_date_lin[i][7] = buf[7];
					set_27_date_lin[i][8] = buf[8];
					set_27_date_lin[i][9]=0x20;
					set_27_date_lin[i][10] = 0x27;
					set_27_date_lin[i][11] = i;
					set_27_date_lin[i][12] = i + 1;
					
					sprintf(sql,"select STAGE_PHASE, STAGE_DETECTOR,SHIELD_OCC,RELIEVE_OCC, STAT_TIME,AUTO_OCC_CONTROL,STAGE_STEP,AUTO_CONTROL_TIME  from  ITC_CFG_STAGE  where signal_id = %d and Stage_id= %d" ,buf[7] * 256 + buf[8] ,i+1);
					//printf("%s\n",sql);
					st->setSQL(sql);
					rs = st->executeQuery();
					if(rs->next() == 0)
					{
						for(j = 13;j<30;j++)
						{
							set_27_date_lin[i][j] = 0;
						}
						
					}
					else
					{
						/*��λ����������*/
						bzero(lin_buf,sizeof(lin_buf));
						sprintf(lin_buf,"%s",rs->getString(1).c_str());
						result = strtok( lin_buf, "," );

						while( result != NULL ) 
						{
							lin_res_num= atoi(result);
			
							if(lin_res_num>=1  &&  lin_res_num<=8)
							{
								if(lin_res_num % 8 == 0)
								{
									set_27_date_lin[i][13] += (0x80 >> (7));
								}
								else
								{
									set_27_date_lin[i][13] += (0x80 >> (lin_res_num % 8 - 1));
								}
							}
							else if(lin_res_num>=9  &&  lin_res_num<=16)
							{
								if(lin_res_num % 8 == 0)
								{
									set_27_date_lin[i][14] += (0x80 >> (7));
								}
								else
								{
									set_27_date_lin[i][14] += (0x80 >> (lin_res_num % 8 - 1));
								}
							}
							else if(lin_res_num>=17  &&  lin_res_num<=24)
							{
								if(lin_res_num % 8 == 0)
								{
									set_27_date_lin[i][15] += (0x80 >> (7));
								}
								else
								{
									set_27_date_lin[i][15] += (0x80 >> (lin_res_num % 8 - 1));
								}
							}
							else if(lin_res_num>=25  &&  lin_res_num<=32)
							{
								if(lin_res_num % 8 == 0)
								{
									set_27_date_lin[i][16] += (0x80 >> (7));
								}
								else
								{
									set_27_date_lin[i][16] += (0x80 >> (lin_res_num % 8 - 1));
								}
							}
							
							result = strtok( NULL, "," );
						}

						

						/*�׶����μ������*/
						bzero(lin_buf,sizeof(lin_buf));
						sprintf(lin_buf,"%s",rs->getString(2).c_str());
						result = strtok( lin_buf, "," );

						for(j = 0;j <4;j++)
						{
							set_27_date_lin[i][17 + j] = 0;
						}
						
						while( result != NULL ) 
						{
						    	
							lin_res_num= atoi(result);
							if(lin_res_num>=1  &&  lin_res_num<=8)
							{
								if(lin_res_num % 8 == 0)
								{
									set_27_date_lin[i][17] += (0x80 >> (7));
								}
								else
								{
									set_27_date_lin[i][17] += (0x80 >>(lin_res_num % 8 - 1));
								}
							}
							else if(lin_res_num>=9  &&  lin_res_num<=16)
							{
								if(lin_res_num % 8 == 0)
								{
									set_27_date_lin[i][18] += (0x80 >> (7));
								}
								else
								{
									set_27_date_lin[i][18] += (0x80 >> (lin_res_num % 8 - 1));
								}
							}
							else if(lin_res_num>=17  &&  lin_res_num<=24)
							{
								if(lin_res_num % 8 == 0)
								{
									set_27_date_lin[i][19] += (0x80 >> (7));
								}
								else
								{
									set_27_date_lin[i][19] += (0x80 >>(lin_res_num % 8 - 1));
								}
							}
							else if(lin_res_num>=25 &&  lin_res_num<=32)
							{
								if(lin_res_num % 8 == 0)
								{
									set_27_date_lin[i][20] += (0x80 >> (7));
								}
								else
								{
									set_27_date_lin[i][20] += (0x80 >> (lin_res_num % 8 - 1));
								}
							}
							
						   	result = strtok( NULL, "," );
						}

						set_27_date_lin[i][21] = rs->getInt(3);
						set_27_date_lin[i][22] = rs->getInt(4);
						set_27_date_lin[i][23] = (rs->getInt(5)) /256;
						set_27_date_lin[i][24] = (rs->getInt(5)) %256;
						set_27_date_lin[i][25] = 0;					
						
						set_27_date_lin[i][26] = rs->getInt(6);
						set_27_date_lin[i][27] = rs->getInt(7);
						set_27_date_lin[i][28] = rs->getInt(8) * 10;
						
					}

				
					set_27_date_lin[i][29] = buf_check_num(set_27_date_lin[i]);
					set_27_date_lin[i][30] = 0x7d;
					sendto(sockfd_recv, set_27_date_lin[i], sizeof(set_27_date_lin[i]), 0, (struct sockaddr*)&signal_addr, sizeof(signal_addr));
/*
					res = recvfrom(sockfd_recv, recv_buf, 512, 0, NULL, NULL);
					printf("recvfrom  date !!!!!!!!!!\n ");
					if(res <= 0)
					{
						close(sockfd_recv);
						return -1;
					}
	*/
					struct timeval 	timeVal;
					fd_set  fdSet;
					int ret;

					timeVal.tv_sec = signal_report_timeout;
					timeVal.tv_usec = 0;

					FD_ZERO(&fdSet);
					FD_SET(sockfd_recv, &fdSet);

					ret = select((int) sockfd_recv + 1, &fdSet,NULL, NULL, &timeVal);
					if(ret == 0)                     //��ʱ
					{
						close(sockfd_recv);
						return -1;
					}
					if(ret)
					{
						res = recvfrom(sockfd_recv, recv_buf, 512, 0, NULL, NULL);
						if(res <= 0)
						{
							close(sockfd_recv);
							return -1;
						}
						printf("recvfrom signal_id = %d,jieduan %d report!!!!!!!!!!\n ",(recv_buf[7]*256+recv_buf[8]),i);
					}						
				}

				close(sockfd_recv);
				return 0;	
			}
			/* �������ݽṹ����*/
			else if(buf[10] == 0x05)
			{

				unsigned char set_05_date_lin[128][213];

				for(i = 0; i < 128;i++)
				{
					for(j = 0; j < 213;j++)
					{
						set_05_date_lin[i][j] = set_05_date[i][j];
					}
					
				}
				
				struct plan_config pian_config_date[129];

				sprintf(sql,"select Plan_id,Plan_week,Plan_record,Xianweicha_time,id,(select count(*) from plan_config_chain where plan_id=t.id )  from  PLAN_CONFIG t  where signal_id = %d  order by Plan_id " ,buf[7] * 256 + buf[8] );
				//printf("%s\n",sql);
				st->setSQL(sql);
				rs = st->executeQuery();
				while(rs->next())
				{
					pian_config_date[rs->getInt(1)].plan_week[0] = (rs->getInt(2) * 10) >> 8;
					pian_config_date[rs->getInt(1)].plan_week[1] = (rs->getInt(2) * 10) & 0xff;
					pian_config_date[rs->getInt(1)].plan_record= rs->getInt(3) ;
					pian_config_date[rs->getInt(1)].xianweicha_time[0] =  (rs->getInt(4) * 10) >> 8;
					pian_config_date[rs->getInt(1)].xianweicha_time[1] =  (rs->getInt(4) * 10) & 0xff;
					pian_config_date[rs->getInt(1)].plan_config_id = rs->getInt(5) ;
					pian_config_date[rs->getInt(1)].plan_num= rs->getInt(6) ;
				}


				sprintf(sql,"select Plan_id,Stage_order,Stage_id,Stage_time,Stage_type  from  PLAN_CONFIG_CHAIN  where signal_id = %d  order by Stage_order" ,buf[7] * 256 + buf[8] );
				printf("%s\n",sql);
				st->setSQL(sql);
				rs = st->executeQuery();
				while(rs->next())
				{
					
					for(i = 1;i <= 128;i++)
					{
						if(rs->getInt(1) == pian_config_date[i].plan_config_id)
						{
							printf("rs->getInt(1) == pian_config_date[i].plan_config_id   %d\n",i);
							pian_config_date[i].plan_config_chin_id[rs->getInt(2) - 1] = rs->getInt(3);
							pian_config_date[i].plan_config_chin_time[(rs->getInt(2) -1) *  2] = (rs->getInt(4) * 10) >> 8;
							pian_config_date[i].plan_config_chin_time[(rs->getInt(2) -1) *  2 + 1] = (rs->getInt(4) * 10) & 0xff;
							pian_config_date[i].plan_config_chin_type[rs->getInt(2) - 1] = rs->getInt(5);
							
						}
					}
						
				}

				for(i = 0;i < 128;i++)
				{
					set_05_date_lin[i][7] = buf[7];
					set_05_date_lin[i][8] = buf[8];
					set_05_date_lin[i][12] = i+ 1;

					set_05_date_lin[i][13] =  pian_config_date[i + 1].plan_week[0];
					set_05_date_lin[i][14] =  pian_config_date[i+ 1].plan_week[1];

					set_05_date_lin[i][15] =  pian_config_date[i + 1].plan_record;
					
					set_05_date_lin[i][16] =  pian_config_date[i+ 1].xianweicha_time[0];
					set_05_date_lin[i][17] =  pian_config_date[i + 1].xianweicha_time[1];

					set_05_date_lin[i][18] =  pian_config_date[i + 1].plan_num;

					for(j = 0; j < 16 ;j++)
					{
						set_05_date_lin[i][19 + j] = pian_config_date[i + 1].plan_config_chin_id[j];
						set_05_date_lin[i][35 + j * 2] = pian_config_date[i + 1].plan_config_chin_time[j * 2];
						set_05_date_lin[i][35 + j * 2 + 1] = pian_config_date[i+ 1].plan_config_chin_time[j * 2 + 1];
						set_05_date_lin[i][67 + j] = pian_config_date[i+ 1].plan_config_chin_type[j];
						
					}

					set_05_date_lin[i][211] = buf_check_num(set_05_date_lin[i]);

					sendto(sockfd_recv, set_05_date_lin[i ], sizeof(set_05_date_lin[i ]), 0, (struct sockaddr*)&signal_addr, sizeof(signal_addr));
/*
					res = recvfrom(sockfd_recv, recv_buf, 512, 0, NULL, NULL);
					printf("recvfrom  date !!!!!!!!!!\n ");
					if(res <= 0)
					{
						close(sockfd_recv);
						return -1;
					}
					else
					{	
						
						
					}
*/
					struct timeval 	timeVal;
					fd_set  fdSet;
					int ret;

					timeVal.tv_sec = signal_report_timeout;
					timeVal.tv_usec = 0;

					FD_ZERO(&fdSet);
					FD_SET(sockfd_recv, &fdSet);

					ret = select((int) sockfd_recv + 1, &fdSet,NULL, NULL, &timeVal);
					if(ret == 0)                     //��ʱ
					{
						close(sockfd_recv);
						return -1;
					}
					if(ret)
					{
						res = recvfrom(sockfd_recv, recv_buf, 512, 0, NULL, NULL);
						if(res <= 0)
						{
							close(sockfd_recv);
							return -1;
						}
						printf("recvfrom signal_id = %d,fang'an %d report!!!!!!!!!!\n ",(recv_buf[7]*256+recv_buf[8]),i);
					}		
				}

				
				close(sockfd_recv);
				return 0;	
				
			}

			//itc100,�������ݽṹ
			else if(buf[10] == 0x2b)
				{

				unsigned char set_2b_date_lin[128][166];

				for(i = 0; i < 128;i++)
				{
					bzero(set_2b_date_lin[i],166);

					set_2b_date_lin[i][0]=0x7e;set_2b_date_lin[i][2]=0xa4;set_2b_date_lin[i][4]=0x01;
					set_2b_date_lin[i][7] = buf[7];
					set_2b_date_lin[i][8] = buf[8];
					set_2b_date_lin[i][9]=0x20;
					set_2b_date_lin[i][10] = 0x2b;
					set_2b_date_lin[i][11] = i;
					set_2b_date_lin[i][12] = i + 1;

					sprintf(sql,"select list_unit_id,Plan_week*10,Plan_record,phase_diff_time*10,(select count(*) from itc_cfg_plan_chain where signal_id=t.signal_id and plan_id = t.plan_id)  from  itc_cfg_plan t  where signal_id = %d  and  Plan_id = %d " ,buf[7] * 256 + buf[8],i+1 );
					//printf("%s\n",sql);
					st->setSQL(sql);
					rs = st->executeQuery();

					if(rs->next())
					{
						set_2b_date_lin[i][13] = rs->getInt(1); //·�ڱ��
						
						set_2b_date_lin[i][14] =  (rs->getInt(2) ) >> 8;
						set_2b_date_lin[i][15] =  (rs->getInt(2) ) & 0xff;
						
						set_2b_date_lin[i][16] =rs->getInt(3); // Э�����
						
						set_2b_date_lin[i][17] =(rs->getInt(4) ) >> 8;
						set_2b_date_lin[i][18] =  (rs->getInt(4) ) & 0xff;

						set_2b_date_lin[i][19] =  rs->getInt(5);
					}

					if(set_2b_date_lin[i][19] != 0)
					{
						sprintf(sql,"select stage_order,Stage_id,Stage_time*10,Stage_type,green_time*10,green_flash_time*10,green_flash_qtime*10,red_time*10  from  ITC_CFG_PLAN_CHAIN  where signal_id = %d  and plan_id = %d order by stage_order" ,buf[7] * 256 + buf[8] ,i+1);

						printf("%s\n",sql);
						st->setSQL(sql);
						rs = st->executeQuery();
						while(rs->next())
						{
							set_2b_date_lin[i][20 + ( rs->getInt(1) -1)] = rs->getInt(2); //�׶���
							set_2b_date_lin[i][36+ ( rs->getInt(1) -1)*2] = (rs->getInt(3) ) >> 8; 
							set_2b_date_lin[i][37+ ( rs->getInt(1) -1)*2] = (rs->getInt(3) ) & 0xff; //�׶�ʱ��
							set_2b_date_lin[i][68+ ( rs->getInt(1) -1)] = rs->getInt(4) ; //�׶�����

							set_2b_date_lin[i][84+ ( rs->getInt(1) -1) * 5] = (rs->getInt(5))/256 ;
							set_2b_date_lin[i][85+ ( rs->getInt(1) -1) *5] = (rs->getInt(5) )%256 ; //�����̵�
							set_2b_date_lin[i][86+ ( rs->getInt(1) -1) *5] = rs->getInt(6) ;//����������δ��
							set_2b_date_lin[i][87+ ( rs->getInt(1) -1) *5] = rs->getInt(7)  ;//�����̿���
							set_2b_date_lin[i][88+ ( rs->getInt(1) -1) *5] = rs->getInt(8) ;//���˺��
						}

					}

					set_2b_date_lin[i][164] = buf_check_num(set_2b_date_lin[i]);
					set_2b_date_lin[i][165] = 0x7d;
					sendto(sockfd_recv, set_2b_date_lin[i ], sizeof(set_2b_date_lin[i ]), 0, (struct sockaddr*)&signal_addr, sizeof(signal_addr));

		/*			res = recvfrom(sockfd_recv, recv_buf, 512, 0, NULL, NULL);
					printf("recvfrom  date !!!!!!!!!!\n ");
					if(res <= 0)
					{
						close(sockfd_recv);
						return -1;
					}
*/
					struct timeval 	timeVal;
					fd_set  fdSet;
					int ret;

					timeVal.tv_sec = signal_report_timeout;
					timeVal.tv_usec = 0;

					FD_ZERO(&fdSet);
					FD_SET(sockfd_recv, &fdSet);

					ret = select((int) sockfd_recv + 1, &fdSet,NULL, NULL, &timeVal);
					if(ret == 0)                     //��ʱ
					{
						close(sockfd_recv);
						return -1;
					}
					if(ret)
					{
						res = recvfrom(sockfd_recv, recv_buf, 512, 0, NULL, NULL);
						if(res <= 0)
						{
							close(sockfd_recv);
							return -1;
						}
						printf("recvfrom signal_id = %d,fang'an%d report!!!!!!!!!!\n ",(recv_buf[7]*256+recv_buf[8]),i);
					}		
				}
				
				close(sockfd_recv);
				return 0;	
				
			}
			/* ʱ�����ݽṹ����*/
			else if(buf[10] == 0x06)
		//	else if(buf[10] == 0x00)
			{

				unsigned char set_06_date_lin[128][336];

				for(i = 0; i < 128;i++)
				{
					for(j = 0; j < 336;j++)
					{
						set_06_date_lin[i][j] = set_06_date[i][j];
					}
					
				}
				
				struct  sjbsdsz_info sjbsdsz_info_date[129];

				
				sprintf(sql,"select Time_id,(select count(*) from SJBSDSZ_INFO where Time_id=t.Time_id  and  signal_id = %d and FUNCTION_TYPE = 1) from  TIME_INFO t  where signal_id = %d and FUNCTION_TYPE = 1   order by Time_id " ,buf[7] * 256 + buf[8],buf[7] * 256 + buf[8] );
			//	printf("%s\n",sql);
				st->setSQL(sql);
				rs = st->executeQuery();
				while(rs->next())
				{
					sjbsdsz_info_date[rs->getInt(1)].time_num = rs->getInt(2);
				}


				
				sprintf(sql,"select Time_id,Time_set_number,Hour,Minute,Yunxing_mode,Fangan  from  SJBSDSZ_INFO t  where signal_id = %d  and FUNCTION_TYPE = 1 order by Time_id " ,buf[7] * 256 + buf[8] );
			//	printf("%s\n",sql);
				st->setSQL(sql);
				rs = st->executeQuery();
				while(rs->next())
				{
					
					sjbsdsz_info_date[rs->getInt(1)].time_all_infomation[(rs->getInt(2) - 1) * 4 ] = rs->getInt(3);
					sjbsdsz_info_date[rs->getInt(1)].time_all_infomation[(rs->getInt(2) - 1) * 4 + 1] = rs->getInt(4);
					sjbsdsz_info_date[rs->getInt(1)].time_all_infomation[(rs->getInt(2) - 1) * 4 + 2] = rs->getInt(5);
					sjbsdsz_info_date[rs->getInt(1)].time_all_infomation[(rs->getInt(2) - 1) * 4 + 3] = rs->getInt(6);
				}

				for(i = 0;i < 128;i++)
				{
					set_06_date_lin[i][7] = buf[7];
					set_06_date_lin[i][8] = buf[8];
					set_06_date_lin[i][12] = i+1;

					set_06_date_lin[i][13] = sjbsdsz_info_date[i + 1].time_num;

			//		printf("i = %d ,  num = %d\n",i+1, sjbsdsz_info_date[i + 1].time_num);
					for(j = 0; j < 48; j++)
					{
						set_06_date_lin[i][14 + j * 4 ] = sjbsdsz_info_date[i + 1].time_all_infomation[j * 4];
						set_06_date_lin[i][14 + j * 4 + 1] = sjbsdsz_info_date[i + 1].time_all_infomation[j * 4 + 1];
						set_06_date_lin[i][14 + j * 4 + 2] = sjbsdsz_info_date[i + 1].time_all_infomation[j * 4 + 2];
						set_06_date_lin[i][14 + j * 4 + 3] = sjbsdsz_info_date[i + 1].time_all_infomation[j * 4 + 3];
					}

					set_06_date_lin[i][334] = buf_check_num(set_06_date_lin[i]);

					sendto(sockfd_recv, set_06_date_lin[i], sizeof(set_06_date_lin[i]), 0, (struct sockaddr*)&signal_addr, sizeof(signal_addr));
#if 0
			if(i < 5)
			{
			int www;
			for(www = 0; www <336 ;www++)
				{
				printf("%02x",set_06_date_lin[i][www]);
				}
			printf("\n");
			}
		
#endif			
	//	printf("i == %d\n",i);
/*
					res = recvfrom(sockfd_recv, recv_buf, 512, 0, NULL, NULL);
				//	printf("recvfrom  date !!!!!!!!!!\n ");
					if(res <= 0)
					{
						close(sockfd_recv);
						return -1;
					}
					else
					{	
						
						
					}
*/
					struct timeval 	timeVal;
					fd_set  fdSet;
					int ret;

					timeVal.tv_sec = signal_report_timeout;
					timeVal.tv_usec = 0;

					FD_ZERO(&fdSet);
					FD_SET(sockfd_recv, &fdSet);

					ret = select((int) sockfd_recv + 1, &fdSet,NULL, NULL, &timeVal);
					if(ret == 0)                     //��ʱ
					{
						close(sockfd_recv);
						return -1;
					}
					if(ret)
					{
						res = recvfrom(sockfd_recv, recv_buf, 512, 0, NULL, NULL);
						if(res <= 0)
						{
							close(sockfd_recv);
							return -1;
						}
						printf("recvfrom signal_id = %d,shijianbiao %d report!!!!!!!!!!\n ",(recv_buf[7]*256+recv_buf[8]),i);
					}		
				}
				close(sockfd_recv);
				return 0;	
				
			}

			//itc100,ʱ�������
			else if(buf[10] == 0x2c)
				{

				unsigned char set_2c_date_lin[128][425];

				for(i = 0; i < 128;i++)
				{
					bzero(set_2c_date_lin[i],425);
					set_2c_date_lin[i][0] = 0x7e; set_2c_date_lin[i][1] = 0x01;set_2c_date_lin[i][2] = 0xa7;
					set_2c_date_lin[i][4] = 0x01; set_2c_date_lin[i][7] = buf[7];set_2c_date_lin[i][8] = buf[8];
					set_2c_date_lin[i][9] = 0x20;set_2c_date_lin[i][10] = 0x2c;

					set_2c_date_lin[i][11] = i;set_2c_date_lin[i][12] = i+1;

					
					sprintf(sql,"select list_unit_id,(select count(*) from ITC_CFG_TIME_PERIOD where Time_id=t.Time_id  and  signal_id = %d ) from  ITC_CFG_TIME t  where signal_id = %d and Time_id = %d" ,buf[7] * 256 + buf[8],buf[7] * 256 + buf[8] ,i+1);
					st->setSQL(sql);
					rs = st->executeQuery();

					if(rs->next())
					{
						set_2c_date_lin[i][13]= rs->getInt(1);
						set_2c_date_lin[i][14]= rs->getInt(2); // ʱ�α�����
					}

					if(set_2c_date_lin[i][14] >0)
					{
						sprintf(sql,"select  time_order,Hour,Minute,plan_id,run_mode,occ_control_flag,shield_stages,variable_sign,special_output_phase,red_status,yellow_status,green_status  from  ITC_CFG_TIME_PERIOD t  where signal_id = %d and Time_id =%d order by time_order " ,buf[7] * 256 + buf[8] ,i+1);
						//	printf("%s\n",sql);
						st->setSQL(sql);
						rs = st->executeQuery();

						while(rs->next())
						{
							set_2c_date_lin[i][15 + (rs->getInt(1)-1)*17]= rs->getInt(2);//
							set_2c_date_lin[i][16 + (rs->getInt(1)-1) *17]= rs->getInt(3);//
							set_2c_date_lin[i][17 +(rs->getInt(1)-1) *17]= rs->getInt(4);//
							set_2c_date_lin[i][18 +(rs->getInt(1)-1) *17]= rs->getInt(5);//

							set_2c_date_lin[i][19 +(rs->getInt(1)-1) *17]= rs->getInt(6);//

							//
							int count_time = 0;
							bzero(lin_buf,sizeof(lin_buf));
							sprintf(lin_buf,"%s",rs->getString(7).c_str());
							result = strtok( lin_buf, "," );
							
							while( result != NULL ) 
							{	
								lin_res_num= atoi(result);

								set_2c_date_lin[i][20+count_time + (rs->getInt(1)-1) *17]=lin_res_num;
								count_time +=1;

								result = strtok( NULL, "," );
							}

							//
							bzero(lin_buf,sizeof(lin_buf));
							sprintf(lin_buf,"%s",rs->getString(8).c_str());
							result = strtok( lin_buf, "," );

							while( result != NULL ) 
							{	
								lin_res_num= atoi(result);

								set_2c_date_lin[i][20+count_time +(rs->getInt(1)-1) *17]=lin_res_num;
								count_time +=1;

								result = strtok( NULL, "," );
							}

							
							set_2c_date_lin[i][28 + (rs->getInt(1)-1) *17]= rs->getInt(9);//
							set_2c_date_lin[i][29 + (rs->getInt(1)-1) *17]= rs->getInt(10);//
							set_2c_date_lin[i][30 + (rs->getInt(1)-1) *17]= rs->getInt(11);//
							set_2c_date_lin[i][31 + (rs->getInt(1)-1) *17]= rs->getInt(12);//
						}						

					}
					set_2c_date_lin[i][423] = buf_check_num(set_2c_date_lin[i]);
					set_2c_date_lin[i][424] = 0x7d;

					sendto(sockfd_recv, set_2c_date_lin[i], sizeof(set_2c_date_lin[i]), 0, (struct sockaddr*)&signal_addr, sizeof(signal_addr));
#if 0
			if(i < 5)
			{
			int www;
			for(www = 0; www <336 ;www++)
				{
				printf("%02x",set_06_date_lin[i][www]);
				}
			printf("\n");
			}
		
#endif			
	//	printf("i == %d\n",i);

/*
					res = recvfrom(sockfd_recv, recv_buf, 512, 0, NULL, NULL);
				//	printf("recvfrom  date !!!!!!!!!!\n ");
					if(res <= 0)
					{
						close(sockfd_recv);
						return -1;
					}
*/
					struct timeval 	timeVal;
					fd_set  fdSet;
					int ret;

					timeVal.tv_sec = signal_report_timeout;
					timeVal.tv_usec = 0;

					FD_ZERO(&fdSet);
					FD_SET(sockfd_recv, &fdSet);

					ret = select((int) sockfd_recv + 1, &fdSet,NULL, NULL, &timeVal);
					if(ret == 0)                     //��ʱ
					{
						close(sockfd_recv);
						return -1;
					}
					if(ret)
					{
						res = recvfrom(sockfd_recv, recv_buf, 512, 0, NULL, NULL);
						if(res <= 0)
						{
							close(sockfd_recv);
							return -1;
						}
						printf("recvfrom signal_id = %d,shijianbiao %d report!!!!!!!!!!\n ",(recv_buf[7]*256+recv_buf[8]),i);
					}		
				}

				close(sockfd_recv);
				return 0;	
				
			}
			
			/* �������ݽṹ����*/
			else if(buf[10] == 0x07)
			{

				unsigned char set_07_date_lin[128][160];

				for(i = 0; i < 128;i++)
				{
					for(j = 0; j < 160;j++)
					{
						set_07_date_lin[i][j] = set_07_date[i][j];
					}
					
				}

				struct diaodu_info diaodu_info_date[129];
		

				//sprintf(sql,"select Ddb_id,Ddb_type,Ddb_priority,Week_ddb_type,Date_ddb_type_y ,Date_ddb_type_r,New_ddb_type_n,New_ddb_type_y,New_ddb_type_r, Time_table_number_1  from  DIAODU_INFO   where signal_id = %d  order by Ddb_id " ,buf[7] * 256 + buf[8] );
				sprintf(sql,"select Ddb_id,Ddb_type,Ddb_priority,Week_ddb_type,Date_ddb_type_y ,Date_ddb_type_r, Time_table_number_1  from  DIAODU_INFO   where signal_id = %d and  FUNCTION_TYPE = 1  order by Ddb_id " ,buf[7] * 256 + buf[8] );
			
		//		printf("%s\n",sql);
				st->setSQL(sql);
				rs = st->executeQuery();
				while(rs->next())
				{
					diaodu_info_date[rs->getInt(1)].Ddb_type = rs->getInt(2);
					diaodu_info_date[rs->getInt(1)].Ddb_priority= rs->getInt(3);

					diaodu_info_date[rs->getInt(1)].Week_ddb_type = 0;
					bzero(lin_buf,sizeof(lin_buf));
					sprintf(lin_buf,"%s",rs->getString(4).c_str());
					result = strtok( lin_buf, "," );
					
					while( result != NULL ) 
					{	
						lin_res_num= atoi(result);

						if(lin_res_num == 7)
						{
							diaodu_info_date[rs->getInt(1)].Week_ddb_type += 0x01;
						}
						else
						{
							diaodu_info_date[rs->getInt(1)].Week_ddb_type += (0x01 << lin_res_num);
						}
						
						result = strtok( NULL, "," );
					}

					diaodu_info_date[rs->getInt(1)].Date_ddb_type_y[0] = 0;
					diaodu_info_date[rs->getInt(1)].Date_ddb_type_y[1] = 0;	
					bzero(lin_buf,sizeof(lin_buf));
					sprintf(lin_buf,"%s",rs->getString(5).c_str());
					result = strtok( lin_buf, "," );
					
					while( result != NULL ) 
					{	
						lin_res_num= atoi(result);

						if(lin_res_num>=1  &&  lin_res_num<=7)
						{
							diaodu_info_date[rs->getInt(1)].Date_ddb_type_y[0]  += (0x01 << (lin_res_num % 8 ));

					
						}
						else if(lin_res_num>=8  &&  lin_res_num<=15)
						{
							diaodu_info_date[rs->getInt(1)].Date_ddb_type_y[1]  += (0x01 << (lin_res_num % 8 ));
						
						}
						
						
						result = strtok( NULL, "," );
					}


					diaodu_info_date[rs->getInt(1)].Date_ddb_type_r[0] = 0;
					diaodu_info_date[rs->getInt(1)].Date_ddb_type_r[1] = 0;
					diaodu_info_date[rs->getInt(1)].Date_ddb_type_r[2] = 0;
					diaodu_info_date[rs->getInt(1)].Date_ddb_type_r[3] = 0;	
					bzero(lin_buf,sizeof(lin_buf));
					sprintf(lin_buf,"%s",rs->getString(6).c_str());
					result = strtok( lin_buf, "," );
					
					while( result != NULL ) 
					{	
						lin_res_num= atoi(result);

						if(lin_res_num>=1  &&  lin_res_num<=7)
						{
							diaodu_info_date[rs->getInt(1)].Date_ddb_type_r[0]  += (0x01 << (lin_res_num % 8 ));	
						}
						else if(lin_res_num>=8  &&  lin_res_num<=15)
						{
							diaodu_info_date[rs->getInt(1)].Date_ddb_type_r[1]  += (0x01 << (lin_res_num % 8 ));
							
						}
						else if(lin_res_num>=16  &&  lin_res_num<=23)
						{
							diaodu_info_date[rs->getInt(1)].Date_ddb_type_r[2]  += (0x01 << (lin_res_num % 8 ));
						}
						else if(lin_res_num>=24  &&  lin_res_num<=31)
						{
							diaodu_info_date[rs->getInt(1)].Date_ddb_type_r[3]  += (0x01 << (lin_res_num % 8 ));
						}
		#if 0				
						if(lin_res_num>=1  &&  lin_res_num<=8)
						{
							if(lin_res_num % 8 == 0)
							{
								diaodu_info_date[rs->getInt(1)].Date_ddb_type_r[0] += (0x01 << (7));
							}
							else
							{
								diaodu_info_date[rs->getInt(1)].Date_ddb_type_r[0]  += (0x01 << (lin_res_num % 8 - 1));
							}
						}
						else if(lin_res_num>=9  &&  lin_res_num<=16)
						{
							if(lin_res_num % 8 == 0)
							{
								diaodu_info_date[rs->getInt(1)].Date_ddb_type_r[1] += (0x01 << (7));
							}
							else
							{
								diaodu_info_date[rs->getInt(1)].Date_ddb_type_r[1]  += (0x01 << (lin_res_num % 8 - 1));
							}
						}
						else if(lin_res_num>=17  &&  lin_res_num<=24)
						{
							if(lin_res_num % 8 == 0)
							{
								diaodu_info_date[rs->getInt(1)].Date_ddb_type_r[2] += (0x01 << (7));
							}
							else
							{
								diaodu_info_date[rs->getInt(1)].Date_ddb_type_r[2]  += (0x01 << (lin_res_num % 8 - 1));
							}
						}
						else if(lin_res_num>=25  &&  lin_res_num<=32)
						{
							if(lin_res_num % 8 == 0)
							{
								diaodu_info_date[rs->getInt(1)].Date_ddb_type_r[3] += (0x01 << (7));
							}
							else
							{
								diaodu_info_date[rs->getInt(1)].Date_ddb_type_r[3]  += (0x01 << (lin_res_num % 8 - 1));
							}
						}
				#endif		
						result = strtok( NULL, "," );
					}

					diaodu_info_date[rs->getInt(1)].Time_table_number_1 = rs->getInt(7);
					
					
				}
				
				for(i = 0;i < 128;i++)
				{
					set_07_date_lin[i][7] = buf[7];
					set_07_date_lin[i][8] = buf[8];
					set_07_date_lin[i][12] = i + 1;

					set_07_date_lin[i][13] = diaodu_info_date[i+ 1].Ddb_type;
					set_07_date_lin[i][14] = diaodu_info_date[i+ 1].Ddb_priority;
					set_07_date_lin[i][15] = diaodu_info_date[i+ 1].Week_ddb_type;

					set_07_date_lin[i][16]  = diaodu_info_date[i+ 1].Date_ddb_type_y[1];
					set_07_date_lin[i][17]  = diaodu_info_date[i+ 1].Date_ddb_type_y[0];

					set_07_date_lin[i][18]  = diaodu_info_date[i+ 1].Date_ddb_type_r[3];
					set_07_date_lin[i][19]  = diaodu_info_date[i+ 1].Date_ddb_type_r[2];
					set_07_date_lin[i][20]  = diaodu_info_date[i+ 1].Date_ddb_type_r[1];
					set_07_date_lin[i][21]  = diaodu_info_date[i+ 1].Date_ddb_type_r[0];

					set_07_date_lin[i][29] = diaodu_info_date[i+ 1].Time_table_number_1;

					set_07_date_lin[i][158] = buf_check_num(set_07_date_lin[i]);

					sendto(sockfd_recv, set_07_date_lin[i], sizeof(set_07_date_lin[i]), 0, (struct sockaddr*)&signal_addr, sizeof(signal_addr));
/*
					res = recvfrom(sockfd_recv, recv_buf, 512, 0, NULL, NULL);
					printf("recvfrom  date !!!!!!!!!!\n ");
					if(res <= 0)
					{
						close(sockfd_recv);
						return -1;
					}
					else
					{		
					}
						
		*/
					struct timeval 	timeVal;
					fd_set  fdSet;
					int ret;

					timeVal.tv_sec = signal_report_timeout;
					timeVal.tv_usec = 0;

					FD_ZERO(&fdSet);
					FD_SET(sockfd_recv, &fdSet);

					ret = select((int) sockfd_recv + 1, &fdSet,NULL, NULL, &timeVal);
					if(ret == 0)                     //��ʱ
					{
						close(sockfd_recv);
						return -1;
					}
					if(ret)
					{
						res = recvfrom(sockfd_recv, recv_buf, 512, 0, NULL, NULL);
						if(res <= 0)
						{
							close(sockfd_recv);
							return -1;
						}
						printf("recvfrom signal_id = %d,diao'du'biao %d report!!!!!!!!!!\n ",(recv_buf[7]*256+recv_buf[8]),i);
					}					
				}

				close(sockfd_recv);
				return 0;	
			}

			//itc100 ,���ȱ�
			else if (buf[10] == 0x2d)
				{

				unsigned char set_2d_date_lin[128][34];

				for(i = 0; i < 128;i++)
				{
					bzero(set_2d_date_lin[i],34);
					set_2d_date_lin[i][0] = 0x7e;set_2d_date_lin[i][2] = 0x20;
					set_2d_date_lin[i][4] = 0x01;set_2d_date_lin[i][7] = buf[7];set_2d_date_lin[i][8]  = buf[8];
					set_2d_date_lin[i][9] = 0x20;set_2d_date_lin[i][10] = 0x2d;
					set_2d_date_lin[i][11] = i; set_2d_date_lin[i][12] = i+1;  

					sprintf(sql,"select list_unit_id,manage_type,manage_priority,week_mng,date_mng_month ,date_mng_day, time_id  from  ITC_CFG_MANAGE   where signal_id = %d and manage_id = %d  " ,buf[7] * 256 + buf[8] ,i+1);
			
					//		printf("%s\n",sql);
					st->setSQL(sql);
					rs = st->executeQuery();

					if(rs->next() != 0)
					{
						set_2d_date_lin[i][13] = rs->getInt(1);
						set_2d_date_lin[i][14] = rs->getInt(2);
						set_2d_date_lin[i][15] = rs->getInt(3);

						//����ֵ
						bzero(lin_buf,sizeof(lin_buf));
						sprintf(lin_buf,"%s",rs->getString(4).c_str());
						result = strtok( lin_buf, "," );
						while( result != NULL ) 
						{
							lin_res_num= atoi(result);
							set_2d_date_lin[i][16]  =set_2d_date_lin[i][16]  | (0x01 << (lin_res_num % 7));
							result = strtok( NULL, "," );
						}

						//�·�
						bzero(lin_buf,sizeof(lin_buf));
						sprintf(lin_buf,"%s",rs->getString(5).c_str());
						result = strtok( lin_buf, "," );

						unsigned int mouth_temp = 0 ;
						while( result != NULL ) 
						{
							lin_res_num= atoi(result);
							mouth_temp=mouth_temp | (0x0001 << ( lin_res_num - 1));
							result = strtok( NULL, "," );
						}
						set_2d_date_lin[i][17] = mouth_temp >> 8;
						set_2d_date_lin[i][18] = mouth_temp & 0xff;

						//����
						bzero(lin_buf,sizeof(lin_buf));
						sprintf(lin_buf,"%s",rs->getString(6).c_str());
						result = strtok( lin_buf, "," );

						unsigned int day_temp = 0 ;
						while( result != NULL ) 
						{
							lin_res_num= atoi(result);
							day_temp=day_temp | (0x00000001 << ( lin_res_num - 1));
							result = strtok( NULL, "," );
						}
						set_2d_date_lin[i][19] = mouth_temp >> 24;
						set_2d_date_lin[i][20] = mouth_temp >> 16;
						set_2d_date_lin[i][21] = mouth_temp >> 8;
						set_2d_date_lin[i][22] = mouth_temp & 0xff;

						set_2d_date_lin[i][30] = rs->getInt(7);

						
					}
					set_2d_date_lin[i][32] = buf_check_num(set_2d_date_lin[i]);
					set_2d_date_lin[i][33] = 0x7d;

					sendto(sockfd_recv, set_2d_date_lin[i], sizeof(set_2d_date_lin[i]), 0, (struct sockaddr*)&signal_addr, sizeof(signal_addr));
/*
					res = recvfrom(sockfd_recv, recv_buf, 512, 0, NULL, NULL);
					printf("recvfrom  date !!!!!!!!!!\n ");
					if(res <= 0)
					{
						close(sockfd_recv);
						return -1;
					}
*/
					struct timeval 	timeVal;
					fd_set  fdSet;
					int ret;

					timeVal.tv_sec = signal_report_timeout;
					timeVal.tv_usec = 0;

					FD_ZERO(&fdSet);
					FD_SET(sockfd_recv, &fdSet);

					ret = select((int) sockfd_recv + 1, &fdSet,NULL, NULL, &timeVal);
					if(ret == 0)                     //��ʱ
					{
						close(sockfd_recv);
						return -1;
					}
					if(ret)
					{
						res = recvfrom(sockfd_recv, recv_buf, 512, 0, NULL, NULL);
						if(res <= 0)
						{
							close(sockfd_recv);
							return -1;
						}
						printf("recvfrom signal_id = %d,diao'du'biao %d report!!!!!!!!!!\n ",(recv_buf[7]*256+recv_buf[8]),i);
					}		
				}
				close(sockfd_recv);
				return 0;	
			}
			
			/* ��λ��ͻ����*/
			else if(buf[10] == 0x08)
			{
				unsigned char set_08_date_lin[270];

				for(i = 0; i < 270;i++)
				{
					
					set_08_date_lin[i] = set_08_date[i];
					
				}

				set_08_date_lin[7] = buf[7];
				set_08_date_lin[8] = buf[8];
				sprintf(sql,"select Clink_id, Rush_clink  from  CHONGTU_INFO    where signal_id = %d  order by Clink_id " ,buf[7] * 256 + buf[8] );
			
				//printf("%s\n",sql);
				st->setSQL(sql);
				rs = st->executeQuery();

			
				while(rs->next())
				{
					bzero(lin_buf,sizeof(lin_buf));
					sprintf(lin_buf,"%s",rs->getString(2).c_str());
					result = strtok( lin_buf, "," );
					
					while( result != NULL ) 
					{	
						lin_res_num= atoi(result);

						set_08_date_lin[12 + (rs->getInt(1) - 1) * 16 + (lin_res_num - 1)]  = 1;
						

						result = strtok( NULL, "," );
						
					}
					
				}
				set_08_date_lin[268] = buf_check_num(set_08_date_lin);

				sendto(sockfd_recv, set_08_date_lin, sizeof(set_08_date_lin), 0, (struct sockaddr*)&signal_addr, sizeof(signal_addr));
/*
				res = recvfrom(sockfd_recv, recv_buf, 512, 0, NULL, NULL);
				printf("recvfrom  date !!!!!!!!!!\n ");
				if(res <= 0)
				{
					close(sockfd_recv);
					return -1;
				}
				else
				{	
					close(sockfd_recv);
					return 0;
				}
*/
				struct timeval 	timeVal;
					fd_set  fdSet;
					int ret;

					timeVal.tv_sec = signal_report_timeout;
					timeVal.tv_usec = 0;

					FD_ZERO(&fdSet);
					FD_SET(sockfd_recv, &fdSet);

					ret = select((int) sockfd_recv + 1, &fdSet,NULL, NULL, &timeVal);
					if(ret == 0)                     //��ʱ
					{
						close(sockfd_recv);
						return -1;
					}
					if(ret)
					{
						res = recvfrom(sockfd_recv, recv_buf, 512, 0, NULL, NULL);
						if(res <= 0)
						{
							close(sockfd_recv);
							return -1;
						}
						else
						{
							printf("recvfrom signal_id = %d,xiangweichongtu report!!!!!!!!!!\n ",(recv_buf[7]*256+recv_buf[8]) );
							return 0;
						}
						
					}		
				
			}

			//itc100,��λ��ͻ��
			else if(buf[10] == 0x25)
				{
				unsigned char set_25_date_lin[32][47];

				for(i = 0; i < 32;i++)
				{
					bzero(set_25_date_lin[i] ,47);

					set_25_date_lin[i][0] = 0x7e;set_25_date_lin[i][2] = 0x2d;set_25_date_lin[i][4] = 0x01;
				set_25_date_lin[i][7] = buf[7];
				set_25_date_lin[i][8] = buf[8];
				set_25_date_lin[i][9] = 0x20;set_25_date_lin[i][10] = 0x25;

				set_25_date_lin[i][11] = i;set_25_date_lin[i][12] = i+1;
				set_25_date_lin[i][46] = 0x7d;

				sprintf(sql,"select  PHASE_RUSH  from  itc_cfg_conflict    where signal_id = %d  and  PHASE_ID = %d " ,buf[7] * 256 + buf[8] ,i+1);

				//printf("%s\n",sql);
				st->setSQL(sql);
				rs = st->executeQuery();

				if(rs->next() )
				{
					
					bzero(lin_buf,sizeof(lin_buf));
					sprintf(lin_buf,"%s",rs->getString(1).c_str());
					result = strtok( lin_buf, "," );
					
					while( result != NULL ) 
					{	
						lin_res_num= atoi(result);

						set_25_date_lin[i][13 +( lin_res_num -1) ]  = 1;

						result = strtok( NULL, "," );
						
					}
					
				}

				set_25_date_lin[i][45] = buf_check_num(set_25_date_lin[i]);
				sendto(sockfd_recv, set_25_date_lin[i], sizeof(set_25_date_lin[i]), 0, (struct sockaddr*)&signal_addr, sizeof(signal_addr));
/*
				res = recvfrom(sockfd_recv, recv_buf, 512, 0, NULL, NULL);
				printf("recvfrom  date !!!!!!!!!!\n ");
				if(res <= 0)
				{
					close(sockfd_recv);
					return -1;
				}
*/				
				
					struct timeval 	timeVal;
					fd_set  fdSet;
					int ret;

					timeVal.tv_sec = signal_report_timeout;
					timeVal.tv_usec = 0;

					FD_ZERO(&fdSet);
					FD_SET(sockfd_recv, &fdSet);

					ret = select((int) sockfd_recv + 1, &fdSet,NULL, NULL, &timeVal);
					if(ret == 0)                     //��ʱ
					{
						close(sockfd_recv);
						return -1;
					}
					if(ret)
					{
						res = recvfrom(sockfd_recv, recv_buf, 512, 0, NULL, NULL);
						if(res <= 0)
						{
							close(sockfd_recv);
							return -1;
						}
						printf("recvfrom signal_id = %d,xiangwei'chongtu %d report!!!!!!!!!!\n ",(recv_buf[7]*256+recv_buf[8]),i);
					}		
				}
			
				
				close(sockfd_recv);
				return 0;	
				
				
			}
			
			/* �׶ι���Լ��������*/
			else if(buf[10] == 0x09)
			{
				unsigned char set_09_date_lin[64][78];

				for(i = 0; i < 64;i++)
				{
					for(j = 0; j < 78;j++)
					{
						set_09_date_lin[i][j] = set_09_date[i][j];
					}
					
				}
				struct guduyueshu guduyueshu_date[65];
				sprintf(sql,"select Stage_id_row, Stage_id_column, Guodu_value  from  GUODUYUESHU   where signal_id = %d  order by Stage_id_row " ,buf[7] * 256 + buf[8] );
			
				//printf("%s\n",sql);
				st->setSQL(sql);
				rs = st->executeQuery();
				while(rs->next())
				{
					guduyueshu_date[rs->getInt(1)].date[(rs->getInt(2) - 1)]  = rs->getInt(3);
				}
				
				for(i = 0;i < 64;i++)
				{
					set_09_date_lin[i][7] = buf[7];
					set_09_date_lin[i][8] = buf[8];
					set_09_date_lin[i][12] = i+1;

					for(j = 0; j < 64 ;j++)
					{
						set_09_date_lin[i][12 + j] = guduyueshu_date[i + 1].date[j];
					}

					set_09_date_lin[i][76] = buf_check_num(set_09_date_lin[i]);

					sendto(sockfd_recv, set_09_date_lin[i], sizeof(set_09_date_lin[i]), 0, (struct sockaddr*)&signal_addr, sizeof(signal_addr));
/*
					res = recvfrom(sockfd_recv, recv_buf, 512, 0, NULL, NULL);
					printf("recvfrom  date !!!!!!!!!!\n ");
					if(res <= 0)
					{
						close(sockfd_recv);
						return -1;
					}
					else
					{	
						
					}
*/
					
					struct timeval 	timeVal;
					fd_set  fdSet;
					int ret;

					timeVal.tv_sec = signal_report_timeout;
					timeVal.tv_usec = 0;

					FD_ZERO(&fdSet);
					FD_SET(sockfd_recv, &fdSet);

					ret = select((int) sockfd_recv + 1, &fdSet,NULL, NULL, &timeVal);
					if(ret == 0)                     //��ʱ
					{
						close(sockfd_recv);
						return -1;
					}
					if(ret)
					{
						res = recvfrom(sockfd_recv, recv_buf, 512, 0, NULL, NULL);
						if(res <= 0)
						{
							close(sockfd_recv);
							return -1;
						}
						printf("recvfrom signal_id = %d,guodu'yueshu %d report!!!!!!!!!!\n ",(recv_buf[7]*256+recv_buf[8]),i);
					}		
				}

				close(sockfd_recv);
				return 0;
			}

			//itc100,�׶ι���Լ����
			else if(buf[10] == 0x28)
				{
				unsigned char set_09_date_lin[64][79];

				for(i = 0; i < 64;i++)
				{
					bzero(set_09_date_lin[i],79);
					set_09_date_lin[i][0] = 0x7e;set_09_date_lin[i][2] = 0x4d;
					set_09_date_lin[i][4] = 0x01;set_09_date_lin[i][7] = buf[7];set_09_date_lin[i][8]  = buf[8];
					set_09_date_lin[i][9] = 0x20;set_09_date_lin[i][10] = 0x28;
					set_09_date_lin[i][11] = i; set_09_date_lin[i][12] = i+1;  

					sprintf(sql,"select  Stage_id_column, transition_value  from  ITC_CFG_STAGE_TRANSITION   where signal_id = %d  and Stage_id_row = %d order by Stage_id_column" ,buf[7] * 256 + buf[8] ,i+1);
			
					//printf("%s\n",sql);
					st->setSQL(sql);
					rs = st->executeQuery();

					while(rs->next())
						set_09_date_lin[i][13+(rs->getInt(1)-1)] = rs->getInt(2);

					set_09_date_lin[i][77] = buf_check_num(set_09_date_lin[i]);
					set_09_date_lin[i][78] = 0x7d;

					sendto(sockfd_recv, set_09_date_lin[i], sizeof(set_09_date_lin[i]), 0, (struct sockaddr*)&signal_addr, sizeof(signal_addr));
/*
					res = recvfrom(sockfd_recv, recv_buf, 512, 0, NULL, NULL);
					printf("recvfrom  date !!!!!!!!!!\n ");
					if(res <= 0)
					{
						close(sockfd_recv);
						return -1;
					}
	*/
					struct timeval 	timeVal;
					fd_set  fdSet;
					int ret;

					timeVal.tv_sec = signal_report_timeout;
					timeVal.tv_usec = 0;

					FD_ZERO(&fdSet);
					FD_SET(sockfd_recv, &fdSet);

					ret = select((int) sockfd_recv + 1, &fdSet,NULL, NULL, &timeVal);
					if(ret == 0)                     //��ʱ
					{
						close(sockfd_recv);
						return -1;
					}
					if(ret)
					{
						res = recvfrom(sockfd_recv, recv_buf, 512, 0, NULL, NULL);
						if(res <= 0)
						{
							close(sockfd_recv);
							return -1;
						}
						printf("recvfrom signal_id = %d,guodu'yueshu %d report!!!!!!!!!!\n ",(recv_buf[7]*256+recv_buf[8]),i);
					}						
				}

				close(sockfd_recv);
				return 0;
			}
			
			/* �ֶ���������*/
			else if(buf[10] == 0x0a)
			{
				unsigned char set_0a_date_lin[28];

				for(i = 0; i < 28;i++)
				{
					
					set_0a_date_lin[i] = set_0a_date[i];
					
				}
				//int num_lin = 0;

				for(i = 12; i <26;i++)
				{
					set_0a_date_lin[i] = 0;
				}
				set_0a_date_lin[7] = buf[7];
				set_0a_date_lin[8] = buf[8];
				sprintf(sql,"select ANJIAN,DYJD,SETIMEOUT  from  SDPZ_INFO   where signal_id = %d  order by ANJIAN " ,buf[7] * 256 + buf[8] );
			
				//printf("%s\n",sql);
				st->setSQL(sql);
				rs = st->executeQuery();
				while(rs->next())
				{
					set_0a_date_lin[12 + rs->getInt(1) - 1] = rs->getInt(2);
					set_0a_date_lin[22] = ((rs->getInt(3) ) >> 24) & 0xff;
					set_0a_date_lin[23] = ((rs->getInt(3) ) >> 16) & 0xff;
					set_0a_date_lin[24] = ((rs->getInt(3) ) >> 8) & 0xff;
					set_0a_date_lin[25] = (rs->getInt(3) ) & 0xff;
				}
				

				set_0a_date_lin[26] = buf_check_num(set_0a_date_lin);

				sendto(sockfd_recv, set_0a_date_lin, sizeof(set_0a_date_lin), 0, (struct sockaddr*)&signal_addr, sizeof(signal_addr));
/*
				res = recvfrom(sockfd_recv, recv_buf, 512, 0, NULL, NULL);
				printf("recvfrom  date !!!!!!!!!!\n ");
				if(res <= 0)
				{
					close(sockfd_recv);
					return -1;
				}
				else
				{	
					close(sockfd_recv);
					return 0;
				}
	*/
					struct timeval 	timeVal;
					fd_set  fdSet;
					int ret;

					timeVal.tv_sec = signal_report_timeout;
					timeVal.tv_usec = 0;

					FD_ZERO(&fdSet);
					FD_SET(sockfd_recv, &fdSet);

					ret = select((int) sockfd_recv + 1, &fdSet,NULL, NULL, &timeVal);
					if(ret == 0)                     //��ʱ
					{
						close(sockfd_recv);
						return -1;
					}
					if(ret)
					{
						res = recvfrom(sockfd_recv, recv_buf, 512, 0, NULL, NULL);
						if(res <= 0)
						{
							close(sockfd_recv);
							return -1;
						}
						else
						{
							printf("recvfrom TC100 signal_id = %d,shoudongcanshu report!!!!!!!!!!\n ",(recv_buf[7]*256+recv_buf[8]) );
							close(sockfd_recv);
							return 0;
						}
					}					
			}

			//ITC100,�ֶ���������
			else if(buf[10] == 0x2e)
				{
				unsigned char set_2e_date_lin[8][25];

				for(i = 0; i < 8;i++)
				{
					bzero(set_2e_date_lin[i],25);
					set_2e_date_lin[i][0] = 0x7e;set_2e_date_lin[i][2] = 0x17;set_2e_date_lin[i][4] = 0x01;
					set_2e_date_lin[i][7] = buf[7];
					set_2e_date_lin[i][8] = buf[8];
					set_2e_date_lin[i][9] = 0x20;set_2e_date_lin[i][10] = 0x2e;
					set_2e_date_lin[i][11] = i;
					set_2e_date_lin[i][12] = i+1;

					sprintf(sql,"select key,stage_id  from  itc_cfg_manual   where signal_id = %d  and list_unit_id = %d order by key" ,buf[7] * 256 + buf[8] ,i+1);
			
					//printf("%s\n",sql);
					st->setSQL(sql);
					rs = st->executeQuery();

					while(rs->next())
					{
					//set_2e_date_lin[12 + rs->getInt(1) - 1] = rs->getInt(2);
					//set_2e_date_lin[22] = ((rs->getInt(3) ) >> 24) & 0xff;
					//set_2e_date_lin[23] = ((rs->getInt(3) ) >> 16) & 0xff;
					//set_2e_date_lin[24] = ((rs->getInt(3) ) >> 8) & 0xff;
					//set_2e_date_lin[25] = (rs->getInt(3) ) & 0xff;
					if(rs->getInt(1) > 0 )
						set_2e_date_lin[i][12+rs->getInt(1)] =rs->getInt(2);
					else
						set_2e_date_lin[i][12+10] =rs->getInt(2);
					}

					set_2e_date_lin[i][23] = buf_check_num(set_2e_date_lin[i]);
					set_2e_date_lin[i][24] = 0x7d;

					sendto(sockfd_recv, set_2e_date_lin[i], sizeof(set_2e_date_lin[i]), 0, (struct sockaddr*)&signal_addr, sizeof(signal_addr));
/*
					res = recvfrom(sockfd_recv, recv_buf, 512, 0, NULL, NULL);
					printf("recvfrom  date !!!!!!!!!!\n ");
					if(res <= 0)
					{
						close(sockfd_recv);
						return -1;
					}
*/

					struct timeval 	timeVal;
					fd_set  fdSet;
					int ret;

					timeVal.tv_sec = signal_report_timeout;
					timeVal.tv_usec = 0;

					FD_ZERO(&fdSet);
					FD_SET(sockfd_recv, &fdSet);

					ret = select((int) sockfd_recv + 1, &fdSet,NULL, NULL, &timeVal);
					if(ret == 0)                     //��ʱ
					{
						close(sockfd_recv);
						return -1;
					}
					if(ret)
					{
						res = recvfrom(sockfd_recv, recv_buf, 512, 0, NULL, NULL);
						if(res <= 0)
						{
							close(sockfd_recv);
							return -1;
						}
						printf("recvfrom signal_id = %d,shoudongcanshu %d report!!!!!!!!!!\n ",(recv_buf[7]*256+recv_buf[8]),i);
					}							
				}
				close(sockfd_recv);
					return 0;

				}
				

			
			/* ������������*/
			else if(buf[10] == 0x0b)
			{
				unsigned char set_0b_date_lin[26];

				for(i = 0; i < 26;i++)
				{
					
					set_0b_date_lin[i] = set_0b_date[i];
					
				}
				
				set_0b_date_lin[7] = buf[7];
				set_0b_date_lin[8] = buf[8];

				sprintf(sql,"select  Jinji_id,Jinji_stage,Jinji_logo  from  JINJIPEIZHI   where signal_id = %d  order by Jinji_id " ,buf[7] * 256 + buf[8] );
			
				//printf("%s\n",sql);
				st->setSQL(sql);
				rs = st->executeQuery();
				while(rs->next())
				{
					set_0b_date_lin[13 + (rs->getInt(1) - 1) * 3 ] = rs->getInt(2);
					set_0b_date_lin[14 + (rs->getInt(1) - 1) * 3 ] = rs->getInt(3);
				}

				set_0b_date_lin[24] = buf_check_num(set_0b_date_lin);

				sendto(sockfd_recv, set_0b_date_lin, sizeof(set_0b_date_lin), 0, (struct sockaddr*)&signal_addr, sizeof(signal_addr));

				res = recvfrom(sockfd_recv, recv_buf, 512, 0, NULL, NULL);
				printf("recvfrom  date !!!!!!!!!!\n ");
				if(res <= 0)
				{
					close(sockfd_recv);
					return -1;
				}
				else
				{	
					close(sockfd_recv);
					return 0;
				}
			}

			//itc100,������������
			else if(buf[10] == 0x29)
				{
				unsigned char set_29_date_lin[30];

				bzero(set_29_date_lin,30);
				
				set_29_date_lin[0] = 0x7e;set_29_date_lin[2] = 0x1c;set_29_date_lin[4] = 0x01;
				set_29_date_lin[7] = buf[7];
				set_29_date_lin[8] = buf[8];
				set_29_date_lin[9] = 0x20;set_29_date_lin[10] = 0x29;

				sprintf(sql,"select  urgent_id,list_unit_id,stage_id,priority,flag  from  itc_cfg_urgent   where signal_id = %d  order by urgent_id " ,buf[7] * 256 + buf[8] );
			
				printf("%s\n",sql);
				st->setSQL(sql);
				rs = st->executeQuery();
				while(rs->next())
				{
					set_29_date_lin[12+ (rs->getInt(1) - 1) * 4] = rs->getInt(2);
					set_29_date_lin[13+ (rs->getInt(1) - 1) * 4] = rs->getInt(3);
					set_29_date_lin[14+ (rs->getInt(1) - 1) * 4] = rs->getInt(4);
					set_29_date_lin[15+ (rs->getInt(1) - 1) * 4] = rs->getInt(5);
				}

				set_29_date_lin[28] = buf_check_num(set_29_date_lin);
				set_29_date_lin[29] = 0x7d;

				sendto(sockfd_recv, set_29_date_lin, sizeof(set_29_date_lin), 0, (struct sockaddr*)&signal_addr, sizeof(signal_addr));
/*
				res = recvfrom(sockfd_recv, recv_buf, 512, 0, NULL, NULL);
				printf("recvfrom  date !!!!!!!!!!\n ");
				if(res <= 0)
				{
					close(sockfd_recv);
					return -1;
				}
				else
				{	
					close(sockfd_recv);
					return 0;
				}
*/
					struct timeval 	timeVal;
					fd_set  fdSet;
					int ret;

					timeVal.tv_sec = signal_report_timeout;
					timeVal.tv_usec = 0;

					FD_ZERO(&fdSet);
					FD_SET(sockfd_recv, &fdSet);

					ret = select((int) sockfd_recv + 1, &fdSet,NULL, NULL, &timeVal);
					if(ret == 0)                     //��ʱ
					{
						printf("signal no reply::jiang ce qi\n");
						close(sockfd_recv);
						return -1;
					}
					if(ret)                //������
					{
						res = recvfrom(sockfd_recv, recv_buf, 512, 0, NULL, NULL);
						if(res == 0)
						{
							printf("select timeout::jin ji can shu\n");
							close(sockfd_recv);
							return -1;
						}
						else
						{
							
							close(sockfd_recv);
							return 0;	
						}
					}								
			}
			
			/* ���Ȳ�������*/
			else if(buf[10] == 0x0c)
			{
				unsigned char set_0c_date_lin[26];

				for(i = 0; i < 26;i++)
				{
					
					set_0c_date_lin[i] = set_0c_date[i];
					
				}
				
				set_0c_date_lin[7] = buf[7];
				set_0c_date_lin[8] = buf[8];

				sprintf(sql,"select  Priority_id,Priority_stage,Priority_logo  from  PRIORIT_CONFIG  where signal_id = %d  order by Priority_id " ,buf[7] * 256 + buf[8] );
			
				//printf("%s\n",sql);
				st->setSQL(sql);
				rs = st->executeQuery();
				while(rs->next())
				{
					set_0c_date_lin[13 + (rs->getInt(1) - 1) * 3 ] = rs->getInt(2);
					set_0c_date_lin[14 + (rs->getInt(1) - 1) * 3 ] = rs->getInt(3);
				}

				set_0c_date_lin[24] = buf_check_num(set_0c_date_lin);

				sendto(sockfd_recv, set_0c_date_lin, sizeof(set_0c_date_lin), 0, (struct sockaddr*)&signal_addr, sizeof(signal_addr));
/*
				res = recvfrom(sockfd_recv, recv_buf, 512, 0, NULL, NULL);
				printf("recvfrom  date !!!!!!!!!!\n ");
				if(res <= 0)
				{
					close(sockfd_recv);
					return -1;
				}
				else
				{	
					close(sockfd_recv);
					return 0;
				}
*/
				struct timeval 	timeVal;
					fd_set  fdSet;
					int ret;

					timeVal.tv_sec = signal_report_timeout;
					timeVal.tv_usec = 0;

					FD_ZERO(&fdSet);
					FD_SET(sockfd_recv, &fdSet);

					ret = select((int) sockfd_recv + 1, &fdSet,NULL, NULL, &timeVal);
					if(ret == 0)                     //��ʱ
					{
						printf("signal no reply::jiang ce qi\n");
						close(sockfd_recv);
						return -1;
					}
					if(ret)                //������
					{
						res = recvfrom(sockfd_recv, recv_buf, 512, 0, NULL, NULL);
						if(res == 0)
						{
							printf("select timeout::you xian can shu\n");
							close(sockfd_recv);
							return -1;
						}
						else
						{
							
							close(sockfd_recv);
							return 0;	
						}
					}		
				
			}

			//itc100, ���Ȳ���
			else if(buf[10] == 0x2a)
				{
				unsigned char set_2a_date_lin[30];

				bzero(set_2a_date_lin,30);
				
				set_2a_date_lin[0] = 0x7e;set_2a_date_lin[2] = 0x1c;set_2a_date_lin[4] = 0x01;
				set_2a_date_lin[7] = buf[7];
				set_2a_date_lin[8] = buf[8];
				set_2a_date_lin[9] = 0x20;set_2a_date_lin[10] = 0x2a;

				sprintf(sql,"select  Priority_id,list_unit_id,stage_id,Priority,flag from  itc_cfg_priority  where signal_id = %d  order by Priority_id " ,buf[7] * 256 + buf[8] );
			
				printf("%s\n",sql);
				st->setSQL(sql);
				rs = st->executeQuery();
				while(rs->next())
				{
					set_2a_date_lin[12 + (rs->getInt(1) - 1) * 4 ] = rs->getInt(2);
					set_2a_date_lin[13 + (rs->getInt(1) - 1) * 4 ] = rs->getInt(3);
					set_2a_date_lin[14 + (rs->getInt(1) - 1) * 4 ] = rs->getInt(4);
					set_2a_date_lin[15 + (rs->getInt(1) - 1) * 4 ] = rs->getInt(5);
				}

				set_2a_date_lin[28] = buf_check_num(set_2a_date_lin);
				set_2a_date_lin[29] = 0x7d;

				sendto(sockfd_recv, set_2a_date_lin, sizeof(set_2a_date_lin), 0, (struct sockaddr*)&signal_addr, sizeof(signal_addr));
/*
				res = recvfrom(sockfd_recv, recv_buf, 512, 0, NULL, NULL);
				printf("recvfrom  date !!!!!!!!!!\n ");
				if(res <= 0)
				{
					close(sockfd_recv);
					return -1;
				}
				else
				{	
					close(sockfd_recv);
					return 0;
				}
*/	
				struct timeval 	timeVal;
					fd_set  fdSet;
					int ret;

					timeVal.tv_sec = signal_report_timeout;
					timeVal.tv_usec = 0;

					FD_ZERO(&fdSet);
					FD_SET(sockfd_recv, &fdSet);

					ret = select((int) sockfd_recv + 1, &fdSet,NULL, NULL, &timeVal);
					if(ret == 0)                     //��ʱ
					{
						printf("signal no reply::jiang ce qi\n");
						close(sockfd_recv);
						return -1;
					}
					if(ret)                //������
					{
						res = recvfrom(sockfd_recv, recv_buf, 512, 0, NULL, NULL);
						if(res == 0)
						{
							printf("select timeout::you xian can shu\n");
							close(sockfd_recv);
							return -1;
						}
						else
						{
							
							close(sockfd_recv);
							return 0;	
						}
					}		

			}
			
			/* �׶�������������*/
			else if(buf[10] == 0x14)
			{
				unsigned char set_14_date_lin[24][87];

				for(i = 0; i < 24;i++)
				{
					for(j = 0; j < 90;j++)
					{
						set_14_date_lin[i][j] = set_14_date[i][j];
					}
					
				}
				struct shield_stage_info shield_stage_info_date[25];
				
				sprintf(sql,"select  SHIELD_ID,STAGE1_ID,STAGE1_TYPE,STAGE1_OCC_DETECTOR,STAGE1_OCC_VALUE,STAGE1_OCC_CHECK, STAGE1_OCC_LONG,STAGE2_ID,STAGE2_TYPE,STAGE2_OCC_DETECTOR,STAGE2_OCC_VALUE,STAGE2_OCC_CHECK, STAGE2_OCC_LONG,STAGE3_ID,STAGE3_TYPE,STAGE3_OCC_DETECTOR,STAGE3_OCC_VALUE,STAGE3_OCC_CHECK, STAGE3_OCC_LONG ,STAGE4_ID,STAGE4_TYPE,STAGE4_OCC_DETECTOR,STAGE4_OCC_VALUE,STAGE4_OCC_CHECK, STAGE4_OCC_LONG    from  SHIELD_STAGE_INFO   where signal_id = %d  order by SHIELD_ID " ,buf[7] * 256 + buf[8] );
			
				printf("%s\n",sql);
				st->setSQL(sql);
				rs = st->executeQuery();
				while(rs->next())
				{
				
					shield_stage_info_date[rs->getInt(1)].stage_id = rs->getInt(1);
					for(i = 0; i < 4; i++)
					{
						
						shield_stage_info_date[rs->getInt(1)].stage1_id[i] = rs->getInt(2 + i* 6);
						shield_stage_info_date[rs->getInt(1)].stage1_type[i] = rs->getInt(3 + i* 6);
						shield_stage_info_date[rs->getInt(1)].stage1_occ_value[i] = rs->getInt(5 + i* 6);
						shield_stage_info_date[rs->getInt(1)].stage1_occ_check[i] = rs->getInt(6 + i* 6);
						shield_stage_info_date[rs->getInt(1)].stage1_occ_long[i] = rs->getInt(7 + i* 6);

						/*��λ����������*/
						bzero(lin_buf,sizeof(lin_buf));
						sprintf(lin_buf,"%s",rs->getString(4 + i* 6).c_str());
						result = strtok( lin_buf, "," );
						
						while( result != NULL ) 
						{
						  
							lin_res_num= atoi(result);

							if(lin_res_num>=1  &&  lin_res_num<=8)
							{
								if(lin_res_num % 8 == 0)
								{
									shield_stage_info_date[rs->getInt(1)].stage1_occ_detector[i][0] += (0x80 >> (7));
								}
								else
								{
									shield_stage_info_date[rs->getInt(1)].stage1_occ_detector[i][0]  += (0x80 >> (lin_res_num % 8 - 1));
								}
							}
							else if(lin_res_num>=9  &&  lin_res_num<=16)
							{
								if(lin_res_num % 8 == 0)
								{
									shield_stage_info_date[rs->getInt(1)].stage1_occ_detector[i][1]  += (0x80 >>(7));
								}
								else
								{
									shield_stage_info_date[rs->getInt(1)].stage1_occ_detector[i][1] += (0x80 >>(lin_res_num % 8 - 1));
								}
								
							}
							else if(lin_res_num>=17  &&  lin_res_num<=24)
							{
								if(lin_res_num % 8 == 0)
								{
									shield_stage_info_date[rs->getInt(1)].stage1_occ_detector[i][2]  += (0x80 >> (7));
								}
								else
								{
									shield_stage_info_date[rs->getInt(1)].stage1_occ_detector[i][2]  += (0x80 >> (lin_res_num % 8 - 1));
								}
							}
							else if(lin_res_num>=25 &&  lin_res_num<=32)
							{
								if(lin_res_num % 8 == 0)
								{
									shield_stage_info_date[rs->getInt(1)].stage1_occ_detector[i][3] += (0x80 >> (7));
								}
								else
								{
									shield_stage_info_date[rs->getInt(1)].stage1_occ_detector[i][3]  += (0x80 >> (lin_res_num % 8 - 1));
								}
							}
							else if(lin_res_num>=33  &&  lin_res_num<=40)
							{
								if(lin_res_num % 8 == 0)
								{
									shield_stage_info_date[rs->getInt(1)].stage1_occ_detector[i][4]  += (0x80 >> (7));
								}
								else
								{
									shield_stage_info_date[rs->getInt(1)].stage1_occ_detector[i][4]  += (0x80 >> (lin_res_num % 8 - 1));
								}
							}
							else if(lin_res_num>=41  &&  lin_res_num<=48)
							{
								if(lin_res_num % 8 == 0)
								{
									shield_stage_info_date[rs->getInt(1)].stage1_occ_detector[i][5]  += (0x80 >> (7));
								}
								else
								{
									shield_stage_info_date[rs->getInt(1)].stage1_occ_detector[i][5]  += (0x80 >> (lin_res_num % 8 - 1));
								}
							}
							else if(lin_res_num>=49  &&  lin_res_num<=56)
							{
								if(lin_res_num % 8 == 0)
								{
									shield_stage_info_date[rs->getInt(1)].stage1_occ_detector[i][6]  += (0x80 >>(7));
								}
								else
								{
									shield_stage_info_date[rs->getInt(1)].stage1_occ_detector[i][6]  += (0x80 >>(lin_res_num % 8 - 1));
								}
							}
							else if(lin_res_num>=57  &&  lin_res_num<=64)
							{
								if(lin_res_num % 8 == 0)
								{
									shield_stage_info_date[rs->getInt(1)].stage1_occ_detector[i][7]  += (0x80 >> (7));
								}
								else
								{
									shield_stage_info_date[rs->getInt(1)].stage1_occ_detector[i][7] += (0x80 >> (lin_res_num % 8 - 1));
								}
							}
							else if(lin_res_num>=65  &&  lin_res_num<=72)
							{
								if(lin_res_num % 8 == 0)
								{
									shield_stage_info_date[rs->getInt(1)].stage1_occ_detector[i][8] += (0x80 >> (7));
								}
								else
								{
									shield_stage_info_date[rs->getInt(1)].stage1_occ_detector[i][8]  += (0x80 >> (lin_res_num % 8 - 1));
								}
							}
							else if(lin_res_num>=73  &&  lin_res_num<=80)
							{
								if(lin_res_num % 8 == 0)
								{
									shield_stage_info_date[rs->getInt(1)].stage1_occ_detector[i][9]  += (0x80 >>(7));
								}
								else
								{
									shield_stage_info_date[rs->getInt(1)].stage1_occ_detector[i][9]  += (0x80 >> (lin_res_num % 8 - 1));
								}
							}
							else if(lin_res_num>=81  &&  lin_res_num<=88)
							{
								if(lin_res_num % 8 == 0)
								{
									shield_stage_info_date[rs->getInt(1)].stage1_occ_detector[i][10]  += (0x80 >> (7));
								}
								else
								{
									shield_stage_info_date[rs->getInt(1)].stage1_occ_detector[i][10] += (0x80 >> (lin_res_num % 8 - 1));
								}
							}
							else if(lin_res_num>=89  &&  lin_res_num<=96)
							{
								if(lin_res_num % 8 == 0)
								{
									shield_stage_info_date[rs->getInt(1)].stage1_occ_detector[i][11]  += (0x80 >> (7));
								}
								else
								{
									shield_stage_info_date[rs->getInt(1)].stage1_occ_detector[i][11] += (0x80 >> (lin_res_num % 8 - 1));
								}
							}
							else if(lin_res_num>=97  &&  lin_res_num<=104)
							{
								if(lin_res_num % 8 == 0)
								{
									shield_stage_info_date[rs->getInt(1)].stage1_occ_detector[i][12]  += (0x80 >> (7));
								}
								else
								{
									shield_stage_info_date[rs->getInt(1)].stage1_occ_detector[i][12] += (0x80 >> (lin_res_num % 8 - 1));
								}
							}
							
						   	result = strtok( NULL, "," );
						}
					

					}
					
				}

				for(i = 0; i < 24; i++)
				{
					set_14_date_lin[i][7] = buf[7];
					set_14_date_lin[i][8] = buf[8];
					set_14_date_lin[i][12] = i+1;

					set_14_date_lin[i][12]  =shield_stage_info_date[i + 1].stage_id;
					for(j = 0; j < 4 ;j++)
					{	
						
						set_14_date_lin[i][13 + 18 * j] =  shield_stage_info_date[i + 1].stage1_id[j];
						set_14_date_lin[i][14 + 18 * j] =  shield_stage_info_date[i + 1].stage1_type[j];
						set_14_date_lin[i][28 + 18 * j] =  shield_stage_info_date[i + 1].stage1_occ_value[j];
						set_14_date_lin[i][29 + 18 * j] =  shield_stage_info_date[i + 1].stage1_occ_check[j];
						set_14_date_lin[i][30 + 18 * j] =  shield_stage_info_date[i + 1].stage1_occ_long[j];
						int m;

						for(m = 0; m < 13 ; m++)
						{
							set_14_date_lin[i][15 + 18 * j + m] = shield_stage_info_date[i + 1].stage1_occ_detector[j][m];
						}
					}

					set_14_date_lin[i][85] = buf_check_num(set_14_date_lin[i]);

					sendto(sockfd_recv, set_14_date_lin[i], sizeof(set_14_date_lin[i]), 0, (struct sockaddr*)&signal_addr, sizeof(signal_addr));
#if 0

					int www;
					for(www = 0 ;www<87 ;www++ )
					{
						printf("%02x",set_14_date_lin[i][www]);
					}
					printf("\n");
#endif
/*
					res = recvfrom(sockfd_recv, recv_buf, 512, 0, NULL, NULL);
					printf("recvfrom  date !!!!!!!!!!\n ");
					if(res <= 0)
					{
						close(sockfd_recv);
						return -1;
					}
					else
					{	
					}
		*/
					struct timeval 	timeVal;
					fd_set  fdSet;
					int ret;

					timeVal.tv_sec = signal_report_timeout;
					timeVal.tv_usec = 0;

					FD_ZERO(&fdSet);
					FD_SET(sockfd_recv, &fdSet);

					ret = select((int) sockfd_recv + 1, &fdSet,NULL, NULL, &timeVal);
					if(ret == 0)                     //��ʱ
					{
						close(sockfd_recv);
						return -1;
					}
					if(ret)
					{
						res = recvfrom(sockfd_recv, recv_buf, 512, 0, NULL, NULL);
						if(res <= 0)
						{
							close(sockfd_recv);
							return -1;
						}
						printf("recvfrom signal_id = %d,jie duan ping bi lei xing %d report!!!!!!!!!!\n ",(recv_buf[7]*256+recv_buf[8]),i);
					}					
				}
				close(sockfd_recv);
				return 0;
			}

			/* �׶���������*/
			else if(buf[10] == 0x0d)
		//	else if(buf[10] == 0x00)
			{
				unsigned char set_0d_date_lin[128][97];

				for(i = 0; i < 128;i++)
				{
					for(j = 0; j < 97;j++)
					{
						set_0d_date_lin[i][j] = set_0d_date[i][j];
					}
					
				}
#if 0
				for(i = 0;i < 128; i ++)
				{
					sendto(sockfd_recv, set_0d_date_lin[i], sizeof(set_0d_date_lin[i]), 0, (struct sockaddr*)&signal_addr, sizeof(signal_addr));

					res = recvfrom(sockfd_recv, recv_buf, 512, 0, NULL, NULL);
				}
#endif			
				struct diaodu_info_and_time diaodu_info_and_time_date[129];	

				sprintf(sql,"select Ddb_id, Ddb_type,Ddb_priority,Week_ddb_type,Date_ddb_type_y,Date_ddb_type_r,Time_table_number_1,(select count(*) from SJBSDSZ_INFO where Time_id=t.Time_table_number_1  and  signal_id = %d and FUNCTION_TYPE = 2)  from  DIAODU_INFO t   where signal_id = %d  and FUNCTION_TYPE = 2  order by Ddb_id " ,buf[7] * 256 + buf[8],buf[7] * 256 + buf[8] );
			
				//printf("%s\n",sql);
				st->setSQL(sql);
				rs = st->executeQuery();
				while(rs->next())
				{
					diaodu_info_and_time_date[rs->getInt(1)].Ddb_type =rs->getInt(2)  ;
					diaodu_info_and_time_date[rs->getInt(1)].Ddb_priority = rs->getInt(3) ;
					diaodu_info_and_time_date[rs->getInt(1)].Time_table_number_1 = rs->getInt(7) ;
					diaodu_info_and_time_date[rs->getInt(1)].time_num = rs->getInt(8) ;

					
				if(rs->getInt(1)>69  && rs->getInt(1) < 74 )
				{
					printf(" rs->getInt(1) = %d   rs->getInt(2) = %02x\n",rs->getInt(1),rs->getInt(2));
				}

					
					diaodu_info_and_time_date[rs->getInt(1)].Week_ddb_type = 0;
					bzero(lin_buf,sizeof(lin_buf));
					sprintf(lin_buf,"%s",rs->getString(4).c_str());
					result = strtok( lin_buf, "," );
					
					while( result != NULL ) 
					{	
						lin_res_num= atoi(result);

						if(lin_res_num == 7)
						{
							diaodu_info_and_time_date[rs->getInt(1)].Week_ddb_type += 0x01;
						}
						else
						{
							diaodu_info_and_time_date[rs->getInt(1)].Week_ddb_type += (0x01 << lin_res_num);
						}
						
						result = strtok( NULL, "," );
					}

					diaodu_info_and_time_date[rs->getInt(1)].Date_ddb_type_y[0] = 0;
					diaodu_info_and_time_date[rs->getInt(1)].Date_ddb_type_y[1] = 0;	
					bzero(lin_buf,sizeof(lin_buf));
					sprintf(lin_buf,"%s",rs->getString(5).c_str());
					result = strtok( lin_buf, "," );
					
					while( result != NULL ) 
					{	
						lin_res_num= atoi(result);


						if(lin_res_num>=1  &&  lin_res_num<=7)
						{
							diaodu_info_and_time_date[rs->getInt(1)].Date_ddb_type_y[0]  += (0x01 << (lin_res_num % 8 ));
					
						}
						else if(lin_res_num>=8  &&  lin_res_num<=15)
						{
							diaodu_info_and_time_date[rs->getInt(1)].Date_ddb_type_y[2]  += (0x01 << (lin_res_num % 8 ));
						
						}
#if 0				
						if(lin_res_num>=1  &&  lin_res_num<=8)
						{
							if(lin_res_num % 8 == 0)
							{
								diaodu_info_and_time_date[rs->getInt(1)].Date_ddb_type_y[0] += (0x01 << (7));
							}
							else
							{
								diaodu_info_and_time_date[rs->getInt(1)].Date_ddb_type_y[0]  += (0x01 << (lin_res_num % 8 - 1));
							}
						}
						else if(lin_res_num>=9  &&  lin_res_num<=16)
						{
							if(lin_res_num % 8 == 0)
							{
								diaodu_info_and_time_date[rs->getInt(1)].Date_ddb_type_y[1] += (0x01 << (7));
							}
							else
							{
								diaodu_info_and_time_date[rs->getInt(1)].Date_ddb_type_y[1]  += (0x01 << (lin_res_num % 8 - 1));
							}
						}
#endif					
						result = strtok( NULL, "," );
					}


					diaodu_info_and_time_date[rs->getInt(1)].Date_ddb_type_r[0] = 0;
					diaodu_info_and_time_date[rs->getInt(1)].Date_ddb_type_r[1] = 0;
					diaodu_info_and_time_date[rs->getInt(1)].Date_ddb_type_r[2] = 0;
					diaodu_info_and_time_date[rs->getInt(1)].Date_ddb_type_r[3] = 0;	
					bzero(lin_buf,sizeof(lin_buf));
					sprintf(lin_buf,"%s",rs->getString(6).c_str());
					result = strtok( lin_buf, "," );
					
					while( result != NULL ) 
					{	
						lin_res_num= atoi(result);
						if(lin_res_num>=1  &&  lin_res_num<=7)
						{	
							diaodu_info_and_time_date[rs->getInt(1)].Date_ddb_type_r[0]  += (0x01 << (lin_res_num % 8 ));
							
						}
						else if(lin_res_num>=8  &&  lin_res_num<=15)
						{
							diaodu_info_and_time_date[rs->getInt(1)].Date_ddb_type_r[1]  += (0x01 << (lin_res_num % 8 ));
							
						}
						else if(lin_res_num>=16  &&  lin_res_num<=23)
						{
							diaodu_info_and_time_date[rs->getInt(1)].Date_ddb_type_r[2]  += (0x01 << (lin_res_num % 8 ));
						}
						else if(lin_res_num>=24  &&  lin_res_num<=31)
						{
							diaodu_info_and_time_date[rs->getInt(1)].Date_ddb_type_r[3]  += (0x01 << (lin_res_num % 8 ));
						}
#if 0				
						if(lin_res_num>=1  &&  lin_res_num<=8)
						{
							if(lin_res_num % 8 == 0)
							{
								diaodu_info_and_time_date[rs->getInt(1)].Date_ddb_type_r[0] += (0x01 << (7));
							}
							else
							{
								diaodu_info_and_time_date[rs->getInt(1)].Date_ddb_type_r[0]  += (0x01 << (lin_res_num % 8 - 1));
							}
						}
						else if(lin_res_num>=9  &&  lin_res_num<=16)
						{
							if(lin_res_num % 8 == 0)
							{
								diaodu_info_and_time_date[rs->getInt(1)].Date_ddb_type_r[1] += (0x01 << (7));
							}
							else
							{
								diaodu_info_and_time_date[rs->getInt(1)].Date_ddb_type_r[1]  += (0x01 << (lin_res_num % 8 - 1));
							}
						}
						else if(lin_res_num>=17  &&  lin_res_num<=24)
						{
							if(lin_res_num % 8 == 0)
							{
								diaodu_info_and_time_date[rs->getInt(1)].Date_ddb_type_r[2] += (0x01 << (7));
							}
							else
							{
								diaodu_info_and_time_date[rs->getInt(1)].Date_ddb_type_r[2]  += (0x01 << (lin_res_num % 8 - 1));
							}
						}
						else if(lin_res_num>=25  &&  lin_res_num<=32)
						{
							if(lin_res_num % 8 == 0)
							{
								diaodu_info_and_time_date[rs->getInt(1)].Date_ddb_type_r[3] += (0x01 << (7));
							}
							else
							{
								diaodu_info_and_time_date[rs->getInt(1)].Date_ddb_type_r[3]  += (0x01 << (lin_res_num % 8 - 1));
							}
						}
#endif					
						result = strtok( NULL, "," );
					}
					
				}

				sprintf(sql,"select Time_id, Hour,Minute,SHIELD_ID,time_set_number  from  SJBSDSZ_INFO    where signal_id = %d and FUNCTION_TYPE = 2 order by Time_id " ,buf[7] * 256 + buf[8]);
			
			//	printf("%s\n",sql);
				st->setSQL(sql);
				rs = st->executeQuery();
				
				while( rs->next() ) 
				{
				//	printf("rs->getInt(1) = %d rs->getInt(2) = %d   rs->getInt(3) \n ",rs->getInt(1),rs->getInt(2),rs->getInt(3));
						
					for(i = 0; i <128 ;i++ )
					{
						if(diaodu_info_and_time_date[i + 1].Time_table_number_1 == rs->getInt(1))
						{
							//printf("rs->getInt(1) = %d rs->getInt(2) = %d   rs->getInt(3) \n ",rs->getInt(1),rs->getInt(2),rs->getInt(3));
							diaodu_info_and_time_date[i + 1].Hour[rs->getInt(5) -1] = rs->getInt(2);
							diaodu_info_and_time_date[i + 1].Minute[rs->getInt(5) -1] = rs->getInt(3);
							diaodu_info_and_time_date[i + 1].SHIELD_ID[rs->getInt(5) -1] = rs->getInt(4);
							break;
						}
					}
				}

				
				for(i = 0;i < 128; i ++)
				{
					set_0d_date_lin[i][7] = buf[7];
					set_0d_date_lin[i][8] = buf[8];
					set_0d_date_lin[i][12] = i+1;

					set_0d_date_lin[i][13] = diaodu_info_and_time_date[i + 1].Ddb_type;
					set_0d_date_lin[i][14] = diaodu_info_and_time_date[i + 1].Ddb_priority;
					set_0d_date_lin[i][15] = diaodu_info_and_time_date[i + 1].Week_ddb_type;
					set_0d_date_lin[i][16] = diaodu_info_and_time_date[i + 1].Date_ddb_type_y[1];
					set_0d_date_lin[i][17] = diaodu_info_and_time_date[i + 1].Date_ddb_type_y[0];
					set_0d_date_lin[i][18] = diaodu_info_and_time_date[i + 1].Date_ddb_type_r[3];
					set_0d_date_lin[i][19] = diaodu_info_and_time_date[i + 1].Date_ddb_type_r[2];
					set_0d_date_lin[i][20] = diaodu_info_and_time_date[i + 1].Date_ddb_type_r[1];
					set_0d_date_lin[i][21] = diaodu_info_and_time_date[i + 1].Date_ddb_type_r[0];

					set_0d_date_lin[i][22] = diaodu_info_and_time_date[i + 1].time_num;
#if 1
					for(j = 0; j < 24 ; j++)
					{
						set_0d_date_lin[i][23 + j * 3 ] = diaodu_info_and_time_date[i + 1].Hour[j];
						set_0d_date_lin[i][24 + j * 3 ] = diaodu_info_and_time_date[i + 1].Minute[j];
						set_0d_date_lin[i][25 + j * 3 ] = diaodu_info_and_time_date[i + 1].SHIELD_ID[j];
						
					}
#endif
					set_0d_date_lin[i][95] = buf_check_num(set_0d_date_lin[i]);

#if 0
					int www;

					if(i >69  && i < 74 )
					{
						for(www = 0; www <97; www++)
							printf("%02x",set_0d_date_lin[i][www] );
						printf("\n");
					}

#endif

					sendto(sockfd_recv, set_0d_date_lin[i], sizeof(set_0d_date_lin[i]), 0, (struct sockaddr*)&signal_addr, sizeof(signal_addr));
/*
					res = recvfrom(sockfd_recv, recv_buf, 512, 0, NULL, NULL);
				//	printf("recvfrom  date !!!!!!!!!!\n ");
					if(res <= 0)
					{
						close(sockfd_recv);
						return -1;
					}
					else
					{	
					}
*/

					struct timeval 	timeVal;
					fd_set  fdSet;
					int ret;

					timeVal.tv_sec = signal_report_timeout;
					timeVal.tv_usec = 0;

					FD_ZERO(&fdSet);
					FD_SET(sockfd_recv, &fdSet);

					ret = select((int) sockfd_recv + 1, &fdSet,NULL, NULL, &timeVal);
					if(ret == 0)                     //��ʱ
					{
						close(sockfd_recv);
						return -1;
					}
					if(ret)
					{
						res = recvfrom(sockfd_recv, recv_buf, 512, 0, NULL, NULL);
						if(res <= 0)
						{
							close(sockfd_recv);
							return -1;
						}
						printf("recvfrom signal_id = %d,jie duan ping bi lei xing %d report!!!!!!!!!!\n ",(recv_buf[7]*256+recv_buf[8]),i);
					}		
				}
				
				close(sockfd_recv);
				return 0;
			}
			/* �ɱ��־��������*/
			else if(buf[10] == 0x15)
			{	
				struct vary_mark_info vary_mark_info_date[25];
				unsigned char set_15_date_lin[24][31];

				for(i = 0; i < 24;i++)
				{
					for(j = 0; j < 31;j++)
					{
						set_15_date_lin[i][j] = set_15_date[i][j];
					}
					
				}

				sprintf(sql,"select  VARY_MARK_ID,CLINK1_ID,CLINK1_TYPE,CLINK1_RED,CLINK1_GREEN ,CLINK2_ID,CLINK2_TYPE,CLINK2_RED,CLINK2_GREEN ,CLINK3_ID,CLINK3_TYPE,CLINK3_RED,CLINK3_GREEN ,CLINK4_ID,CLINK4_TYPE,CLINK4_RED,CLINK4_GREEN from  VARY_MARK_INFO   where signal_id = %d  order by VARY_MARK_ID " ,buf[7] * 256 + buf[8] );
			
				//printf("%s\n",sql);
				st->setSQL(sql);
				rs = st->executeQuery();
				while(rs->next())
				{
					for(i= 0; i < 4; i++)
					{
						vary_mark_info_date[rs->getInt(1)].chink_id[i] = rs->getInt(2 + i * 4);
						vary_mark_info_date[rs->getInt(1)].chink_type[i] = rs->getInt(3 + i * 4);
						vary_mark_info_date[rs->getInt(1)].chink_red[i] = rs->getInt(4 + i * 4);
						vary_mark_info_date[rs->getInt(1)].chink_green[i] = rs->getInt(5 + i * 4);
					}
					
				}

				for( i =0 ; i < 24 ; i++ )
				{
					set_15_date_lin[i][7] = buf[7];
					set_15_date_lin[i][8] = buf[8];
					set_15_date_lin[i][12] = i+1;

					for(j = 0; j < 4 ;j ++)
					{
						set_15_date_lin[i][13 + 4 * j] = vary_mark_info_date[i + 1].chink_id[j];
						set_15_date_lin[i][14 + 4 * j] = vary_mark_info_date[i + 1].chink_type[j];
						set_15_date_lin[i][15 + 4 * j] = vary_mark_info_date[i + 1].chink_red[j];
						set_15_date_lin[i][16 + 4 * j] = vary_mark_info_date[i + 1].chink_green[j];
					}

					set_15_date_lin[i][29] = buf_check_num(set_15_date_lin[i]);

					sendto(sockfd_recv, set_15_date_lin[i], sizeof(set_15_date_lin[i]), 0, (struct sockaddr*)&signal_addr, sizeof(signal_addr));
/*
					res = recvfrom(sockfd_recv, recv_buf, 512, 0, NULL, NULL);
					printf("recvfrom  date !!!!!!!!!!\n ");
					if(res <= 0)
					{
						close(sockfd_recv);
						return -1;
					}
					else
					{	
					}
*/

					struct timeval 	timeVal;
					fd_set  fdSet;
					int ret;

					timeVal.tv_sec = signal_report_timeout;
					timeVal.tv_usec = 0;

					FD_ZERO(&fdSet);
					FD_SET(sockfd_recv, &fdSet);

					ret = select((int) sockfd_recv + 1, &fdSet,NULL, NULL, &timeVal);
					if(ret == 0)                     //��ʱ
					{
						close(sockfd_recv);
						return -1;
					}
					if(ret)
					{
						res = recvfrom(sockfd_recv, recv_buf, 512, 0, NULL, NULL);
						if(res <= 0)
						{
							close(sockfd_recv);
							return -1;
						}
						printf("recvfrom signal_id = %d,ke bian biao zhi %d report!!!!!!!!!!\n ",(recv_buf[7]*256+recv_buf[8]),i);
					}							
				}
				close(sockfd_recv);
				return  0;
			}
			/* �ɱ��־�����������*/
			else if(buf[10] == 0x0e)
			{
				unsigned char set_0e_date_lin[128][97];

				for(i = 0; i < 128;i++)
				{
					for(j = 0; j < 97;j++)
					{
						set_0e_date_lin[i][j] = set_0e_date[i][j];
					}
					
				}
					struct diaodu_info_and_time_0e diaodu_info_and_time_0e_dte[129];	

				sprintf(sql,"select Ddb_id, Ddb_type,Ddb_priority,Week_ddb_type,Date_ddb_type_y,Date_ddb_type_r,Time_table_number_1,(select count(*) from SJBSDSZ_INFO where Time_id=t.Time_table_number_1  and  signal_id = %d and FUNCTION_TYPE = 3)  from  DIAODU_INFO t   where signal_id = %d and FUNCTION_TYPE = 3  order by Ddb_id " ,buf[7] * 256 + buf[8],buf[7] * 256 + buf[8] );
			
				//printf("%s\n",sql);
				st->setSQL(sql);
				rs = st->executeQuery();
				while(rs->next())
				{
					diaodu_info_and_time_0e_dte[rs->getInt(1)].Ddb_type =rs->getInt(2)  ;
					diaodu_info_and_time_0e_dte[rs->getInt(1)].Ddb_priority = rs->getInt(3) ;
					diaodu_info_and_time_0e_dte[rs->getInt(1)].Time_table_number_1 = rs->getInt(7) ;
					diaodu_info_and_time_0e_dte[rs->getInt(1)].time_num = rs->getInt(8) ;

					diaodu_info_and_time_0e_dte[rs->getInt(1)].Week_ddb_type = 0;
					bzero(lin_buf,sizeof(lin_buf));
					sprintf(lin_buf,"%s",rs->getString(4).c_str());
					result = strtok( lin_buf, "," );
					
					while( result != NULL ) 
					{	
						lin_res_num= atoi(result);

						if(lin_res_num == 7)
						{
							diaodu_info_and_time_0e_dte[rs->getInt(1)].Week_ddb_type += 0x01;
						}
						else
						{
							diaodu_info_and_time_0e_dte[rs->getInt(1)].Week_ddb_type += (0x01 << lin_res_num);
						}
						
						result = strtok( NULL, "," );
					}
					
					diaodu_info_and_time_0e_dte[rs->getInt(1)].Date_ddb_type_y[0] = 0;
					diaodu_info_and_time_0e_dte[rs->getInt(1)].Date_ddb_type_y[1] = 0;	
					bzero(lin_buf,sizeof(lin_buf));
					sprintf(lin_buf,"%s",rs->getString(5).c_str());
					result = strtok( lin_buf, "," );
					
					while( result != NULL ) 
					{	
						lin_res_num= atoi(result);

						if(lin_res_num>=1  &&  lin_res_num<=7)
						{
							diaodu_info_and_time_0e_dte[rs->getInt(1)].Date_ddb_type_y[0]  += (0x01 << (lin_res_num % 8 ));

					
						}
						else if(lin_res_num>=8  &&  lin_res_num<=15)
						{
							diaodu_info_and_time_0e_dte[rs->getInt(1)].Date_ddb_type_y[1]  += (0x01 << (lin_res_num % 8 ));
						
						}
						
						result = strtok( NULL, "," );
					}

					diaodu_info_and_time_0e_dte[rs->getInt(1)].Date_ddb_type_r[0] = 0;
					diaodu_info_and_time_0e_dte[rs->getInt(1)].Date_ddb_type_r[1] = 0;
					diaodu_info_and_time_0e_dte[rs->getInt(1)].Date_ddb_type_r[2] = 0;
					diaodu_info_and_time_0e_dte[rs->getInt(1)].Date_ddb_type_r[3] = 0;	
					bzero(lin_buf,sizeof(lin_buf));
					sprintf(lin_buf,"%s",rs->getString(6).c_str());
					result = strtok( lin_buf, "," );
					
					while( result != NULL ) 
					{	
						lin_res_num= atoi(result);
						if(lin_res_num>=1  &&  lin_res_num<=7)
						{
							
							diaodu_info_and_time_0e_dte[rs->getInt(1)].Date_ddb_type_r[0]  += (0x01 << (lin_res_num % 8 ));
							
						}
						else if(lin_res_num>=8  &&  lin_res_num<=15)
						{
							diaodu_info_and_time_0e_dte[rs->getInt(1)].Date_ddb_type_r[1]  += (0x01 << (lin_res_num % 8 ));
							
						}
						else if(lin_res_num>=16  &&  lin_res_num<=23)
						{
							diaodu_info_and_time_0e_dte[rs->getInt(1)].Date_ddb_type_r[2]  += (0x01 << (lin_res_num % 8 ));
						}
						else if(lin_res_num>=24  &&  lin_res_num<=31)
						{
							diaodu_info_and_time_0e_dte[rs->getInt(1)].Date_ddb_type_r[3]  += (0x01 << (lin_res_num % 8 ));
						}
#if 0						
						if(lin_res_num>=1  &&  lin_res_num<=7)
						{
							if(lin_res_num % 8 == 0)
							{
								diaodu_info_and_time_0e_dte[rs->getInt(1)].Date_ddb_type_r[0] += (0x01 << (7));
							}
							else
							{
								diaodu_info_and_time_0e_dte[rs->getInt(1)].Date_ddb_type_r[0]  += (0x01 << (lin_res_num % 8 - 1));
							}
						}
						else if(lin_res_num>=8  &&  lin_res_num<=15)
						{
							if(lin_res_num % 8 == 0)
							{
								diaodu_info_and_time_0e_dte[rs->getInt(1)].Date_ddb_type_r[1] += (0x01 << (7));
							}
							else
							{
								diaodu_info_and_time_0e_dte[rs->getInt(1)].Date_ddb_type_r[1]  += (0x01 << (lin_res_num % 8 - 1));
							}
						}
						else if(lin_res_num>=16  &&  lin_res_num<=23)
						{
							if(lin_res_num % 8 == 0)
							{
								diaodu_info_and_time_0e_dte[rs->getInt(1)].Date_ddb_type_r[2] += (0x01 << (7));
							}
							else
							{
								diaodu_info_and_time_0e_dte[rs->getInt(1)].Date_ddb_type_r[2]  += (0x01 << (lin_res_num % 8 - 1));
							}
						}
						else if(lin_res_num>=24  &&  lin_res_num<=31)
						{
							if(lin_res_num % 8 == 0)
							{
								diaodu_info_and_time_0e_dte[rs->getInt(1)].Date_ddb_type_r[3] += (0x01 << (7));
							}
							else
							{
								diaodu_info_and_time_0e_dte[rs->getInt(1)].Date_ddb_type_r[3]  += (0x01 << (lin_res_num % 8 - 1));
							}
						}
#endif
						
						result = strtok( NULL, "," );

						
					}

								
				}
				sprintf(sql,"select Time_id, Hour,Minute,VARY_MARK_ID,time_set_number  from  SJBSDSZ_INFO    where signal_id = %d and FUNCTION_TYPE = 3 order by Time_id " ,buf[7] * 256 + buf[8]);
			
				//printf("%s\n",sql);
				st->setSQL(sql);
				rs = st->executeQuery();
				
				while( rs->next() ) 
				{
				//	printf("rs->getInt(1) = %d rs->getInt(2) = %d   rs->getInt(3) \n ",rs->getInt(1),rs->getInt(2),rs->getInt(3));
						
					for(i = 0; i <128 ;i++ )
					{
						if(diaodu_info_and_time_0e_dte[i + 1].Time_table_number_1 == rs->getInt(1))
						{
						//	printf("rs->getInt(1) = %d rs->getInt(2) = %d   rs->getInt(3) \n ",rs->getInt(1),rs->getInt(2),rs->getInt(3));
							diaodu_info_and_time_0e_dte[i + 1].Hour[rs->getInt(5) -1] = rs->getInt(2);
							diaodu_info_and_time_0e_dte[i + 1].Minute[rs->getInt(5) -1] = rs->getInt(3);
							diaodu_info_and_time_0e_dte[i + 1].VARY_MARK_ID[rs->getInt(5) -1] = rs->getInt(4);
							break;
						}
					}
				}

				
				for(i = 0;i < 128; i ++)
				{
					set_0e_date_lin[i][7] = buf[7];
					set_0e_date_lin[i][8] = buf[8];
					set_0e_date_lin[i][12] = i+1;

					set_0e_date_lin[i][13] = diaodu_info_and_time_0e_dte[i + 1].Ddb_type;
					set_0e_date_lin[i][14] = diaodu_info_and_time_0e_dte[i + 1].Ddb_priority;
					set_0e_date_lin[i][15] = diaodu_info_and_time_0e_dte[i + 1].Week_ddb_type;
					set_0e_date_lin[i][16] = diaodu_info_and_time_0e_dte[i + 1].Date_ddb_type_y[1];
					set_0e_date_lin[i][17] = diaodu_info_and_time_0e_dte[i + 1].Date_ddb_type_y[0];
					set_0e_date_lin[i][18] = diaodu_info_and_time_0e_dte[i + 1].Date_ddb_type_r[3];
					set_0e_date_lin[i][19] = diaodu_info_and_time_0e_dte[i + 1].Date_ddb_type_r[2];
					set_0e_date_lin[i][20] = diaodu_info_and_time_0e_dte[i + 1].Date_ddb_type_r[1];
					set_0e_date_lin[i][21] = diaodu_info_and_time_0e_dte[i + 1].Date_ddb_type_r[0];

					set_0e_date_lin[i][22] = diaodu_info_and_time_0e_dte[i + 1].time_num;
#if 1
					for(j = 0; j < 24 ; j++)
					{
						set_0e_date_lin[i][23 + j * 3 ] = diaodu_info_and_time_0e_dte[i + 1].Hour[j];
						set_0e_date_lin[i][24 + j * 3 ] = diaodu_info_and_time_0e_dte[i + 1].Minute[j];
						set_0e_date_lin[i][25 + j * 3 ] = diaodu_info_and_time_0e_dte[i + 1].VARY_MARK_ID[j];
						
					}
#endif
					set_0e_date_lin[i][95] = buf_check_num(set_0e_date_lin[i]);
#if 0
					if(i == 0)
					{
						int ww_ww;
						printf("OE:");
						for(ww_ww = 0; ww_ww < 97;ww_ww++)
						{
							printf("%02x ",set_0e_date_lin[0][ww_ww]);
						}
						printf("\n");
					}
#endif				
					
					
					
				//	sendto(sockfd_recv, set_0e_date_lin[i], sizeof(set_0e_date_lin[i]), 0, (struct sockaddr*)&signal_addr, sizeof(signal_addr));
				
				sendto(sockfd_recv, set_0e_date_lin[i],97, 0, (struct sockaddr*)&signal_addr, sizeof(signal_addr));


				//	sendto(sockfd_recv, set_01_date_lin, sizeof(set_01_date_lin), 0, (struct sockaddr*)&signal_addr, sizeof(signal_addr));
/*				
					res = recvfrom(sockfd_recv, recv_buf, 512, 0, NULL, NULL);
				//	printf("recvfrom  date !!!!!!!!!!\n ");
					if(res <= 0)
					{
						close(sockfd_recv);
						return -1;
					}
					else
					{	
					}
*/
					struct timeval 	timeVal;
					fd_set  fdSet;
					int ret;

					timeVal.tv_sec = signal_report_timeout;
					timeVal.tv_usec = 0;

					FD_ZERO(&fdSet);
					FD_SET(sockfd_recv, &fdSet);

					ret = select((int) sockfd_recv + 1, &fdSet,NULL, NULL, &timeVal);
					if(ret == 0)                     //��ʱ
					{
						close(sockfd_recv);
						return -1;
					}
					if(ret)
					{
						res = recvfrom(sockfd_recv, recv_buf, 512, 0, NULL, NULL);
						if(res <= 0)
						{
							close(sockfd_recv);
							return -1;
						}
						printf("recvfrom signal_id = %d,�ɱ��־�������  %d report!!!!!!!!!!\n ",(recv_buf[7]*256+recv_buf[8]),i);
					}		
				}
				
				close(sockfd_recv);
				return 0;
			}
			/* ���������*/
			else if(buf[10] == 0x0f)
			{
				unsigned char set_0f_date_lin[104][23];

				for(i = 0; i < 104;i++)
				{
					for(j = 0; j < 23;j++)
					{
						set_0f_date_lin[i][j] = set_0f_date[i][j];
					}
					
				}

				struct detector_info detector_info_date[105];	

				sprintf(sql,"select Detector_id,Detector_type,Flow_week,Occ_week,Warning_week,DIRECTION  from  DETECTOR_INFO   where signal_id = %d  order by Detector_id " ,buf[7] * 256 + buf[8] );
			
				//printf("%s\n",sql);
				st->setSQL(sql);
				rs = st->executeQuery();
				while(rs->next())
				{
					detector_info_date[rs->getInt(1)].Detector_type = rs->getInt(2);
					detector_info_date[rs->getInt(1)].Flow_week[0]= (rs->getInt(3) ) >> 8;
					detector_info_date[rs->getInt(1)].Flow_week[1]= (rs->getInt(3) ) & 0xff;
					detector_info_date[rs->getInt(1)].Occ_week[0]= (rs->getInt(4) ) >> 8;
					detector_info_date[rs->getInt(1)].Occ_week[1]= (rs->getInt(4) ) & 0xff;
					detector_info_date[rs->getInt(1)].Warning_week[0]= (rs->getInt(5) * 10) >> 8;
					detector_info_date[rs->getInt(1)].Warning_week[1]= (rs->getInt(5) * 10) & 0xff;
					detector_info_date[rs->getInt(1)].direction = rs->getInt(6);
				}

				for(i = 0;i < 104 ; i++)
				{
					set_0f_date_lin[i][7] = buf[7];
					set_0f_date_lin[i][8] = buf[8];
					set_0f_date_lin[i][12] = i+1;
					
					set_0f_date_lin[i][13] = detector_info_date[i + 1].Detector_type;

					set_0f_date_lin[i][14] = detector_info_date[i + 1].Flow_week[0];
					set_0f_date_lin[i][15] = detector_info_date[i + 1].Flow_week[1];
					set_0f_date_lin[i][16] = detector_info_date[i + 1].Occ_week[0];
					set_0f_date_lin[i][17] = detector_info_date[i + 1].Occ_week[1];
					set_0f_date_lin[i][18] = detector_info_date[i + 1].Warning_week[0];
					set_0f_date_lin[i][19] = detector_info_date[i + 1].Warning_week[1];

					set_0f_date_lin[i][120] = detector_info_date[i + 1].direction;

					set_0f_date_lin[i][21] = buf_check_num(set_0f_date_lin[i]);

					sendto(sockfd_recv, set_0f_date_lin[i], sizeof(set_0f_date_lin[i]), 0, (struct sockaddr*)&signal_addr, sizeof(signal_addr));
/*
					res = recvfrom(sockfd_recv, recv_buf, 512, 0, NULL, NULL);
				//	printf("recvfrom  date  i = %d\n ",i);
					if(res <= 0)
					{
						close(sockfd_recv);
						return -1;
					}
					else
					{	
					}
*/
					struct timeval 	timeVal;
					fd_set  fdSet;
					int ret;

					timeVal.tv_sec = signal_report_timeout;
					timeVal.tv_usec = 0;

					FD_ZERO(&fdSet);
					FD_SET(sockfd_recv, &fdSet);

					ret = select((int) sockfd_recv + 1, &fdSet,NULL, NULL, &timeVal);
					if(ret == 0)                     //��ʱ
					{
						close(sockfd_recv);
						return -1;
					}
					if(ret)
					{
						res = recvfrom(sockfd_recv, recv_buf, 512, 0, NULL, NULL);
						if(res <= 0)
						{
							close(sockfd_recv);
							return -1;
						}
						printf("recvfrom signal_id = %d,jian ce qi %d report!!!!!!!!!!\n ",(recv_buf[7]*256+recv_buf[8]),i);
					}		
				}
				
				close(sockfd_recv);
				return 0;
					
			}

			//itc100,���������
			else if (buf[10] == 0x26)
			{
				unsigned char set_26_date_lin[238];

				for(i = 0; i < 238;i++)
				{

					set_26_date_lin[i] =0;
					
				}

				set_26_date_lin[0] = 0x7e;set_26_date_lin[2] = 0xec;
				set_26_date_lin[4] = 0x01;
				set_26_date_lin[7] = buf[7];
				set_26_date_lin[8] = buf[8];
				set_26_date_lin[9] = 0x20;set_26_date_lin[10] = 0x26;

				sprintf(sql,"select Detector_id,Detector_type,Flow_week,Occ_week,Warning_week  from  ITC_CFG_DETECTOR   where signal_id = %d  order by Detector_id " ,buf[7] * 256 + buf[8] );
			
				printf("%s\n",sql);
				st->setSQL(sql);
				rs = st->executeQuery();
				while(rs->next())
				{
					set_26_date_lin[12+ (rs->getInt(1)-1)*7] = rs->getInt(2);
					set_26_date_lin[13+(rs->getInt(1)-1)*7] =(rs->getInt(3) ) >> 8;
					set_26_date_lin[14+(rs->getInt(1)-1)*7] =(rs->getInt(3) ) & 0xff;
					set_26_date_lin[15+(rs->getInt(1)-1)*7] = (rs->getInt(4) ) >> 8;
					set_26_date_lin[16+(rs->getInt(1)-1)*7] =(rs->getInt(4) ) & 0xff;
					set_26_date_lin[17+(rs->getInt(1)-1)*7] =  (rs->getInt(5) ) >> 8;
					set_26_date_lin[18+(rs->getInt(1)-1)*7] = (rs->getInt(5) ) & 0xff;

				}

				set_26_date_lin[236] = buf_check_num(set_26_date_lin);
				set_26_date_lin[237] = 0x7d;

				sendto(sockfd_recv, set_26_date_lin, sizeof(set_26_date_lin), 0, (struct sockaddr*)&signal_addr, sizeof(signal_addr));
/*
				res = recvfrom(sockfd_recv, recv_buf, 512, 0, NULL, NULL);
				//	printf("recvfrom  date  i = %d\n ",i);
				if(res <= 0)
				{
					close(sockfd_recv);
					return -1;
				}
				else
				{
					close(sockfd_recv);
					return 0;
				}
	*/
				
					struct timeval 	timeVal;
					fd_set  fdSet;
					int ret;

					timeVal.tv_sec = signal_report_timeout;
					timeVal.tv_usec = 0;

					FD_ZERO(&fdSet);
					FD_SET(sockfd_recv, &fdSet);

					ret = select((int) sockfd_recv + 1, &fdSet,NULL, NULL, &timeVal);
					if(ret == 0)                     //��ʱ
					{
						printf("signal no reply::jiang ce qi\n");
						close(sockfd_recv);
						return -1;
					}
					if(ret)                //������
					{
						res = recvfrom(sockfd_recv, recv_buf, 512, 0, NULL, NULL);
						if(res == 0)
						{
							printf("select timeout::jiang ce qi\n");
							close(sockfd_recv);
							return -1;
						}
						else
						{
							close(sockfd_recv);
							printf("recvfrom signal_id = %d,jian ce qi  report!!!!!!!!!!\n ",(recv_buf[7]*256+recv_buf[8]));
							return 0;
						}
					}						
			}
			
			/*��λ��������*/
			else if(buf[10] == 0x10)
			{
				unsigned char set_10_date_lin[270];

				for(i = 0; i < 270;i++)
				{
					
					set_10_date_lin[i] = set_10_date[i];
					
				}

				set_10_date_lin[7] = buf[7];
				set_10_date_lin[8] = buf[8];
				sprintf(sql,"select CLINK_ID_ROW, CLINK_ID_COLUMN,GUODU_VALUE  from TRANSITION_CLINK    where signal_id = %d  order by CLINK_ID_ROW " ,buf[7] * 256 + buf[8] );
			
				//printf("%s\n",sql);
				st->setSQL(sql);
				rs = st->executeQuery();

			
				while(rs->next())
				{
					
					set_10_date_lin[ 12 +  (rs->getInt(1) - 1) * 16 + (rs->getInt(2) - 1) ] = rs->getInt(3);
				}
				set_10_date_lin[268] = buf_check_num(set_10_date_lin);

				sendto(sockfd_recv, set_10_date_lin, sizeof(set_10_date_lin), 0, (struct sockaddr*)&signal_addr, sizeof(signal_addr));
#if 0
					int www;
					printf("send buf:");
					for(www = 0; www <270;www++)
						printf( "%02x", set_10_date_lin[www]);
					printf("\n");

#endif
/*
				res = recvfrom(sockfd_recv, recv_buf, 512, 0, NULL, NULL);

				
				
				if(res <= 0)
				{
					close(sockfd_recv);
					return -1;
				}
				else
				{	
					close(sockfd_recv);
					return 0;
				}
		*/
					struct timeval 	timeVal;
					fd_set  fdSet;
					int ret;

					timeVal.tv_sec = signal_report_timeout;
					timeVal.tv_usec = 0;

					FD_ZERO(&fdSet);
					FD_SET(sockfd_recv, &fdSet);

					ret = select((int) sockfd_recv + 1, &fdSet,NULL, NULL, &timeVal);
					if(ret == 0)                     //��ʱ
					{
						printf("signal no reply::jiang ce qi\n");
						close(sockfd_recv);
						return -1;
					}
					if(ret)                //������
					{
						res = recvfrom(sockfd_recv, recv_buf, 512, 0, NULL, NULL);
						if(res == 0)
						{
							printf("select timeout::xiang wei guo du she zhi\n");
							close(sockfd_recv);
							return -1;
						}
						else
						{
							
							close(sockfd_recv);
							return 0;	
						}
					}				
			}

			//ITC100,��������
			else if(buf[10] == 0x2f)
			{
				unsigned char set_2f_date_lin[8][192];

				for(i = 0 ; i < 8;i++)
				{
					int count_stage = 0;
					bzero(set_2f_date_lin[i],192);

					set_2f_date_lin[i][0] = 0x7e;set_2f_date_lin[i][2] = 0xbe;
					set_2f_date_lin[i][4] = 0x01;
					set_2f_date_lin[i][7] = buf[7];
					set_2f_date_lin[i][8] = buf[8];
					set_2f_date_lin[i][9] = 0x20;set_2f_date_lin[i][10] = 0x2f;
					set_2f_date_lin[i][11] = i;set_2f_date_lin[i][12] = i+1;
					
					sprintf(sql,"select stage_id, stage_time,green_time,green_flash_time,green_flash_qtime,red_time ,(select t.stage_phase from itc_cfg_stage t where t.signal_id=icd.signal_id and t.stage_id=icd.stage_id) as stagePhase  from ITC_CFG_DOWNGRADE_CHAIN   icd where signal_id = %d and list_unit_id = %d  order by stage_order " ,buf[7] * 256 + buf[8] ,i+1);
			
					printf("%s\n",sql);
					st->setSQL(sql);
					rs = st->executeQuery();

					while(rs->next() != 0)
					{
						if(rs->getInt(1) != 0)
						{
							//unsigned int phase_temp = 0x80000000 >> (rs->getInt(1) - 1);
							//set_2f_date_lin[i][14 + 4*count_stage] = phase_temp >> 24;
							//set_2f_date_lin[i][15 + 4*count_stage] = phase_temp >> 16;
							//set_2f_date_lin[i][16 + 4*count_stage] = phase_temp >> 8;
							//set_2f_date_lin[i][17 + 4*count_stage] = phase_temp & 0xff;

							unsigned int phase_temp = 0;
							bzero(lin_buf,sizeof(lin_buf));
							sprintf(lin_buf,"%s",rs->getString(7).c_str());
							result = strtok( lin_buf, "," );
						
							while( result != NULL ) 
							{
								lin_res_num= atoi(result);

								phase_temp = phase_temp | (0x80000000>>(lin_res_num-1));

								result = strtok( NULL, "," );
							}

							set_2f_date_lin[i][14 + 4*count_stage] = phase_temp >> 24;
							set_2f_date_lin[i][15 + 4*count_stage] = phase_temp >> 16;
							set_2f_date_lin[i][16 + 4*count_stage] = phase_temp >> 8;
							set_2f_date_lin[i][17 + 4*count_stage] = phase_temp & 0xff;
					
							set_2f_date_lin[i][78+ 2*count_stage] = rs->getInt(2) /256;
							set_2f_date_lin[i][79+ 2*count_stage] = rs->getInt(2) %256;

							set_2f_date_lin[i][110+ 5* count_stage] = rs->getInt(3) /256 ;
							set_2f_date_lin[i][111+ 5* count_stage] = rs->getInt(3) %256 ;
							set_2f_date_lin[i][112+ 5* count_stage] = rs->getInt(4);
							set_2f_date_lin[i][113+ 5* count_stage] = rs->getInt(5);
							set_2f_date_lin[i][114+ 5* count_stage] = rs->getInt(6);
							
							count_stage++;
						}
					}

					set_2f_date_lin[i][13] =count_stage ;
					set_2f_date_lin[i][190] = buf_check_num(set_2f_date_lin[i]);
					set_2f_date_lin[i][191] = 0x7d;
					
					sendto(sockfd_recv, set_2f_date_lin[i], sizeof(set_2f_date_lin[i]), 0, (struct sockaddr*)&signal_addr, sizeof(signal_addr));
/*
					res = recvfrom(sockfd_recv, recv_buf, 512, 0, NULL, NULL);
					if(res <= 0)
					{
						close(sockfd_recv);
						return -1;
					}
*/
					struct timeval 	timeVal;
					fd_set  fdSet;
					int ret;

					timeVal.tv_sec = signal_report_timeout;
					timeVal.tv_usec = 0;

					FD_ZERO(&fdSet);
					FD_SET(sockfd_recv, &fdSet);

					ret = select((int) sockfd_recv + 1, &fdSet,NULL, NULL, &timeVal);
								if(ret == 0)                     //��ʱ
								{
									printf("signal no reply::jiang ji pei zhi\n");
									close(sockfd_recv);
									return -1;
								}
								if(ret)                //������
								{
									res = recvfrom(sockfd_recv, recv_buf, 512, 0, NULL, NULL);
									if(res == 0)
									{
										printf("select timeout::jiang ji pei zhi \n");
										close(sockfd_recv);
										return -1;
									}
									else
									{
										printf("\njiangji pei zhi:: %d\n\n",i);
										
									}
					}		
				}
				close(sockfd_recv);
				return 0;	
				
			}

			//itc100,�ֶ�����
			else if(buf[10] == 0x30)
			{
				unsigned char set_30_date_lin[20];

				for(i = 0; i < 20;i++)
				{

					set_30_date_lin[i] =0;
					
				}

				set_30_date_lin[0] = 0x7e;set_30_date_lin[2] = 0x12;
				set_30_date_lin[4] = 0x01;
				set_30_date_lin[7] = buf[7];
				set_30_date_lin[8] = buf[8];
				set_30_date_lin[9] = 0x20;set_30_date_lin[10] = 0x30;

				sprintf(sql,"select   password as pwd_,signal_id  from  itc_cfg_password   where signal_id = %d  " ,buf[7] * 256 + buf[8] );
			
				printf("%s\n",sql);
				st->setSQL(sql);
				rs = st->executeQuery();

				if(rs->next() != 0)
					{
				bzero(lin_buf,sizeof(lin_buf));
				sprintf(lin_buf,"%s",rs->getString(1).c_str());
				result = strtok( lin_buf, "," );

				i = 0;

				while( result != NULL ) 
					{	
						set_30_date_lin[12+i]= atoi(result);
						i++;
						result = strtok( NULL, "," );
					}
					}

				set_30_date_lin[18] = buf_check_num(set_30_date_lin);
				set_30_date_lin[19] = 0x7d;
				
				sendto(sockfd_recv, set_30_date_lin, sizeof(set_30_date_lin), 0, (struct sockaddr*)&signal_addr, sizeof(signal_addr));
/*
				res = recvfrom(sockfd_recv, recv_buf, 512, 0, NULL, NULL);				
				
				if(res <= 0)
				{
					close(sockfd_recv);
					return -1;
				}
				else
				{	
					close(sockfd_recv);
					return 0;
				}
*/
					struct timeval 	timeVal;
					fd_set  fdSet;
					int ret;

					timeVal.tv_sec = signal_report_timeout;
					timeVal.tv_usec = 0;

					FD_ZERO(&fdSet);
					FD_SET(sockfd_recv, &fdSet);

					ret = select((int) sockfd_recv + 1, &fdSet,NULL, NULL, &timeVal);
					if(ret == 0)                     //��ʱ
					{
						printf("signal no reply::shou dong mi ma\n");
						close(sockfd_recv);
						return -1;
					}
					if(ret)                //������
					{
						res = recvfrom(sockfd_recv, recv_buf, 512, 0, NULL, NULL);
						if(res == 0)
						{
							printf("signal reply error::shou dong mi ma \n");
							close(sockfd_recv);
							return -1;
						}
						else
						{
							close(sockfd_recv);
							printf("recvfrom signal_id = %d,shoud dong mi ma!!!!!!!!!!\n ",(recv_buf[7]*256+recv_buf[8]));
							return 0;
						}
					}						
			}
			else if(buf[10] == 0x31)
			{
				unsigned char set_31_date_lin[15];

				bzero(set_31_date_lin,sizeof(set_31_date_lin));

				set_31_date_lin[0] = 0x7e;

				set_31_date_lin[1] = 0x00;
				set_31_date_lin[2]=  0x0d;

				set_31_date_lin[3] = 0x00;
				set_31_date_lin[4]=  0x01;

				set_31_date_lin[5] = 0x00;
				set_31_date_lin[6] = 0x00;	

				set_31_date_lin[7] = buf[7];
				set_31_date_lin[8] = buf[8];

				set_31_date_lin[9] = 0x20;
				set_31_date_lin[10] = 0x31;
				set_31_date_lin[11] = 0x00;
				set_31_date_lin[12] = 0x00;
				
				set_31_date_lin[13] = buf_check_num(set_31_date_lin);
				set_31_date_lin[14] = 0x7D;
				sleep(5);
				sendto(sockfd_recv, set_31_date_lin, sizeof(set_31_date_lin), 0, (struct sockaddr*)&signal_addr, sizeof(signal_addr));
				printf("if set is enable\n\n");
			struct timeval 	timeVal;
					fd_set  fdSet;
					int ret;

					timeVal.tv_sec = signal_report_timeout;
					timeVal.tv_usec = 0;

					FD_ZERO(&fdSet);
					FD_SET(sockfd_recv, &fdSet);

					ret = select((int) sockfd_recv + 1, &fdSet,NULL, NULL, &timeVal);
					if(ret == 0)                     //��ʱ
					{
						printf("deng dai jie shou  chao shi \n");
						close(sockfd_recv);
						return -1;
					}
					if(ret)
					{
						res = recvfrom(sockfd_recv, recv_buf, 512, 0, NULL, NULL);
						if(res <= 0)
						{
							printf("shu ju cuo wu \n\n");	
							close(sockfd_recv);
							return -1;
						}
						else
						{
							printf("recvfrom signal_id = %d,itc100 jiao yan shu ju report!!!!!!!!!!\n ",(recv_buf[7]*256+recv_buf[8]));
							if(recv_buf[12] == 1)
							{
								printf("pei zhi xia zai cheng gong\n\n");
								close(sockfd_recv);
								return 0;
							}
							else
							{
								printf("pei zhi xia zai shi bai\n\n");
								close(sockfd_recv);
								return -1;	
							}			
						}
					}				
			}
			
			
		}
		/*��ѯ*/
		else if (buf[9] == 0x10 )
		{
			unsigned char select_recv_buf[512];					
			unsigned char select_buf[16];
			select_buf[0] = 0x7e;
			select_buf[1] = 0x00;
			select_buf[2] = 0x0e;
			select_buf[3] = 0x00;
			select_buf[4] = 0x01;
			select_buf[5] = 0x00;
			select_buf[6] = 0x01;
			select_buf[9] = 0x10;
			select_buf[11] = 0x00;
			select_buf[12] = 0x00;
			select_buf[13] = 0x00;
			select_buf[15] = 0x7d;

			select_buf[7] = buf[7];
			select_buf[8] = buf[8];
			select_buf[10] = buf[10];
			select_buf[14] = buf_check_num(select_buf);
		
		
			/*������Ϣ��ѯ*/
			if(buf[10] == 0x01)
			{
				if(strcmp(signal_info_data[sugnal_num].device_type,"TC100") == 0)
					{
				sendto(sockfd_recv, select_buf, sizeof(select_buf), 0, (struct sockaddr*)&signal_addr, sizeof(signal_addr));
/*
				res = recvfrom(sockfd_recv, select_recv_buf, 512, 0, NULL, NULL);
				printf("recvfrom  date !!!!!!!!!!\n ");
				if(res <= 0)
				{
					close(sockfd_recv);
					return -1;
				}					
*/
				struct timeval 	timeVal;
					fd_set  fdSet;
					int ret;

					timeVal.tv_sec = signal_report_timeout;
					timeVal.tv_usec = 0;

					FD_ZERO(&fdSet);
					FD_SET(sockfd_recv, &fdSet);

					ret = select((int) sockfd_recv + 1, &fdSet,NULL, NULL, &timeVal);
					if(ret == 0)                     //��ʱ
					{
						printf("signal no reply::jiang ce qi\n");
						close(sockfd_recv);
						return -1;
					}
					if(ret)                //������
					{
						res = recvfrom(sockfd_recv, recv_buf, 512, 0, NULL, NULL);
						if(res == 0)
						{
							printf("select timeout::jin ji can shu\n");
							close(sockfd_recv);
							return -1;
						}
					}		
				/*�ĸ����Ե��»����ĳ�ͻ��־λ
					1 Ϊʹ�ܣ�0Ϊ�ر�ʹ��*/
				int config_fault;
				int green_fault;
				int yellow_fault;
				int lamp_fault;

				if((select_recv_buf[35] & 0x08) == 0)
				{
					config_fault = 0;
				}
				else
				{
					config_fault = 1;
				}

				if((select_recv_buf[35] & 0x04) == 0)
				{
					green_fault = 0;
				}
				else
				{
					green_fault = 1;
				}

				if((select_recv_buf[35] & 0x02) == 0)
				{
					yellow_fault = 0;
				}
				else
				{
					yellow_fault = 1;
				}

				if((select_recv_buf[35] & 0x01) == 0)
				{
					lamp_fault = 0;
				}
				else
				{
					lamp_fault = 1;
				}
				
				try
				{	
					sprintf(sql,"update  SIGNAL_CONFIG_INFO  set signal_ip = '%d.%d.%d.%d',signal_netmask='%d.%d.%d.%d',signal_gateway = '%d.%d.%d.%d',signal_port = %d,swj_ip= '%d.%d.%d.%d',swj_port = %d,CONFIG_FAULT_YELLOWFLASH = %d,GREEN_CONFLICT_YELLOWFLASH = %d,REDGREEN_CONFLICT_YELLOWFLASH = %d,LAMP_FAILURE_YELLOWFLASH = %d  where signal_id = %d  " ,
						  select_recv_buf[13],select_recv_buf[14],select_recv_buf[15],select_recv_buf[16],
						select_recv_buf[17],select_recv_buf[18],select_recv_buf[19],select_recv_buf[20],   select_recv_buf[21],select_recv_buf[22],select_recv_buf[23],select_recv_buf[24],
						select_recv_buf[25] * 256 + select_recv_buf[26],   select_recv_buf[27],select_recv_buf[28],select_recv_buf[29],select_recv_buf[30],
						select_recv_buf[31] * 256 + select_recv_buf[32],config_fault,green_fault,yellow_fault,lamp_fault,
						select_recv_buf[33] * 256 + select_recv_buf[34]
						);

					printf("%s\n",sql);
					st->setSQL(sql);
					st->executeUpdate();
				}
				catch (SQLException& ex)
				{
					close(sockfd_recv);
					return -1;
				}
				}

				else if(strcmp(signal_info_data[sugnal_num].device_type,"ITC100") == 0)
				{
					char sql_temp[500];
					strcpy(sql,"update  SIGNAL_CONFIG_INFO  set  " );	
					for(i=0;i<6;i++) //0x05 ;0x06;0x07;0x09;0x0a
					{
						bzero(sql_temp,500);
						if(i == 3 || i == 1)
							continue;
						select_buf[10] = 0x05 + i;//��ѯ�źŻ�����·��
						select_buf[14] = buf_check_num(select_buf);

						sendto(sockfd_recv, select_buf, sizeof(select_buf), 0, (struct sockaddr*)&signal_addr, sizeof(signal_addr));
/*
						res = recvfrom(sockfd_recv, select_recv_buf, 512, 0, NULL, NULL);
						printf("recvfrom  date !!!!!!!!!!\n ");
						if(res <= 0)
						{
							close(sockfd_recv);
							return -1;
						}
*/					
						struct timeval 	timeVal;
						fd_set  fdSet;
						int ret;

						timeVal.tv_sec = signal_report_timeout;
						timeVal.tv_usec = 0;

						FD_ZERO(&fdSet);
						FD_SET(sockfd_recv, &fdSet);

						ret = select((int) sockfd_recv + 1, &fdSet,NULL, NULL, &timeVal);
						if(ret == 0)                     //��ʱ
						{
							printf("signal no reply::ji chu xin xi \n");
							close(sockfd_recv);
							return -1;
						}
						if(ret)                //������
						{
							res = recvfrom(sockfd_recv, select_recv_buf, 512, 0, NULL, NULL);
							if(res == 0)
							{
								printf("select timeout::ji chu xin xi\n");
								close(sockfd_recv);
								return -1;
							}
						}		
						
						switch(i)
						{
							case 0: // 0x05,�䳤������
								strcat(sql," unit_name_ascii = '");
								for(j = 0 ; j < select_recv_buf[1]*256+select_recv_buf[2] -13;j++)//
								{
									if(j != select_recv_buf[1]*256+select_recv_buf[2] -14)
										sprintf(sql_temp,"%02x,",select_recv_buf[13 + j]);
									else
										sprintf(sql_temp,"%02x' ",select_recv_buf[13 + j]);
									strcat(sql,sql_temp);
								}
								break;
							case 1://0x06
								if(select_recv_buf[10] == 0x06)
								{
									sprintf(sql_temp,",signal_ip = '%d.%d.%d.%d',signal_netmask='%d.%d.%d.%d',signal_gateway='%d.%d.%d.%d'  ",
										select_recv_buf[13],select_recv_buf[14],select_recv_buf[15],select_recv_buf[16],
										select_recv_buf[17],select_recv_buf[18],select_recv_buf[19],select_recv_buf[20],
										select_recv_buf[21],select_recv_buf[22],select_recv_buf[23],select_recv_buf[24]);

									strcat(sql,sql_temp);
								}
								break;
							case 2://0x07
								if(select_recv_buf[10] == 0x07)
								{
									sprintf(sql_temp," ,swj_ip = '%d.%d.%d.%d',swj_port=%d,swj_type=%d  ",
										select_recv_buf[13],select_recv_buf[14],select_recv_buf[15],select_recv_buf[16],
										select_recv_buf[17]*256+select_recv_buf[18],select_recv_buf[19]);

									strcat(sql,sql_temp);
								}
								break;
							case 4://0x09
								if(select_recv_buf[10] == 0x09)
								{
									sprintf(sql_temp,",signal_id = %d ",select_recv_buf[13]*256+select_recv_buf[14]);

									strcat(sql,sql_temp);
								}
								break;
							case 5://0x0a
								if(select_recv_buf[10] == 0x0a)
								{
									sprintf(sql_temp,",cotrol_conut = %d ",select_recv_buf[13]);

									strcat(sql,sql_temp);
								}
								break;
						}
						}
						bzero(sql_temp,500);
						sprintf(sql_temp," where signal_id = %d",buf[7]*256+buf[8]);
						strcat(sql,sql_temp);

						try{
							st->setSQL(sql);
							st->executeUpdate();
						}
						catch (SQLException& ex)
						{
							close(sockfd_recv);
							return -1;
						}
						
					//}
					
				}
				close(sockfd_recv);
				return 0;
			
			}
			/*��������ѯ*/
			else if(buf[10] == 0x02)
			{

				sendto(sockfd_recv, select_buf, sizeof(select_buf), 0, (struct sockaddr*)&signal_addr, sizeof(signal_addr));
/*
				res = recvfrom(sockfd_recv, select_recv_buf, 512, 0, NULL, NULL);
				printf("recvfrom  date !!!!!!!!!!\n ");
				if(res <= 0)
				{
					close(sockfd_recv);
					return -1;
				}
		*/
				struct timeval 	timeVal;
					fd_set  fdSet;
					int ret;

					timeVal.tv_sec = signal_report_timeout;
					timeVal.tv_usec = 0;

					FD_ZERO(&fdSet);
					FD_SET(sockfd_recv, &fdSet);

					ret = select((int) sockfd_recv + 1, &fdSet,NULL, NULL, &timeVal);
					if(ret == 0)                     //��ʱ
					{
						printf("cha xun ::select timeout::san can shu\n");
						close(sockfd_recv);
						return -1;
					}
					if(ret)                //������
					{
						res = recvfrom(sockfd_recv, select_recv_buf, 512, 0, NULL, NULL);
						if(res == 0)
						{
							
							close(sockfd_recv);
							return -1;
						}
					}			
				sprintf(sql,"delete  from  SANPINLV  where signal_id = %d" ,buf[7] * 256 + buf[8]);
				printf("%s\n",sql);
				st->setSQL(sql);
				st->executeUpdate();


				try
				{
					sprintf(sql,"insert into SANPINLV( signal_id,san_zq,san_zkb,kuaisan_zq,kuaisan_zkb,PULSE_WIDTH_TIME)  values(%d,%d,%d,%d,%d,%d)" ,
			//			buf[7] * 256 + buf[8],(select_recv_buf[13] * 256 + select_recv_buf[14]) /10,(select_recv_buf[15] * 256 + select_recv_buf[16]) /10,(select_recv_buf[17] * 256 + select_recv_buf[18])/ 10,(select_recv_buf[19] * 256 + select_recv_buf[20]) /10,(select_recv_buf[21] * 256 + select_recv_buf[22]) /10);
					buf[7] * 256 + buf[8],(select_recv_buf[13] * 256 + select_recv_buf[14]) ,(select_recv_buf[15] * 256 + select_recv_buf[16]) ,(select_recv_buf[17] * 256 + select_recv_buf[18]),(select_recv_buf[19] * 256 + select_recv_buf[20]) ,(select_recv_buf[21] * 256 + select_recv_buf[22]));
					printf("%s\n",sql);
					st->setSQL(sql);
					st->executeUpdate();
				}
				catch (SQLException& ex)
				{
					close(sockfd_recv);
					return -1;
				}
				
				close(sockfd_recv);
				return 0;
			}
			/*��λ�������Ͳ�ѯ*/
			else if(buf[10] == 0x11)
			{
				unsigned char set_11_date_select[9][22+10];

				
				struct timeval 	timeVal;
				fd_set  fdSet;
				int ret;
				sendto(sockfd_recv, select_buf, sizeof(select_buf), 0, (struct sockaddr*)&signal_addr, sizeof(signal_addr));
				i = 0;
				while(1)
				{
					bzero(select_recv_buf,sizeof(select_recv_buf));
					//res = recvfrom(sockfd_recv, select_recv_buf, 512, 0, NULL, NULL);

#if 0				
					printf("recvfrom  date !!!!!!!!!!: ");
					int www;
					for(www = 0;www < res ;www++)
					{
						printf("%02x",select_recv_buf[www]);
					}
					printf("\n");
#endif				
					//if(res <= 0)
					//{
						//break;
					//}
					

					timeVal.tv_sec = signal_report_timeout;
					timeVal.tv_usec = 0;

					FD_ZERO(&fdSet);
					FD_SET(sockfd_recv, &fdSet);

					ret = select((int) sockfd_recv + 1, &fdSet,NULL, NULL, &timeVal);
					if(ret == 0)                     //��ʱ
					{
						printf("cha xun ::select timeout::xiang wei kai ji lei xing \n");
						close(sockfd_recv);
						return -1;
					}
					if(ret)                //������
					{
						res = recvfrom(sockfd_recv, select_recv_buf, 512, 0, NULL, NULL);
						if(res == 0)
						{
							
							close(sockfd_recv);
							return -1;
						}		
						else
						{
							if(select_recv_buf[12 + 1] == 0)
							{
							
							}
							else
							{
								if(select_recv_buf[12 + 1] > 8)
								{
									continue;
								}
							
								for(j = 0; j < 22; j++)
								{
									set_11_date_select[select_recv_buf[12+ 1]][j] = select_recv_buf[j];
								}
							//strncpy(set_11_date_select[i][j],select_recv_buf,21);
							}
						
						}
					}
					i++;

					if(i == 8)
					{
						break;
					}
				}
				
				if(i  !=  8)
				{
					close(sockfd_recv);
					return -1;
				}
				
				sprintf(sql,"delete  from  KAIJIDENGSE  where signal_id = %d  " ,buf[7] * 256 + buf[8] );
				printf("%s\n",sql);
				st->setSQL(sql);
				st->executeUpdate();
				
				for(i = 1 ; i <= 8; i++)
				{
					if(set_11_date_select[i][12+ 1] != 0 )
					{
						try
						{
							sprintf(sql,"insert into KAIJIDENGSE (signal_id,clink_id,kaijidengse1,kaijishijian1,kaijidengse2,kaijishijian2, kaijidengse3,kaijishijian3)   values( %d,%d,%d,%d,%d,%d,%d,%d)" ,
								buf[7] * 256 + buf[8] ,set_11_date_select[i][12 + 1] ,set_11_date_select[i][13 + 1] ,set_11_date_select[i][14  + 1]/ 10,set_11_date_select[i][15  + 1],set_11_date_select[i][16 + 1]/ 10,set_11_date_select[i][17 + 1],set_11_date_select[i][18 + 1]/ 10);
							
							printf("%s\n",sql);
							st->setSQL(sql);
							st->executeUpdate();
						}
						catch(char* value)
						{
							close(sockfd_recv);
							return -1;
						}
					}
					
				}
					
				close(sockfd_recv);
				return 0;
			
			}
			/*����̲�ѯ*/
			else if(buf[10] == 0x12)
			{
				unsigned char set_12_date_lin[9][22+10];
				struct timeval 	timeVal;
				fd_set  fdSet;
				int ret;
				sendto(sockfd_recv, select_buf, sizeof(select_buf), 0, (struct sockaddr*)&signal_addr, sizeof(signal_addr));
				i = 0;
				while(1)
				{
					bzero(select_recv_buf,sizeof(select_recv_buf));
					//res = recvfrom(sockfd_recv, select_recv_buf, 512, 0, NULL, NULL);
					//printf("recvfrom  date !!!!!!!!!!\n ");
					//if(res <= 0)
					//{
						//break;
					//}
					timeVal.tv_sec = signal_report_timeout;
					timeVal.tv_usec = 0;

					FD_ZERO(&fdSet);
					FD_SET(sockfd_recv, &fdSet);

					ret = select((int) sockfd_recv + 1, &fdSet,NULL, NULL, &timeVal);
					if(ret == 0)                     //��ʱ
					{
						printf("cha xun ::select timeout::RED 2 GREEN \n");
						close(sockfd_recv);
						return -1;
					}
					if(ret)                //������
					{
						res = recvfrom(sockfd_recv, select_recv_buf, 512, 0, NULL, NULL);
						if(res == 0)
						{
							
							close(sockfd_recv);
							return -1;
						}		
						else
						{
							if(select_recv_buf[12 + 1 ] !=0 && select_recv_buf[12 + 1 ] <= 8 )
							{
								for(j = 0; j < 22; j++)
								{
									set_12_date_lin[select_recv_buf[12+ 1]][j] = select_recv_buf[j];
								}
							}	
						}
					}
					i++;

					if(i == 8)
					{
						break;
					}
				}
				
				if(i  !=  8)
				{
					close(sockfd_recv);
					return -1;
				}

					
				sprintf(sql,"delete  from  GUODUDENGSE  where signal_id = %d  " ,buf[7] * 256 + buf[8] );
				printf("%s\n",sql);
				st->setSQL(sql);
				st->executeUpdate();

				try
				{
					for(i = 0; i< 8;i++)
					{
						sprintf(sql,"insert into GUODUDENGSE(signal_id,clink_id) values(%d,%d)  " ,buf[7] * 256 + buf[8] ,i + 1);
						printf("%s\n",sql);
						st->setSQL(sql);
						st->executeUpdate();
					}
					
					for(i = 1 ; i <= 8; i++)
					{
						if(set_12_date_lin[i][12+ 1] != 0 )
						{
							sprintf(sql,"update GUODUDENGSE  set Red_green_1 = '%d',Red_green_time1= %d,Red_green_2='%d',Red_green_time2= %d, Red_green_3= '%d',Red_green_time3= %d where  signal_id = %d and clink_id = %d " ,
							set_12_date_lin[i][13 + 1] ,set_12_date_lin[i][14+ 1]/ 10,set_12_date_lin[i][15+ 1],set_12_date_lin[i][16+ 1]/ 10,set_12_date_lin[i][17+ 1],set_12_date_lin[i][18+ 1]/ 10,buf[7] * 256 + buf[8] ,set_12_date_lin[i][12+ 1] );
							
							printf("%s\n",sql);
							st->setSQL(sql);
							st->executeUpdate();
						}
						
					}
				}
				catch (SQLException& ex)
				{
					close(sockfd_recv);
					return -1;
				}
			
			
				close(sockfd_recv);
				return 0;
				
				
			}
			/*�̱���ѯ*/
			else if(buf[10] == 0x13)
			{
				unsigned char set_13_date_lin[9][22+10];
				struct timeval 	timeVal;
				fd_set  fdSet;
				int ret;
				sendto(sockfd_recv, select_buf, sizeof(select_buf), 0, (struct sockaddr*)&signal_addr, sizeof(signal_addr));
				i = 0;
				while(1)
				{
					bzero(select_recv_buf,sizeof(select_recv_buf));
					//res = recvfrom(sockfd_recv, select_recv_buf, 512, 0, NULL, NULL);
					//printf("recvfrom  date !!!!!!!!!!\n ");
					//if(res <= 0)
					//{
						//break;
					//}
					timeVal.tv_sec = signal_report_timeout;
					timeVal.tv_usec = 0;

					FD_ZERO(&fdSet);
					FD_SET(sockfd_recv, &fdSet);

					ret = select((int) sockfd_recv + 1, &fdSet,NULL, NULL, &timeVal);
					if(ret == 0)                     //��ʱ
					{
						printf("cha xun ::select timeout::GREEN 2 RED \n");
						close(sockfd_recv);
						return -1;
					}
					if(ret)                //������
					{
						res = recvfrom(sockfd_recv, select_recv_buf, 512, 0, NULL, NULL);
						if(res == 0)
						{	
	  						close(sockfd_recv);
							return -1;
						}		
						else
						{
							if(select_recv_buf[12+ 1] !=0 && select_recv_buf[12+ 1]  <= 8)
							{
								for(j = 0; j < 22; j++)
								{
									set_13_date_lin[select_recv_buf[12+ 1]][j] = select_recv_buf[j];
								}
							}
						}
					}
					i++;

					if(i == 8)
					{
						break;
					}
				}
				
				if(i  !=  8)
				{
					close(sockfd_recv);
					return -1;
				}

			
				try
				{
					for(i = 1 ; i <= 8; i++)
					{
						if(set_13_date_lin[i][12+ 1] != 0 )
						{
							sprintf(sql,"update GUODUDENGSE  set Green_red_1 = '%d',Green_red_time1= %d,Green_red_2='%d',Green_red_time2= %d, Green_red_3= '%d',Green_red_time3= %d where  signal_id = %d and clink_id = %d " ,
							set_13_date_lin[i][13 + 1 ] ,set_13_date_lin[i][14+ 1]/ 10,set_13_date_lin[i][15+ 1],set_13_date_lin[i][16+ 1]/ 10,set_13_date_lin[i][17+ 1],set_13_date_lin[i][18+ 1]/ 10,buf[7] * 256 + buf[8] ,set_13_date_lin[i][12+ 1] );
							
							printf("%s\n",sql);
							st->setSQL(sql);
							st->executeUpdate();
						}
						
					}
				}
				catch (SQLException& ex)
				{
					close(sockfd_recv);
					return -1;
				}
					
				close(sockfd_recv);
				return 0;
				
				
			}
			/*��λ���ݽṹ��ѯ*/
			else if(buf[10] == 0x03)
			{
				unsigned char set_03_date_select[17][185+5+10];
				char  linshi_buf_1[80];
				 char  linshi_buf_2[80];
				char  linshi_buf_lin[80];
				struct timeval 	timeVal;
				fd_set  fdSet;
				int ret;
				sendto(sockfd_recv, select_buf, sizeof(select_buf), 0, (struct sockaddr*)&signal_addr, sizeof(signal_addr));
				i = 0;
				printf("chaxun xiangwei shuju \n\n");
				while(1)
				{
					bzero(select_recv_buf,sizeof(select_recv_buf));
					//res = recvfrom(sockfd_recv, select_recv_buf, 512, 0, NULL, NULL);
					//printf("recvfrom  date !!!!!!!!!!:");
#if 0
					int www;

					for(www = 0; www<res; www++)
					{
						printf("%02x",select_recv_buf[www]);
					}
					printf("\n");
#endif				
					//if(res <= 0)
					//{
						//break;
					//}
					timeVal.tv_sec = signal_report_timeout;
					timeVal.tv_usec = 0;

					FD_ZERO(&fdSet);
					FD_SET(sockfd_recv, &fdSet);

					ret = select((int) sockfd_recv + 1, &fdSet,NULL, NULL, &timeVal);
					if(ret == 0)                     //��ʱ
					{
						printf("cha xun ::select timeout::xiang wei shu ju jie gou \n");
						close(sockfd_recv);
						return -1;
					}
					if(ret)                //������
					{
						res = recvfrom(sockfd_recv, select_recv_buf, 512, 0, NULL, NULL);
						if(res <= 0)
						{		
							close(sockfd_recv);
							return -1;
						}		
						else
						{
							printf("xiang wei shu ju %d\n",i);
							if(select_recv_buf[12 + 1] !=0 && select_recv_buf[12 + 1] <= 16)
							{
								for(j = 0; j < 185+5; j++)
								{
									set_03_date_select[select_recv_buf[12 + 1]][j] = select_recv_buf[j];
								}
							}
						}
					}
					i++;
				
					if(i == 16)
					{
						break;
					}
				}
				
				if(i  !=  16)
				{
					close(sockfd_recv);
					return -1;
				}

				sprintf(sql,"delete from  CLINK_INFO  where signal_id = %d " ,buf[7] * 256 + buf[8] );
				printf("%s\n",sql);
				st->setSQL(sql);
				st->executeUpdate();

			
					
				for(i = 1 ; i <= 16; i++)
				{
					if(set_03_date_select[i][12 + 1  ] != 0 )
					{
						int mm;
						bzero(linshi_buf_1,sizeof(linshi_buf_1));
						
						for(j = 0; j < 13 ;j ++)
						{
							for(mm = 0; mm < 8;mm++)
							{
								if((set_03_date_select[i][20 + 1+ j] & (0x80 >> mm)) != 0)
								{	
									sprintf(linshi_buf_lin,"%d," ,j * 8 + mm + 1);
									strcat(linshi_buf_1,linshi_buf_lin);
								}
							}
						}

			
						
						bzero(linshi_buf_2,sizeof(linshi_buf_2));
						
						for(j = 0; j < 13 ;j ++)
						{
							for(mm = 0; mm < 8;mm++)
							{
								if((set_03_date_select[i][33 + 1+ j] & (0x80 >> mm)) != 0)
								{
									sprintf(linshi_buf_lin,"%d," ,j * 8 + mm + 1);
									strcat(linshi_buf_2,linshi_buf_lin);
								//	sprintf(linshi_buf_2,"%s%d,",linshi_buf_2,j * 8 + m + 1);
								}
							}
						}

						try
						{
							sprintf(sql," insert into CLINK_INFO(id,signal_id,clink_id,CLINK_TYPE ,GREEN_MIN,GREEN_MAX,GREEN_LONG,CLINK_APPLY, CLINK_LONG,FOLLOW_CLINK_ID,LATE_GREEN_TIME,LONG_GREEN_TIME,SIGNAL_START_TYPE,RED_GREEN_TYPE, GREEN_RED_TYPE,PULSE_WIDTH_TIME,PEDESTRIAN_EMPTY_FLAG,PEDESTRIAN_EMPTY_TIME ) values( HIBERNATE_SEQUENCE.Nextval,%d,%d,%d,%d,%d,%d,'%s','%s',%d,%d,%d,%d,%d,%d,%d,%d,%d)" ,
							buf[7] * 256 + buf[8],set_03_date_select[i][12+ 1],set_03_date_select[i][13+ 1],(set_03_date_select[i][14+ 1] * 256 + set_03_date_select[i][15+ 1]) ,(set_03_date_select[i][16+ 1] * 256 + set_03_date_select[i][17+ 1]) ,(set_03_date_select[i][18+ 1] * 256 + set_03_date_select[i][19+ 1]) ,
							linshi_buf_1,linshi_buf_2,set_03_date_select[i][46+ 1],(set_03_date_select[i][47+ 1] * 256 + set_03_date_select[i][48+ 1]) ,(set_03_date_select[i][49+ 1] * 256 + set_03_date_select[i][50+ 1]) ,
							set_03_date_select[i][51+ 1+5],set_03_date_select[i][52+ 1+5],set_03_date_select[i][53+ 1+5],
							(set_03_date_select[i][51+ 1] * 256 + set_03_date_select[i][52+ 1]),set_03_date_select[i][53+ 1],(set_03_date_select[i][54+ 1] * 256 + set_03_date_select[i][55+ 1]));
							
							printf("%s\n",sql);
							st->setSQL(sql);
							st->executeUpdate();
						}
						catch (SQLException& ex)
						{
							close(sockfd_recv);
							return -1;
						}
						
					}
					
				}

				close(sockfd_recv);
				return 0;


			}
			/*�׶����ݽṹ��ѯ*/
			else if(buf[10] == 0x04)
			{
				unsigned char set_04_date_select[65][175+10];
				char  linshi_buf_1[80];
				char  linshi_buf_2[80];
				 char  linshi_buf_3[80];
				char  linshi_buf_lin[80];
				struct timeval 	timeVal;
				fd_set  fdSet;
				int ret;
				sendto(sockfd_recv, select_buf, sizeof(select_buf), 0, (struct sockaddr*)&signal_addr, sizeof(signal_addr));
				i = 0;
				printf("cha xun jie duan shu ju \n\n");
				while(1)
				{
					bzero(select_recv_buf,sizeof(select_recv_buf));
					//res = recvfrom(sockfd_recv, select_recv_buf, 512, 0, NULL, NULL);
#if 0		
					printf("recvfrom  date !!!!!!!!!!\n ");
					int www;
					for(www = 0; www < res;www ++)
					{
						printf("%02x",select_recv_buf[www]);
					}
					printf("\n");
#endif		
					//if(res <= 0)
					//{
					//	break;
					//}
					timeVal.tv_sec = signal_report_timeout;
					timeVal.tv_usec = 0;

					FD_ZERO(&fdSet);
					FD_SET(sockfd_recv, &fdSet);

					ret = select((int) sockfd_recv + 1, &fdSet,NULL, NULL, &timeVal);
					if(ret == 0)                     //��ʱ
					{
						printf("cha xun ::select timeout::jie duan shu ju jie gou \n");
						close(sockfd_recv);
						return -1;
					}
					if(ret)                //������
					{
						printf("jie duan shu ju %d\n",i);
						res = recvfrom(sockfd_recv, select_recv_buf, 512, 0, NULL, NULL);
						if(res <= 0)
						{		
							close(sockfd_recv);
							return -1;
						}		
						else
						{
							if(select_recv_buf[12+ 1] !=0 && select_recv_buf[12+ 1] <= 64)
							{
								for(j = 0; j < 175; j++)
								{
									set_04_date_select[select_recv_buf[12+ 1]][j] = select_recv_buf[j];
								}
							}
						}
					}
					i++;
				
					if(i == 64)
					{
						break;
					}


				}
				
				if(i  !=  64)
				{
					close(sockfd_recv);
					return -1;
				}

				/*���������λ3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30 ������*/
				char l_concat_buf[500];
				/*64���׶�idû�е�д0*/
				int l_concat_id[64];
				/*���ݿ���ڵĽ׶�����*/
				int l_concat_num;

				int l_4_num = 0;

				
				

				bzero(sql,sizeof(sql));
				sprintf(sql,"select replace(WMSYS.WM_CONCAT(t.stage_id),',',',') ,count(*) from stage_info t where t.signal_id= %d " ,buf[7] * 256 + buf[8] );
				printf("%s\n",sql);
				st->setSQL(sql);
				rs = st->executeQuery();
				while(rs->next())
				{
					
					bzero(l_concat_buf,sizeof(l_concat_buf));
					strcpy( l_concat_buf, rs->getString(1).c_str());
					l_concat_num = rs->getInt(2) ;
					
				}
			


					
				/*64���׶�ֱ�Ӹ���*/
				if(l_concat_num == 64)
				{
					for(i = 1 ; i <= 64; i++)
					{
						if(set_04_date_select[i][12+ 1] != 0 )
						{
							int mm;
							bzero(linshi_buf_1,sizeof(linshi_buf_1));
							
							for(j = 0; j < 2 ;j ++)
							{
								for(mm = 0; mm < 8;mm++)
								{
									if((set_04_date_select[i][13 + 1+ j] & (0x80 >> mm)) != 0)
									{	
										sprintf(linshi_buf_lin,"%d," ,j * 8 + mm + 1);
										strcat(linshi_buf_1,linshi_buf_lin);
									}
								}
							}
							bzero(linshi_buf_2,sizeof(linshi_buf_2));
							for(j = 0; j < 13 ;j ++)
							{
								for(mm = 0; mm < 8;mm++)
								{
									if((set_04_date_select[i][15 + 1+ j] & (0x80 >> mm)) != 0)
									{
										sprintf(linshi_buf_lin,"%d," ,j * 8 + mm + 1);
										strcat(linshi_buf_2,linshi_buf_lin);
									//	sprintf(linshi_buf_2,"%s%d,",linshi_buf_2,j * 8 + m + 1);
									}
								}
							}

							bzero(linshi_buf_3,sizeof(linshi_buf_3));
							for(j = 0; j < 13 ;j ++)
							{
								for(mm = 0; mm < 8;mm++)
								{
									if((set_04_date_select[i][28 + 1+ j] & (0x80 >> mm)) != 0)
									{
										sprintf(linshi_buf_lin,"%d," ,j * 8 + mm + 1);
										strcat(linshi_buf_3,linshi_buf_lin);
									//	sprintf(linshi_buf_2,"%s%d,",linshi_buf_2,j * 8 + m + 1);
									}
								}
							}

							
							bzero(sql,sizeof(sql));
							sprintf(sql," update STAGE_INFO  set Stage_clink = '%s', App_detector = '%s',Extend_Detector = '%s', Occ_auto = %d,Buchang_auto = %d,Stat_time_auto = %d  where signal_id = %d and Stage_id = %d " ,
							linshi_buf_1,linshi_buf_2,linshi_buf_3,set_04_date_select[i][41+ 1],set_04_date_select[i][42+ 1],set_04_date_select[i][43+ 1] / 10,buf[7] * 256 + buf[8],set_04_date_select[i][12+ 1]);
								
						//	printf("%s\n",sql);
							st->setSQL(sql);
							st->executeUpdate();
							
						}
						
					}
				}
				else
				{
					/*ȡ����λID*/
					
					result = strtok( l_concat_buf, "," );

					
					while( result != NULL ) 
					{
						l_concat_id[l_4_num]= atoi(result);

						l_4_num++;

						if(l_4_num == 63)
						{
							break;
						}
						
						result = strtok( NULL, "," );
					}

					


					for(i = 1 ; i <= 64; i++)
					{
						if(set_04_date_select[i][12+ 1] != 0 )
						{
							int mm;
							bzero(linshi_buf_1,sizeof(linshi_buf_1));
							
							for(j = 0; j < 2 ;j ++)
							{
								for(mm = 0; mm < 8;mm++)
								{
									if((set_04_date_select[i][13 + 1+ j] & (0x80 >> mm)) != 0)
									{	
										sprintf(linshi_buf_lin,"%d," ,j * 8 + mm + 1);
										strcat(linshi_buf_1,linshi_buf_lin);
									}
								}
							}
							bzero(linshi_buf_2,sizeof(linshi_buf_2));
							for(j = 0; j < 13 ;j ++)
							{
								for(mm = 0; mm < 8;mm++)
								{
									if((set_04_date_select[i][15 + 1+ j] & (0x80 >> mm)) != 0)
									{
										sprintf(linshi_buf_lin,"%d," ,j * 8 + mm + 1);
										strcat(linshi_buf_2,linshi_buf_lin);
									//	sprintf(linshi_buf_2,"%s%d,",linshi_buf_2,j * 8 + m + 1);
									}
								}
							}

							bzero(linshi_buf_3,sizeof(linshi_buf_3));
							for(j = 0; j < 13 ;j ++)
							{
								for(mm = 0; mm < 8;mm++)
								{
									if((set_04_date_select[i][28 + 1+ j] & (0x80 >> mm)) != 0)
									{
										sprintf(linshi_buf_lin,"%d," ,j * 8 + mm + 1);
										strcat(linshi_buf_3,linshi_buf_lin);
									//	sprintf(linshi_buf_2,"%s%d,",linshi_buf_2,j * 8 + m + 1);
									}
								}
							}

							for(mm = 0; mm < l_concat_num;mm++)
							{
								if(set_04_date_select[i][12+ 1] == (unsigned char)(l_concat_id[mm] & 0xff))
								{
									break;
								}
							}

							/*�׶β�����*/
							if(mm == l_concat_num)
							{
								bzero(sql,sizeof(sql));
								sprintf(sql," insert into STAGE_INFO(id,signal_id,Stage_id,Stage_clink, App_detector,Extend_Detector, Occ_auto,Buchang_auto,Stat_time_auto ) values( HIBERNATE_SEQUENCE.Nextval,%d,%d,'%s','%s','%s',%d,%d,%d)" ,
								buf[7] * 256 + buf[8],set_04_date_select[i][12+ 1],linshi_buf_1,linshi_buf_2,linshi_buf_3,set_04_date_select[i][41+ 1],set_04_date_select[i][42+ 1],set_04_date_select[i][43+ 1] / 10);
								
							//	printf("%s\n",sql);
								st->setSQL(sql);
								st->executeUpdate();
							}
							/*�׶��Ѿ�����*/
							else
							{
								bzero(sql,sizeof(sql));
								sprintf(sql," update STAGE_INFO  set Stage_clink = '%s', App_detector = '%s',Extend_Detector = '%s', Occ_auto = %d,Buchang_auto = %d,Stat_time_auto = %d  where signal_id = %d and Stage_id = %d " ,
								linshi_buf_1,linshi_buf_2,linshi_buf_3,set_04_date_select[i][41+ 1],set_04_date_select[i][42+ 1],set_04_date_select[i][43+ 1] / 10,buf[7] * 256 + buf[8],set_04_date_select[i][12+ 1]);
									
							//	printf("%s\n",sql);
								st->setSQL(sql);
								st->executeUpdate();
							}
							
							
							
						}
						
					}
					

				}

			

#if 0			
				
			
				sprintf(sql,"delete from  STAGE_INFO  where signal_id = %d " ,buf[7] * 256 + buf[8] );
				printf("%s\n",sql);
				st->setSQL(sql);
				st->executeUpdate();

				for(i = 1 ; i <= 64; i++)
				{
					if(set_04_date_select[i][12+ 1] != 0 )
					{
						int mm;
						bzero(linshi_buf_1,sizeof(linshi_buf_1));
						
						for(j = 0; j < 2 ;j ++)
						{
							for(mm = 0; mm < 8;mm++)
							{
								if((set_04_date_select[i][13 + 1+ j] & (0x80 >> mm)) != 0)
								{	
									sprintf(linshi_buf_lin,"%d," ,j * 8 + mm + 1);
									strcat(linshi_buf_1,linshi_buf_lin);
								}
							}
						}
						bzero(linshi_buf_2,sizeof(linshi_buf_2));
						for(j = 0; j < 13 ;j ++)
						{
							for(mm = 0; mm < 8;mm++)
							{
								if((set_04_date_select[i][15 + 1+ j] & (0x80 >> mm)) != 0)
								{
									sprintf(linshi_buf_lin,"%d," ,j * 8 + mm + 1);
									strcat(linshi_buf_2,linshi_buf_lin);
								//	sprintf(linshi_buf_2,"%s%d,",linshi_buf_2,j * 8 + m + 1);
								}
							}
						}

						bzero(linshi_buf_3,sizeof(linshi_buf_3));
						for(j = 0; j < 13 ;j ++)
						{
							for(mm = 0; mm < 8;mm++)
							{
								if((set_04_date_select[i][28 + 1+ j] & (0x80 >> mm)) != 0)
								{
									sprintf(linshi_buf_lin,"%d," ,j * 8 + mm + 1);
									strcat(linshi_buf_3,linshi_buf_lin);
								//	sprintf(linshi_buf_2,"%s%d,",linshi_buf_2,j * 8 + m + 1);
								}
							}
						}

						try
						{
							sprintf(sql," insert into STAGE_INFO(id,signal_id,Stage_id,Stage_clink, App_detector,Extend_Detector, Occ_auto,Buchang_auto,Stat_time_auto ) values( HIBERNATE_SEQUENCE.Nextval,%d,%d,'%s','%s','%s',%d,%d,%d)" ,
							buf[7] * 256 + buf[8],set_04_date_select[i][12+ 1],linshi_buf_1,linshi_buf_2,linshi_buf_3,set_04_date_select[i][41+ 1],set_04_date_select[i][42+ 1],set_04_date_select[i][43+ 1] / 10);
							
							printf("%s\n",sql);
							st->setSQL(sql);
							st->executeUpdate();
						}
						catch (SQLException& ex)
						{
							close(sockfd_recv);
							return -1;
						}
					}
					
				}
#endif
				
				close(sockfd_recv);
				return 0;
			}
			/* �������ݽṹ��ѯ*/
			else if(buf[10] == 0x05)
			{
				int plan_id;
				int plan_num;
				struct timeval 	timeVal;
				fd_set  fdSet;
				int ret;
				unsigned char set_05_date_select[129][214+10];
				


				sendto(sockfd_recv, select_buf, sizeof(select_buf), 0, (struct sockaddr*)&signal_addr, sizeof(signal_addr));
				
				i = 0;
				while(1)
				{
					bzero(select_recv_buf,sizeof(select_recv_buf));
					//res = recvfrom(sockfd_recv, select_recv_buf, 512, 0, NULL, NULL);

			#if 0	
					if(select_recv_buf[12+ 1] == 5)
						{
					printf("recvfrom  date !!!!!!!!!!\n ");
					int www;
					for(www = 0; www < res ;www ++)
						printf("%02x",select_recv_buf[www]);
					printf("\n");

								printf("select_recv_buf35=  %02x   select_recv_buf36= %02x\n",select_recv_buf[35],select_recv_buf[36]);
									
						}
			#endif
					
					//if(res <= 0)
					//{
					//	break;
					//}
					timeVal.tv_sec = signal_report_timeout;
					timeVal.tv_usec = 0;

					FD_ZERO(&fdSet);
					FD_SET(sockfd_recv, &fdSet);

					ret = select((int) sockfd_recv + 1, &fdSet,NULL, NULL, &timeVal);
					if(ret == 0)                     //��ʱ
					{
						printf("cha xun ::select timeout::xiang wei shu ju jie gou \n");
						close(sockfd_recv);
						return -1;
					}
					if(ret)                //������
					{
						res = recvfrom(sockfd_recv, select_recv_buf, 512, 0, NULL, NULL);
						if(res <= 0)
						{		
							close(sockfd_recv);
							return -1;
						}		
						else
						{
							if(select_recv_buf[12+ 1] != 0 &&  select_recv_buf[12+ 1] <=128)
							{
								for(j = 0; j < 214; j++)
								{
									set_05_date_select[select_recv_buf[12+ 1]][j] = select_recv_buf[j];
								}
							}
						}
					}
					i++;
				
					if(i == 128)
					{
						break;
					}
				}
				
				if(i  !=  128)
				{
					close(sockfd_recv);
					return -1;
				}

				
					

				
				sprintf(sql,"delete from  PLAN_CONFIG_CHAIN  where signal_id = %d  and  plan_id in (select t.id from plan_config t where to_number(t.plan_id)<129 and t.signal_id= %d) " ,buf[7] * 256 + buf[8] ,buf[7] * 256 + buf[8]);
				//printf("%s\n",sql);
				st->setSQL(sql);
				st->executeUpdate();

				sprintf(sql,"delete from  PLAN_CONFIG  where signal_id = %d  and to_number(plan_id)<129  " ,buf[7] * 256 + buf[8] );
			//	printf("%s\n",sql);
				st->setSQL(sql);
				st->executeUpdate();

				for(i = 1 ; i <= 128; i++)
				{
					
					if(set_05_date_select[i][12 + 1 ] != 0 )
					{
						sprintf(sql,"select HIBERNATE_SEQUENCE.Nextval from dual");
						st->setSQL(sql);
						rs = st->executeQuery();
						if(rs->next() != 0)
						{
							plan_id = rs->getInt(1);
						}
						plan_num = set_05_date_select[i][18 + 1];

						try
						{
							sprintf(sql," insert into PLAN_CONFIG(list_unit_id,id,signal_id,Plan_id,Plan_week,Plan_record,Xianweicha_time) values(1, %d,%d,%d,%d,%d,%d)" ,
							plan_id,buf[7] * 256 + buf[8],set_05_date_select[i][12 + 1],(set_05_date_select[i][13+ 1] * 256 + set_05_date_select[i][14+ 1]) / 10,set_05_date_select[i][15+ 1],(set_05_date_select[i][16+ 1] * 256 + set_05_date_select[i][17+ 1]) / 10);
							
						//	printf("%s\n",sql);
							st->setSQL(sql);
							st->executeUpdate();
							for(j = 0 ; j <plan_num;j++ )
							{
							
								
								sprintf(sql," insert into PLAN_CONFIG_CHAIN(id,signal_id,Plan_id,Stage_order,Stage_id,Stage_time,Stage_type) values( HIBERNATE_SEQUENCE.Nextval,%d,%d,%d,%d,%d,%d)" ,
								buf[7] * 256 + buf[8],plan_id,j+ 1,set_05_date_select[i][19 + 1+ j],(set_05_date_select[i][35 + 1+ j * 2] * 256 + set_05_date_select[i][36 + 1+ j * 2]) / 10,set_05_date_select[i][67 + 1+ j]);
							
							//	printf("%s\n",sql);
								st->setSQL(sql);
								st->executeUpdate();
							}
						}
						catch (SQLException& ex)
						{
							close(sockfd_recv);
							return -1;
						}
					}
				}

				close(sockfd_recv);
				return 0;
				
			}
			/* ʱ�����ݽṹ��ѯ*/
			else if(buf[10] == 0x06)
			{

				unsigned char set_06_date_slelect[129][337+10];
				struct timeval 	timeVal;
				fd_set  fdSet;
				int ret;
				sendto(sockfd_recv, select_buf, sizeof(select_buf), 0, (struct sockaddr*)&signal_addr, sizeof(signal_addr));
				
				i = 0;
				while(1)
				{
					bzero(select_recv_buf,sizeof(select_recv_buf));
					//res = recvfrom(sockfd_recv, select_recv_buf, 512, 0, NULL, NULL);
			//		printf("recvfrom  date !!!!!!!!!!\n ");
					//if(res <= 0)
					//{
						//break;
					//}
					timeVal.tv_sec = signal_report_timeout;
					timeVal.tv_usec = 0;

					FD_ZERO(&fdSet);
					FD_SET(sockfd_recv, &fdSet);

					ret = select((int) sockfd_recv + 1, &fdSet,NULL, NULL, &timeVal);
					if(ret == 0)                     //��ʱ
					{
						printf("cha xun ::select timeout::shi jian shu ju jie gou \n");
						close(sockfd_recv);
						return -1;
					}
					if(ret)                //������
					{
						res = recvfrom(sockfd_recv, select_recv_buf, 512, 0, NULL, NULL);
						if(res <= 0)
						{		
							close(sockfd_recv);
							return -1;
						}		
						else
						{
							if(select_recv_buf[12 + 1 ] !=0 && select_recv_buf[12+ 1] <= 128)
							{
								for(j = 0; j < 337; j++)
								{
									set_06_date_slelect[select_recv_buf[12+ 1]][j] = select_recv_buf[j];
								}
							}
						}
					}
					i++;

		//			printf("i = %d ,Time_id = %d\n",i,select_recv_buf[13]);
					
					if(i == 128)
					{
						break;
					}
				}
				
				if(i  !=  128)
				{
					close(sockfd_recv);
					return -1;
				}

				sprintf(sql,"delete from  SJBSDSZ_INFO  where signal_id = %d  and  FUNCTION_TYPE = 1" ,buf[7] * 256 + buf[8] );
				printf("%s\n",sql);
				st->setSQL(sql);
				st->executeUpdate();
				
				sprintf(sql,"delete from  TIME_INFO  where signal_id = %d and FUNCTION_TYPE = 1 " ,buf[7] * 256 + buf[8] );
				printf("%s\n",sql);
				st->setSQL(sql);
				st->executeUpdate();

			
				try
				{
					for(i = 1 ; i <= 128; i++)
					{
						
						if(set_06_date_slelect[i][12 + 1] != 0 )
						{
							
							sprintf(sql," insert into TIME_INFO(signal_id,Time_id,Unit_id,FUNCTION_TYPE) values(%d,%d,(select Unit_id from UNIT_INFO where signal_id = %d ),1)" ,
							buf[7] * 256 + buf[8],set_06_date_slelect[i][12+ 1],buf[7] * 256 + buf[8]);
						//	printf("%s\n",sql);
							st->setSQL(sql);
							st->executeUpdate();

							
							for(j = 0 ; j <set_06_date_slelect[i][13 + 1];j++ )
							{
								sprintf(sql," insert into SJBSDSZ_INFO(signal_id,Time_id,Time_set_number,Hour,Minute,Yunxing_mode,Fangan,FUNCTION_TYPE) values( %d,%d,%d,%d,%d,%d,%d,1)" ,
								buf[7] * 256 + buf[8],set_06_date_slelect[i][12+ 1], j +1,set_06_date_slelect[i][14 + 1+ j * 4] ,set_06_date_slelect[i][15 + 1+ j * 4] ,set_06_date_slelect[i][16+ 1 + j * 4] ,set_06_date_slelect[i][17 + 1+ j * 4]);

								if(set_06_date_slelect[i][12+ 1] == 128)
								{
									printf("%s\n",sql);
								}
								
								st->setSQL(sql);
								st->executeUpdate();
							}
						}
					}
				}
				catch (SQLException& ex)
				{
					close(sockfd_recv);
					return -1;
				}
					
			}
			/*d���ȱ��ѯ*/
			else if(buf[10] == 0x07)
			{
				unsigned char set_07_date_select[129][161+10];
				char  linshi_buf_1[80];
				char  linshi_buf_2[80];
				 char  linshi_buf_3[80];
				char  linshi_buf_lin[80];
				struct timeval 	timeVal;
				fd_set  fdSet;
				int ret;

				sendto(sockfd_recv, select_buf, sizeof(select_buf), 0, (struct sockaddr*)&signal_addr, sizeof(signal_addr));
				
				i = 0;
				while(1)
				{
					bzero(select_recv_buf,sizeof(select_recv_buf));
					//res = recvfrom(sockfd_recv, select_recv_buf, 512, 0, NULL, NULL);
					//printf("recvfrom  date !!!!!!!!!!\n ");
					//if(res <= 0)
					//{
						//break;
					//}
					timeVal.tv_sec = signal_report_timeout;
					timeVal.tv_usec = 0;

					FD_ZERO(&fdSet);
					FD_SET(sockfd_recv, &fdSet);

					ret = select((int) sockfd_recv + 1, &fdSet,NULL, NULL, &timeVal);
					if(ret == 0)                     //��ʱ
					{
						printf("cha xun ::select timeout::diao du biao \n");
						close(sockfd_recv);
						return -1;
					}
					if(ret)                //������
					{
						res = recvfrom(sockfd_recv, select_recv_buf, 512, 0, NULL, NULL);
						if(res <= 0)
						{		
							close(sockfd_recv);
							return -1;
						}		
						else
						{
							if(select_recv_buf[12+ 1] !=0 && select_recv_buf[12+ 1] <= 128)
							{
								for(j = 0; j < 161; j++)
								{
									set_07_date_select[select_recv_buf[12+ 1]][j] = select_recv_buf[j];
								}
							}
						}
					}
					i++;
				
					if(i == 128)
					{
						break;
					}
				}
				
				
				if(i  !=  128)
				{
					close(sockfd_recv);
					return -1;
				}

				sprintf(sql,"delete from  DIAODU_INFO  where signal_id = %d  and FUNCTION_TYPE = 1" ,buf[7] * 256 + buf[8] );
				printf("%s\n",sql);
				st->setSQL(sql);
				st->executeUpdate();

				for(i = 1 ; i <= 128; i++)
				{
					
					
					if(set_07_date_select[i][12+ 1] != 0 )
					{
						int mm;


					
						bzero(linshi_buf_1,sizeof(linshi_buf_1));
						for(mm = 0; mm < 8;mm++)
						{
							
							if((set_07_date_select[i][15+ 1] & (0x01 << mm)) != 0)
							{	
								if(mm == 0)
								{
									sprintf(linshi_buf_lin,"7," );	
								}
								else
								{
									sprintf(linshi_buf_lin,"%d," , mm );
								}
								
								strcat(linshi_buf_1,linshi_buf_lin);
							}
						}
					
						
						bzero(linshi_buf_2,sizeof(linshi_buf_2));
						for(j = 0; j < 2 ;j ++)
						{
							for(mm = 0; mm < 8;mm++)
							{
								if((set_07_date_select[i][17+ 1 - j] & (0x01 << mm)) != 0)
								{
									sprintf(linshi_buf_lin,"%d," ,j * 8 + mm );
									strcat(linshi_buf_2,linshi_buf_lin);
								//	sprintf(linshi_buf_2,"%s%d,",linshi_buf_2,j * 8 + m + 1);
								}
							}
						}
						
						bzero(linshi_buf_3,sizeof(linshi_buf_3));
						for(j = 0; j < 4 ;j ++)
						{
							for(mm = 0; mm < 8;mm++)
							{
								if((set_07_date_select[i][21+ 1 - j] & (0x01 << mm)) != 0)
								{
									sprintf(linshi_buf_lin,"%d," ,j * 8 + mm );
									strcat(linshi_buf_3,linshi_buf_lin);
								//	sprintf(linshi_buf_2,"%s%d,",linshi_buf_2,j * 8 + m + 1);
								}
							}
						}

						try
						{
							sprintf(sql," insert into DIAODU_INFO(unit_id,signal_id,Ddb_id,Ddb_type,Ddb_priority,Week_ddb_type,Date_ddb_type_y ,Date_ddb_type_r, Time_table_number_1,FUNCTION_TYPE) values((select unit_id from unit_info where signal_id = %d),%d,%d,%d,%d,'%s','%s','%s',%d,1)" ,
							buf[7] * 256 + buf[8],buf[7] * 256 + buf[8],set_07_date_select[i][12+ 1],set_07_date_select[i][13+ 1],set_07_date_select[i][14+ 1],linshi_buf_1,linshi_buf_2,linshi_buf_3,set_07_date_select[i][29+ 1]);
							printf("%s\n",sql);
							st->setSQL(sql);
							st->executeUpdate();
						}
						catch (SQLException& ex)
						{
							close(sockfd_recv);
							return -1;
						}
					}
				}
				close(sockfd_recv);
				return 0;
			}
			/* ��λ��ͻ��ѯ*/
			else if(buf[10] == 0x08)
			{
				unsigned char set_08_date_select[270 + 1+10];
				char  linshi_buf_1[80];
				char  linshi_buf_lin[80];

				struct timeval 	timeVal;
				fd_set  fdSet;
				int ret;
				sendto(sockfd_recv, select_buf, sizeof(select_buf), 0, (struct sockaddr*)&signal_addr, sizeof(signal_addr));
				bzero(select_recv_buf,sizeof(select_recv_buf));
				//res = recvfrom(sockfd_recv, select_recv_buf, 512, 0, NULL, NULL);
				//printf("recvfrom  date !!!!!!!!!!\n ");
				//if(res <= 0)
				//{
					//close(sockfd_recv);
					//return -1;
				//}
				timeVal.tv_sec = signal_report_timeout;
					timeVal.tv_usec = 0;

					FD_ZERO(&fdSet);
					FD_SET(sockfd_recv, &fdSet);

					ret = select((int) sockfd_recv + 1, &fdSet,NULL, NULL, &timeVal);
					if(ret == 0)                     //��ʱ
					{
						printf("cha xun ::select timeout::xiang wei chong tu \n");
						close(sockfd_recv);
						return -1;
					}
					if(ret)                //������
					{
						res = recvfrom(sockfd_recv, select_recv_buf, 512, 0, NULL, NULL);
						if(res <= 0)
						{		
							close(sockfd_recv);
							return -1;
						}		
						else
						{
							for(j = 0; j < 270+ 1; j++)
							{
								set_08_date_select[j] = select_recv_buf[j];
							}
						}
					}
				
					sprintf(sql,"delete from  CHONGTU_INFO  where signal_id = %d " ,buf[7] * 256 + buf[8] );
					printf("%s\n",sql);
					st->setSQL(sql);
					st->executeUpdate();

					for(i = 0; i < 16 ; i++)
					{
						int all_num = 0;
						bzero(linshi_buf_1,sizeof(linshi_buf_1));
						for(j = 0; j < 16; j++)
						{
							all_num += set_08_date_select[12 + 1 + i * 16 + j ];
							if(set_08_date_select[12 + 1+ i * 16 + j ] == 1)
							{
								sprintf(linshi_buf_lin,"%d," ,j+1);
								strcat(linshi_buf_1,linshi_buf_lin);
							}
						}

						//if(all_num != 0)
						try
						{
							if(1)
							{
								sprintf(sql," insert into CHONGTU_INFO(signal_id,Clink_id,Rush_clink) values(%d,%d,'%s')" ,
								buf[7] * 256 + buf[8],i+1,linshi_buf_1);
								printf("%s\n",sql);
								st->setSQL(sql);
								st->executeUpdate();
							}
						}
						catch (SQLException& ex)
						{
							close(sockfd_recv);
							return -1;
						}
					//}
					
				}

				close(sockfd_recv);
				return 0;		
			}
			/* �׶ι���Լ�����ѯ*/
			else if(buf[10] == 0x09)
			{
				unsigned char set_09_date_select[65+500][78+ 1+10+100];

				time_t now_2, now_1 ;
				struct timeval 	timeVal;
				fd_set  fdSet;
				int ret;

				sendto(sockfd_recv, select_buf, sizeof(select_buf), 0, (struct sockaddr*)&signal_addr, sizeof(signal_addr));
				
				now_2 = time(NULL);
				i = 0;
				while(1)
				{
					bzero(select_recv_buf,sizeof(select_recv_buf));
					//res = recvfrom(sockfd_recv, select_recv_buf, 512, 0, NULL, NULL);
				//	printf("recvfrom  date !!!!!!!!!!\n ");
#if 0
				int www;
				for(www = 0; www < res;www++)
				{
					printf("%02x",select_recv_buf[www]);
				}
				printf("\n");
				
				
#endif
					//if(res <= 0)
					//{
						//break;
					//}
					timeVal.tv_sec = signal_report_timeout;
					timeVal.tv_usec = 0;

					FD_ZERO(&fdSet);
					FD_SET(sockfd_recv, &fdSet);

					ret = select((int) sockfd_recv + 1, &fdSet,NULL, NULL, &timeVal);
					if(ret == 0)                     //��ʱ
					{
						printf("cha xun ::select timeout::xiang wei shu ju jie gou \n");
						close(sockfd_recv);
						return -1;
					}
					if(ret)                //������
					{
						res = recvfrom(sockfd_recv, select_recv_buf, 512, 0, NULL, NULL);
						if(res <= 0)
						{		
							close(sockfd_recv);
							return -1;
						}		
						else
						{
							if(select_recv_buf[11] > 64)
							{
								continue;
							}
						
							for(j = 0; j < 78+ 1; j++)
							{
								set_09_date_select[select_recv_buf[11]][j] = select_recv_buf[j];
							}
						}
					
					}
					i++;
				
					if(i == 64)
					{
						break;
					}
				}
				
				
				if(i  != 64)
				{
					close(sockfd_recv);
					return -1;
				}

				int num_stage;
				sprintf(sql,"select  count(*) from  stage_info where signal_id = %d  " ,buf[7] * 256 + buf[8] );
			//	printf("%s\n",sql);
				st->setSQL(sql);
				rs = st->executeQuery();
				while(rs->next())
				{
					num_stage = rs->getInt(1);
				}
				
				sprintf(sql,"delete from  GUODUYUESHU  where signal_id = %d " ,buf[7] * 256 + buf[8] );
			//	printf("%s\n",sql);
				st->setSQL(sql);
				st->executeUpdate();

#if 1

				/*��������*/

				string userName = oracle_username;  
			    	string password = oracle_password;  
			    	string connectString = oracle_connectstring; 

					
				unsigned int maxConn=5;
				unsigned int minConn=1;
				unsigned int incrConn=2;

				 oracle::occi::Environment *env_09 = oracle::occi::Environment::createEnvironment(Environment::OBJECT);
				       ConnectionPool *connPool=env_09->createConnectionPool(
			         userName,
			            password,
			        connectString,
			        minConn,
			       maxConn,
			       incrConn);

				
				  //�����ӳػ�ȡ����
				Connection *conn_09=connPool->createConnection(userName,password);
				 
												 
				Statement *stmt = conn_09->createStatement();
					
				stmt->setAutoCommit(false);


				string sql_09;

				sql_09 = "insert into   GUODUYUESHU(signal_id,Stage_id_row,Stage_id_column,Guodu_value )  values(:x,:y,:z,:m)";
			//	sprintf(sql,"%s","insert into   GUODUYUESHU(signal_id,Stage_id_row,Stage_id_column,Guodu_value )  values(:x,:y,:z,:m)");
			//	printf("sql = %s\n",sql_09);
			//	sql="insert into luanyu values(:x,:y)";
				stmt->setSQL(sql_09);
			       stmt->setMaxIterations(num_stage * num_stage);
				   
				
				stmt->setMaxParamSize(1,sizeof(int));
			       stmt->setMaxParamSize(2,sizeof(int));
				stmt->setMaxParamSize(3,sizeof(int));
				stmt->setMaxParamSize(4,sizeof(int));

				int l_num = 0;
				for(i = 0 ; i < num_stage; i++)
				{
					
					for(j = 0; j < num_stage;j++)
					{
						 stmt->setNumber(1,buf[7] * 256 + buf[8]);
						 stmt->setNumber(2,i+1);
						 stmt->setNumber(3,j+1);
						 stmt->setNumber(4,set_09_date_select[i][12 + 1 + j]);
				//		sprintf(sql,"insert into   GUODUYUESHU(signal_id,Stage_id_row,Stage_id_column,Guodu_value )  values(%d,%d,%d,'%d') " ,
				//		buf[7] * 256 + buf[8] ,i+1, j+1 ,set_09_date_select[i][12 + 1 + j]);
				//		printf("%s\n",sql);
				//		st->setSQL(sql);
				//		st->executeUpdate();
						  if (l_num !=(num_stage * num_stage - 1))
					           {
					           	stmt->addIteration();	
					           }
						  l_num++;
					}
				
				}

				stmt->executeUpdate();
		       //�Ͽ����ӳ�����
		        conn_09->terminateStatement(stmt);
		        connPool->terminateConnection(conn_09);

			
			 env_09->terminateConnectionPool(connPool);
			  //env->terminateConnection(conn);
			oracle::occi::Environment::terminateEnvironment(env_09);

#endif
				

#if 0			
				/*��������*/
				try
				{
					for(i = 0 ; i < num_stage; i++)
					{
						
						for(j = 0; j < num_stage;j++)
						{
					

							sprintf(sql,"insert into   GUODUYUESHU(signal_id,Stage_id_row,Stage_id_column,Guodu_value )  values(%d,%d,%d,'%d') " ,
							buf[7] * 256 + buf[8] ,i+1, j+1 ,set_09_date_select[i][12 + 1 + j]);
					//		printf("%s\n",sql);
							st->setSQL(sql);
							st->executeUpdate();
						}
					
					}
				}
				catch (SQLException& ex)
				{
					close(sockfd_recv);
					return -1;
				}
#endif			
				now_1 = time(NULL);

				printf("time = %d\n",(int)(now_1 - now_2));
				close(sockfd_recv);
				return 0;
				
			}
			/* �ֶ�������ѯ*/
			else if(buf[10] == 0x0a)
			{
				unsigned char set_0a_date_select[28+ 1+10];
				struct timeval 	timeVal;
				fd_set  fdSet;
				int ret;

				sendto(sockfd_recv, select_buf, sizeof(select_buf), 0, (struct sockaddr*)&signal_addr, sizeof(signal_addr));
				bzero(select_recv_buf,sizeof(select_recv_buf));
				//res = recvfrom(sockfd_recv, select_recv_buf, 512, 0, NULL, NULL);
		#if 0
				int www;
				for(www = 0; www<res ; www++)
					printf("%02x",select_recv_buf[www]);
				printf("\n");
		#endif
				
				//if(res <= 0)
				//{
					//close(sockfd_recv);
					//return -1;
				//}
				timeVal.tv_sec = signal_report_timeout;
					timeVal.tv_usec = 0;

					FD_ZERO(&fdSet);
					FD_SET(sockfd_recv, &fdSet);

					ret = select((int) sockfd_recv + 1, &fdSet,NULL, NULL, &timeVal);
					if(ret == 0)                     //��ʱ
					{
						printf("cha xun ::select timeout::xiang wei shu ju jie gou \n");
						close(sockfd_recv);
						return -1;
					}
					if(ret)                //������
					{
						res = recvfrom(sockfd_recv, select_recv_buf, 512, 0, NULL, NULL);
						if(res <= 0)
						{		
							close(sockfd_recv);
							return -1;
						}		
						else
						{
							for(j = 0; j < 28+ 1; j++)
							{
								set_0a_date_select[j] = select_recv_buf[j];
							}
						}
					}
				
					sprintf(sql,"delete from  SDPZ_INFO  where signal_id = %d " ,buf[7] * 256 + buf[8] );
				//	printf("%s\n",sql);
					st->setSQL(sql);
					st->executeUpdate();

					try
					{
						for(i = 0; i < 10; i++)
						{
							sprintf(sql,"insert into  SDPZ_INFO(unit_id,signal_id,ANJIAN,DYJD,SETIMEOUT)  values(1,'%d','%d','%d',%d)" 
								 ,buf[7] * 256 + buf[8] ,i+1,set_0a_date_select[12+ 1 + i],
								(set_0a_date_select[22+ 1] * 256 * 256*256 +set_0a_date_select[23+ 1]* 256 * 256 + set_0a_date_select[24+ 1] * 256 + set_0a_date_select[25+ 1]) );
							printf("%s\n",sql);
							st->setSQL(sql);
							st->executeUpdate();
						}
					}
					catch (SQLException& ex)
					{
						close(sockfd_recv);
						return -1;
					}
					
					close(sockfd_recv);
					return 0;
				//}
				
			}
			/* ����������ѯ*/
			else if(buf[10] == 0x0b)
			{
				unsigned char set_0b_date_select[26+ 1+10];
				sendto(sockfd_recv, select_buf, sizeof(select_buf), 0, (struct sockaddr*)&signal_addr, sizeof(signal_addr));
				bzero(select_recv_buf,sizeof(select_recv_buf));
				
				//res = recvfrom(sockfd_recv, select_recv_buf, 512, 0, NULL, NULL);
				//printf("recvfrom  date !!!!!!!!!!\n ");
				//if(res <= 0)
				//{
					//close(sockfd_recv);
					//return -1;
				//}
				struct timeval 	timeVal;
				fd_set  fdSet;
				int ret;
				timeVal.tv_sec = signal_report_timeout;
					timeVal.tv_usec = 0;

					FD_ZERO(&fdSet);
					FD_SET(sockfd_recv, &fdSet);

					ret = select((int) sockfd_recv + 1, &fdSet,NULL, NULL, &timeVal);
					if(ret == 0)                     //��ʱ
					{
						printf("cha xun ::select timeout::xiang wei shu ju jie gou \n");
						close(sockfd_recv);
						return -1;
					}
					if(ret)                //������
					{
						res = recvfrom(sockfd_recv, select_recv_buf, 512, 0, NULL, NULL);
						if(res <= 0)
						{		
							close(sockfd_recv);
							return -1;
						}		
						else
						{
							for(j = 0; j < 26+ 1; j++)
							{
								set_0b_date_select[j] = select_recv_buf[j];
							}
						}
					}

					sprintf(sql,"delete from  JINJIPEIZHI  where signal_id = %d " ,buf[7] * 256 + buf[8] );
					printf("%s\n",sql);
					st->setSQL(sql);
					st->executeUpdate();

					try
					{
						for(i = 0; i < 4 ;i++)
						{
							sprintf(sql,"insert into  JINJIPEIZHI(unit_id,signal_id,Jinji_id,Jinji_stage,Jinji_logo)  values((select unit_id from unit_info where signal_id = %d),%d,%d,%d,%d)" 
								,buf[7] * 256 + buf[8] ,buf[7] * 256 + buf[8] ,set_0b_date_select[12+ 1 + i * 3],set_0b_date_select[13 + 1+ i * 3],set_0b_date_select[14 + 1+ i * 3]);
							printf("%s\n",sql);
							st->setSQL(sql);
							st->executeUpdate();
						}
					}
					catch (SQLException& ex)
					{
						close(sockfd_recv);
						return -1;
					}
				
				close(sockfd_recv);
				return 0;

			}
			/* ���Ȳ�����ѯ*/
			else if(buf[10] == 0x0c)
			{
				unsigned char set_0c_date_select[26+ 1+10];
				struct timeval 	timeVal;
				fd_set  fdSet;
				int ret;
				sendto(sockfd_recv, select_buf, sizeof(select_buf), 0, (struct sockaddr*)&signal_addr, sizeof(signal_addr));
				bzero(select_recv_buf,sizeof(select_recv_buf));
				//res = recvfrom(sockfd_recv, select_recv_buf, 512, 0, NULL, NULL);
				//printf("recvfrom  date !!!!!!!!!!\n ");
				//if(res <= 0)
				//{
					//close(sockfd_recv);
					//return -1;
				//}
				timeVal.tv_sec = signal_report_timeout;
					timeVal.tv_usec = 0;

					FD_ZERO(&fdSet);
					FD_SET(sockfd_recv, &fdSet);

					ret = select((int) sockfd_recv + 1, &fdSet,NULL, NULL, &timeVal);
					if(ret == 0)                     //��ʱ
					{
						printf("cha xun ::select timeout::xiang wei shu ju jie gou \n");
						close(sockfd_recv);
						return -1;
					}
					if(ret)                //������
					{
						res = recvfrom(sockfd_recv, select_recv_buf, 512, 0, NULL, NULL);
						if(res <= 0)
						{		
							close(sockfd_recv);
							return -1;
						}		
						else
						{
							for(j = 0; j < 26+ 1; j++)
							{
								set_0c_date_select[j] = select_recv_buf[j];
							}
						}
					}

					sprintf(sql,"delete from  PRIORIT_CONFIG  where signal_id = %d " ,buf[7] * 256 + buf[8] );
					printf("%s\n",sql);
					st->setSQL(sql);
					st->executeUpdate();

					for(i = 0; i < 4 ;i++)
					{
						try
						{
							sprintf(sql,"insert into  PRIORIT_CONFIG(unit_id,signal_id,Priority_id,Priority_stage,Priority_logo)  values((select unit_id from unit_info where signal_id = %d),%d,%d,%d,%d)" 
								,buf[7] * 256 + buf[8] ,buf[7] * 256 + buf[8] ,set_0c_date_select[12 + 1+ i * 3],set_0c_date_select[13+ 1 + i * 3],set_0c_date_select[14+ 1 + i * 3]);
							printf("%s\n",sql);
							st->setSQL(sql);
							st->executeUpdate();


						}
						catch(SQLException& ex)
						{
							close(sockfd_recv);
							return -1;
						}
					}
				
				close(sockfd_recv);
				return 0;
			
			}
			/* �׶��������Ͳ�ѯ*/
			else if(buf[10] == 0x14)
			{
				unsigned char set_14_date_select[26+100][100+10];
				char  linshi_buf_1[500];
				char  linshi_buf_2[500];
				char  linshi_buf_3[500];
				char  linshi_buf_4[500];
				char  linshi_buf_lin[500];
				struct timeval 	timeVal;
				fd_set  fdSet;
				int ret;
				sendto(sockfd_recv, select_buf, sizeof(select_buf), 0, (struct sockaddr*)&signal_addr, sizeof(signal_addr));
				i = 0;
				while(1)
				{
					bzero(select_recv_buf,sizeof(select_recv_buf));
					//res = recvfrom(sockfd_recv, select_recv_buf, 512, 0, NULL, NULL);
					//printf("recvfrom  date !!!!!!!!!!\n ");

					
		#if 0
				int www;
				for(www = 0; www<res ; www++)
					printf("%02x",select_recv_buf[www]);
				printf("\n");
		#endif
					//if(res <= 0)
					//{
						//break;
					//}
					timeVal.tv_sec = signal_report_timeout;
					timeVal.tv_usec = 0;

					FD_ZERO(&fdSet);
					FD_SET(sockfd_recv, &fdSet);

					ret = select((int) sockfd_recv + 1, &fdSet,NULL, NULL, &timeVal);
					if(ret == 0)                     //��ʱ
					{
						printf("cha xun ::select timeout::xiang wei shu ju jie gou \n");
						close(sockfd_recv);
						return -1;
					}
					if(ret)                //������
					{
						res = recvfrom(sockfd_recv, select_recv_buf, 512, 0, NULL, NULL);
						if(res <= 0)
						{		
							close(sockfd_recv);
							return -1;
						}		
						else
						{
							if(select_recv_buf[12+ 1] !=0  && select_recv_buf[12+ 1] <= 24)
							{
								for(j = 0; j < 87+ 1; j++)
								{
									set_14_date_select[select_recv_buf[11]+1][j] = select_recv_buf[j];
								}
							}
						
						}
					}
					i++;
				
					if(i == 24)
					{
						break;
					}


				}
				
				if(i  !=  24)
				{
					close(sockfd_recv);
					return -1;
				}

				
				for(i = 1 ; i <= 24; i++)
				{
					if(set_14_date_select[i][13] != 0)
					{
						printf("***********************\n");
						for(j = 0; j < 87+ 1; j++)
						{
							printf("%02x",set_14_date_select[i][j] );
							
						}
						printf("\n");
						printf("***********************\n");
					}
					
				}
				
#if 1			
				sprintf(sql,"delete from  SHIELD_STAGE_INFO  where signal_id = %d " ,buf[7] * 256 + buf[8] );
				printf("%s\n",sql);
				st->setSQL(sql);
				st->executeUpdate();

				for(i = 1 ; i <= 24; i++)
				{
					if(set_14_date_select[i][12+ 1] != 0   )
					{
						int m_m;

						if(check_buf(set_14_date_select[i]) != 0)
						{
							continue;
						}
						
						bzero(linshi_buf_1,sizeof(linshi_buf_1));
						for(j = 0; j < 13 ;j ++)
						{
							for(m_m = 0; m_m < 8;m_m++)
							{
								if((set_14_date_select[i][15 + 1+  j] & (0x80 >> m_m)) != 0)
								{	
									sprintf(linshi_buf_lin,"%d," ,j * 8 + m_m + 1);
									strcat(linshi_buf_1,linshi_buf_lin);
								}
							}
						}
						
#if 1
						bzero(linshi_buf_2,sizeof(linshi_buf_2));
						for(j = 0; j < 13 ;j ++)
						{
							for(m_m = 0; m_m < 8;m_m++)
							{
								if((set_14_date_select[i][33 + 1+  j] & (0x80 >> m_m)) != 0)
								{	
									sprintf(linshi_buf_lin,"%d," ,j * 8 + m_m + 1);
									strcat(linshi_buf_2,linshi_buf_lin);
								}
							}
						}

						bzero(linshi_buf_3,sizeof(linshi_buf_3));
						for(j = 0; j < 13 ;j ++)
						{
							for(m_m = 0; m_m < 8;m_m++)
							{
								if((set_14_date_select[i][51+ 1+  j] & (0x80 >> m_m)) != 0)
								{	
									sprintf(linshi_buf_lin,"%d," ,j * 8 + m_m + 1);
									strcat(linshi_buf_3,linshi_buf_lin);
								}
							}
						}

						bzero(linshi_buf_4,sizeof(linshi_buf_4));
						for(j = 0; j < 13 ;j ++)
						{
							for(m_m = 0; m_m < 8;m_m++)
							{
								if((set_14_date_select[i][69+ 1 +  j] & (0x80 >> m_m)) != 0)
								{	
									sprintf(linshi_buf_lin,"%d," ,j * 8 + m_m + 1);
									strcat(linshi_buf_4,linshi_buf_lin);
								}
							}
						}
#endif


						sprintf(sql,"insert into  SHIELD_STAGE_INFO(signal_id,SHIELD_ID,STAGE1_ID,STAGE1_TYPE,STAGE1_OCC_DETECTOR,STAGE1_OCC_VALUE,STAGE1_OCC_CHECK, STAGE1_OCC_LONG,STAGE2_ID,STAGE2_TYPE,STAGE2_OCC_DETECTOR,STAGE2_OCC_VALUE,STAGE2_OCC_CHECK, STAGE2_OCC_LONG,STAGE3_ID,STAGE3_TYPE,STAGE3_OCC_DETECTOR,STAGE3_OCC_VALUE,STAGE3_OCC_CHECK, STAGE3_OCC_LONG ,STAGE4_ID,STAGE4_TYPE,STAGE4_OCC_DETECTOR,STAGE4_OCC_VALUE,STAGE4_OCC_CHECK, STAGE4_OCC_LONG  )  values(%d,%d, %d,%d,'%s',%d,%d,%d,  %d,%d,'%s',%d,%d,%d,  %d,%d,'%s',%d,%d,%d,%d,%d,'%s',%d,%d,%d)" 
								,buf[7] * 256 + buf[8] ,set_14_date_select[i][11] + 1,  
								set_14_date_select[i][13+ 1],set_14_date_select[i][14+ 1],linshi_buf_1,set_14_date_select[i][28+ 1],set_14_date_select[i][29+ 1],set_14_date_select[i][30+ 1],
								set_14_date_select[i][31+ 1],set_14_date_select[i][32+ 1],linshi_buf_2,set_14_date_select[i][46+ 1],set_14_date_select[i][47+ 1],set_14_date_select[i][48+ 1],
								set_14_date_select[i][49+ 1],set_14_date_select[i][50+ 1],linshi_buf_3,set_14_date_select[i][64+ 1],set_14_date_select[i][65+ 1],set_14_date_select[i][66+ 1],
								set_14_date_select[i][67+ 1],set_14_date_select[i][68+ 1],linshi_buf_4,set_14_date_select[i][82+ 1],set_14_date_select[i][83+ 1],set_14_date_select[i][84+ 1]);
							printf("%s\n",sql);
							st->setSQL(sql);
							st->executeUpdate();	

#if 0

						try
						{
							sprintf(sql,"insert into  SHIELD_STAGE_INFO(signal_id,SHIELD_ID,STAGE1_ID,STAGE1_TYPE,STAGE1_OCC_DETECTOR,STAGE1_OCC_VALUE,STAGE1_OCC_CHECK, STAGE1_OCC_LONG,STAGE2_ID,STAGE2_TYPE,STAGE2_OCC_DETECTOR,STAGE2_OCC_VALUE,STAGE2_OCC_CHECK, STAGE2_OCC_LONG,STAGE3_ID,STAGE3_TYPE,STAGE3_OCC_DETECTOR,STAGE3_OCC_VALUE,STAGE3_OCC_CHECK, STAGE3_OCC_LONG ,STAGE4_ID,STAGE4_TYPE,STAGE4_OCC_DETECTOR,STAGE4_OCC_VALUE,STAGE4_OCC_CHECK, STAGE4_OCC_LONG  )  values(%d,%d, %d,%d,'%s',%d,%d,%d,  %d,%d,'%s',%d,%d,%d,  %d,%d,'%s',%d,%d,%d,  %d,%d,'%s',%d,%d,%d)" 
								,buf[7] * 256 + buf[8] ,set_14_date_select[i][12+ 1],  
								set_14_date_select[i][13+ 1],set_14_date_select[i][14+ 1],linshi_buf_1,set_14_date_select[i][28+ 1],set_14_date_select[i][29+ 1],set_14_date_select[i][30+ 1],
								set_14_date_select[i][31+ 1],set_14_date_select[i][32+ 1],linshi_buf_2,set_14_date_select[i][46+ 1],set_14_date_select[i][47+ 1],set_14_date_select[i][48+ 1],
								set_14_date_select[i][49+ 1],set_14_date_select[i][50+ 1],linshi_buf_3,set_14_date_select[i][64+ 1],set_14_date_select[i][65+ 1],set_14_date_select[i][66+ 1],
								set_14_date_select[i][67+ 1],set_14_date_select[i][68+ 1],linshi_buf_4,set_14_date_select[i][82+ 1],set_14_date_select[i][83+ 1],set_14_date_select[i][84+ 1]);
							printf("%s\n",sql);
							st->setSQL(sql);
							st->executeUpdate();
						}
						catch (SQLException& ex)
						{
							
							close(sockfd_recv);
							return -1;
						}
#endif		
					}
		
						
				}
	#endif	
				close(sockfd_recv);
				return 0;

			}
			/* �׶����β�ѯ*/
			else if(buf[10] == 0x0d)
			{
				unsigned char set_0d_date_select[129][97+ 1+10];
				char  linshi_buf_1[80];
				char  linshi_buf_2[80];
				 char  linshi_buf_3[80];
				char  linshi_buf_lin[80];
				struct timeval 	timeVal;
				fd_set  fdSet;
				int ret;
				sendto(sockfd_recv, select_buf, sizeof(select_buf), 0, (struct sockaddr*)&signal_addr, sizeof(signal_addr));
				i = 0;

				time_t now_2, now_1 ;

				now_1 = time(NULL);
				while(1)
				{
					bzero(select_recv_buf,sizeof(select_recv_buf));
					//res = recvfrom(sockfd_recv, select_recv_buf, 512, 0, NULL, NULL);
			//		printf("recvfrom  date !!!!!!!!!!\n ");
					//if(res <= 0)
					//{
					//	break;
					//}
					timeVal.tv_sec = signal_report_timeout;
					timeVal.tv_usec = 0;

					FD_ZERO(&fdSet);
					FD_SET(sockfd_recv, &fdSet);

					ret = select((int) sockfd_recv + 1, &fdSet,NULL, NULL, &timeVal);
					if(ret == 0)                     //��ʱ
					{
						printf("cha xun ::select timeout::xiang wei shu ju jie gou \n");
						close(sockfd_recv);
						return -1;
					}
					if(ret)                //������
					{
						res = recvfrom(sockfd_recv, select_recv_buf, 512, 0, NULL, NULL);
						if(res <= 0)
						{		
							close(sockfd_recv);
							return -1;
						}	
						else
						{
							if(select_recv_buf[12+ 1] !=0 &&  select_recv_buf[12+ 1] <=128)
							{
								for(j = 0; j < 97+ 1; j++)
								{
									set_0d_date_select[select_recv_buf[12+ 1]][j] = select_recv_buf[j];
								}
							}
						}
					}
					i++;
				
					if(i == 128)
					{
						break;
					}


				}
				
				if(i  !=  128)
				{
					close(sockfd_recv);
					return -1;
				}

				sprintf(sql,"delete from  DIAODU_INFO  where signal_id = %d  and FUNCTION_TYPE = 2" ,buf[7] * 256 + buf[8] );
			//	printf("%s\n",sql);
				st->setSQL(sql);
				st->executeUpdate();

				sprintf(sql,"delete from  SJBSDSZ_INFO  where signal_id = %d  and FUNCTION_TYPE = 2" ,buf[7] * 256 + buf[8] );
			//	printf("%s\n",sql);
				st->setSQL(sql);
				st->executeUpdate();	

				sprintf(sql,"delete from  TIME_INFO  where signal_id = %d and FUNCTION_TYPE = 2 " ,buf[7] * 256 + buf[8] );
			//	printf("%s\n",sql);
				st->setSQL(sql);
				st->executeUpdate();

				
		

			
				
				

				for(i = 1 ; i <= 128; i++)
				{
					if(set_0d_date_select[i][12+ 1] != 0 )
					{
						int mm;


					
						bzero(linshi_buf_1,sizeof(linshi_buf_1));
						for(mm = 0; mm < 8;mm++)
						{
							
							if((set_0d_date_select[i][15+ 1] & (0x01 << mm)) != 0)
							{	
								if(mm == 0)
								{
									sprintf(linshi_buf_lin,"7," );	
								}
								else
								{
									sprintf(linshi_buf_lin,"%d," , mm );
								}
								
								strcat(linshi_buf_1,linshi_buf_lin);
							}
						}
					
						
						bzero(linshi_buf_2,sizeof(linshi_buf_2));
						for(j = 0; j < 2 ;j ++)
						{
							for(mm = 0; mm < 8;mm++)
							{
								if((set_0d_date_select[i][17 + 1- j] & (0x01 << mm)) != 0)
								{
									
									sprintf(linshi_buf_lin,"%d," ,j * 8 + mm );
									strcat(linshi_buf_2,linshi_buf_lin);
								//	sprintf(linshi_buf_2,"%s%d,",linshi_buf_2,j * 8 + m + 1);
								}
							}
						}
						
						bzero(linshi_buf_3,sizeof(linshi_buf_3));
						for(j = 0; j < 4 ;j ++)
						{
							for(mm = 0; mm < 8;mm++)
							{
								if((set_0d_date_select[i][21 + 1- j] & (0x01 << mm)) != 0)
								{
									sprintf(linshi_buf_lin,"%d," ,j * 8 + mm );
									strcat(linshi_buf_3,linshi_buf_lin);
								//	sprintf(linshi_buf_2,"%s%d,",linshi_buf_2,j * 8 + m + 1);
								}
							}
						}

#if 0					
						
					if(set_06_date_slelect[i][12 + 1] != 0 )
					{
						
						sprintf(sql," insert into TIME_INFO(signal_id,Time_id,Unit_id,FUNCTION_TYPE) values(%d,%d,(select Unit_id from UNIT_INFO where signal_id = %d ),1)" ,
						buf[7] * 256 + buf[8],set_06_date_slelect[i][12+ 1],buf[7] * 256 + buf[8]);
						printf("%s\n",sql);
						st->setSQL(sql);
						st->executeUpdate();

						
						for(j = 0 ; j <set_06_date_slelect[i][13 + 1];j++ )
						{
							sprintf(sql," insert into SJBSDSZ_INFO(signal_id,Time_id,Time_set_number,Hour,Minute,Yunxing_mode,Fangan,FUNCTION_TYPE) values( %d,%d,%d,%d,%d,%d,%d,1)" ,
							buf[7] * 256 + buf[8],set_06_date_slelect[i][12+ 1], j +1,set_06_date_slelect[i][14 + 1+ j * 4] ,set_06_date_slelect[i][15 + 1+ j * 4] ,set_06_date_slelect[i][16+ 1 + j * 4] ,set_06_date_slelect[i][17 + 1+ j * 4]);
						
							printf("%s\n",sql);
							st->setSQL(sql);
							st->executeUpdate();
						}
					}		
#endif			
						try
						{
							sprintf(sql," insert into TIME_INFO(signal_id,Time_id,Unit_id,FUNCTION_TYPE) values(%d,%d,(select Unit_id from UNIT_INFO where signal_id = %d ),2)" ,
							buf[7] * 256 + buf[8],set_0d_date_select[i][12+ 1],buf[7] * 256 + buf[8]);
					//		printf("%s\n",sql);
							st->setSQL(sql);
							st->executeUpdate();

							sprintf(sql," insert into DIAODU_INFO(unit_id,signal_id,Ddb_id,Ddb_type,Ddb_priority,Week_ddb_type,Date_ddb_type_y ,Date_ddb_type_r, Time_table_number_1,FUNCTION_TYPE) values((select unit_id from unit_info where signal_id = %d),%d,%d,%d,%d,'%s','%s','%s',%d,2)" ,
							buf[7] * 256 + buf[8],buf[7] * 256 + buf[8],set_0d_date_select[i][12+ 1],set_0d_date_select[i][13+ 1],set_0d_date_select[i][14+ 1],linshi_buf_1,linshi_buf_2,linshi_buf_3,set_0d_date_select[i][12+ 1]);
					//		printf("%s\n",sql);
							st->setSQL(sql);
							st->executeUpdate();

							

							for(j = 0; j < set_0d_date_select[i][22+ 1] ; j++)
							{
								sprintf(sql," insert into SJBSDSZ_INFO(signal_id,Time_id,Time_set_number,Hour,Minute,SHIELD_ID,FUNCTION_TYPE) values( %d,%d,%d,%d,%d,%d,2)" ,
								buf[7] * 256 + buf[8],set_0d_date_select[i][12+ 1], j +1,set_0d_date_select[i][23 + 1+ j *3],set_0d_date_select[i][24+ 1 + j*3],set_0d_date_select[i][25+ 1 + j*3]);
							
							//	printf("%s\n",sql);
								st->setSQL(sql);
								st->executeUpdate();
#if 0						
								sprintf(sql," update SJBSDSZ_INFO  set Hour = %d,Minute = %d ,SHIELD_ID= %d  where signal_id = %d and Time_id = (select Time_table_number_1 from DIAODU_INFO where signal_id = %d and Ddb_id = %d) and Time_set_number = %d ",
								set_0d_date_select[i][23 + 1+ j *3],set_0d_date_select[i][24+ 1 + j*3],set_0d_date_select[i][25+ 1 + j*3], buf[7] * 256 + buf[8],buf[7] * 256 + buf[8],set_0d_date_select[i][12+ 1],j+1);
						
								printf("%s\n",sql);
								st->setSQL(sql);
								st->executeUpdate();
#endif
							}
						}
						catch (SQLException& ex)
						{
							close(sockfd_recv);
							return -1;
						}

						
					}
				}

				now_2 = time(NULL);

				printf("time ===== %d\n",(int)(now_2 - now_1) );
				close(sockfd_recv);
				return 0;
			}
			/* �ɱ��־���Ͳ�ѯ*/
			else if(buf[10] == 0x15)
			{	
				unsigned char set_15_date_lselect[35][42+10];

				struct timeval 	timeVal;
				fd_set  fdSet;
				int ret;
								
		#if 0
				int www;
				for(www = 0; www<16 ; www++)
					printf("%02x",select_buf[www]);
				printf("\n");
		#endif


		

				printf("ip  an  port  [%s:%d] :\n", inet_ntoa(signal_addr.sin_addr), ntohs(signal_addr.sin_port));
				int lin_long = sendto(sockfd_recv, select_buf, sizeof(select_buf), 0, (struct sockaddr*)&signal_addr, sizeof(signal_addr));
			//	sendto(sockfd_recv, select_buf, sizeof(select_buf), 0, (struct sockaddr*)&signal_addr, sizeof(signal_addr));
			//	sendto(sockfd_recv, select_buf, sizeof(select_buf), 0, (struct sockaddr*)&signal_addr, sizeof(signal_addr));
				printf("ip  an  port  [%s:%d] :\n", inet_ntoa(signal_addr.sin_addr), ntohs(signal_addr.sin_port));
				printf("lin_lon= %d\n",lin_long);
				i = 0;
				while(1)
				{
					bzero(select_recv_buf,sizeof(select_recv_buf));
					//res = recvfrom(sockfd_recv, select_recv_buf, 512, 0, NULL, NULL);
					
					
					
					//if(res <= 0)
					//{
						//printf("recvfrom  date  time out!\n ");
						//break;
					//}
					timeVal.tv_sec = signal_report_timeout;
					timeVal.tv_usec = 0;

					FD_ZERO(&fdSet);
					FD_SET(sockfd_recv, &fdSet);

					ret = select((int) sockfd_recv + 1, &fdSet,NULL, NULL, &timeVal);
					if(ret == 0)                     //��ʱ
					{
						printf("cha xun ::select timeout::ke bian biao zhi lei xing \n");
						close(sockfd_recv);
						return -1;
					}
					if(ret)                //������
					{
						res = recvfrom(sockfd_recv, select_recv_buf, 512, 0, NULL, NULL);
						if(res <= 0)
						{		
							close(sockfd_recv);
							return -1;
						}	
						else
						{
							if(select_recv_buf[12+ 1] !=0 && select_recv_buf[12+ 1] <= 24)
							{
								for(j = 0; j < 31+ 1; j++)
								{
									set_15_date_lselect[select_recv_buf[12+ 1]][j] = select_recv_buf[j];
								}
							}
					
						}
					}
					i++;
				
					if(i == 24)
					{
						break;
					}


				}
				
				if(i  !=  24)
				{
					close(sockfd_recv);
					return -1;
				}

				sprintf(sql,"delete from  VARY_MARK_INFO  where signal_id = %d " ,buf[7] * 256 + buf[8] );
				printf("%s\n",sql);
				st->setSQL(sql);
				st->executeUpdate();

				for(i = 1 ; i <= 24; i++)
				{
					if(set_15_date_lselect[i][12+ 1] != 0 )
					{
						
					
						sprintf(sql,"insert into  VARY_MARK_INFO(id,signal_id,VARY_MARK_ID,CLINK1_ID,CLINK1_TYPE,CLINK1_RED,CLINK1_GREEN ,CLINK2_ID,CLINK2_TYPE,CLINK2_RED,CLINK2_GREEN ,CLINK3_ID,CLINK3_TYPE,CLINK3_RED,CLINK3_GREEN ,CLINK4_ID,CLINK4_TYPE,CLINK4_RED,CLINK4_GREEN )  values(HIBERNATE_SEQUENCE.Nextval,%d,%d,  %d,%d,%d,%d,  %d,%d,%d,%d,   %d,%d,%d,%d,   %d,%d,%d,%d )" ,
							buf[7] * 256 + buf[8],i,set_15_date_lselect[i][13+ 1],set_15_date_lselect[i][14+ 1],set_15_date_lselect[i][15+ 1],set_15_date_lselect[i][16+ 1],
							set_15_date_lselect[i][17+ 1],set_15_date_lselect[i][18+ 1],set_15_date_lselect[i][19+ 1],set_15_date_lselect[i][20+ 1],
							set_15_date_lselect[i][21+ 1],set_15_date_lselect[i][22+ 1],set_15_date_lselect[i][23+ 1],set_15_date_lselect[i][24+ 1],
							set_15_date_lselect[i][25+ 1],set_15_date_lselect[i][26+ 1],set_15_date_lselect[i][27+ 1],set_15_date_lselect[i][28+ 1]);
						printf("%s\n",sql);
						st->setSQL(sql);
						st->executeUpdate();
						
					}
				}
				
				
				close(sockfd_recv);
				return 0;

				
			}
			/*�ɱ��־��ѯ*/
			else if(buf[10] == 0x0e)
			{
				unsigned char set_0e_date_select[129][97+ 1+10];
				char  linshi_buf_1[80];
				char  linshi_buf_2[80];
				 char  linshi_buf_3[80];
				char  linshi_buf_lin[80];
				struct timeval 	timeVal;
				fd_set  fdSet;
				int ret;
				sendto(sockfd_recv, select_buf, sizeof(select_buf), 0, (struct sockaddr*)&signal_addr, sizeof(signal_addr));
				i = 0;

				time_t now_2, now_1 ;

				now_1 = time(NULL);
				while(1)
				{
			//		printf("i = %d  select_recv_buf[12+ 1] = %d\n",i+1,select_recv_buf[12+ 1]);
					bzero(select_recv_buf,sizeof(select_recv_buf));
					//res = recvfrom(sockfd_recv, select_recv_buf, 512, 0, NULL, NULL);
				//	printf("recvfrom  date !!!!!!!!!!\n ");
					//if(res <= 0)
					//{
					//	break;
					//}
					timeVal.tv_sec = signal_report_timeout;
					timeVal.tv_usec = 0;

					FD_ZERO(&fdSet);
					FD_SET(sockfd_recv, &fdSet);

					ret = select((int) sockfd_recv + 1, &fdSet,NULL, NULL, &timeVal);
					if(ret == 0)                     //��ʱ
					{
						printf("cha xun ::select timeout::ke bian biao zhi  \n");
						close(sockfd_recv);
						return -1;
					}
					if(ret)                //������
					{
						res = recvfrom(sockfd_recv, select_recv_buf, 512, 0, NULL, NULL);
						if(res <= 0)
						{		
							close(sockfd_recv);
							return -1;
						}	
						else
						{
							if(select_recv_buf[12+ 1] !=0 && select_recv_buf[12+ 1] <= 128)
							{
								for(j = 0; j < 97+ 1; j++)
								{
									set_0e_date_select[select_recv_buf[12+ 1]][j] = select_recv_buf[j];
								}
							}
						}
					}
					i++;
				
					if(i == 128)
					{
						break;
					}


				}
				
				if(i  !=  128)
				{
					close(sockfd_recv);
					return -1;
				}

				sprintf(sql,"delete from  DIAODU_INFO  where signal_id = %d  and FUNCTION_TYPE = 3" ,buf[7] * 256 + buf[8] );
				printf("%s\n",sql);
				st->setSQL(sql);
				st->executeUpdate();

				sprintf(sql,"delete from  SJBSDSZ_INFO  where signal_id = %d and FUNCTION_TYPE = 3" ,buf[7] * 256 + buf[8] );
				printf("%s\n",sql);
				st->setSQL(sql);
				st->executeUpdate();	

				sprintf(sql,"delete from  TIME_INFO  where signal_id = %d and FUNCTION_TYPE = 3 " ,buf[7] * 256 + buf[8] );
				printf("%s\n",sql);
				st->setSQL(sql);
				st->executeUpdate();
				



				for(i = 1 ; i <= 128; i++)
				{
				//	printf("i = %d  set_0e_date_select[i][13] = %d\n ",i,set_0e_date_select[i][12+ 1]);
					if(set_0e_date_select[i][12+ 1] != 0 )
					{
						int mm;


					
						bzero(linshi_buf_1,sizeof(linshi_buf_1));
						for(mm = 0; mm < 8;mm++)
						{
							
							if((set_0e_date_select[i][15+ 1] & (0x01 << mm)) != 0)
							{	
								if(mm == 0)
								{
									sprintf(linshi_buf_lin,"7," );	
								}
								else
								{
									sprintf(linshi_buf_lin,"%d," , mm );
								}
								
								strcat(linshi_buf_1,linshi_buf_lin);
							}
						}
					
						
						bzero(linshi_buf_2,sizeof(linshi_buf_2));
						for(j = 0; j < 2 ;j ++)
						{
							for(mm = 0; mm < 8;mm++)
							{
								if((set_0e_date_select[i][17+ 1 - j] & (0x01 << mm)) != 0)
								{
									sprintf(linshi_buf_lin,"%d," ,j * 8 + mm );
									strcat(linshi_buf_2,linshi_buf_lin);
								//	sprintf(linshi_buf_2,"%s%d,",linshi_buf_2,j * 8 + m + 1);
								}
							}
						}
						
						bzero(linshi_buf_3,sizeof(linshi_buf_3));
						for(j = 0; j < 4 ;j ++)
						{
							for(mm = 0; mm < 8;mm++)
							{
								if((set_0e_date_select[i][21+ 1 - j] & (0x01 << mm)) != 0)
								{
									sprintf(linshi_buf_lin,"%d," ,j * 8 + mm );
									strcat(linshi_buf_3,linshi_buf_lin);
								//	sprintf(linshi_buf_2,"%s%d,",linshi_buf_2,j * 8 + m + 1);
								}
							}
						}

						try
						{
							sprintf(sql," insert into TIME_INFO(signal_id,Time_id,Unit_id,FUNCTION_TYPE) values(%d,%d,(select Unit_id from UNIT_INFO where signal_id = %d ),3)" ,
							buf[7] * 256 + buf[8],set_0e_date_select[i][12+ 1],buf[7] * 256 + buf[8]);
						//	printf("%s\n",sql);
							st->setSQL(sql);
							st->executeUpdate();

							sprintf(sql," insert into DIAODU_INFO(unit_id,signal_id,Ddb_id,Ddb_type,Ddb_priority,Week_ddb_type,Date_ddb_type_y ,Date_ddb_type_r, Time_table_number_1,FUNCTION_TYPE) values((select unit_id from unit_info where signal_id = %d),%d,%d,%d,%d,'%s','%s','%s',%d,3)" ,
							buf[7] * 256 + buf[8],buf[7] * 256 + buf[8],set_0e_date_select[i][12+ 1],set_0e_date_select[i][13+ 1],set_0e_date_select[i][14+ 1],linshi_buf_1,linshi_buf_2,linshi_buf_3,set_0e_date_select[i][12+ 1]);
						//	printf("%s\n",sql);
							st->setSQL(sql);
							st->executeUpdate();
							
					

							for(j = 0; j < set_0e_date_select[i][22+ 1] ; j++)
							{

								sprintf(sql," insert into SJBSDSZ_INFO(signal_id,Time_id,Time_set_number,Hour,Minute,VARY_MARK_ID,FUNCTION_TYPE) values( %d,%d,%d,%d,%d,%d,3)" ,
								buf[7] * 256 + buf[8],set_0e_date_select[i][12+ 1], j +1,set_0e_date_select[i][23 + 1+ j *3],set_0e_date_select[i][24+ 1 + j*3],set_0e_date_select[i][25+ 1 + j*3]);
							
							//	printf("%s\n",sql);
								st->setSQL(sql);
								st->executeUpdate();
								
							#if 0	
								sprintf(sql," update SJBSDSZ_INFO  set Hour = %d,Minute = %d ,VARY_MARK_ID= %d  where signal_id = %d and Time_id = (select Time_table_number_1 from DIAODU_INFO where signal_id = %d and Ddb_id = %d) and Time_set_number = %d ",
								set_0e_date_select[i][23+ 1 + j *3],set_0e_date_select[i][24+ 1 + j*3],set_0e_date_select[i][25+ 1 + j*3], buf[7] * 256 + buf[8],buf[7] * 256 + buf[8],set_0e_date_select[i][12+ 1],j+1);
						
								printf("%s\n",sql);
								st->setSQL(sql);
								st->executeUpdate();
							#endif	
							}
						}
						catch (SQLException& ex)
						{
							close(sockfd_recv);
							return -1;
						}

						
					}
				}

				now_2 = time(NULL);

				printf("shi jian  xiao hao = %d\n",(int)(now_2 - now_1));
				close(sockfd_recv);
				return 0;
			}
			/* �������ѯ*/
			else if(buf[10] == 0x0f)
			{
				unsigned char set_0f_date_select[105][23+ 1+10];
				struct timeval 	timeVal;
				fd_set  fdSet;
				int ret;
				sendto(sockfd_recv, select_buf, sizeof(select_buf), 0, (struct sockaddr*)&signal_addr, sizeof(signal_addr));
				i = 0;
				while(1)
				{
					bzero(select_recv_buf,sizeof(select_recv_buf));
					//res = recvfrom(sockfd_recv, select_recv_buf, 512, 0, NULL, NULL);
					//printf("recvfrom  date !!!!!!!!!!\n ");
					//if(res <= 0)
					//{
					//	break;
					//}
					timeVal.tv_sec = signal_report_timeout;
					timeVal.tv_usec = 0;

					FD_ZERO(&fdSet);
					FD_SET(sockfd_recv, &fdSet);

					ret = select((int) sockfd_recv + 1, &fdSet,NULL, NULL, &timeVal);
					if(ret == 0)                     //��ʱ
					{
						printf("cha xun ::select timeout::jian ce qi \n");
						close(sockfd_recv);
						return -1;
					}
					if(ret)                //������
					{
						res = recvfrom(sockfd_recv, select_recv_buf, 512, 0, NULL, NULL);
						if(res <= 0)
						{		
							close(sockfd_recv);
							return -1;
						}	
						else
						{
							if(select_recv_buf[12+ 1] !=0 && select_recv_buf[12+ 1] <= 104)
							{
								for(j = 0; j < 23+ 1; j++)
								{
									set_0f_date_select[select_recv_buf[12+ 1]][j] = select_recv_buf[j];
								}
							}
						}
					}
					i++;
				
					if(i == 104)
					{
						break;
					}


				}
				
				if(i  !=  104)
				{
					close(sockfd_recv);
					return -1;
				}

				sprintf(sql,"delete from  DETECTOR_INFO  where signal_id = %d " ,buf[7] * 256 + buf[8] );
				printf("%s\n",sql);
				st->setSQL(sql);
				st->executeUpdate();

				for(i = 1 ; i <= 104; i++)
				{
					if(set_0f_date_select[i][12+ 1] != 0 )
					{
						sprintf(sql,"insert into  DETECTOR_INFO(signal_id,Detector_id,Detector_type,Flow_week,Occ_week,Warning_week,DIRECTION )  values(%d,%d, %d,%d,%d,%d ,%d )" ,
							buf[7] * 256 + buf[8],set_0f_date_select[i][12+ 1],set_0f_date_select[i][13+ 1] ,(set_0f_date_select[i][14+ 1] * 256 + set_0f_date_select[i][15+ 1]) ,
							(set_0f_date_select[i][16+ 1] * 256 + set_0f_date_select[i][17+ 1]) ,(set_0f_date_select[i][18+ 1] * 256 + set_0f_date_select[i][19+ 1]) / 10 ,set_0f_date_select[i][20+ 1]);
						printf("%s\n",sql);
						st->setSQL(sql);
						st->executeUpdate();
					}
				}
				close(sockfd_recv);
				return 0;
				
			}
			/* ��λ����Լ����ѯ*/
			else if(buf[10] == 0x10)
			{
				unsigned char set_10_date_select[270+ 1+10];



				struct timeval 	timeVal;
				fd_set  fdSet;
				int ret;
				sendto(sockfd_recv, select_buf, sizeof(select_buf), 0, (struct sockaddr*)&signal_addr, sizeof(signal_addr));
				bzero(select_recv_buf,sizeof(select_recv_buf));
				//res = recvfrom(sockfd_recv, select_recv_buf, 512, 0, NULL, NULL);
				//printf("recvfrom  date !!!!!!!!!!\n ");
				//if(res <= 0)
				//{
				//	close(sockfd_recv);
				//	return -1;
				//}
				timeVal.tv_sec = signal_report_timeout;
					timeVal.tv_usec = 0;

					FD_ZERO(&fdSet);
					FD_SET(sockfd_recv, &fdSet);

					ret = select((int) sockfd_recv + 1, &fdSet,NULL, NULL, &timeVal);
					if(ret == 0)                     //��ʱ
					{
						printf("cha xun ::select timeout::xiang wei guo du yue shu \n");
						close(sockfd_recv);
						return -1;
					}
					if(ret)                //������
					{
						res = recvfrom(sockfd_recv, select_recv_buf, 512, 0, NULL, NULL);
						if(res <= 0)
						{		
							close(sockfd_recv);
							return -1;
						}	
						else
						{
							for(j = 0; j < 270+ 1; j++)
							{
								set_10_date_select[j] = select_recv_buf[j];
							}
						}
					}

					int num_stage;
					sprintf(sql,"select  count(*) from  clink_info where signal_id = %d  " ,buf[7] * 256 + buf[8] );
					printf("%s\n",sql);
					st->setSQL(sql);
					rs = st->executeQuery();
					while(rs->next())
					{
						num_stage = rs->getInt(1);
					}
					
					sprintf(sql,"delete from  TRANSITION_CLINK  where signal_id = %d " ,buf[7] * 256 + buf[8] );
					printf("%s\n",sql);
					st->setSQL(sql);
					st->executeUpdate();
					
					for(i = 0 ; i < num_stage; i++)
					{
					
						for(j = 0; j < num_stage;j++)
						{

							sprintf(sql,"insert into   TRANSITION_CLINK(signal_id,CLINK_ID_ROW,CLINK_ID_COLUMN,GUODU_VALUE )  values(%d,%d,%d,%d) " ,
							buf[7] * 256 + buf[8] ,i+1, j+1 ,set_10_date_select[12 + 1 + i * 16 + j]);
							printf("%s\n",sql);
							st->setSQL(sql);
							st->executeUpdate();
						}
				
					}

				close(sockfd_recv);
				return 0;
			//	sprintf(sql,"select CLINK_ID_ROW, CLINK_ID_COLUMN,GUODU_VALUE  from TRANSITION_CLINK    where signal_id = %d  order by CLINK_ID_ROW " ,buf[7] * 256 + buf[8] );
			
			}

			//itc100 ��ѯ 
			//itc100,����������Ͳ�ѯ
			else if(buf[10] == 0x20)
			{
				struct timeval 	timeVal;
				fd_set  fdSet;
				int ret;
				sendto(sockfd_recv, select_buf, sizeof(select_buf), 0, (struct sockaddr*)&signal_addr, sizeof(signal_addr));
				bzero(select_recv_buf,sizeof(select_recv_buf));
				//res = recvfrom(sockfd_recv, select_recv_buf, 512, 0, NULL, NULL);
				//printf("recvfrom  date !!!!!!!!!!\n ");
				//if(res <= 0)
				//{
				//	close(sockfd_recv);
				//	return -1;
				//}
				timeVal.tv_sec = signal_report_timeout;
					timeVal.tv_usec = 0;

					FD_ZERO(&fdSet);
					FD_SET(sockfd_recv, &fdSet);

					ret = select((int) sockfd_recv + 1, &fdSet,NULL, NULL, &timeVal);
					if(ret == 0)                     //��ʱ
					{
						printf("cha xun ::select timeout::xiang wei shu ju jie gou \n");
						close(sockfd_recv);
						return -1;
					}
					if(ret)                //������
					{
						res = recvfrom(sockfd_recv, select_recv_buf, 512, 0, NULL, NULL);
						if(res <= 0)
						{		
							close(sockfd_recv);
							return -1;
						}	
						else
						{	
						sprintf(sql,"delete  from  ITC_CFG_TRANS_COLOR  where signal_id = %d  " ,buf[7] * 256 + buf[8] );
						printf("%s\n",sql);
						st->setSQL(sql);
						st->executeUpdate();

						try
						{
							for(i = 0; i< 32;i++)
							{
								sprintf(sql,"insert into  ITC_CFG_TRANS_COLOR(signal_id,phase_id,Red_green_1,Red_green_time1,Red_green_2,Red_green_time2,Red_green_3,Red_green_time3,green_red_1,green_red_time1,green_red_2,green_red_time2,green_red_3,green_red_time3) values(%d,%d, '%d', %d,'%d', %d, '%d',%d ,'%d',%d,'%d',%d,'%d',%d)" ,
								buf[7] * 256 + buf[8] ,i+1,select_recv_buf[13+i*6] ,select_recv_buf[14+i*6],select_recv_buf[15+i*6],select_recv_buf[16+i*6],select_recv_buf[17+i*6],select_recv_buf[18+i*6],
								select_recv_buf[205+i*6] ,select_recv_buf[206+i*6],select_recv_buf[207+i*6],select_recv_buf[208+i*6],select_recv_buf[209+i*6],select_recv_buf[210+i*6] );
							
							printf("%s\n",sql);
							st->setSQL(sql);
							st->executeUpdate();
						
							}
						}
						catch (SQLException& ex)
						{	
							close(sockfd_recv);
							return -1;
						}
						}
					}
				close(sockfd_recv);
				return 0;
			}
			
			else if(buf[10] == 0x21)
			{
				struct timeval 	timeVal;
				fd_set  fdSet;
				int ret;
				sendto(sockfd_recv, select_buf, sizeof(select_buf), 0, (struct sockaddr*)&signal_addr, sizeof(signal_addr));
				bzero(select_recv_buf,sizeof(select_recv_buf));
				//res = recvfrom(sockfd_recv, select_recv_buf, 512, 0, NULL, NULL);
			//	printf("recvfrom  date !!!!!!!!!!\n ");
				//if(res <= 0)
			//	{
			//		close(sockfd_recv);
			//		return -1;
			//	}
				timeVal.tv_sec = signal_report_timeout;
					timeVal.tv_usec = 0;

					FD_ZERO(&fdSet);
					FD_SET(sockfd_recv, &fdSet);

					ret = select((int) sockfd_recv + 1, &fdSet,NULL, NULL, &timeVal);
					if(ret == 0)                     //��ʱ
					{
						printf("cha xun ::select timeout::xiang wei shu ju jie gou \n");
						close(sockfd_recv);
						return -1;
					}
					if(ret)                //������
					{
						res = recvfrom(sockfd_recv, select_recv_buf, 512, 0, NULL, NULL);
						if(res <= 0)
						{		
							close(sockfd_recv);
							return -1;
						}	
				else
				{
					sprintf(sql,"delete  from  ITC_CFG_STARTUP_COLOR  where signal_id = %d  " ,buf[7] * 256 + buf[8] );
					printf("%s\n",sql);
					st->setSQL(sql);
					st->executeUpdate();

					try
					{
						for(i = 0; i< 32;i++)
						{
							sprintf(sql,"insert into ITC_CFG_STARTUP_COLOR(signal_id,phase_id,color1,time1,color2,time2,color3,time3) values(%d,%d,'%d',%d,'%d',%d,'%d',%d)  " ,
								buf[7] * 256 + buf[8] ,i + 1,select_recv_buf[13+i*6] ,select_recv_buf[14+i*6] ,select_recv_buf[15+i*6] ,select_recv_buf[16+i*6] ,select_recv_buf[17+i*6] ,select_recv_buf[18+i*6] );
							printf("%s\n",sql);
							st->setSQL(sql);
							st->executeUpdate();

						}
					
					}
					catch (SQLException& ex)
					{
						close(sockfd_recv);
						return -1;
					}
				}
				}    // end:: if(ret)  
				close(sockfd_recv);
				return 0;
			}

			else if(buf[10] == 0x22)
			{
				struct timeval 	timeVal;
				fd_set  fdSet;
				int ret;
				sendto(sockfd_recv, select_buf, sizeof(select_buf), 0, (struct sockaddr*)&signal_addr, sizeof(signal_addr));
				bzero(select_recv_buf,sizeof(select_recv_buf));
				//res = recvfrom(sockfd_recv, select_recv_buf, 512, 0, NULL, NULL);
				//printf("recvfrom  date !!!!!!!!!!\n ");
			//	if(res <= 0)
				//{
				//	close(sockfd_recv);
			//		return -1;
			//	}
				timeVal.tv_sec = signal_report_timeout;
					timeVal.tv_usec = 0;

					FD_ZERO(&fdSet);
					FD_SET(sockfd_recv, &fdSet);

					ret = select((int) sockfd_recv + 1, &fdSet,NULL, NULL, &timeVal);
					if(ret == 0)                     //��ʱ
					{
						printf("cha xun ::select timeout::xiang wei shu ju jie gou \n");
						close(sockfd_recv);
						return -1;
					}
					if(ret)                //������
					{
						res = recvfrom(sockfd_recv, select_recv_buf, 512, 0, NULL, NULL);
						if(res <= 0)
						{		
							close(sockfd_recv);
							return -1;
						}	
				else
				{
					sprintf(sql,"delete  from  ITC_CFG_FLASH  where signal_id = %d  " ,buf[7] * 256 + buf[8] );
					printf("%s\n",sql);
					st->setSQL(sql);
					st->executeUpdate();

					try
					{
						sprintf(sql,"insert into ITC_CFG_FLASH(signal_id,cycle,duty_ratio,fast_cycle,fast_duty_ratio) values(%d,%d,%d,%d,%d)  " ,
								buf[7] * 256 + buf[8] ,select_recv_buf[13] ,select_recv_buf[14] ,select_recv_buf[15] ,select_recv_buf[16] );
						printf("%s\n",sql);
						st->setSQL(sql);
						st->executeUpdate();

					
					}
					catch (SQLException& ex)
					{
						close(sockfd_recv);
						return -1;
					}
				}
				}
				close(sockfd_recv);
				return 0;
			}

			else if(buf[10] == 0x23)
			{
				unsigned char set23_data[32][19];
				struct timeval 	timeVal;
				fd_set  fdSet;
				int ret;
				sendto(sockfd_recv, select_buf, sizeof(select_buf), 0, (struct sockaddr*)&signal_addr, sizeof(signal_addr));
				i = 0;//�յ����ݵļ�����
				while(1)
				{
					bzero(select_recv_buf,sizeof(select_recv_buf));
					//res = recvfrom(sockfd_recv, select_recv_buf, 512, 0, NULL, NULL);
					//printf("recvfrom  date !!!!!!!!!!\n ");
					//if(res <= 0)
					//{
					//	close(sockfd_recv);
					//	return -1;
					//}
					timeVal.tv_sec = signal_report_timeout;
					timeVal.tv_usec = 0;

					FD_ZERO(&fdSet);
					FD_SET(sockfd_recv, &fdSet);

					ret = select((int) sockfd_recv + 1, &fdSet,NULL, NULL, &timeVal);
					if(ret == 0)                     //��ʱ
					{
						printf("cha xun ::select timeout::xiang wei shu ju jie gou \n");
						close(sockfd_recv);
						return -1;
					}
					if(ret)                //������
					{
						res = recvfrom(sockfd_recv, select_recv_buf, 512, 0, NULL, NULL);
						if(res <= 0)
						{		
							close(sockfd_recv);
							return -1;
						}	
						else
						{
							if((select_recv_buf[13]>0 && select_recv_buf[13] < 33) && select_recv_buf[10] == 0x23)
							{
								i++;
								for(j = 0 ;j < 19;j++)
									set23_data[select_recv_buf[13]-1][j] = select_recv_buf[14+j];
							}

							if(i == 32)
								break;//�յ�32�����ݣ��˳�ѭ������
						}
					}
				}

				sprintf(sql,"delete  from  ITC_CFG_PHASE  where signal_id = %d  " ,buf[7] * 256 + buf[8] );
				printf("%s\n",sql);
				st->setSQL(sql);
				st->executeUpdate();
					
				for(i = 0;i <32; i++)
				{
					try
					{
						char detector_apply[200],detector_long[200],detector_temp[4];
						bzero(detector_apply,200);
						bzero(detector_long,200);
						for(j = 0 ; j<4;j++) // ��λ����������4���ֽ�
						{
							int count;
							for(count = 0;count < 8;count++)
							{
								if((set23_data[i][10 + j] & (0x80 >> count) ) !=0 )
								{
									sprintf(detector_temp,"%d,",(count+1+j*8));
									strcat(detector_apply,detector_temp);
								}

								if((set23_data[i][14 + j] & (0x80 >> count) ) !=0 )
								{
									sprintf(detector_temp,"%d,",(count+1+j*8));
									strcat(detector_long,detector_temp);
								}
									
							}
						}
						sprintf(sql,"insert into ITC_CFG_PHASE(signal_id,phase_id,phase_type,green_min,green_max,green_long,detector_apply,detector_long) values(%d,%d,%d,%d,%d,%d,'%s','%s')  " ,
								buf[7] * 256 + buf[8] ,i+1,set23_data[i][0] ,set23_data[i][2]*256+set23_data[i][3] ,set23_data[i][4]*256+set23_data[i][5],set23_data[i][8]*256+set23_data[i][9],
								detector_apply,detector_long);
						printf("%s\n",sql);
						st->setSQL(sql);
						st->executeUpdate();
					
					}
					catch (SQLException& ex)
					{
						close(sockfd_recv);
						return -1;
					}
				}
				close(sockfd_recv);
				return 0;
			}

			else if(buf[10] == 0x25)
			{
				unsigned char set25_data[32][32];
				struct timeval 	timeVal;
				fd_set  fdSet;
				int ret;
				sendto(sockfd_recv, select_buf, sizeof(select_buf), 0, (struct sockaddr*)&signal_addr, sizeof(signal_addr));
				i = 0;//�յ����ݵļ�����
				while(1)
				{
					bzero(select_recv_buf,sizeof(select_recv_buf));
					//res = recvfrom(sockfd_recv, select_recv_buf, 512, 0, NULL, NULL);
				//	printf("recvfrom  date !!!!!!!!!!\n ");
				//	if(res <= 0)
				//	{
				//		close(sockfd_recv);
				//		return -1;
				//	}
					timeVal.tv_sec = signal_report_timeout;
					timeVal.tv_usec = 0;

					FD_ZERO(&fdSet);
					FD_SET(sockfd_recv, &fdSet);

					ret = select((int) sockfd_recv + 1, &fdSet,NULL, NULL, &timeVal);
					if(ret == 0)                     //��ʱ
					{
						printf("cha xun ::select timeout::xiang wei shu ju jie gou \n");
						close(sockfd_recv);
						return -1;
					}
					if(ret)                //������
					{
						res = recvfrom(sockfd_recv, select_recv_buf, 512, 0, NULL, NULL);
						if(res <= 0)
						{		
							close(sockfd_recv);
							return -1;
						}	
						else
						{
							if((select_recv_buf[13]>0 && select_recv_buf[13] < 33) && select_recv_buf[10] == 0x25)
							{
								i++;
								for(j = 0 ;j < 32;j++)
									set25_data[select_recv_buf[13]-1][j] = select_recv_buf[14+j];
							}

							if(i == 32)
								break;//�յ�32�����ݣ�?˳�ѭ�����?
						
						}
					}
				}
				sprintf(sql,"delete  from  ITC_CFG_CONFLICT  where signal_id = %d  " ,buf[7] * 256 + buf[8] );
				printf("%s\n",sql);
				st->setSQL(sql);
				st->executeUpdate();
					
				for(i = 0;i <32; i++)
				{
					try
					{
						char phase_rush[200],phase_temp[4];
						bzero(phase_rush,200);
						for(j = 0 ; j<32;j++) // ��λ����������4���ֽ�
						{
							if((set25_data[i][j] & 0x01 )!=0 )
								{
									sprintf(phase_temp,"%d,",j+1);
									strcat(phase_rush,phase_temp);
								}

						}
						sprintf(sql,"insert into ITC_CFG_CONFLICT(signal_id,phase_id,phase_rush) values(%d,%d,'%s')  " ,
								buf[7] * 256 + buf[8] ,i+1,phase_rush);
						printf("%s\n",sql);
						st->setSQL(sql);
						st->executeUpdate();
					
					}
					catch (SQLException& ex)
					{
						close(sockfd_recv);
						return -1;
					}
				}
				close(sockfd_recv);
				return 0;
			}
			
			//itc100,���������
			else if(buf[10] == 0x26)
			{
				struct timeval 	timeVal;
				fd_set  fdSet;
				int ret;

				sendto(sockfd_recv, select_buf, sizeof(select_buf), 0, (struct sockaddr*)&signal_addr, sizeof(signal_addr));
				bzero(select_recv_buf,sizeof(select_recv_buf));
				//res = recvfrom(sockfd_recv, select_recv_buf, 512, 0, NULL, NULL);
				//printf("recvfrom  date !!!!!!!!!!\n ");
				//if(res <= 0)
				//{
				//	close(sockfd_recv);
				//	return -1;
				//}
				timeVal.tv_sec = signal_report_timeout;
					timeVal.tv_usec = 0;

					FD_ZERO(&fdSet);
					FD_SET(sockfd_recv, &fdSet);

					ret = select((int) sockfd_recv + 1, &fdSet,NULL, NULL, &timeVal);
					if(ret == 0)                     //��ʱ
					{
						printf("cha xun ::select timeout::xiang wei shu ju jie gou \n");
						close(sockfd_recv);
						return -1;
					}
					if(ret)                //������
					{
						res = recvfrom(sockfd_recv, select_recv_buf, 512, 0, NULL, NULL);
						if(res <= 0)
						{		
							close(sockfd_recv);
							return -1;
						}	
						else
						{
							sprintf(sql,"delete  from  ITC_CFG_DETECTOR  where signal_id = %d  " ,buf[7] * 256 + buf[8] );
							printf("%s\n",sql);
							st->setSQL(sql);
							st->executeUpdate();
						}
					

					try
					{
						for(i = 0;i< 32;i++)
						{
							sprintf(sql,"insert into ITC_CFG_DETECTOR(signal_id,detector_id,detector_type,flow_week,occ_week,warning_week) values(%d,%d,%d,%d,%d,%d)  " ,
								buf[7] * 256 + buf[8] ,i+1,select_recv_buf[13+i*7] ,select_recv_buf[14+i*7]*256+ select_recv_buf[15+i*7], select_recv_buf[16+i*7]*256+ select_recv_buf[17+i*7],select_recv_buf[18+i*7]*256+ select_recv_buf[19+i*7]);
							printf("%s\n",sql);
							st->setSQL(sql);
							st->executeUpdate();
						}

					
					}
					catch (SQLException& ex)
					{
						close(sockfd_recv);
						return -1;
					}
				}
				close(sockfd_recv);
				return 0;
			}
			//itc100,�׶β�ѯ
			else if(buf[10] == 0x27)
			{
				unsigned char set27_data[64][17];
				struct timeval 	timeVal;
				fd_set  fdSet;
				int ret;

				sendto(sockfd_recv, select_buf, sizeof(select_buf), 0, (struct sockaddr*)&signal_addr, sizeof(signal_addr));
				printf("fa song cha xun jieduan mingling\n\n");
				i = 0;//�յ����ݵļ�����
				while(1)
				{
					bzero(select_recv_buf,sizeof(select_recv_buf));
					//res = recvfrom(sockfd_recv, select_recv_buf, 512, 0, NULL, NULL);
					//printf("recvfrom  date !!!!!!!!!!\n ");
					//if(res <= 0)
					//{
					//	close(sockfd_recv);
					//	return -1;
					//}
					timeVal.tv_sec = signal_report_timeout;
					timeVal.tv_usec = 0;

					FD_ZERO(&fdSet);
					FD_SET(sockfd_recv, &fdSet);

					ret = select((int) sockfd_recv + 1, &fdSet,NULL, NULL, &timeVal);
					if(ret == 0)                     //��ʱ
					{
						printf("cha xun ::select timeout::jieduan shu ju jie gou \n");
						close(sockfd_recv);
						return -1;
					}
					if(ret)                //������
					{
						res = recvfrom(sockfd_recv, select_recv_buf, 512, 0, NULL, NULL);
						if(res <= 0)
						{		
							close(sockfd_recv);
							return -1;
						}	
						else
						{
							printf("shou dao jie duan shu ju::%d\n",i);
							if((select_recv_buf[13]>0 && select_recv_buf[13] < 65) && select_recv_buf[10] == 0x27)
							{
								i++;
								for(j = 0 ;j < 17;j++)
									set27_data[select_recv_buf[13]-1][j] = select_recv_buf[14+j];
							}
						}
					}

					if(i == 64)
						break;//�յ�32�����ݣ��˳�ѭ������
	
				}

				sprintf(sql,"delete  from  ITC_CFG_STAGE  where signal_id = %d  " ,buf[7] * 256 + buf[8] );
				printf("%s\n",sql);
				st->setSQL(sql);
				st->executeUpdate();
					
				for(i = 0;i <64; i++)
				{
					try
					{
						char stage_phase[200],stage_detector[200],stage_temp[4];
						bzero(stage_phase,200);
						bzero(stage_detector,200);
						for(j = 0 ; j<4;j++) // ��λ����������4���ֽ�
						{
							int count;
							for(count = 0;count < 8;count++)
							{
								if((set27_data[i][j] & (0x80 >> count) ) !=0 )
								{
									sprintf(stage_temp,"%d,",(count+1+j*8));
									strcat(stage_phase,stage_temp);
								}

								if((set27_data[i][4 + j] & (0x80 >> count) ) !=0 )
								{
									sprintf(stage_temp,"%d,",(count+1+j*8));
									strcat(stage_detector,stage_temp);
								}
									
							}
						}
						sprintf(sql,"insert into ITC_CFG_STAGE(signal_id,stage_id,stage_phase,stage_detector,shield_occ,relieve_occ,stat_time,auto_occ_control,stage_step,auto_control_time) values(%d,%d,'%s','%s',%d,%d,%d,%d,%d,%d)  " ,
								buf[7] * 256 + buf[8] ,i+1,stage_phase,stage_detector,set27_data[i][8] ,set27_data[i][9] ,set27_data[i][10]*256+set27_data[i][11],set27_data[i][13] ,set27_data[i][14] ,set27_data[i][15]   );
						printf("%s\n",sql);
						st->setSQL(sql);
						st->executeUpdate();
					
					}
					catch (SQLException& ex)
					{
						close(sockfd_recv);
						return -1;
					}
				}
				close(sockfd_recv);
				return 0;
			}
			else if(buf[10] == 0x28)
			{
				unsigned char set28_data[64][64];
				struct timeval 	timeVal;
				fd_set  fdSet;
				int ret;
				sendto(sockfd_recv, select_buf, sizeof(select_buf), 0, (struct sockaddr*)&signal_addr, sizeof(signal_addr));
				i = 0;//�յ����ݵļ�����
				while(1)
				{
					bzero(select_recv_buf,sizeof(select_recv_buf));
					//res = recvfrom(sockfd_recv, select_recv_buf, 512, 0, NULL, NULL);
					//printf("recvfrom  date !!!!!!!!!!\n ");
					//if(res <= 0)
					//{
					//	close(sockfd_recv);
					//	return -1;
					//}
					timeVal.tv_sec = signal_report_timeout;
					timeVal.tv_usec = 0;

					FD_ZERO(&fdSet);
					FD_SET(sockfd_recv, &fdSet);

					ret = select((int) sockfd_recv + 1, &fdSet,NULL, NULL, &timeVal);
					if(ret == 0)                     //��ʱ
					{
						printf("cha xun ::select timeout::xiang wei shu ju jie gou \n");
						close(sockfd_recv);
						return -1;
					}
					if(ret)                //������
					{
						res = recvfrom(sockfd_recv, select_recv_buf, 512, 0, NULL, NULL);
						if(res <= 0)
						{		
							close(sockfd_recv);
							return -1;
						}	
						else
						{
							if((select_recv_buf[13]>0 && select_recv_buf[13] < 65) && select_recv_buf[10] == 0x28)
							{
								i++;
								for(j = 0 ;j < 64;j++)
									set28_data[select_recv_buf[13]-1][j] = select_recv_buf[14+j];
							}

							if(i == 64)
								break;//�յ�32�����ݣ��˳�ѭ������
						}
					}
				}

				sprintf(sql,"delete  from  ITC_CFG_STAGE_TRANSITION  where signal_id = %d  " ,buf[7] * 256 + buf[8] );
				printf("%s\n",sql);
				st->setSQL(sql);
				st->executeUpdate();
					
				for(i = 0;i <64; i++)
				{
					try
					{
						for(j = 0 ; j<64;j++) // ��λ����������4���ֽ�
						{
							sprintf(sql,"insert into ITC_CFG_STAGE_TRANSITION(signal_id,stage_id_row,stage_id_column,transition_value) values(%d,%d,%d,%d)  " ,
								buf[7] * 256 + buf[8] ,i+1,j+1,set28_data[i][j]  );
						printf("%s\n",sql);
						st->setSQL(sql);
						st->executeUpdate();
						}
						
					
					}
					catch (SQLException& ex)
					{
						close(sockfd_recv);
						return -1;
					}
				}
				close(sockfd_recv);
				return 0;
			}

			//ITC100,�������ò�ѯ
			else if(buf[10] == 0x29)
			{
				struct timeval 	timeVal;
				fd_set  fdSet;
				int ret;
				sendto(sockfd_recv, select_buf, sizeof(select_buf), 0, (struct sockaddr*)&signal_addr, sizeof(signal_addr));
				bzero(select_recv_buf,sizeof(select_recv_buf));
				//res = recvfrom(sockfd_recv, select_recv_buf, 512, 0, NULL, NULL);
				//printf("recvfrom  date !!!!!!!!!!\n ");
				//if(res <= 0)
				//{
				//	close(sockfd_recv);
				//	return -1;
				//}
				timeVal.tv_sec = signal_report_timeout;
					timeVal.tv_usec = 0;

					FD_ZERO(&fdSet);
					FD_SET(sockfd_recv, &fdSet);

					ret = select((int) sockfd_recv + 1, &fdSet,NULL, NULL, &timeVal);
					if(ret == 0)                     //��ʱ
					{
						printf("cha xun ::select timeout::xiang wei shu ju jie gou \n");
						close(sockfd_recv);
						return -1;
					}
					if(ret)                //������
					{
						res = recvfrom(sockfd_recv, select_recv_buf, 512, 0, NULL, NULL);
						if(res <= 0)
						{		
							close(sockfd_recv);
							return -1;
						}	
						else
						{
							sprintf(sql,"delete  from  ITC_CFG_URGENT  where signal_id = %d  " ,buf[7] * 256 + buf[8] );
							printf("%s\n",sql);
							st->setSQL(sql);
							st->executeUpdate();

							try
							{
								for(i = 0;i< 4;i++)
								{
									sprintf(sql,"insert into ITC_CFG_URGENT(signal_id,urgent_id,list_unit_id,stage_id,priority,flag) values(%d,%d,%d,%d,%d,%d)  " ,
									buf[7] * 256 + buf[8] ,i+1,select_recv_buf[13+i*4] ,select_recv_buf[14+i*4],select_recv_buf[15+i*4], select_recv_buf[16+i*4]);
								printf("%s\n",sql);
								st->setSQL(sql);
								st->executeUpdate();
								}

					
							}
							catch (SQLException& ex)
							{
								close(sockfd_recv);
								return -1;
							}
						}
					}

				close(sockfd_recv);
				return 0;
			}
			else if(buf[10] == 0x2a)
			{
				struct timeval 	timeVal;
				fd_set  fdSet;
				int ret;
				sendto(sockfd_recv, select_buf, sizeof(select_buf), 0, (struct sockaddr*)&signal_addr, sizeof(signal_addr));
				bzero(select_recv_buf,sizeof(select_recv_buf));
				//res = recvfrom(sockfd_recv, select_recv_buf, 512, 0, NULL, NULL);
				//printf("recvfrom  date !!!!!!!!!!\n ");
				//if(res <= 0)
				//{
				//	close(sockfd_recv);
				//	return -1;
				//}
				timeVal.tv_sec = signal_report_timeout;
					timeVal.tv_usec = 0;

					FD_ZERO(&fdSet);
					FD_SET(sockfd_recv, &fdSet);

					ret = select((int) sockfd_recv + 1, &fdSet,NULL, NULL, &timeVal);
					if(ret == 0)                     //��ʱ
					{
						printf("cha xun ::select timeout::xiang wei shu ju jie gou \n");
						close(sockfd_recv);
						return -1;
					}
					if(ret)                //������
					{
						res = recvfrom(sockfd_recv, select_recv_buf, 512, 0, NULL, NULL);
						if(res <= 0)
						{		
							close(sockfd_recv);
							return -1;
						}	
						else
						{
							sprintf(sql,"delete  from  ITC_CFG_PRIORITY  where signal_id = %d  " ,buf[7] * 256 + buf[8] );
							printf("%s\n",sql);
							st->setSQL(sql);
							st->executeUpdate();

							try
							{
								for(i = 0;i< 4;i++)
								{
									sprintf(sql,"insert into ITC_CFG_PRIORITY(signal_id,priority_id,list_unit_id,stage_id,priority,flag) values(%d,%d,%d,%d,%d,%d)  " ,
									buf[7] * 256 + buf[8] ,i+1,select_recv_buf[13+i*4] ,select_recv_buf[14+i*4],select_recv_buf[15+i*4], select_recv_buf[16+i*4]);
									printf("%s\n",sql);
									st->setSQL(sql);
									st->executeUpdate();
								}

					
							}
							catch (SQLException& ex)
							{
								close(sockfd_recv);
								return -1;
							}
						}
					}
				close(sockfd_recv);
				return 0;
			}
			else if(buf[10] == 0x2b)
			{
				unsigned char set2b_data[128][151];
				struct timeval 	timeVal;
				fd_set  fdSet;
				int ret;
				sendto(sockfd_recv, select_buf, sizeof(select_buf), 0, (struct sockaddr*)&signal_addr, sizeof(signal_addr));
				
				int count;
				for(count = 0;count <sizeof(select_buf); count++)
					printf("%x  ",select_buf[count]);
				printf("\n");
				i = 0;//�յ����ݵļ�����
				while(1)
				{
					bzero(select_recv_buf,sizeof(select_recv_buf));
					//res = recvfrom(sockfd_recv, select_recv_buf, 512, 0, NULL, NULL);
					//printf("recvfrom  date !!!!!!!!!!\n ");
					//if(res <= 0)
					//{
					//	close(sockfd_recv);
					//	return -1;
					//}
					timeVal.tv_sec = signal_report_timeout;
					timeVal.tv_usec = 0;

					FD_ZERO(&fdSet);
					FD_SET(sockfd_recv, &fdSet);

					ret = select((int) sockfd_recv + 1, &fdSet,NULL, NULL, &timeVal);
					if(ret == 0)                     //��ʱ
					{
						printf("cha xun ::select timeout::fang an shu ju \n");
						close(sockfd_recv);
						return -1;
					}
					if(ret)                //������
					{
						res = recvfrom(sockfd_recv, select_recv_buf, 512, 0, NULL, NULL);
						if(res <= 0)
						{		
							close(sockfd_recv);
							return -1;
						}	
						else
						{
							if((select_recv_buf[13]>0 && select_recv_buf[13] < 129) && select_recv_buf[10] == 0x2b)
							{
								i++;
								for(j = 0 ;j < 151;j++)
									set2b_data[select_recv_buf[13]-1][j] = select_recv_buf[14+j];
							}

							if(i == 128)
								break;//�յ�32�����ݣ��˳�ѭ������
						}
					}
				}

				sprintf(sql,"delete  from  ITC_CFG_PLAN  where signal_id = %d  " ,buf[7] * 256 + buf[8] );
				printf("%s\n",sql);
				st->setSQL(sql);
				st->executeUpdate();

				sprintf(sql,"delete  from  ITC_CFG_PLAN_CHAIN  where signal_id = %d  " ,buf[7] * 256 + buf[8] );
				printf("%s\n",sql);
				st->setSQL(sql);
				st->executeUpdate();
					
				for(i = 0;i <128; i++)
				{
					try
					{
						sprintf(sql,"insert into ITC_CFG_PLAN(signal_id,plan_id,list_unit_id,plan_week,plan_record,phase_diff_time) values(%d,%d,%d,%.1f,%d,%d)  " ,
								buf[7] * 256 + buf[8] ,i+1,set2b_data[i][0], (set2b_data[i][1] *256+set2b_data[i][2])/10.0,set2b_data[i][3], set2b_data[i][4] *256+set2b_data[i][5]  );
						printf("%s\n",sql);
						st->setSQL(sql);
						st->executeUpdate();

						for(j=0;j<set2b_data[i][6];j++)
						{
							sprintf(sql,"insert into ITC_CFG_PLAN_CHAIN(signal_id,plan_id,stage_order,stage_id,stage_time,stage_type,green_time,green_flash_time,green_flash_qtime,red_time) values(%d,%d,%d,%d,%.1f,%d,%.1f,%.1f,%.1f,%.1f)  " ,
								buf[7] * 256 + buf[8] ,i+1,j+1,set2b_data[i][7+j],(set2b_data[i][23+j*2]*256+set2b_data[i][24+j*2])/10.0,set2b_data[i][55+j],
								(set2b_data[i][71+j*5]*256+set2b_data[i][72+j*5])/10.0,set2b_data[i][73+j*5]/10.0,set2b_data[i][74+j*5]/10.0,set2b_data[i][75+j*5]/10.0);
							printf("%s\n",sql);
							st->setSQL(sql);
							st->executeUpdate();
						}
					}
					catch (SQLException& ex)
					{
						close(sockfd_recv);
						return -1;
					}
				}
				close(sockfd_recv);
				return 0;
			}
			
			//itc100,ʱ������ò�ѯ
			else if(buf[10] == 0x2c)
			{
				unsigned char set2c_data[128][410];
				struct timeval 	timeVal;
				fd_set  fdSet;
				int ret;
				sendto(sockfd_recv, select_buf, sizeof(select_buf), 0, (struct sockaddr*)&signal_addr, sizeof(signal_addr));
				int count;
				for(count = 0;count < sizeof(select_buf);count ++)
					printf("%x  ",select_buf[count]);
				printf("\n");
				i = 0;//�յ����ݵļ�����
				while(1)
				{
					bzero(select_recv_buf,sizeof(select_recv_buf));
					//res = recvfrom(sockfd_recv, select_recv_buf, 512, 0, NULL, NULL);
					//printf("recvfrom  date !!!!!!!!!!\n ");
					//if(res <= 0)
					//{
					//	close(sockfd_recv);
					//	return -1;
					//}
					timeVal.tv_sec = signal_report_timeout;
					timeVal.tv_usec = 0;

					FD_ZERO(&fdSet);
					FD_SET(sockfd_recv, &fdSet);

					ret = select((int) sockfd_recv + 1, &fdSet,NULL, NULL, &timeVal);
					if(ret == 0)                     //��ʱ
					{
						printf("cha xun ::select timeout::shi jian biao\n");
						close(sockfd_recv);
						return -1;
					}
					if(ret)                //������
					{
						res = recvfrom(sockfd_recv, select_recv_buf, 512, 0, NULL, NULL);
						if(res <= 0)
						{		
							close(sockfd_recv);
							return -1;
						}	
						else
						{
							if((select_recv_buf[13]>0 && select_recv_buf[13] < 129) && select_recv_buf[10] == 0x2c)
							{
								i++;
								for(j = 0 ;j < 410;j++)
									set2c_data[select_recv_buf[13]-1][j] = select_recv_buf[14+j];
							}

							if(i == 128)
								break;//�յ�32�����ݣ��˳�ѭ������
						}
					}
				}

				sprintf(sql,"delete  from  ITC_CFG_TIME  where signal_id = %d  " ,buf[7] * 256 + buf[8] );
				printf("%s\n",sql);
				st->setSQL(sql);
				st->executeUpdate();

				sprintf(sql,"delete  from  ITC_CFG_TIME_DETAIL  where signal_id = %d  " ,buf[7] * 256 + buf[8] );
				printf("%s\n",sql);
				st->setSQL(sql);
				st->executeUpdate();

				sprintf(sql,"delete  from  ITC_CFG_TIME_PERIOD  where signal_id = %d  " ,buf[7] * 256 + buf[8] );
				printf("%s\n",sql);
				st->setSQL(sql);
				st->executeUpdate();
					
				for(i = 0;i <128; i++)
				{
					try
					{
						sprintf(sql,"insert into ITC_CFG_TIME(signal_id,time_id,list_unit_id) values(%d,%d,%d)  " ,
								buf[7] * 256 + buf[8] ,i+1, set2c_data[i][0] );
						printf("%s\n",sql);
						st->setSQL(sql);
						st->executeUpdate();

						sprintf(sql,"insert into ITC_CFG_TIME_DETAIL(signal_id,time_id,list_unit_id) values(%d,%d,%d)  " ,
								buf[7] * 256 + buf[8] ,i+1, set2c_data[i][0] );
						printf("%s\n",sql);
						st->setSQL(sql);
						st->executeUpdate();

						char stage[200],variable[200],stage_temp[4];
						bzero(stage,200);
						bzero(variable,200);				

						for(j=0;j<set2c_data[i][1];j++)
						{
							int k;
							for(k = 0 ; k<4;k++) // ��λ����������4���ֽ�
							{
							int count;
							for(count = 0;count < 8;count++)
							{
								if((set2c_data[i][7+k+j*17] & (0x80 >> count) ) !=0 )
								{
									sprintf(stage_temp,"%d,",(count+1+k*8));
									strcat(stage,stage_temp);
								}

								if((set2c_data[i][11 + k+j*17] & (0x80 >> count) ) !=0 )
								{
									sprintf(stage_temp,"%d,",(count+1+k*8));
									strcat(variable,stage_temp);
								}
									
							}
							}
							
							sprintf(sql,"insert into ITC_CFG_TIME_PERIOD(signal_id,time_id,time_order,hour,minute,plan_id,run_mode,occ_control_flag,shield_stages,variable_sign,special_output_phase,red_status,yellow_status,green_status) values(%d,%d,%d,%d,%d,%d,%d,%d,'%s','%s',%d,%d,%d,%d)  " ,
								buf[7] * 256 + buf[8] ,i+1,j+1,set2c_data[i][2+j*17],set2c_data[i][3+j*17],set2c_data[i][4+j*17],
								set2c_data[i][5+j*17],set2c_data[i][6+j*17],stage,variable,set2c_data[i][15+j*17],set2c_data[i][16+j*17],set2c_data[i][17+j*17],set2c_data[i][18+j*17]);
							printf("%s\n",sql);
							st->setSQL(sql);
							st->executeUpdate();
						}
					}
					catch (SQLException& ex)
					{
						close(sockfd_recv);
						return -1;
					}
				}
				close(sockfd_recv);
				return 0;
			}
			else if(buf[10] == 0x2d)
			{
				unsigned char set2d_data[128][19];
				struct timeval 	timeVal;
				fd_set  fdSet;
				int ret;
				sendto(sockfd_recv, select_buf, sizeof(select_buf), 0, (struct sockaddr*)&signal_addr, sizeof(signal_addr));
				i = 0;//�յ����ݵļ�����
				while(1)
				{
					bzero(select_recv_buf,sizeof(select_recv_buf));
					//res = recvfrom(sockfd_recv, select_recv_buf, 512, 0, NULL, NULL);
					//printf("recvfrom  date !!!!!!!!!!\n ");
					//if(res <= 0)
					//{
					//	close(sockfd_recv);
					//	return -1;
					//}
					timeVal.tv_sec = signal_report_timeout;
					timeVal.tv_usec = 0;

					FD_ZERO(&fdSet);
					FD_SET(sockfd_recv, &fdSet);

					ret = select((int) sockfd_recv + 1, &fdSet,NULL, NULL, &timeVal);
					if(ret == 0)                     //��ʱ
					{
						printf("cha xun ::select timeout::xiang wei shu ju jie gou \n");
						close(sockfd_recv);
						return -1;
					}
					if(ret)                //������
					{
						res = recvfrom(sockfd_recv, select_recv_buf, 512, 0, NULL, NULL);
						if(res <= 0)
						{		
							close(sockfd_recv);
							return -1;
						}	
						else
						{
							if((select_recv_buf[13]>0 && select_recv_buf[13] < 129) && select_recv_buf[10] == 0x2d)
							{
								i++;
								for(j = 0 ;j < 19;j++)
									set2d_data[select_recv_buf[13]-1][j] = select_recv_buf[14+j];
							}

							if(i == 128)
								break;//�յ�32�����ݣ��˳�ѭ������
						}
					}
				}

				sprintf(sql,"delete  from  ITC_CFG_MANAGE  where signal_id = %d  " ,buf[7] * 256 + buf[8] );
				printf("%s\n",sql);
				st->setSQL(sql);
				st->executeUpdate();

				
					
				for(i = 0;i <128; i++)
				{



					char week[50],month[100],day[200],temp[5];
					bzero(week,50);
					bzero(month,100);
					bzero(day,200);
					bzero(temp,5);
					for(j = 1;j<8;j++)
					{
						if((set2d_data[i][3] &(0x01<<(j%7))) != 0)
						{
							sprintf(temp,"%d,",j);
							strcat(week,temp);
						}
						
					}
#if 0
					for(j = 1;j<=12;j++)
					{

						if(j<9)
						{
							if((set2d_data[i][5] &(0x01<<(j-1))) != 0)
							{
								sprintf(temp,"%d,",j);
								strcat(month,temp);
							}
						}
						else
						{
							if((set2d_data[i][4] &(0x01<<((j%8)-1))) != 0)
							{
								sprintf(temp,"%d,",j);
								strcat(month,temp);
							}
						}
					}
#else

					if((((set2d_data[i][4]<< 8) + set2d_data[i][5]) & (0x01 < (j-1))) !=0)
					{
						sprintf(temp,"%d,",j);
						strcat(month,temp);
					}
#endif						
					
					//����
					for(j = 1;j<32;j++)
					{
						if((((set2d_data[i][6]<< 24)+(set2d_data[i][7]<< 16)+(set2d_data[i][8]<< 8) + set2d_data[i][9]) & (0x01 < (j-1))) !=0)
						{
							sprintf(temp,"%d,",j);
							strcat(day,temp);
						}
					}
					try
					{
						sprintf(sql,"insert into ITC_CFG_MANAGE(signal_id,manage_id,list_unit_id,manage_type,manage_priority,time_id,downgrade_id,week_mng,date_mng_month,date_mng_day) values(%d,%d,%d,%d,%d,%d,%d,'%s','%s','%s')  " ,
								buf[7] * 256 + buf[8] ,i+1,set2d_data[i][0], set2d_data[i][1] ,set2d_data[i][2], set2d_data[i][17] , set2d_data[i][18] ,week,month,day);
						printf("%s\n",sql);
						st->setSQL(sql);
						st->executeUpdate();

						
					}
					catch (SQLException& ex)
					{
						close(sockfd_recv);
						return -1;
					}
				}
				close(sockfd_recv);
				return 0;
			}
			else if(buf[10] == 0x2e)
			{
				unsigned char set2e_data[8][10];
				struct timeval 	timeVal;
				fd_set  fdSet;
				int ret;
				sendto(sockfd_recv, select_buf, sizeof(select_buf), 0, (struct sockaddr*)&signal_addr, sizeof(signal_addr));
				int count;	
				for(count=0;count <sizeof(select_buf);count++ )
				{
					printf("%x  ",select_buf[count]);
				}
				printf("\n");
				i = 0;//�յ����ݵļ�����
				while(1)
				{
					bzero(select_recv_buf,sizeof(select_recv_buf));
					//res = recvfrom(sockfd_recv, select_recv_buf, 512, 0, NULL, NULL);
					//printf("recvfrom  date !!!!!!!!!!\n ");
					//if(res <= 0)
					//{
					//	close(sockfd_recv);
					//	return -1;
					//}
					timeVal.tv_sec = signal_report_timeout;
					timeVal.tv_usec = 0;

					FD_ZERO(&fdSet);
					FD_SET(sockfd_recv, &fdSet);

					ret = select((int) sockfd_recv + 1, &fdSet,NULL, NULL, &timeVal);
					if(ret == 0)                     //��ʱ
					{
						printf("cha xun ::select timeout::shou dong can shu \n");
						close(sockfd_recv);
						return -1;
					}
					if(ret)                //������
					{
						res = recvfrom(sockfd_recv, select_recv_buf, 512, 0, NULL, NULL);
						if(res <= 0)
						{		
							close(sockfd_recv);
							return -1;
						}	
						else
						{
							if((select_recv_buf[13]>0 && select_recv_buf[13] < 9) && select_recv_buf[10] == 0x2e)
							{
								i++;
								for(j = 0 ;j < 10;j++)
									set2e_data[select_recv_buf[13]-1][j] = select_recv_buf[14+j];
							}	

							if(i == 8)
								break;//�յ�32�����ݣ��˳�ѭ������
						}
					}
				}

				sprintf(sql,"delete  from  ITC_CFG_MANUAL  where signal_id = %d  " ,buf[7] * 256 + buf[8] );
				printf("%s\n",sql);
				st->setSQL(sql);
				st->executeUpdate();
					
				for(i = 0;i <8; i++)
				{
					try
					{
						for(j = 0;j<10;j++)
						{
							sprintf(sql,"insert into ITC_CFG_MANUAL(signal_id,list_unit_id,key,stage_id) values(%d,%d,%d,%d)  " ,
								buf[7] * 256 + buf[8] ,i+1,(j+1)%10,set2e_data[i][j]  );
							printf("%s\n",sql);
							st->setSQL(sql);
							st->executeUpdate();
						}
						
					
					}
					catch (SQLException& ex)
					{
						close(sockfd_recv);
						return -1;
					}
				}
				close(sockfd_recv);
				return 0;
			}
			else if(buf[10] == 0x2f)
			{
				unsigned char set2f_data[8][177];
				struct timeval 	timeVal;
				fd_set  fdSet;
				int ret;
				sendto(sockfd_recv, select_buf, sizeof(select_buf), 0, (struct sockaddr*)&signal_addr, sizeof(signal_addr));
				i = 0;//�յ����ݵļ�����
				while(1)
				{
					bzero(select_recv_buf,sizeof(select_recv_buf));
					//res = recvfrom(sockfd_recv, select_recv_buf, 512, 0, NULL, NULL);
					//printf("recvfrom  date !!!!!!!!!!\n ");
					//if(res <= 0)
					//{
					//	close(sockfd_recv);
					//	return -1;
					//}
					timeVal.tv_sec = signal_report_timeout;
					timeVal.tv_usec = 0;

					FD_ZERO(&fdSet);
					FD_SET(sockfd_recv, &fdSet);

					ret = select((int) sockfd_recv + 1, &fdSet,NULL, NULL, &timeVal);
					if(ret == 0)                     //��ʱ
					{
						printf("cha xun ::select timeout::xiang wei shu ju jie gou \n");
						close(sockfd_recv);
						return -1;
					}
					if(ret)                //������
					{
						res = recvfrom(sockfd_recv, select_recv_buf, 512, 0, NULL, NULL);
						if(res <= 0)
						{		
							close(sockfd_recv);
							return -1;
						}	
						else
						{
							if((select_recv_buf[13]>0 && select_recv_buf[13] < 9) && select_recv_buf[10] == 0x2f)
							{
								i++;
								for(j = 0 ;j < 177;j++)
									set2f_data[select_recv_buf[13]-1][j] = select_recv_buf[14+j];
							}

							if(i == 8)
								break;//�յ�8�����ݣ��˳�ѭ������
						}
					}
				}

				sprintf(sql,"delete  from  ITC_CFG_DOWNGRADE_CHAIN  where signal_id = %d  " ,buf[7] * 256 + buf[8] );
				printf("%s\n",sql);
				st->setSQL(sql);
				st->executeUpdate();
					
				for(i = 0;i <8; i++)
				{
					
					int count,stageTime ,greenTime,greenFlash,greenQtime,redTime,downPhase_temp=0;
					char dPhase_temp[200];
					
					
					for(j = 0 ;j< set2f_data[i][0] ;j++)
					{	
						//�����׶���
						bzero(dPhase_temp,200);
						for(count = 1;count < 33;count++)
						{
							if((((set2f_data[i][1+j*4] <<24) +(set2f_data[i][2+j*4]<<16)+(set2f_data[i][3+j*4] <<8) +(set2f_data[i][4+j*4]  ) ) & (0x80000000 >> (count -1)))!= 0)
							{	
								char down_temp[4];
								sprintf(down_temp,"%d,",count);
								strcat(dPhase_temp,down_temp);
								//downPhase_temp = count;
								//break;//4               	//	4���ֽڱ�ʾһ����λ
							}
						}

						sprintf(sql,"select t.stage_id from itc_cfg_stage t where t.signal_id=%d and (t.stage_phase = '%s' or t.stage_phase||',' = '%s') order by t.stage_id  " ,
								buf[7] * 256 + buf[8] ,dPhase_temp,dPhase_temp);
						printf("%s\n",sql);
						st->setSQL(sql);
						rs = st->executeQuery();

						if(rs->next() != 0)
							downPhase_temp = rs->getInt(1);

						 //�׶���ʱ��
						stageTime = set2f_data[i][65+j*2] *256 +  set2f_data[i][66+j*2] ;

						//������ʱ��
						greenTime = set2f_data[i][97+j*5] *256 +  set2f_data[i][98+j*5] ; 
						greenFlash = set2f_data[i][99+j*5];
						greenQtime = set2f_data[i][100+j*5];
						redTime = set2f_data[i][101+j*5];	

						try
						{

							sprintf(sql,"insert into ITC_CFG_DOWNGRADE_CHAIN(signal_id,list_unit_id,stage_order,stage_id,stage_time,green_time,green_flash_time,green_flash_qtime,red_time) values(%d,%d,%d,%d,%d,%d,%d,%d,%d)  " ,
								buf[7] * 256 + buf[8] ,i+1,(j+1),downPhase_temp,stageTime, greenTime,greenFlash, greenQtime,redTime);
							printf("%s\n",sql);
							st->setSQL(sql);
							st->executeUpdate();
						}
						
						catch (SQLException& ex)
						{
							close(sockfd_recv);
							return -1;
						}
					}
					
					
				}
				close(sockfd_recv);
				return 0;
			}
			else if(buf[10] == 0x30)
			{
				struct timeval 	timeVal;
				fd_set  fdSet;
				int ret;
				sendto(sockfd_recv, select_buf, sizeof(select_buf), 0, (struct sockaddr*)&signal_addr, sizeof(signal_addr));
				bzero(select_recv_buf,sizeof(select_recv_buf));
				//res = recvfrom(sockfd_recv, select_recv_buf, 512, 0, NULL, NULL);
				//printf("recvfrom  date !!!!!!!!!!\n ");
				//if(res <= 0)
				//{
				//	close(sockfd_recv);
				//	return -1;
				//}
				timeVal.tv_sec = signal_report_timeout;
					timeVal.tv_usec = 0;

					FD_ZERO(&fdSet);
					FD_SET(sockfd_recv, &fdSet);

					ret = select((int) sockfd_recv + 1, &fdSet,NULL, NULL, &timeVal);
					if(ret == 0)                     //��ʱ
					{
						printf("cha xun ::select timeout::xiang wei shu ju jie gou \n");
						close(sockfd_recv);
						return -1;
					}
					if(ret)                //������
					{
						res = recvfrom(sockfd_recv, select_recv_buf, 512, 0, NULL, NULL);
						if(res <= 0)
						{		
							close(sockfd_recv);
							return -1;
						}	
						else
						{
							try
							{
								sprintf(sql,"update ITC_CFG_PASSWORD  set password = '%d,%d,%d,%d,%d,%d' where signal_id = %d  " ,
								select_recv_buf[13] ,select_recv_buf[14] ,select_recv_buf[15] ,select_recv_buf[16],select_recv_buf[17],select_recv_buf[18],buf[7] * 256 + buf[8] );
								printf("%s\n",sql);
								st->setSQL(sql);
								st->executeUpdate();	
							}
							catch (SQLException& ex)
							{
								close(sockfd_recv);
								return -1;
							}
						}
						close(sockfd_recv);
						return 0;		
					}
		}
	
	}	
		/**********************/
	}
	catch (SQLException &sqlExcp)
    	{
    		cout << "oracle  failed: " << sqlExcp.getMessage() << endl; 
    		close(sockfd_recv);
    		return 10;
    	}
	return -1;
}


/*************************************************** 
 * ��������all_send_signal_message_info
 * ������������ʼ�����з��͵��źŻ��ı���
 *�����ʵı�
 *���޸ĵı�
* ����˵����
*����ֵ:  ��
***************************************************/
void all_send_signal_message_info(void)
{
	int i;
	
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

	/*����ȡ������*/
	F2_date[0] = 0x7e;
	F2_date[1] = 0x00;
	F2_date[2] = 0x0d;
	F2_date[3] = 0x00;
	F2_date[4] = 0x01;
	F2_date[5] = 0x00;
	F2_date[6] = 0x00;
	F2_date[7] = 0x00;
	F2_date[8] = 0x00;
	F2_date[9] = 0x20;
	F2_date[10] = 0xf2;
	F2_date[11] = 0x00;
	F2_date[12] = 0x00;
	F2_date[13] = 0x00;
	F2_date[14] = 0x7d;

	/*��������*/
	F3_date[0] = 0x7e;
	F3_date[1] = 0x00;
	F3_date[2] = 0x0c;
	F3_date[3] = 0x00;
	F3_date[4] = 0x01;
	F3_date[5] = 0x00;
	F3_date[6] = 0x00;
	F3_date[7] = 0x00;
	F3_date[8] = 0x00;
	F3_date[9] = 0x20;
	F3_date[10] = 0xf3;
	F3_date[11] = 0x00;
	F3_date[12] = 0x00;
	F3_date[13] = 0x7d;

	/*�ֶ�����*/
	C0_date[0] = 0x7e;
	C0_date[1] = 0x00;
	C0_date[2] = 0x0c;
	C0_date[3] = 0x00;
	C0_date[4] = 0x01;
	C0_date[5] = 0x00;
	C0_date[6] = 0x00;
	C0_date[7] = 0x00;
	C0_date[8] = 0x00;
	C0_date[9] = 0x20;
	C0_date[10] = 0xc0;
	C0_date[11] = 0x00;
	C0_date[12] = 0x00;
	C0_date[13] = 0x7d;

	/*��������*/
	C1_date[0] = 0x7e;
	C1_date[1] = 0x00;
	C1_date[2] = 0x0c;
	C1_date[3] = 0x00;
	C1_date[4] = 0x01;
	C1_date[5] = 0x00;
	C1_date[6] = 0x00;
	C1_date[7] = 0x00;
	C1_date[8] = 0x00;
	C1_date[9] = 0x20;
	C1_date[10] = 0xc1;
	C1_date[11] = 0x00;
	C1_date[12] = 0x00;
	C1_date[13] = 0x7d;

	/*ָ������*/
	C2_date[0] = 0x7e;
	C2_date[1] = 0x00;
	C2_date[2] = 0x0d;
	C2_date[3] = 0x00;
	C2_date[4] = 0x01;
	C2_date[5] = 0x00;
	C2_date[6] = 0x00;
	C2_date[7] = 0x00;
	C2_date[8] = 0x00;
	C2_date[9] = 0x20;
	C2_date[10] = 0xc2;
	C2_date[11] = 0x00;
	C2_date[12] = 0x00;
	C2_date[13] = 0x00;
	C2_date[14] = 0x7d;

	
	/*ָ��ģʽ*/
	C3_date[0] = 0x7e;
	C3_date[1] = 0x00;
	C3_date[2] = 0x0d;
	C3_date[3] = 0x00;
	C3_date[4] = 0x01;
	C3_date[5] = 0x00;
	C3_date[6] = 0x00;
	C3_date[7] = 0x00;
	C3_date[8] = 0x00;
	C3_date[9] = 0x20;
	C3_date[10] = 0xc3;
	C3_date[11] = 0x00;
	C3_date[12] = 0x00;
	C3_date[13] = 0x00;
	C3_date[14] = 0x7d;

	/*ָ������*/
	C4_date[0] = 0x7e;
	C4_date[1] = 0x00;
	C4_date[2] = 0x0e;
	C4_date[3] = 0x00;
	C4_date[4] = 0x01;
	C4_date[5] = 0x00;
	C4_date[6] = 0x00;
	C4_date[7] = 0x00;
	C4_date[8] = 0x00;
	C4_date[9] = 0x20;
	C4_date[10] = 0xc4;
	C4_date[11] = 0x00;
	C4_date[12] = 0x00;
	C4_date[13] = 0x00;
	C4_date[14] = 0x00;
	C4_date[15] = 0x7d;

	/*ָ����λ��*/
	C5_date[0] = 0x7e;
	C5_date[1] = 0x00;
	C5_date[2] = 0x0e;
	C5_date[3] = 0x00;
	C5_date[4] = 0x01;
	C5_date[5] = 0x00;
	C5_date[6] = 0x00;
	C5_date[7] = 0x00;
	C5_date[8] = 0x00;
	C5_date[9] = 0x20;
	C5_date[10] = 0xc5;
	C5_date[11] = 0x00;
	C5_date[12] = 0x00;
	C5_date[13] = 0x00;
	C5_date[14] = 0x00;
	C5_date[15] = 0x7d;

	/*�·���ʱ����*/
	C6_date[0] = 0x7e;
	C6_date[1] = 0x00;
	C6_date[2] = 0x53;
	C6_date[3] = 0x00;
	C6_date[4] = 0x01;
	C6_date[5] = 0x00;
	C6_date[6] = 0x00;
	C6_date[7] = 0x00;
	C6_date[8] = 0x00;
	C6_date[9] = 0x20;
	C6_date[10] = 0xc6;
	C6_date[11] = 0x00;
	C6_date[12] = 0x01;
	C6_date[84] = 0x7d;


	/*���û�����Ϣ*/
	set_01_date[0] = 0x7e;
	set_01_date[1] = 0x00;
	set_01_date[2] = 0xa2 + 1;
	set_01_date[3] = 0x00;
	set_01_date[4] = 0x01;
	set_01_date[5] = 0x00;
	set_01_date[6] = 0x00;
	set_01_date[7] = 0x00;
	set_01_date[8] = 0x00;
	set_01_date[9] = 0x20;
	set_01_date[10] = 0x01;
	set_01_date[11] = 0x00;
	set_01_date[163 + 1] = 0x7d;

	/*����������*/
	set_02_date[0] = 0x7e;
	set_02_date[1] = 0x00;
	set_02_date[2] = 0x16;
	set_02_date[3] = 0x00;
	set_02_date[4] = 0x01;
	set_02_date[5] = 0x00;
	set_02_date[6] = 0x00;
	set_02_date[7] = 0x00;
	set_02_date[8] = 0x00;
	set_02_date[9] = 0x20;
	set_02_date[10] = 0x02;
	set_02_date[11] = 0x00;
	set_02_date[23] = 0x7d;

	/*���ÿ���״̬����*/
	for(i = 0;i < 8 ; i++)
	{
		set_11_date[i][0] = 0x7e;
		set_11_date[i][1] = 0x00;
		set_11_date[i][2] = 0x13;
		set_11_date[i][3] = 0x00;
		set_11_date[i][4] = 0x01;
		set_11_date[i][5] = 0x00;
		set_11_date[i][6] = 0x00;
		set_11_date[i][7] = 0x00;
		set_11_date[i][8] = 0x00;
		set_11_date[i][9] = 0x20;
		set_11_date[i][10] = 0x11;
		set_11_date[i][11] = i;
		set_11_date[i][12] = i + 1;
		set_11_date[i][20] = 0x7d;
	}

	/*���ú���̲���*/
	for(i = 0;i < 8 ; i++)
	{
		set_12_date[i][0] = 0x7e;
		set_12_date[i][1] = 0x00;
		set_12_date[i][2] = 0x13;
		set_12_date[i][3] = 0x00;
		set_12_date[i][4] = 0x01;
		set_12_date[i][5] = 0x00;
		set_12_date[i][6] = 0x00;
		set_12_date[i][7] = 0x00;
		set_12_date[i][8] = 0x00;
		set_12_date[i][9] = 0x20;
		set_12_date[i][10] = 0x12;
		set_12_date[i][11] = i;
		set_12_date[i][12] = i + 1;
		set_12_date[i][20] = 0x7d;
	}

	/*�����̱�����*/
	for(i = 0;i < 8 ; i++)
	{
		set_13_date[i][0] = 0x7e;
		set_13_date[i][1] = 0x00;
		set_13_date[i][2] = 0x13;
		set_13_date[i][3] = 0x00;
		set_13_date[i][4] = 0x01;
		set_13_date[i][5] = 0x00;
		set_13_date[i][6] = 0x00;
		set_13_date[i][7] = 0x00;
		set_13_date[i][8] = 0x00;
		set_13_date[i][9] = 0x20;
		set_13_date[i][10] = 0x13;
		set_13_date[i][11] = i;
		set_13_date[i][12] = i + 1;
		set_13_date[i][20] = 0x7d;
	}

	/*������λ����*/
	for(i = 0;i < 16 ; i++)
	{
		set_03_date[i][0] = 0x7e;
		set_03_date[i][1] = 0x00;
		set_03_date[i][2] = 0xb6 + 5;
		set_03_date[i][3] = 0x00;
		set_03_date[i][4] = 0x01;
		set_03_date[i][5] = 0x00;
		set_03_date[i][6] = 0x00;
		set_03_date[i][7] = 0x00;
		set_03_date[i][8] = 0x00;
		set_03_date[i][9] = 0x20;
		set_03_date[i][10] = 0x03;
		set_03_date[i][11] = i;
		set_03_date[i][12] = i + 1;
		set_03_date[i][183 + 5] = 0x7d;
	}

	/*���ý׶�����*/
	for(i = 0;i < 64 ; i++)
	{
		set_04_date[i][0] = 0x7e;
		set_04_date[i][1] = 0x00;
		set_04_date[i][2] = 0xac;
		set_04_date[i][3] = 0x00;
		set_04_date[i][4] = 0x01;
		set_04_date[i][5] = 0x00;
		set_04_date[i][6] = 0x00;
		set_04_date[i][7] = 0x00;
		set_04_date[i][8] = 0x00;
		set_04_date[i][9] = 0x20;
		set_04_date[i][10] = 0x04;
		set_04_date[i][11] = i;
		set_04_date[i][12] = i + 1;
		set_04_date[i][173] = 0x7d;
	}

	/*���÷�������*/
	for(i = 0;i < 128 ; i++)
	{
		set_05_date[i][0] = 0x7e;
		set_05_date[i][1] = 0x00;
		set_05_date[i][2] = 0xd3;
		set_05_date[i][3] = 0x00;
		set_05_date[i][4] = 0x01;
		set_05_date[i][5] = 0x00;
		set_05_date[i][6] = 0x00;
		set_05_date[i][7] = 0x00;
		set_05_date[i][8] = 0x00;
		set_05_date[i][9] = 0x20;
		set_05_date[i][10] = 0x05;
		set_05_date[i][11] = i;
		set_05_date[i][12] = i + 1;
		set_05_date[i][212] = 0x7d;
	}

	/*����ʱ�������*/
	for(i = 0;i < 128 ; i++)
	{
		set_06_date[i][0] = 0x7e;
		set_06_date[i][1] = 0x01;
		set_06_date[i][2] = 0x4e;
		set_06_date[i][3] = 0x00;
		set_06_date[i][4] = 0x01;
		set_06_date[i][5] = 0x00;
		set_06_date[i][6] = 0x00;
		set_06_date[i][7] = 0x00;
		set_06_date[i][8] = 0x00;
		set_06_date[i][9] = 0x20;
		set_06_date[i][10] = 0x06;
		set_06_date[i][11] = i;
		set_06_date[i][12] = i + 1;
		set_06_date[i][335] = 0x7d;
	}

	/*���õ��ȱ�����*/
	for(i = 0;i < 128 ; i++)
	{
		set_07_date[i][0] = 0x7e;
		set_07_date[i][1] = 0x00;
		set_07_date[i][2] = 0x9e;
		set_07_date[i][3] = 0x00;
		set_07_date[i][4] = 0x01;
		set_07_date[i][5] = 0x00;
		set_07_date[i][6] = 0x00;
		set_07_date[i][7] = 0x00;
		set_07_date[i][8] = 0x00;
		set_07_date[i][9] = 0x20;
		set_07_date[i][10] = 0x07;
		set_07_date[i][11] = i;
		set_07_date[i][12] = i + 1;
		set_07_date[i][159] = 0x7d;
	}

	set_08_date[0] = 0x7e;
	set_08_date[1] = 0x01;
	set_08_date[2] = 0x0C;
	set_08_date[3] = 0x00;
	set_08_date[4] = 0x01;
	set_08_date[5] = 0x00;
	set_08_date[6] = 0x00;
	set_08_date[7] = 0x00;
	set_08_date[8] = 0x00;
	set_08_date[9] = 0x20;
	set_08_date[10] = 0x08;
	set_08_date[11] = 0;
	set_08_date[269] = 0x7d;


	
	/*�׶ι���Լ��*/
	for(i = 0;i < 64 ; i++)
	{
		set_09_date[i][0] = 0x7e;
		set_09_date[i][1] = 0x00;
		set_09_date[i][2] = 0x4c;
		set_09_date[i][3] = 0x00;
		set_09_date[i][4] = 0x01;
		set_09_date[i][5] = 0x00;
		set_09_date[i][6] = 0x00;
		set_09_date[i][7] = 0x00;
		set_09_date[i][8] = 0x00;
		set_09_date[i][9] = 0x20;
		set_09_date[i][10] = 0x09;
		set_09_date[i][11] = i;
		set_09_date[i][77] = 0x7d;
	}

	set_0a_date[0] = 0x7e;
	set_0a_date[1] = 0x00;
	set_0a_date[2] = 0x1a;
	set_0a_date[3] = 0x00;
	set_0a_date[4] = 0x01;
	set_0a_date[5] = 0x00;
	set_0a_date[6] = 0x00;
	set_0a_date[7] = 0x00;
	set_0a_date[8] = 0x00;
	set_0a_date[9] = 0x20;
	set_0a_date[10] = 0x0a;
	set_0a_date[11] = 0;
	set_0a_date[27] = 0x7d;



	set_0b_date[0] = 0x7e;
	set_0b_date[1] = 0x00;
	set_0b_date[2] = 0x18;
	set_0b_date[3] = 0x00;
	set_0b_date[4] = 0x01;
	set_0b_date[5] = 0x00;
	set_0b_date[6] = 0x00;
	set_0b_date[7] = 0x00;
	set_0b_date[8] = 0x00;
	set_0b_date[9] = 0x20;
	set_0b_date[10] = 0x0b;

	set_0b_date[12] = 0x01;
	set_0b_date[15] = 0x02;
	set_0b_date[18] = 0x03;
	set_0b_date[21] = 0x04;
	
	set_0b_date[11] = 0;
	set_0b_date[25] = 0x7d;



	set_0c_date[0] = 0x7e;
	set_0c_date[1] = 0x00;
	set_0c_date[2] = 0x18;
	set_0c_date[3] = 0x00;
	set_0c_date[4] = 0x01;
	set_0c_date[5] = 0x00;
	set_0c_date[6] = 0x00;
	set_0c_date[7] = 0x00;
	set_0c_date[8] = 0x00;
	set_0c_date[9] = 0x20;
	set_0c_date[10] = 0x0c;

	set_0c_date[12] = 0x01;
	set_0c_date[15] = 0x02;
	set_0c_date[18] = 0x03;
	set_0c_date[21] = 0x04;
	
	set_0c_date[11] = 0;
	set_0c_date[25] = 0x7d;



	for(i = 0;i < 24 ; i++)
	{
		set_14_date[i][0] = 0x7e;
		set_14_date[i][1] = 0x00;
		set_14_date[i][2] = 0x55;
		set_14_date[i][3] = 0x00;
		set_14_date[i][4] = 0x01;
		set_14_date[i][5] = 0x00;
		set_14_date[i][6] = 0x00;
		set_14_date[i][7] = 0x00;
		set_14_date[i][8] = 0x00;
		set_14_date[i][9] = 0x20;
		set_14_date[i][10] = 0x14;
		set_14_date[i][11] = i;
		set_14_date[i][12] = i + 1;
		set_14_date[i][86] = 0x7d;
	}


	
	for(i = 0;i < 128 ; i++)
	{
		set_0d_date[i][0] = 0x7e;
		set_0d_date[i][1] = 0x00;
		set_0d_date[i][2] = 0x5F;
		set_0d_date[i][3] = 0x00;
		set_0d_date[i][4] = 0x01;
		set_0d_date[i][5] = 0x00;
		set_0d_date[i][6] = 0x00;
		set_0d_date[i][7] = 0x00;
		set_0d_date[i][8] = 0x00;
		set_0d_date[i][9] = 0x20;
		set_0d_date[i][10] = 0x0d;
		set_0d_date[i][11] = i;
		set_0d_date[i][12] = i + 1;
		set_0d_date[i][96] = 0x7d;
	}

	for(i = 0;i < 24 ; i++)
	{
		set_15_date[i][0] = 0x7e;
		set_15_date[i][1] = 0x00;
		set_15_date[i][2] = 0x1d;
		set_15_date[i][3] = 0x00;
		set_15_date[i][4] = 0x01;
		set_15_date[i][5] = 0x00;
		set_15_date[i][6] = 0x00;
		set_15_date[i][7] = 0x00;
		set_15_date[i][8] = 0x00;
		set_15_date[i][9] = 0x20;
		set_15_date[i][10] = 0x15;
		set_15_date[i][11] = i;
		set_15_date[i][12] = i + 1;
		set_15_date[i][30] = 0x7d;
	}

	for(i = 0;i < 128 ; i++)
	{
		set_0e_date[i][0] = 0x7e;
		set_0e_date[i][1] = 0x00;
		set_0e_date[i][2] = 0x5F;
		set_0e_date[i][3] = 0x00;
		set_0e_date[i][4] = 0x01;
		set_0e_date[i][5] = 0x00;
		set_0e_date[i][6] = 0x00;
		set_0e_date[i][7] = 0x00;
		set_0e_date[i][8] = 0x00;
		set_0e_date[i][9] = 0x20;
		set_0e_date[i][10] = 0x0e;
		set_0e_date[i][11] = i;
		set_0e_date[i][12] = i + 1;
		set_0e_date[i][96] = 0x7d;
	}

	for(i = 0;i < 104 ; i++)
	{
		set_0f_date[i][0] = 0x7e;
		set_0f_date[i][1] = 0x00;
		set_0f_date[i][2] = 0x15;
		set_0f_date[i][3] = 0x00;
		set_0f_date[i][4] = 0x01;
		set_0f_date[i][5] = 0x00;
		set_0f_date[i][6] = 0x00;
		set_0f_date[i][7] = 0x00;
		set_0f_date[i][8] = 0x00;
		set_0f_date[i][9] = 0x20;
		set_0f_date[i][10] = 0x0f;
		set_0f_date[i][11] = i;
		set_0f_date[i][12] = i + 1;
		set_0f_date[i][22] = 0x7d;
	}

	set_10_date[0] = 0x7e;
	set_10_date[1] = 0x01;
	set_10_date[2] = 0x0c;
	set_10_date[3] = 0x00;
	set_10_date[4] = 0x01;
	set_10_date[5] = 0x00;
	set_10_date[6] = 0x00;
	set_10_date[7] = 0x00;
	set_10_date[8] = 0x00;
	set_10_date[9] = 0x20;
	set_10_date[10] = 0x10;
	set_10_date[11] = 0;
	set_10_date[269] = 0x7d;



	int m,n;
	for(m = 1; m<signal_number;m++ )
	{
		for(n = 0; n<14;n++)
		{
			signal_info_data[m].stop_C0_date[n] = C0_date[n];
		}
	}
	
}



void *pthread_function_flag_sql_error(void *arg)
{
	printf("new pthread, file=%s,func=%s,line=%d\n\n",__FILE__,__FUNCTION__,__LINE__);
	while(1)
	{
		if(flag_sql_error == -1)
		{
			env = Environment::createEnvironment(Environment::OBJECT);  
			

			string username = oracle_username;  
    			string userpass = oracle_password;  
    			string connstr = oracle_connectstring; 

			while(1)
			{
				try
				{
					conn_parsing = env->createConnection(username, userpass, connstr); 
					 if(conn_parsing == NULL)  
				    	{  
				  	  cout << "access oracle failed..." << endl;  
				   	 }  
				    
					st = conn_parsing->createStatement(); 
					st->setAutoCommit (TRUE);
					break;
				}
				catch (SQLException &sqlExcp)
			    	{
					 env->terminateConnection(conn_parsing);  
					Environment::terminateEnvironment(env);  
					printf("creat init occi_singal_Init_control_heart  oracle fail!\n");
			    	}
				sleep(1);
				
			}
			
		}

		flag_sql_error = 0;
		sleep(1);
	
	}
	
}

/*************************************************** 
 * ��������buf_check_num
 * �������������㱨��У���
 *�����ʵı�
 *���޸ĵı�
* ����˵������Ҫ����ı���
*����ֵ:  У��ֵ
***************************************************/
unsigned  char buf_check_num( unsigned  char* buf )
{
	int i ;
	int num  = 0;
	for(i = 1;i < buf[1] * 256 + buf[2];i++ )
	{
		num += buf[i];
	}

	return  (unsigned  char) (num & 0xff);
}


