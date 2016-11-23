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

int CommunicateWithDevice()
{
	int ret;

	//init oracle link

	ret = GetConnectFromPool(&conn_device,&stmt_device);
	if(ret == false)
		return false;
	stmt_device->setAutoCommit(true);
	InitQueue();
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
	return true;
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
	cout << "receive device data pthread\n"<<endl;
	prctl(PR_SET_NAME, (unsigned long)"ReceiveData_Device");
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
			exit(-1);
		}
		else          //select success
		{
			memset(Rec_buf,'\0',SingleRecvMaxLen);
			if ((numbytes=recvfrom(sockfd_device,  Rec_buf,  SingleRecvMaxLen,  0,  (struct sockaddr *)&ServerAddr, (socklen_t *) &sin_size_1))==-1)
			 {
				perror("recv ");
				exit(-1);
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
	int ret;
	cout << "parsing device data pthread \n"<<endl;
	prctl(PR_SET_NAME, (unsigned long)"ParsingData_Device");
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
						ret = Device_Time(data);
						break;
					case 0x01:  //车辆检测信息
						ret = Device_DetectData(data);
						break;
					case 0x02:  //故障数据
						ret = Device_Fault(data);
						break;
					default:
						break;
				}
			}
			if(ret == false)
			{
				DisconnectOracle(&conn_device,&stmt_device);
				int try_count = 0;
		_connetc_oracle:
				try_count++;
				ret = GetConnectFromPool(&conn_device,&stmt_device);
				if(ret == false)
				{
					if(try_count >= 10)
					{
						exit(0);
					}
					else
					{
						goto _connetc_oracle;
					}
				}

			}

		}
	}
	return (void *)0;
}

void * UpdateStatus_Device(void *arg)
{
	Connection *conn_update;
	Statement *stmt_update;
	int i,device_num;
	time_t time_now;
	int ret;
	ret = GetConnectFromPool(&conn_update,&stmt_update);
	if(ret == false)
		return (void *) 0;
	stmt_update->setAutoCommit(false);
	cout << "update device status pthread\n"<<endl;
	prctl(PR_SET_NAME, (unsigned long)"UpdateStatus_Device");
	char sqlbuf_status[200];
	char sqlbuf[] = "update UNIT_CUR_STAUS set DATA_UPDATE_TIME = sysdate,device_time = to_date(:x1,'yyyy-mm-dd hh24:mi:ss') where UNIT_ID = :x2";
	stmt_update->setSQL(sqlbuf);
	stmt_update->setMaxIterations(CurrentExistDevice);

	stmt_update->setMaxParamSize(1,30);
	stmt_update->setMaxParamSize(2,sizeof(int));
	char temp_buf[10];
	char time_buf[30];
	struct tm *fault_time;
	ResultSet *Result;
	char fault_id[30];
	while(1)
	{
		time(&time_now);
		try
		{
			device_num = 0;
			for(i = 0; i < CurrentExistDevice;i++)
			{
				if(device[i].status == OFFLINE)    //设备状态是离线，判断最后上报时间是否更新，如果已经更新就认为是在线
				{
					if((time_now - device[i].last_report_time) < 5)
					{
						device[i].status = ONLINE;
						sprintf(sqlbuf_status,"update UNIT_CUR_STAUS set DATA_UPDATE_TIME = sysdate,CONTROL_MODE = '2' where UNIT_ID = '%d'",device[i].id);
						stmt_device->execute(sqlbuf_status);
//						if(device_num != 0)
//							stmt_update->addIteration();
//						sprintf(temp_buf,"2");
//						stmt_update->setString(1,temp_buf);
//						stmt_update->setString(2,device[i].realdata.DeviceTime);
//						sprintf(temp_buf,"%d",device[i].id);
//						stmt_update->setString(3,temp_buf);
//						device_num = device_num + 1;

						//删除实时故障表里的故障数据   在历史故障表插入恢复信息
						try
						{
							sprintf(sqlbuf,"select ID from REAL_FAULT_RECORD where UNIT_ID = %d and FAULT_TYPE = 1 and FAULT_INFO = '设备离线'",device[i].id);
							Result = stmt_device->executeQuery(sqlbuf);
							if(Result->next() != 0)
							{

								sprintf(sqlbuf,"delete from REAL_FAULT_RECORD where ID = %s ",Result->getString(1).c_str());
								stmt_device->execute(sqlbuf);
								stmt_device->closeResultSet(Result);
								fault_time = localtime(&time_now);
								sprintf(fault_id,"%ld%05d1700",time_now,device[i].id);
								sprintf(time_buf,"%d-%02d-%02d %02d:%02d:%02d",fault_time->tm_year+1900,fault_time->tm_mon+1,
														fault_time->tm_mday,fault_time->tm_hour,fault_time->tm_min,fault_time->tm_sec);
								sprintf(sqlbuf,"insert into HIS_FAULT_RECORD(ID,UNIT_ID,update_time,FAULT_TYPE,FAULT_INFO) values('%s',%d,to_date('%s','yyyy-mm-dd hh24:mi:ss'),17,'通讯恢复')",
														fault_id,device[i].id,time_buf);
								stmt_device->execute(sqlbuf);
							}
						}
						catch(SQLException &Excp)
						{
							Excp.getErrorCode();
							cout << Excp.getMessage() <<endl;
							printf("%s\n",sqlbuf);
							cout <<__FUNCTION__<< __LINE__ <<endl;
						}
						continue;
					}
				}
				else  //设备状态是在线
				{
					if(device_num != 0)
						stmt_update->addIteration();
					//但是已经10秒没有上报时间,认为已经离线
					if((time_now - device[i].last_report_time) > 10)
					{
						device[i].status = OFFLINE;
						sprintf(sqlbuf_status,"update UNIT_CUR_STAUS set DATA_UPDATE_TIME = sysdate,CONTROL_MODE = '4' where UNIT_ID = '%d'",device[i].id);
						stmt_device->execute(sqlbuf_status);

						//插入设备离线故障
						try
						{
							sprintf(sqlbuf,"select ID from REAL_FAULT_RECORD where UNIT_ID = %d and FAULT_TYPE = 1 and FAULT_INFO = '设备离线'",device[i].id);
							puts(sqlbuf);
							Result = stmt_device->executeQuery(sqlbuf);
							if(Result->next() != 0)
							{
								stmt_device->closeResultSet(Result);
							}
							else
							{
								fault_time = localtime(&time_now);
								sprintf(fault_id,"%ld%05d0100",time_now,device[i].id);
								sprintf(time_buf,"%d-%02d-%02d %02d:%02d:%02d",fault_time->tm_year+1900,fault_time->tm_mon+1,
										fault_time->tm_mday,fault_time->tm_hour,fault_time->tm_min,fault_time->tm_sec);
								sprintf(sqlbuf,"insert into REAL_FAULT_RECORD(ID,UNIT_ID,FAULT_TIME,FAULT_TYPE,FAULT_INFO) values('%s',%d,to_date('%s','yyyy-mm-dd hh24:mi:ss'),1,'设备离线')",
													fault_id,device[i].id,time_buf);
								stmt_device->execute(sqlbuf);
							}
						}
						catch(SQLException &Excp)
						{
							Excp.getErrorCode();
							cout << Excp.getMessage() <<endl;
							printf("%s\n",sqlbuf);
							cout <<__FUNCTION__<< __LINE__ <<endl;
						}
					}
					else			//设备状态是正常的
					{
						stmt_update->setString(1,device[i].realdata.DeviceTime);
						sprintf(temp_buf,"%d",device[i].id);
						stmt_update->setString(2,temp_buf);
						device_num = device_num + 1;
						continue;
					}
				}
			}
			if(device_num !=0 )
			{
				stmt_update->executeUpdate();
				conn_update->commit();
			}
		}
		catch (SQLException &sqlExcp)
		{

			int ErrorCode = sqlExcp.getErrorCode();
			printf("ErrorCode = %d\n",ErrorCode);
		   cout << sqlExcp.getMessage() <<endl;
		   printf("%s\n",sqlbuf);
		   cout <<__FUNCTION__<< __LINE__ <<endl;
		}
		sleep(1);
		//puts("update device status\n");
	}
	return (void *)0;
}


int Device_Time(unsigned char *buf)
{
	int i = GetDeviceIndex(buf);
	if(i < 0)
		return false;
	time_t time_now_s;
	time(&time_now_s);
	device[i].last_report_time = time_now_s;
	time_t time_s = (buf[12]<<24) | (buf[13]<<16) | (buf[14]<<8) | buf[15];
	struct tm *time_now = gmtime(&time_s);
	memset(device[i].realdata.DeviceTime,'\0',sizeof(device[i].realdata.DeviceTime));
	sprintf(device[i].realdata.DeviceTime,"%d-%02d-%02d %02d:%02d:%02d",time_now->tm_year+1900,time_now->tm_mon+1,time_now->tm_mday,time_now->tm_hour,time_now->tm_min,time_now->tm_sec);
	//puts(device[i].realdata.DeviceTime);
	return true;
}
int Device_DetectData(unsigned char *buf)
{
	//char temp_buf[50];
	//char temp_outbuf[100];
	//int ret;
	char sqlbuf[500];
	ResultSet *Result;
	char Record_ID[25];
	int i =GetDeviceIndex(buf);
	if(i < 0)
		return false;
	time_t time_now_s;
	time(&time_now_s);
	device[i].last_report_time = time_now_s;
	int index = 12;
	puts("车辆检测信息\n");
	memset(device[i].realdata.line_number,'\0',sizeof(device[i].realdata.line_number));
	memcpy(device[i].realdata.line_number , buf + index + 1 , buf[index]);
	index = index + buf[index] +1;

	memset(device[i].realdata.plate_number,'\0',sizeof(device[i].realdata.plate_number));
	memcpy(device[i].realdata.plate_number , buf + index + 1 , buf[index]);
	index = index + buf[index] +1;

	device[i].realdata.RFID = (buf[index]<<24) | (buf[index + 1]<<16) | (buf[index + 2]<<8) | buf[index + 3];
	index = index +4;

	device[i].realdata.detect_time = (buf[index]<<24) | (buf[index + 1]<<16) | (buf[index + 2]<<8) | buf[index + 3];
	index = index + 4;

	device[i].realdata.is_priority = buf[index];
	index++;

	device[i].realdata.priority_level = buf[index];
	index++;

	device[i].realdata.priority_time = buf[index];
	index++;

	device[i].realdata.output_port = (buf[index] << 8) | buf[index+1];
	index += 2;

	device[i].realdata.detect_direction = buf[index];
	index++;

	device[i].realdata.request_time = (buf[index]<<24) | (buf[index + 1]<<16) | (buf[index + 2]<<8) | buf[index + 3];
	index= index + 4;
	device[i].realdata.IsLeave = buf[index];

	//printf("车辆检测信息\n");
	struct tm *request_time = gmtime( &(device[i].realdata.request_time));//(long *)
	char time_buf[30];
	sprintf(time_buf,"%d-%02d-%02d %02d:%02d:%02d",request_time->tm_year+1900,request_time->tm_mon+1,
	request_time->tm_mday,request_time->tm_hour,request_time->tm_min,request_time->tm_sec);
	sprintf(sqlbuf,"select RECORD_ID FROM REAL_TIME_REQUEST_INFO where RFID_ID = '%ld'",device[i].realdata.RFID);
	try
	{
		Result = stmt_device->executeQuery(sqlbuf);
		if(Result->next() != 0)							//数据库中存在记录
		{
			sprintf(Record_ID,Result->getString(1).c_str());
			stmt_device->closeResultSet(Result);
			if(device[i].realdata.IsLeave == 0)		//数据库中存在记录，但是车辆已经离开检测范围，应该删除记录
			{
				puts("车辆离开，删除记录\n");
				sprintf(sqlbuf,"delete from REAL_TIME_REQUEST_INFO where RECORD_ID = '%s'",Record_ID);
				stmt_device->execute(sqlbuf);
				sprintf(sqlbuf,"delete from REAL_TIME_REQUEST where RECORD_ID = '%s'",Record_ID);
				stmt_device->execute(sqlbuf);
			}
			else												//数据库中存在记录，但是车辆还在检测范围，应该更新记录
			{
				puts("更新记录\n");
				sprintf(sqlbuf,"update REAL_TIME_REQUEST set REQUEST_TIME = to_date('%s','yyyy-mm-dd hh24:mi:ss'),PRIORITY = '%d',"
						"CONTROL_BOARD_OUTPUT = '%d',PRIORITY_TIME = '%d',IS_PASS = '%d',DIRECTION = '%d' where RECORD_ID = '%s'",
						time_buf,device[i].realdata.priority_level,device[i].realdata.output_port,device[i].realdata.priority_time,
						device[i].realdata.is_priority,device[i].realdata.detect_direction,Record_ID);
				//puts(sqlbuf);
				stmt_device->execute(sqlbuf);
			}
		}
		else													//数据库中不存在记录，应该插入记录
		{
			puts("插入检测记录\n");
			time_t time_now;
			time(&time_now);
			//记录ID为当前时间+10位卡号
			sprintf(Record_ID,"%ld%010ld",time_now,device[i].realdata.RFID);

			request_time = gmtime( &(device[i].realdata.detect_time));
			sprintf(time_buf,"%d-%02d-%02d %02d:%02d:%02d",request_time->tm_year+1900,request_time->tm_mon+1,
									request_time->tm_mday,request_time->tm_hour,request_time->tm_min,request_time->tm_sec);

			sprintf(sqlbuf,"insert into REAL_TIME_REQUEST(RECORD_ID,INTERSECTION_ID,REQUEST_TIME,PRIORITY,CONTROL_BOARD_OUTPUT,PRIORITY_TIME,IS_PASS,DIRECTION) "
					"values('%s',%d,to_date('%s','yyyy-mm-dd hh24:mi:ss'),'%d','%d','%d','%d','%d')",Record_ID,device[i].id,time_buf,device[i].realdata.priority_level,
					device[i].realdata.output_port,device[i].realdata.priority_time,device[i].realdata.is_priority,device[i].realdata.detect_direction);
			stmt_device->execute(sqlbuf);
			puts(sqlbuf);

			sprintf(sqlbuf,"insert into REAL_TIME_REQUEST_INFO(RECORD_ID,PLATE_ID,REQUEST_TIME,BUS_CLASS,RFID_ID) "
							"values('%s','%s',to_date('%s','yyyy-mm-dd hh24:mi:ss'),'%s','%ld')",
							Record_ID,device[i].realdata.plate_number,time_buf,device[i].realdata.line_number,device[i].realdata.RFID);
			puts(sqlbuf);
			stmt_device->execute(sqlbuf);
		}
	}
	catch (SQLException &sqlExcp)
	{
	   sqlExcp.getErrorCode();
	   string strinfo=sqlExcp.getMessage();
	   cout<<strinfo;
	   puts(sqlbuf);
	   cout <<__FUNCTION__<< __LINE__ <<endl;
	   return false;
	}

	return true;
}

int Device_Fault(unsigned char *buf)
{
	char sqlbuf[500];
	ResultSet *Result;
	char fault_id[30];
	time_t time_now;
	bool IsFaultExist = false;
	int i =GetDeviceIndex(buf);
	if(i < 0)
		return false;
	time_t time_now_s;
	time(&time_now_s);
	device[i].last_report_time = time_now_s;
	char time_buf[30];
	char fault_info[100];
	char recover_info[100];
	device[i].realdata.fault_type = buf[12];
	device[i].realdata.fault_number = (buf[13] << 8) | buf[14];
	device[i].realdata.fault_time = (buf[15]<<24) | (buf[16]<<16) | (buf[17]<<8) | buf[18];
	switch(device[i].realdata.fault_type)
	{
		case 0x12:
			sprintf(recover_info,"检测器故障恢复: %d",device[i].realdata.fault_number);
		case 0x02:
			sprintf(fault_info,"检测器故障: %d",device[i].realdata.fault_number);
			break;

		case 0x13:
			sprintf(recover_info,"主板zigbee故障恢复");
		case 0x03:
			sprintf(fault_info,"主板zigbee故障");
			break;

		case 0x14:
			sprintf(recover_info,"从板zigbee故障恢复: %d",device[i].realdata.fault_number);
		case 0x04:
			sprintf(fault_info,"从板zigbee故障: %d",device[i].realdata.fault_number);
			break;

		default:
			return false;
	}
	time(&time_now);
	//故障id 为  时间+5位信号机ID+2位故障类型+2位故障编号
	sprintf(fault_id,"%ld%05d%02d%02d",time_now,device[i].id,device[i].realdata.fault_type,device[i].realdata.fault_number);
	struct tm *fault_time = gmtime( &(device[i].realdata.fault_time));
	sprintf(time_buf,"%d-%02d-%02d %02d:%02d:%02d",fault_time->tm_year+1900,fault_time->tm_mon+1,
			fault_time->tm_mday,fault_time->tm_hour,fault_time->tm_min,fault_time->tm_sec);

	//先判断实时故障表是否有相同故障
	try
	{
		sprintf(sqlbuf,"select ID from REAL_FAULT_RECORD where UNIT_ID = %d and FAULT_TYPE = %d and FAULT_INFO = '%s'",device[i].id,(device[i].realdata.fault_type & 0x0F),fault_info);
		Result = stmt_device->executeQuery(sqlbuf);
		if(Result->next() != 0)
		{
			//数据库中存在相同故障
			stmt_device->closeResultSet(Result);
			IsFaultExist = true;
		}

		if(device[i].realdata.fault_type < 0x10)			//故障报文
		{
			// 有就不不处理  没有就插入
			if(IsFaultExist == false)
			{
				sprintf(sqlbuf,"insert into REAL_FAULT_RECORD(ID,UNIT_ID,FAULT_TIME,FAULT_TYPE,FAULT_INFO) values('%s',%d,to_date('%s','yyyy-mm-dd hh24:mi:ss'),%d,'%s')",
									fault_id,device[i].id,time_buf,device[i].realdata.fault_type,fault_info);
				stmt_device->execute(sqlbuf);
			}
		}
		else														//故障恢复报文
		{
			//实时故障表里有故障
			if(IsFaultExist == true)
			{
				//删除实时故障表里的故障数据   在历史故障表插入恢复信息
				sprintf(sqlbuf,"delete from REAL_FAULT_RECORD where UNIT_ID = %d and FAULT_TYPE = %d and FAULT_INFO = '%s'",device[i].id,(device[i].realdata.fault_type & 0x0F),fault_info);
				stmt_device->execute(sqlbuf);

				sprintf(sqlbuf,"insert into HIS_FAULT_RECORD(ID,UNIT_ID,UPDATE_TIME,FAULT_TYPE,FAULT_INFO) values('%s',%d,to_date('%s','yyyy-mm-dd hh24:mi:ss'),%d,'%s')",
										fault_id,device[i].id,time_buf,device[i].realdata.fault_type,recover_info);
				stmt_device->execute(sqlbuf);
			}
		}
	}
	catch (SQLException &sqlExcp)
	{
	   sqlExcp.getErrorCode();
	   cout<<sqlExcp.getMessage();
	   printf("%s\n",sqlbuf);
	   cout <<__FUNCTION__<< __LINE__ <<endl;
	   return false;
	}
	return true;
}
/*
 * 获取报文中的设备在设备信息结构体数组中的下标
 */
int GetDeviceIndex(unsigned char *buf)
{
	unsigned int device_id = ( buf[7] << 8 ) | buf[8];
	int i;

	for(i = 0; i< CurrentExistDevice; i++)
	{
		if(device_id == device[i].id)
		{
			break;
		}
	}
	if(i == CurrentExistDevice)
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
