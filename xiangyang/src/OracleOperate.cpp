/*
 * OracleOperate.cpp
 *
 * Created on: 2016年11月30日
 * Author: LIRUIJIE
 */
#include "include.h"
extern string oracle_username ;
extern string oracle_password ;
extern string oracle_connectstring;

oracle::occi::Environment *OraEnviroment;
StatelessConnectionPool *pConnPool;
Connection *conn;
Statement *stmt;

SignalInfo	Signal[MaxSignalNum];
int ExistSignalNum;

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
									 return true;
                     	 	 }
                  	  }
            }
         catch (SQLException &sqlExcp)
            {
               sqlExcp.getErrorCode();
               string strinfo=sqlExcp.getMessage();
               cout<<strinfo;
               return false;
            }
			printf("oracle open fail\n");
			sleep(1);
        }

  return 0;
}

int Init_SignalInfo()
{
	char sqlbuf[] = "select t.signal_id,t.signal_ip,k.network_stuats from SIGNAL_CONFIG_INFO t,REAL_TC_STATUS k where t.signal_type!='TC100' AND T.SIGNAL_TYPE!='ITC100' and t.signal_id=k.signal_id order by to_number( t.signal_id)";
	ResultSet *Result;
	int i=0;
	try
	{
		stmt->setSQL(sqlbuf);
		Result = stmt->executeQuery();
		while(Result->next() != 0)                //cun zai
		{
			Signal[i].cross_id = Result->getInt(1);
			sprintf(Signal[i].ip,"%s",Result->getString(2).c_str());
			Signal[i].status = Result->getInt(3);
			if(Signal[i].status == 1)
			{
				Signal[i].LastReportTime = time(NULL);
			}
			i++;
		}
		stmt->closeResultSet(Result);
	}
	catch(SQLException &sqlExcp)
	{
		cout << " Error Number : "<< sqlExcp.getErrorCode() << endl; //获得异常代码
		cout << sqlExcp.getMessage() << endl; //获得异常信息
		return false;
	}
	if(i==0)
	{
		return false;
	}
	ExistSignalNum = i;
	for(;i<MaxSignalNum;i++)                //将剩下的信号机ID置成负数
	{
		Signal[i].cross_id = -1;
	}
	return true;
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
		stmt->closeResultSet(Result);
	}
	catch(SQLException &sqlExcp)
	{
		cout << " Error Number : "<< sqlExcp.getErrorCode() << endl; //获得异常代码
		cout << sqlExcp.getMessage() << endl; //获得异常信息
	}
	return false;
}
/*
 *
 */
int ExcuteSql(char *sqlbuf)
{
	try
	{
		stmt->execute(sqlbuf);
		return true;
	}
	catch(SQLException &sqlExcp)
	{
		cout << " Error Number : "<< sqlExcp.getErrorCode() << endl; //获得异常代码
		cout << sqlExcp.getMessage() << endl; //获得异常信息
	}
	return false;
}


/*
 * 从连接池中获取一个连接   成功返回true，失败返回false
 */
int GetConnectFromPool(Connection **new_conn,Statement **new_stmt)
{
	try
	{
		*new_conn=pConnPool->getConnection();
		if(*new_conn)
		{
			*new_stmt = (*new_conn)->createStatement();
//			cout << "Connect oracle"<<endl;
			return true;
		}
		else
		{
			return false;
		}
	}
	catch (SQLException &sqlExcp)
	{
	   sqlExcp.getErrorCode();
	   cout<<sqlExcp.getMessage() << endl;
	   return false;
	}
}
void DisconnectOracle(Connection **_conn,Statement **_stmt)
{
	try
	{
		(*_conn)->terminateStatement(*_stmt);
		(*_conn)->flushCache();
		OraEnviroment->terminateConnection(*_conn);
	}
	catch (SQLException &sqlExcp)
	{
	   sqlExcp.getErrorCode();
	   cout << sqlExcp.getMessage() << endl;
	}
}


