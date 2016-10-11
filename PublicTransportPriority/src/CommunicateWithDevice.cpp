/*
 * CommunicateWithDevice.cpp
 *
 * Created on: 2016年10月9日
 * Author: liruijie
 */
#include "include.h"


#define DeviceRealDataPort 10085

char Rec_buf[SingleRecvMaxLen];
extern int CurrentExistDevice;
Connection *conn_device;
Statement *stmt_device;

void CommunicateWithDevice()
{
	int ret;

	//init oracle link

	ret = GetConnectFromPool(conn_device,stmt_device);
	if(ret == false)
		return;
	//init queue
	InitQueue();
	//creat receive pthread
	pthread_t pth_receive_data;
	ret = pthread_create(&pth_receive_data,NULL,ReceiveData_Device,NULL);
	if(ret != 0)
	{
		perror("Fail to create device receive data pthread\n");
		exit(EXIT_FAILURE);
	}
	//creat parsing pthread
	pthread_t pth_parsing_data;
	ret = pthread_create(&pth_parsing_data,NULL,ParsingData_Device,NULL);
	if(ret != 0)
	{
		perror("Fail to create device receive data pthread\n");
		exit(EXIT_FAILURE);
	}
	//creat update pthread

}



void * ReceiveData_Device(void *arg)
{
	struct sockaddr_in myaddr;
	int udp_sockfd;
	struct sockaddr_in ServerAddr;
	int sin_size_1=sizeof(struct sockaddr_in);    //sin_size值需要初始化，否则在第一次接受数据时将无法获得对方的地址信息
	//socklen_t peerlen;
	if ((udp_sockfd = socket(PF_INET, SOCK_DGRAM, 0)) < 0)
	{
		perror("fail to socket");
		exit(-1);
	}

	int on=1;
	if((setsockopt(udp_sockfd,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(on)))<0)
	 {
			perror("setsockopt failed");
			exit(EXIT_FAILURE);
	 }

	bzero(&myaddr, sizeof(myaddr));
	myaddr.sin_family = PF_INET;
	myaddr.sin_port = htons(DeviceRealDataPort);
	myaddr.sin_addr.s_addr = INADDR_ANY;
	if ( bind( udp_sockfd, (struct sockaddr *)&myaddr, sizeof(myaddr) ) < 0 )
	{
		perror("fail to bind");
		exit(-1);
	}

	while(1)
	{
		fd_set  Rec_fdSet;
		int ret,numbytes;
		FD_ZERO(&Rec_fdSet);
		FD_SET(udp_sockfd, &Rec_fdSet);
		ret = select((int) udp_sockfd + 1, &Rec_fdSet,NULL, NULL, NULL);
		if( ret == 0 )    //select timeout
		{
			continue;
		}
		if( !ret )    //select error
		{
			printf("UDP Select error result=%d,file=%s,function=%s,line=%d\n",ret,__FILE__,__FUNCTION__,__LINE__);
			return 0;
		}
		else          //select success
		{
			memset(Rec_buf,'\0',SingleRecvMaxLen);
			if ((numbytes=recvfrom(udp_sockfd,  Rec_buf,  SingleRecvMaxLen,  0,  (struct sockaddr *)&ServerAddr, (socklen_t *) &sin_size_1))==-1)
			 {
				perror("recv ");
				return 0;
			 }
			if(numbytes == 0)
			{
				continue;
			}
			else if(numbytes >5)           //头   长度_H   长度_L   校验    尾         至少5个
			{
				Rec_buf[numbytes] = '\0';
				//enqueue
			}
		}
	}
	return (void *)0;
}

void * ParsingData_Device(void *arg)
{
	unsigned char data[SingleRecvMaxLen];
	while(1)
	{
		if(IsQueueEmpty() ==  true)    //队列为空
		{
			usleep(10*1000);
			continue;
		}
		else
		{
			DelQueue(data);
			switch(data[9])
			{
				case 0x30:
				{
					switch(data[10])
					{
						case 0x03:  //设备参数
							break;
						case 0x04:  //系统参数
							break;
						case 0x05:  //优先策略
							break;
						default:
							break;
					}
					break;
				}
				case 0x40:
				{
					switch(data[10])
					{
						case 0x00:  //系统时间
							Device_Time(data);
							break;
						case 0x01:  //车辆检测信息
							Device_DetectData(data);
							break;
						case 0x02:  //故障数据
							Device_Fault(data);
							break;
						default:
							break;
					}
					break;
				}
				default:
					break;
			}
		}
	}
	return (void *)0;
}

void * UpdateStatus_Device(void *arg)
{
	int i;
	char sqlbuf[] = "update table set name1 = :x1 where name3 = x2";
	stmt_device->setMaxIterations(CurrentExistDevice);
	stmt_device->setMaxParamSize(1,30);
	stmt_device->setMaxParamSize(2,sizeof(int));

	while(1)
	{
		for(i = 0; i < CurrentExistDevice;i++)
		{
			if(device[i].status == 0)    //offline
			{
				continue;
			}
			stmt_device->setString(1,device[i].realdata.DeviceTime);
			stmt_device->setInt(2,device[i].id);
		}
		sleep(1);
	}
	return (void *)0;
}


int Device_Time(unsigned char *buf)
{
	int i = GetDeviceIndex(buf);
	if(i < 0)
		return false;
	time_t time_s = (buf[12]<<24) | (buf[13]<<16) | (buf[14]<<8) | buf[15];
	struct tm *time_now = localtime(&time_s);
	memset(device[i].realdata.DeviceTime,'\0',sizeof(device[i].realdata.DeviceTime));
	sprintf(device[i].realdata.DeviceTime,"%d-%02d-%02d %02d:%02d:%02d",time_now->tm_year+1900,time_now->tm_mon+1,time_now->tm_yday,time_now->tm_hour,time_now->tm_min,time_now->tm_sec);
	return true;
}
int Device_DetectData(unsigned char *buf)
{
	int i =GetDeviceIndex(buf);
	if(i < 0)
		return false;
	int index = 12;
	//7E 00 14 00 01 FF FF 00 02 40 01 01 07 D4 CB CD A8 31 30 35   08 BE A9 41 4B 53 38 38 37   04 31 30 30 31 4F DC 68 2E 01 01 04 00 01 01 00 03 4F DC 68 2E 6D 7D
	memcpy(device[i].realdata.line_number , buf + index + 1 , buf[index]);
	index = index + buf[index] +1;

	memcpy(device[i].realdata.plate_number , buf + index + 1 , buf[index]);
	index = index + buf[index] +1;

	memcpy(device[i].realdata.RFID , buf + index + 1 , buf[index]);
	index = index + buf[index] +1;

	device[i].realdata.detect_time = (buf[index]<<24) | (buf[index + 1]<<16) | (buf[index + 2]<<8) | buf[index + 3];
	index = index + 4;

	device[i].realdata.is_priority = buf[index]?true:false;
	index++;

	device[i].realdata.priority_time = buf[index];
	index++;

	device[i].realdata.output_port = (buf[index] << 8) | buf[index+1];
	index += 2;

	device[i].realdata.detect_direction = buf[index];
	index++;

	device[i].realdata.cross_number = (buf[index] << 8) | buf[index+1];
	index += 2;

	device[i].realdata.request_time = (buf[index]<<24) | (buf[index + 1]<<16) | (buf[index + 2]<<8) | buf[index + 3];

	return true;
}

int Device_Fault(unsigned char *buf)
{
	int i =GetDeviceIndex(buf);
	if(i < 0)
		return false;
	device[i].realdata.fault_type = buf[12];
	device[i].realdata.fault_number = (buf[13] << 8) | buf[14];
	device[i].realdata.fault_time = (buf[15]<<24) | (buf[16]<<16) | (buf[17]<<8) | buf[18];
	return true;
}

int GetDeviceIndex(unsigned char *buf)
{
	int device_id = ( buf[7] << 8 ) | buf[8];
	int i;
	for(i = 0; i< DeviceMaxNum; i++)
	{
		if(device_id == device[i].id)
			break;
	}
	if(i == DeviceMaxNum)
		return -1;
	return i;
}
