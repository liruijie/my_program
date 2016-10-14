//============================================================================
// Name        : occi_batch_insert.cpp
// Author      : LIRUIJIE
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <iostream>
#include <occi.h>
#include <stdlib.h>
#include <stdio.h>
using namespace std;
using namespace oracle::occi;
oracle::occi::Environment *OraEnviroment;
StatelessConnectionPool *pConnPool;
Connection *conn;
Statement *stmt;
int main()
{
	cout << "!!!Hello World!!!" << endl; // prints !!!Hello World!!!
	char sqlbuf[100];
	try
	{
	   const string userName = "busp";
	   const string password = "busp";
	   const string connectString= "192.168.1.81:1521/oracle";

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
	try
	{
		int i = 10;
		sprintf(sqlbuf,"insert into DEVICE_CONFIG(INTERSECTION_ID,READER_POWER,READER_BLOCK,BUS_PRIORITY_SWITCH_NUMBER,EMERGENCY_SWITCH_NUMBER,ZIGBEE_ADDRESS,READER_ID,DIRECTION_ID) values(:x1,:x2,:x3,:x4,:x5,:x6,:x7,:x8)");
		stmt->setSQL(sqlbuf);
		stmt->setMaxIterations(20);
		stmt->setMaxParamSize(1,10);
		stmt->setMaxParamSize(2,sizeof(int));
		stmt->setMaxParamSize(3,sizeof(int));
		stmt->setMaxParamSize(4,sizeof(int));
		stmt->setMaxParamSize(5,sizeof(int));
		stmt->setMaxParamSize(6,sizeof(int));
		stmt->setMaxParamSize(7,sizeof(int));
		stmt->setMaxParamSize(8,sizeof(int));

		int temp_t;
		char temp_buf[10];
		int count = 0;
		for(temp_t = 0;temp_t < i;temp_t++)
		{
			if(count != 0)
				stmt->addIteration();
			sprintf(temp_buf,"%d",temp_t);
			stmt->setString(1,temp_buf);
			stmt->setInt(2,temp_t);
			stmt->setInt(3,temp_t);
			stmt->setInt(4,temp_t);
			stmt->setInt(5,temp_t);
			stmt->setInt(6,temp_t);
			stmt->setInt(7,temp_t);
			stmt->setInt(8,temp_t);
			count++;
			//if(temp_t!= i-1)
				stmt->addIteration();
		}
		stmt->executeUpdate();
	}
	catch (SQLException &sqlExcp)
	{
	   sqlExcp.getErrorCode();
	   string strinfo=sqlExcp.getMessage();
	   cout<<strinfo;
	   return false;
	}
	getchar();
}
