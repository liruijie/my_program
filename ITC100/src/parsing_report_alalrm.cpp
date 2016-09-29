/*************************************************** 
 * 文件名：parsing_report_alalrm.cpp
 * 版权：
 * 描述：处理实时上报来的报警数据
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
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include "/home/ITC100/signal_report.h"
#include "/home/ITC100/occi_opration.h"
#include "/home/ITC100/public_data.h"
#include "/home/ITC100/parsing_report_alalrm.h"
#include "/home/ITC100/queue.h"

using namespace std;
using namespace oracle::occi;

#define BUFF_NUM  512


/*************************************************** 
 * 函数名：parsing_signal_report
 * 功能描述：把所有信号机的实时数据更新到表中
 *被访问的表：
 *被修改的表：TC_ALARMLIST 
* 参数说明：无
*返回值: 无
***************************************************/
void parsing_signal_report_alarm()
{
	printf("new pthread, file=%s,func=%s,line=%d\n\n",__FILE__,__FUNCTION__,__LINE__);	

	char all_sql[1000000];
	bzero(all_sql,sizeof(all_sql));
	sprintf(all_sql,"insert into DETECTOR_REAL_DATA(UNIT_ID,DETECTOR_ID,UPDATE_TIME,FLOW,OCC) ( select * from (");
	
	int lis_n = 0;
	int  i,j;
	char sql[100000];
	unsigned char** li_s;
	//unsigned char buf[512];
	unsigned char* buf= NULL;
	char faultcomment[10000];
	char lin_buf[1000];
	//char* faultcomment = NULL;
	char li_buf[1000];

	time_t now;
	time_t rawtime;
	struct tm * timeinfo;
	
	time_t check_time;
	char check_time_num[30];
	struct tm * timeinfo_check;
	
	char nonth[2];
	
	Environment *env = Environment::createEnvironment(Environment::OBJECT); 
	
//	string username = "atc";  
//    	string userpass = "atc";  
//    	string connstr = "192.168.1.24:1521/oracle"; 

	string username = oracle_username;  
    	string userpass = oracle_password;  
    	string connstr = oracle_connectstring; 
		
	Statement *st;
	Connection *conn_parsing ;

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
			

	int signal_num;

	/*新的*/
	int lin_val[16];
	int lin_bal_num = 0;
	ResultSet *rs ;
	char select_buf[2000];
	while(1)
	{
		now = time(NULL);
		time ( &rawtime );
		timeinfo = localtime ( &rawtime );

//		printf("parsing_signal_report_alarm !!!!!!!!!!\n");
		while(1)
		{
			try
			{
				/*************************/
				while (! queue_empty(sq))
		   		 {  
		   	//	 printf("!!!!!!!!!!!!!!!!!!!!!!!!!\n");	
		   			
					dequeue(sq, (unsigned char**)&buf);
				 	//strcpy(buf,*li_s);
					
				//	printf("buf[10]  = %x\n",buf[10] );
					signal_num = check_signal_id(buf[7] * 256 + buf[8]);

					if(signal_num == -1)
					{
						/*没有这个信号机*/
						continue;
					}
					
					/*收到故障也认为通讯恢复*/
					//signal_info_data[signal_num].last_time = now;
					
					/*报警数据*/
					//if(buf[10] == 0x80 || buf[10] == 0x81 || buf[10] == 0x82 || buf[10] == 0x83 || buf[10] == 0x85 || buf[10] == 0x87 || buf[10] == 0x89 || buf[10] == 0xA0 || buf[10] == 0xA2|| buf[10] == 0x8A)
					if((buf[10] >= 0x80 &&  buf[10] <= 0x8A ) || (buf[10] >= 0xA0&&  buf[10] <= 0xA5))

					{
						bzero(lin_val,sizeof(lin_val));
						lin_bal_num = 0;
						char deviceType[10];

						//根据device_type ，判断为tc100
						for(i = 1; i < signal_number; i++)
						{
							if(signal_info_data[i].signal_id == buf[7] * 256 + buf[8])
								strcpy(deviceType,signal_info_data[i].device_type);
						}

						if(strcmp(deviceType,"TC100") == 0)
						{
						/*处理绿冲突*/
						if(buf[10] == 0x80)
						{
							
							for(i = 0; i < 16;i++)
							{
								if(i < 8)
								{
									if((buf[12] & (0x80 >> i)) != 0)
									{
										lin_val[lin_bal_num] = i+1;
										lin_bal_num++;
									}
								}
								else
								{
									if((buf[13] & (0x80 >> (i-8))) != 0)
									{
										lin_val[lin_bal_num] = i+1;
										lin_bal_num++;
									}
								}
							}

							if(lin_bal_num == 0)
							{
								/*无错误*/
								continue;
							}
							else
							{
								
								bzero(faultcomment,sizeof(faultcomment));
								sprintf(faultcomment,"&#32511;&#20914;&#31361;&#65306;&#30456;&#20301;");




								bzero(select_buf,2000);
								sprintf(select_buf,"\'&#32511;&#20914;&#31361;&#65306;&#30456;&#20301;");            //
								for(i = 0; i <lin_bal_num;i++)
								{
									sprintf(lin_buf,"%d,",lin_val[i]);
									strcat(faultcomment,lin_buf);
									strcat(select_buf,lin_buf);
								}
								strcat(select_buf,"\'");

								bzero(sql,sizeof(sql));
								sprintf(sql,"select * from ATC_ALARMLIST where device_id  = %d and alarm_id = 128 and faultcomment = %s",buf[7] * 256 + buf[8],select_buf);
								st->setSQL(sql);
				 				rs = st->executeQuery();
								if(rs->next() != 0)              //查询故障表里是否有存在该报警  有就不处理  没有就插入故障表
								{
									printf("signal_id =%d ,yi cun zai green_fault\n",buf[7] * 256 + buf[8]);
									continue;
								}

								bzero(sql,sizeof(sql));
								sprintf(sql,"insert into ATC_ALARMLIST(Sn,device_id,alarm_id,processstatus_id,occur_time,faultcomment,unit_id) values((select nvl(max(t.Sn)+1,1) from ATC_ALARMLIST t),%d,%d,0,sysdate,'%s',%d) ",
									buf[7] * 256 + buf[8],buf[10],faultcomment,signal_info_data[signal_num].unit_id);

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

								sprintf(sql,"insert into ATC_ALARMHIS_%d%s(Sn,deviceid,alarm_id,faultcomment,unit_id,type,Report_time) values((select nvl(max(t.Sn)+1,1) from ATC_ALARMHIS_%d%s t), %d,%d,' %s ',%d,1,sysdate) ",
									1900+timeinfo->tm_year,nonth,1900+timeinfo->tm_year,nonth,signal_info_data[signal_num].signal_id,buf[10],faultcomment,signal_info_data[signal_num].unit_id);
								printf("%s\n",sql);
								st->setSQL(sql);
								st->executeUpdate();
								
								signal_info_data[signal_num].green_conflicts_flag = 1;
							}
						

							
							
						}
						else if(buf[10] == 0x81)
						{
							//sprintf(faultcomment," 红绿冲突!");

							
							for(i = 0; i < 16;i++)
							{
								if(i < 8)
								{
									if((buf[12] & (0x80 >> i)) != 0)
									{
										lin_val[lin_bal_num] = i+1;
										lin_bal_num++;
									}
								}
								else
								{
									if((buf[13] & (0x80 >> (i-8))) != 0)
									{
										lin_val[lin_bal_num] = i+1;
										lin_bal_num++;
									}
								}
							}

							if(lin_bal_num == 0)
							{
								/*无错误*/
								continue;
							}
							else
							{
								
								bzero(faultcomment,sizeof(faultcomment));
								sprintf(faultcomment,"&#32418;&#32511;&#20914;&#31361;&#65306;&#30456;&#20301;");

								bzero(select_buf,2000);
								sprintf(select_buf,"\'&#32418;&#32511;&#20914;&#31361;&#65306;&#30456;&#20301;");            //
								for(i = 0; i <lin_bal_num;i++)
								{
									sprintf(lin_buf,"%d,",lin_val[i]);
									strcat(faultcomment,lin_buf);
									strcat(select_buf,lin_buf);
								}
								strcat(select_buf,"\'");

								bzero(sql,sizeof(sql));
								sprintf(sql,"select * from ATC_ALARMLIST where device_id  = %d and alarm_id = 129 and faultcomment = %s",buf[7] * 256 + buf[8],select_buf);
								st->setSQL(sql);
				 				rs = st->executeQuery();
								if(rs->next() != 0)              //查询故障表里是否有存在该报警  有就不处理  没有就插入故障表
								{
									printf("signal_id =%d ,yi cun zai red_green_fault\n",buf[7] * 256 + buf[8]);
									continue;
								}



								bzero(sql,sizeof(sql));
								sprintf(sql,"insert into ATC_ALARMLIST(Sn,device_id,alarm_id,processstatus_id,occur_time,faultcomment,unit_id) values((select nvl(max(t.Sn)+1,1) from ATC_ALARMLIST t),%d,%d,0,sysdate,'%s',%d) ",
									buf[7] * 256 + buf[8],buf[10],faultcomment,signal_info_data[signal_num].unit_id);

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

								sprintf(sql,"insert into ATC_ALARMHIS_%d%s(Sn,deviceid,alarm_id,faultcomment,unit_id,type,Report_time) values((select nvl(max(t.Sn)+1,1) from ATC_ALARMHIS_%d%s t), %d,%d,' %s ',%d,1,sysdate) ",
									1900+timeinfo->tm_year,nonth,1900+timeinfo->tm_year,nonth,signal_info_data[signal_num].signal_id,buf[10],faultcomment,signal_info_data[signal_num].unit_id);
								printf("%s\n",sql);
								st->setSQL(sql);
								st->executeUpdate();
								
								signal_info_data[signal_num].red_green_conflicts_flag = 1;
							}
						

							
						}
						//tc100
						else if(buf[10] == 0x82)
						{
							//sprintf(faultcomment," 灯故障!");

							printf("buf[12] = %d\n",buf[12]);
							printf("buf[13] = %d\n",buf[13]);
							
							for(i = 0; i < 16;i++)
							{
								if(i < 8)
								{
									if((buf[12] & (0x80 >> i)) != 0)
									{
										lin_val[lin_bal_num] = i+1;
										lin_bal_num++;
									}
								}
								else
								{
									if((buf[13] & (0x80 >> (i-8))) != 0)
									{
										lin_val[lin_bal_num] = i+1;
										lin_bal_num++;
									}
								}
							}

							if(lin_bal_num == 0)
							{
								/*无错误*/
								printf("lin_bal_num = %d\n",lin_bal_num);
								continue;
							}
							else
							{
								
								bzero(faultcomment,sizeof(faultcomment));
								sprintf(faultcomment,"&#28783;&#25925;&#38556;&#65306;&#30456;&#20301;");
								for(i = 0; i <lin_bal_num;i++)
								{
									sprintf(lin_buf,"%d,",lin_val[i]);
									strcat(faultcomment,lin_buf);

									//signal_info_data[signal_num].lamp_failures[lin_val[i]-1][0]= 1;
								}

								bzero(sql,sizeof(sql));
								sprintf(sql,"insert into ATC_ALARMLIST(Sn,device_id,alarm_id,processstatus_id,occur_time,faultcomment,unit_id,memo) values((select nvl(max(t.Sn)+1,1) from ATC_ALARMLIST t),%d,%d,0,sysdate,'%s',%d,'%s') ",
									buf[7] * 256 + buf[8],buf[10],faultcomment,signal_info_data[signal_num].unit_id,lin_buf);

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

								sprintf(sql,"insert into ATC_ALARMHIS_%d%s(Sn,deviceid,alarm_id,faultcomment,unit_id,type,Report_time) values((select nvl(max(t.Sn)+1,1) from ATC_ALARMHIS_%d%s t), %d,%d,' %s ',%d,1,sysdate) ",
									1900+timeinfo->tm_year,nonth,1900+timeinfo->tm_year,nonth,signal_info_data[signal_num].signal_id,buf[10],faultcomment,signal_info_data[signal_num].unit_id);
								printf("%s\n",sql);
								st->setSQL(sql);
								st->executeUpdate();

								signal_info_data[signal_num].lamp_failures_flag= 1;
								
								
							}
						

							
							
						}
						else if(buf[10] == 0x83)
						{
							//sprintf(faultcomment," 配置故障!");
							bzero(faultcomment,sizeof(faultcomment));
								
							if(buf[13] == 1)
							{
								sprintf(faultcomment,"&#26102;&#38388;&#34920;&#25925;&#38556;");
							
							}
							else if(buf[13] == 2)
							{
								sprintf(faultcomment,"&#37197;&#32622;&#34920;&#25925;&#38556;");
							
							}
							else
							{
								continue;
							}

							bzero(sql,sizeof(sql));
							
							if((1+timeinfo->tm_mon) < 10)
							{
								sprintf(nonth,"0%d",1+timeinfo->tm_mon);
							}
							else
							{
								sprintf(nonth,"%d",1+timeinfo->tm_mon);
							}

							sprintf(sql,"insert into ATC_ALARMHIS_%d%s(Sn,deviceid,alarm_id,faultcomment,unit_id,type,Report_time) values((select nvl(max(t.Sn)+1,1) from ATC_ALARMHIS_%d%s t), %d,%d,' %s ',%d,1,sysdate) ",
								1900+timeinfo->tm_year,nonth,1900+timeinfo->tm_year,nonth,signal_info_data[signal_num].signal_id,buf[10],faultcomment,signal_info_data[signal_num].unit_id);
							printf("%s\n",sql);
							st->setSQL(sql);
							st->executeUpdate();
							
						}

						else if(buf[10] == 0x85)
						{
							//sprintf(faultcomment," 检测器故障!");



							/*故障恢复*/
							if( buf[12] == 0)
							{
								if(signal_info_data[signal_num].detector_failures[buf[13]][0] == -1)
								{
									sprintf(sql,"delete from ATC_ALARMLIST where device_id = %d and alarm_id = %d  and faultcomment ='&#26816;&#27979;&#22120;&#25925;&#38556;&#65306;&#32534;&#21495;%d'",signal_info_data[signal_num].signal_id,0x85,buf[13]);
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
								
									sprintf(sql,"insert into ATC_ALARMHIS_%d%s(Sn,deviceid,alarm_id,faultcomment,unit_id,type,Report_time) values((select nvl(max(t.Sn)+1,1) from ATC_ALARMHIS_%d%s t), %d,%d,' &#26816;&#27979;&#22120;&#25925;&#38556;&#65306;&#32534;&#21495;%d ',%d,0,sysdate) ",
									1900+timeinfo->tm_year,nonth,1900+timeinfo->tm_year,nonth,signal_info_data[signal_num].signal_id,0x85,buf[13],signal_info_data[signal_num].unit_id);
									printf("%s\n",sql);
									st->setSQL(sql);
									st->executeUpdate();

									signal_info_data[signal_num].detector_failures[buf[13]][0] = 0;
								}
							}
							/*故障*/
							else if(  buf[12] == 1)
							{
								if(signal_info_data[signal_num].detector_failures[buf[13]][0] == 0)
								{
									
								
									bzero(faultcomment,sizeof(faultcomment));
									sprintf(faultcomment,"&#26816;&#27979;&#22120;&#25925;&#38556;&#65306;&#32534;&#21495;%d",buf[13]);
													

									bzero(sql,sizeof(sql));


									if((1+timeinfo->tm_mon) < 10)
									{
										sprintf(nonth,"0%d",1+timeinfo->tm_mon);
									}
									else
									{
										sprintf(nonth,"%d",1+timeinfo->tm_mon);
									}

									sprintf(sql,"insert into ATC_ALARMHIS_%d%s(Sn,deviceid,alarm_id,faultcomment,unit_id,type,Report_time) values((select nvl(max(t.Sn)+1,1) from ATC_ALARMHIS_%d%s t), %d,%d,' %s ',%d,1,sysdate) ",
										1900+timeinfo->tm_year,nonth,1900+timeinfo->tm_year,nonth,signal_info_data[signal_num].signal_id,buf[10],faultcomment,signal_info_data[signal_num].unit_id);
									printf("%s\n",sql);
									st->setSQL(sql);
									st->executeUpdate();

									signal_info_data[signal_num].detector_failures[buf[13]][0] = -1;
								}
								
							}
							else
							{
								
							}

							
						}		
						else if(buf[10] == 0x87)
						{
							//sprintf(faultcomment," 检测板缺失");


							for(i = 0; i < 4;i++)
							{
								
								if((buf[13] & (0x08 >> i)) != 0)
								{
									lin_val[lin_bal_num] = i+1;
									lin_bal_num++;
								}
								
							}

							if(lin_bal_num == 0)
							{
								/*无错误*/
								continue;
							}
							else
							{
								
								bzero(faultcomment,sizeof(faultcomment));
								sprintf(faultcomment,"&#26816;&#27979;&#26495;&#32570;&#22833;&#65306;&#26495;&#21495;");
								for(i = 0; i <lin_bal_num;i++)
								{
									sprintf(lin_buf,"%d,",lin_val[i]);
									strcat(faultcomment,lin_buf);
								}

								bzero(sql,sizeof(sql));
								
								if((1+timeinfo->tm_mon) < 10)
								{
									sprintf(nonth,"0%d",1+timeinfo->tm_mon);
								}
								else
								{
									sprintf(nonth,"%d",1+timeinfo->tm_mon);
								}

								sprintf(sql,"insert into ATC_ALARMHIS_%d%s(Sn,deviceid,alarm_id,faultcomment,unit_id,type,Report_time) values((select nvl(max(t.Sn)+1,1) from ATC_ALARMHIS_%d%s t), %d,%d,' %s ',%d,1,sysdate) ",
									1900+timeinfo->tm_year,nonth,1900+timeinfo->tm_year,nonth,signal_info_data[signal_num].signal_id,buf[10],faultcomment,signal_info_data[signal_num].unit_id);
								printf("%s\n",sql);
								st->setSQL(sql);
								st->executeUpdate();
								
								
							}
							
							
						}		
						else if(buf[10] == 0x89)
						{
							//sprintf(faultcomment," 相位板缺失");

							for(i = 0; i < 4;i++)
							{
								
								if((buf[13] & (0x08 >> i)) != 0)
								{
									lin_val[lin_bal_num] = i+1;
									lin_bal_num++;
								}
								
							}

							if(lin_bal_num == 0)
							{
								/*无错误*/
								continue;
							}
							else
							{
								
								bzero(faultcomment,sizeof(faultcomment));
								sprintf(faultcomment,"&#30456;&#20301;&#26495;&#32570;&#22833;&#65306;&#26495;&#21495;");
								for(i = 0; i <lin_bal_num;i++)
								{
									sprintf(lin_buf,"%d,",lin_val[i]);
									strcat(faultcomment,lin_buf);
								}

								bzero(sql,sizeof(sql));
								
								if((1+timeinfo->tm_mon) < 10)
								{
									sprintf(nonth,"0%d",1+timeinfo->tm_mon);
								}
								else
								{
									sprintf(nonth,"%d",1+timeinfo->tm_mon);
								}

								sprintf(sql,"insert into ATC_ALARMHIS_%d%s(Sn,deviceid,alarm_id,faultcomment,unit_id,type,Report_time) values((select nvl(max(t.Sn)+1,1) from ATC_ALARMHIS_%d%s t), %d,%d,' %s ',%d,1,sysdate) ",
									1900+timeinfo->tm_year,nonth,1900+timeinfo->tm_year,nonth,signal_info_data[signal_num].signal_id,buf[10],faultcomment,signal_info_data[signal_num].unit_id);
								printf("%s\n",sql);
								st->setSQL(sql);
								st->executeUpdate();
								
								
							}
							
						}
						else if(buf[10] == 0x8A)
						{
							bzero(faultcomment,sizeof(faultcomment));
							sprintf(faultcomment,"&#37325;&#21551;&#25925;&#38556;");
							

							bzero(sql,sizeof(sql));
							
							if((1+timeinfo->tm_mon) < 10)
							{
								sprintf(nonth,"0%d",1+timeinfo->tm_mon);
							}
							else
							{
								sprintf(nonth,"%d",1+timeinfo->tm_mon);
							}

							sprintf(sql,"insert into ATC_ALARMHIS_%d%s(Sn,deviceid,alarm_id,faultcomment,unit_id,type,Report_time) values((select nvl(max(t.Sn)+1,1) from ATC_ALARMHIS_%d%s t), %d,%d,' %s ',%d,1,sysdate) ",
								1900+timeinfo->tm_year,nonth,1900+timeinfo->tm_year,nonth,signal_info_data[signal_num].signal_id,buf[10],faultcomment,signal_info_data[signal_num].unit_id);
							printf("%s\n",sql);
							st->setSQL(sql);
							st->executeUpdate();
							
						}
						else if(buf[10] == 0xA0)
						{
							//sprintf(faultcomment," 检测器超时!");

							/*故障恢复*/
							if( buf[12] == 0)
							{
								if(signal_info_data[signal_num].detector_timeout[buf[13]][0] == -1)
								{
									sprintf(sql,"delete from ATC_ALARMLIST where device_id = %d and alarm_id = %d  and faultcomment ='&#26816;&#27979;&#22120;&#36229;&#26102;&#65306;&#32534;&#21495;%d'",signal_info_data[signal_num].signal_id,0xA0,buf[13]);
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
								
									sprintf(sql,"insert into ATC_ALARMHIS_%d%s(Sn,deviceid,alarm_id,faultcomment,unit_id,type,Report_time) values((select nvl(max(t.Sn)+1,1) from ATC_ALARMHIS_%d%s t), %d,%d,' &#26816;&#27979;&#22120;&#25925;&#38556;&#65306;&#32534;&#21495;%d ',%d,0,sysdate) ",
									1900+timeinfo->tm_year,nonth,1900+timeinfo->tm_year,nonth,signal_info_data[signal_num].signal_id,0xA0,buf[13],signal_info_data[signal_num].unit_id);
									printf("%s\n",sql);
									st->setSQL(sql);
									st->executeUpdate();

									signal_info_data[signal_num].detector_timeout[buf[13]][0] = 0;
								}
							}
							/*故障*/
							else if(  buf[12] == 1)
							{
								if(signal_info_data[signal_num].detector_timeout[buf[13]][0] == 0)
								{
									
									bzero(faultcomment,sizeof(faultcomment));
									sprintf(faultcomment,"&#26816;&#27979;&#22120;&#36229;&#26102;&#65306;&#32534;&#21495;%d",buf[13]);

									bzero(sql,sizeof(sql));


									if((1+timeinfo->tm_mon) < 10)
									{
										sprintf(nonth,"0%d",1+timeinfo->tm_mon);
									}
									else
									{
										sprintf(nonth,"%d",1+timeinfo->tm_mon);
									}

									sprintf(sql,"insert into ATC_ALARMHIS_%d%s(Sn,deviceid,alarm_id,faultcomment,unit_id,type,Report_time) values((select nvl(max(t.Sn)+1,1) from ATC_ALARMHIS_%d%s t), %d,%d,' %s ',%d,1,sysdate) ",
										1900+timeinfo->tm_year,nonth,1900+timeinfo->tm_year,nonth,signal_info_data[signal_num].signal_id,buf[10],faultcomment,signal_info_data[signal_num].unit_id);
									printf("%s\n",sql);
									st->setSQL(sql);
									st->executeUpdate();

									signal_info_data[signal_num].detector_timeout[buf[13]][0] = -1;
								}
								
							}
							else
							{
								
							}
							
							
						}	
						else if(buf[10] == 0xA2)
						{
							//sprintf(faultcomment," 门!");

							printf("buf[12] = %d\n",buf[12]);
							printf("buf[13] = %d\n",buf[13]);
							/*故障恢复*/
							if( buf[12] == 0)
							{
								
			
								if(buf[13] == 1)
								{
									sprintf(sql,"delete  from ATC_ALARMLIST where device_id = %d and alarm_id = 162 and faultcomment ='&#21069;&#38376;&#25171;&#24320;&#25253;&#35686;' ",signal_info_data[signal_num].signal_id);
								}
								else if(buf[13] == 2)
								{
									sprintf(sql,"delete  from ATC_ALARMLIST where device_id = %d and alarm_id = 162 and faultcomment ='&#21518;&#38376;&#25171;&#24320;&#25253;&#35686;' ",signal_info_data[signal_num].signal_id);
							
								}
								else
								{
									continue;
								}
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

								if(buf[13] == 1)
								{
									sprintf(sql,"insert into ATC_ALARMHIS_%d%s(Sn,deviceid,alarm_id,faultcomment,unit_id,type,Report_time) values((select nvl(max(t.Sn)+1,1) from ATC_ALARMHIS_%d%s t), %d,%d,' &#21069;&#38376;&#25171;&#24320;&#25253;&#35686;&#24674;&#22797; ',%d,0,sysdate) ",
										1900+timeinfo->tm_year,nonth,1900+timeinfo->tm_year,nonth,signal_info_data[signal_num].signal_id,buf[10],signal_info_data[signal_num].unit_id);
									printf("%s\n",sql);
									st->setSQL(sql);
									st->executeUpdate();

								}
								else if(buf[13] == 2)
								{
									sprintf(sql,"insert into ATC_ALARMHIS_%d%s(Sn,deviceid,alarm_id,faultcomment,unit_id,type,Report_time) values((select nvl(max(t.Sn)+1,1) from ATC_ALARMHIS_%d%s t), %d,%d,' &#21518;&#38376;&#25171;&#24320;&#25253;&#35686;&#24674;&#22797; ',%d,0,sysdate) ",
										1900+timeinfo->tm_year,nonth,1900+timeinfo->tm_year,nonth,signal_info_data[signal_num].signal_id,buf[10],signal_info_data[signal_num].unit_id);
									printf("%s\n",sql);
									st->setSQL(sql);
									st->executeUpdate();
								}
								else
								{
									continue;
								}

								
								
								
							}
							/*故障*/
							if( buf[12] == 1)
							{
								if(buf[13] == 1)
								{
									bzero(faultcomment,sizeof(faultcomment));
									sprintf(faultcomment,"&#21069;&#38376;&#25171;&#24320;&#25253;&#35686;");
									signal_info_data[signal_num].door_state[1][0] = -1;
								}
								else if(buf[13] == 2)
								{
									bzero(faultcomment,sizeof(faultcomment));
									sprintf(faultcomment,"&#21518;&#38376;&#25171;&#24320;&#25253;&#35686;");
										signal_info_data[signal_num].door_state[2][0] = -1;
								}
								else
								{
									continue;
								}
								
								bzero(sql,sizeof(sql));
								sprintf(sql,"insert into ATC_ALARMLIST(Sn,device_id,alarm_id,processstatus_id,occur_time,faultcomment,unit_id) values((select nvl(max(t.Sn)+1,1) from ATC_ALARMLIST t),%d,%d,0,sysdate,'%s',%d) ",
									buf[7] * 256 + buf[8],buf[10],faultcomment,signal_info_data[signal_num].unit_id);

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

								sprintf(sql,"insert into ATC_ALARMHIS_%d%s(Sn,deviceid,alarm_id,faultcomment,unit_id,type,Report_time) values((select nvl(max(t.Sn)+1,1) from ATC_ALARMHIS_%d%s t), %d,%d,' %s ',%d,1,sysdate) ",
									1900+timeinfo->tm_year,nonth,1900+timeinfo->tm_year,nonth,signal_info_data[signal_num].signal_id,buf[10],faultcomment,signal_info_data[signal_num].unit_id);
								printf("%s\n",sql);
								st->setSQL(sql);
								st->executeUpdate();
							}

							
						
						}		
					}

						//ITC100  故障数据处理
						else if(strcmp(deviceType,"ITC100") == 0)
						{
						/*处理绿冲突*/
						if(buf[10] == 0x80)
						{
							
							unsigned int fault_data =  (buf[12]<<24) + (buf[13]<<16) + (buf[14]<<8) + buf[15];
							char temp_value[200];
							bzero(temp_value,200);

							for(i = 0 ; i < 32; i++)
							{
								unsigned int temp = 0x80000000 >> i;

								if((fault_data & temp) != 0)
								{
									lin_bal_num++;
									char stemp[4];
									sprintf(stemp,"%d,",(i+1));
									strcat(temp_value,stemp);
								}
							}

							time_t fault_time = (time_t)((buf[16]<<24) + (buf[17]<<16) + (buf[18]<<8) +buf[19]);
							
							struct tm *tbclock;
							//tbclock = localtime(&fault_time);
							tbclock = gmtime(&fault_time);
							if(lin_bal_num == 0)
							{
								/*无错误*/
								continue;
							}
							else
							{
								char updateTime[30];
								sprintf(updateTime,"%d-%02d-%02d %02d:%02d:%02d",(1900+tbclock->tm_year),(1+tbclock->tm_mon),tbclock->tm_mday,tbclock->tm_hour,tbclock->tm_min,tbclock->tm_sec);
								
								bzero(faultcomment,sizeof(faultcomment));
								sprintf(faultcomment,"&#32511;&#20914;&#31361;&#65306;&#30456;&#20301;");
								strcat(faultcomment,temp_value);


								bzero(select_buf,2000);
								sprintf(select_buf,"\'&#32511;&#20914;&#31361;&#65306;&#30456;&#20301;%s\'",temp_value);            //
								bzero(sql,sizeof(sql));
								sprintf(sql,"select * from ATC_ALARMLIST where device_id  = %d and alarm_id = 128 and faultcomment = %s",buf[7] * 256 + buf[8],select_buf);
								st->setSQL(sql);
				 				rs = st->executeQuery();
								if(rs->next() != 0)              //查询故障表里是否有存在该报警  有就不处理  没有就插入故障表
								{
									printf("signal_id =%d ,yi cun zai green_fault\n",buf[7] * 256 + buf[8]);
									continue;
								}




								bzero(sql,sizeof(sql));
								sprintf(sql,"insert into ATC_ALARMLIST(Sn,device_id,alarm_id,processstatus_id,occur_time,faultcomment,unit_id,memo) values((select nvl(max(t.Sn)+1,1) from ATC_ALARMLIST t),%d,%d,0,to_date('%s','yyyy-mm-dd hh24:mi:ss'),'%s',%d,'%s') ",
									buf[7] * 256 + buf[8],buf[10],updateTime,faultcomment,signal_info_data[signal_num].unit_id,temp_value);

								printf("%s\n",sql);
								st->setSQL(sql);
								st->executeUpdate();


								if((1+tbclock->tm_mon) < 10)
								{
									sprintf(nonth,"0%d",1+tbclock->tm_mon);
								}
								else
								{
									sprintf(nonth,"%d",1+tbclock->tm_mon);
								}

								sprintf(sql,"insert into ATC_ALARMHIS_%d%s(Sn,deviceid,alarm_id,faultcomment,unit_id,type,Report_time) values((select nvl(max(t.Sn)+1,1) from ATC_ALARMHIS_%d%s t), %d,%d,' %s ',%d,1,to_date('%s','yyyy-mm-dd hh24:mi:ss')) ",
									1900+tbclock->tm_year,nonth,1900+tbclock->tm_year,nonth,signal_info_data[signal_num].signal_id,buf[10],faultcomment,signal_info_data[signal_num].unit_id,updateTime);
								printf("%s\n",sql);
								st->setSQL(sql);
								st->executeUpdate();
								
								signal_info_data[signal_num].green_conflicts_flag = 1;
							}
						

							
							
						}
						else if(buf[10] == 0x81)
						{
							//sprintf(faultcomment," 红绿冲突!");

							
							unsigned int fault_data =  (buf[12]<<24) + (buf[13]<<16) + (buf[14]<<8) + buf[15];
							char temp_value[200];
							bzero(temp_value,200);

							for(i = 0 ; i < 32; i++)
							{
								unsigned int temp = 0x80000000 >> i;

								if((fault_data & temp) != 0)
								{
									lin_bal_num++;
									char stemp[4];
									sprintf(stemp,"%d,",(i+1));
									strcat(temp_value,stemp);
								}
							}

							time_t fault_time = (time_t)((buf[16]<<24) + (buf[17]<<16) + (buf[18]<<8) +buf[19]);
							
							struct tm *tbclock;
							//tbclock = localtime(&fault_time);
							tbclock = gmtime(&fault_time);

							if(lin_bal_num == 0)
							{
								/*无错误*/
								continue;
							}
							else
							{
								char updateTime[30];
								sprintf(updateTime,"%d-%02d-%02d %02d:%02d:%02d",(1900+tbclock->tm_year),(1+tbclock->tm_mon),tbclock->tm_mday,tbclock->tm_hour,tbclock->tm_min,tbclock->tm_sec);
								
								bzero(faultcomment,sizeof(faultcomment));
								sprintf(faultcomment,"&#32418;&#32511;&#20914;&#31361;&#65306;&#30456;&#20301;");
								strcat(faultcomment,temp_value);

								bzero(select_buf,2000);
								sprintf(select_buf,"\'&#32418;&#32511;&#20914;&#31361;&#65306;&#30456;&#20301;%s\'",temp_value);            //
								bzero(sql,sizeof(sql));
								sprintf(sql,"select * from ATC_ALARMLIST where device_id  = %d and alarm_id = 129 and faultcomment = %s",buf[7] * 256 + buf[8],select_buf);
								st->setSQL(sql);
				 				rs = st->executeQuery();
								if(rs->next() != 0)              //查询实时故障表里是否有存在该报警  有就不处理  没有就插入故障表
								{
									printf("signal_id =%d ,yi cun zai green_fault\n",buf[7] * 256 + buf[8]);
									continue;
								}



								bzero(sql,sizeof(sql));
								sprintf(sql,"insert into ATC_ALARMLIST(Sn,device_id,alarm_id,processstatus_id,occur_time,faultcomment,unit_id,memo) values((select nvl(max(t.Sn)+1,1) from ATC_ALARMLIST t),%d,%d,0,to_date('%s','yyyy-mm-dd hh24:mi:ss'),'%s',%d,'%s') ",
									buf[7] * 256 + buf[8],buf[10],updateTime,faultcomment,signal_info_data[signal_num].unit_id,temp_value);

								printf("%s\n",sql);
								st->setSQL(sql);
								st->executeUpdate();


								if((1+tbclock->tm_mon) < 10)
								{
									sprintf(nonth,"0%d",1+tbclock->tm_mon);
								}
								else
								{
									sprintf(nonth,"%d",1+tbclock->tm_mon);
								}

								sprintf(sql,"insert into ATC_ALARMHIS_%d%s(Sn,deviceid,alarm_id,faultcomment,unit_id,type,Report_time) values((select nvl(max(t.Sn)+1,1) from ATC_ALARMHIS_%d%s t), %d,%d,' %s ',%d,1,to_date('%s','yyyy-mm-dd hh24:mi:ss')) ",
									1900+tbclock->tm_year,nonth,1900+tbclock->tm_year,nonth,signal_info_data[signal_num].signal_id,buf[10],faultcomment,signal_info_data[signal_num].unit_id,updateTime);
								printf("%s\n",sql);
								st->setSQL(sql);
								st->executeUpdate();
								
								signal_info_data[signal_num].red_green_conflicts_flag = 1;
							}
						

							
						}
						//ITC100
						else if(buf[10] == 0x82)
						{
							//sprintf(faultcomment," 灯故障!");

							unsigned int fault_data =  (buf[12]<<24) + (buf[13]<<16) + (buf[14]<<8) + buf[15];
							char temp_value[200];
							bzero(temp_value,200);

							for(i = 0 ; i < 32; i++)
							{
								unsigned int temp = 0x80000000 >> i;

								if((fault_data & temp) != 0)
								{
									lin_bal_num++;
									char stemp[4];
									sprintf(stemp,"%d,",(i+1));
									strcat(temp_value,stemp);

									//signal_info_data[signal_num].lamp_failures[i][0]= 1;// 对应相位灯故障标志
								}
							}

							time_t fault_time = (time_t)((buf[16]<<24 )+ (buf[17]<<16) + (buf[18]<<8) +buf[19]);
							
							struct tm *tbclock;
							//tbclock = localtime(&fault_time);
							tbclock = gmtime(&fault_time);

							if(lin_bal_num == 0)
							{
								/*无错误*/
								printf("lin_bal_num = %d\n",lin_bal_num);
								continue;
							}
							else
							{
								char updateTime[30];
								sprintf(updateTime,"%d-%02d-%02d %02d:%02d:%02d",(1900+tbclock->tm_year),(1+tbclock->tm_mon),tbclock->tm_mday,tbclock->tm_hour,tbclock->tm_min,tbclock->tm_sec);
								
								bzero(faultcomment,sizeof(faultcomment));
								sprintf(faultcomment,"&#28783;&#25925;&#38556;&#65306;&#30456;&#20301;");
								strcat(faultcomment,temp_value);

								bzero(sql,sizeof(sql));
								sprintf(sql,"insert into ATC_ALARMLIST(Sn,device_id,alarm_id,processstatus_id,occur_time,faultcomment,unit_id,memo) values((select nvl(max(t.Sn)+1,1) from ATC_ALARMLIST t),%d,%d,0,to_date('%s','yyyy-mm-dd hh24:mi:ss'),'%s',%d,'%s') ",
									buf[7] * 256 + buf[8],buf[10],updateTime,faultcomment,signal_info_data[signal_num].unit_id,temp_value);

								printf("%s\n",sql);
								st->setSQL(sql);
								st->executeUpdate();
								
								if((1+tbclock->tm_mon) < 10)
								{
									sprintf(nonth,"0%d",1+tbclock->tm_mon);
								}
								else
								{
									sprintf(nonth,"%d",1+tbclock->tm_mon);
								}

								sprintf(sql,"insert into ATC_ALARMHIS_%d%s(Sn,deviceid,alarm_id,faultcomment,unit_id,type,Report_time) values((select nvl(max(t.Sn)+1,1) from ATC_ALARMHIS_%d%s t), %d,%d,' %s ',%d,1,to_date('%s','yyyy-mm-dd hh24:mi:ss')) ",
									1900+tbclock->tm_year,nonth,1900+tbclock->tm_year,nonth,signal_info_data[signal_num].signal_id,buf[10],faultcomment,signal_info_data[signal_num].unit_id,updateTime);
								printf("%s\n",sql);
								st->setSQL(sql);
								st->executeUpdate();
								
								signal_info_data[signal_num].lamp_failures_flag= 1;
							}
						

							
							
						}
						else if(buf[10] == 0x83)
						{
							//sprintf(faultcomment," 配置故障!");

							unsigned int fault_data =  (buf[12]<<24) + (buf[13]<<16) + (buf[14]<<8) + buf[15];
							char temp_value[200];
							bzero(temp_value,200);

							for(i = 0 ; i < 32; i++)
							{
								unsigned int temp = 0x80000000 >> i;

								if((fault_data & temp) != 0)
								{
									lin_bal_num++;
									char stemp[4];
									sprintf(stemp,"%d,",(i+1));
									strcat(temp_value,stemp);
								}
							}

							time_t fault_time = (time_t)((buf[16]<<24) + (buf[17]<<16) + (buf[18]<<8) +buf[19]);
							
							struct tm *tbclock;
							//tbclock = localtime(&fault_time);
							tbclock = gmtime(&fault_time);

							if(lin_bal_num == 0)
							{
								/*无错误*/
								printf("lin_bal_num = %d\n",lin_bal_num);
								continue;
							}
							
							bzero(faultcomment,sizeof(faultcomment));
								
							sprintf(faultcomment,"&#37197;&#32622;&#25925;&#38556;");
							strcat(faultcomment,temp_value);


							char updateTime[30];
							sprintf(updateTime,"%d-%02d-%02d %02d:%02d:%02d",(1900+tbclock->tm_year),(1+tbclock->tm_mon),tbclock->tm_mday,tbclock->tm_hour,tbclock->tm_min,tbclock->tm_sec);
								
							bzero(sql,sizeof(sql));
							
							if((1+tbclock->tm_mon) < 10)
							{
								sprintf(nonth,"0%d",1+tbclock->tm_mon);
							}
							else
							{
								sprintf(nonth,"%d",1+tbclock->tm_mon);
							}

							sprintf(sql,"insert into ATC_ALARMHIS_%d%s(Sn,deviceid,alarm_id,faultcomment,unit_id,type,Report_time) values((select nvl(max(t.Sn)+1,1) from ATC_ALARMHIS_%d%s t), %d,%d,' %s ',%d,1,to_date('%s','yyyy-mm-dd hh24:mi:ss')) ",
								1900+tbclock->tm_year,nonth,1900+tbclock->tm_year,nonth,signal_info_data[signal_num].signal_id,buf[10],faultcomment,signal_info_data[signal_num].unit_id,updateTime);
							printf("%s\n",sql);
							st->setSQL(sql);
							st->executeUpdate();
							
						}
						//itc100,0x84,控制单元故障
						else if(buf[10] == 0x84)
						{
							unsigned int fault_data =  (buf[12]<<24) + (buf[13]<<16) + (buf[14]<<8) + buf[15];
							char temp_value[200];
							bzero(temp_value,200);

							for(i = 0 ; i < 6; i++)
							{
								unsigned int temp = 0x80 >> i;

								if((fault_data & temp) != 0)
								{
									if(( (i+1)==5) || ( (i+1)==6))
										continue;
									lin_bal_num++;
									char stemp[4];
									sprintf(stemp,"%d,",(i+1));
									strcat(temp_value,stemp);
								}
							}

							time_t fault_time = (time_t)((buf[16]<<24) + (buf[17]<<16) + (buf[18]<<8) +buf[19]);
							
							struct tm *tbclock;
							//tbclock = localtime(&fault_time);
							tbclock = gmtime(&fault_time);

							if(lin_bal_num == 0)
							{
								/*无错误*/
								printf("lin_bal_num = %d\n",lin_bal_num);
								continue;
							}
							
							bzero(faultcomment,sizeof(faultcomment));
								
							sprintf(faultcomment,"&#25511;&#21046;&#21333;&#20803;&#32570;&#22833;");
							strcat(faultcomment,temp_value);

							char updateTime[30];
							sprintf(updateTime,"%d-%02d-%02d %02d:%02d:%02d",(1900+tbclock->tm_year),(1+tbclock->tm_mon),tbclock->tm_mday,tbclock->tm_hour,tbclock->tm_min,tbclock->tm_sec);
								

							bzero(sql,sizeof(sql));
							
							if((1+tbclock->tm_mon) < 10)
							{
								sprintf(nonth,"0%d",1+tbclock->tm_mon);
							}
							else
							{
								sprintf(nonth,"%d",1+tbclock->tm_mon);
							}

							sprintf(sql,"insert into ATC_ALARMHIS_%d%s(Sn,deviceid,alarm_id,faultcomment,unit_id,type,Report_time) values((select nvl(max(t.Sn)+1,1) from ATC_ALARMHIS_%d%s t), %d,%d,' %s ',%d,1,to_date('%s','yyyy-mm-dd hh24:mi:ss')) ",
								1900+tbclock->tm_year,nonth,1900+tbclock->tm_year,nonth,signal_info_data[signal_num].signal_id,buf[10],faultcomment,signal_info_data[signal_num].unit_id,updateTime);
							printf("%s\n",sql);
							st->setSQL(sql);
							st->executeUpdate();
						}
						else if(buf[10] == 0x85)
						{
							//sprintf(faultcomment," 检测器故障!");

							unsigned int fault_data =  (buf[12]<<24) + (buf[13]<<16) + (buf[14]<<8) + buf[15];
							char temp_value[200];
							bzero(temp_value,200);

							for(i = 0 ; i < 32; i++)
							{
								unsigned int temp = 0x80000000 >> i;

								if((fault_data & temp) != 0)
								{
									lin_bal_num++;
									char stemp[4];
									sprintf(stemp,"%d,",(i+1));
									strcat(temp_value,stemp);
								}
							}

							time_t fault_time = (time_t)((buf[16]<<24) + (buf[17]<<16) + (buf[18]<<8) +buf[19]);
							
							struct tm *tbclock;
							//tbclock = localtime(&fault_time);
							tbclock = gmtime(&fault_time);

							if(lin_bal_num == 0)
							{
								/*无错误*/
								printf("lin_bal_num = %d\n",lin_bal_num);
								continue;
							}
							
							bzero(faultcomment,sizeof(faultcomment));
							sprintf(faultcomment,"&#26816;&#27979;&#22120;&#25925;&#38556;&#65306;&#32534;&#21495;");
							strcat(faultcomment,temp_value);

							char updateTime[30];
							sprintf(updateTime,"%d-%02d-%02d %02d:%02d:%02d",(1900+tbclock->tm_year),(1+tbclock->tm_mon),tbclock->tm_mday,tbclock->tm_hour,tbclock->tm_min,tbclock->tm_sec);
							
							bzero(sql,sizeof(sql));
							
							if((1+tbclock->tm_mon) < 10)
							{
								sprintf(nonth,"0%d",1+tbclock->tm_mon);
							}
							else
							{
								sprintf(nonth,"%d",1+tbclock->tm_mon);
							}

							sprintf(sql,"insert into ATC_ALARMHIS_%d%s(Sn,deviceid,alarm_id,faultcomment,unit_id,type,Report_time) values((select nvl(max(t.Sn)+1,1) from ATC_ALARMHIS_%d%s t), %d,%d,' %s ',%d,1,to_date('%s','yyyy-mm-dd hh24:mi:ss')) ",
								1900+tbclock->tm_year,nonth,1900+tbclock->tm_year,nonth,signal_info_data[signal_num].signal_id,buf[10],faultcomment,signal_info_data[signal_num].unit_id,updateTime);
							printf("%s\n",sql);
							st->setSQL(sql);
							st->executeUpdate();		

							signal_info_data[signal_num].detector_failures[buf[13]][0] = -1;
								

							
						}	

						//itc100,0x86 ，检测板通讯故障
						else if(buf[10] == 0x86)
						{
							unsigned int fault_data =  (buf[12]<<24) + (buf[13]<<16) + (buf[14]<<8) + buf[15];
							char temp_value[200];
							bzero(temp_value,200);

							for(i = 0 ; i < 8; i++)
							{
								unsigned int temp = 0x80 >> i;

								if((fault_data & temp) != 0)
								{
									lin_bal_num++;
									char stemp[4];
									sprintf(stemp,"%d,",(i+1));
									strcat(temp_value,stemp);
								}
							}

							time_t fault_time = (time_t)((buf[16]<<24) +( buf[17]<<16) + (buf[18]<<8) +buf[19]);
							
							struct tm *tbclock;
							//tbclock = localtime(&fault_time);
							tbclock = gmtime(&fault_time);

							if(lin_bal_num == 0)
							{
								/*无错误*/
								printf("lin_bal_num = %d\n",lin_bal_num);
								continue;
							}
							
							bzero(faultcomment,sizeof(faultcomment));
								
							sprintf(faultcomment,"&#26816;&#27979;&#26495;&#36890;&#35759;&#25925;&#38556;");
							strcat(faultcomment,temp_value);

							char updateTime[30];
							sprintf(updateTime,"%d-%02d-%02d %02d:%02d:%02d",(1900+tbclock->tm_year),(1+tbclock->tm_mon),tbclock->tm_mday,tbclock->tm_hour,tbclock->tm_min,tbclock->tm_sec);
							
							bzero(sql,sizeof(sql));
							
							if((1+tbclock->tm_mon) < 10)
							{
								sprintf(nonth,"0%d",1+tbclock->tm_mon);
							}
							else
							{
								sprintf(nonth,"%d",1+tbclock->tm_mon);
							}

							sprintf(sql,"insert into ATC_ALARMHIS_%d%s(Sn,deviceid,alarm_id,faultcomment,unit_id,type,Report_time) values((select nvl(max(t.Sn)+1,1) from ATC_ALARMHIS_%d%s t), %d,%d,' %s ',%d,1,to_date('%s','yyyy-mm-dd hh24:mi:ss')) ",
								1900+tbclock->tm_year,nonth,1900+tbclock->tm_year,nonth,signal_info_data[signal_num].signal_id,buf[10],faultcomment,signal_info_data[signal_num].unit_id,updateTime);
							printf("%s\n",sql);
							st->setSQL(sql);
							st->executeUpdate();
						}
							
						else if(buf[10] == 0x87)
						{
							//sprintf(faultcomment," 检测板缺失");


							unsigned int fault_data =  (buf[12]<<24) + (buf[13]<<16) + (buf[14]<<8) + buf[15];
							char temp_value[200];
							bzero(temp_value,200);

							for(i = 0 ; i < 8; i++)
							{
								unsigned int temp = 0x80 >> i;

								if((fault_data & temp) != 0)
								{
									lin_bal_num++;
									char stemp[4];
									sprintf(stemp,"%d,",(i+1));
									strcat(temp_value,stemp);
								}
							}

							time_t fault_time = (time_t)((buf[16]<<24) + (buf[17]<<16) + (buf[18]<<8) +buf[19]);
							
							struct tm *tbclock;
							//tbclock = localtime(&fault_time);
							tbclock = gmtime(&fault_time);

							if(lin_bal_num == 0)
							{
								/*无错误*/
								continue;
							}
							else
							{
								
								bzero(faultcomment,sizeof(faultcomment));
								sprintf(faultcomment,"&#26816;&#27979;&#26495;&#32570;&#22833;&#65306;&#26495;&#21495;");
								strcat(faultcomment,temp_value);

								char updateTime[30];
								sprintf(updateTime,"%d-%02d-%02d %02d:%02d:%02d",(1900+tbclock->tm_year),(1+tbclock->tm_mon),tbclock->tm_mday,tbclock->tm_hour,tbclock->tm_min,tbclock->tm_sec);
							
								bzero(sql,sizeof(sql));
							
								if((1+tbclock->tm_mon) < 10)
								{
									sprintf(nonth,"0%d",1+tbclock->tm_mon);
								}
								else
								{
									sprintf(nonth,"%d",1+tbclock->tm_mon);
								}

								sprintf(sql,"insert into ATC_ALARMHIS_%d%s(Sn,deviceid,alarm_id,faultcomment,unit_id,type,Report_time) values((select nvl(max(t.Sn)+1,1) from ATC_ALARMHIS_%d%s t), %d,%d,' %s ',%d,1,to_date('%s','yyyy-mm-dd hh24:mi:ss')) ",
									1900+tbclock->tm_year,nonth,1900+tbclock->tm_year,nonth,signal_info_data[signal_num].signal_id,buf[10],faultcomment,signal_info_data[signal_num].unit_id,updateTime);
								printf("%s\n",sql);
								st->setSQL(sql);
								st->executeUpdate();
								
								
							}
							
							
						}	

						//itc100,0x88，相位板通讯故障
						else if(buf[10] == 0x88)
						{
							unsigned int fault_data =  (buf[12]<<24) + (buf[13]<<16 )+ (buf[14]<<8) + buf[15];
							char temp_value[200];
							bzero(temp_value,200);

							for(i = 0 ; i < 8; i++)
							{
								unsigned int temp = 0x80 >> i;

								if((fault_data & temp) != 0)
								{
									lin_bal_num++;
									char stemp[4];
									sprintf(stemp,"%d,",(i+1));
									strcat(temp_value,stemp);
								}
							}

							time_t fault_time = (time_t)((buf[16]<<24) + (buf[17]<<16) + (buf[18]<<8) +buf[19]);
							
							struct tm *tbclock;
							//tbclock = localtime(&fault_time);
							tbclock = gmtime(&fault_time);

							if(lin_bal_num == 0)
							{
								/*无错误*/
								continue;
							}
							else
							{
								bzero(faultcomment,sizeof(faultcomment));
								sprintf(faultcomment,"&#30456;&#20301;&#26495;&#36890;&#35759;&#25925;&#38556;");
								strcat(faultcomment,temp_value);

								char updateTime[30];
								sprintf(updateTime,"%d-%02d-%02d %02d:%02d:%02d",(1900+tbclock->tm_year),(1+tbclock->tm_mon),tbclock->tm_mday,tbclock->tm_hour,tbclock->tm_min,tbclock->tm_sec);
							
								bzero(sql,sizeof(sql));
							
								if((1+tbclock->tm_mon) < 10)
								{
									sprintf(nonth,"0%d",1+tbclock->tm_mon);
								}
								else
								{
									sprintf(nonth,"%d",1+tbclock->tm_mon);
								}

								sprintf(sql,"insert into ATC_ALARMHIS_%d%s(Sn,deviceid,alarm_id,faultcomment,unit_id,type,Report_time) values((select nvl(max(t.Sn)+1,1) from ATC_ALARMHIS_%d%s t), %d,%d,' %s ',%d,1,to_date('%s','yyyy-mm-dd hh24:mi:ss')) ",
									1900+tbclock->tm_year,nonth,1900+tbclock->tm_year,nonth,signal_info_data[signal_num].signal_id,buf[10],faultcomment,signal_info_data[signal_num].unit_id,updateTime);
								printf("%s\n",sql);
								st->setSQL(sql);
								st->executeUpdate();
								
								
							}
						}
						else if(buf[10] == 0x89)
						{
							//sprintf(faultcomment," 相位板缺失");

							unsigned int fault_data =  (buf[12]<<24) + (buf[13]<<16) + (buf[14]<<8 )+ buf[15];
							char temp_value[200];
							bzero(temp_value,200);

							for(i = 0 ; i < 8; i++)
							{
								unsigned int temp = 0x80 >> i;

								if((fault_data & temp) != 0)
								{
									lin_bal_num++;
									char stemp[4];
									sprintf(stemp,"%d,",(i+1));
									strcat(temp_value,stemp);
								}
							}

							time_t fault_time = (time_t)((buf[16]<<24) + (buf[17]<<16) + (buf[18]<<8) +buf[19]);
							
							struct tm *tbclock;
							//tbclock = localtime(&fault_time);
							tbclock = gmtime(&fault_time);

							if(lin_bal_num == 0)
							{
								/*无错误*/
								continue;
							}
							else
							{
								
								bzero(faultcomment,sizeof(faultcomment));
								sprintf(faultcomment,"&#30456;&#20301;&#26495;&#32570;&#22833;&#65306;&#26495;&#21495;");
								strcat(faultcomment,temp_value);

								char updateTime[30];
								sprintf(updateTime,"%d-%02d-%02d %02d:%02d:%02d",(1900+tbclock->tm_year),(1+tbclock->tm_mon),tbclock->tm_mday,tbclock->tm_hour,tbclock->tm_min,tbclock->tm_sec);
							
								bzero(sql,sizeof(sql));
							
								if((1+tbclock->tm_mon) < 10)
								{
									sprintf(nonth,"0%d",1+tbclock->tm_mon);
								}
								else
								{
									sprintf(nonth,"%d",1+tbclock->tm_mon);
								}

								sprintf(sql,"insert into ATC_ALARMHIS_%d%s(Sn,deviceid,alarm_id,faultcomment,unit_id,type,Report_time) values((select nvl(max(t.Sn)+1,1) from ATC_ALARMHIS_%d%s t), %d,%d,' %s ',%d,1,to_date('%s','yyyy-mm-dd hh24:mi:ss')) ",
									1900+tbclock->tm_year,nonth,1900+tbclock->tm_year,nonth,signal_info_data[signal_num].signal_id,buf[10],faultcomment,signal_info_data[signal_num].unit_id,updateTime);
								printf("%s\n",sql);
								st->setSQL(sql);
								st->executeUpdate();
								
								
							}
							
						}

						else if(buf[10] == 0xa0)
							{
							//检测器长时间不动作报警
							unsigned int fault_data =  (buf[12]<<24) + (buf[13]<<16) + (buf[14]<<8) + buf[15];
							char temp_value[200];
							bzero(temp_value,200);

							for(i = 0 ; i < 32; i++)
							{
								unsigned int temp = 0x80000000 >> i;

								if((fault_data & temp) != 0)
								{
									lin_bal_num++;
									char stemp[4];
									sprintf(stemp,"%d,",(i+1));
									strcat(temp_value,stemp);
								}
							}

							time_t fault_time = (time_t)((buf[16]<<24) + (buf[17]<<16) + (buf[18]<<8) +buf[19]);
							
							struct tm *tbclock;
							//tbclock = localtime(&fault_time);
							tbclock = gmtime(&fault_time);

							if(lin_bal_num == 0)
							{
								/*无错误*/
								printf("lin_bal_num = %d\n",lin_bal_num);
								continue;
							}
														
							char updateTime[30];
							sprintf(updateTime,"%d-%02d-%02d %02d:%02d:%02d",(1900+timeinfo->tm_year),(1+timeinfo->tm_mon),timeinfo->tm_mday,timeinfo->tm_hour,timeinfo->tm_min,timeinfo->tm_sec);
							
							
							bzero(faultcomment,sizeof(faultcomment));
							sprintf(faultcomment,"&#26816;&#27979;&#22120;&#38271;&#26102;&#38388;&#19981;&#21160;&#20316;&#25253;&#35686;&#65306;&#32534;&#21495;");
							strcat(faultcomment,temp_value);

							bzero(sql,sizeof(sql));
							
								if((1+timeinfo->tm_mon) < 10)
								{
									sprintf(nonth,"0%d",1+timeinfo->tm_mon);
								}
								else
								{
									sprintf(nonth,"%d",1+timeinfo->tm_mon);
								}

							sprintf(sql,"insert into ATC_ALARMHIS_%d%s(Sn,deviceid,alarm_id,faultcomment,unit_id,type,Report_time) values((select nvl(max(t.Sn)+1,1) from ATC_ALARMHIS_%d%s t), %d,%d,' %s ',%d,1,to_date('%s','yyyy-mm-dd hh24:mi:ss')) ",
									1900+timeinfo->tm_year,nonth,1900+timeinfo->tm_year,nonth,signal_info_data[signal_num].signal_id,buf[10],faultcomment,signal_info_data[signal_num].unit_id,updateTime);
							printf("%s\n",sql);
							st->setSQL(sql);
							st->executeUpdate();

							
							
							
						}
						else if(buf[10] == 0xa1)
							{
							//电压报警
							char updateTime[30];
							sprintf(updateTime,"%d-%02d-%02d %02d:%02d:%02d",(1900+timeinfo->tm_year),(1+timeinfo->tm_mon),timeinfo->tm_mday,timeinfo->tm_hour,timeinfo->tm_min,timeinfo->tm_sec);
							

							bzero(faultcomment,sizeof(faultcomment));
							sprintf(faultcomment,"&#30005;&#21387;&#25253;&#35686;");

							bzero(sql,sizeof(sql));
							
								if((1+timeinfo->tm_mon) < 10)
								{
									sprintf(nonth,"0%d",1+timeinfo->tm_mon);
								}
								else
								{
									sprintf(nonth,"%d",1+timeinfo->tm_mon);
								}

							sprintf(sql,"insert into ATC_ALARMHIS_%d%s(Sn,deviceid,alarm_id,faultcomment,unit_id,type,Report_time) values((select nvl(max(t.Sn)+1,1) from ATC_ALARMHIS_%d%s t), %d,%d,' %s ',%d,1,to_date('%s','yyyy-mm-dd hh24:mi:ss')) ",
									1900+timeinfo->tm_year,nonth,1900+timeinfo->tm_year,nonth,signal_info_data[signal_num].signal_id,buf[10],faultcomment,signal_info_data[signal_num].unit_id,updateTime);
							printf("%s\n",sql);
							st->setSQL(sql);
							st->executeUpdate();
						
						}
						else if(buf[10] == 0xa2)
							{
							//开门报警
							unsigned int fault_data =  buf[12];
							char temp_value[20];
							bzero(temp_value,20);

							bzero(sql,sizeof(sql));
							
							if((1+timeinfo->tm_mon) < 10)
							{
								sprintf(nonth,"0%d",1+timeinfo->tm_mon);
							}
							else
							{
								sprintf(nonth,"%d",1+timeinfo->tm_mon);
							}

							char updateTime[30];
							sprintf(updateTime,"%d-%02d-%02d %02d:%02d:%02d",(1900+timeinfo->tm_year),(1+timeinfo->tm_mon),timeinfo->tm_mday,timeinfo->tm_hour,timeinfo->tm_min,timeinfo->tm_sec);

							for(i = 0 ; i < 3; i++)
							{
								unsigned int temp = 0x80 >> i;

								switch(i)
								{
									case 0:
										if((fault_data & temp) != 0)
										{
											bzero(faultcomment,sizeof(faultcomment));
											sprintf(faultcomment,"&#21069;&#38376;&#24320;&#38376;&#25253;&#35686;");

											sprintf(sql,"insert into ATC_ALARMLIST(Sn,device_id,alarm_id,processstatus_id,occur_time,faultcomment,unit_id,memo) values((select nvl(max(t.Sn)+1,1) from ATC_ALARMLIST t),%d,%d,0,sysdate,'%s',%d,%d) ",
									buf[7] * 256 + buf[8],buf[10],faultcomment,signal_info_data[signal_num].unit_id,1);

											printf("%s\n",sql);
											st->setSQL(sql);
											st->executeUpdate();

											sprintf(sql,"insert into ATC_ALARMHIS_%d%s(Sn,deviceid,alarm_id,faultcomment,unit_id,type,Report_time) values((select nvl(max(t.Sn)+1,1) from ATC_ALARMHIS_%d%s t), %d,%d,' %s ',%d,1,to_date('%s','yyyy-mm-dd hh24:mi:ss')) ",
									1900+timeinfo->tm_year,nonth,1900+timeinfo->tm_year,nonth,signal_info_data[signal_num].signal_id,buf[10],faultcomment,signal_info_data[signal_num].unit_id,updateTime);
											printf("%s\n",sql);
											st->setSQL(sql);
											st->executeUpdate();
										}
										else
										{
											sprintf(sql,"delete  from ATC_ALARMLIST where device_id = %d and alarm_id = 162 and memo = 1 ",signal_info_data[signal_num].signal_id);
											st->setSQL(sql);
											st->executeUpdate();

											sprintf(sql,"insert into ATC_ALARMHIS_%d%s(Sn,deviceid,alarm_id,faultcomment,unit_id,type,Report_time) values((select nvl(max(t.Sn)+1,1) from ATC_ALARMHIS_%d%s t), %d,%d,' &#21069;&#38376;&#25171;&#24320;&#25253;&#35686;&#24674;&#22797; ',%d,0,sysdate) ",
										1900+timeinfo->tm_year,nonth,1900+timeinfo->tm_year,nonth,signal_info_data[signal_num].signal_id,buf[10],signal_info_data[signal_num].unit_id);
											printf("%s\n",sql);
											st->setSQL(sql);
											st->executeUpdate();
										}
										break;
										case 1:
											if((fault_data & temp) != 0)
										{
											bzero(faultcomment,sizeof(faultcomment));
											sprintf(faultcomment,"&#21518;&#38376;&#24320;&#38376;&#25253;&#35686;");

											sprintf(sql,"insert into ATC_ALARMLIST(Sn,device_id,alarm_id,processstatus_id,occur_time,faultcomment,unit_id,memo) values((select nvl(max(t.Sn)+1,1) from ATC_ALARMLIST t),%d,%d,0,sysdate,'%s',%d,%d) ",
									buf[7] * 256 + buf[8],buf[10],faultcomment,signal_info_data[signal_num].unit_id,2);

											printf("%s\n",sql);
											st->setSQL(sql);
											st->executeUpdate();

											sprintf(sql,"insert into ATC_ALARMHIS_%d%s(Sn,deviceid,alarm_id,faultcomment,unit_id,type,Report_time) values((select nvl(max(t.Sn)+1,1) from ATC_ALARMHIS_%d%s t), %d,%d,' %s ',%d,1,to_date('%s','yyyy-mm-dd hh24:mi:ss')) ",
									1900+timeinfo->tm_year,nonth,1900+timeinfo->tm_year,nonth,signal_info_data[signal_num].signal_id,buf[10],faultcomment,signal_info_data[signal_num].unit_id,updateTime);
											printf("%s\n",sql);
											st->setSQL(sql);
											st->executeUpdate();
										}
											else
										{
											sprintf(sql,"delete  from ATC_ALARMLIST where device_id = %d and alarm_id = 162 and memo = 2 ",signal_info_data[signal_num].signal_id);
											st->setSQL(sql);
											st->executeUpdate();

											sprintf(sql,"insert into ATC_ALARMHIS_%d%s(Sn,deviceid,alarm_id,faultcomment,unit_id,type,Report_time) values((select nvl(max(t.Sn)+1,1) from ATC_ALARMHIS_%d%s t), %d,%d,' &#21518;&#38376;&#25171;&#24320;&#25253;&#35686;&#24674;&#22797; ',%d,0,sysdate) ",
										1900+timeinfo->tm_year,nonth,1900+timeinfo->tm_year,nonth,signal_info_data[signal_num].signal_id,buf[10],signal_info_data[signal_num].unit_id);
											printf("%s\n",sql);
											st->setSQL(sql);
											st->executeUpdate();
										}
											break;
										case 2:
											if((fault_data & temp) != 0)
										{
											bzero(faultcomment,sizeof(faultcomment));
											sprintf(faultcomment,"&#20391;&#38376;&#24320;&#38376;&#25253;&#35686;");

											sprintf(sql,"insert into ATC_ALARMLIST(Sn,device_id,alarm_id,processstatus_id,occur_time,faultcomment,unit_id,memo) values((select nvl(max(t.Sn)+1,1) from ATC_ALARMLIST t),%d,%d,0,sysdate,'%s',%d,%d) ",
									buf[7] * 256 + buf[8],buf[10],faultcomment,signal_info_data[signal_num].unit_id,3);

											printf("%s\n",sql);
											st->setSQL(sql);
											st->executeUpdate();

											sprintf(sql,"insert into ATC_ALARMHIS_%d%s(Sn,deviceid,alarm_id,faultcomment,unit_id,type,Report_time) values((select nvl(max(t.Sn)+1,1) from ATC_ALARMHIS_%d%s t), %d,%d,' %s ',%d,1,to_date('%s','yyyy-mm-dd hh24:mi:ss')) ",
									1900+timeinfo->tm_year,nonth,1900+timeinfo->tm_year,nonth,signal_info_data[signal_num].signal_id,buf[10],faultcomment,signal_info_data[signal_num].unit_id,updateTime);
											printf("%s\n",sql);
											st->setSQL(sql);
											st->executeUpdate();
										}
											else
										{
											sprintf(sql,"delete  from ATC_ALARMLIST where device_id = %d and alarm_id = 162 and memo = 3 ",signal_info_data[signal_num].signal_id);
											st->setSQL(sql);
											st->executeUpdate();

											sprintf(sql,"insert into ATC_ALARMHIS_%d%s(Sn,deviceid,alarm_id,faultcomment,unit_id,type,Report_time) values((select nvl(max(t.Sn)+1,1) from ATC_ALARMHIS_%d%s t), %d,%d,'&#20391;&#38376;&#25171;&#24320;&#25253;&#35686;&#24674;&#22797; ',%d,0,sysdate) ",
										1900+timeinfo->tm_year,nonth,1900+timeinfo->tm_year,nonth,signal_info_data[signal_num].signal_id,buf[10],signal_info_data[signal_num].unit_id);
											printf("%s\n",sql);
											st->setSQL(sql);
											st->executeUpdate();
										}
											break;
											
								}
							}
							
							

							

							
						
						}
						else if(buf[10] == 0xa3)
							{
							//震动报警
							char updateTime[30];
							sprintf(updateTime,"%d-%02d-%02d %02d:%02d:%02d",(1900+timeinfo->tm_year),(1+timeinfo->tm_mon),timeinfo->tm_mday,timeinfo->tm_hour,timeinfo->tm_min,timeinfo->tm_sec);
							

							bzero(faultcomment,sizeof(faultcomment));
							sprintf(faultcomment,"&#38663;&#21160;&#25253;&#35686;");


							bzero(sql,sizeof(sql));
							
								if((1+timeinfo->tm_mon) < 10)
								{
									sprintf(nonth,"0%d",1+timeinfo->tm_mon);
								}
								else
								{
									sprintf(nonth,"%d",1+timeinfo->tm_mon);
								}

							sprintf(sql,"insert into ATC_ALARMHIS_%d%s(Sn,deviceid,alarm_id,faultcomment,unit_id,type,Report_time) values((select nvl(max(t.Sn)+1,1) from ATC_ALARMHIS_%d%s t), %d,%d,' %s ',%d,1,to_date('%s','yyyy-mm-dd hh24:mi:ss')) ",
									1900+timeinfo->tm_year,nonth,1900+timeinfo->tm_year,nonth,signal_info_data[signal_num].signal_id,buf[10],faultcomment,signal_info_data[signal_num].unit_id,updateTime);
							printf("%s\n",sql);
							st->setSQL(sql);
							st->executeUpdate();
						
						}
						else if(buf[10] == 0xa4)
							{
							//烟雾报警
							char updateTime[30];
							sprintf(updateTime,"%d-%02d-%02d %02d:%02d:%02d",(1900+timeinfo->tm_year),(1+timeinfo->tm_mon),timeinfo->tm_mday,timeinfo->tm_hour,timeinfo->tm_min,timeinfo->tm_sec);
							

							bzero(faultcomment,sizeof(faultcomment));
							sprintf(faultcomment,"&#28895;&#38654;&#25253;&#35686;");


							bzero(sql,sizeof(sql));
							
								if((1+timeinfo->tm_mon) < 10)
								{
									sprintf(nonth,"0%d",1+timeinfo->tm_mon);
								}
								else
								{
									sprintf(nonth,"%d",1+timeinfo->tm_mon);
								}

							sprintf(sql,"insert into ATC_ALARMHIS_%d%s(Sn,deviceid,alarm_id,faultcomment,unit_id,type,Report_time) values((select nvl(max(t.Sn)+1,1) from ATC_ALARMHIS_%d%s t), %d,%d,' %s ',%d,1,to_date('%s','yyyy-mm-dd hh24:mi:ss')) ",
									1900+timeinfo->tm_year,nonth,1900+timeinfo->tm_year,nonth,signal_info_data[signal_num].signal_id,buf[10],faultcomment,signal_info_data[signal_num].unit_id,updateTime);
							printf("%s\n",sql);
							st->setSQL(sql);
							st->executeUpdate();
						
						}
						else if(buf[10] == 0xa5)
							{
							//水浸报警
							char updateTime[30];
							sprintf(updateTime,"%d-%02d-%02d %02d:%02d:%02d",(1900+timeinfo->tm_year),(1+timeinfo->tm_mon),timeinfo->tm_mday,timeinfo->tm_hour,timeinfo->tm_min,timeinfo->tm_sec);
							

							bzero(faultcomment,sizeof(faultcomment));
							sprintf(faultcomment,"&#27700;&#28024;&#25253;&#35686;");

							bzero(sql,sizeof(sql));
							
								if((1+timeinfo->tm_mon) < 10)
								{
									sprintf(nonth,"0%d",1+timeinfo->tm_mon);
								}
								else
								{
									sprintf(nonth,"%d",1+timeinfo->tm_mon);
								}

							sprintf(sql,"insert into ATC_ALARMHIS_%d%s(Sn,deviceid,alarm_id,faultcomment,unit_id,type,Report_time) values((select nvl(max(t.Sn)+1,1) from ATC_ALARMHIS_%d%s t), %d,%d,' %s ',%d,1,to_date('%s','yyyy-mm-dd hh24:mi:ss')) ",
									1900+timeinfo->tm_year,nonth,1900+timeinfo->tm_year,nonth,signal_info_data[signal_num].signal_id,buf[10],faultcomment,signal_info_data[signal_num].unit_id,updateTime);
							printf("%s\n",sql);
							st->setSQL(sql);
							st->executeUpdate();
						
						}
								
					}
					}
					/*占有率 流量数据*/
					else if(buf[10] == 0x40 || buf[10] == 0x41)
					{
					//	printf("40 and 41 \n");
						if(buf[10] == 0x40)                        
						{
							signal_info_data[signal_num].signal_traffic_share[buf[11]][0] = buf[13];
							signal_info_data[signal_num].signal_traffic_share[buf[11]][1] = buf[14];

							signal_info_data[signal_num].signal_traffic_share_flag[buf[11]]++;
						
						}
						else if(buf[10] = 0x41)
						{
							signal_info_data[signal_num].signal_traffic_share[buf[11]][2] = buf[13];
							signal_info_data[signal_num].signal_traffic_share_flag[buf[11]]++;
						#if 1
							

							check_time = (time_t)(buf[14] *256*256*256 + buf[15] *256*256 + buf[16] *256 + buf[17]);

							//timeinfo_check = localtime(&check_time);
							timeinfo_check = gmtime(&check_time);

							bzero(check_time_num,sizeof(check_time_num));
							sprintf(check_time_num,"%d-%d-%d %d:%d:%d",1900+timeinfo_check->tm_year, 1+timeinfo_check->tm_mon,timeinfo_check->tm_mday,timeinfo_check->tm_hour,timeinfo_check->tm_min,timeinfo_check->tm_sec);

							
							
						#endif	
						}
#if 1
						if(signal_info_data[signal_num].signal_traffic_share_flag[buf[11]] == 2)
						{
						//	printf("signal_info_data[signal_num].signal_traffic_share_flag[buf[11]] = %d\n",signal_info_data[signal_num].signal_traffic_share_flag[buf[11]]);
						
							signal_info_data[signal_num].signal_traffic_share_flag[buf[11]] = 0;

							
								//  lt=(time_t)count;     printf("具体时间：%s\n",asctime(localtime(&lt)));
						#if 1
							/*插入表*/
							bzero(sql,sizeof(sql));
							sprintf(sql,"insert into DETECTOR_REAL_DATA(UNIT_ID,DETECTOR_ID,UPDATE_TIME,FLOW,OCC) values(%d,%d,to_date('%s','yyyy-mm-dd hh24:mi:ss'),%d,%d)",
								signal_info_data[signal_num].unit_id,buf[11],check_time_num,signal_info_data[signal_num].signal_traffic_share[buf[11]][0] * 256 + signal_info_data[signal_num].signal_traffic_share[buf[11]][1],
								signal_info_data[signal_num].signal_traffic_share[buf[11]][2]);
						//	sprintf(sql,"insert into DETECTOR_REAL_DATA(UNIT_ID,DETECTOR_ID,UPDATE_TIME,FLOW,OCC) values(%d,%d,sysdate,%d,%d)",
						//		signal_info_data[signal_num].unit_id,buf[11],check_time_num,signal_info_data[signal_num].signal_traffic_share[buf[11]][0] * 256 + signal_info_data[signal_num].signal_traffic_share[buf[11]][1],
						//		signal_info_data[signal_num].signal_traffic_share[buf[11]][2]);

		//					printf("%s\n",sql);
								
							
						//	strcat(all_sql,sql);
						//	sprintf(all_sql,"%s\r",all_sql);

							
					//		lis_n++;
							st->setSQL(sql);
							st->executeUpdate();

							
							
						#endif

						//	signal_info_data[signal_num].signal_traffic_share_flag[buf[11]] = 0;

						}
#endif			
#if 0
						if(signal_info_data[signal_num].signal_traffic_share_flag[buf[11]] == 2)
						{
							bzero(sql,sizeof(sql));
							sprintf(sql," select %d as aa,%d as bb,sysdate as cc,%d as dd,%d as ee from dual union all ",
								signal_info_data[signal_num].unit_id,buf[11],signal_info_data[signal_num].signal_traffic_share[buf[11]][0] * 256 + signal_info_data[signal_num].signal_traffic_share[buf[11]][1],
								signal_info_data[signal_num].signal_traffic_share[buf[11]][2]);
							strcat(all_sql,sql);
							lis_n++;
							signal_info_data[signal_num].signal_traffic_share_flag[buf[11]] = 0;
						}
#endif
						
					}
				
					
		    		}
#if 0		  
				 if(lis_n != 0)
				 {
				 	string str(all_sql);

					str =str.substr(0,str.length()-10);

					str = str +"))";

			//		strcat(all_sql,"insert into DETECTOR_REAL_DATA(UNIT_ID,DETECTOR_ID,UPDATE_TIME,FLOW,OCC) values(11,36,sysdate,0,0)");
				 	cout << str<< endl; 
				 	st->setSQL(str);
				 	st->executeUpdate();

						
					 
					
					printf("@@@@@@@@@@@@@@@@@@@@@@@@@\n");
					  bzero(all_sql,sizeof(all_sql));
					  sprintf(all_sql,"insert into DETECTOR_REAL_DATA(UNIT_ID,DETECTOR_ID,UPDATE_TIME,FLOW,OCC) ( select * from (");
			
					  lis_n = 0;
					
					  
				 }
#endif		 
				
			//	sleep(1);
				/*************************/
				
				break;
			}
			catch (SQLException &sqlExcp)
		    	{
		    		env = Environment::createEnvironment(Environment::OBJECT); 
				 env->terminateConnection(conn_parsing);  
				Environment::terminateEnvironment(env);  
				sleep(5);
				
				//printf("!!!!!!!!!!!887!!!!!!!!!!!!!!!!!!!!!\n");
				while(1)
				{
					//printf("!!!!!!!!!!!890!!!!!!!!!!!!!!!!!!!!!\n");
				#if 1
					unsigned char* ll_buf;
					while (! queue_empty(sq))
		   		 	{  
		   		 	
						dequeue(sq, (unsigned char**)&ll_buf);
					}
				#endif	
					try
					{
				//		 username = "atc";  
    				//		userpass = "atc";  
    				//		connstr = "192.168.1.24:1521/oracle"; 

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
					//	printf("!!!!!!!!!!!892!!!!!!!!!!!!!!!!!!!!!\n");
						st->setAutoCommit (TRUE);
					//	printf("!!!!!!!!!!!904!!!!!!!!!!!!!!!!!!!!!\n");
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
		
		
	}
}



