/*
 * CommunicateWithWeb.cpp
 *
 * Created on: 2016年10月11日
 * Author: LIRUIJIE
 */
#include "include.h"

#define ReceiveWebPort	10084
#define TimeOut_S			5
extern oracle::occi::Environment *OraEnviroment;
static int WebSockfd;
pthread_mutex_t WebSend_Mutex;
struct web_message_info
{
		unsigned char RecvBuf[SingleRecvMaxLen];
		struct sockaddr_in WebServerAddr;
};



int CommunicateWithWeb()
{
	int ret;
	pthread_t pth_receive_data;
	pthread_mutex_init(&WebSend_Mutex,NULL);
	ret = pthread_create(&pth_receive_data,NULL,ReceiveData_Web,NULL);
	if(ret != 0)
	{
		perror("Fail to create device receive data pthread\n");
		exit(EXIT_FAILURE);
	}
	return true;
}
void * ReceiveData_Web(void *arg)
{
	struct sockaddr_in myaddr;
	struct sockaddr_in WebServerAddr;
	fd_set  Rec_fdSet;
	int ret,numbytes;
	unsigned char recvbuf[SingleRecvMaxLen];
	int sin_size_1=sizeof(struct sockaddr_in);
	cout <<"creat ReceiveData_Web pthread\n"<<endl;
	prctl(PR_SET_NAME, (unsigned long)"ReceiveData_Web");

	if ((WebSockfd = socket(PF_INET, SOCK_DGRAM, 0)) < 0)
	{
		perror("fail to socket");
		exit(-1);
	}

	int on=1;
	if((setsockopt(WebSockfd,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(on)))<0)
	 {
			perror("setsockopt failed");
			exit(EXIT_FAILURE);
	 }

	bzero(&myaddr, sizeof(myaddr));
	myaddr.sin_family = PF_INET;
	myaddr.sin_port = htons(ReceiveWebPort);
	myaddr.sin_addr.s_addr = INADDR_ANY;
	if ( bind( WebSockfd, (struct sockaddr *)&myaddr, sizeof(myaddr) ) < 0 )
	{
		perror("fail to bind");
		exit(-1);
	}

	while(1)
	{

		FD_ZERO(&Rec_fdSet);
		FD_SET(WebSockfd, &Rec_fdSet);
		ret = select((int) WebSockfd + 1, &Rec_fdSet,NULL, NULL, NULL);
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
			memset(recvbuf,'\0',SingleRecvMaxLen);
			if ((numbytes=recvfrom(WebSockfd,  recvbuf,  SingleRecvMaxLen,  0,  (struct sockaddr *)&WebServerAddr, (socklen_t *) &sin_size_1))==-1)
			{
				perror("recv ");
				return 0;
			}
			if(numbytes == 0)
			{
				continue;
			}
			else if(numbytes >7)
			{
				recvbuf[numbytes] = '\0';
				if( check_buf( recvbuf ) == true)
				{
					ParsingData_Web(recvbuf,WebServerAddr);
				}
			}
		}
	}
	return (void *)0;
}

void SendToWeb(struct sockaddr_in WebAddr,unsigned char *buf,int len)
{
	pthread_mutex_lock(&WebSend_Mutex);
	sendto(WebSockfd, buf, len, 0, (struct sockaddr *)&WebAddr, sizeof(struct sockaddr));
	pthread_mutex_unlock(&WebSend_Mutex);
}


/*
 *建立一个处理平台报文的线程,将Web平台的地址和收到的数据封装到结构体中,当成参数传入线程函数
 */
void ParsingData_Web(unsigned char *data,struct sockaddr_in WebAddr)
{
	int i =GetDeviceIndex(data);
	if(i < 0)
		return ;
	pthread_t pth_handle;
	pthread_attr_t attr;
	int detachstate;
	pthread_attr_init(&attr);
	pthread_attr_getdetachstate(&attr,&detachstate);
	if(detachstate==PTHREAD_CREATE_JOINABLE)
	{
		pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);//设置为线程分离属性设置为分离
	}
	struct web_message_info *web_message = (struct web_message_info *)malloc(sizeof(web_message_info));
	memcpy(web_message->RecvBuf,data,SingleRecvMaxLen);
	web_message->WebServerAddr = WebAddr;
	pthread_create(&pth_handle,&attr,Handle_Pthread,web_message);
	pthread_attr_destroy(&attr);					//回收分配给属性的资源

}

/*
 *处理平台报文的线程函数，解析报文，调用相应的功能函数,并向Web平台回复结果
 */
void * Handle_Pthread(void *arg)
{
	unsigned char data[SingleRecvMaxLen];
	pthread_detach(pthread_self());
	struct web_message_info web_message = *(struct web_message_info *)arg;
	memcpy(&web_message,arg,sizeof(web_message_info));
	free(arg);
	struct sockaddr_in WebAddr = web_message.WebServerAddr;
	memcpy(data,web_message.RecvBuf,SingleRecvMaxLen);

	printf("Web pthread ,addr:%s,port:%d\n",inet_ntoa(WebAddr.sin_addr),ntohs(WebAddr.sin_port));
	int udp_sockfd;
	struct sockaddr_in DeviceAddr;
	int CrossID = (data[7] << 8) | data[8];
	int ret;
	int on=1;
	unsigned char reply_buf[15] = {0x7E,0x00,0x0C,0x00,0x00,0xFF,0xFF,0x00,0x00,0x30,0x00,0x00,0x00,0x00,0x7D};
	//协议版本
	reply_buf[3] = data[3];
	reply_buf[4] = data[4];
	//路口ID
	reply_buf[7] = data[7];
	reply_buf[8] = data[8];

	reply_buf[10] = data[10];	//数据编码
	reply_buf[11] = data[11];	//数据编号
	int i =GetDeviceIndex(data);
	//创建一个和设备通讯的socket
	if ((udp_sockfd = socket(PF_INET, SOCK_DGRAM, 0)) < 0)
	{
		perror("fail to socket");
		ret = false;
		goto report_result;
	}


	if((setsockopt(udp_sockfd,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(on)))<0)
	{
		perror("setsockopt failed");
		close(udp_sockfd);
		ret = false;
		goto report_result;
	}
	DeviceAddr.sin_family = PF_INET;
	DeviceAddr.sin_port = htons(10086);
	DeviceAddr.sin_addr.s_addr = inet_addr(device[i].ip);
	switch(data[9])
	{
		case 0x10:			//查询参数
		{
			switch(data[10])
			{
				case 0x03:  //设备参数
					ret = Request_DeviceParam_0x03(udp_sockfd,DeviceAddr,data);
					break;
				case 0x04:  //系统参数
					ret = Request_DeviceSystemParam_0x04(udp_sockfd,DeviceAddr,data);
					break;
				case 0x05:  //优先策略
					ret = Request_DeviceStrategy_0x05(udp_sockfd,DeviceAddr,data);
					break;
				case 0x06:  //策略时间表
					ret = Request_DeviceStrategyTime_0x06(udp_sockfd,DeviceAddr,data);
					break;
				case 0x07:  //时间调度表
					ret = Request_DeviceSchedule_0x07(udp_sockfd,DeviceAddr,data);
					break;
				case 0x08:  //车辆信息表
					ret = Request_DeviceCard_0x08(udp_sockfd,DeviceAddr,data);
					break;
				default:
					ret = false;
					break;
			}
			break;
		}
		case 0x20:			//设置参数
		{
			switch(data[10])
			{
				case 0x03:  //设备参数
					ret = Set_DeviceParam_0x03(udp_sockfd,DeviceAddr,data);
					break;
				case 0x04:  //系统参数
					ret = Set_DeviceSystemParam_0x04(udp_sockfd,DeviceAddr,data);
					break;
				case 0x05:  //优先策略
					ret = Set_DeviceStrategy_0x05(udp_sockfd,DeviceAddr,data);
					break;
				case 0x06:  //策略时间表
					ret = Set_DeviceStrategyTime_0x06(udp_sockfd,DeviceAddr,data);
					break;
				case 0x07:  //时间调度表
					ret = Set_DeviceSchedule_0x07(udp_sockfd,DeviceAddr,data);
					break;
				case 0x08:  //车辆信息表
					ret = Set_DeviceCard_0x08(udp_sockfd,DeviceAddr,data);
					break;
				case 0x09:	//设置时间
					ret = Set_DeviceTime(udp_sockfd,CrossID, DeviceAddr);
					break;
				case 0x0A:	//重启设备
					ret = Se_tDeviceReboot(udp_sockfd,CrossID,DeviceAddr);
					break;
				case 0x0B:	//设置优先
					ret = Se_tDevicePriority(udp_sockfd,DeviceAddr,data);
					break;
				default:
					break;
			}
			break;
		}
		default:
			break;
	}
	close(udp_sockfd);
report_result:

	if(ret == true)
	{
		reply_buf[12] = 0x00;
	}
	else
	{
		reply_buf[12] = 0x10;
	}
	reply_buf[13] = makecheck(reply_buf);
	SendToWeb(WebAddr,reply_buf,sizeof(reply_buf));
	pthread_exit((void * ) 0 );
}


/*
 * 查询设备参数
 */
int Request_DeviceParam_0x03(int sockfd,struct sockaddr_in DeviceAddr,unsigned char * send_data)
{
	unsigned char Rec_buf[SingleRecvMaxLen];
	struct Device_CardReaderInfo cardreader[8];
	Connection *_conn = NULL;
	Statement *_stmt = NULL;
	int CrossID = ( send_data[7] << 8 ) | send_data[8];
	char sqlbuf[500];
	int recv_len,i;
	int j = 0;				//结构体下标

	if(GetConnectFromPool(&_conn,&_stmt) == false)
	{
		printf("查询设备参数,连接数据库失败,cross_id= %d\n",CrossID);
		return false;
	}
	_stmt->setAutoCommit(false);
	sendto(sockfd, send_data, ((( send_data[1] << 8 ) | send_data[2])+3), 0, (struct sockaddr *)&DeviceAddr, sizeof(struct sockaddr));
	recv_len = ReceiveDeviceReply( sockfd, DeviceAddr, Rec_buf, TimeOut_S);
	i = 13; 						//数据开始的下标
	j = 0;						//
	if(recv_len>0)
	{
		while( i+13 < recv_len )
		{
			i++;	//空出序号所占字节
			cardreader[j].id = Rec_buf[i++];
			cardreader[j].direction = Rec_buf[i++];
			cardreader[j].dBm = (Rec_buf[i] << 8 | Rec_buf[i+1]);
			i = i+2;
			cardreader[j].zigbee_addr = (Rec_buf[i] << 8 | Rec_buf[i+1]);
			i = i+2;
			cardreader[j].transport_priority_relay = (Rec_buf[i] << 8 | Rec_buf[i+1]);
			i = i+2;
			cardreader[j].emergency_priority_relay = (Rec_buf[i] << 8 | Rec_buf[i+1]);
			i = i+2;
			cardreader[j].detection_range = (Rec_buf[i] << 8 | Rec_buf[i+1]);
			i = i+2;
			j++;
		}
		if(j > 0)         //解析出有效的数据
		{
			try
			{
				sprintf(sqlbuf,"delete from DEVICE_CONFIG where INTERSECTION_ID = '%d'",CrossID);
				_stmt->execute(sqlbuf);
				sprintf(sqlbuf,"insert into DEVICE_CONFIG(INTERSECTION_ID,READER_POWER,READER_BLOCK,BUS_PRIORITY_SWITCH_NUMBER,EMERGENCY_SWITCH_NUMBER,ZIGBEE_ADDRESS,READER_ID,DIRECTION_ID) "
						"values(:x1,:x2,:x3,:x4,:x5,:x6,:x7,:x8)");
				_stmt->setSQL(sqlbuf);
				_stmt->setMaxIterations(j);
				_stmt->setMaxParamSize(1,10);
				_stmt->setMaxParamSize(2,sizeof(int));
				_stmt->setMaxParamSize(3,sizeof(int));
				_stmt->setMaxParamSize(4,sizeof(int));
				_stmt->setMaxParamSize(5,sizeof(int));
				_stmt->setMaxParamSize(6,sizeof(int));
				_stmt->setMaxParamSize(7,sizeof(int));
				_stmt->setMaxParamSize(8,sizeof(int));
				int temp_t;
				char temp_buf[10];

				for(temp_t = 0;temp_t < j;temp_t++)
				{
					if(temp_t != 0)
						_stmt->addIteration();
					sprintf(temp_buf,"%d",CrossID);
					_stmt->setString(1,temp_buf);
					_stmt->setInt(2,cardreader[temp_t].dBm);
					_stmt->setInt(3,cardreader[temp_t].detection_range);
					_stmt->setInt(4,cardreader[temp_t].transport_priority_relay);
					_stmt->setInt(5,cardreader[temp_t].emergency_priority_relay);
					_stmt->setInt(6,cardreader[temp_t].zigbee_addr);
					_stmt->setInt(7,cardreader[temp_t].id);
					_stmt->setInt(8,cardreader[temp_t].direction);
				}
				_stmt->executeUpdate();
				_conn->commit();
				_conn->terminateStatement(_stmt);
				OraEnviroment->terminateConnection(_conn);
				return true;
			}
			catch (SQLException &sqlExcp)
			{
				try
				{
					_conn->rollback();
				}
				catch(SQLException &sqlExcp1)
				{

				}
				cout << __FILE__ << '\t'<< __FUNCTION__ << '\t' << __LINE__ << '\n' << endl;
			   sqlExcp.getErrorCode();
			   string strinfo=sqlExcp.getMessage();
			   cout<<strinfo;
			}
		}
	}
	_conn->terminateStatement(_stmt);
	OraEnviroment->terminateConnection(_conn);
	printf("查询设备参数失败,cross_id= %d\n",CrossID);
	return false;
}


/*
 * 查询设备系统参数
 */
int Request_DeviceSystemParam_0x04(int sockfd,struct sockaddr_in DeviceAddr,unsigned char * send_data)
{
	unsigned char Rec_buf[SingleRecvMaxLen];
	int index =GetDeviceIndex(send_data);

	Connection *_conn = NULL;
	Statement *_stmt = NULL;
	int CrossID = ( send_data[7] << 8 ) | send_data[8];
	char sqlbuf[500];
	int recv_len;
	if(GetConnectFromPool(&_conn,&_stmt) == false)
	{
		printf("查询设备系统信息,连接数据库失败,cross_id = %d\n",CrossID);
		return false;
	}
	_stmt->setAutoCommit(true);
	sendto(sockfd, send_data, ((( send_data[1] << 8 ) | send_data[2])+3), 0, (struct sockaddr *)&DeviceAddr, sizeof(struct sockaddr));
	recv_len = ReceiveDeviceReply( sockfd, DeviceAddr, Rec_buf, 10);
	if(recv_len>0)
	{
		sprintf(device[index].ip,"%d.%d.%d.%d",Rec_buf[13],Rec_buf[14],Rec_buf[15],Rec_buf[16]);
		sprintf(device[index].mask,"%d.%d.%d.%d",Rec_buf[17],Rec_buf[18],Rec_buf[19],Rec_buf[20]);
		sprintf(device[index].gateway,"%d.%d.%d.%d",Rec_buf[21],Rec_buf[22],Rec_buf[23],Rec_buf[24]);
		sprintf(device[index].center_ip,"%d.%d.%d.%d",Rec_buf[25],Rec_buf[26],Rec_buf[27],Rec_buf[28]);
		device[index].center_port = (Rec_buf[29] << 8 |Rec_buf[30]);
		device[index].control_uart = (Rec_buf[31] << 8 |Rec_buf[32]);
		device[index].wireless_uart = (Rec_buf[33] << 8 |Rec_buf[34]);
		try
		{
			sprintf(sqlbuf,"update SYSTEM_CONFIG set INTERSECTION_IP = '%s',"
					"NETMASK = '%s',GATEWAY_ADDRESS = '%s',FORWARD_SERVER_IP = '%s',"
					"FORWARD_SERVER_PORT = '%d',CONTROL_COM_NAME = '%d',READER_COM_NAME = '%d' where intersection_id ='%d')",
					device[index].ip,device[index].mask,device[index].gateway,device[index].center_ip,
					device[index].center_port,device[index].control_uart,device[index].wireless_uart,CrossID);
			_stmt->execute(sqlbuf);
			_conn->terminateStatement(_stmt);
			OraEnviroment->terminateConnection(_conn);
			return true;
		}
		catch (SQLException &sqlExcp)
		{
			cout << __FILE__ << '\t'<< __FUNCTION__ << '\t' << __LINE__ << '\n' << endl;
		   sqlExcp.getErrorCode();
		   string strinfo=sqlExcp.getMessage();
		   cout<<strinfo;
		}
	}
	_conn->terminateStatement(_stmt);
	OraEnviroment->terminateConnection(_conn);
	printf("查询设备系统信息失败,cross_id = %d\n",CrossID);
	return false;
}


/*
 * 查询优先策略
 */
int Request_DeviceStrategy_0x05(int sockfd,struct sockaddr_in DeviceAddr,unsigned char * send_data)
{
	unsigned char Rec_buf[SingleRecvMaxLen];

	Connection *_conn = NULL;
	Statement *_stmt = NULL;
	struct Device_Strategy Strategy[128];
	int CrossID = ( send_data[7] << 8 ) | send_data[8];
	char sqlbuf[500];
	int recv_len,i;
	int j = 0;				//结构体下标

	if(GetConnectFromPool(&_conn,&_stmt) == false)
	{
		printf("查询设备策略时间表,连接数据库失败,cross_id= %d\n",CrossID);
		return false;
	}
	_stmt->setAutoCommit(false);        //设置为手动提交
	sendto(sockfd, send_data, ((( send_data[1] << 8 ) | send_data[2])+3), 0, (struct sockaddr *)&DeviceAddr, sizeof(struct sockaddr));
	recv_len = ReceiveDeviceReply( sockfd, DeviceAddr, Rec_buf, 10);
	i = 13; 						//数据开始的下标

	int data_num,dir_num;//  数据编号  方向序号
	int data_count =Rec_buf[11], dir_count;
	if(recv_len>0)
	{
		for(data_num = 1;data_num<= data_count;data_num++)
		{
			if(data_num != Rec_buf[i])
				goto _error;
			i++;
			dir_count = Rec_buf[i];
			i++;
			for(dir_num = 1; dir_num < dir_count;dir_num++)
			{
				if(dir_num != Rec_buf[i])
					goto _error;
				i++;
				Strategy[j].strategy_id = data_num;
				Strategy[j].sequence_num = dir_num;
				Strategy[j].direction = Rec_buf[i++];
				Strategy[j].level = Rec_buf[i++];
				Strategy[j].param1 = (Rec_buf[i] << 8 |Rec_buf[i+1]);
				i +=2;
				Strategy[j].param2 = (Rec_buf[i] << 8 |Rec_buf[i+1]);
				i +=2;
				Strategy[j].param3 = (Rec_buf[i] << 8 |Rec_buf[i+1]);
				i +=2;
				Strategy[j].param4 = (Rec_buf[i] << 8 |Rec_buf[i+1]);
				i +=2;
				Strategy[j].threshold = Rec_buf[i++];
				Strategy[j].interval = Rec_buf[i++];
				Strategy[j].max_time_allowed = Rec_buf[i++];
				Strategy[j].card_live_time = (Rec_buf[i] << 8 |Rec_buf[i+1]);
				i +=2;
				j++;
			}
		}
		if(j > 0)         //解析出有效的数据
		{
			try
			{
				/******更新优先策略表******/
				sprintf(sqlbuf,"delete from STRATEGY_CONFIG where INTERSECTION_ID = '%d'",CrossID);
				_stmt->execute(sqlbuf);
				sprintf(sqlbuf,"insert into STRATEGY_CONFIG(INTERSECTION_ID,STRATEGY_ID,UPDATE_TIME) values(:x1,:x2,sysdate)");
				_stmt->setSQL(sqlbuf);
				_stmt->setMaxIterations(data_num);
				_stmt->setMaxParamSize(1,10);
				_stmt->setMaxParamSize(2,sizeof(int));
				int temp_t;
				char temp_buf[10];
				for(temp_t = 1; temp_t <= data_num; temp_t++)
				{
					if(temp_t != 0)
							_stmt->addIteration();
					sprintf(temp_buf,"%d",CrossID);
					_stmt->setString(1,temp_buf);
					_stmt->setInt(2,temp_t);
				}
				_stmt->executeUpdate();

				/******更新优先策略详细表******/
				sprintf(sqlbuf,"delete from STRATEGY_CONFIG_INFO where INTERSECTION_ID = '%d'",CrossID);
				_stmt->execute(sqlbuf);
				sprintf(sqlbuf,"insert into STRATEGY_CONFIG_INFO(INTERSECTION_ID,STRATEGY_ID,SEQUENCE_NUMBER,PRIORITY_DIRECTION,"
						"PRIORITY_LEVEL,PARAMETER1,PARAMETER2,PARAMETER3,PARAMETER4,THRESHOLD,PROCESS_INTERVAL,MAX_TIME_ALLOWED,CARD_LIVE_TIME) "
						"values(:x1,:x2,:x3,:x4,:x5,:x6,:x7,:x8,:x9,:x10,:x11,:x12,:x13)");
				_stmt->setSQL(sqlbuf);
				_stmt->setMaxIterations(j);
				_stmt->setMaxParamSize(1,10);
				_stmt->setMaxParamSize(2,sizeof(int));
				_stmt->setMaxParamSize(3,sizeof(int));
				_stmt->setMaxParamSize(4,sizeof(int));
				_stmt->setMaxParamSize(5,sizeof(int));
				_stmt->setMaxParamSize(6,sizeof(int));
				_stmt->setMaxParamSize(7,sizeof(int));
				_stmt->setMaxParamSize(8,sizeof(int));
				_stmt->setMaxParamSize(9,sizeof(int));
				_stmt->setMaxParamSize(10,sizeof(int));
				_stmt->setMaxParamSize(11,sizeof(int));
				_stmt->setMaxParamSize(12,sizeof(int));
				_stmt->setMaxParamSize(13,sizeof(int));

				for(temp_t = 0;temp_t < j;temp_t++)
				{
					if(temp_t != 0)
						_stmt->addIteration();
					sprintf(temp_buf,"%d",CrossID);
					_stmt->setString(1,temp_buf);
					_stmt->setInt(2,Strategy[j].strategy_id);
					_stmt->setInt(3,Strategy[j].sequence_num);
					_stmt->setInt(4,Strategy[j].direction);
					_stmt->setInt(5,Strategy[j].level);
					_stmt->setInt(6,Strategy[j].param1);
					_stmt->setInt(7,Strategy[j].param2);
					_stmt->setInt(8,Strategy[j].param3);
					_stmt->setInt(9,Strategy[j].param4);
					_stmt->setInt(10,Strategy[j].threshold);
					_stmt->setInt(11,Strategy[j].interval);
					_stmt->setInt(12,Strategy[j].max_time_allowed);
					_stmt->setInt(13,Strategy[j].card_live_time);
				}
				_stmt->executeUpdate();
				_conn->commit();
				_conn->terminateStatement(_stmt);
				OraEnviroment->terminateConnection(_conn);
				return true;
			}
			catch (SQLException &sqlExcp)
			{
				try
				{
					_conn->rollback();
				}
				catch (SQLException &sqlExcp1)
				{

				}
				cout << __FILE__ << '\t'<< __FUNCTION__ << '\t' << __LINE__ << '\n' << endl;
			   sqlExcp.getErrorCode();
			   string strinfo=sqlExcp.getMessage();
			   cout<<strinfo;
			}
		}
	}
_error:
	_conn->terminateStatement(_stmt);
	OraEnviroment->terminateConnection(_conn);
	printf("查询设备参数失败,cross_id= %d",CrossID);
	return false;
}


/*
 * 查询设备策略时间表
 */
int Request_DeviceStrategyTime_0x06(int sockfd,struct sockaddr_in DeviceAddr,unsigned char * send_data)
{
	unsigned char Rec_buf[SingleRecvMaxLen];

	Connection *_conn = NULL;
	Statement *_stmt = NULL;
	struct Device_StrategyTime StrategyTime[128];
	int CrossID = ( send_data[7] << 8 ) | send_data[8];
	char sqlbuf[500];
	int recv_len,i;
	int j = 0;				//结构体下标

	if(GetConnectFromPool(&_conn,&_stmt) == false)
	{
		printf("查询设备策略时间表,连接数据库失败,cross_id= %d\n",CrossID);
		return false;
	}
	_stmt->setAutoCommit(false);        //设置为手动提交
	sendto(sockfd, send_data, ((( send_data[1] << 8 ) | send_data[2])+2), 0, (struct sockaddr *)&DeviceAddr, sizeof(struct sockaddr));
	recv_len = ReceiveDeviceReply( sockfd, DeviceAddr, Rec_buf, 10);
	i = 13; 						//数据开始的下标

	int timetable_num,time_num;//  时间表编号  时间表中时间序号
	int timetable_count =Rec_buf[11], time_count;			//  时间表数量   时间表中时间段的数量
	if(recv_len>0)
	{
		for(timetable_num = 1;timetable_num<= timetable_count;timetable_num++)
		{
			if(timetable_num != Rec_buf[i])
				goto _error;
			i++;
			time_count = Rec_buf[i];
			i++;
			for(time_num = 1; time_num < time_count;time_num++)
			{
				if(time_num != Rec_buf[i])
					goto _error;
				i++;
				StrategyTime[j].time_table_id = timetable_num;
				StrategyTime[j].time_id = time_num;
				StrategyTime[j].start_time[0]= Rec_buf[i++];
				StrategyTime[j].start_time[1]= Rec_buf[i++];
				StrategyTime[j].end_time[0]= Rec_buf[i++];
				StrategyTime[j].end_time[1]= Rec_buf[i++];
				StrategyTime[j].strategy_id = Rec_buf[i++];
				j++;
			}
		}
		if(j > 0)         //解析出有效的数据
		{
			try
			{
				/******更新优先策略时间表******/
				sprintf(sqlbuf,"delete from TIME_CONFIG where INTERSECTION_ID = '%d'",CrossID);
				_stmt->execute(sqlbuf);
				sprintf(sqlbuf,"insert into TIME_CONFIG(INTERSECTION_ID,TIME_ID,UPDATE_TIME) values(:x1,:x2,sysdate)");
				_stmt->setSQL(sqlbuf);
				_stmt->setMaxIterations(timetable_count);
				_stmt->setMaxParamSize(1,10);
				_stmt->setMaxParamSize(2,sizeof(int));
				int temp_t;
				char temp_buf[10];
				for(temp_t = 1; temp_t <= timetable_count; temp_t++)
				{
					if(temp_t != 0)
						_stmt->addIteration();
					sprintf(temp_buf,"%d",CrossID);
					_stmt->setString(1,temp_buf);
					_stmt->setInt(2,temp_t);
				}
				_stmt->executeUpdate();
				/********更新优先策略时间配置详细表***************************/
				sprintf(sqlbuf,"delete from TIME_CONFIG_INFO where INTERSECTION_ID = '%d'",CrossID);
				_stmt->execute(sqlbuf);
				sprintf(sqlbuf,"insert into TIME_CONFIG_INFO(INTERSECTION_ID,TIME_ID,START_TIME,END_TIME,STRATEGY_ID) "
						"values(:x1,:x2,:x3,:x4,:x5)");
				_stmt->setSQL(sqlbuf);
				_stmt->setMaxIterations(j);
				_stmt->setMaxParamSize(1,10);
				_stmt->setMaxParamSize(2,sizeof(int));
				_stmt->setMaxParamSize(3,10);
				_stmt->setMaxParamSize(4,10);
				_stmt->setMaxParamSize(5,sizeof(int));

				for(temp_t = 0; temp_t < j; temp_t++)
				{
					if(temp_t != 0)
						_stmt->addIteration();
					sprintf(temp_buf,"%d",CrossID);
					_stmt->setString(1,temp_buf);
					_stmt->setInt(2,StrategyTime[temp_t].time_table_id);
					sprintf(temp_buf,"%d:%d",StrategyTime[temp_t].start_time[0],StrategyTime[temp_t].start_time[1]);
					_stmt->setString(3,temp_buf);
					sprintf(temp_buf,"%d:%d",StrategyTime[temp_t].end_time[0],StrategyTime[temp_t].end_time[1]);
					_stmt->setString(4,temp_buf);
					_stmt->setInt(5,StrategyTime[temp_t].strategy_id);
				}
				_stmt->executeUpdate();
				_conn->commit();
			}
			catch (SQLException &sqlExcp)
			{
				try
				{
					_conn->rollback();
				}
				catch (SQLException &sqlExcp1)
				{

				}
				cout << __FILE__ << '\t'<< __FUNCTION__ << '\t' << __LINE__ << '\n' << endl;
			   sqlExcp.getErrorCode();
			   string strinfo=sqlExcp.getMessage();
			   cout<<strinfo;
			   goto _error;
			}
		}
		_conn->terminateStatement(_stmt);
		OraEnviroment->terminateConnection(_conn);
		return true;
	}
_error:
	_conn->terminateStatement(_stmt);
	OraEnviroment->terminateConnection(_conn);
	printf("查询设备策略时间表失败,cross_id= %d",CrossID);
	return false;
}

/*
 * 查询设备调度表
 */
int Request_DeviceSchedule_0x07(int sockfd,struct sockaddr_in DeviceAddr,unsigned char * send_data)
{
	unsigned char Rec_buf[SingleRecvMaxLen];
	struct Device_Schedule schedule[128];
	Connection *_conn = NULL;
	Statement *_stmt = NULL;
	int CrossID = ( send_data[7] << 8 ) | send_data[8];
	char sqlbuf[500];
	int recv_len,i;
	int j = 0;				//结构体下标

	if(GetConnectFromPool(&_conn,&_stmt) == false)
	{
		printf("查询设备调度表,连接数据库失败,cross_id= %d\n",CrossID);
		return false;
	}
	_stmt->setAutoCommit(false);
	sendto(sockfd, send_data, ((( send_data[1] << 8 ) | send_data[2])+2), 0, (struct sockaddr *)&DeviceAddr, sizeof(struct sockaddr));
	recv_len = ReceiveDeviceReply( sockfd, DeviceAddr, Rec_buf, TimeOut_S);
	i = 13; 						//数据开始的下标

	if(recv_len>0)
	{
		int data_count = Rec_buf[11];
		for(j = 0;j<data_count;j++)
		{
			if(Rec_buf[i] != j+1)
			{
				goto _error;
			}
			schedule[j].id = Rec_buf[i++];
			schedule[j].type = Rec_buf[i++];
			schedule[j].priority_level = Rec_buf[i++];
			schedule[j].week_value = Rec_buf[i++];
			schedule[j].month_value = (Rec_buf[i] << 8 | Rec_buf[i+1]);
			i = i+2;
			schedule[j].day_value = ( (Rec_buf[i] << 24) | (Rec_buf[i+1] << 16) | (Rec_buf[i+2] << 8) | Rec_buf[i+3]);
			i = i+4;
			schedule[j].time_table_id = Rec_buf[i++];
		}
		if(j > 0)         //解析出有效的数据
		{
			try
			{
				sprintf(sqlbuf,"delete from DISPATCH_INFO where INTERSECTION_ID = '%d'",CrossID);
				_stmt->execute(sqlbuf);
				sprintf(sqlbuf,"insert into DISPATCH_INFO(INTERSECTION_ID,DISPATCH_ID,DISPATCH_TYPE,DISPATCH_PRIORITY,WEEK_VALUE,MONTH_VALUE,DAY_VALUE,TIME_ID) "
						"values(:x1,:x2,:x3,:x4,:x5,:x6,:x7,:x8)");
				_stmt->setSQL(sqlbuf);
				_stmt->setMaxIterations(j);
				_stmt->setMaxParamSize(1,10);
				_stmt->setMaxParamSize(2,sizeof(int));
				_stmt->setMaxParamSize(3,sizeof(int));
				_stmt->setMaxParamSize(4,sizeof(int));
				_stmt->setMaxParamSize(5,500);
				_stmt->setMaxParamSize(6,500);
				_stmt->setMaxParamSize(7,500);
				_stmt->setMaxParamSize(8,sizeof(int));
				int temp_t;
				char temp_buf[500],temp_value[100];
				int temp_i;
				for(temp_t = 0;temp_t < j;temp_t++)
				{
					if(temp_t != 0)
						_stmt->addIteration();
					sprintf(temp_buf,"%d",CrossID);
					_stmt->setString(1,temp_buf);
					_stmt->setInt(2,schedule[j].id);
					_stmt->setInt(3,schedule[j].type);
					_stmt->setInt(4,schedule[j].priority_level);
					for(temp_i = 0;temp_i < 7;temp_i++)
					{
						if(schedule[j].week_value & (0x01 <<temp_i))
						{
							sprintf(temp_buf,"%d,",temp_i+1);
							strcat(temp_value,temp_buf);
						}
					}
					_stmt->setString(5,temp_value);
					memset(temp_value,'\0',sizeof(temp_value));
					for(temp_i = 0;temp_i < 12;temp_i++)
					{
						if(schedule[j].month_value & (0x01 <<temp_i))
						{
							sprintf(temp_buf,"%d,",temp_i+1);
							strcat(temp_value,temp_buf);
						}
					}
					_stmt->setString(6,temp_value);
					memset(temp_value,'\0',sizeof(temp_value));
					for(temp_i = 0;temp_i < 31;temp_i++)
					{
						if(schedule[j].day_value & (0x01 <<temp_i))
						{
							sprintf(temp_buf,"%d,",temp_i+1);
							strcat(temp_value,temp_buf);
						}
					}
					_stmt->setString(7,temp_value);
					_stmt->setInt(8,schedule[j].time_table_id);
				}
				_stmt->executeUpdate();
				_conn->commit();
				_conn->terminateStatement(_stmt);
				OraEnviroment->terminateConnection(_conn);
				return true;
			}
			catch (SQLException &sqlExcp)
			{
				try
				{
					_conn->rollback();
				}
				catch(SQLException &sqlExcp1)
				{

				}
				cout << __FILE__ << '\t'<< __FUNCTION__ << '\t' << __LINE__ << '\n' << endl;
			   sqlExcp.getErrorCode();
			   string strinfo=sqlExcp.getMessage();
			   cout<<strinfo;
			}
		}
	}
_error:
	_conn->terminateStatement(_stmt);
	OraEnviroment->terminateConnection(_conn);
	printf("查询设备调度表失败,cross_id= %d\n",CrossID);
	return false;
}

/*
 * 查询车辆信息表
 */
int Request_DeviceCard_0x08(int sockfd,struct sockaddr_in DeviceAddr,unsigned char * send_data)
{
	unsigned char Rec_buf[SingleRecvMaxLen];
	struct Device_CardInfo card[CardMaxNum];
	Connection *_conn = NULL;
	Statement *_stmt = NULL;
	int CrossID = ( send_data[7] << 8 ) | send_data[8];
	char sqlbuf[500];
	int recv_len,i;
	int j = 0;				//结构体下标

	if(GetConnectFromPool(&_conn,&_stmt) == false)
	{
		printf("查询车辆信息表,连接数据库失败,cross_id= %d\n",CrossID);
		return false;
	}
	_stmt->setAutoCommit(false);
	sendto(sockfd, send_data, ((( send_data[1] << 8 ) | send_data[2])+2), 0, (struct sockaddr *)&DeviceAddr, sizeof(struct sockaddr));
	int CurrentNum = 0,TotalNum;
	int temp_len;
_receive_loop:
	recv_len = ReceiveDeviceReply( sockfd, DeviceAddr, Rec_buf, TimeOut_S);
	i = 13; 						//数据开始的下标
	if(recv_len>0)
	{
		//当不是第一包报文是，报文中的总条数和原来的值不一样
		if( (CurrentNum != 0 ) && (TotalNum != (Rec_buf[i] << 8 | Rec_buf[i+1]) ))
		{
			goto _error;
		}

		TotalNum = (Rec_buf[i] << 8 | Rec_buf[i+1]);
		i = i+2;
		//报文序号错误
		if((CurrentNum +1) != (Rec_buf[i] << 8 | Rec_buf[i+1]))
		{
			goto _error;
		}
		CurrentNum = (Rec_buf[i] << 8 | Rec_buf[i+1]);
		if(CurrentNum > TotalNum)
		{
			goto _error;
		}
		i = i+2;
		card[j].RFID = ( (Rec_buf[i] << 24) | (Rec_buf[i+1] << 16) | (Rec_buf[i+2] << 8) | Rec_buf[i+3]);
		i = i+4;
		temp_len = Rec_buf[i++];
		memset(card[j].plate_num,'\0',sizeof(card[j].plate_num));
		memcpy(card[j].plate_num,Rec_buf,temp_len);
		i = i+temp_len;
		card[j].plate_color = Rec_buf[i++];
		card[j].threshold = Rec_buf[i++];
		card[j].install_time = ( (Rec_buf[i] << 24) | (Rec_buf[i+1] << 16) | (Rec_buf[i+2] << 8) | Rec_buf[i+3]);
		i = i+4;

		temp_len = Rec_buf[i++];
		memset(card[j].line_num,'\0',sizeof(card[j].line_num));
		memcpy(card[j].line_num,Rec_buf,temp_len);
		i = i+temp_len;

		temp_len = Rec_buf[i++];
		memset(card[j].company,'\0',sizeof(card[j].company));
		memcpy(card[j].company,Rec_buf,temp_len);

		j++;

		 //只有当前序号和总包数相等时才可更新数据库
		if(CurrentNum < TotalNum)
		{
			goto _receive_loop;
		}
	}
	if(j > 0)         //解析出有效的数据
	{
		try
		{
			sprintf(sqlbuf,"delete from BUSP_CARD where INTERSECTION_ID = '%d'",CrossID);
			_stmt->execute(sqlbuf);
			sprintf(sqlbuf,"insert into BUSP_CARD(CARD_ID ,CARD_TYPE,BUS_CLASS,BUS_PLATE,PLATE_COLOR,INSTALL_TIME,COMPANY,CAR_PRIORITY) "
					"values(:x1,:x2,:x3,:x4,:x5,to_date(:x6,'yyyy-mm-dd hh24:mi:ss'),:x7,:x8)");
			_stmt->setSQL(sqlbuf);
			_stmt->setMaxIterations(j);
			_stmt->setMaxParamSize(1,20);
			_stmt->setMaxParamSize(2,sizeof(int));
			_stmt->setMaxParamSize(3,100);
			_stmt->setMaxParamSize(4,100);
			_stmt->setMaxParamSize(5,sizeof(int));
			_stmt->setMaxParamSize(6,30);
			_stmt->setMaxParamSize(7,100);
			_stmt->setMaxParamSize(8,sizeof(int));
			int temp_t;
			char temp_buf[100];

			struct tm * time_install;
			time_t install_t;
			for(temp_t = 0;temp_t < j;temp_t++)
			{
				if(temp_t != 0)
					_stmt->addIteration();
				_stmt->setNumber(1,card[temp_t].RFID);
				_stmt->setInt(2,1);
				_stmt->setString(3,(char *)card[j].line_num);
				_stmt->setString(4,(char *)card[j].plate_num);
				_stmt->setInt(5,card[j].plate_color);
				install_t = card[j].install_time;
				time_install = localtime(&install_t);
				sprintf(temp_buf,"%d-%d-%d %02d:%02d:%02d",
						time_install->tm_year+1900,time_install->tm_mon+1,time_install->tm_mday,time_install->tm_hour,time_install->tm_min,time_install->tm_sec);
				_stmt->setString(6,temp_buf);
				_stmt->setString(7,(char *)card[j].company);
				_stmt->setInt(8,card[j].threshold);
			}
			_stmt->executeUpdate();
			_conn->commit();
			_conn->terminateStatement(_stmt);
			OraEnviroment->terminateConnection(_conn);
			return true;
		}
		catch (SQLException &sqlExcp)
		{
			try
			{
				_conn->rollback();
			}
			catch(SQLException &sqlExcp1)
			{

			}
			cout << __FILE__ << '\t'<< __FUNCTION__ << '\t' << __LINE__ << '\n' << endl;
		   sqlExcp.getErrorCode();
		   string strinfo=sqlExcp.getMessage();
		   cout<<strinfo;
		}
	}
_error:
	_conn->terminateStatement(_stmt);
	OraEnviroment->terminateConnection(_conn);
	printf("查询车辆信息表失败,cross_id= %d\n",CrossID);
	return false;
}
/*
 * 设置设备参数
 */
int Set_DeviceParam_0x03(int sockfd,struct sockaddr_in DeviceAddr,unsigned char * web_data)
{
	unsigned char Rec_buf[SingleRecvMaxLen];
	unsigned char send_buf[SingleRecvMaxLen];
	struct Device_CardReaderInfo cardreader[8];
	Connection *_conn = NULL;
	Statement *_stmt = NULL;
	ResultSet *Result;
	int CrossID = ( web_data[7] << 8 ) | web_data[8];
	char sqlbuf[500];
	int recv_len,i;
	int j = 0;				//结构体下标

	if(GetConnectFromPool(&_conn,&_stmt) == false)
	{
		printf("设置设备参数,连接数据库失败,cross_id= %d\n",CrossID);
		return false;
	}
	_stmt->setAutoCommit(true);
	//select count(*) from DEVICE_CONFIG t  where t.intersection_id = '10'
	try
	{
		sprintf(sqlbuf,"select READER_POWER,READER_BLOCK,BUS_PRIORITY_SWITCH_NUMBER,EMERGENCY_SWITCH_NUMBER,ZIGBEE_ADDRESS,READER_ID,DIRECTION_ID from DEVICE_CONFIG where INTERSECTION_ID = '%d'",CrossID );
		Result = _stmt->executeQuery(sqlbuf);
		while(Result->next() != 0)
		{
			cardreader[j].dBm = Result->getInt(1);
			cardreader[j].detection_range = Result->getInt(2);
			cardreader[j].transport_priority_relay = Result->getInt(3);
			cardreader[j].emergency_priority_relay = Result->getInt(4);
			cardreader[j].zigbee_addr = Result->getInt(5);
			cardreader[j].id = Result->getInt(6);
			cardreader[j].direction = Result->getInt(7);
			j++;
		}
		_stmt->closeResultSet(Result);
	}
	catch(SQLException &sqlExcp)
	{
		cout << __FILE__ << '\t'<< __FUNCTION__ << '\t' << __LINE__ << '\n' << endl;
		sqlExcp.getErrorCode();
		cout << sqlExcp.getMessage();
		goto _error;
	}
	if(j == 0)
		goto _error;
	memset(send_buf,'\0',SingleRecvMaxLen);
	memcpy(send_buf,web_data,11);
	send_buf[11] = (j & 0xFF);
	i = 12;
	int temp_i;
	for(temp_i = 0; temp_i < j; temp_i++)
	{
		send_buf[i++] = temp_i+1;
		send_buf[i++] = (cardreader[j].id & 0xFF);
		send_buf[i++] = (cardreader[j].direction & 0xFF);
		send_buf[i++] = (cardreader[j].dBm & 0xFF00) >> 8;
		send_buf[i++] = (cardreader[j].dBm & 0xFF);
		send_buf[i++] = (cardreader[j].zigbee_addr & 0xFF00) >> 8;
		send_buf[i++] = (cardreader[j].zigbee_addr & 0xFF);
		send_buf[i++] = (cardreader[j].transport_priority_relay & 0xFF00) >> 8;
		send_buf[i++] = (cardreader[j].transport_priority_relay & 0xFF);
		send_buf[i++] = (cardreader[j].emergency_priority_relay & 0xFF00) >> 8;
		send_buf[i++] = (cardreader[j].emergency_priority_relay & 0xFF);
		send_buf[i++] = (cardreader[j].detection_range & 0xFF00) >> 8;
		send_buf[i++] = (cardreader[j].detection_range & 0xFF);
	}
	send_buf[1] = (0xFF & i) >> 8;
	send_buf[2] = (0XFF & i);
	send_buf[i++] = makecheck(send_buf);
	send_buf[i++] = 0x7D;
	sendto(sockfd, send_buf, i, 0, (struct sockaddr *)&DeviceAddr, sizeof(struct sockaddr));
	recv_len = ReceiveDeviceReply( sockfd, DeviceAddr, Rec_buf, TimeOut_S);
	if( (recv_len>0 ) && (Rec_buf[12] == 0x00) )
	{
		_conn->terminateStatement(_stmt);
		OraEnviroment->terminateConnection(_conn);
		return true;
	}
_error:
	_conn->terminateStatement(_stmt);
	OraEnviroment->terminateConnection(_conn);
	return false;
}

/*
 * 设置设备系统参数
 */
int Set_DeviceSystemParam_0x04(int sockfd,struct sockaddr_in DeviceAddr,unsigned char * web_data)
{
	unsigned char Rec_buf[SingleRecvMaxLen];
	unsigned char send_buf[SingleRecvMaxLen];
	Connection *_conn = NULL;
	Statement *_stmt = NULL;
	ResultSet *Result;
	int CrossID = ( web_data[7] << 8 ) | web_data[8];
	char sqlbuf[500];
	int recv_len,i;

	if(GetConnectFromPool(&_conn,&_stmt) == false)
	{
		printf("设置设备系统参数,连接数据库失败,cross_id= %d\n",CrossID);
		return false;
	}
	_stmt->setAutoCommit(true);
	sprintf(sqlbuf,"select INTERSECTION_IP,NETMASK ,GATEWAY_ADDRESS ,FORWARD_SERVER_IP ,FORWARD_SERVER_PORT ,"
			"CONTROL_COM_NAME ,READER_COM_NAME  from SYSTEM_CONFIG where intersection_id ='%d'",CrossID);
	memcpy(send_buf,web_data,11);
	send_buf[11] = 0;
	i = 12;
	int temp_j;
	char temp_buf[50];
	try
	{
		Result = _stmt->executeQuery(sqlbuf);
		char *result;
		if(Result->next() != 0)
		{



			memset(temp_buf,'\0',50);
			sprintf(temp_buf,Result->getString(1).c_str());
			result = strtok( temp_buf, "." );
			temp_j = 0;
			while( result != NULL )
			{
				send_buf[i+temp_j] = atoi(result);
				result = strtok( NULL, "." );
				temp_j++;
				if(temp_j >= 4)
				{
					break;
				}
			}
			i = i+4;

			memset(temp_buf,'\0',50);
			sprintf(temp_buf,Result->getString(2).c_str());
			result = strtok( temp_buf, "." );
			temp_j = 0;
			while( result != NULL )
			{
				send_buf[i+temp_j] = atoi(result);
				result = strtok( NULL, "." );
				temp_j++;
				if(temp_j >= 4)
				{
					break;
				}
			}
			i = i+4;

			memset(temp_buf,'\0',50);
			sprintf(temp_buf,Result->getString(3).c_str());
			result = strtok( temp_buf, "." );
			temp_j = 0;
			while( result != NULL )
			{
				send_buf[i+temp_j] = atoi(result);
				result = strtok( NULL, "." );
				temp_j++;
				if(temp_j >= 4)
				{
					break;
				}
			}
			i = i+4;

			memset(temp_buf,'\0',50);
			sprintf(temp_buf,Result->getString(4).c_str());
			result = strtok( temp_buf, "." );
			temp_j = 0;
			while( result != NULL )
			{
				send_buf[i+temp_j] = atoi(result);
				result = strtok( NULL, "." );
				temp_j++;
				if(temp_j >= 4)
				{
					break;
				}
			}
			i = i+4;
			temp_j = Result->getInt(5);
			send_buf[i++] = (temp_j & 0xFF00)>>8;
			send_buf[i++] = temp_j & 0xFF;
			temp_j = Result->getInt(6);
			send_buf[i++] = (temp_j & 0xFF00)>>8;
			send_buf[i++] = temp_j & 0xFF;
			temp_j = Result->getInt(7);
			send_buf[i++] = (temp_j & 0xFF00)>>8;
			send_buf[i++] = temp_j & 0xFF;
		}
		else
		{
			goto _error;
		}
		_stmt->closeResultSet(Result);
	}
	catch(SQLException &sqlExcp)
	{
		cout << __FILE__ << '\t'<< __FUNCTION__ << '\t' << __LINE__ << '\n' << endl;
		sqlExcp.getErrorCode();
		cout << sqlExcp.getMessage();
		goto _error;
	}
	send_buf[1] = (i & 0xFF00)>>8;
	send_buf[2] = (i & 0xFF);
	send_buf[i++] = makecheck(send_buf);
	send_buf[i++] = 0x7D;
	sendto(sockfd, send_buf, i, 0, (struct sockaddr *)&DeviceAddr, sizeof(struct sockaddr));
	recv_len = ReceiveDeviceReply( sockfd, DeviceAddr, Rec_buf, TimeOut_S);
	if( (recv_len>0 ) && (Rec_buf[12] == 0x00) )
	{
		InitDeviceInfo();
		_conn->terminateStatement(_stmt);
		OraEnviroment->terminateConnection(_conn);
		return true;
	}
_error:
	_conn->terminateStatement(_stmt);
	OraEnviroment->terminateConnection(_conn);
	return false;

}

/*
 * 设置优先策略
 */
int Set_DeviceStrategy_0x05(int sockfd,struct sockaddr_in DeviceAddr,unsigned char * web_data)
{
	unsigned char Rec_buf[SingleRecvMaxLen];
	unsigned char send_buf[SingleRecvMaxLen];
	struct Device_Strategy Strategy[128];
	Connection *_conn = NULL;
	Statement *_stmt = NULL;
	ResultSet *Result;
	int CrossID = ( web_data[7] << 8 ) | web_data[8];
	char sqlbuf[500];
	int recv_len,i;
	int j = 0;				//结构体下标
	char strategy_id = 0;
	if(GetConnectFromPool(&_conn,&_stmt) == false)
	{
		printf("设置优先策略,连接数据库失败,cross_id= %d\n",CrossID);
		return false;
	}
	try
	{
		_stmt->setAutoCommit(true);
		sprintf(sqlbuf,"select STRATEGY_ID,SEQUENCE_NUMBER,PRIORITY_DIRECTION,PRIORITY_LEVEL,PARAMETER1,"
				"PARAMETER2,PARAMETER3,PARAMETER4,THRESHOLD,PROCESS_INTERVAL,MAX_TIME_ALLOWED,CARD_LIVE_TIME "
				"from STRATEGY_CONFIG_INFO t  where t.INTERSECTION_ID = '%d' order by STRATEGY_ID and SEQUENCE_NUMBER",CrossID);
		Result = _stmt->executeQuery(sqlbuf);
		while(Result->next() != 0)
		{
			Strategy[j].strategy_id = Result->getInt(1);
			Strategy[j].sequence_num = Result->getInt(2);
			Strategy[j].direction = Result->getInt(3);
			Strategy[j].level = Result->getInt(4);
			Strategy[j].param1 = Result->getInt(5);
			Strategy[j].param2 = Result->getInt(6);
			Strategy[j].param3 = Result->getInt(7);
			Strategy[j].param4 = Result->getInt(8);
			Strategy[j].threshold = Result->getInt(9);
			Strategy[j].interval = Result->getInt(10);
			Strategy[j].max_time_allowed = Result->getInt(11);
			Strategy[j].card_live_time = Result->getInt(12);
			j++;
		}
		_stmt->closeResultSet(Result);
	}
	catch(SQLException &sqlExcp)
	{
		cout << __FILE__ << '\t'<< __FUNCTION__ << '\t' << __LINE__ << '\n' << endl;
		sqlExcp.getErrorCode();
		cout << sqlExcp.getMessage();
		goto _error;
	}
	if(j == 0)
		goto _error;
	memset(send_buf,'\0',SingleRecvMaxLen);
	memcpy(send_buf,web_data,11);
	send_buf[11] = 0;
	i = 12;
	int temp_i;
	unsigned char *direction_count;

	for(temp_i = 0;temp_i < j;temp_i++)
	{
		if(Strategy[temp_i].strategy_id != strategy_id )
		{
			send_buf[11] = send_buf[11] + 1;							//策略条数增加
			strategy_id = Strategy[temp_i].strategy_id;			//
			send_buf[i++] = Strategy[temp_i].strategy_id;		//策略编号
			direction_count =  (send_buf + i);                 //记录下方向条数在内存中的位置
			send_buf[i++] = 0;
		}
		*direction_count = (*direction_count) +1;					//方向条数增加  相应的send_buf中的值也增加

		send_buf[i++] = Strategy[temp_i].sequence_num;
		send_buf[i++] = Strategy[temp_i].direction;
		send_buf[i++] = Strategy[temp_i].level;
		send_buf[i++] = ( Strategy[temp_i].param1 & 0xFF00 ) >> 8;
		send_buf[i++] = ( Strategy[temp_i].param1 &0xFF);
		send_buf[i++] = ( Strategy[temp_i].param2 & 0xFF00 ) >> 8;
		send_buf[i++] = ( Strategy[temp_i].param2 &0xFF);
		send_buf[i++] = ( Strategy[temp_i].param3 & 0xFF00 ) >> 8;
		send_buf[i++] = ( Strategy[temp_i].param3 &0xFF);
		send_buf[i++] = ( Strategy[temp_i].param4 & 0xFF00 ) >> 8;
		send_buf[i++] = ( Strategy[temp_i].param4 &0xFF);
		send_buf[i++] = Strategy[temp_i].threshold;
		send_buf[i++] = Strategy[temp_i].interval;
		send_buf[i++] = Strategy[temp_i].max_time_allowed;
		send_buf[i++] = Strategy[temp_i].card_live_time;
	}

	send_buf[1] = (0xFF & i) >> 8;
	send_buf[2] = (0XFF & i);
	send_buf[i++] = makecheck(send_buf);
	send_buf[i++] = 0x7D;
	sendto(sockfd, send_buf, i, 0, (struct sockaddr *)&DeviceAddr, sizeof(struct sockaddr));
	recv_len = ReceiveDeviceReply( sockfd, DeviceAddr, Rec_buf, TimeOut_S);
	if( (recv_len>0 ) && (Rec_buf[12] == 0x00) )
	{
		_conn->terminateStatement(_stmt);
		OraEnviroment->terminateConnection(_conn);
		return true;
	}
_error:
	_conn->terminateStatement(_stmt);
	OraEnviroment->terminateConnection(_conn);
	return false;

}

/*
 * 设置设备策略时间表
 */
int Set_DeviceStrategyTime_0x06(int sockfd,struct sockaddr_in DeviceAddr,unsigned char * web_data)
{
	unsigned char Rec_buf[SingleRecvMaxLen];
	unsigned char send_buf[SingleRecvMaxLen];
	struct Device_StrategyTime StrategyTime[128];
	Connection *_conn = NULL;
	Statement *_stmt = NULL;
	ResultSet *Result;
	int CrossID = ( web_data[7] << 8 ) | web_data[8];
	char sqlbuf[500];
	int recv_len,i;
	int j = 0;				//结构体下标

	int timetable_id=0;
	if(GetConnectFromPool(&_conn,&_stmt) == false)
	{
		printf("设置设备策略时间表,连接数据库失败,cross_id= %d\n",CrossID);
		return false;
	}
	_stmt->setAutoCommit(true);
	try
	{
		sprintf(sqlbuf,"select TIME_ID,TIME_ORDER,START_TIME,END_TIME,STRATEGY_ID from TIME_CONFIG_INFO where INTERSECTION_ID = '%d' order by TIME_ID AND TIME_ORDER",CrossID);
		Result = _stmt->executeQuery(sqlbuf);
		while(Result->next() != 0)
		{
			StrategyTime[j].time_table_id = Result->getInt(1);
			StrategyTime[j].time_id = Result->getInt(2);
			sprintf((char *)StrategyTime[j].start_time,Result->getString(3).c_str());
			sprintf((char *)StrategyTime[j].end_time,Result->getString(4).c_str());
			StrategyTime[j].strategy_id = Result->getInt(5);
			j++;
		}
		_stmt->closeResultSet(Result);
	}
	catch(SQLException &sqlExcp)
	{
		cout << __FILE__ << '\t'<< __FUNCTION__ << '\t' << __LINE__ << '\n' << endl;
		sqlExcp.getErrorCode();
		cout << sqlExcp.getMessage();
		goto _error;
	}
	if(j == 0)
		goto _error;

	int temp_i;
	unsigned char *time_id_count;
	memset(send_buf,'\0',SingleRecvMaxLen);
	memcpy(send_buf,web_data,11);
	i = 12;
	for(temp_i = 0; temp_i < j; temp_i++)
	{
		if(timetable_id != StrategyTime[temp_i].time_table_id)
		{
			send_buf[11] = send_buf[11] +1;				//有多少时间表ID  数据编号就是多少
			timetable_id = StrategyTime[temp_i].time_table_id;			//记录当前数据表ID
			send_buf[i++] = timetable_id;					//设置报文
			time_id_count = (send_buf + i);				//记录报文中时间段数量 在内存中的地址
			send_buf[i] = 0;									//将时间段数量置0
			i++;
		}
		*time_id_count = (*time_id_count) + 1;				//同过time_id_count变化改变报文中时间段数量的值
		send_buf[i++] = (StrategyTime[temp_i].time_id & 0xFF);
		send_buf[i++] = StrategyTime[temp_i].start_time[0];
		send_buf[i++] = StrategyTime[temp_i].start_time[1];
		send_buf[i++] = StrategyTime[temp_i].end_time[0];
		send_buf[i++] = StrategyTime[temp_i].end_time[1];
		send_buf[i++] = StrategyTime[temp_i].strategy_id;

	}
	send_buf[1] = (0xFF & i) >> 8;
	send_buf[2] = (0XFF & i);
	send_buf[i++] = makecheck(send_buf);
	send_buf[i++] = 0x7D;
	sendto(sockfd, send_buf, i, 0, (struct sockaddr *)&DeviceAddr, sizeof(struct sockaddr));
	recv_len = ReceiveDeviceReply( sockfd, DeviceAddr, Rec_buf, TimeOut_S);
	if( (recv_len>0 ) && (Rec_buf[12] == 0x00) )
	{
		_conn->terminateStatement(_stmt);
		OraEnviroment->terminateConnection(_conn);
		return true;
	}
_error:
	_conn->terminateStatement(_stmt);
	OraEnviroment->terminateConnection(_conn);
	return false;
}

/*
 * 设置设备调度表
 */
int Set_DeviceSchedule_0x07(int sockfd,struct sockaddr_in DeviceAddr,unsigned char * web_data)
{
	unsigned char Rec_buf[SingleRecvMaxLen];
	unsigned char send_buf[SingleRecvMaxLen];
	struct Device_Schedule schedule[128];
	Connection *_conn = NULL;
	Statement *_stmt = NULL;
	ResultSet *Result;
	int CrossID = ( web_data[7] << 8 ) | web_data[8];
	char sqlbuf[500];
	int recv_len,i;
	int j = 0;				//结构体下标

	if(GetConnectFromPool(&_conn,&_stmt) == false)
	{
		printf("设置设备调度表,连接数据库失败,cross_id= %d\n",CrossID);
		return false;
	}
	_stmt->setAutoCommit(true);
	char temp_buf[100];
	char *result;
	int temp_j;
	try
	{
		sprintf(sqlbuf,"select DISPATCH_ID,DISPATCH_TYPE,DISPATCH_PRIORITY,WEEK_VALUE,MONTH_VALUE,DAY_VALUE,TIME_ID "
				"from DISPATCH_INFO where INTERSECTION_ID = '%d' order by DISPATCH_ID",CrossID);
		Result = _stmt->executeQuery(sqlbuf);
		while(Result->next() != 0)
		{
			schedule[j].id = Result->getInt(1);
			schedule[j].type = Result->getInt(2);
			schedule[j].priority_level = Result->getInt(3);
			schedule[j].week_value =0;
			memset(temp_buf,'\0',100);
			sprintf(temp_buf,Result->getString(4).c_str());
			result = strtok( temp_buf, "." );
			while( result != NULL )
			{
				temp_j = atoi(result);
				if(temp_j != 0)
				{
					schedule[j].week_value = schedule[j].week_value | (1 << temp_j );
				}
				result = strtok( NULL, "." );
			}
			schedule[j].month_value =0;
			memset(temp_buf,'\0',100);
			sprintf(temp_buf,Result->getString(5).c_str());
			result = strtok( temp_buf, "." );
			while( result != NULL )
			{
				temp_j = atoi(result);
				if(temp_j != 0)
				{
					schedule[j].month_value = schedule[j].month_value | (1 << temp_j );
				}
				result = strtok( NULL, "." );
			}
			schedule[j].day_value =0;
			memset(temp_buf,'\0',100);
			sprintf(temp_buf,Result->getString(6).c_str());
			result = strtok( temp_buf, "." );
			while( result != NULL )
			{
				temp_j = atoi(result);
				if(temp_j != 0)
				{
					schedule[j].day_value = schedule[j].day_value | (1 << temp_j );
				}
				result = strtok( NULL, "." );
			}
			schedule[j].time_table_id = Result->getInt(7);
			j++;
		}
		_stmt->closeResultSet(Result);
	}
	catch(SQLException &sqlExcp)
	{
		cout << __FILE__ << '\t'<< __FUNCTION__ << '\t' << __LINE__ << '\n' << endl;
		sqlExcp.getErrorCode();
		cout << sqlExcp.getMessage();
		goto _error;
	}
	if(j == 0)
		goto _error;

	int temp_i;
	for(temp_i = 0; temp_i < j; temp_i++)
	{

		memset(send_buf,'\0',SingleRecvMaxLen);
		memcpy(send_buf,web_data,11);
		send_buf[11] = (j & 0xFF);
		i = 12;
		send_buf[i++] = schedule[temp_i].id;
		send_buf[i++] = schedule[temp_i].type;
		send_buf[i++] = schedule[temp_i].priority_level;
		send_buf[i++] = schedule[temp_i].week_value;
		send_buf[i++] = (schedule[temp_i].month_value & 0xFF00) >> 8;
		send_buf[i++] = (schedule[temp_i].month_value & 0xFF);
		send_buf[i++] = (schedule[temp_i].day_value & 0xFF000000) >> 24;
		send_buf[i++] = (schedule[temp_i].day_value & 0xFF0000)>>16;
		send_buf[i++] = (schedule[temp_i].day_value & 0xFF00) >> 8;
		send_buf[i++] = (schedule[temp_i].day_value & 0xFF);
		send_buf[i++] = schedule[temp_i].time_table_id;
	}
	send_buf[1] = (0xFF & i) >> 8;
	send_buf[2] = (0XFF & i);
	send_buf[i++] = makecheck(send_buf);
	send_buf[i++] = 0x7D;
	sendto(sockfd, send_buf, i, 0, (struct sockaddr *)&DeviceAddr, sizeof(struct sockaddr));
	recv_len = ReceiveDeviceReply( sockfd, DeviceAddr, Rec_buf, TimeOut_S);
	if( (recv_len>0 ) && (Rec_buf[12] == 0x00) )
	{
		_conn->terminateStatement(_stmt);
		OraEnviroment->terminateConnection(_conn);
		return true;
	}
_error:
	_conn->terminateStatement(_stmt);
	OraEnviroment->terminateConnection(_conn);
	return false;
}

/*
 * 设置车辆信息表
 */
int Set_DeviceCard_0x08(int sockfd,struct sockaddr_in DeviceAddr,unsigned char * web_data)
{
	unsigned char Rec_buf[SingleRecvMaxLen];
	unsigned char send_buf[SingleRecvMaxLen];
	struct Device_CardInfo card[CardMaxNum];
	Connection *_conn = NULL;
	Statement *_stmt = NULL;
	ResultSet *Result;
	int CrossID = ( web_data[7] << 8 ) | web_data[8];
	char sqlbuf[500];
	int recv_len,i;
	int j = 0;				//结构体下标

	if(GetConnectFromPool(&_conn,&_stmt) == false)
	{
		printf("设置设备调度表,连接数据库失败,cross_id= %d\n",CrossID);
		return false;
	}
	_stmt->setAutoCommit(true);
	char temp_buf[100];
	char time_fmt[] = "%Y-%m-%d %H:%M:%S";
	struct tm timeinfo;
	try
	{
		sprintf(sqlbuf,"select CARD_ID, BUS_CLASS, BUS_PLATE, PLATE_COLOR, to_char(INSTALL_TIME,'yyyy-mm-dd hh24:mi:ss'), COMPANY, CAR_PRIORITY from BUSP_CARD");
		Result = _stmt->executeQuery(sqlbuf);
		while(Result->next()!=0)
		{
			card[j].RFID = Result->getNumber(1);
			sprintf((char *)card[j].line_num,Result->getString(2).c_str());
			sprintf((char *)card[j].plate_num,Result->getString(3).c_str());
			card[j].plate_color = Result->getInt(4);
			sprintf(temp_buf,Result->getString(5).c_str());
			if (strptime(temp_buf, time_fmt, &timeinfo) != NULL)
			{
				card[j].install_time = mktime(&timeinfo);

			}
			else
			{
				time((long int *) &(card[j].install_time));
			}
			card[j].install_time = card[j].install_time + (8 * 60 *60);
			sprintf((char *)card[j].company,Result->getString(6).c_str());
			card[j].threshold = Result->getInt(7);
			j++;
		}
		_stmt->closeResultSet(Result);
	}
	catch(SQLException &sqlExcp)
	{
		cout << __FILE__ << '\t'<< __FUNCTION__ << '\t' << __LINE__ << '\n' << endl;
		sqlExcp.getErrorCode();
		cout << sqlExcp.getMessage();
		goto _error;
	}
	if(j == 0)
		goto _error;


	int temp_i,temp_j;
	for(temp_i = 0; temp_i < j; temp_i++)
	{
		i = 12;
		memset(send_buf,'\0',SingleRecvMaxLen);
		memcpy(send_buf,web_data,11);
		send_buf[11] = (temp_i & 0xFF);
		send_buf[i++] = (j & 0x0FF00)>>8;
		send_buf[i++] = (j & 0xFF);
		send_buf[i++] = ((temp_i+1)& 0x0FF00)>>8;
		send_buf[i++] = ((temp_i+1)& 0xFF);
		send_buf[i++] = (card[temp_i].RFID & 0xFF000000) >> 24;
		send_buf[i++] = (card[temp_i].RFID & 0xFF0000)>>16;
		send_buf[i++] = (card[temp_i].RFID & 0xFF00) >> 8;
		send_buf[i++] = (card[temp_i].RFID & 0xFF);
		send_buf[i++] = strlen((char *)card[temp_i].plate_num);
		for(temp_j = 0;temp_j< (int)strlen((char *)card[temp_i].plate_num);temp_j++)
		{
			send_buf[i++] = card[temp_i].plate_num[temp_j];
		}
		send_buf[i++] = card[temp_i].plate_color;
		send_buf[i++] = card[temp_i].threshold;

		send_buf[i++] = (card[temp_i].install_time & 0xFF000000) >> 24;
		send_buf[i++] = (card[temp_i].install_time & 0xFF0000)>>16;
		send_buf[i++] = (card[temp_i].install_time & 0xFF00) >> 8;
		send_buf[i++] = (card[temp_i].install_time & 0xFF);
//		struct tm *temp_time = localtime((long *)&(card[temp_i].install_time));
//		printf("%d-%02d-%02d %02d:%02d:%02d\n",temp_time->tm_year+1900,temp_time->tm_mon+1,temp_time->tm_mday,temp_time->tm_hour,temp_time->tm_min,temp_time->tm_sec);
		send_buf[i++] = strlen((char *)card[temp_i].line_num);
		for(temp_j = 0;temp_j< (int)strlen((char *)card[temp_i].line_num);temp_j++)
		{
			send_buf[i++] = card[temp_i].line_num[temp_j];
		}
		send_buf[i++] = strlen((char *)card[temp_i].company);
		for(temp_j = 0;temp_j< (int)strlen((char *)card[temp_i].company);temp_j++)
		{
			send_buf[i++] = card[temp_i].company[temp_j];
		}

		send_buf[1] = (0xFF & i) >> 8;
		send_buf[2] = (0XFF & i);
		send_buf[i++] = makecheck(send_buf);
		send_buf[i++] = 0x7D;
		sendto(sockfd, send_buf, i, 0, (struct sockaddr *)&DeviceAddr, sizeof(struct sockaddr));
		recv_len = ReceiveDeviceReply( sockfd, DeviceAddr, Rec_buf, TimeOut_S);
		if( (recv_len<=0 ) && (Rec_buf[12] != 0x00) )
		{
			goto _error;
		}
	}
	if(temp_i == j)
	{
		_conn->terminateStatement(_stmt);
		OraEnviroment->terminateConnection(_conn);
		return true;
	}
_error:
	_conn->terminateStatement(_stmt);
	OraEnviroment->terminateConnection(_conn);
	return false;
}

int Set_DeviceTime(int sockfd,int CrossID,struct sockaddr_in DeviceAddr)
{
	//0x7E	2字节	2字节	2字节	2字节	0x20	0x09	0x00	4字节	1字节	0x7D
	unsigned char Rec_buf[SingleRecvMaxLen];
	int recv_len;
	unsigned char buf[18] = {0x7E,0x00,0x10,0x00,0x01,0xFF,0xFF,0x00,0x00,0x20,0x09,0x00,0x00,0x00,0x00,0x00,0x00,0x7D};
	time_t time_now;
	time(&time_now);
	time_now = time_now + (8*60*60);				//设备中的时区为0时区   为了让上报的时间和现实一致  所以下发是要加上8个小时
	buf[7] = (CrossID & 0xFF00)>>8;
	buf[8] = (CrossID & 0xFF);
	buf[12] = (time_now & 0xFF000000) >> 24;
	buf[13] = (time_now & 0xFF0000) >> 16;
	buf[14] = (time_now & 0xFF00) >> 8;
	buf[15] = (time_now & 0xFF);
	buf[16] = makecheck(buf);

	sendto(sockfd, buf, 18, 0, (struct sockaddr *)&DeviceAddr, sizeof(struct sockaddr));
	recv_len = ReceiveDeviceReply( sockfd, DeviceAddr, Rec_buf, TimeOut_S);
	if( (recv_len>0 ) && (Rec_buf[12] == 0x00) )
	{
		return true;
	}

	return false;
}

int Se_tDeviceReboot(int sockfd,int CrossID,struct sockaddr_in DeviceAddr)
{
	//0x7E	2字节	2字节	2字节	2字节	0x20	0x09	0x00	4字节	1字节	0x7D
	unsigned char Rec_buf[SingleRecvMaxLen];
	int recv_len;
	unsigned char buf[15] = {0x7E,0x00,0x0D,0x00,0x01,0xFF,0xFF,0x00,0x00,0x20,0x0A,0x00,0x00,0x00,0x7D};
	buf[7] = (CrossID & 0xFF00)>>8;
	buf[8] = (CrossID & 0xFF);
	buf[13] = makecheck(buf);

	sendto(sockfd, buf, 15, 0, (struct sockaddr *)&DeviceAddr, sizeof(struct sockaddr));
	recv_len = ReceiveDeviceReply( sockfd, DeviceAddr, Rec_buf, TimeOut_S);
	if( (recv_len>0 ) && (Rec_buf[12] == 0x00) )
	{
		return true;
	}
	else
	{
		return false;
	}
}

int Se_tDevicePriority(int sockfd,struct sockaddr_in DeviceAddr,unsigned char * send_data)
{
	unsigned char Rec_buf[SingleRecvMaxLen];
	int recv_len;
	sendto(sockfd, send_data, ((( send_data[1] << 8 ) | send_data[2])+2), 0, (struct sockaddr *)&DeviceAddr, sizeof(struct sockaddr));
	recv_len = ReceiveDeviceReply( sockfd, DeviceAddr, Rec_buf, TimeOut_S);
	if( (recv_len>0 ) && (Rec_buf[12] == 0x00) )
	{
		return true;
	}
	else
	{
		return false;
	}
}

/*
 * 根据参数接收报文，成功返回接收到的字节数，失败返回0
 */
int ReceiveDeviceReply(int sockfd,struct sockaddr_in DeviceAddr,unsigned char * data,int timeout_s)
{
	struct timeval 	timeVal;
	int ret;
	int numbytes;
	int sin_size_1=sizeof(struct sockaddr_in);
	timeVal.tv_sec = timeout_s;
	timeVal.tv_usec= 0;
	fd_set  Rec_fdSet;
	FD_ZERO(&Rec_fdSet);
	FD_SET(sockfd, &Rec_fdSet);
	ret = select((int) sockfd + 1, &Rec_fdSet,NULL, NULL, &timeVal);
	if( ret == 0 )    //select timeout
	{
		return 0;
	}
	if( !ret )    //select error
	{
		printf("UDP Select error result=%d,file=%s,function=%s,line=%d\n",ret,__FILE__,__FUNCTION__,__LINE__);
		return 0;
	}
	else          //select success
	{
		memset(data,'\0',SingleRecvMaxLen);
		if ((numbytes=recvfrom(sockfd,  data,  SingleRecvMaxLen,  0,  (struct sockaddr *)&DeviceAddr, (socklen_t *) &sin_size_1))==-1)
		{
			perror("recv ");
			return 0;
		}
		if(numbytes == 0)
		{
			return 0;
		}
		else if(numbytes >5)           //头   长度_H   长度_L   校验    尾         至少5个
		{
			data[numbytes] = '\0';
			if( check_buf( (unsigned char *) data) == true)
			{
				return numbytes;
			}
			else
				return 0;
		}
	}
	return 0;
}
