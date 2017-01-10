/*
 * GetFlowData.cpp
 *
 * Created on: 2016年12月27日
 * Author: LIRUIJIE
 */

#include "include.h"

oracle::occi::Environment *Flow_Env;
StatelessConnectionPool *Flow_ConnectPool;
Connection *Flow_conn;
Statement *Flow_stmt;
extern string View_username;
extern string View_password;
extern string View_connectstring;
extern SignalInfo	Signal[MaxSignalNum];
extern int ExistSignalNum;
void GetFlowDataFromServer()
{
	pthread_t p_FlowData;
	int ret;
	ret = ConnectView();
	if(ret == false)
	{
		exit(0);
	}
	ret = pthread_create(&p_FlowData,NULL,GetFlowData,NULL);
	if(ret != 0)
	{
		exit(0);
	}

}

int ConnectView()
{
	while(1)
	{
		try
		{
		   const string userName = View_username;
		   const string password = View_password;
		   const string connectString= View_connectstring;

		   // cout <<"name = " << oracle_username     <<"   oracle_password = " <<  oracle_password <<        "   oracle_connectstring = " << oracle_connectstring << endl;
		   cout <<"view name = " << userName <<"view password = " << password << "view connectstring = " << connectString << endl;
		   unsigned int maxConn=1000;
		   unsigned int minConn=5;
		   unsigned int incrConn=2;

			string sql;

			Flow_Env = oracle::occi::Environment::createEnvironment("UTF8","UTF8");
			Flow_ConnectPool= Flow_Env->createStatelessConnectionPool(userName,password,connectString,maxConn,minConn,incrConn);

			if(Flow_ConnectPool)
			{
				Flow_ConnectPool->setTimeOut(10);
				Flow_conn=Flow_ConnectPool->getConnection();
				if(Flow_conn)
				{
					Flow_stmt = Flow_conn->createStatement();
					Flow_stmt->setAutoCommit(true);
					return true;
				}
			}
		}
		catch (SQLException &sqlExcp)
		{
			sqlExcp.getErrorCode();
			cout<<sqlExcp.getMessage()<< endl;
			return false;
		}
		printf("oracle open fail\n");
		sleep(1);
	}
}


void * GetFlowData(void *arg)
{
	char sqlbuf[100];
	//char insert_buf[200];
	struct FlowInfo FlowData[5000];
	pthread_detach(pthread_self());
	prctl(PR_SET_NAME, (unsigned long)"GetFlowData");
	Connection *insert_conn;
	Statement *insert_stmt;
	ResultSet *Result;
	int i,j;
	int result_num;//用来判断查询时返回的结果集是不是空，不是空的用完要释放
	char IP[30];
	//int  Direction;
	//int Flow;
	int ret = GetConnectFromPool(&insert_conn,&insert_stmt);
	if(ret !=  true)
	{
		exit(0);
	}
	insert_stmt->setAutoCommit(true);
	sprintf(sqlbuf,"select UtcIP,DirectionNo,Flow from V_TrafficFlow");
	string insert_sql = "insert into DETECTOR_REAL_DATA (unit_id,DETECTOR_ID,FLOW,Update_Time) values(:x1,nvl((select DETECTOR_ID from ITC_CFG_DETECTOR_DETAIL a where a.signal_id = :x2 and a.direction = :x3 and rownum=1),:x4),:x5,sysdate)";

	while(1)
	{
		try
		{
			j=0;					//有效流量数据个数
			result_num = 0;	//结果集中数据个数
			Result = Flow_stmt->executeQuery(sqlbuf);
			while(Result->next() != 0)
			{
				result_num++;
				memset(IP,'\0',30);
				sprintf(IP,Result->getString(1).c_str());

				for(i=0;i<ExistSignalNum;i++)
				{
					if(memcmp(Signal[i].ip,IP,strlen(Signal[i].ip)) == 0)   //
					{
						FlowData[j].signal_id = Signal[i].cross_id;
						FlowData[j].dirction = Result->getInt(2)+1;
						FlowData[j].flow = Result->getInt(3);
						j++;
//						memset(insert_buf,'\0',200);
//						sprintf(insert_buf,"insert into DETECTOR_REAL_DATA (unit_id,DETECTOR_ID,FLOW,Update_Time) values(%d,nvl((select DETECTOR_ID from ITC_CFG_DETECTOR_DETAIL a where a.signal_id = %d and a.direction = %d and rownum=1),%d),%d,sysdate)",
//								Signal[i].cross_id,Signal[i].cross_id,Direction,Direction,Flow);
//						ExcuteSql(insert_buf);
//						break;
					}
				}
			}
			Flow_stmt->closeResultSet(Result);

			if(j != 0)			//如果取得正确数据
			{
				insert_stmt->setSQL(insert_sql);
				insert_stmt->setMaxIterations(5000);
				insert_stmt->setMaxParamSize(1,sizeof(int));
				insert_stmt->setMaxParamSize(2,sizeof(int));
				insert_stmt->setMaxParamSize(3,sizeof(int));
				insert_stmt->setMaxParamSize(4,sizeof(int));
				insert_stmt->setMaxParamSize(5,sizeof(int));
				for(i = 0;i<j;i++)
				{
					if(i!=0)
						insert_stmt->addIteration();
					insert_stmt->setInt(1,FlowData[i].signal_id);
					insert_stmt->setInt(2,FlowData[i].signal_id);
					insert_stmt->setInt(3,FlowData[i].dirction);
					insert_stmt->setInt(4,FlowData[i].dirction);
					insert_stmt->setInt(5,FlowData[i].flow);
				}
				insert_stmt->executeUpdate();
			}
		}
		catch(SQLException &ex)
		{
			cout << " Error Number : "<< ex.getErrorCode() << endl; //获得异常代码
			cout << ex.getMessage() << endl; //获得异常信息
		}
		sleep(60*5);
	}
	return (void *)0;
}


