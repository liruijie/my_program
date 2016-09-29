/*
 * oracle.cpp
 *
 *  Created on: 2016年7月4日
 *      Author: Liruijie
 */

#include "include.h"


extern string oracle_username ;
extern string oracle_password ;
extern string oracle_connectstring;

oracle::occi::Environment *OraEnviroment;
StatelessConnectionPool *pConnPool;
Connection *conn;
Statement *stmt;
int occi_open()
{
		while(1)
        {
         try
            {
               const string userName = oracle_username;
               const string password = oracle_password;
               const string connectString= oracle_connectstring;

               // cout <<"name = " << oracle_username     <<"   oracle_password = " <<  oracle_password <<        "   oracle_connectstring = " << oracle_connectstring << endl;
               cout <<"name = " << userName    <<"  oracle_password = " <<  password <<        "    oracle_connectstring = " << connectString << endl;
               unsigned int maxConn=1000;
               unsigned int minConn=5;
               unsigned int incrConn=2;

                string sql;

                OraEnviroment = oracle::occi::Environment::createEnvironment("UTF8","UTF8");
               pConnPool= OraEnviroment->createStatelessConnectionPool(userName,password,connectString,maxConn,minConn,incrConn);

                if(pConnPool)
                  	  {
                     pConnPool->setTimeOut(10);
                     //conn=pConnPool->getConnection(userName,password,connectString);
                     conn=pConnPool->getConnection();
                     if(conn)
                     	 	 {
									 stmt = conn->createStatement();
									 stmt->setAutoCommit(true);
									 return succeed;
                     	 	 }
                  	  }
            }
         catch (SQLException &sqlExcp)
            {
               sqlExcp.getErrorCode();
               string strinfo=sqlExcp.getMessage();
               cout<<strinfo;
               return fail;
            }
			printf("oracle open fail\n");
			sleep(1);
        }

  return 0;
}

int Init_SignalInfo()
{
	char sqlbuf[] = "select t.signal_id from SIGNAL_CONFIG_INFO t order by to_number( t.signal_id)";
	ResultSet *Result;
	int i=0;
	try
	{
		stmt->setSQL(sqlbuf);
		Result = stmt->executeQuery();
		while(Result->next() != 0)                //cun zai
		{
			SignalRealData[i].CrossID = Result->getInt(1);
			SignalRealData[i].SignalState = Offline;
			i++;
		}
		stmt->closeResultSet(Result);
	}
	catch(SQLException &sqlExcp)
	{
		cout << " Error Number : "<< sqlExcp.getErrorCode() << endl; //获得异常代码
		cout << sqlExcp.getMessage() << endl; //获得异常信息
		return fail;
	}
	for(;i<SignalMaxNum;i++)                //将剩下的信号机ID置成负数
	{
		SignalRealData[i].CrossID = -1;
	}
	return succeed;
}

int JudgeIsExist(char *sqlbuf)
{
	ResultSet *Result;
	try
	{
		stmt->setSQL(sqlbuf);
		Result = stmt->executeQuery();
		if(Result->next() != 0)                //cun zai
		{
			stmt->closeResultSet(Result);
			return true;
		}
	}
	catch(SQLException &sqlExcp)
	{
		cout << " Error Number : "<< sqlExcp.getErrorCode() << endl; //获得异常代码
		cout << sqlExcp.getMessage() << endl; //获得异常信息
	}
	return false;
}






