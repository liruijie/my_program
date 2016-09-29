/*************************************************** 
 * �ļ�����occi_opration.cpp
 * ��Ȩ��
 * ������oracle ���ݿ�����ļ�
 * �޸��ˣ�����
 * �޸�ʱ�䣺2014-1-6
 * �޸İ汾��V0.1
 * �޸����ݣ�
***************************************************/
#include <iostream>
#include <string.h>
#include <unistd.h>
#include <occi.h>
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include "/home/ITC100/occi_opration.h"
#include "/home/ITC100/public_data.h"


using namespace oracle::occi;

using namespace std;
Statement *stmt ;
 
/*************************************************** 
 * ��������occi_open
 * ���������������ݿ�
 *�����ʵı�
 *���޸ĵı�
* ����˵������
*����ֵ: 0 ʧ�� 1�ɹ�
***************************************************/
int occi_open()
{	
	while(1)
	{
	  	try
	    	{
	       const string userName = oracle_username;
	       const string password = oracle_password;
	       const string connectString= oracle_connectstring;


		
		cout <<"name = " << oracle_username 	<<"   oracle_password = " <<  oracle_password <<	"   oracle_connectstring = " << oracle_connectstring << endl;
		cout <<"name = " << userName 	<<"  oracle_password = " <<  password <<	"    oracle_connectstring = " << connectString << endl;
	       unsigned int maxConn=10;
	       unsigned int minConn=1;
	       unsigned int incrConn=2;
		int i=0;
		string sql;

	       oracle::occi::Environment *env = oracle::occi::Environment::createEnvironment(Environment::OBJECT);

	       //�������ӳ�
	       ConnectionPool *connPool=env->createConnectionPool(
	       userName,
	       password,
	       connectString,
	       minConn,
	       maxConn,
	       incrConn);

	      	//�����ӳػ�ȡ����
	      	Connection *conn=connPool->createConnection(userName,password);
		 
	     	stmt = conn->createStatement();
			
		stmt->setAutoCommit(false);
		break;

	    }

	    catch (SQLException &sqlExcp)
	    {
	       int i=sqlExcp.getErrorCode();
	       string strinfo=sqlExcp.getMessage();
	       cout<<strinfo;
		//return -1;
	    }
		printf("oracle open fail\n");
		sleep(1);
		
	}

 
  return 0;
}



/*************************************************** 
 * ��������occi_singal_Init
 * ������������ʼ���źŻ���Ϣ���浽
 	signal_info signal_info_data[512]��
 *�����ʵı�SIGNAL_CONFIG_INFO
 *���޸ĵı�
* ����˵������
*����ֵ: 0 ʧ�� 1�ɹ�
***************************************************/
int occi_singal_Init()
{
printf("into 92\n");

	time_t now;
	now= time(NULL);
	ResultSet *rs;
#if 1
	/*��ѯ�źŻ���ʼ��Ϣ*/
	//string sql="select signal_id,signal_ip,signal_port  from  SIGNAL_CONFIG_INFO order by signal_id";
	string sql="select signal_id,signal_ip,signal_port,unit_id,signal_type  from  SIGNAL_CONFIG_INFO order by signal_id";
	while(1)
	{
		try
		{
			stmt->setSQL(sql);
			printf("into 140\n");
			rs = stmt->executeQuery();
			printf("into 142\n");
			break;
		}
		catch (SQLException &sqlExcp)
		{
		    //   int mmm=sqlExcp.getErrorCode();
		    //   string strinfo=sqlExcp.getMessage();
		    //   cout<<strinfo;
			//return -1;
			occi_open();
		}
		
		sleep(1);
		
	}
	signal_number = 1;
	
	while(rs->next())  
    	{  
    		signal_info_data[signal_number].signal_id  = (int)rs->getInt(1);
		strcpy( signal_info_data[signal_number].signal_ip, rs->getString(2).c_str());
		signal_info_data[signal_number].signal_port= (int)rs->getInt(3);
		signal_info_data[signal_number].unit_id= (int)rs->getInt(4);
		strcpy( signal_info_data[signal_number].device_type, rs->getString(5).c_str());
		
		//signal_info_data[signal_number].last_time= now;
		signal_info_data[signal_number].signal_state  = 1;
		signal_info_data[signal_number].oracleFlag[0]  = 0;//��������ʱ��д���ݿ�
		signal_info_data[signal_number].oracleFlag[1]  = 0;
		signal_number++;
    		//printf("name = %s  ID = %d\n",rs->getString(1).c_str(),(int)rs->getInt(2));
   		// cout << "user name:  " << rs->getString(1) << "user ID:  " << rs->getInt(2) << endl;			
    	} 
	stmt->closeResultSet(rs); 	
#endif
	//stmt->executeUpdate();

  return 0;
}


/*************************************************** 
 * ��������occi_singal_Init_alalrm
 * ������������ʼ���źŻ���ʱ������Ϣ���
 	signal_info signal_info_data[512]��
 *�����ʵı�TC_ALARMLIST
 *���޸ĵı�
* ����˵������
*����ֵ: 0 ʧ�� 1�ɹ�
***************************************************/
int occi_singal_Init_alalrm()
{

	
	//printf("signal_number = %d\n",signal_number);

	if(signal_number == 1)
	{
		return 0;
	}
	
	time_t now;

	int i ,j;
	char sql[10000];

	Environment *env = Environment::createEnvironment(Environment::OBJECT);  
	string username = oracle_username;  
    	string userpass = oracle_password;  
    	string connstr = oracle_connectstring; 

	ResultSet *rs ;
	Connection *conn_parsing ;
	Statement *st;
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
			printf("creat init alarm  oracle fail!\n");
	    	}
		sleep(1);
	}


	
	now = time(NULL);
	while(1)
	{
		try
		{
			/*��ʼ���Ƿ����߹���*/
			for(i = 1;i <signal_number;i++)
			{
				
				bzero(sql,sizeof(sql));
				sprintf(sql,"select to_char(occur_time,'yyyy-mm-dd hh24:mi:ss')   from  ATC_ALARMLIST where device_id = %d  and alarm_id = 1 order by Sn" ,signal_info_data[i].signal_id);
				//printf("%s\n",sql);
				
				 st->setSQL(sql);
				 rs = st->executeQuery();

			
				if(rs->next() == 0)
				{
					
					//printf("0000000000000000000000%d\n",now);
					signal_info_data[i].l_online_alarm = 0;
					
					signal_info_data[i].last_time= now;
					sprintf(sql,"update REAL_TC_STATUS set network_stuats = 1,Updatetime = sysdate where signal_id = %d ",signal_info_data[i].signal_id);
					//printf("%s\n",sql);
					st->setSQL(sql);
					st->executeUpdate();
				}
				else
				{
					

					//printf("@@@@@@@%s\n",rs->getString(1).c_str());
					signal_info_data[signal_number].last_time= 0;
					signal_info_data[i].l_online_alarm = 1;
					signal_info_data[i].signal_state = 0;
					bzero(signal_info_data[i].l_online_alarm_time,sizeof(signal_info_data[i].l_online_alarm_time));
					sprintf(signal_info_data[i].l_online_alarm_time,"%s",rs->getString(1).c_str());

					sprintf(sql,"update REAL_TC_STATUS set network_stuats = 0,Updatetime = sysdate where signal_id = %d ",signal_info_data[i].signal_id);
					printf("%s\n",sql);
					st->setSQL(sql);
					st->executeUpdate();
					
				}
				//st->closeResultSet(rs);  
				
			}

			
			st->closeResultSet(rs);  

			
			for(i = 1;i <signal_number;i++)
			{
			#if 1
				/*��ʼ���̳�ͻ*/
				
				bzero(sql,sizeof(sql));
				sprintf(sql,"select *  from  ATC_ALARMLIST where device_id = %d  and alarm_id = 128 " ,signal_info_data[i].signal_id);
				 st->setSQL(sql);
				 rs = st->executeQuery();
				 /*��ʱ���ϱ����޹�����Ϣ*/
				if(rs->next() == 0)
				{
					/*��ʱ����û�й�������*/
					signal_info_data[i].green_conflicts_flag = 0;
					
				}
				/*��*/
				else
				{
					/*��ʱ�����й�������*/
					signal_info_data[i].green_conflicts_flag = 1;
				}
				
				
			#endif

				/*��ʼ�����̳�ͻ*/
				bzero(sql,sizeof(sql));
				sprintf(sql,"select  *   from  ATC_ALARMLIST where device_id = %d  and alarm_id = 129   " ,signal_info_data[i].signal_id);
				 st->setSQL(sql);
				 rs = st->executeQuery();
				 /*��ʱ���ϱ����޹�����Ϣ*/
				if(rs->next() == 0)
				{
					/*��ʱ����û�й�������*/
					
					signal_info_data[i].red_green_conflicts_flag = 0;
				}
				/*��*/
				else
				{
					signal_info_data[i].red_green_conflicts_flag = 1;
				}
					
				
				/*��ʼ���ƹ���*/
				bzero(sql,sizeof(sql));
				sprintf(sql,"select  *   from  ATC_ALARMLIST where device_id = %d  and alarm_id = 130   " ,signal_info_data[i].signal_id);
				 st->setSQL(sql);
				 rs = st->executeQuery();
				 /*��ʱ���ϱ����޹�����Ϣ*/
				if(rs->next() == 0)
				{
					/*��ʱ����û�й�������*/
					
					signal_info_data[i].lamp_failures_flag= 0;
				}
				/*��*/
				else
				{
					signal_info_data[i].lamp_failures_flag = 1;
				}

				/*��ʼ���ù���*/
					//���ù��ϲ��ó�ʼ��

				/*��ʼ�����������*/
				for(j = 1;j <=104;j++)
				{
					bzero(sql,sizeof(sql));
					sprintf(sql,"select *  from  ATC_ALARMLIST where device_id = %d  and alarm_id = 133 and  faultcomment ='&#26816;&#27979;&#22120;&#25925;&#38556;&#65306;&#32534;&#21495;%d'  order by Sn" ,signal_info_data[i].signal_id,j);
					 st->setSQL(sql);
					 rs = st->executeQuery();
					 /*��ʱ���ϱ����޹�����Ϣ*/
					if(rs->next() == 0)
					{
						signal_info_data[i].detector_failures[j][0] = 0;
						
					}
					/*��*/
					else
					{
						signal_info_data[i].detector_failures[j][0] = -1;
					}
					
					//printf("%s\n",sql);
				} 

				/*��ʼ������ȱʧ*/
					//����Ҫ��ʼ��


				/*��ʼ����λ��ȱʧ*/
					//����Ҫ��ʼ��

				/*��ʼ����������*/
					//����Ҫ��ʼ��

				/*��ʼ���������ʱ�䲻����*/
				for(j = 1;j <=104;j++)
				{
					bzero(sql,sizeof(sql));
					sprintf(sql,"select  *  from  ATC_ALARMLIST where device_id = %d  and alarm_id = 160 and  faultcomment ='&#26816;&#27979;&#22120;&#36229;&#26102;&#65306;&#32534;&#21495;%d'  order by Sn" ,signal_info_data[i].signal_id,j);
					 st->setSQL(sql);
					 rs = st->executeQuery();
					 /*��ʱ���ϱ����޹�����Ϣ*/
					if(rs->next() == 0)
					{
						signal_info_data[i].detector_timeout[j][0] = 0;
						
					}
					/*��*/
					else
					{
						signal_info_data[i].detector_timeout[j][0] = -1;
					
					}
					
					//printf("%s\n",sql);
				} 

				/*��ʼ��ǰ��*/
				bzero(sql,sizeof(sql));
				sprintf(sql,"select  *   from  ATC_ALARMLIST where device_id = %d  and alarm_id = 162 and  faultcomment ='&#21069;&#38376;&#25171;&#24320;&#25253;&#35686;'  " ,signal_info_data[i].signal_id);
				 st->setSQL(sql);
				 rs = st->executeQuery();
				 /*��ʱ���ϱ����޹�����Ϣ*/
				if(rs->next() == 0)
				{
					signal_info_data[i].door_state[1][0] = 0;
					
				}
				/*��*/
				else
				{
					signal_info_data[i].door_state[1][0] = -1;
				
				}
				
				/*��ʼ������*/
				bzero(sql,sizeof(sql));
				sprintf(sql,"select  *   from  ATC_ALARMLIST where device_id = %d  and alarm_id = 162 and  faultcomment ='&#21518;&#38376;&#25171;&#24320;&#25253;&#35686;'  " ,signal_info_data[i].signal_id);
				 st->setSQL(sql);
				 rs = st->executeQuery();
				 /*��ʱ���ϱ����޹�����Ϣ*/
				if(rs->next() == 0)
				{
					signal_info_data[i].door_state[2][0] = 0;
				
				}
				/*��*/
				else
				{
					signal_info_data[i].door_state[2][0] = -1;
					
				}


				
				
			}
			st->closeResultSet(rs);  
			break;
		}
		 catch (SQLException &sqlExcp)
	    	{
	     		printf("init alram oracle select fail\n");
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
					printf("creat init alarm  oracle fail!\n");
			    	}
				
				sleep(1);
			}
			printf("init alram oracle select OK\n");
		//return -1;
	    	}
		
	}
	
    	env->terminateConnection(conn_parsing);  
    	Environment::terminateEnvironment(env);  

  return 0;
}


/*************************************************** 
 * ��������occi_singal_Init_control_heart
 * ������������ʼ���źŻ�����״̬
 	signal_info signal_info_data[512]��
 *�����ʵı�TC_ALARMLIST
 *���޸ĵı�
* ����˵������
*����ֵ: 0 ʧ�� 1�ɹ�
***************************************************/
int occi_singal_Init_control_heart()
{
	printf("!!!!!!!!!!!!!!!!!\n");
	time_t now;

	int i ,j;
	char sql[10000];

	Environment *env = Environment::createEnvironment(Environment::OBJECT);  
	

	string username = oracle_username;  
	string userpass = oracle_password;  
	string connstr = oracle_connectstring; 
	
	Connection *conn_parsing;
	Statement *st;

#if 0
	while(1)
	{
		try
		{
			break;
		}
		catch (SQLException &sqlExcp)
	    	{
			 env->terminateConnection(conn_parsing);  
			Environment::terminateEnvironment(env);  
			printf("creat init occi_singal_Init_control_heart  oracle fail!\n");
	    	}
	}
#endif

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
	
	for(i = 1;i <signal_number;i++)
	{
		signal_info_data[i].signal_control_flag = 0;
		signal_info_data[i].control_heart_flag= 0;
		signal_info_data[i].control_d1_flag = 0;
	}

	
	while(1)
	{
		try
		{
			sprintf(sql,"update CONTROL_CMD set control_flag = 0  ");
			st->setSQL(sql);
			st->executeUpdate();
			break;
		}
		catch (SQLException &sqlExcp)
	    	{
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
	}
	
    	env->terminateConnection(conn_parsing);  
    	Environment::terminateEnvironment(env);  
	
}


