/*
 * CommunicateWithDevice.cpp
 *
 * Created on: 2016年10月9日
 * Author: liruijie
 */
#include "include.h"


#define ReceiveDevicePort 10085


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
		perror("Fail to create device parse data pthread\n");
		exit(EXIT_FAILURE);
	}
	//creat update pthread
	pthread_t pth_update_time;
	ret = pthread_create(&pth_update_time,NULL,UpdateStatus_Device,NULL);
	if(ret != 0)
	{
		perror("Fail to create device updte status pthread\n");
		exit(EXIT_FAILURE);
	}
}



void * ReceiveData_Device(void *arg)
{
	struct sockaddr_in myaddr;
	int sockfd_device;
	struct sockaddr_in ServerAddr;
	int sin_size_1=sizeof(struct sockaddr_in);
	char Rec_buf[SingleRecvMaxLen];
	fd_set  Rec_fdSet;
	int ret,numbytes;
	if ((sockfd_device = socket(PF_INET, SOCK_DGRAM, 0)) < 0)
	{
		perror("fail to socket");
		exit(-1);
	}

	int on=1;
	if((setsockopt(sockfd_device,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(on)))<0)
	 {
			perror("setsockopt failed");
			exit(EXIT_FAILURE);
	 }

	bzero(&myaddr, sizeof(myaddr));
	myaddr.sin_family = PF_INET;
	myaddr.sin_port = htons(ReceiveDevicePort);
	myaddr.sin_addr.s_addr = INADDR_ANY;
	if ( bind( sockfd_device, (struct sockaddr *)&myaddr, sizeof(myaddr) ) < 0 )
	{
		perror("fail to bind");
		exit(-1);
	}

	while(1)
	{

		FD_ZERO(&Rec_fdSet);
		FD_SET(sockfd_device, &Rec_fdSet);
		ret = select((int) sockfd_device + 1, &Rec_fdSet,NULL, NULL, NULL);
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
			if ((numbytes=recvfrom(sockfd_device,  Rec_buf,  SingleRecvMaxLen,  0,  (struct sockaddr *)&ServerAddr, (socklen_t *) &sin_size_1))==-1)
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
				if( check_buf( (unsigned char *) Rec_buf) == true)
				{
					EnQueue((unsigned char *)Rec_buf);
				}
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
			if(data[9] ==0x40)
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
			}
		}
	}
	return (void *)0;
}

void * UpdateStatus_Device(void *arg)
{
	int i,device_num;
	time_t time_now;
	char temp_buf[100];
	char sqlbuf[] = "update table set name1 = :x1 where name3 = x2";
	stmt_device->setSQL(sqlbuf);
	stmt_device->setMaxIterations(CurrentExistDevice);
	stmt_device->setMaxParamSize(1,30);
	stmt_device->setMaxParamSize(2,sizeof(int));

	while(1)
	{
		time(&time_now);
		try
		{
			device_num = 0;
			for(i = 0; i < CurrentExistDevice;i++)
			{
				if(device[i].status == 0)    //设备状态是离线，查询实时数据中的时间是否更新，如果已经更新就认为是在线
				{
					if((time_now - device[i].last_report_time) < 5)
					{
						device[i].status = 1;
						sprintf(temp_buf,"update ");
						//stmt_device->execute(temp_buf);
					}
					else //设备状态确实是离线的
					{
						continue;
					}
				}
				else if((time_now - device[i].last_report_time) > 10) //设备状态是在线，但是已经10秒没有上报时间,认为已经离线
				{
					device[i].status = 0;
					sprintf(temp_buf,"update ");
					//stmt_device->execute(temp_buf);
					continue;
				}
				if(device_num != 0)
					stmt_device->addIteration();
				stmt_device->setString(1,device[i].realdata.DeviceTime);
				stmt_device->setInt(2,device[i].id);
				device_num = device_num + 1;
			}
			if(device_num !=0 )
				stmt_device->executeUpdate();
		}
		catch (SQLException &sqlExcp)
		{
		   sqlExcp.getErrorCode();
		   string strinfo=sqlExcp.getMessage();
		   cout<<strinfo;
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
	time_t _time_;
	time(&_time_);
	device[i].last_report_time = _time_;
	time_t time_s = (buf[12]<<24) | (buf[13]<<16) | (buf[14]<<8) | buf[15];
	struct tm *time_now = localtime(&time_s);
	memset(device[i].realdata.DeviceTime,'\0',sizeof(device[i].realdata.DeviceTime));
	sprintf(device[i].realdata.DeviceTime,"%d-%02d-%02d %02d:%02d:%02d",time_now->tm_year+1900,time_now->tm_mon+1,time_now->tm_yday,time_now->tm_hour,time_now->tm_min,time_now->tm_sec);
	return true;
}
int Device_DetectData(unsigned char *buf)
{
	char temp_buf[50];
	char temp_outbuf[100];
	int ret;
	int i =GetDeviceIndex(buf);
	if(i < 0)
		return false;
	time_t time_now;
	time(&time_now);
	device[i].last_report_time = time_now;
	int index = 12;
	//7E 00 14 00 01 FF FF 00 02 40 01 01 07 D4 CB CD A8 31 30 35   08 BE A9 41 4B 53 38 38 37   04 31 30 30 31 4F DC 68 2E 01 01 04 00 01 01 00 03 4F DC 68 2E 6D 7D

	//memcpy(device[i].realdata.line_number , buf + index + 1 , buf[index]);
	memset(temp_buf,'\0',sizeof(temp_buf));
	memset(temp_outbuf,'\0',sizeof(temp_outbuf));
	memcpy(temp_buf , buf + index + 1 , buf[index]);
//	ret = code_convert((char *)"GBK",(char *)"UTF-8",temp_buf,buf[index],temp_outbuf,100);
//	if(ret == 0)
//		return false;
	memset(device[i].realdata.line_number,'\0',sizeof(device[i].realdata.line_number));
	memcpy(device[i].realdata.line_number , temp_outbuf , ret);
	index = index + buf[index] +1;

	//memcpy(device[i].realdata.plate_number , buf + index + 1 , buf[index]);
	memset(temp_buf,'\0',sizeof(temp_buf));
	memset(temp_outbuf,'\0',sizeof(temp_outbuf));
	memcpy(temp_buf , buf + index + 1 , buf[index]);
//	ret = code_convert((char *)"GBK",(char *)"UTF-8",temp_buf,buf[index],temp_outbuf,100);
//	if(ret == 0)
//		return false;
	memset(device[i].realdata.plate_number,'\0',sizeof(device[i].realdata.plate_number));
	memcpy(device[i].realdata.plate_number , temp_outbuf , ret);
	index = index + buf[index] +1;

	memcpy(device[i].realdata.RFID , buf + index + 1 , buf[index]);
	index = index + buf[index] +1;

	device[i].realdata.detect_time = (buf[index]<<24) | (buf[index + 1]<<16) | (buf[index + 2]<<8) | buf[index + 3];
	index = index + 4;

	device[i].realdata.is_priority = buf[index];
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
	time_t time_now;
	time(&time_now);
	device[i].last_report_time = time_now;
	device[i].realdata.fault_type = buf[12];
	device[i].realdata.fault_number = (buf[13] << 8) | buf[14];
	device[i].realdata.fault_time = (buf[15]<<24) | (buf[16]<<16) | (buf[17]<<8) | buf[18];
	return true;
}
/*
 * 获取报文中的设备在设备信息结构体数组中的下标
 */
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

bool check_buf( unsigned char * rcv_buf)
{
	int rcv_len = rcv_buf[1] * 256 + rcv_buf[2];
	int check_sum = 0;
	int i;
	if (0x7e == rcv_buf[0] && 0x7d == rcv_buf[rcv_len + 1])
	{

		for (i = 1; i < rcv_len; i++)
		{
			check_sum += rcv_buf[i];
		}

		if (rcv_buf[rcv_len] == (check_sum & 0xFF))
		{
			return true;
		}
		else
		{
			printf(" jiao yan cuo wu\n");
			return false;
		}

		}
		else
		{
			printf(" signal tou wei cuo wu\n");
			return false;
		}
}
int makecheck(unsigned char * rcv_buf)
{
	int rcv_len = rcv_buf[1] * 256 + rcv_buf[2];
	int check_sum = 0;
	int i;
	for (i = 1; i < rcv_len; i++)
	{
		check_sum += rcv_buf[i];
	}
	return (check_sum & 0xFF);
}
