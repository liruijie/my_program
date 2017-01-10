//============================================================================
// Name        : OracleChinese.cpp
// Author      : LIRUIJIE
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <iostream>
#include <occi.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <wait.h>
#include <time.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <arpa/inet.h>
using namespace oracle::occi;
using namespace std;
StatelessConnectionPool *pConnPool;
Connection *conn;
Statement *stmt;
char zhongwen[] = "北京博研智通科技有限公司\n";
int occi_open();
int selectoperate();
int insertoperate();
enum Error
{
	SDE_Version,
	SDE_Token,
	SDE_Address
}ErrorType;

int main() {
	cout << "!!!Hello World!!!" << endl; // prints !!!Hello World!!!
	printf(zhongwen);

	printf("%d",SDE_Address);
    char s[] = "123,1,111111,5";

    char *p;
    p = strtok(s, (char *)&",");
    printf("%s ",p);
    while((p = strtok(NULL, (char *)&",")))
        printf("%s ", p);
    printf("\n");
    cout << __FILE__<< '\t'<< __FUNCTION__<< '\t'<< __LINE__<< '\t'<< " Error Number : "<<endl; //获得异常代码
	occi_open();
	selectoperate();
	//insertoperate();
	//getchar();
	return 0;
}

int occi_open()
{
        while(1)
        {
         try
            {
               const string userName = "atc";
               const string password = "atc";
               const string connectString= "192.168.1.81:1521/oracle";

                cout <<"name = " << userName    <<"  oracle_password = " <<  password <<  " oracle_connectstring = " << connectString << endl;
               unsigned int maxConn=20;
               unsigned int minConn=5;
               unsigned int incrConn=2;

                string sql;

               oracle::occi::Environment *env = oracle::occi::Environment::createEnvironment("UTF8","UTF8");
               pConnPool= env->createStatelessConnectionPool(userName,password,connectString,minConn,maxConn,incrConn);

                if(pConnPool)
                  	  {
                     pConnPool->setTimeOut(10000);
                     conn=pConnPool->getConnection();
                     if(conn)
                     	 	 {
									 stmt = conn->createStatement();
									 stmt->setAutoCommit(true);
									 return 0;
                     	 	 }
                  	  }
            }
         catch (SQLException &sqlExcp)
            {
               sqlExcp.getErrorCode();
               string strinfo=sqlExcp.getMessage();
               cout<<strinfo;
               return 1;
            }
			printf("oracle open fail\n");
			sleep(1);
        }

  return 0;
}

int selectoperate()
{
	ResultSet *Result;
	char sqlbuf[] = "select t.disrict_id , t.disrict_name from DISRICT_INFOR t where t.disrict_name='邯郸' order by t.disrict_id";
	//int disrict_id;
	//char disrict_name[100];
	//char sqlbuf[] = "select t.detail from DISRICT_INFOR t  order by t.disrict_id";
	try
	{
		stmt->setSQL(sqlbuf);
		Result = stmt->executeQuery();

		while(Result->next() != 0)                //cun zai
		{

			//disrict_id = Result->getInt(1);
			//strcpy(disrict_name , Result->getString(1).c_str());

			printf("disrict_id = %d  ,disrict_name = %s\n",Result->getInt(1),Result->getString(2).c_str());
			//cout << Result->getString(1).c_str() << Result->getString(2)<< endl;
			//printf("disrict_id = %s  \n",disrict_name);
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
int insertoperate()
{
#if 0
	char sqlbuf[] = "insert into disrict_infor (disrict_id,disrict_name) values(10,'昌平')";
	printf(sqlbuf);
	int nRet = 0;
	try
	{
		stmt->setSQL(sqlbuf);
		nRet = stmt->executeUpdate();
		if(nRet == 0)
		{
			printf("executeUpdate insert error.\n");
		}
		else
		{
			printf("executeUpdate insert succeed ret = %d.\n",nRet);
		}
	}
	catch(SQLException &sqlExcp)
	{
		cout << " Error Number : "<< sqlExcp.getErrorCode() << endl; //获得异常代码
		cout << sqlExcp.getMessage() << endl; //获得异常信息
	}
	if(nRet == 0) {
	            printf("executeUpdate insert error.\n");
	        }
	return 0;
#endif

	char sqlbuf[] = "insert into ITC_CFG_STAGE t(t.signal_id,t.stage_id,t.stage_phase,t.shield_occ,t.relieve_occ,t.stat_time,t.auto_occ_control,t.stage_step,t.auto_control_time) values(:x1,:x2,:x3,:x4,:x5,:x6,:x7,:x8,:x9)";
	try
	{
			stmt->setSQL(sqlbuf);
			stmt->setMaxIterations(1000);
			stmt->setMaxParamSize(1,sizeof(int));
			stmt->setMaxParamSize(2,sizeof(int));
			stmt->setMaxParamSize(3,300);
			stmt->setMaxParamSize(4,sizeof(int));
			stmt->setMaxParamSize(5,sizeof(int));
			stmt->setMaxParamSize(6,sizeof(int));
			stmt->setMaxParamSize(7,sizeof(int));
			stmt->setMaxParamSize(8,sizeof(int));
			stmt->setMaxParamSize(9,sizeof(int));
			char buf1[]="2,2,2,2,2,2,2,2,2,2,2,2,2,2,";
			char buf2[]="3,3,3,3,3,3,3,3,3,3,3,3,3,3,3";
			char buf3[]="4444";
			stmt->setNumber(1,333);
			stmt->setNumber(2,333);
			stmt->setString(3,buf1);
			stmt->setNumber(4,333);
			stmt->setNumber(5,333);
			stmt->setNumber(6,333);
			stmt->setNumber(7,333);
			stmt->setNumber(8,333);
			stmt->setNumber(9,333);
			stmt->addIteration();
			stmt->setNumber(1,444);
			stmt->setNumber(2,444);
			stmt->setString(3,buf2);
			stmt->setString(3,buf3);
			stmt->setNumber(4,444);
			stmt->setNumber(5,444);
			stmt->setNumber(6,444);
			stmt->setNumber(7,444);
			stmt->setNumber(8,444);
			stmt->setNumber(9,444);
			stmt->executeUpdate();
	}
	catch(SQLException &sqlExcp)
	{
		cout << " Error Number : "<< sqlExcp.getErrorCode() << endl; //获得异常代码
		cout << sqlExcp.getMessage() << endl; //获得异常信息
	}
	return 0;


}
