/*************************************************** 
 * 文件名：signal_timeout_check.cpp
 * 版权：
 * 描述：处理检查信号机在线离线状态
 * 修改人：栾宇
 * 修改时间：2014-1-7
 * 修改版本：V0.1
 * 修改内容：
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
#include <unistd.h>
#include "/home/ITC100/signal_report.h"
#include "/home/ITC100/occi_opration.h"
#include "/home/ITC100/public_data.h"
#include "/home/ITC100/parsing_report_alalrm.h"
#include "/home/ITC100/signal_timeout_check.h"
#include "/home/ITC100/web_message_recv.h"

using namespace std;
using namespace oracle::occi;




/*************************************************** 
 * 函数名：signal_timeout_check
 * 功能描述：检测信号机在线状态
 *被访问的表：
 *被修改的表：ATC_ALARMLIST  ATC_ALARMHIS_YYMM
* 参数说明：无
*返回值: 无
***************************************************/
void signal_timeout_check()
{
	
	int sockfd_recv;
	unsigned char recv_buf[512] ;
	struct sockaddr_in signal_addr;
	 struct sockaddr SA;
	socklen_t peerlen;
	
	
	if ((sockfd_recv = socket(PF_INET, SOCK_DGRAM, 0)) < 0)
	{
		perror("fail to socket bind");
		return ;
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
       tv_out.tv_sec = 2;//等待2秒设置为0则阻塞
       tv_out.tv_usec = 0;
	setsockopt(sockfd_recv,SOL_SOCKET,SO_RCVTIMEO,&tv_out, sizeof(tv_out));
#endif




	int i ,j;
	time_t now;
	char sql[10000];
	char alarm_time[100];

	time_t rawtime;
	struct tm * timeinfo;

	char nonth[2];
	Connection *conn_parsing;
	Statement *st;
	ResultSet *rs ;

	
	
	Environment *env = Environment::createEnvironment(Environment::OBJECT);  
//	string username = "atc";  
//    	string userpass = "atc";  
//    	string connstr = "192.168.1.24:1521/oracle"; 

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
	}
		

	
	while(1)
	{
		now = time(NULL);
		time ( &rawtime );
		timeinfo = localtime ( &rawtime );
		

		while(1)
		{
		
			try
			{
#if 1 			
				/**************************/
				/*处理信号机超时*/
				for(i = 1; i <signal_number;i++ )
				{
					//printf("id = %d  now = %d  singnal_now = %d",signal_info_data[i].signal_id,now,signal_info_data[i].last_time);
					if((now -  signal_info_data[i].last_time) > 20 )
					{
						/*信号机超时*/
						if(signal_info_data[i].signal_state == 1)
						{
							if(signal_info_data[i].l_online_alarm == 0)
							{
								
								bzero(signal_info_data[i].l_online_alarm_time,sizeof(signal_info_data[i].l_online_alarm_time));
								sprintf(signal_info_data[i].l_online_alarm_time,"%d-%d-%d %d:%d:%d",1900+timeinfo->tm_year, 1+timeinfo->tm_mon,timeinfo->tm_mday,timeinfo->tm_hour,timeinfo->tm_min,timeinfo->tm_sec);
								
								bzero(sql,sizeof(sql));
								
								sprintf(sql,"insert into ATC_ALARMLIST(Sn,device_id,alarm_id,processstatus_id,occur_time,faultcomment,unit_id) values((select nvl(max(t.Sn)+1,1) from ATC_ALARMLIST t),%d,1,0,sysdate,' &#20449;&#21495;&#26426;%d&#31163;&#32447; ',%d) ",signal_info_data[i].signal_id,signal_info_data[i].signal_id,signal_info_data[i].unit_id);
								
								printf("%s\n",sql);
								st->setSQL(sql);
								st->executeUpdate();


								if((1+timeinfo->tm_mon) < 10)
								{
									sprintf(nonth,"0%d",1+timeinfo->tm_mon);
								}
								else
								{
									sprintf(nonth,"%d",1+timeinfo->tm_mon);
								}
							
								sprintf(sql,"insert into ATC_ALARMHIS_%d%s(Sn,deviceid,alarm_id,faultcomment,unit_id,type,Report_time) values((select nvl(max(t.Sn)+1,1) from ATC_ALARMHIS_%d%s t), %d,%d,' &#20449;&#21495;&#26426;%d&#31163;&#32447; ',%d,1,sysdate) ",
								1900+timeinfo->tm_year,nonth,1900+timeinfo->tm_year,nonth,signal_info_data[i].signal_id,1,signal_info_data[i].signal_id,signal_info_data[i].unit_id);
								printf("%s\n",sql);
								st->setSQL(sql);
								st->executeUpdate();
								

								

								/*更新REAL_TC_STATUS信号机在线状态表*/
								sprintf(sql,"update REAL_TC_STATUS set network_stuats = 0,Updatetime = sysdate where signal_id = %d ",signal_info_data[i].signal_id);
								printf("%s\n",sql);
								st->setSQL(sql);
								st->executeUpdate();

								signal_info_data[i].l_online_alarm =1;
							}
							else
							{

							}
						}

						signal_info_data[i].signal_state  = 0;
					}
					else
					{	
						
							/*信号机超时恢复*/
							if(signal_info_data[i].signal_state == 0)
							{				 
								signal_info_data[i].l_online_alarm =0;
								
								sprintf(sql,"delete  from ATC_ALARMLIST where device_id = %d and alarm_id = 1 ",signal_info_data[i].signal_id);
								//printf("%s\n",sql);
								st->setSQL(sql);
								st->executeUpdate();

								
								if((1+timeinfo->tm_mon) < 10)
								{
									sprintf(nonth,"0%d",1+timeinfo->tm_mon);
								}
								else
								{
									sprintf(nonth,"%d",1+timeinfo->tm_mon);
								}
								sprintf(sql,"insert into ATC_ALARMHIS_%d%s(Sn,deviceid,alarm_id,faultcomment,unit_id,type,Report_time) values((select nvl(max(t.Sn)+1,1) from ATC_ALARMHIS_%d%s t), %d,%d,' &#20449;&#21495;&#26426;%d&#31163;&#32447;&#24674;&#22797; ',%d,0,sysdate) ",
								1900+timeinfo->tm_year,nonth,1900+timeinfo->tm_year,nonth,signal_info_data[i].signal_id,1,signal_info_data[i].signal_id,signal_info_data[i].unit_id);
								printf("%s\n",sql);
								st->setSQL(sql);
								st->executeUpdate();
							
									

								sprintf(sql,"update REAL_TC_STATUS set network_stuats = 1,Updatetime = sysdate where signal_id = %d ",signal_info_data[i].signal_id);
								//	printf("%s\n",sql);
								st->setSQL(sql);
								st->executeUpdate();



								/*信号机离线恢复则所有以存在临时故障时间更新为现在时间*/
								int k ;
								
								for(k = 0; k < 17;k++)
								{
									signal_info_data[i].green_conflicts[k][1] = now;
								}

								for(k = 0; k < 17;k++)
								{
									signal_info_data[i].red_green_conflicts[k][1] = now;
								}

								for(k = 0; k < 32;k++)
								{
									signal_info_data[i].lamp_failures[k][1] = now;
								}

								for(k = 0; k < 105;k++)
								{
									signal_info_data[i].detector_failures[k][1] = now;
								}

								for(k = 0; k < 16;k++)
								{
									signal_info_data[i].detector_loss[k][1] = now;
								}

								for(k = 0; k < 5;k++)
								{
									signal_info_data[i].phase_plate_loss[k][1] = now;
								}

								for(k = 0; k < 105;k++)
								{
									signal_info_data[i].detector_timeout[k][1] = now;
								}

								for(k = 0; k < 3;k++)
								{
									signal_info_data[i].door_state[k][1] = now;
								}
								
								 
							}

							signal_info_data[i].signal_state  = 1;
						
					}

					
				}

#endif


				/*处理各种故障*/
				
				for(i = 1; i <signal_number;i++ )
				{
					if(signal_info_data[i].signal_state == 1)
					{
							
						/*处理率冲突*/
						if(signal_info_data[i].green_conflicts_flag == 1)
						{
							/*绿冲突恢复*/
							if((signal_info_data[i].control_model != 0x2e) && (signal_info_data[i].control_model != 0x2e))
							{
								if(signal_info_data[i].control_last_model == 0x2e)
								{
								sprintf(sql,"select  memo   from  ATC_ALARMLIST where device_id = %d  and alarm_id = 128   " ,signal_info_data[i].signal_id);
				 					st->setSQL(sql);
				 					rs = st->executeQuery();

									if(rs->next())
									{
									sprintf(sql,"delete from ATC_ALARMLIST where device_id = %d and alarm_id = %d ",signal_info_data[i].signal_id,0x80);
									printf("%s\n",sql);
									st->setSQL(sql);
									st->executeUpdate();

									if((1+timeinfo->tm_mon) < 10)
									{
										sprintf(nonth,"0%d",1+timeinfo->tm_mon);
									}
									else
									{
										sprintf(nonth,"%d",1+timeinfo->tm_mon);
									}

									
								
									sprintf(sql,"insert into ATC_ALARMHIS_%d%s(Sn,deviceid,alarm_id,faultcomment,unit_id,type,Report_time) values((select nvl(max(t.Sn)+1,1) from ATC_ALARMHIS_%d%s t), %d,%d,' &#32511;&#20914;&#31361;&#24674;&#22797;&#65306;&#30456;&#20301;%s ',%d,0,sysdate) ",
									1900+timeinfo->tm_year,nonth,1900+timeinfo->tm_year,nonth,signal_info_data[i].signal_id,0x80,(rs->getString(1).c_str()),signal_info_data[i].unit_id);
									printf("%s\n",sql);
									st->setSQL(sql);
									st->executeUpdate();
									}

									signal_info_data[i].green_conflicts_flag = 0;
								}
							}
						}

						/*处理红率冲突*/
						if(signal_info_data[i].red_green_conflicts_flag == 1)
						{
							/*红绿冲突恢复*/
							
							if((signal_info_data[i].control_model != 0x2e) &&(signal_info_data[i].control_model != 0x00))
							{
								if(signal_info_data[i].control_last_model == 0x2e)
								{
								sprintf(sql,"select  memo   from  ATC_ALARMLIST where device_id = %d  and alarm_id = 129   " ,signal_info_data[i].signal_id);
				 					st->setSQL(sql);
				 					rs = st->executeQuery();

									if(rs->next())
										{
									sprintf(sql,"delete from ATC_ALARMLIST where device_id = %d and alarm_id = %d ",signal_info_data[i].signal_id,0x81);
									printf("%s\n",sql);
									st->setSQL(sql);
									st->executeUpdate();

									if((1+timeinfo->tm_mon) < 10)
									{
										sprintf(nonth,"0%d",1+timeinfo->tm_mon);
									}
									else
									{
										sprintf(nonth,"%d",1+timeinfo->tm_mon);
									}

									
									sprintf(sql,"insert into ATC_ALARMHIS_%d%s(Sn,deviceid,alarm_id,faultcomment,unit_id,type,Report_time) values((select nvl(max(t.Sn)+1,1) from ATC_ALARMHIS_%d%s t), %d,%d,' &#32418;&#32511;&#20914;&#31361;&#24674;&#22797;&#65306;&#30456;&#20301;%s  ',%d,0,sysdate) ",
									1900+timeinfo->tm_year,nonth,1900+timeinfo->tm_year,nonth,signal_info_data[i].signal_id,0x81,(rs->getString(1).c_str()),signal_info_data[i].unit_id);
									printf("%s\n",sql);
									st->setSQL(sql);
									st->executeUpdate();
										}

									signal_info_data[i].red_green_conflicts_flag = 0;
								}
							}
						}

						

						/*处理灯故障*/
						if(signal_info_data[i].lamp_failures_flag== 1)
							{
							/*灯故障恢复*/
							
							if((signal_info_data[i].control_model != 0x2e) &&(signal_info_data[i].control_model != 0x00))
							{
								if(signal_info_data[i].control_last_model == 0x2e)
								{
									sprintf(sql,"select  memo   from  ATC_ALARMLIST where device_id = %d  and alarm_id = 130   " ,signal_info_data[i].signal_id);
				 					st->setSQL(sql);
				 					rs = st->executeQuery();

									if(rs->next())
										{
									sprintf(sql,"delete from ATC_ALARMLIST where device_id = %d and alarm_id = %d ",signal_info_data[i].signal_id,0x82);
									printf("%s\n",sql);
									st->setSQL(sql);
									st->executeUpdate();

									if((1+timeinfo->tm_mon) < 10)
									{
										sprintf(nonth,"0%d",1+timeinfo->tm_mon);
									}
									else
									{
										sprintf(nonth,"%d",1+timeinfo->tm_mon);
									}

									
									sprintf(sql,"insert into ATC_ALARMHIS_%d%s(Sn,deviceid,alarm_id,faultcomment,unit_id,type,Report_time) values((select nvl(max(t.Sn)+1,1) from ATC_ALARMHIS_%d%s t), %d,%d,' &#28783;&#25925;&#38556;&#24674;&#22797;&#65306;&#30456;&#20301; %s',%d,0,sysdate) ",
									1900+timeinfo->tm_year,nonth,1900+timeinfo->tm_year,nonth,signal_info_data[i].signal_id,0x82,(rs->getString(1).c_str()),signal_info_data[i].unit_id);
									printf("%s\n",sql);
									st->setSQL(sql);
									st->executeUpdate();
										}
									signal_info_data[i].lamp_failures_flag = 0;
								}
								}

							}
							/*信号机离线恢复认为灯故障也恢复*/
						

						/*处理配置故障*/
							//配置故障无恢复

						/*处理检测器故障*/
							//检测器故障在接收里面处理

						/*处理检测板缺失*/
							//检测班缺失不恢复
						/*处理相位板缺失*/
							//相位板缺失不恢复
							
						/*处理检测器超时*/
							//检测器故障在接收里面处理
							
						/*处理门状态*/
							//检测器故障在接收里面处理

						
					}

				}
#if 0			/*处理信号机超时*/
				for(i = 1; i <signal_number;i++ )
				{
					if(signal_info_data[i].signal_id == 1)
						printf("signal_info_data[i].control_model = %x\n",signal_info_data[i].control_model);
				}
#endif		
				
				/**************************/
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
					//	username = "atc";  
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

			
		}
			
		

		
		sleep(1);
	}
}



/*************************************************** 
 * 函数名：signal_control_state_timeout_check
 * 功能描述：检测信号机控制状态是否超时
 *被访问的表：
 *被修改的表：ATC_ALARMLIST  ATC_ALARMHIS_YYMM
* 参数说明：无
*返回值: 无
***************************************************/
void signal_control_state_timeout_check()
{
	int sockfd_recv;
	unsigned char recv_buf[512] ;
	struct sockaddr_in signal_addr;
	 struct sockaddr SA;
	socklen_t peerlen;
	
	
	if ((sockfd_recv = socket(PF_INET, SOCK_DGRAM, 0)) < 0)
	{
		perror("fail to socket bind");
		return ;
		//exit(-1);
	}
	   // 设置套接字选项避免地址使用错误  
    	int on=1;  
   	 if((setsockopt(sockfd_recv,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(on)))<0)  
   	 {  
      	  	perror("setsockopt failed");  
       		 exit(EXIT_FAILURE);  
    	} 


#if 1
	struct timeval tv_out;
       tv_out.tv_sec = 2;//等待2秒设置为0则阻塞
       tv_out.tv_usec = 0;
	setsockopt(sockfd_recv,SOL_SOCKET,SO_RCVTIMEO,&tv_out, sizeof(tv_out));
#endif




	int i ,j;
	time_t now;
	char sql[10000];
	char alarm_time[100];

	time_t rawtime;
	struct tm * timeinfo;

	char nonth[2];

	printf("new pthread, file=%s,func=%s,line=%d\n\n",__FILE__,__FUNCTION__,__LINE__);	
	
	Environment *env = Environment::createEnvironment(Environment::OBJECT);  

	
//	string username = "atc";  
//    	string userpass = "atc";  
//    	string connstr = "192.168.1.24:1521/oracle"; 


	string username = oracle_username;  
    	string userpass = oracle_password;  
    	string connstr = oracle_connectstring; 

		
	Connection *conn_parsing; 
	Statement *st ;

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
	}
		


	while(1)
	{	
		now = time(NULL);
		time ( &rawtime );
		timeinfo = localtime ( &rawtime );
		
	
		while(1)
		{
			try
			{
				/*************************/

					#if 1
				/*处理信号机控制状态*/
				for(i = 1; i <signal_number;i++ )
				{
					//if(signal_info_data[i].signal_id == 1)
					//	{
					//		printf("signal_info_data[i].signal_control_flag = %d\n",signal_info_data[i].signal_control_flag);
					//	}

					
					if(signal_info_data[i].signal_control_flag == 1)
					{
						printf("now - signal_info_data[i].signal_control_time  = %d\n",now - signal_info_data[i].signal_control_time  );
						if(now - signal_info_data[i].signal_control_time > 5)
						{
							printf("chaoshi le   %d\n",signal_info_data[i].signal_control_flag);
						
							/*上一次控制是0*/
								if(signal_info_data[i].signal_last_control_flag == 0)
								{
									bzero(sql,sizeof(sql));
									sprintf(sql,"update CONTROL_CMD set control_flag = 0 where signal_id = %d ",signal_info_data[i].signal_id);
							//		printf("%s\n",sql);
									st->setSQL(sql);
									st->executeUpdate();
									signal_info_data[i].signal_control_flag = 0;
								}
								/*上一次控制是3 滤波*/
								else if(signal_info_data[i].signal_last_control_flag == 3)
								{
									bzero(sql,sizeof(sql));
									sprintf(sql,"update CONTROL_CMD set control_flag = 3 where signal_id = %d ",signal_info_data[i].signal_id);
							//		printf("%s\n",sql);
									st->setSQL(sql);
									st->executeUpdate();
									
									signal_info_data[i].signal_control_flag = 3;
									
									bzero(&signal_addr, sizeof(signal_addr));
									signal_addr.sin_family = PF_INET;
			
									signal_addr.sin_port = htons(signal_info_data[i].signal_port);
									signal_addr.sin_addr.s_addr = inet_addr(signal_info_data[i].signal_ip);


									signal_info_data[i].stop_C0_date[7] = signal_info_data[i].signal_id & 0xff00;
									signal_info_data[i].stop_C0_date[8] = signal_info_data[i].signal_id & 0xff;
									signal_info_data[i].stop_C0_date[12]  = buf_check_num(signal_info_data[i].stop_C0_date);
									/*补发一次停止手动*/
									if(strcmp(signal_info_data[i].device_type,"TC100") == 0)
										sendto(sockfd_recv, signal_info_data[i].stop_C0_date, sizeof(signal_info_data[i].stop_C0_date) , 0, (struct sockaddr*)&signal_addr, sizeof(signal_addr));
									else if(strcmp(signal_info_data[i].device_type,"ITC100") == 0)
									{
										unsigned char stop_C0_date_itc[15];

										bzero(stop_C0_date_itc,15);
										stop_C0_date_itc[0]=0x7e;stop_C0_date_itc[2]= 0x0d;stop_C0_date_itc[4]=0x01;
										stop_C0_date_itc[7]=signal_info_data[i].signal_id & 0xff00;stop_C0_date_itc[8] =signal_info_data[i].signal_id & 0xff;
										stop_C0_date_itc[9]=0x20;stop_C0_date_itc[10] =0xc0;
					
										//stop_C0_date_itc[11] = buf[11];
				
										stop_C0_date_itc[13] = buf_check_num(stop_C0_date_itc);
										stop_C0_date_itc[14] = 0x7d;

										sendto(sockfd_recv, stop_C0_date_itc, sizeof(stop_C0_date_itc), 0, (struct sockaddr*)&signal_addr, sizeof(signal_addr));

									}

									int  num = recvfrom(sockfd_recv, recv_buf, 512, 0, NULL, NULL);
									 if(num <= 0)
									 {
									 	printf(" jieshu  zai ci  lv bo shibai \n");
									 }
									 else
									 {
									 	printf("  jieshu zai ci  lv bo chenggong \n");
									 }


								

									/*补发一次滤波控制*/
									sendto(sockfd_recv, signal_info_data[i].last_C6_date_lin, sizeof(signal_info_data[i].last_C6_date_lin) , 0, (struct sockaddr*)&signal_addr, sizeof(signal_addr));
									printf("sendto to  bufa de C6\n");

									 num = recvfrom(sockfd_recv, recv_buf, 512, 0, NULL, NULL);
									 if(num <= 0)
									 {
									 	printf("  chao shi zai ci  lv bo shibai \n");
									 }
									 else
									 {
									 	printf(" chaoshi  zai ci  lv bo chenggong \n");
									 }

									 
								}
						}
						
					}
					if(signal_info_data[i].signal_control_flag == 2)
					{
						if(signal_info_data[i].signal_control_heart_time  <= 0)
						{
							bzero(&signal_addr, sizeof(signal_addr));
							signal_addr.sin_family = PF_INET;
			
							signal_addr.sin_port = htons(signal_info_data[i].signal_port);
							signal_addr.sin_addr.s_addr = inet_addr(signal_info_data[i].signal_ip);

							//	printf("signal_info_data[i].signal_last_control_flag = %d\n",signal_info_data[i].signal_last_control_flag);
							/*上一次控制是0*/
								if(signal_info_data[i].signal_last_control_flag == 0)
								{
									
									signal_info_data[i].stop_C0_date[7] = signal_info_data[i].signal_id & 0xff00;
									signal_info_data[i].stop_C0_date[8] = signal_info_data[i].signal_id & 0xff;
									signal_info_data[i].stop_C0_date[12]  = buf_check_num(signal_info_data[i].stop_C0_date);
									/*补发一次停止手动*/
									if(strcmp(signal_info_data[i].device_type,"TC100") == 0)
										sendto(sockfd_recv, signal_info_data[i].stop_C0_date, sizeof(signal_info_data[i].stop_C0_date) , 0, (struct sockaddr*)&signal_addr, sizeof(signal_addr));

									else if(strcmp(signal_info_data[i].device_type,"ITC100") == 0)
									{
										unsigned char stop_C0_date_itc[15];

										bzero(stop_C0_date_itc,15);
										stop_C0_date_itc[0]=0x7e;stop_C0_date_itc[2]= 0x0d;stop_C0_date_itc[4]=0x01;
										stop_C0_date_itc[7]=signal_info_data[i].signal_id & 0xff00;stop_C0_date_itc[8] =signal_info_data[i].signal_id & 0xff;
										stop_C0_date_itc[9]=0x20;stop_C0_date_itc[10] =0xc0;
					
										//stop_C0_date_itc[11] = buf[11];
				
										stop_C0_date_itc[13] = buf_check_num(stop_C0_date_itc);
										stop_C0_date_itc[14] = 0x7d;

										sendto(sockfd_recv, stop_C0_date_itc, sizeof(stop_C0_date_itc), 0, (struct sockaddr*)&signal_addr, sizeof(signal_addr));

									}
									
									int  num = recvfrom(sockfd_recv, recv_buf, 512, 0, NULL, NULL);
									 if(num <= 0)
									 {
									 	printf(" jieshu  zai ci  shoudong shibai \n");
									 }
									 else
									 {
									 	printf("  jieshu zai ci  shoudong chenggong \n");
									 }
								
									bzero(sql,sizeof(sql));
									sprintf(sql,"update CONTROL_CMD set control_flag = 0 where signal_id = %d ",signal_info_data[i].signal_id);
									printf("%s\n",sql);
									st->setSQL(sql);
									st->executeUpdate();
									signal_info_data[i].signal_control_flag = 0;
								}
								/*上一次控制是3 滤波*/
								else if(signal_info_data[i].signal_last_control_flag == 3)
								{

								
									signal_info_data[i].stop_C0_date[7] = signal_info_data[i].signal_id >> 8;
									signal_info_data[i].stop_C0_date[8] = signal_info_data[i].signal_id & 0xff;
									signal_info_data[i].stop_C0_date[12]  = buf_check_num(signal_info_data[i].stop_C0_date);
									/*补发一次停止手动*/
									if(strcmp(signal_info_data[i].device_type,"TC100") == 0)
										sendto(sockfd_recv, signal_info_data[i].stop_C0_date, sizeof(signal_info_data[i].stop_C0_date) , 0, (struct sockaddr*)&signal_addr, sizeof(signal_addr));
									else if(strcmp(signal_info_data[i].device_type,"ITC100") == 0)
									{
										unsigned char stop_C0_date_itc[15];

										bzero(stop_C0_date_itc,15);
										stop_C0_date_itc[0]=0x7e;stop_C0_date_itc[2]= 0x0d;stop_C0_date_itc[4]=0x01;
										stop_C0_date_itc[7]=signal_info_data[i].signal_id & 0xff00;stop_C0_date_itc[8] =signal_info_data[i].signal_id & 0xff;
										stop_C0_date_itc[9]=0x20;stop_C0_date_itc[10] =0xc0;
					
										//stop_C0_date_itc[11] = buf[11];
				
										stop_C0_date_itc[13] = buf_check_num(stop_C0_date_itc);
										stop_C0_date_itc[14] = 0x7d;

										sendto(sockfd_recv, stop_C0_date_itc, sizeof(stop_C0_date_itc), 0, (struct sockaddr*)&signal_addr, sizeof(signal_addr));

									}
									
									int  num = recvfrom(sockfd_recv, recv_buf, 512, 0, NULL, NULL);
									 if(num <= 0)
									 {
									 	printf(" jieshu  zai ci  lv bo shibai \n");
									 }
									 else
									 {
									 	printf("  jieshu zai ci  lv bo chenggong \n");
									 }

									/*补发一次滤波控制*/
									sendto(sockfd_recv, signal_info_data[i].last_C6_date_lin, sizeof(signal_info_data[i].last_C6_date_lin) , 0, (struct sockaddr*)&signal_addr, sizeof(signal_addr));
									printf("sendto to  bufa de C6\n");

									 num = recvfrom(sockfd_recv, recv_buf, 512, 0, NULL, NULL);
									 if(num <= 0)
									 {
									 	printf(" zai ci  lv bo shibai \n");
									 }
									 else
									 {
									 	printf(" zai ci  lv bo chenggong \n");
									 }
									
									bzero(sql,sizeof(sql));
									sprintf(sql,"update CONTROL_CMD set control_flag = 3 where signal_id = %d ",signal_info_data[i].signal_id);
									printf("%s\n",sql);
									st->setSQL(sql);
									st->executeUpdate();
									signal_info_data[i].signal_control_flag = 3;



									
								}
						}
					}
#if 0
					char lin_buf[100];
					int ll = 0;
					char * ll_res = NULL;
					int lin_res_num;
					
					if(signal_info_data[i].signal_state == 1)
					{
							if(signal_info_data[i].l_online_alarm == 0)
							{
								
								bzero(lin_buf,sizeof(lin_buf));
								sprintf(lin_buf,"%s",signal_info_data[i].phase_lht);
								ll_res = strtok( lin_buf, "," );
								
								while( ll_res != NULL ) 
								{
									ll++;
									lin_res_num= atoi(ll_res);
									if(lin_res_num == 1)
									{
											bzero(sql,sizeof(sql));
										sprintf(sql,"insert into ATC_ALARMLIST(Sn,device_id,alarm_id,processstatus_id,occur_time,faultcomment,unit_id) values((select nvl(max(t.Sn)+1,1) from ATC_ALARMLIST t),%d,%d,0,sysdate,'error is %d, fangan = %d , jieduan = %d ,yuxingmoshi = %d',%d ) ",
											signal_info_data[i].signal_id,0x88,ll,signal_info_data[i].cur_plan, signal_info_data[i].cur_stage,signal_info_data[i].control_last_model ,signal_info_data[i].unit_id);

										printf("%s\n",sql);
										st->setSQL(sql);
										st->executeUpdate();
									}
									
									
									ll_res = strtok( NULL, "," );
								}
								
							}
					}

#endif					
					
				}

#endif
				
				/*************************/
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

		}
		
		
		sleep(1);
	}
}


