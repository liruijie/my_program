/*
 * oracle.cpp
 *
 *  Created on: 2016年10月9日
 *      Author: root
 */


#include "include.h"


extern string oracle_username ;
extern string oracle_password ;
extern string oracle_connectstring;
extern int CurrentExistDevice;
oracle::occi::Environment *OraEnviroment;
StatelessConnectionPool *pConnPool;
Connection *conn;
Statement *stmt;
int OCCI_Open()
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
                     conn=pConnPool->getConnection();
                     if(conn)
                     	 	 {
									 stmt = conn->createStatement();
									 stmt->setAutoCommit(true);
									 cout << "Connect oracle"<<endl;
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
/*
 * 从连接池中获取一个连接   成功返回true，失败返回false
 */
int GetConnectFromPool(Connection *new_conn,Statement *new_stmt)
{
	try
	{
		new_conn=pConnPool->getConnection();
		if(new_conn)
		{
			new_stmt = new_conn->createStatement();
			//new_stmt->setAutoCommit(true);
			cout << "Connect oracle"<<endl;
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
	   string strinfo=sqlExcp.getMessage();
	   cout<<strinfo;
	   return false;
	}

}



int InitDeviceInfo()
{
	char sqlbuf[] = "select t.INTERSECTION_ID, INTERSECTION_IP from SYSTEM_CONFIG t order by to_number( t.INTERSECTION_ID)";
	ResultSet *Result;
	int i=0;
	try
	{
		stmt->setSQL(sqlbuf);
		Result = stmt->executeQuery();

		while(Result->next() != 0)                //cun zai
		{
			device[i].id = Result->getInt(1);
			sprintf(device[i].ip,Result->getString(2).c_str());
			device[i].status = 0;
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
	CurrentExistDevice = i;
	for(;i<DeviceMaxNum;i++)                //将剩下的信号机ID置成负数
	{
		device[i].id = -1;
	}
	return true;
}
int InitDeviceStatus()
{
	char sqlbuf[] = "update UNIT_CUR_STATE set CONTROL_MODE = 4";
	try
	{
		stmt->execute(sqlbuf);
		return true;
	}
	catch(SQLException &sqlExcp)
	{
		cout << " Error Number : "<< sqlExcp.getErrorCode() << endl; //获得异常代码
		cout << sqlExcp.getMessage() << endl; //获得异常信息
		return false;
	}
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

