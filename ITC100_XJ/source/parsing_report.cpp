/*************************************************** 
 * �ļ�����parsing_report.cpp
 * ��Ȩ��
 * ����������ʵʱ�ϱ�����״̬����
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
#include "signal_report.h"
#include "occi_opration.h"
#include "public_data.h"
#include "parsing_report.h"

using namespace std;
using namespace oracle::occi;

#define BUFF_NUM  512

//void *signal_report_cl(void *arg);
long get_time(void);


/* ���������õ��߳�*/
void *signal_report_pl_1(void *arg);
void *signal_report_pl_2(void *arg);




/*************************************************** 
 * ��������parsing_signal_report
 * �����������������źŻ���ʵʱ���ݸ��µ�����
 *�����ʵı�
 *���޸ĵı�REAL_SIGNAL_STATUS  REAL_SIGNAL_PLAN_STATUS 
* ����˵������
*����ֵ: ��
***************************************************/

/*************���̴߳���Ŀǰ������******************/



/*************������������ʵʱ״̬������******************/
#if 2
/*��������*/
void parsing_signal_report()
{

	int res;
	pthread_t pth_signal_report[2];

	printf("new pthread ,file=%s,func=%s,line=%d\n",__FILE__,__FUNCTION__,__LINE__);
	sleep(3);
	/*����REAL_SIGNAL_STATUS  REAL_SIGNAL_PLAN_STATUS*/
	res = pthread_create(&(pth_signal_report[0]),NULL,signal_report_pl_1,NULL);
	if(res != 0)
	{
		printf("!!!!!!!!!!!!!!!!!! shi bai\n");
		//perror("Fail to create pthread");
		//exit(EXIT_FAILURE);
	}
	sleep(4);
	/*����REAL_SIGNAL_PLAN_STATUS*/
	res = pthread_create(&(pth_signal_report[1]),NULL,signal_report_pl_2,NULL);
	if(res != 0)
	{
		printf("!!!!!!!!!!!!!!!!!! shi bai\n\n");
		//perror("Fail to create pthread");
		//exit(EXIT_FAILURE);
	}
		



	while(1)
	{
		sleep(24*3600);
	}

}


void *signal_report_pl_1(void *arg)
{

	while(1)
	{
		
		int  i;
		

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

		
		  //�����ӳػ�ȡ����
		Connection *conn_pl=connPool->createConnection(userName,password);
		 
										 
		Statement *stmt_tc100 = conn_pl->createStatement();
		Statement *stmt_itc100 = conn_pl->createStatement();
			
		stmt_tc100->setAutoCommit(true);
		stmt_itc100->setAutoCommit(true);


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
						if(i!= signal_number-1) //ȥ������Ķ����źŻ�
							continue;
						else //�����һ�������źŻ��滻Ϊ���ߵ�
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
							//stmt_tc100->executeUpdate();
							if (i !=signal_number-1)
								stmt_tc100->addIteration();
							
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
				if(tc100flag == 1)
					stmt_tc100->executeUpdate();
				if(itc100flag == 1)
					stmt_itc100->executeUpdate();
					


				//conn_pl->commit();

/**********************************
������
test :: TC100  �������������²���        ��������conn_pl->commit()      

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

				   //�Ͽ����ӳ�����
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

return (void *)0;
	
}


void *signal_report_pl_2(void *arg)
{

	while(1)
	{
		
		int  i;
		

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
		  //�����ӳػ�ȡ����
		Connection *conn_pl=connPool->createConnection(userName,password);
		 
										 
		Statement *stmt_tc100 = conn_pl->createStatement();
		Statement *stmt_itc100 = conn_pl->createStatement();
			
		stmt_tc100->setAutoCommit(true);
		stmt_itc100->setAutoCommit(true);

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
						//stmt_tc100->executeUpdate();

						if (i !=signal_number-1)
							stmt_tc100->addIteration();
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
				if(tc100flag == 1)
					stmt_tc100->executeUpdate();

				//conn_pl->commit();

				signal_id_online = 0;
			}
			catch(SQLException& ex)
			{
				cout << "oracle  failed: " << ex.getMessage() << endl; 

				   //�Ͽ����ӳ�����
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
	return (void *)0;
}




#endif
/*************������������ʵʱ״̬������******************/




long get_time()
{
	struct timeval tv;
	gettimeofday(&tv,NULL);
	return tv.tv_sec *1000 + tv.tv_usec / 1000;
}


