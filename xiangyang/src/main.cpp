//============================================================================
// Name        : xiangyang.cpp
// Author      : LIRUIJIE
// Version     :
// Copyright   : BYZT
// Description : Hello World in C++, Ansi-style
//============================================================================

#include "include.h"

#define ConfigeFile   "/home/config"
string oracle_username;
string oracle_password;
string oracle_connectstring;

string View_username;
string View_password;
string View_connectstring;

ServiceInfo Server;
int GetConfige();
int GetLocalIP();
int main()
{
	int ret;
	cout << "!!!Hello World!!!" << endl; // prints !!!Hello World!!!
	//获取本地IP
	ret = GetLocalIP();
	if(ret == false)
			exit(0);
	//读取配置文件
	ret = GetConfige();
	if(ret == false)
		exit(0);
	//连接数据库
	ret = occi_open();
	if(ret == false)
		exit(0);
	//初始化信号机信息
	Init_SignalInfo();
	//和服务端通信
	CommunicateWithServer();
	//和Web平台通信
	CommunicateWithWeb();
	//获取流量数据

	while(1)
		sleep(60*60);
	return 0;
}
int GetLocalIP()
{

	int fd, num;
	struct ifreq ifq[16];
	struct ifconf ifc;
	int i;
	char *ips, *tmp_ip;

	int val;

	fd = socket(AF_INET, SOCK_DGRAM, 0);
	if(fd < 0)
	{
		fprintf(stderr, "socket failed\n");
		return -1;
	}
	ifc.ifc_len = sizeof(ifq);
	ifc.ifc_buf = (caddr_t)ifq;
	if(ioctl(fd, SIOCGIFCONF, (char *)&ifc))
	{
		fprintf(stderr, "ioctl failed\n");
		return -1;
	}
	num = ifc.ifc_len / sizeof(struct ifreq);
	if(ioctl(fd, SIOCGIFADDR, (char *)&ifq[num-1]))
	{
		fprintf(stderr, "ioctl failed\n");
		return -1;
	}
	close(fd);

	val = 0;
	for(i=0; i<num; i++)
	{
		tmp_ip = inet_ntoa(((struct sockaddr_in*)(&ifq[i].ifr_addr))-> sin_addr);
		if(strcmp(tmp_ip, "127.0.0.1") != 0)
		{
			val++;
		}
	}

	ips = (char *)malloc(val * 16 * sizeof(char));
	if(ips == NULL)
	{
		fprintf(stderr, "malloc failed\n");
		return -1;
	}
	memset(ips, 0, val * 16 * sizeof(char));
	val = 0;
	for(i=0; i<num; i++)
	{
		tmp_ip = inet_ntoa(((struct sockaddr_in*)(&ifq[i].ifr_addr))-> sin_addr);
		if(strcmp(tmp_ip, "127.0.0.1") != 0)
		{
			strcpy(Server.my_ip[val],tmp_ip);
			val ++;
		}
	}


	if(val == 0)
	{
		return false;
	}
	return val;

/*
	struct ifaddrs * ifAddrStruct=NULL;
	void * tmpAddrPtr=NULL;
	getifaddrs(&ifAddrStruct);

	while (ifAddrStruct!=NULL)
	{
		if (ifAddrStruct->ifa_addr->sa_family==AF_INET)
		{   // check it is IP4
			// is a valid IP4 Address
			tmpAddrPtr = &((struct sockaddr_in *)ifAddrStruct->ifa_addr)->sin_addr;
			char addressBuffer[INET_ADDRSTRLEN];
			inet_ntop(AF_INET, tmpAddrPtr, addressBuffer, INET_ADDRSTRLEN);
			printf("%s IPV4 Address %s\n", ifAddrStruct->ifa_name, addressBuffer);
		}
		else if (ifAddrStruct->ifa_addr->sa_family==AF_INET6)
		{   // check it is IP6
			// is a valid IP6 Address
			tmpAddrPtr=&((struct sockaddr_in *)ifAddrStruct->ifa_addr)->sin_addr;
			char addressBuffer[INET6_ADDRSTRLEN];
			inet_ntop(AF_INET6, tmpAddrPtr, addressBuffer, INET6_ADDRSTRLEN);
			printf("%s IPV6 Address %s\n", ifAddrStruct->ifa_name, addressBuffer);
		}
		ifAddrStruct = ifAddrStruct->ifa_next;
	}
	return 0;
*/
}


int GetConfige()
{
	char buf[80];
	getcwd(buf,sizeof(buf));
	printf("current working directory: %s\n", buf);

	FILE *fp = fopen(ConfigeFile,"r");
	if( fp == NULL )
	{
	printf("!!! Can't open file %s\n",ConfigeFile);
	return false;
	}
	char line[64];
	char tmp[64];

	char config[50];
	//数据库配置
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

	//对方服务的IP和端口
	memset(config,'\0',sizeof(config));
	fgets(line,64,fp);
	sscanf(line,"%[^=]=%s",tmp,config);
	memcpy(Server.ip,config,strlen(config));
	printf("temp = %s,config = %s\n",tmp,Server.ip);

	memset(config,'\0',sizeof(config));
	fgets(line,64,fp);
	sscanf(line,"%[^=]=%s",tmp,config);
	Server.port = atoi(config);
	printf("temp = %s,config = %s\n",tmp,config);

	//对方视图的配置
	memset(config,'\0',sizeof(config));
	fgets(line,64,fp);
	sscanf(line,"%[^=]=%s",tmp,config);
	printf("temp = %s,config = %s\n",tmp,config);
	View_username = config;

	memset(config,'\0',sizeof(config));
	fgets(line,64,fp);
	sscanf(line,"%[^=]=%s",tmp,config);
	printf("temp = %s,config = %s\n",tmp,config);
	View_password = config;

	memset(config,'\0',sizeof(config));
	fgets(line,64,fp);
	sscanf(line,"%[^=]=%s",tmp,config);
	printf("temp = %s,config = %s\n",tmp,config);
	View_connectstring = config;

	fclose(fp);
	return true;
}