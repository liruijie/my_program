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
	//ret = CommunicateWithHiCON();
//	if(ret== false)
//		exit(0);
	while(1)
		sleep(60*60);
	return 0;
}

int GetConfige()
{
	FILE *fp = fopen(Config_PATH,"r");
	if( fp == NULL )
	{
	printf("!!! Can't open file %s\n",Config_PATH);
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

	//  memset(config,'\0',sizeof(config));
	//  fgets(line,64,fp);
	//  sscanf(line,"%[^=]=%s",tmp,config);
	//  memcpy(ITC_ServerAddr,config,strlen(config));
	//  printf("temp = %s,config = %s\n",tmp,config);
	//  printf("signal report timeout = %d\n",signal_report_timeout);

	fclose(fp);
	return true;
}

