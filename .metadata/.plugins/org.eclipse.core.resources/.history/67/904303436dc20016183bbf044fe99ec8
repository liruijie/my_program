#include <iostream>
#include <string.h>
#include <occi.h>
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include "/home/ITC100/signal_report.h"
#include "/home/ITC100/occi_opration.h"
#include "/home/ITC100/public_data.h"
#include "/home/ITC100/queue.h"

using namespace std;
using namespace oracle::occi;

#define BUFF_NUM  512

/*信号机初始信息*/
 signal_info signal_info_data[512];
/*信号机总个数*/
int signal_number;
/*数据库操作句柄*/
 Statement *stmt ;

/*队列结构*/
sequeue *sq;

void receive_signal_report()
{
	
	receive_signal_report_udp_server(10086);
}

void receive_signal_report_udp_server(int port)
{
	time_t now;
	int sockfd;
	unsigned char recv_buf[BUFF_NUM] ;
	struct sockaddr_in myaddr, peeraddr;
	 struct sockaddr SA;
	socklen_t peerlen;

	Statement *st;
	Connection *conn_parsing ;

	Environment *env = Environment::createEnvironment(Environment::OBJECT); 
	string username = oracle_username;  
    	string userpass = oracle_password;  
    	string connstr = oracle_connectstring; 
	printf("new pthread, file=%s,func=%s,line=%d\n\n",__FILE__,__FUNCTION__,__LINE__);		
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
		      printf("signal_report  crart oracle falil!!\n");
	   	}
		sleep(1);
	}
	
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
	
#if 0
	struct timeval tv_out;
       tv_out.tv_sec = 10;//等待10秒设置为0则阻塞
       tv_out.tv_usec = 0;
	setsockopt(sockfd,SOL_SOCKET,SO_RCVTIMEO,&tv_out, sizeof(tv_out));
#endif

	bzero(&myaddr, sizeof(myaddr));
	myaddr.sin_family = PF_INET;
	myaddr.sin_port = htons(port);
	myaddr.sin_addr.s_addr = INADDR_ANY;
	//int bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
	if ( bind( sockfd, (struct sockaddr *)&myaddr, sizeof(myaddr) ) < 0 )
	{
		perror("fail to bind");
		exit(-1);
	}

	int signal_num;
	peerlen = sizeof(peeraddr);
	while ( 1 )
	{	
		now = time(NULL);
		bzero(recv_buf,sizeof(recv_buf));
		int num = recvfrom(sockfd, recv_buf, BUFF_NUM, 0, (struct sockaddr *)&peeraddr, &peerlen);
		int i;
#if 0		
		printf("recv buf from signal:");
		for(i = 0; i < num;i++)
		{
			printf("%02X  ",recv_buf[i]);
		}
		printf("\n");
#endif		
		if(check_buf(recv_buf) == 0)
		{
			recv_buf[8] = (recv_buf[8] & 0xff);

	//		printf("signal_id = %d\n",recv_buf[7] * 256 + recv_buf[8]);
			signal_num = check_signal_id(recv_buf[7] * 256 + recv_buf[8]);
	//		printf(" signal_num = %d,check_signal_id  = %d \n", signal_num,recv_buf[7] * 256 + recv_buf[8]);
	//		printf(" %d  %d \n", recv_buf[7] , recv_buf[8]);
			
			if(signal_num == -1)
			{
				printf("check_signal_id  fail\n");
				continue;
			}
			else
			{
				if(recv_buf[9] == 0x40)
				{
				//	printf("recv_buf[10] = %x\n",recv_buf[10]);
					signal_info_data[signal_num].last_time = now;
				
					/*实时数据*/
					
					if(recv_buf[10] == 0x50)                    
					{
					//	printf("now = %d\n",now);

					//	printf("now = %d\n",now);
						parsing_recv_buf(signal_num,recv_buf);
					}

					//ITC100, 主动上报流量

					else if(recv_buf[10] == 0x44 || recv_buf[10] == 0x45)
					{
						//int detector_num = recv_buf[11];

						//时间
						char updateTime[30],sql[500];
						struct tm *p;
						time_t seconds =(time_t)( recv_buf[16]*256*256*256 +recv_buf[17]*256*256 +recv_buf[18]*256+recv_buf[19]);

						//p = localtime(&seconds);
						p = gmtime(&seconds);	
						sprintf(updateTime,"%d-%02d-%02d %02d:%02d:%02d",(1900+p->tm_year),(1+p->tm_mon),p->tm_mday,p->tm_hour,p->tm_min,p->tm_sec);
						
						long volume,occu;
						if(recv_buf[10] == 0x44)
						{
							volume = recv_buf[12]*256*256*256 +recv_buf[13]*256*256 +recv_buf[14]*256+recv_buf[15];
							sprintf(sql,"insert into DETECTOR_REAL_DATA(UNIT_ID,DETECTOR_ID,UPDATE_TIME,FLOW) values(%d,%d,to_date('%s','yyyy-mm-dd hh24:mi:ss'),%d)",
								signal_info_data[signal_num].unit_id,recv_buf[11],updateTime,volume);
							
						}
						else if(recv_buf[10] == 0x45)
						{
							occu = recv_buf[12]*256*256*256 +recv_buf[13]*256*256 +recv_buf[14]*256+recv_buf[15];
							sprintf(sql,"insert into DETECTOR_REAL_DATA(UNIT_ID,DETECTOR_ID,UPDATE_TIME,OCC) values(%d,%d,to_date('%s','yyyy-mm-dd hh24:mi:ss'),%d)",
								signal_info_data[signal_num].unit_id,recv_buf[11],updateTime,occu);
						}


						st->setSQL(sql);
						st->executeUpdate();

					}
						
					/*报警数据入列*/
					//else if(recv_buf[10] == 0x80 || recv_buf[10] == 0x81 || recv_buf[10] == 0x82 || recv_buf[10] == 0x83 || recv_buf[10] == 0x85 || recv_buf[10] == 0x87 || recv_buf[10] == 0x89 || recv_buf[10] == 0xA0 || recv_buf[10] == 0xA2|| recv_buf[10] == 0x8A)
					else if((recv_buf[10] >= 0x80 &&  recv_buf[10] <= 0x8A ) || (recv_buf[10] >= 0xA0&&  recv_buf[10] <= 0xA5))
					{ 
						printf(">>> recv_buf[9] = %02x,recv_buf[10] = %02x,recv_buf[14] = %02x,\n",recv_buf[9],recv_buf[10],recv_buf[14]);
						//printf("en_queue(&queue,recv_buf);!!!!!!!\n");
						enqueue(sq, recv_buf);
						//printf("en_queue(&queue,recv_buf);   chenggong \n");
					}
					/*占有率 流量数据入列*/
					else if(recv_buf[10]  == 0x41 || recv_buf[10]  == 0x40)
					{
					//	printf("this is a 40 and 41\n");
#if 0
						int ll_w;
						printf("recv buf:");
						for(ll_w = 0;ll_w <num;ll_w++)
						{
							printf("%02x",recv_buf[ll_w]);
						}
						printf("\n");
#endif
						enqueue(sq, recv_buf);
					}
					
				}
				
			}
		}
		else
		{
			printf("check_buf  fail\n");
			continue;
		}
#if 0		
		printf("recv from [%s:%d] : %s\n", inet_ntoa(peeraddr.sin_addr), ntohs(peeraddr.sin_port), recv_buf);
		strcpy((char*)recv_buf, "123123");
		sendto(sockfd, recv_buf, BUFF_NUM, 0, (struct sockaddr*)&peeraddr, sizeof(peeraddr));
#endif		
	}
}


/*************************************************** 
 * 函数名：check_buf
 * 功能描述：检查BUF报文是否是正确的
 *被访问的表：
 *被修改的表：
* 参数说明：BUF:需要检查的 报文数组
*返回值: -1失败 0成功
***************************************************/
int check_buf( unsigned char * rcv_buf)
{
	int rcv_len = rcv_buf[1] * 256 + rcv_buf[2];
	int check_sum = 0;
	int i;
	 if (0x7e == rcv_buf[0] && 0x7d == rcv_buf[rcv_len + 1])
        {
             
		 for (i = 1; i < rcv_len; i++)
		{
			check_sum += rcv_buf[i];
		}

		if (rcv_buf[rcv_len] == (check_sum & 0xFF))
		{
                     return 0;
		}
		else
		{
			return -1;
			printf(" jiao yan cuo wu\n");
		}
		
         }
	 else
	 {
	 	printf(" signal tou wei cuo wu\n");
	 	return -1;	
	 }
}

/*************************************************** 
 * 函数名：check_signal_id
 * 功能描述：检查信号机ID 在signal_info_data中的编号
 *被访问的表：
 *被修改的表：
* 参数说明：signal_id 信号机ID
*返回值: 在数组中的编号，无返回-1
***************************************************/
int check_signal_id(int signal_id)
{
	int i ;
	for(i = 1;i < signal_number;i++)
	{
		if(signal_id == signal_info_data[i].signal_id)
		{
			return i;
		}
	}

	return -1;
}

/*************************************************** 
 * 函数名：parsing_recv_buf
 * 功能描述：把数据解析存入公共结构体数组中
 *被访问的表：
 *被修改的表：
* 参数说明：signal_num: 在公共结构体中的编号
					buf:接收的报文
*返回值: 失败返回-1,成功返回0
***************************************************/
int parsing_recv_buf(int signal_num, unsigned char * buf)
{
	time_t now;
	now = time(NULL);
	signal_info_data[signal_num].signal_id = buf[7] * 256 + buf[8];
	//signal_info_data[signal_num].last_time = now;

	int i ;
	int j = 0;

	if(strcmp(signal_info_data[signal_num].device_type,"TC100")== 0  &&buf[1] == 0x00 &&  buf[2] == 0x4d)
	{

	/*处理板状态*/
			for(i = 8; i > 0; i--)
			{
				if((buf[12] & (0x01 <<  (i -1)) ) == 0)
				{	
					signal_info_data[signal_num].board_state[j  * 2 ] = '0';
					signal_info_data[signal_num].board_state[j  * 2  + 1] = '\,';
				}
				else
				{
					signal_info_data[signal_num].board_state[j  * 2 ] = '1';
					signal_info_data[signal_num].board_state[j  * 2  + 1] = '\,';
				}
		
				j++;
			}

			char a[2];
			
			/*处理灯态*/
			bzero(signal_info_data[signal_num].phase_lht,sizeof(signal_info_data[signal_num].phase_lht));
			for(i = 0; i < 16 ; i++)
			{
				//printf("buf[%d] = %d\n",i,buf[13 + i]);
				sprintf(a,"%d",buf[13 + i]);
		
				strcat((char*)signal_info_data[signal_num].phase_lht,a);
				strcat((char *)signal_info_data[signal_num].phase_lht,",");
				//signal_info_data[signal_num].phase_lht[i  * 2 ] =  buf[13 + i];
				//signal_info_data[signal_num].phase_lht[i  * 2  + 1] = '\,';
			}
	
//	printf("!!!!!!!!!!!signal_info_data[signal_num].phase_lht = %s\n",signal_info_data[signal_num].phase_lht);
	/*处理检测器状态*/
			int m = 0;
			
			for(i = 0; i < 13 ; i++)
			{
						for(j = 0;j < 8; j++)
						{
									if((buf[29 +  i] &(0x80 >>  j)) == 0  )
									{
										signal_info_data[signal_num].detect_status[m * 2]  = '0';
										signal_info_data[signal_num].detect_status[m * 2 + 1]  = '\,';
									}
									else
									{
										signal_info_data[signal_num].detect_status[m * 2]  = '1';
										signal_info_data[signal_num].detect_status[m * 2 + 1]  = '\,';
									}
									m++;
						}
			}
	//printf("!!!!!!!!!!!chuliqi zhuangtai = %s\n",signal_info_data[signal_num].detect_status);

	
			/*处理检测器数据*/
			m = 0;
			for(i = 0; i < 13 ; i++)
			{
				for(j = 0;j < 8; j++)
				{
					if((buf[42 +  i] &(0x80 >>  j)) == 0  )
					{
						signal_info_data[signal_num].detect_data_status[m * 2]  = '0';
						signal_info_data[signal_num].detect_data_status[m * 2 + 1]  = '\,';
					}
					else
					{
						signal_info_data[signal_num].detect_data_status[m * 2]  = '1';
						signal_info_data[signal_num].detect_data_status[m * 2 + 1]  = '\,';
					}
					m++;
				}
			}

			/*处理紧急输入状态*/
			m = 0;
			for(i = 0; i < 4 ; i++)
			{
				if((buf[55] &(0x80 >>  i)) == 0  )
				{
						signal_info_data[signal_num].detect_urgent_status[m * 2]  = '0';
						signal_info_data[signal_num].detect_urgent_status[m * 2 + 1]  = '\,';
				}
				else
				{
						signal_info_data[signal_num].detect_urgent_status[m * 2]  = '1';
						signal_info_data[signal_num].detect_urgent_status[m * 2 + 1]  = '\,';
				}
				m++;
			}

	/*处理优先输入状态*/
			m = 0;
			for(i = 0; i < 4 ; i++)
			{
				if((buf[55] &(0x08 >>  i)) == 0  )
				{
						signal_info_data[signal_num].detect_pri_status[m * 2]  = '0';
						signal_info_data[signal_num].detect_pri_status[m * 2 + 1]  = '\,';
				}
				else
				{
						signal_info_data[signal_num].detect_pri_status[m * 2]  = '1';
						signal_info_data[signal_num].detect_pri_status[m * 2 + 1]  = '\,';
				}
				m++;
			}

	/*处理门状态*/
			m = 0;
			for(i = 0; i < 2 ; i++)
			{
				if((buf[56] &(0x80 >>  i)) == 0  )
				{
						signal_info_data[signal_num].door_status[m * 2]  = '0';
						signal_info_data[signal_num].door_status[m * 2 + 1]  = '\,';
				}
				else
				{
						signal_info_data[signal_num].door_status[m * 2]  = '1';
						signal_info_data[signal_num].door_status[m * 2 + 1]  = '\,';
				}
				m++;
			}

	/*处理方案数据*/
			if(signal_info_data[signal_num].control_model != 0x00)
				signal_info_data[signal_num].control_last_model = signal_info_data[signal_num].control_model;//记录上一次运行模式
			signal_info_data[signal_num].control_model = buf[57];
			signal_info_data[signal_num].cur_plan = buf[58];
			signal_info_data[signal_num].cur_week = buf[59] * 256 + buf[60];
			signal_info_data[signal_num].cur_stage = buf[61];
			signal_info_data[signal_num].coordination_number = buf[62];
			signal_info_data[signal_num].coordination_time = buf[63] * 256 + buf[64];
			signal_info_data[signal_num].release_time = buf[65] * 256 + buf[66];
			signal_info_data[signal_num].rest_time = buf[67] * 256 + buf[68];
		
			signal_info_data[signal_num].signal_info_state = 1;
			signal_info_data[signal_num].oracleFlag[0]= 1;
			signal_info_data[signal_num].oracleFlag[1]= 1;
			/*处理更新时间*/
			sprintf(signal_info_data[signal_num].update_time,"%d-%d-%d %d:%d:%d",buf[69] * 256 + buf[70],buf[71],buf[72],buf[73],buf[74],buf[75]);


//	printf("signal_num = %d,signal_info_data[signal_num].cur_stage = %d,signal_info_data[signal_num].unit_id = %d\n",signal_num,signal_info_data[signal_num].cur_stage,signal_info_data[signal_num].unit_id);
//	printf("%d     %d-%d-%d %d:%d:%d",buf[69] ,buf[70],buf[71],buf[72],buf[73],buf[74],buf[75]);
//	printf("signal_info_data[signal_num].update_time = %s\n",signal_info_data[signal_num].update_time);
	//printf(" id =%d ,signal_info_state = %d\n",signal_info_data[signal_num].signal_id,signal_info_data[signal_num].signal_info_state);
	//printf(" id =%d ,ban zhuangtai = %s\n",signal_info_data[signal_num].signal_id,signal_info_data[signal_num].board_state);
		}

	else if(strcmp(signal_info_data[signal_num].device_type , "ITC100")== 0 && buf[1] == 0x01 && buf[2] == 0x02)
	{
		/*模块状态，4个字节*/
		int k;
		bzero(signal_info_data[signal_num].board_state,sizeof(signal_info_data[signal_num].board_state));

		for(k = 0;k < 3; k ++) //第一个字节为空
		{
			for(i = 8; i > 0; i--)
			{
				if((buf[13+k] & (0x01 <<  (i -1)) ) == 0)
				{	
					signal_info_data[signal_num].board_state[j  * 2 ] = '0';
					signal_info_data[signal_num].board_state[j  * 2  + 1] = '\,';
				}
				else
				{
					signal_info_data[signal_num].board_state[j  * 2 ] = '1';
					signal_info_data[signal_num].board_state[j  * 2  + 1] = '\,';
				}

				j++;
			}
		}

		//相位灯态 ,从第16字节开始
		char a[2];
		bzero(signal_info_data[signal_num].phase_lht,sizeof(signal_info_data[signal_num].phase_lht));
	for(i = 0; i < 32 ; i++)
	{
		//printf("buf[%d] = %d\n",i,buf[13 + i]);
		sprintf(a,"%d",buf[16 + i]);

		strcat((char*)signal_info_data[signal_num].phase_lht,a);
		strcat((char *)signal_info_data[signal_num].phase_lht,",");
		//signal_info_data[signal_num].phase_lht[i  * 2 ] =  buf[13 + i];
		//signal_info_data[signal_num].phase_lht[i  * 2  + 1] = '\,';
	}

	//相位电流，每个相位，对应3个电流 
	bzero(signal_info_data[signal_num].phase_current,sizeof(signal_info_data[signal_num].phase_current));
	char aa[3];
	for(i = 0; i < 96 ; i++)
	{
		sprintf(aa,"%d",buf[48 + i]);
		strcat((char*)signal_info_data[signal_num].phase_current,aa);
		strcat((char *)signal_info_data[signal_num].phase_current,":");	
		i++;
		sprintf(aa,"%d",buf[48 + i]);
		strcat((char*)signal_info_data[signal_num].phase_current,aa);
		strcat((char *)signal_info_data[signal_num].phase_current,":");	
		i++;
		sprintf(aa,"%d",buf[48 + i]);
		strcat((char*)signal_info_data[signal_num].phase_current,aa);
		strcat((char *)signal_info_data[signal_num].phase_current,",");	
	}
	//printf("dian liu::%s\n\n",signal_info_data[signal_num].phase_current);
	//检测器状态，4个字节
	int m = 0;
	
	for(i = 0; i < 4 ; i++)
	{
		for(j = 0;j < 8; j++)
		{
			if((buf[144 +  i] &(0x80 >>  j)) != 0  )
			{
				signal_info_data[signal_num].detect_status[m * 2]  = '0';
				signal_info_data[signal_num].detect_status[m * 2 + 1]  = '\,';
			}
			else
			{
				signal_info_data[signal_num].detect_status[m * 2]  = '1';
				signal_info_data[signal_num].detect_status[m * 2 + 1]  = '\,';
			}
			m++;
		}
	}
	//printf("!!!!!!!!!!!chuliqi zhuangtai = %s\n",signal_info_data[signal_num].detect_status);

	
	/*处理检测器数据*/
	m = 0;
	for(i = 0; i < 4 ; i++)
	{
		for(j = 0;j < 8; j++)
		{
			if((buf[148 +  i] &(0x80 >>  j)) == 0  )
			{
				signal_info_data[signal_num].detect_data_status[m * 2]  = '0';
				signal_info_data[signal_num].detect_data_status[m * 2 + 1]  = '\,';
			}
			else
			{
				signal_info_data[signal_num].detect_data_status[m * 2]  = '1';
				signal_info_data[signal_num].detect_data_status[m * 2 + 1]  = '\,';
			}
			m++;
		}
	}

	/*处理紧急输入状态*/
	m = 0;
	for(i = 0; i < 4 ; i++)
	{
		if((buf[152] &(0x80 >>  i)) == 0  )
		{
				signal_info_data[signal_num].detect_urgent_status[m * 2]  = '0';
				signal_info_data[signal_num].detect_urgent_status[m * 2 + 1]  = '\,';
		}
		else
		{
				signal_info_data[signal_num].detect_urgent_status[m * 2]  = '1';
				signal_info_data[signal_num].detect_urgent_status[m * 2 + 1]  = '\,';
		}
		m++;
	}

	/*处理优先输入状态*/
	m = 0;
	for(i = 0; i < 4 ; i++)
	{
		if((buf[152] &(0x08 >>  i)) == 0  )
		{
				signal_info_data[signal_num].detect_pri_status[m * 2]  = '0';
				signal_info_data[signal_num].detect_pri_status[m * 2 + 1]  = '\,';
		}
		else
		{
				signal_info_data[signal_num].detect_pri_status[m * 2]  = '1';
				signal_info_data[signal_num].detect_pri_status[m * 2 + 1]  = '\,';
		}
		m++;
	}

	/*处理门状态,前门、后门、侧门*/
	m = 0;
	for(i = 0; i < 3 ; i++)
	{
		if((buf[153] &(0x01 << i)) == 0  )
		{
				signal_info_data[signal_num].door_status[m * 2]  = '0';
				signal_info_data[signal_num].door_status[m * 2 + 1]  = '\,';
		}
		else
		{
				signal_info_data[signal_num].door_status[m * 2]  = '1';
				signal_info_data[signal_num].door_status[m * 2 + 1]  = '\,';
		}
		m++;
	}

	/*处理方案数据*/
	if(signal_info_data[signal_num].control_model != 0x00)
		signal_info_data[signal_num].control_last_model = signal_info_data[signal_num].control_model;//记录上一次运行模式
	signal_info_data[signal_num].control_model = buf[158];
	signal_info_data[signal_num].cur_plan = buf[159];
	signal_info_data[signal_num].cur_week = (buf[160] * 256 + buf[161])/10;
	signal_info_data[signal_num].cur_stage = buf[162];
	signal_info_data[signal_num].coordination_number = buf[163];
	signal_info_data[signal_num].coordination_time = (buf[164] * 256 + buf[165])/10;
	signal_info_data[signal_num].release_time = (buf[166] * 256 + buf[167])/10;
	signal_info_data[signal_num].rest_time = (buf[168] * 256 + buf[169])/10;

	signal_info_data[signal_num].signal_info_state = 1;
	signal_info_data[signal_num].oracleFlag[0]= 1;
	signal_info_data[signal_num].oracleFlag[1]= 1;
	/*处理更新时间*/
	//sprintf(signal_info_data[signal_num].update_time,"%d-%d-%d %d:%d:%d",buf[254] * 256 + buf[255],buf[256],buf[257],buf[258],buf[259],buf[260]);
	struct tm *p;
	time_t signalTime = (time_t)(buf[254] *256*256*256 + buf[255]*256*256 + buf[256]*256 +buf[257]);
	//p = localtime(&signalTime);
	p = gmtime(&signalTime);
	sprintf(signal_info_data[signal_num].update_time,"%d-%d-%d %d:%d:%d",(1900+p->tm_year),(1+p->tm_mon),p->tm_mday,p->tm_hour,p->tm_min,p->tm_sec);
	
	}
}
