/*************************************************** 
 * �ļ�����external_query.cpp
 * ��Ȩ��
 * ������׷�ӵ��ⲿ�ӿڣ��ɲ�ѯģʽ�ͽ׶�
 * �޸��ˣ�����
 * �޸�ʱ�䣺2014-1-7
 * �޸İ汾��V0.1
 * �޸����ݣ�
***************************************************/


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

#include "occi_opration.h"
#include "public_data.h"

#include "external_query.h"


using namespace std;
using namespace oracle::occi;


void function_external_query()
{

	
	
	int i,j;
	int sockfd;
	socklen_t peerlen;
	struct sockaddr_in peeraddr;

	peerlen = sizeof(peeraddr);




	char sql_exter[5000];
	


	Statement *st;
	ResultSet *rs ;
	Environment *env;
	Connection *conn_parsing ;
		
	unsigned char recv_buf[50];
	unsigned char send_buf[68];

	char lin_buf[10000];



	/*����UDP������*/
	
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

	
	bzero(&myaddr, sizeof(myaddr));
	myaddr.sin_family = PF_INET;
	myaddr.sin_port = htons(12345);
	myaddr.sin_addr.s_addr = INADDR_ANY;

	
	if ( bind( sockfd, (struct sockaddr *)&myaddr, sizeof(myaddr) ) < 0 )
	{
		perror("fail to bind");
		exit(-1);
	}

	int signal_num;
	

	/*�������ݿ�*/
		
	env = Environment::createEnvironment(Environment::OBJECT);  
	
//	string username = "atc";  
//    	string userpass = "atc";  
//    	string connstr = "192.168.1.24:1521/oracle";

	string username = oracle_username;  
    	string userpass = oracle_password;  
    	string connstr = oracle_connectstring; 


	send_buf[0] = 0x7e;
	send_buf[1] = 0x00;
	send_buf[2] = 0x42;
	send_buf[3] = 0x00;
	send_buf[4] = 0x01;
	send_buf[7] = 0xee;
	send_buf[67] = 0x7d;
	
	
	
	while(1)
	{
		try
		{
			conn_parsing	= env->createConnection(username, userpass, connstr);  
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
		      printf("parsing_signal_report_alarm   crart oracle falil!!\n");
	   	}
		sleep(1);
	}

	
	
#if 1	
	while(1)
	{
		printf("wait a buf :\n");
		bzero(recv_buf,sizeof(recv_buf));
		int num = recvfrom(sockfd, recv_buf, sizeof(recv_buf), 0, (struct sockaddr *)&peeraddr, &peerlen);

		if(num < 5)
		{
			/*���ݳ��Ȳ����׵�*/
			continue;
		}

		
		if(check_buf_external(recv_buf) == -1)
		{
			continue;
		}

		/*��ѯ����*/
		if(((recv_buf[5] * 256 + recv_buf[6]) == 0) &&(recv_buf[7]  == 0xee) )
		{

			for(i = 1;i <signal_number;i++)
			{

				send_buf[5]  = (signal_info_data[i].unit_id>> 8) & 0xff;
				send_buf[6]  =  signal_info_data[i].unit_id & 0xff;
				send_buf[8]  = (signal_info_data[i].signal_id >> 8) & 0xff;
				send_buf[9]  = signal_info_data[i].signal_id & 0xff;
				send_buf[10]  = (signal_info_data[i].unit_id>> 8) & 0xff;
				send_buf[11]  = signal_info_data[i].unit_id & 0xff;

				/*����*/
				if(signal_info_data[i].signal_state == 1)
				{
					send_buf[12]  = (signal_info_data[i].control_last_model  >> 8) & 0xff;
					send_buf[13]  = signal_info_data[i].control_last_model & 0xff;
					send_buf[14]  = (signal_info_data[i].cur_stage  >> 8) & 0xff;
					send_buf[15]  = signal_info_data[i].cur_stage & 0xff;

				//	printf("signal_num = %d, signal_info_data[signal_num].signal_id = %d,signal_info_data[signal_num].cur_stage = %d\n",i , signal_info_data[i].signal_id ,signal_info_data[i].cur_stage );
				//	printf("signal_info_data[signal_num].unit_id = %d\n",signal_info_data[i].unit_id);
					
					sprintf(sql_exter,"select File_name  from  STAGE_INFO    where signal_id = %d  and Stage_id = %d " ,signal_info_data[i].signal_id, signal_info_data[i].cur_stage);
					
			//		printf("%s\n",sql_exter);

					try
					{
						st->setSQL(sql_exter);
					
					rs = st->executeQuery();

					}
					catch (SQLException &sqlExcp)
					{
					     printf(" wai bu jie kou  sql chucuole !!!!!!\n");
					    
						
					}
												
					while(rs->next())  
					{  
						bzero(lin_buf,sizeof(lin_buf));
						strcpy(lin_buf, rs->getString(1).c_str() );
						printf("typian ==== %s\n",lin_buf);		
					} 
									
					st->closeResultSet(rs); 	
											
					for(j = 0; j < 50;j++)
					{
						send_buf[j+16] = lin_buf[j];
					}
				}
				else
				{
					

					for(j = 0; j < 54;j++)
					{
						send_buf[j+12] = 0;
					}
					
				}
				
				
				

				
				send_buf[66] = buf_check_external_num(send_buf);

				

				sendto(sockfd, send_buf, sizeof(send_buf), 0, (struct sockaddr*)&peeraddr, peerlen);

				usleep(1000);
			
				
			}
			
		}
		/*��ѯ����*/
		else if((recv_buf[5] * 256 + recv_buf[6]) > 0)
		{
			signal_num = check_unid_id(recv_buf[5] * 256 + recv_buf[6]);
			
		

			if(signal_num == -1)
			{
				continue;
			}

			if(recv_buf[7]  != 0xee)
			{
				continue;
			}

			send_buf[5]  = recv_buf[5];
			send_buf[6]  = recv_buf[6];
			send_buf[8]  = (signal_info_data[signal_num].signal_id >> 8) & 0xff;
			send_buf[9]  = signal_info_data[signal_num].signal_id & 0xff;
			send_buf[10]  = recv_buf[5];
			send_buf[11]  = recv_buf[6];

			/*����*/
			if(signal_info_data[signal_num].signal_state == 1)
			{
				send_buf[12]  = (signal_info_data[signal_num].control_last_model  >> 8) & 0xff;
				send_buf[13]  = signal_info_data[signal_num].control_last_model & 0xff;
				send_buf[14]  = (signal_info_data[signal_num].cur_stage  >> 8) & 0xff;
				send_buf[15]  = signal_info_data[signal_num].cur_stage & 0xff;

		//		printf("signal_num = %d, signal_info_data[signal_num].signal_id = %d,signal_info_data[signal_num].cur_stage = %d\n",signal_num , signal_info_data[signal_num].signal_id ,signal_info_data[signal_num].cur_stage );
		//		printf("signal_info_data[signal_num].unit_id = %d\n",signal_info_data[signal_num].unit_id);
				
				sprintf(sql_exter,"select File_name  from  STAGE_INFO    where signal_id = %d  and Stage_id = %d " ,signal_info_data[signal_num].signal_id, signal_info_data[signal_num].cur_stage);
				
		//		printf("%s\n",sql_exter);

				try
				{
					st->setSQL(sql_exter);
				
					rs = st->executeQuery();

				}
				catch (SQLException &sqlExcp)
				{
				     printf(" wai bu jie kou sql chucuole !!!!!!\n");
				    
					
				}
											
				while(rs->next())  
				{  
					bzero(lin_buf,sizeof(lin_buf));
					strcpy(lin_buf, rs->getString(1).c_str() );
		//			printf("typian ==== %s\n",lin_buf);		
				} 
								
				st->closeResultSet(rs); 	
										
				for(i = 0; i < 50;i++)
				{
					send_buf[i+16] = lin_buf[i];
				}
			}
			else
			{
				

				for(i = 0; i < 54;i++)
				{
					send_buf[i+12] = 0;
				}
				
			}
			
			
			

			
			send_buf[66] = buf_check_external_num(send_buf);

			

			sendto(sockfd, send_buf, sizeof(send_buf), 0, (struct sockaddr*)&peeraddr, peerlen);
			
		}
		/*�׵���������*/
		else
		{
			
		}


		
		

		
		
		
	}
		
#endif
		
	
}


/*************************************************** 
 * ��������check_buf_external
 * ��������������ⲿ���ͱ����Ƿ�����ȷ��
 *�����ʵı�
 *���޸ĵı�
* ����˵����BUF:��Ҫ���� ��������
*����ֵ: -1ʧ�� 0�ɹ�
***************************************************/
int check_buf_external( unsigned char * rcv_buf)
{
	int rcv_len = rcv_buf[1] * 256 + rcv_buf[2];
	int check_sum = 0;
	int i;
//	printf("%x %x \n",0x8e == rcv_buf[0],rcv_buf[rcv_len + 1]);
	 if (0x7e == rcv_buf[0] && 0x7d == rcv_buf[rcv_len + 1])
        {
             
		 for (i = 1; i < rcv_len; i++)
		{
			check_sum += rcv_buf[i];
		}

//		printf("check_sum & 0xFF = %x\n",(check_sum & 0xFF));
		if (rcv_buf[rcv_len] == (check_sum & 0xFF))
		{
                     return 0;
		}
		else
		{
			return -1;
			printf(" web  jiao yan cuo wu\n");
		}
		
         }
	 else
	 {
	 	
	 	printf(" web  tou wei cuo wu\n");
	 	return -1;	
	 }
}
	
/*************************************************** 
 * ��������check_unid_id
 * ��������������źŻ�·��ID ��signal_info_data�еı��
 *�����ʵı�
 *���޸ĵı�
* ����˵����signal_id �źŻ�·��ID
*����ֵ: �������еı�ţ��޷���-1
***************************************************/
int check_unid_id(int unid_id)
{
	int i ;
	for(i = 1;i < signal_number;i++)
	{
		if(unid_id == signal_info_data[i].unit_id)
		{
			return i;
		}
	}

	return -1;
}


/*************************************************** 
 * ��������buf_check_num
 * �������������㱨��У���
 *�����ʵı�
 *���޸ĵı�
* ����˵������Ҫ����ı���
*����ֵ:  У��ֵ
***************************************************/
unsigned  char buf_check_external_num( unsigned  char* buf )
{
	int i ;
	int num  = 0;
	for(i = 1;i < buf[1] * 256 + buf[2];i++ )
	{
		num += buf[i];
	}

	return  (unsigned  char) (num & 0xff);
}


