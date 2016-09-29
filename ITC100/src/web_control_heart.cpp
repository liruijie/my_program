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
#include "/home/ITC100/web_message_recv.h"

using namespace std;
using namespace oracle::occi;

#define BUFF_NUM  512



/*************************************************** 
 * 函数名：revice_web_control_heart
 * 功能描述：接收web发来的控制心跳 更新信号机控制状态
 *被访问的表：
 *被修改的表：CONTROL_CMD
* 参数说明：无
*返回值:  无
***************************************************/

void revice_web_control_heart()
{
	/*创建UDP服务器*/
	int sockfd;
	unsigned char recv_buf[BUFF_NUM] ;
	struct sockaddr_in myaddr, peeraddr;
	struct sockaddr_in signal_addr;
	 struct sockaddr SA;
	socklen_t peerlen;
	if ((sockfd = socket(PF_INET, SOCK_DGRAM, 0)) < 0)
	{
		perror("fail to socket");
		exit(-1);
	}
	   // 设置套接字选项避免地址使用错误  
    	int on=1;  
   	 if((setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(on)))<0)  
   	 {  
      	  	perror("setsockopt failed");  
       		 exit(EXIT_FAILURE);  
    	} 

	
	bzero(&myaddr, sizeof(myaddr));
	myaddr.sin_family = PF_INET;
	myaddr.sin_port = htons(10000);
	myaddr.sin_addr.s_addr = INADDR_ANY;
	//int bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
	if ( bind( sockfd, (struct sockaddr *)&myaddr, sizeof(myaddr) ) < 0 )
	{
		perror("fail to bind");
		exit(-1);
	}

	int signal_num;
	peerlen = sizeof(peeraddr);

	/*链接数据库*/
	time_t now;

	int i ,j;
	char sql[10000];

	Environment *env = Environment::createEnvironment(Environment::OBJECT);  
	
//	string username = "atc";  
//    	string userpass = "atc";  
//    	string connstr = "192.168.1.24:1521/oracle"; 

	string username = oracle_username;  
    	string userpass = oracle_password;  
    	string connstr = oracle_connectstring; 
		
	Connection *conn_parsing ;
	Statement *st;
	ResultSet *rs ;
	printf("new pthread, file=%s,func=%s,line=%d\n\n",__FILE__,__FUNCTION__,__LINE__);	
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
		

	
#if 1
	struct timeval tv_out;
       tv_out.tv_sec = 0;//等待5秒设置为0则阻塞
       tv_out.tv_usec = 0;
	setsockopt(sockfd,SOL_SOCKET,SO_RCVTIMEO,&tv_out, sizeof(tv_out));
#endif





	while(1)
	{
		
		tv_out.tv_sec = 0;//等待5秒设置为0则阻塞
		tv_out.tv_usec = 0;
		setsockopt(sockfd,SOL_SOCKET,SO_RCVTIMEO,&tv_out, sizeof(tv_out));
		

			while(1)
			{
				try
				{
					/*******************/
									
					bzero(recv_buf,sizeof(recv_buf));
					int num = recvfrom(sockfd, recv_buf, BUFF_NUM, 0, (struct sockaddr *)&peeraddr, &peerlen);
					
					
				#if 0
					int lll;
					printf("recv kongzhixintiao:");
					for(lll = 0; lll < num;lll++)
					{
						printf("%02x",recv_buf[lll]);
					}
					printf("\n");
					printf("%s",inet_ntoa(peeraddr.sin_addr));
				#endif

					sendto(sockfd, recv_buf, num , 0, (struct sockaddr*)&peeraddr, sizeof(peeraddr));
					now = time(NULL);

					signal_num = check_signal_id(recv_buf[7] * 256 + recv_buf[8]);

					//printf("recv revice_web_control_heart\n");
					//printf("recv_buf[9] = %x   recv_buf[10]= %d \n", recv_buf[9], recv_buf[10]);
					//printf("signal_info_data[signal_num].signal_control_flag= %d \n", signal_info_data[signal_num].signal_control_flag);

					bzero(&signal_addr, sizeof(signal_addr));
					signal_addr.sin_family = PF_INET;
					
					signal_addr.sin_port = htons(signal_info_data[signal_num].signal_port);
					signal_addr.sin_addr.s_addr = inet_addr(signal_info_data[signal_num].signal_ip);
						
				//	printf("control heart 114\n");
					if(check_buf_web(recv_buf) == 0)
					{
					//	printf("check_buf_web(recv_buf) == 0 \n");
						if(recv_buf[9] == 0x60)
						{
						//	printf("0x60!!!!!!!!!!\n");
							/*申请控制*/
							if(recv_buf[10] == 0)
							{
							//	printf("recv_buf[11] == 0\n");
								/*没有被控制*/
								if(signal_info_data[signal_num].signal_control_flag == 0  || signal_info_data[signal_num].signal_control_flag == 3)
								{
									bzero(sql,sizeof(sql));
									sprintf(sql,"update CONTROL_CMD set control_flag = 1 where signal_id = %d ",recv_buf[7] * 256 + recv_buf[8]);
									printf("%s\n",sql);
									st->setSQL(sql);
									st->executeUpdate();
								}
								signal_info_data[signal_num].signal_control_flag = 1;
								signal_info_data[signal_num].control_d1_flag = 0;
								signal_info_data[signal_num].signal_control_time = now;
							}
							/*结束控制*/
							else
							{
							//	printf("signal_info_data[signal_num].signal_control_flag = %d\n",signal_info_data[signal_num].signal_control_flag);
								/*已经被控制*/
								if(signal_info_data[signal_num].signal_control_flag == 1)
								{
									/*上一次控制是0*/
									if(signal_info_data[signal_num].signal_last_control_flag == 0)
									{
										bzero(sql,sizeof(sql));
										sprintf(sql,"update CONTROL_CMD set control_flag = 0 where signal_id = %d ",recv_buf[7] * 256 + recv_buf[8]);
								//		printf("%s\n",sql);
										st->setSQL(sql);
										st->executeUpdate();
										signal_info_data[signal_num].signal_control_flag = 0;
										signal_info_data[signal_num].control_d1_flag = 0;
									}
									/*上一次控制是3 滤波*/
									else if(signal_info_data[signal_num].signal_last_control_flag == 3)
									{
										bzero(sql,sizeof(sql));
										sprintf(sql,"update CONTROL_CMD set control_flag = 3 where signal_id = %d ",recv_buf[7] * 256 + recv_buf[8]);
										printf("%s\n",sql);
										st->setSQL(sql);
										st->executeUpdate();
										signal_info_data[signal_num].signal_control_flag = 3;

										tv_out.tv_sec = 2;//等待2秒设置为0则阻塞
					 					 tv_out.tv_usec = 0;
										setsockopt(sockfd,SOL_SOCKET,SO_RCVTIMEO,&tv_out, sizeof(tv_out));

										signal_info_data[signal_num].stop_C0_date[7] = recv_buf[7];
										signal_info_data[signal_num].stop_C0_date[8] = recv_buf[8];
										signal_info_data[signal_num].stop_C0_date[12]  = buf_check_num(signal_info_data[signal_num].stop_C0_date);
										/*补发一次停止手动*/
										sendto(sockfd, signal_info_data[signal_num].stop_C0_date, sizeof(signal_info_data[signal_num].stop_C0_date) , 0, (struct sockaddr*)&signal_addr, sizeof(signal_addr));

										 num = recvfrom(sockfd, recv_buf, BUFF_NUM, 0, (struct sockaddr *)&peeraddr, &peerlen);
										 if(num <= 0)
										 {
										 	printf(" jieshu  zai ci  lv bo shibai \n");
										 }
										 else
										 {
										 	printf("  jieshu zai ci  lv bo chenggong \n");
										 }


										

										/*补发一次滤波控制*/
										sendto(sockfd, signal_info_data[signal_num].last_C6_date_lin, sizeof(signal_info_data[signal_num].last_C6_date_lin) , 0, (struct sockaddr*)&signal_addr, sizeof(signal_addr));

										 num = recvfrom(sockfd, recv_buf, BUFF_NUM, 0, (struct sockaddr *)&peeraddr, &peerlen);
										 if(num <= 0)
										 {
										 	printf(" jieshu  zai ci  lv bo shibai \n");
										 }
										 else
										 {
										 	printf("  jieshu zai ci  lv bo chenggong \n");
										 }
									}
									
								}
								
							}
						}
						
					}
					else
					{
						printf("it is a bad  message\n");
					}

					
					/*******************/
					break;
				}
				catch (SQLException &sqlExcp)
			    	{
					 env->terminateConnection(conn_parsing);  
					Environment::terminateEnvironment(env);  
					while(1)
					{
						try
						{
						//	 username = "atc";  
	    					//	userpass = "atc";  
	    					//	connstr = "192.168.1.24:1521/oracle"; 

							username = oracle_username;  
    							userpass = oracle_password;  
    							connstr = oracle_connectstring; 
								
							env = Environment::createEnvironment(Environment::OBJECT); 
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

						printf("break!!!!!!!!!!!!\n");
						sleep(1);
					}
			    	}
				sleep(1);
				
			}
		

		
	}
	
}


/*************************************************** 
 * 函数名：check_buf_web
 * 功能描述：检查BUF报文是否是正确的
 *被访问的表：
 *被修改的表：
* 参数说明：BUF:需要检查的 报文数组
*返回值: -1失败 0成功
***************************************************/
int check_buf_web( unsigned char * rcv_buf)
{
	int rcv_len = rcv_buf[1] * 256 + rcv_buf[2];
	int check_sum = 0;
	int i;
//	printf("%x %x \n",0x8e == rcv_buf[0],rcv_buf[rcv_len + 1]);
	 if (0x8e == rcv_buf[0] && 0x8d == rcv_buf[rcv_len + 1])
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
	
