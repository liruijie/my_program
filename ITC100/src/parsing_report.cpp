/*************************************************** 
 * 文件名：parsing_report.cpp
 * 版权：
 * 描述：处理实时上报来的状态数据
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
#include "signal_report.h"
#include "occi_opration.h"
#include "public_data.h"
#include "parsing_report.h"

using namespace std;
using namespace oracle::occi;

#define BUFF_NUM  512

void *signal_report_cl(void *arg);
long get_time(void);


/* 批量操作用的线程*/
void *signal_report_pl_1(void *arg);
void *signal_report_pl_2(void *arg);




/*************************************************** 
 * 函数名：parsing_signal_report
 * 功能描述：把所有信号机的实时数据更新到表中
 *被访问的表：
 *被修改的表：REAL_SIGNAL_STATUS  REAL_SIGNAL_PLAN_STATUS 
* 参数说明：无
*返回值: 无
***************************************************/

/*************单线程处理目前不用了******************/
# if 0
/*一个线程更新所有*/
void parsing_signal_report()
{

	Environment *env = Environment::createEnvironment(Environment::OBJECT);  
	string username = "atc";  
    	string userpass = "atc";  
    	string connstr = "192.168.1.101:1521/utms"; 
	Connection *conn_parsing = env->createConnection(username, userpass, connstr);  
	
    	 if(conn_parsing == NULL)  
    	{  
  	  cout << "access oracle failed..." << endl;  
   	 }  
    
	Statement *st = conn_parsing->createStatement(); 
	st->setAutoCommit (TRUE);
	
	while(1)
	{
		
		int  i;
		char sql[10000];
		
		//printf("zhi xing yi ci \n");

		char sql_yy[65535];
#if 1	
		for(i = 1 ; i < signal_number;i++)
		{
			//printf(" id =%d ,signal_info_state = %d\n",signal_info_data[i].signal_id,signal_info_data[i].signal_info_state);

			bzero(sql_yy,sizeof(sql_yy));
			if(signal_info_data[i].signal_info_state == 0)
			{
				//printf("wei  NULL !!!!!!!!!!!!!!!!!!!!!!!!!!");
				continue;
			}

			if(signal_info_data[i].l_online_alarm == 0)
			{
			//	printf("signal id = %d  time = %s\n",i,signal_info_data[i].update_time);
				
		#if 1		
				sprintf(sql,"update REAL_SIGNAL_STATUS set Xiangweidengtai='%s',detect_status='%s',detect_data='%s',detect_urgent_status='%s',detect_pri_status='%s',door_status='%s',BoardOnline='%s' ,update_time=sysdate ,signal_time = to_date('%s','yyyy-mm-dd hh24:mi:ss') where signal_id = %d ",
					signal_info_data[i].phase_lht,signal_info_data[i].detect_status,signal_info_data[i].detect_data_status,signal_info_data[i].detect_urgent_status,
					signal_info_data[i].detect_pri_status,signal_info_data[i].door_status,signal_info_data[i].board_state,signal_info_data[i].update_time,
					signal_info_data[i].signal_id);
				//printf("%s\n",sql);
				st->setSQL(sql);
				st->executeUpdate();

				sprintf(sql,"update REAL_SIGNAL_PLAN_STATUS set cur_plan=%d,control_model=%d,cur_week=%d,cur_stage=%d,xietiaoxuhao=%d,xietiaoshijian=%d,fangxingshijian=%d ,shengyushijian=%d ,update_time=sysdate  where signal_id = %d ",
					signal_info_data[i].cur_plan,signal_info_data[i].control_model,signal_info_data[i].cur_week,signal_info_data[i].cur_stage,
					signal_info_data[i].coordination_number,signal_info_data[i].coordination_time,signal_info_data[i].release_time,signal_info_data[i].rest_time,signal_info_data[i].signal_id);
			//	printf("%s\n",sql);
				st->setSQL(sql);
				st->executeUpdate();
		#endif
				
			
			}
			
		}
		
		
#endif
#if 0 
		printf(" Xiangweidengtai=%s\n",signal_info_data[1].phase_lht);
		printf("detect_status =%s\n",signal_info_data[1].detect_status);
		printf("detect_urgent_status=%s\n",signal_info_data[1].detect_urgent_status);
		printf(" door_status=%s\n",signal_info_data[1].door_status);
		printf(" BoardOnline=%s\n",signal_info_data[1].board_state);
#endif
		
		
		//sleep(1);
	}
}

#endif
/*************单线程处理目前不用了******************/



/*************多线程处理，40个信号机一个线程******************/
#if 0
//Statement *st_re;
int num_xc;

/*一个线程更新40台*/
void parsing_signal_report()
{
	int i;
	num_xc = 0;
	int res;
	pthread_t pth_signal_report[1000];

#if 0	
	Environment *env = Environment::createEnvironment(Environment::OBJECT);  
	string username = "atc";  
    	string userpass = "atc";  
    	string connstr = "192.168.1.101:1521/utms"; 
	Connection *conn_parsing = env->createConnection(username, userpass, connstr);  
	
    	 if(conn_parsing == NULL)  
    	{  
  	  cout << "access oracle failed..." << endl;  
   	 }  
    
	st_re = conn_parsing->createStatement(); 
	st_re->setAutoCommit (TRUE);
#endif

	for(i= 0; i < signal_number /40 + 1;i++ )
	{
		printf("parsing  i  = %d\n",i);
		printf("num_xc  = %d\n",num_xc);

		res = pthread_create(&(pth_signal_report[i]),NULL,signal_report_cl,NULL);
		if(res != 0)
		{
			printf("!!!!!!!!!!!!!!!!!! shi bai\n");
			//perror("Fail to create pthread");
			//exit(EXIT_FAILURE);
		}
		

		usleep(100000);
		num_xc ++;
		
	}

	while(1)
	{
		sleep(10000);
	}

}

void *signal_report_cl(void *arg)
{
#if 1
	Environment *env = Environment::createEnvironment(Environment::OBJECT);  

//	string username = "atc";  
//    	string userpass = "atc";  
//    	string connstr = "192.168.1.24:1521/oracle";

	string username = oracle_username;  
    	string userpass = oracle_password;  
    	string connstr = oracle_connectstring; 
		
	Statement *st_re ;
	Connection *conn_parsing;
	


	while(1)
	{
		try
		{
			conn_parsing= env->createConnection(username, userpass, connstr);  
		    	 if(conn_parsing == NULL)  
		    	{  
		  	  cout << "access oracle failed..." << endl;  
		   	 }  
		    
			st_re = conn_parsing->createStatement(); 
			st_re->setAutoCommit (TRUE);
			break;
		}
		catch (SQLException &sqlExcp)
	    	{
	    		
			 env->terminateConnection(conn_parsing);  
			Environment::terminateEnvironment(env);  
			cout << "oracle  failed: " << sqlExcp.getMessage() << endl; 
			printf("creat signal_report_cl  oracle fail!\n");
	    	}
	}
	
#endif

#if 1
	int max_num;
	int num = num_xc;

	long time1,time2;
	
	if(num  == (signal_number /40 ) )
	{
		max_num = signal_number;
	}
	else
	{
		max_num = num * 40 + 41;
	}
	printf("********************************\n");
	printf("signal_report_cl   num   = %d : max_num = %d\n",num ,max_num);
	printf("signal_number = %d\n",signal_number);
	printf("********************************\n");

	while(1)
	{
		
		int  i;
		char sql[10000];
		
	//	printf("zhi xing yi ci \n");
		
		
		time1 =  get_time();
		
		while(1)
		{
			try
			{
				for(i = num * 40 + 1 ; i < max_num;i++)
				{
					//printf(" id =%d ,signal_info_state = %d\n",signal_info_data[i].signal_id,signal_info_data[i].signal_info_state);

					
					if(signal_info_data[i].signal_info_state == 0)
					{
						//printf("wei  NULL !!!!!!!!!!!!!!!!!!!!!!!!!!");
						continue;
					}

					if(signal_info_data[i].l_online_alarm == 0)
					{
					//	printf("signal id = %d  time = %s\n",i,signal_info_data[i].update_time);
						
				
						sprintf(sql,"update REAL_SIGNAL_STATUS set Xiangweidengtai='%s',detect_status='%s',detect_data='%s',detect_urgent_status='%s',detect_pri_status='%s',door_status='%s',BoardOnline='%s' ,update_time=sysdate ,signal_time = to_date('%s','yyyy-mm-dd hh24:mi:ss') where signal_id = %d ",
							signal_info_data[i].phase_lht,signal_info_data[i].detect_status,signal_info_data[i].detect_data_status,signal_info_data[i].detect_urgent_status,
							signal_info_data[i].detect_pri_status,signal_info_data[i].door_status,signal_info_data[i].board_state,signal_info_data[i].update_time,
							signal_info_data[i].signal_id);
				//		printf("%s\n",sql);
						st_re->setSQL(sql);
						st_re->executeUpdate();

						sprintf(sql,"update REAL_SIGNAL_PLAN_STATUS set cur_plan=%d,control_model=%d,cur_week=%d,cur_stage=%d,xietiaoxuhao=%d,xietiaoshijian=%d,fangxingshijian=%d ,shengyushijian=%d ,update_time=sysdate  where signal_id = %d ",
							signal_info_data[i].cur_plan,signal_info_data[i].control_model,signal_info_data[i].cur_week,signal_info_data[i].cur_stage,
							signal_info_data[i].coordination_number,signal_info_data[i].coordination_time,signal_info_data[i].release_time,signal_info_data[i].rest_time,signal_info_data[i].signal_id);
				//		printf("%s\n",sql);
						st_re->setSQL(sql);
						st_re->executeUpdate();
					

				
					}


				//	printf("zhi xing yi ci aaaaaaaaaaaaaaaaa\n");
					usleep(1000);
				#if 0	
					if(num == signal_number /20 )
					{
						usleep(200000);
					}
				#endif
					
				}

				
				break;
			}
			catch (SQLException &sqlExcp)
		    	{	

				printf("sql chucuole !!!!!!\n");

				cout << "oracle  failed: " << sqlExcp.getMessage() << endl; 
		    		while(1)
				{
					try
					{
						conn_parsing= env->createConnection(username, userpass, connstr);  
					    	 if(conn_parsing == NULL)  
					    	{  
					  	  cout << "access oracle failed..." << endl;  
					   	 }  
					    
						st_re = conn_parsing->createStatement(); 
						st_re->setAutoCommit (TRUE);
						break;
					}
					catch (SQLException &sqlExcp)
				    	{
						 env->terminateConnection(conn_parsing);  
						Environment::terminateEnvironment(env);  
						sleep(1);

						while(1)
						{
							try
							{
								username = oracle_username;  
    								userpass = oracle_password;  
    								connstr = oracle_connectstring; 
									
								conn_parsing= env->createConnection(username, userpass, connstr);  
							    	 if(conn_parsing == NULL)  
							    	{  
							  	  cout << "access oracle failed..." << endl;  
							   	 }  
							    
								st_re = conn_parsing->createStatement(); 
								st_re->setAutoCommit (TRUE);
								break;
							}
							catch (SQLException &sqlExcp)
						    	{
								 env->terminateConnection(conn_parsing);  
								Environment::terminateEnvironment(env);  
								printf("creat signal_report_cl  oracle fail!\n");
						    	}
						}

						sleep(1);
						
				    	}
				}
				
		    	}
		}
		
		
		
	//	printf("parsing  num= %d\n",num);

		time2 =  get_time();
#if 0		
		printf("##################\n");
		printf("  id == %d  time = %d\n",num,time2 - time1);
		printf("##################\n");
		//sleep(1);
#endif		
	}

	

	

#endif
	
}



#endif
/*************多现场处理，40个信号机一个线程******************/



/*************批量操作更新实时状态测试中******************/
#if 2
/*批量操作*/
void parsing_signal_report()
{
	int i;
	int res;
	pthread_t pth_signal_report[2];

#if 0	
	Environment *env = Environment::createEnvironment(Environment::OBJECT);  
	string username = "atc";  
    	string userpass = "atc";  
    	string connstr = "192.168.1.101:1521/utms"; 
	Connection *conn_parsing = env->createConnection(username, userpass, connstr);  
	
    	 if(conn_parsing == NULL)  
    	{  
  	  cout << "access oracle failed..." << endl;  
   	 }  
    
	st_re = conn_parsing->createStatement(); 
	st_re->setAutoCommit (TRUE);
#endif
	printf("new pthread ,file=%s,func=%s,line=%d\n",__FILE__,__FUNCTION__,__LINE__);
	sleep(3);
	/*更新REAL_SIGNAL_STATUS  REAL_SIGNAL_PLAN_STATUS*/
	res = pthread_create(&(pth_signal_report[0]),NULL,signal_report_pl_1,NULL);
	if(res != 0)
	{
		printf("!!!!!!!!!!!!!!!!!! shi bai\n");
		//perror("Fail to create pthread");
		//exit(EXIT_FAILURE);
	}
	sleep(4);
	/*更新REAL_SIGNAL_PLAN_STATUS*/
	res = pthread_create(&(pth_signal_report[1]),NULL,signal_report_pl_2,NULL);
	if(res != 0)
	{
		printf("!!!!!!!!!!!!!!!!!! shi bai\n\n");
		//perror("Fail to create pthread");
		//exit(EXIT_FAILURE);
	}
		



	while(1)
	{
		sleep(10000);
	}

}


void *signal_report_pl_1(void *arg)
{

	while(1)
	{
		
		int  i;
		char sql[10000];
		
		//printf("zhi xing yi ci \n");



		string userName = oracle_username;  
	    	string password = oracle_password;  
	    	string connectString = oracle_connectstring; 

		printf("new pthread ,file=%s,func=%s,line=%d\n",__FILE__,__FUNCTION__,__LINE__);	
		unsigned int maxConn=5;
		unsigned int minConn=1;
		unsigned int incrConn=2;

		 oracle::occi::Environment *env_pl = oracle::occi::Environment::createEnvironment(Environment::OBJECT);
		   ConnectionPool *connPool=env_pl->createConnectionPool(
	         userName,
	            password,
	        connectString,
	        minConn,
	       maxConn,
	       incrConn);

		
		  //从连接池获取连接
		Connection *conn_pl=connPool->createConnection(userName,password);
		 
										 
		Statement *stmt_tc100 = conn_pl->createStatement();
		Statement *stmt_itc100 = conn_pl->createStatement();
			
		stmt_tc100->setAutoCommit(false);
		stmt_itc100->setAutoCommit(false);


		string sql_pl_tc100,sql_pl_itc100;

		sql_pl_tc100 = "update REAL_SIGNAL_STATUS set Xiangweidengtai=:x1,detect_status=:x2,detect_data=:x3,detect_urgent_status=:x4,detect_pri_status=:x5,door_status=:x6,BoardOnline=:x7 ,update_time=sysdate ,signal_time = to_date(:x8,'yyyy-mm-dd hh24:mi:ss') where signal_id = :x9 ";
		stmt_tc100->setSQL(sql_pl_tc100);
	       stmt_tc100->setMaxIterations(1000);
		   
		stmt_tc100->setMaxParamSize(1,100);
		stmt_tc100->setMaxParamSize(2,210);
		stmt_tc100->setMaxParamSize(3,210);
		stmt_tc100->setMaxParamSize(4,10);
		stmt_tc100->setMaxParamSize(5,10);
		stmt_tc100->setMaxParamSize(6,8);
		stmt_tc100->setMaxParamSize(7,54);
		stmt_tc100->setMaxParamSize(8,21);
		stmt_tc100->setMaxParamSize(9,sizeof(int));
		
		
		sql_pl_itc100 = "update REAL_SIGNAL_STATUS set PHASE_LIGHT_STATE=:x1,PHASE_CURRENT=:x2,DETECT_STATUS=:x3,DETECT_DATA=:x4,DETECT_URGENT_STATUS=:x5,DETECT_PRI_STATUS=:x6,DOOR_STATUS=:x7 ,BOARDONLINE=:x8,update_time=sysdate ,signal_time = to_date(:x9,'yyyy-mm-dd hh24:mi:ss') where signal_id = :10 ";
		stmt_itc100->setSQL(sql_pl_itc100);
	       stmt_itc100->setMaxIterations(1000);
		   
		stmt_itc100->setMaxParamSize(1,100);
		stmt_itc100->setMaxParamSize(2,400);
		stmt_itc100->setMaxParamSize(3,210);
		stmt_itc100->setMaxParamSize(4,210);
		stmt_itc100->setMaxParamSize(5,10);
		stmt_itc100->setMaxParamSize(6,10);
		stmt_itc100->setMaxParamSize(7,8);
		stmt_itc100->setMaxParamSize(8,54);
		stmt_itc100->setMaxParamSize(9,21);
		stmt_itc100->setMaxParamSize(10,sizeof(int));
		
		while(1)
		{
			try
			{
				int signal_id,tc100flag=0,itc100flag = 0;
				static int signal_id_online;
				for(i = 1 ; i < signal_number;i++)
				{
					if(i <signal_number )
					{
						signal_id = i;
					}
					
#if 1		
					if(signal_info_data[signal_id].oracleFlag[0]== 0 )
					{
						//printf("wei  NULL !!!!!!!!!!!!!!!!!!!!!!!!!!");
						if(i!= signal_number-1) //去掉其余的断线信号机
							continue;
						else //将最后一个断线信号机替换为在线的
							signal_id = signal_id_online;
					}
					else
						signal_info_data[signal_id].oracleFlag[0] = 0;
#endif
			//		if(signal_info_data[signal_id].l_online_alarm == 0)
				if(strcmp(signal_info_data[signal_id].device_type,"TC100") == 0)
				
						{
							tc100flag= 1;
							signal_id_online = signal_id;
							 stmt_tc100->setString(1,signal_info_data[signal_id].phase_lht);
							 stmt_tc100->setString(2,signal_info_data[signal_id].detect_status);
							 stmt_tc100->setString(3,signal_info_data[signal_id].detect_data_status);
							 stmt_tc100->setString(4,signal_info_data[signal_id].detect_urgent_status);
							 stmt_tc100->setString(5,signal_info_data[signal_id].detect_pri_status);
							 stmt_tc100->setString(6,signal_info_data[signal_id].door_status);
							 stmt_tc100->setString(7,signal_info_data[signal_id].board_state);
							 stmt_tc100->setString(8,signal_info_data[signal_id].update_time);
							 stmt_tc100->setNumber(9,signal_info_data[signal_id].signal_id);
							//printf("update TC100 real status,id=%d\n",signal_info_data[signal_id].signal_id);
						//	printf("signal_time :: %s\n",signal_info_data[signal_id].update_time);
							stmt_tc100->executeUpdate();
							//if (i !=signal_number-1)
								//stmt_tc100->addIteration();	
							
						}
				else if(strcmp(signal_info_data[signal_id].device_type,"ITC100") == 0)
					{
						itc100flag= 1;
						signal_id_online = signal_id;
							stmt_itc100->setString(1,signal_info_data[signal_id].phase_lht);
							 stmt_itc100->setString(2,signal_info_data[signal_id].phase_current);
							 stmt_itc100->setString(3,signal_info_data[signal_id].detect_status);
							 stmt_itc100->setString(4,signal_info_data[signal_id].detect_data_status);
							 stmt_itc100->setString(5,signal_info_data[signal_id].detect_urgent_status);
							 stmt_itc100->setString(6,signal_info_data[signal_id].detect_pri_status);
							 stmt_itc100->setString(7,signal_info_data[signal_id].door_status);
							 stmt_itc100->setString(8,signal_info_data[signal_id].board_state);
							 stmt_itc100->setString(9,signal_info_data[signal_id].update_time);
							 stmt_itc100->setInt(10,signal_info_data[signal_id].signal_id);
							//printf("update ITC100 real status,id=%d\n",signal_info_data[signal_id].signal_id);

						if (i !=signal_number-1)
								stmt_itc100->addIteration();	
						
					}
				}
				//if(tc100flag == 1)
					//stmt_tc100->executeUpdate();			
				if(itc100flag == 1)
					stmt_itc100->executeUpdate();
					


				conn_pl->commit();

/**********************************
待测试
test :: TC100  还是用批量更新操作        调用两次conn_pl->commit()      

				if(tc100flag == 1)
				{
					stmt_tc100->executeUpdate();	
					conn_pl->commit();
				}		
				if(itc100flag == 1)
				{
					stmt_itc100->executeUpdate()
					conn_pl->commit();
				}	




******************************/

				signal_id_online = 0;
			}
			catch(SQLException& ex)
			{
				cout << "oracle  failed: " << ex.getMessage() << endl; 

				   //断开连接池连接
			        conn_pl->terminateStatement(stmt_tc100);
				   conn_pl->terminateStatement(stmt_itc100);
			        connPool->terminateConnection(conn_pl);

				
				 env_pl->terminateConnectionPool(connPool);
				  //env->terminateConnection(conn);
				oracle::occi::Environment::terminateEnvironment(env_pl);
				break;
			}
			
			sleep(1);
				
		}
		
		
	}


	
}


void *signal_report_pl_2(void *arg)
{

	while(1)
	{
		
		int  i;
		char sql[10000];
		
		//printf("zhi xing yi ci \n");



		string userName = oracle_username;  
	    	string password = oracle_password;  
	    	string connectString = oracle_connectstring; 

			
		unsigned int maxConn=5;
		unsigned int minConn=1;
		unsigned int incrConn=2;

		 oracle::occi::Environment *env_pl = oracle::occi::Environment::createEnvironment(Environment::OBJECT);
		   ConnectionPool *connPool=env_pl->createConnectionPool(
	         userName,
	            password,
	        connectString,
	        minConn,
	       maxConn,
	       incrConn);

		printf("new pthread ,file=%s,func=%s,line=%d\n",__FILE__,__FUNCTION__,__LINE__);
		  //从连接池获取连接
		Connection *conn_pl=connPool->createConnection(userName,password);
		 
										 
		Statement *stmt_tc100 = conn_pl->createStatement();
		Statement *stmt_itc100 = conn_pl->createStatement();
			
		stmt_tc100->setAutoCommit(false);
		stmt_itc100->setAutoCommit(false);

		string sql_pl_tc100,sql_pl_itc100;

		sql_pl_tc100 ="update REAL_SIGNAL_PLAN_STATUS set cur_plan=:x1,control_model=:x2,cur_week=:x3,cur_stage=:x4,xietiaoxuhao=:x5,xietiaoshijian=:x6,fangxingshijian=:x7 ,shengyushijian=:x8 ,update_time=sysdate  where signal_id = :x9 ";
		stmt_tc100->setSQL(sql_pl_tc100);
	       stmt_tc100->setMaxIterations(1000);

		stmt_tc100->setMaxParamSize(1,sizeof(int));
		stmt_tc100->setMaxParamSize(2,sizeof(int));
		stmt_tc100->setMaxParamSize(3,sizeof(int));
		stmt_tc100->setMaxParamSize(4,sizeof(int));
		stmt_tc100->setMaxParamSize(5,sizeof(int));
		stmt_tc100->setMaxParamSize(6,sizeof(int));
		stmt_tc100->setMaxParamSize(7,sizeof(int));
		stmt_tc100->setMaxParamSize(8,sizeof(int));
		stmt_tc100->setMaxParamSize(9,sizeof(int));
		
		sql_pl_itc100 ="update REAL_SIGNAL_PLAN_STATUS set cur_plan=:1,control_model=:2,cur_week=:3,cur_stage=:4,adjust_no=:5,adjust_time=:6,go_time=:7 ,remain_time=:8 ,update_time=sysdate  where signal_id = :9  and list_unit_id =1";
		stmt_itc100->setSQL(sql_pl_itc100);
	       stmt_itc100->setMaxIterations(1000);

	       stmt_itc100->setMaxParamSize(1,sizeof(int));
		stmt_itc100->setMaxParamSize(2,sizeof(int));
		stmt_itc100->setMaxParamSize(3,sizeof(int));
		stmt_itc100->setMaxParamSize(4,sizeof(int));
		stmt_itc100->setMaxParamSize(5,sizeof(int));
		stmt_itc100->setMaxParamSize(6,sizeof(int));
		stmt_itc100->setMaxParamSize(7,sizeof(int));
		stmt_itc100->setMaxParamSize(8,sizeof(int));
		stmt_itc100->setMaxParamSize(9,sizeof(int));
		
		while(1)
		{
			try
			{
				int signal_id,tc100flag = 0,itc100flag = 0;
				static int signal_id_online;
				for(i = 1 ; i < signal_number;i++)
				{
					if(i <signal_number )
					{
						signal_id = i;
					}
					
			#if 1		
					if(signal_info_data[signal_id].oracleFlag[1]== 0)
					{
						//printf("wei  NULL !!!!!!!!!!!!!!!!!!!!!!!!!!");
						if(i!=signal_number-1)
							continue;
						else
							signal_id = signal_id_online;
					}
					else
						signal_info_data[signal_id].oracleFlag[1] = 0;
			#endif

		//			if(signal_info_data[signal_id].l_online_alarm == 0)
					if(strcmp(signal_info_data[signal_id].device_type,"TC100") == 0)
					{
						tc100flag = 1;
						signal_id_online = signal_id;
						 stmt_tc100->setNumber(1,signal_info_data[signal_id].cur_plan);
						 stmt_tc100->setNumber(2,signal_info_data[signal_id].control_model);
						 stmt_tc100->setNumber(3,signal_info_data[signal_id].cur_week);
						 stmt_tc100->setNumber(4,signal_info_data[signal_id].cur_stage);
						 stmt_tc100->setNumber(5,signal_info_data[signal_id].coordination_number);
						 stmt_tc100->setNumber(6,signal_info_data[signal_id].coordination_time);
						 stmt_tc100->setNumber(7,signal_info_data[signal_id].release_time);
						 stmt_tc100->setNumber(8,signal_info_data[signal_id].rest_time);
						 stmt_tc100->setNumber(9,signal_info_data[signal_id].signal_id);	 
						//printf("ID::%d   release_time ::%d    rest_time%d\n",signal_info_data[signal_id].signal_id,signal_info_data[signal_id].release_time,signal_info_data[signal_id].rest_time);
						stmt_tc100->executeUpdate();

						//if (i !=signal_number-1)
							//stmt_tc100->addIteration();	
					}
					else if(strcmp(signal_info_data[signal_id].device_type,"ITC100") == 0)
					{

						itc100flag = 1;
						signal_id_online = signal_id;
						stmt_itc100->setInt(1,signal_info_data[signal_id].cur_plan);
						 stmt_itc100->setInt(2,signal_info_data[signal_id].control_model);
						 stmt_itc100->setInt(3,signal_info_data[signal_id].cur_week);
						 stmt_itc100->setInt(4,signal_info_data[signal_id].cur_stage);
						 stmt_itc100->setInt(5,signal_info_data[signal_id].coordination_number);
						 stmt_itc100->setInt(6,signal_info_data[signal_id].coordination_time);
						 stmt_itc100->setInt(7,signal_info_data[signal_id].release_time);
						 stmt_itc100->setInt(8,signal_info_data[signal_id].rest_time);
						 stmt_itc100->setInt(9,signal_info_data[signal_id].signal_id);

						if (i !=signal_number-1)
							stmt_itc100->addIteration();	
					}

				}
				
				if(itc100flag == 1)
					stmt_itc100->executeUpdate();
				//if(tc100flag == 1)
					//stmt_tc100->executeUpdate();

				conn_pl->commit();

				signal_id_online = 0;
			}
			catch(SQLException& ex)
			{
				cout << "oracle  failed: " << ex.getMessage() << endl; 

				   //断开连接池连接
			        conn_pl->terminateStatement(stmt_tc100);
				   conn_pl->terminateStatement(stmt_itc100);
			        connPool->terminateConnection(conn_pl);

				
				 env_pl->terminateConnectionPool(connPool);
				  //env->terminateConnection(conn);
				oracle::occi::Environment::terminateEnvironment(env_pl);
				break;
			}
			sleep(1);
				
		}
		
	}
	
}




#endif
/*************批量操作更新实时状态测试中******************/




long get_time()
{
	struct timeval tv;
	gettimeofday(&tv,NULL);
	return tv.tv_sec *1000 + tv.tv_usec / 1000;
}


