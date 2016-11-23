//============================================================================
// Name        : PublicTransportPriority.cpp
// Author      : LIRUIJIE
// Version     :
// Copyright   : Your copyright notice
// Description :
//============================================================================

#include "include.h"

string oracle_username ;
string oracle_password ;
string oracle_connectstring;
#define Config_PATH "/home/eclipseprogram/PublicTransportPriority/confige"

int GetConfige();
void CheckTime();
int main()
{
	int ret;

	ret = GetConfige();
	if(ret== false)
		exit(0);
	ret = OCCI_Open();
	if(ret== false)
		exit(0);
	ret = InitDeviceInfo();
	if(ret== false)
		exit(0);
	ret = InitDeviceStatus();
	if(ret== false)
		exit(0);
	ret = CommunicateWithDevice();
	if(ret== false)
		exit(0);
	ret = CommunicateWithWeb();
	if(ret== false)
		exit(0);
//	ret = CommunicateWithHiCON();
//	if(ret== false)
//		exit(0);
	sleep(5);
	while(1)
	{
//		CheckTime();
		sleep(600);				//
	}

	return 0;
}

int GetConfige()
{
	FILE *fp = fopen(Config_PATH,"r");
	if( fp == NULL )
	{
		printf("!!! Can't open file %s\n",Config_PATH);
		perror("error");
		return false;
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
	printf("temp = %s,config = %s\n",tmp,config);
	strcat(HiCON.ip,config);

	memset(config,'\0',sizeof(config));
	fgets(line,64,fp);
	sscanf(line,"%[^=]=%s",tmp,config);
	printf("temp = %s,config = %s\n",tmp,config);
	HiCON.port = atoi(config);

	fclose(fp);
	return true;
}

void CheckTime()
{
	int checktime_socket;
	int i;
	struct sockaddr_in DeviceAddr;
	if ((checktime_socket = socket(PF_INET, SOCK_DGRAM, 0)) < 0)
	{
		perror("fail to socket");
		exit(EXIT_FAILURE);
	}

	int on=1;
	if((setsockopt(checktime_socket,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(on)))<0)
	{
		perror("setsockopt failed");
		close(checktime_socket);
		exit(EXIT_FAILURE);
	}
	printf("开始校准设备时间\n");
	for(i = 0; i< CurrentExistDevice; i++)
	{
		if(device[i].status == OFFLINE)
		{
			continue;
		}
		memset(&DeviceAddr,'\0',sizeof(DeviceAddr));
		DeviceAddr.sin_family = PF_INET;
		DeviceAddr.sin_port = htons(10086);
		DeviceAddr.sin_addr.s_addr = inet_addr(device[i].ip);
		Set_DeviceTime(checktime_socket,device[i].id,DeviceAddr);
	}
	close(checktime_socket);
}
