//============================================================================
// Name        : xinjiang.cpp
// Author      : LIRUIJIE
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include "include.h"

using namespace std;

string oracle_username ;
string oracle_password ;
string oracle_connectstring;
char ITC_ServerAddr[20];
int GetConfige();
int main()
{
	int ret;
	system("systemctl stop firewalld");
	if(fail == GetConfige())
	{
		printf("Get Oracle Confige fail\n");
		return fail;
	}
	if(fail == occi_open())
	{
		printf("Init Oracle CoonPool fail\n");
		return fail;
	}
	if(fail == Init_SignalInfo())
	{
		printf("Init SignalInfo fail\n");
		return fail;
	}
	pthread_t pth_ItcServerCommt;
	ret = pthread_create(&pth_ItcServerCommt,NULL,ItcServerCommt,NULL);
	if(ret != 0)
	{
		perror("Fail to create pthread");
		exit(EXIT_FAILURE);
	}

	pthread_t pth_ControlCenterCommt;
	ret = pthread_create(&pth_ControlCenterCommt,NULL,ControlCenterCommt,NULL);
	if(ret != 0)
	{
		perror("Fail to create pthread");
		exit(EXIT_FAILURE);
	}
	//void *status;
	//pthread_join(pth_ControlCenterCommt,&status);

	pthread_t pth_ReportRealStatus;
	ret = pthread_create(&pth_ReportRealStatus,NULL,ReportRealStatus,NULL);
	if(ret != 0)
	{
		perror("Fail to create pthread");
		exit(EXIT_FAILURE);
	}
	pthread_t pth_KeepConnectAlive;
	ret = pthread_create(&pth_KeepConnectAlive,NULL,KeepConnectAlive,NULL);
	if(ret != 0)
	{
		perror("Fail to create pthread");
		exit(EXIT_FAILURE);
	}
	pthread_t pth_ControlSignal;
	ret = pthread_create(&pth_ControlSignal,NULL,ControlSignal,NULL);
	if(ret != 0)
	{
		perror("Fail to create pthread");
		exit(EXIT_FAILURE);
	}
	int kill_rc;
	while(1)
	{
		kill_rc = pthread_kill(pth_ItcServerCommt,0);
		if(kill_rc == ESRCH)
		{
			printf("pth_ItcServerCommt 线程不存在或者已经退出\n");
			exit(EXIT_FAILURE);
		}
		else if(kill_rc == EINVAL)
		{
			printf("pth_ItcServerCommt 发送信号非法\n");
		}
		else
		{
			printf("pth_ItcServerCommt 线程目前仍然存活\n");
		}

		kill_rc = pthread_kill(pth_ControlCenterCommt,0);
		if(kill_rc == ESRCH)
		{
			printf("pth_ControlCenterCommt 线程不存在或者已经退出\n");
			exit(EXIT_FAILURE);
		}
		else if(kill_rc == EINVAL)
		{
			printf("pth_ControlCenterCommt 发送信号非法\n");
		}
		else
		{
			printf("pth_ControlCenterCommt 线程目前仍然存活\n");
		}

		kill_rc = pthread_kill(pth_ReportRealStatus,0);
		if(kill_rc == ESRCH)
		{
			printf("pth_ReportRealStatus 线程不存在或者已经退出\n");
			exit(EXIT_FAILURE);
		}
		else if(kill_rc == EINVAL)
		{
			printf("pth_ReportRealStatus 发送信号非法\n");
		}
		else
		{
			printf("pth_ReportRealStatus 线程目前仍然存活\n");
		}

		kill_rc = pthread_kill(pth_KeepConnectAlive,0);
		if(kill_rc == ESRCH)
		{
			printf("pth_KeepConnectAlive 线程不存在或者已经退出\n");
			exit(EXIT_FAILURE);
		}
		else if(kill_rc == EINVAL)
		{
			printf("pth_KeepConnectAlive 发送信号非法\n");
		}
		else
		{
			printf("pth_KeepConnectAlive 线程目前仍然存活\n");
		}

		kill_rc = pthread_kill(pth_ControlSignal,0);
		if(kill_rc == ESRCH)
		{
			printf("pth_ControlSignal 线程不存在或者已经退出\n");
			exit(EXIT_FAILURE);
		}
		else if(kill_rc == EINVAL)
			printf("pth_ControlSignal 发送信号非法\n");
		else
			printf("pth_ControlSignal 线程目前仍然存活\n");
		printf("\n");
		sleep(60);
	}
	return 0;

}
int GetConfige()
{
  char buf[80];
  getcwd(buf,sizeof(buf));
  printf("current working directory: %s\n", buf);


  FILE *fp = fopen(OracleConfigeFile,"r");
  if( fp == NULL )
  {
          printf("!!! Can't open file %s\n",OracleConfigeFile);
          return fail;
  }
  char line[64];
  char tmp[64];

  char config[50];

  memset(config,'\0',sizeof(config));
  fgets(line,64,fp);
  sscanf(line,"%[^=]=%s",tmp,config);
  printf("temp = %s,config = %s\n",tmp,config);
  oracle_username = config;

  memset(config,'\0',sizeof(config));
  fgets(line,64,fp);
  sscanf(line,"%[^=]=%s",tmp,config);
  printf("temp = %s,config = %s\n",tmp,config);
  oracle_password = config;

  memset(config,'\0',sizeof(config));
  fgets(line,64,fp);
  sscanf(line,"%[^=]=%s",tmp,config);
  printf("temp = %s,config = %s\n",tmp,config);
  oracle_connectstring = config;

  memset(config,'\0',sizeof(config));
  fgets(line,64,fp);
  sscanf(line,"%[^=]=%s",tmp,config);
  memcpy(ITC_ServerAddr,config,strlen(config));
  printf("temp = %s,config = %s\n",tmp,config);
 // printf("signal report timeout = %d\n",signal_report_timeout);

  fclose(fp);
  return succeed;
}

