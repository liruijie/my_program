/*
 * CommunicateWithWeb.cpp
 *
 * Created on: 2016年12月13日
 * Author: LIRUIJIE
 */
#include "include.h"
#define 	WebBufLen		50
#define 	WebPort 			10010
#define 	WebControlPort	10011


#define straight_percent 	0.7
#define left_percent 		0.2
#define right_percent 		0.1

int web_sock;
int web_control_sock;
extern int ExistSignalNum;

struct RegionsCoordInfo RegionsCoord;
struct web_message_info
{
		unsigned char RecvBuf[WebBufLen];
		struct sockaddr_in WebServerAddr;
};
extern SignalInfo	Signal[MaxSignalNum];
extern ServiceInfo Server;
void CommunicateWithWeb()
{
	int ret;
	ret = InitWebSocket();
	if(ret == false)
	{
		exit(0);
	}
	pthread_t p_ReceiveWebData;
	ret = pthread_create(&p_ReceiveWebData,NULL,ReceiveWebData,NULL);
	if(ret != 0)
	{
		exit(0);
	}

	pthread_t p_WebControl;
	ret = pthread_create(&p_WebControl,NULL,WebControlReceive,NULL);
	if(ret != 0)
	{
		exit(0);
	}
}
int InitWebSocket()
{
	struct sockaddr_in my_addr;
	struct sockaddr_in my_addr2;
	//初始化通信端口
	if ((web_sock = socket(PF_INET, SOCK_DGRAM, 0)) < 0)
	{
		perror("fail to socket");
		return false;
	}
	int on=1;
	if((setsockopt(web_sock,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(on)))<0)
	{
		perror("setsockopt failed");
		close(web_sock);
		return false;
	}
	my_addr.sin_family = PF_INET;
	my_addr.sin_port = htons(WebPort);
	my_addr.sin_addr.s_addr = INADDR_ANY;
	if( bind( web_sock, (struct sockaddr *)&my_addr, sizeof(my_addr) ) < 0 )
	{
		perror("fail to bind");
		exit(-1);
	}
	//初始化控制通讯端口
	if ((web_control_sock = socket(PF_INET, SOCK_DGRAM, 0)) < 0)
	{
		perror("fail to socket");
		return false;
	}
	if((setsockopt(web_control_sock,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(on)))<0)
	{
		perror("setsockopt failed");
		close(web_control_sock);
		return false;
	}
	my_addr2.sin_family = PF_INET;
	my_addr2.sin_port = htons(WebControlPort);
	my_addr2.sin_addr.s_addr = INADDR_ANY;
	if( bind( web_control_sock, (struct sockaddr *)&my_addr2, sizeof(my_addr2) ) < 0 )
	{
		perror("fail to bind");
		exit(-1);
	}
	return true;
}
/*
 * 接收WEB平台命令
 */
void * ReceiveWebData(void *arg)
{
	char data[WebBufLen];
	int ret;
	fd_set fdset;
	struct sockaddr_in WebServerAddr;
	int socklen=sizeof(struct sockaddr_in);
	prctl(PR_SET_NAME, (unsigned long)"ReceiveWebData");
	while(1)
	{
		//ret = ReceiveData(data);
		FD_ZERO(&fdset);
		FD_SET(web_sock, &fdset);
		ret = select((int) web_sock + 1, &fdset,NULL, NULL, NULL);
		if( ret == 0 )    //select timeout
		{
			continue;
		}
		if( !ret )    //select error
		{
			printf("receive error result=%d,file=%s,function=%s,line=%d\n",ret,__FILE__,__FUNCTION__,__LINE__);
			return (void *)0;
		}
		else          //select success
		{
			ret = recvfrom(web_sock,  data,  WebBufLen,  0,  (struct sockaddr *)&WebServerAddr, (socklen_t *) &socklen);
			if(ret==-1)
			{
				return 0;
			}
			else if(ret == 0)
			{
				return 0;
			}
			if(ret > 7)
			{
				data[ret] = '\0';
				if( check_buf( data ) == true)
				{
					ParsingData_Web((unsigned char *)data,WebServerAddr);
				}
			}
		}

	}
	return (void *)0;
}
/*
 *
 */
void * WebControlReceive(void * arg)
{
	char data[WebBufLen];
	char sqlbuf[100];
	int ret;
	fd_set fdset;
	struct sockaddr_in WebServerAddr;
	timeval timeout;
	time_t time_now;
	int i,check_sum;
	timeout.tv_sec = 1;
	timeout.tv_usec = 0;
	char buf_response[20];
	prctl(PR_SET_NAME, (unsigned long)"WebControlReceive");
	int socklen=sizeof(struct sockaddr_in);
	Connection *_conn = NULL;
	Statement *_stmt = NULL;
	if(GetConnectFromPool(&_conn,&_stmt) == false)
	{
		printf("更新信号机控制标志,连接数据库失败,cross_id= %d\n",1111);
		return (void *)0;
	}
	_stmt->setAutoCommit(true);
	while(1)
	{
		time(&time_now);
		FD_ZERO(&fdset);
		FD_SET(web_control_sock, &fdset);
		memset(data,'\0',WebBufLen);
		//检查所有信号机控制标志是否被设置，设置过的是否超时
		for(i = 0;i<ExistSignalNum;i++)
		{
			if(Signal[i].IsWebControled == true)
			{
				if(time_now - Signal[i].WebHeartbeatTime > 10)
				{
					Signal[i].IsWebControled = false;
					sprintf(sqlbuf,"update CONTROL_CMD set control_flag = 0 where signal_id = %d",Signal[i].cross_id);
					_stmt->execute(sqlbuf);
				}
			}
		}
		timeout.tv_sec = 1;
		timeout.tv_usec = 0;
		ret = select((int) web_control_sock + 1, &fdset,NULL, NULL, &timeout);
		if( ret == 0 )    //select timeout
		{
			continue;
		}
		if( !ret )    //select error
		{
			printf("receive error result=%d,file=%s,function=%s,line=%d\n",ret,__FILE__,__FUNCTION__,__LINE__);
			return (void *)0;
		}
		else          //select success
		{
			ret = recvfrom(web_control_sock,  data,  WebBufLen,  0,  (struct sockaddr *)&WebServerAddr, (socklen_t *) &socklen);
			if(ret==-1)
			{
				return (void *)0;;
			}
			else if(ret == 0)
			{
				continue;
			}
			if(ret > 7)
			{
				data[ret] = '\0';
				//8E 00 0D 00 01 00 00 04 4D 60 00 00 00 BF 8D
				if ((0x8e == (unsigned char)data[0]) && (0x8d == (unsigned char)data[15] )&& (0x60 == (unsigned char)data[9]))
				{
					memcpy(buf_response,data,ret);
					buf_response[9] = 0x30;
					sendto(web_control_sock, buf_response, ret , 0, (struct sockaddr*)&WebServerAddr, sizeof(WebServerAddr));
					//printf("data is correct,response web control\n");
					check_sum = 0;
					for (i = 1; i < 14; i++)
					{
						check_sum += data[i];
					}
					if ((unsigned char)data[14] == (unsigned char)(check_sum & 0xFF))
					{
						i = GetDeviceIndex((unsigned char *)data);
						if(i == -1)
						{
							continue;
						}
						switch(data[10])
						{
							case 0x00:
								Signal[i].IsWebControled = true;
								Signal[i].WebHeartbeatTime = time_now;
								try
								{
									sprintf(sqlbuf,"update CONTROL_CMD set control_flag = 1 where signal_id = %d",Signal[i].cross_id);
									//puts(sqlbuf);
									_stmt->execute(sqlbuf);
								}
								catch(SQLException &ex)
								{
									puts(sqlbuf);
									ex.getErrorCode();
									cout<<ex.getMessage()<<endl;
								}
								break;
							case 0x01:
								Signal[i].IsWebControled = false;
								try
								{
									sprintf(sqlbuf,"update CONTROL_CMD set control_flag = 0 where signal_id = %d",Signal[i].cross_id);
									//puts(sqlbuf);
									_stmt->execute(sqlbuf);
								}
								catch(SQLException &ex)
								{
									puts(sqlbuf);
									ex.getErrorCode();
									cout<<ex.getMessage()<<endl;
								}
								break;
							default:
								break;
						}
					}
				}
			}
		}
	}
	printf("error********\n\n");
	//pthread_exit(0);
	//return (void *)0;
}





/*
 *建立一个处理平台报文的线程,将Web平台的地址和收到的数据封装到结构体中,当成参数传入线程函数
 */
void ParsingData_Web(unsigned char *data,struct sockaddr_in WebAddr)
{
	int i =GetDeviceIndex(data);
	if(i < 0)
	{
		if( (data[9] != 0x50) || (data[10] != 0x05))		//不是区域协调命令
		{
			return ;
		}
	}
	pthread_t pth_handle;
	pthread_attr_t attr;
	int ret;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);//设置为线程分离属性设置为分离

	struct web_message_info *web_message = (struct web_message_info *)malloc(sizeof(web_message_info));
	memcpy(web_message->RecvBuf,data,WebBufLen);
	web_message->WebServerAddr = WebAddr;
	ret = pthread_create(&pth_handle,&attr,Handle_Pthread,web_message);
	if(ret != 0)		//创建线程失败  释放内存
	{
		free(web_message);
	}
	pthread_attr_destroy(&attr);					//回收分配给属性的资源
}

/*
 *处理平台报文的线程函数，解析报文，调用相应的功能函数,并向Web平台回复结果
 */
void * Handle_Pthread(void *arg)
{
	unsigned char data[WebBufLen];
	pthread_detach(pthread_self());
	prctl(PR_SET_NAME, (unsigned long)"ParseWebData");
	struct web_message_info web_message = *(struct web_message_info *)arg;
	memcpy(&web_message,arg,sizeof(web_message_info));
	free(arg);
	struct sockaddr_in WebAddr = web_message.WebServerAddr;
	memcpy(data,web_message.RecvBuf,WebBufLen);

	printf("Web pthread ,addr:%s,port:%d\n",inet_ntoa(WebAddr.sin_addr),ntohs(WebAddr.sin_port));

	//struct sockaddr_in DeviceAddr;
	int ret;
	unsigned char reply_buf[15] = {0x8E,0x00,0x0D,0x00,0x00,0xFF,0xFF,0x00,0x00,0x30,0x00,0x00,0x00,0x00,0x8D};
	//协议版本
	reply_buf[3] = data[3];
	reply_buf[4] = data[4];
	//路口ID
	reply_buf[7] = data[7];
	reply_buf[8] = data[8];

	reply_buf[10] = data[10];	//数据编码
	reply_buf[11] = data[11];	//数据编号
	int i =GetDeviceIndex(data);
	if(i < 0)
	{
		if( (data[9] != 0x50) || (data[10] != 0x05))		//不是区域协调命令
		{
			return (void *)0;
		}
	}
	switch(data[9])
	{
		case 0x10:			//查询参数
		{
			switch(data[10])
			{
				case 0x01:  //基础信息
					ret = Request_BaseInfo_0x01(i);
					break;
				case 0x23:  //相位配置
					int phase_id;
					phase_id = data[13];
					ret = Request_Phase_0x23(i,phase_id);
					break;
				case 0x2B:  //方案配置
					int plan_id;
					plan_id= data[13];
					ret = Request_Plan_0x2B(i,plan_id);
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
				case 0x23:  //相位配置
					int phase_id;
					phase_id = data[13];
					ret = Set_Phase_0x23(i,phase_id);
					break;
				case 0x2B:  //方案配置
					int plan_id;
					plan_id= data[13];
					ret = Set_Plan_0x2B(i,plan_id);
					break;
				default:
					break;
			}
			break;
		}
		case 0x50:
		{
			int type;
			switch(data[10])
			{
				case 0x01:	//步进
					ret = Control_NextStep_0x01(i);
					break;
				case 0x02:	//切换方案
					int plan_id;
					plan_id= data[13];
					ret = Control_ChangePlane_0x02(i,plan_id);
					break;
				case 0x03:	//特勤控制
					type = data[13];
					ret = Control_SpecialService_0x03(i,type);
					break;
				case 0x04:	//降级控制
					type = data[13];
					ret = Control_DemoteMode_0x04(i,type);
					break;
				case 0x05:	//区域协调指令下发
					ret = Control_RegionsCoordinate_0x05(data[13]);
					break;
				case 0xF1:	//校时报文
					ret = Control_CheckTime_0xF1(i);
					break;
				case 0xF3:	//重启信号机
					ret = Control_RestartSignal_0xF3(i);
					break;
				case 0xFF:	//重启通讯服务
					ret = Control_RestartServer_0xFF();
					break;
				default:
					break;
			}
			break;
		}
		default:
			break;
	}

	if(ret == true)
	{
		reply_buf[12] = 0x00;
	}
	else
	{
		reply_buf[12] = 0x10;
	}
	reply_buf[13] = makecheck(reply_buf);
	sendto(web_sock, reply_buf, 15, 0, (struct sockaddr *)&WebAddr, sizeof(struct sockaddr));
	return (void *)0;
}
/*
 * 获取报文中的设备在设备信息结构体数组中的下标
 */
int GetDeviceIndex(unsigned char *buf)
{
	int device_id = ( buf[7] << 8 ) | buf[8];
	int i;

	for(i = 0; i< ExistSignalNum; i++)
	{
		if(device_id == Signal[i].cross_id)
		{
			break;
		}
	}
	if(i == ExistSignalNum)
		return -1;
	return i;
}
bool check_buf(  char * rcv_buf)
{
	int rcv_len = rcv_buf[1] * 256 + rcv_buf[2];
	int check_sum = 0;
	int i;
	if (0x8e == (unsigned char )rcv_buf[0] && 0x8d == (unsigned char )rcv_buf[rcv_len + 1])
	{
		for (i = 1; i < rcv_len; i++)
		{
			check_sum += rcv_buf[i];
		}

		if ((unsigned char )rcv_buf[rcv_len] == (check_sum & 0xFF))
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

int Request_BaseInfo_0x01(int index)
{
	static int i = index;
	char xml_buf[1000];
	char seq[20];
	int ret = false;
	int j;
	GetSeq(seq);
	sprintf(xml_buf,	"<?xml version=\"1.0\" encoding=\"utf-8\"?>"
							"<Message>"
								"<Version>%s</Version>"
								"<Token>%s</Token>"
								"<From>%s</From>"
								"<To>%s</To>"
								"<Type>REQUEST</Type>"
								"<Seq>%s</Seq>"
								"<Body>"
									"<Operation order=\"1\" name=\"Get\">"
										"<SignalControler/>"
									"</Operation>"
								"</Body>"
							"</Message>",XmlVersion,Server.token,Server.my_ip[0],Signal[i].ip,seq);
	Signal[i].flag.GetBaseInfo = WAIT;
	ret = SendData(xml_buf,strlen(xml_buf));
	if(ret != true)
	{
		return false;
	}
	ret = false;
	for(j = 0;j<500;j++)
	{
		usleep(10*1000);
		if(Signal[i].flag.GetBaseInfo == true)
		{
			ret = true;
			break;
		}
		else if(Signal[i].flag.GetBaseInfo == false)
		{
			ret = false;
			break;
		}
	}
	return ret;
}
int Request_Phase_0x23(int index,int phase_id)
{
	static int i = index;
	char xml_buf[1000];
	char seq[20];
	int ret = false;
	int j;
	GetSeq(seq);
	sprintf(xml_buf,	"<?xml version=\"1.0\" encoding=\"utf-8\"?>"
							"<Message>"
								"<Version>%s</Version>"
								"<Token>%s</Token>"
								"<From>%s</From>"
								"<To>%s</To>"
								"<Type>REQUEST</Type>"
								"<Seq>%s</Seq>"
								"<Body>"
									"<Operation order=\"1\" name=\"Get\">"
										"<Phase>"
											"<PhaseNo>%d</PhaseNo>"
										"</Phase>"
									"</Operation>"
								"</Body>"
							"</Message>",XmlVersion,Server.token,Server.my_ip[0],Signal[i].ip,seq,phase_id);
	Signal[i].flag.GetPhase = WAIT;
	ret = SendData(xml_buf,strlen(xml_buf));
	if(ret != true)
	{
		return false;
	}
	ret = false;
	for(j = 0;j<500;j++)
	{
		usleep(10*1000);
		if(Signal[i].flag.GetPhase == true)
		{
			ret = true;
			break;
		}
		else if(Signal[i].flag.GetPhase == false)
		{
			ret = false;
			break;
		}
	}
	return ret;

}
int Request_Plan_0x2B(int index,int phase_id)
{
	static int i = index;
	char xml_buf[1000];
	char seq[20];
	int ret = false;
	int j;
	GetSeq(seq);
	sprintf(xml_buf,	"<?xml version=\"1.0\" encoding=\"utf-8\"?>"
							"<Message>"
								"<Version>%s</Version>"
								"<Token>%s</Token>"
								"<From>%s</From>"
								"<To>%s</To>"
								"<Type>REQUEST</Type>"
								"<Seq>%s</Seq>"
								"<Body>"
									"<Operation order=\"1\" name=\"Get\">"
										"<Period>"
											"<PlanNo>%d</PlanNo>"
										"</Period>"
									"</Operation>"
								"</Body>"
							"</Message>",XmlVersion,Server.token,Server.my_ip[0],Signal[i].ip,seq,phase_id);
	Signal[i].flag.GetPlan = WAIT;
	ret = SendData(xml_buf,strlen(xml_buf));
	if(ret != true)
	{
		return false;
	}
	ret = false;
	for(j = 0;j<500;j++)
	{
		usleep(10*1000);
		if(Signal[i].flag.GetPlan == true)
		{
			ret = true;
			break;
		}
		else if(Signal[i].flag.GetPlan == false)
		{
			ret = false;
			break;
		}
	}
	return ret;
}
int Set_Phase_0x23(int index,int phase_id)
{
	static int i = index;
	Connection *_conn = NULL;
	Statement *_stmt = NULL;
	ResultSet *Result;
	char sqlbuf[200];

	if(GetConnectFromPool(&_conn,&_stmt) == false)
	{
		printf("设置相位参数,连接数据库失败,cross_id= %d\n",Signal[i].cross_id);
		return false;
	}
	_stmt->setAutoCommit(true);
	struct SignalPhase Phase;
	try
	{
		sprintf(sqlbuf,"select t.yellow,t.clearred,t.pgreenflash,t.vgreenflash,t.predflash,t.vredflash,t.channels from ZKTD_PHASE t where t.unit_id = %d and t.phaseno = %d",Signal[i].cross_id,phase_id);
		Result = _stmt->executeQuery(sqlbuf);
	}
	catch (SQLException &sqlExcp)
	{
		DisconnectOracle(&_conn,&_stmt);
	   sqlExcp.getErrorCode();
	   cout<<sqlExcp.getMessage() << endl;
	   return false;
	}
	char Channel[100];
	char *t_channel;
	memset(Channel,'\0',100);
	int j = 0;
	if(Result->next() != 0)
	{
		Phase.Yellow =  Result->getInt(1);
		Phase.ClearRed =  Result->getInt(2);
		Phase.PGreenFlash =  Result->getInt(3);
		Phase.VGreenFlash =  Result->getInt(4);
		Phase.PRedFlash =  Result->getInt(5);
		Phase.VRedFlash =  Result->getInt(6);
		sprintf(Channel,Result->getString(7).c_str());

		t_channel = strtok(Channel, (char *)&",");
		if(t_channel == NULL)
		{
			_stmt->closeResultSet(Result);
			DisconnectOracle(&_conn,&_stmt);
			return false;
		}
		Phase.LaneNo[j++] = atoi(t_channel);
		while((t_channel = strtok(NULL, (char *)&",")))
		{
			Phase.LaneNo[j++] = atoi(t_channel);
		}
		Phase.LaneCount = j;
		_stmt->closeResultSet(Result);
		DisconnectOracle(&_conn,&_stmt);
	}
	else
	{
		_stmt->closeResultSet(Result);
		DisconnectOracle(&_conn,&_stmt);
		return false;
	}


	char seq_buf[20];
	GetSeq(seq_buf);
	char xml_buf[5000];
	char xml_buf_end[500];
	memset(xml_buf,'\0',5000);
	memset(xml_buf_end,'\0',500);
	sprintf(xml_buf,	"<?xml version=\"1.0\" encoding=\"utf-8\"?>"
							"<Message>"
								"<Version>%s</Version>"
								"<Token>%s</Token>"
								"<From>%s</From>"
								"<To>%s</To>"
								"<Type>REQUEST</Type>"
								"<Seq>%s</Seq>"
								"<Body>"
									"<Operation order=\"1\" name=\"Set\">"
										"<Phase>"
											"<PhaseNo>%d</PhaseNo>"
											"<Yellow>%d</Yellow>"
											"<ClearRed>%d</ClearRed>"
											"<PGreenFlash>%d</PGreenFlash>"
											"<VGreenFlash>%d</VGreenFlash>"
											"<PRedFlash>%d</PRedFlash>"
											"<VRedFlash>%d</VRedFlash>"
											"<LaneNoList>",XmlVersion,Server.token,Server.my_ip[0],Signal[i].ip,seq_buf,phase_id,Phase.Yellow,Phase.ClearRed,
																Phase.PGreenFlash,Phase.VGreenFlash,Phase.PRedFlash,Phase.VRedFlash);
	sprintf(xml_buf_end,				"</LaneNoList>"
									 	"</Phase>"
									"</Operation>"
								"</Body>"
							"</Message>");

	char xml_channel[500];
	char tempbuf[30];
	memset(xml_channel,'\0',500);
	j=0;
	for(j = 0;j< Phase.LaneCount;j++)
	{
		memset(tempbuf,'\0',30);
		sprintf(tempbuf,"<LaneNo>%d</LaneNo>",Phase.LaneNo[j]);
		strcat(xml_channel,tempbuf);
	}
	strcat(xml_buf,xml_channel);
	strcat(xml_buf,xml_buf_end);
	int ret;
	Signal[i].flag.SetPhase = WAIT;
	ret = SendData(xml_buf,strlen(xml_buf));
	if(ret != true)
	{

		return false;
	}
	ret = false;
	for(j = 0;j<500;j++)
	{
		usleep(10*1000);
		if(Signal[i].flag.SetPhase == true)
		{
			ret = true;
			break;
		}
		else if(Signal[i].flag.SetPhase == false)
		{
			ret = false;
			break;
		}
	}
	return ret;
}
int Set_Plan_0x2B(int index,int plan_id)
{
	static int i = index;
	Connection *_conn = NULL;
	Statement *_stmt = NULL;
	ResultSet *Result;
	char sqlbuf[200];
	struct SignalPlan Plan;
	if(GetConnectFromPool(&_conn,&_stmt) == false)
	{
		printf("设置方案参数,连接数据库失败,cross_id= %d\n",Signal[i].cross_id);
		return false;
	}


	try
	{
		_stmt->setAutoCommit(true);
		sprintf(sqlbuf,"select t.time_no,t.starthour,t.startminute,t.controlmode,t.modeno,t.coordphaseno,t.offset "
				"from ZKTD_PLAN_TIME t where t.unit_id = %d and t.plan_no = %d order by t.time_no",Signal[i].cross_id,plan_id);
		Result =  _stmt->executeQuery(sqlbuf);
	}
	catch (SQLException &sqlExcp)
	{
		DisconnectOracle(&_conn,&_stmt);
	   sqlExcp.getErrorCode();
	   cout<<sqlExcp.getMessage() << endl;
	   return false;
	}
	int j=0;
	while(Result->next() != 0)
	{
		Plan.Period[j].PeriodNum = Result->getInt(1);
		Plan.Period[j].StartHour = Result->getInt(2);
		Plan.Period[j].StartMinute = Result->getInt(3);
		Plan.Period[j].ControlMode = Result->getInt(4);
		Plan.Period[j].ModeNo = Result->getInt(5);
		Plan.Period[j].CoordPhaseNo = Result->getInt(6);
		Plan.Period[j].OffSet = Result->getInt(7);
		j++;
	}
	_stmt->closeResultSet(Result);
	if(j == 0)
	{
		DisconnectOracle(&_conn,&_stmt);
		return false;
	}
	Plan.PeriodCount = j;


	int k;
	for(j = 0;j<Plan.PeriodCount;j++)
	{
		memset(sqlbuf,'\0',200);
		k = 0;
		try
		{
			sprintf(sqlbuf,"select t.phaseno,t.green from ZKTD_PLAN_TIME_STAGE t where t.unit_id = %d and t.plan_no = %d and t.time_no = %d order by t.stage_no",
					Signal[i].cross_id,plan_id,Plan.Period[j].PeriodNum);
			Result =  _stmt->executeQuery(sqlbuf);
			while(Result->next() != 0)
			{
				Plan.Period[j].PhaseNo[k] = Result->getInt(1);
				Plan.Period[j].Green[k] = Result->getInt(2);
				k++;
			}
			_stmt->closeResultSet(Result);
			if(k == 0)
			{
				DisconnectOracle(&_conn,&_stmt);
				return false;
			}
			else
			{
				Plan.Period[j].StageCount = k;
			}
		}
		catch (SQLException &sqlExcp)
		{
			DisconnectOracle(&_conn,&_stmt);
		   sqlExcp.getErrorCode();
		   cout<<sqlExcp.getMessage() << endl;
		   return false;
		}
	}

	DisconnectOracle(&_conn,&_stmt);
	char xml_buf[10000];
	char seq_buf[20];
	char period_buf[2000];
	char stage_buf[1000];
	GetSeq(seq_buf);
	memset(xml_buf,'\0',10000);
	sprintf(xml_buf,	"<?xml version=\"1.0\" encoding=\"utf-8\"?>"
							"<Message>"
								"<Version>%s</Version>"
								"<Token>%s</Token>"
								"<From>%s</From>"
								"<To>%s</To>"
								"<Type>REQUEST</Type>"
								"<Seq>%s</Seq>"
								"<Body>"
									"<Operation order=\"1\" name=\"Set\">"
										"<Period>"
											"<PlanNo></PlanNo>"
											"<PeriodList>",XmlVersion,Server.token,Server.my_ip[0],Signal[i].ip,seq_buf);
	for(j=0;j<Plan.PeriodCount;j++)
	{
		memset(period_buf,'\0',2000);
			sprintf(period_buf,			"<PeriodParam>"
													"<StartHour>%d</StartHour>"
													"<StartMinute>%d</StartMinute>"
													"<ControlMode>%d</ControlMode>"
													"<ModelNo>%d</ModelNo>"
													"<CoordPhaseNo>%d</CoordPhaseNo>"
													"<OffSet>%d</OffSet>"
													"<StageList>",Plan.Period[j].StartHour,Plan.Period[j].StartMinute,
													Plan.Period[j].ControlMode,Plan.Period[j].ModeNo,Plan.Period[j].CoordPhaseNo,
													Plan.Period[j].OffSet);
		for(k=0;k<Plan.Period[j].StageCount;k++)
		{
			memset(stage_buf,'\0',1000);
				sprintf(stage_buf,				"<Stage>"
															"<PhaseNo>%d</PhaseNo>"
															"<Green>%d</Green>"
														"</Stage>",Plan.Period[j].PhaseNo[k],Plan.Period[j].Green[k]);
			strcat(period_buf,stage_buf);
		}
		strcat(period_buf, 					"</StageList>"
												"</PeriodParam>");
		strcat(xml_buf,period_buf);
	}
	strcat(xml_buf,					"</PeriodList>"
										"</Period>"
									"</Operation>"
								"</Body>"
							"</Message>");
	int ret;
	Signal[i].flag.SetPlan = WAIT;
	ret = SendData(xml_buf,strlen(xml_buf));
	if(ret != true)
	{
		return false;
	}
	ret = false;
	for(j = 0;j<500;j++)
	{
		usleep(10*1000);
		if(Signal[i].flag.SetPlan == true)
		{
			ret = true;
			break;
		}
		else if(Signal[i].flag.SetPlan == false)
		{
			ret = false;
			break;
		}
	}
	return ret;

}

int Control_NextStep_0x01(int index)
{
	static int i = index;
	char xml_buf[500];
	char seq_buf[20];
	GetSeq(seq_buf);
	sprintf(xml_buf,	"<?xml version=\"1.0\" encoding=\"utf-8\"?>"
							"<Message>"
								"<Version>%s</Version>"
								"<Token>%s</Token>"
								"<From>%s</From>"
								"<To>%s</To>"
								"<Type>REQUEST</Type>"
								"<Seq>%s</Seq>"
								"<Body>"
									"<Operation order=\"1\" name=\"Set\">"
										"<NextStep/>"
									"</Operation>"
								"</Body>"
							"</Message>",XmlVersion,Server.token,Server.my_ip[0],Signal[i].ip,seq_buf);
	int ret,j;
	Signal[i].flag.CtrlNextStep = WAIT;
	ret = SendData(xml_buf,strlen(xml_buf));
	if(ret != true)
	{
		return false;
	}
	ret = false;
	for(j = 0;j<500;j++)
	{
		usleep(10*1000);
		if(Signal[i].flag.CtrlNextStep == true)
		{
			ret = true;
			break;
		}
		else if(Signal[i].flag.CtrlNextStep == false)
		{
			ret = false;
			break;
		}
	}
	return ret;

}
int Control_ChangePlane_0x02(int index,int plan_id)
{
	static int i = index;
	char xml_buf[500];
	char seq_buf[20];
	GetSeq(seq_buf);
	sprintf(xml_buf,	"<?xml version=\"1.0\" encoding=\"utf-8\"?>"
							"<Message>"
								"<Version>%s</Version>"
								"<Token>%s</Token>"
								"<From>%s</From>"
								"<To>%s</To>"
								"<Type>REQUEST</Type>"
								"<Seq>%s</Seq>"
								"<Body>"
									"<Operation order=\"1\" name=\"Set\">"
										"<CrossPlan>"
										"<PlanNo>%d</PlanNo>"
										"</CrossPlan>"
									"</Operation>"
								"</Body>"
							"</Message>",XmlVersion,Server.token,Server.my_ip[0],Signal[i].ip,seq_buf,plan_id);
	int ret,j;
	Signal[i].flag.CtrlChangePlan = WAIT;
	ret = SendData(xml_buf,strlen(xml_buf));
	if(ret != true)
	{
		return false;
	}
	ret = false;
	for(j = 0;j<500;j++)
	{
		usleep(10*1000);
		if(Signal[i].flag.CtrlChangePlan == true)
		{
			ret = true;
			break;
		}
		else if(Signal[i].flag.CtrlChangePlan == false)
		{
			ret = false;
			break;
		}
	}
	return ret;

}
int Control_SpecialService_0x03(int index,int type)
{

	char xml_buf[500];
	char seq_buf[20];
	GetSeq(seq_buf);
	static int i = index;
	sprintf(xml_buf,	"<?xml version=\"1.0\" encoding=\"utf-8\"?>"
							"<Message>"
								"<Version>%s</Version>"
								"<Token>%s</Token>"
								"<From>%s</From>"
								"<To>%s</To>"
								"<Type>REQUEST</Type>"
								"<Seq>%s</Seq>"
								"<Body>"
									"<Operation order=\"1\" name=\"Set\">"
										"<GreenChannel>"
										"<Type>%d</Type>"
										"</GreenChannel>"
									"</Operation>"
								"</Body>"
							"</Message>",XmlVersion,Server.token,Server.my_ip[0],Signal[i].ip,seq_buf,type);
	int ret,j;
	Signal[i].flag.CtrlSpecialService = WAIT;
	ret = SendData(xml_buf,strlen(xml_buf));
	if(ret != true)
	{
		return false;
	}
	ret = false;
	for(j = 0;j<500;j++)
	{
		usleep(10*1000);
		if(Signal[i].flag.CtrlSpecialService == true)
		{
			ret = true;
			break;
		}
		else if(Signal[i].flag.CtrlSpecialService == false)
		{
			ret = false;
			break;
		}
	}
	return ret;
}

int Control_DemoteMode_0x04(int index,int type)
{
	static int i = index;
	char xml_buf[500];
	char seq_buf[20];
	GetSeq(seq_buf);
	sprintf(xml_buf,	"<?xml version=\"1.0\" encoding=\"utf-8\"?>"
							"<Message>"
								"<Version>%s</Version>"
								"<Token>%s</Token>"
								"<From>%s</From>"
								"<To>%s</To>"
								"<Type>REQUEST</Type>"
								"<Seq>%s</Seq>"
								"<Body>"
									"<Operation order=\"1\" name=\"Set\">"
										"<DomoteMode>"
										"<Type>%d</Type>"
										"</DomoteMode>"
									"</Operation>"
								"</Body>"
							"</Message>",XmlVersion,Server.token,Server.my_ip[0],Signal[i].ip,seq_buf,type);
	int ret,j;
	Signal[i].flag.CtrlDemoteMode = WAIT;
	ret = SendData(xml_buf,strlen(xml_buf));
	if(ret != true)
	{
		return false;
	}
	ret = false;
	for(j = 0;j<500;j++)
	{
		usleep(10*1000);
		if(Signal[i].flag.CtrlDemoteMode == true)
		{
			ret = true;
			break;
		}
		else if(Signal[i].flag.CtrlDemoteMode == false)
		{
			ret = false;
			break;
		}
	}
	return ret;

}
int Control_RegionsCoordinate_0x05(int Coord_ID)
{
	static int i = Coord_ID;
	pthread_t p_RegionsCoordinate;
	Connection *_conn = NULL;
	Statement *_stmt = NULL;
	char sqlbuf[200];
	int ret= pthread_create(&p_RegionsCoordinate,NULL,RegionsCoordinate_pthread,(void *)&i);
	if(ret !=0 )
	{
		printf("区域协调线程建立失败\n");
		return false;
	}

	if(GetConnectFromPool(&_conn,&_stmt) == false)
	{
		printf("区域协调,连接数据库失败,协调ID= %d\n",Coord_ID);
		return false;
	}
	_stmt->setAutoCommit(true);
	sprintf(sqlbuf,"update AREA_COORD_PLAN set TASK_STATE = 1 where ID = %d",Coord_ID);
	_stmt->execute(sqlbuf);
	DisconnectOracle(&_conn,&_stmt);

	sleep(1);
	return true;

}
int Control_CheckTime_0xF1(int index)
{
	static int i = index;
	char xml_buf[500];
	char seq_buf[20];
	GetSeq(seq_buf);
	time_t time_now;
	time(&time_now);
	tm * timeinfo = localtime(&time_now);
	char time_buf[30];
	sprintf(time_buf,"%d-%02d-%02d %02d:%02d:%02d",
			timeinfo->tm_year+1900,timeinfo->tm_mon+1,timeinfo->tm_mday,timeinfo->tm_hour,timeinfo->tm_min,timeinfo->tm_sec);

	sprintf(xml_buf,	"<?xml version=\"1.0\" encoding=\"utf-8\"?>"
							"<Message>"
								"<Version>%s</Version>"
								"<Token>%s</Token>"
								"<From>%s</From>"
								"<To>%s</To>"
								"<Type>REQUEST</Type>"
								"<Seq>%s</Seq>"
								"<Body>"
									"<Operation order=\"1\" name=\"Set\">"
										"<SDO_TimeServer>"
										"<DateTime>%s</DateTime>"
										"</SDO_TimeServer>"
									"</Operation>"
								"</Body>"
							"</Message>",XmlVersion,Server.token,Server.my_ip[0],Signal[i].ip,seq_buf,time_buf);
	int ret,j;
	Signal[i].flag.CtrlCheckTime = WAIT;
	ret = SendData(xml_buf,strlen(xml_buf));
	if(ret != true)
	{
		return false;
	}
	ret = false;
	for(j = 0;j<500;j++)
	{
		usleep(10*1000);
		if(Signal[i].flag.CtrlCheckTime == true)
		{
			ret = true;
			break;
		}
		else if(Signal[i].flag.CtrlCheckTime == false)
		{
			ret = false;
			break;
		}
	}
	return ret;

}
int Control_RestartSignal_0xF3(int index)
{
		static int i = index;
		char xml_buf[500];
		char seq_buf[20];
		memset(seq_buf,'\0',20);
		GetSeq(seq_buf);
		sleep(1);
		sprintf(xml_buf,	"<?xml version=\"1.0\" encoding=\"utf-8\"?>"
								"<Message>"
									"<Version>%s</Version>"
									"<Token>%s</Token>"
									"<From>%s</From>"
									"<To>%s</To>"
									"<Type>REQUEST</Type>"
									"<Seq>%s</Seq>"
									"<Body>"
										"<Operation order=\"1\" name=\"Set\">"
											"<Restart/>"
										"</Operation>"
									"</Body>"
								"</Message>",XmlVersion,Server.token,Server.my_ip[0],Signal[i].ip,seq_buf);
		int ret,j;
		Signal[i].flag.CtrlRestartSignal = WAIT;
		ret = SendData(xml_buf,strlen(xml_buf));
		if(ret != true)
		{
			return false;
		}
		ret = false;
		for(j = 0;j<500;j++)
		{
			usleep(10*1000);
			if(Signal[i].flag.CtrlRestartSignal == true)
			{
				ret = true;
				break;
			}
			else if(Signal[i].flag.CtrlRestartSignal == false)
			{
				ret = false;
				break;
			}
		}
		return ret;

}
int Control_RestartServer_0xFF()
{
	return true;
}

void * RegionsCoordinate_pthread(void *arg)
{
	Connection *_conn = NULL;
	Statement *_stmt = NULL;
	ResultSet *Result;
	char sqlbuf[200];
	int coor_id = *((int *)arg);
	pthread_detach(pthread_self());
	prctl(PR_SET_NAME, (unsigned long)"RegionsCoordinate");
	if(GetConnectFromPool(&_conn,&_stmt) == false)
	{
		printf("区域协调,连接数据库失败,协调ID= %d\n",coor_id);
		pthread_exit(0);
	}
	_stmt->setAutoCommit(true);
	memset(sqlbuf,'\0',200);
	sprintf(sqlbuf,"select CYCLE_MAX,CYCLE_MIN,PLAN_UPDATE_CYCLE from AREA_COORD_PLAN where ID = %d",coor_id);
	Result = _stmt->executeQuery(sqlbuf);
	if(Result->next() !=0)
	{
		RegionsCoord.CYCLE_MAX = Result->getInt(1);
		RegionsCoord.CYCLE_MIN = Result->getInt(2);
		RegionsCoord.PLAN_UPDATE_CYCLE = Result->getInt(3);
		_stmt->closeResultSet(Result);
	}
	else
	{
		printf("查询区域协调信息(最大周期、最小周期、方案更新周期)失败\n");
		_stmt->closeResultSet(Result);
		DisconnectOracle(&_conn,&_stmt);
		pthread_exit(0);
	}
	memset(sqlbuf,'\0',200);
	sprintf(sqlbuf,"select t.unit_id,t.is_master,k.signal_type from AREA_COORD_UNIT t,SIGNAL_CONFIG_INFO k where t.area_coord_id = %d and t.unit_id = k.signal_id",coor_id);
	Result = _stmt->executeQuery(sqlbuf);
	int j=0;
	while(Result->next() !=0)
	{
		RegionsCoord.Unit[j].ID = Result->getInt(1);
		RegionsCoord.Unit[j].IS_MASTER = Result->getInt(2);
		memset(RegionsCoord.Unit[j].Signal_type,'\0',10);
		sprintf(RegionsCoord.Unit[j].Signal_type,Result->getString(3).c_str());
		if(memcmp(RegionsCoord.Unit[j].Signal_type,"ITC100",6) == 0)
		{
			RegionsCoord.Unit[j].GetCoordinatePlan = ITC_CoordinatePlan;
		}
		else
		{
			RegionsCoord.Unit[j].GetCoordinatePlan = ZKTD_CoordinatePlan;
		}
		j++;
	}
	_stmt->closeResultSet(Result);
	if(j == 0)
	{
		printf("查询区域协调信息(路口信息)失败\n");
		DisconnectOracle(&_conn,&_stmt);
		pthread_exit(0);
	}
	RegionsCoord.UnitCount = j;

_StartCoordinate:
	//把所有路口的IS_Coordinated（参与协调标志清零）

	//先把主要路口的协调方案算出来
	GetMasterUnitPlan(_conn,_stmt );
	//计算其他路口的配时方案
	for(j=0;j<RegionsCoord.UnitCount;j++)
	{
		if(RegionsCoord.Unit[j].IS_MASTER == 2)
		{
			RegionsCoord.Unit[j].GetCoordinatePlan(j,_conn,_stmt);
		}
	}
	//下发方案

	sleep(30* 60);
goto _StartCoordinate;

	return (void *)0;
}

void GetMasterUnitPlan(Connection *_conn,Statement *_stmt )
{
	int i;
	//ResultSet *Result;
	//找到主要路口
	for(i=0;i<RegionsCoord.UnitCount;i++)
	{
		if(RegionsCoord.Unit[i].IS_MASTER ==1)
		{
			break;
		}
	}
	if(i == RegionsCoord.UnitCount)
	{
		printf("没有找到主要路口\n");
		return ;
	}

	//设置参与协调标志
	RegionsCoord.Unit[i].IS_Coordinated = 1;
	//查询主要路口阶段链  生成协调方案
	RegionsCoord.Unit[i].GetCoordinatePlan(i,_conn,_stmt);

	//设置主要路口周期  RegionsCoord.MasterUnitCycle   其他路口的周期要和主要路口的周期相同或一半

	//根据协调方向查找下一个路口，设置下一个路口的IS_Coordinated（是否参与协调标志）的值

}

/*
 * 计算ITC100信号机的协调方案
 */
void ITC_CoordinatePlan(int index,Connection *_conn,Statement *_stmt )
{
	ResultSet *Result;
	int i=index;		//i 路口下标
	int j=0,k=0;		//j 阶段下标    k 相位下标
	char sqlbuf[200];
	char Phase[100];
	char *PhaseID;
	try
	{
		//查询一个路口中各个阶段中包含的相位
		sprintf(sqlbuf,"select t.stage_no,t.stage_id,k.stage_phase from CFG_STAGE_CHAIN t ,ITC_CFG_STAGE k where t.unit_id=%d and t.unit_id=k.signal_id and t.stage_id = k.stage_id order by t.stage_no",RegionsCoord.Unit[i].ID);
		Result = _stmt->executeQuery(sqlbuf);
		while(Result->next() != 0)
		{
			RegionsCoord.Unit[i].Stage[j].num = Result->getInt(1);
			RegionsCoord.Unit[i].Stage[j].id = Result->getInt(2);
			memset(Phase,'\0',100);
			sprintf(Phase,Result->getString(3).c_str());
			k=0;
			PhaseID = strtok(Phase,",");
			while(PhaseID != NULL)
			{
				RegionsCoord.Unit[i].Stage[j].phase[k].id = atoi(PhaseID);
				k++;
				PhaseID = strtok(NULL,",");
			}
			RegionsCoord.Unit[i].Stage[j].phase_count = k;
			j++;
		}
		_stmt->closeResultSet(Result);
		RegionsCoord.Unit[i].StageCount = j;
		char detector_buf[50];
		//循环一个路口下的各个阶段（查询各个阶段中 所包含 各个相位 的流量 并计算放行时间）
		RegionsCoord.Unit[i].flow=0;
		for(j=0;j<RegionsCoord.Unit[i].StageCount;j++)
		{
			RegionsCoord.Unit[i].Stage[j].flow = 0;
			//（查询一个相位下 所有检测器在一段时间的流量数据 之和）
			for(k = 0; k<RegionsCoord.Unit[i].Stage[j].phase_count;k++)
			{
				RegionsCoord.Unit[i].Stage[j].phase[k].flow = 0;
				memset(sqlbuf,'\0',200);
				//查询相位中包含的检测器
				sprintf(sqlbuf,"select t.detector_apply from ITC_CFG_PHASE t where t.signal_id = %d and t.phase_id = %d",
						RegionsCoord.Unit[i].ID,RegionsCoord.Unit[i].Stage[j].phase[k].id);
				Result = _stmt->executeQuery(sqlbuf);
				if(Result->next() !=0)
				{
					memset(detector_buf,'\0',50);
					sprintf(detector_buf,Result->getString(1).c_str());
					_stmt->closeResultSet(Result);

					//查询一个相位下 所有检测器在一段时间的流量数据 之和
					memset(detector_buf,'\0',200);
					sprintf(sqlbuf,"select sum(t.flow) as sumFlow,avg(t.occ) from DETECTOR_REAL_DATA t where t.unit_id=%d and t.detector_id in(%s) and t.update_time>=sysdate -((1/24/60d)*(%d))",
							RegionsCoord.Unit[i].ID,
							detector_buf,
							RegionsCoord.PLAN_UPDATE_CYCLE );
					Result = _stmt->executeQuery(sqlbuf);
					if(Result->next() !=0)
					{
						RegionsCoord.Unit[i].Stage[j].phase[k].flow = Result->getInt(1);
					}
					_stmt->closeResultSet(Result);
				}
				else		//没有查询到相位下的检测器
				{

				}
				//计算阶段流量
				RegionsCoord.Unit[i].Stage[j].flow +=  RegionsCoord.Unit[i].Stage[j].phase[k].flow;
			}
			RegionsCoord.Unit[i].flow += RegionsCoord.Unit[i].Stage[j].flow;
		}
		//得到协调方案各阶段的放行时间
		for(j=0;j<RegionsCoord.Unit[i].StageCount;j++)
		{
			RegionsCoord.Unit[i].Stage[j].green_time = PlanTime * RegionsCoord.Unit[i].Stage[j].flow / RegionsCoord.Unit[i].flow;
		}

		//将方案保存到数据库

		//判断是不是协调路口  如果是就找到协调方向  根据协调方向将相邻下一个路口（该路口在协调区域范围内）设置成协调路口
		if(RegionsCoord.Unit[i].IS_Coordinated == 1)
		{

		}
	}
	catch(SQLException &ex)
	{
		cout << " Error Number : "<< ex.getErrorCode() << endl; //获得异常代码
		cout << ex.getMessage() << endl; //获得异常信息
	}
}
/*
 * 计算中科信号机的协调方案
 */
void ZKTD_CoordinatePlan(int index,Connection *_conn,Statement *_stmt )
{
	struct CANALIZATION_CFG Canalization[16];
	ResultSet *Result;

	int i=index;		//i 路口下标
	int j=0,k=0;		//j 阶段下标    k 相位下标
	int temp_t=0;
	char sqlbuf[200];
	char Phase[100];
	char *PhaseID;
	char canalization_buf[100];
	int canalization_id[16];
	int canalization_count;
	char *cana_id;


	struct RegionsCoord_ZKTD_Detector ZKTD_Detector[16];
	int detect_count=0;
	//int exit_dir_flow[16];
	//渠化ID 释义
	//			1	南左转			2	南调头			3	南直行			4	西左转			5	西直行			6	西右转			7	西调头			8	北左转
	//			9	北调头			10	北直行			11	北右转			12	东直行			13	南右转			14	东左转			15	东调头			16	东右转

	enum exit_dir
	{
		SW = 0,					//1	南左转
		ST	= 1,					//2	南调头
		SN = 2,					//3	南直行
		WN = 3,					//4	西左转
		WE = 4,					//5	西直行
		WS = 5,					//6	西右转
		WT = 6,					//7	西调头
		NE = 7,					//8	北左转
		NT = 8,					//9	北调头
		NS = 9,					//10	北直行
		NW = 10,					//11	北右转
		EW = 11,					//12	东直行
		SE = 12,					//13	南右转
		ES = 13,					//14	东左转
		ET = 14,					//15	东调头
		EN	= 15					//16	东右转
	};
	//查询路口所有相位的渠化关联ID
	sprintf(sqlbuf,"select WM_CONCAT(t.canalization_id) as cana_id from UNIT_PHASE_CANALIZATION_CFG t where t.signal_id=%d",RegionsCoord.Unit[i].ID);
	Result = _stmt->executeQuery(sqlbuf);
	if(Result->next() != 0)
	{
		memset(canalization_buf,'\0',sizeof(canalization_buf));
		sprintf(canalization_buf,Result->getString(1).c_str());
	}
	_stmt->closeResultSet(Result);
	cana_id = strtok(canalization_buf,",");
	temp_t = 0;
	while(cana_id != NULL)
	{
		canalization_id[temp_t] = atoi(cana_id);
		temp_t++;
		cana_id = strtok(NULL,",");
	}
	canalization_count = temp_t;


	//查询路口的检测器id和方向
	sprintf(sqlbuf,"select d.detector_id,d.direction from ITC_CFG_DETECTOR_DETAIL d where d.signal_id=%d and d.direction in("
              	  	  	  "select distinct c.direction_enter from UNIT_CANALIZATION_CFG c where c.id in(%s))",RegionsCoord.Unit[i].ID,canalization_buf);
	Result = _stmt->executeQuery(sqlbuf);
	temp_t = 0;
	while(Result->next() != 0)
	{
		ZKTD_Detector[temp_t].id = Result->getInt(1);
		ZKTD_Detector[temp_t].dir = Result->getInt(2);
		temp_t++;
	}
	_stmt->closeResultSet(Result);
	detect_count = temp_t;

	//查询检测器流量和计算相关出口方向流量
	for(temp_t=0;temp_t < detect_count;temp_t++)
	{
		memset(sqlbuf,'\0',sizeof(sqlbuf));
		sprintf(sqlbuf,"select sum(t.flow),avg(t.occ) from DETECTOR_REAL_DATA t where t.unit_id=%d and t.detector_id = %d and t.update_time>=sysdate -((1/24/60)*30)",
							RegionsCoord.Unit[i].ID,ZKTD_Detector[temp_t].id);
		Result = _stmt->executeQuery(sqlbuf);
		if(Result->next() != 0)
		{
			ZKTD_Detector[temp_t].flow = Result->getInt(1);
			_stmt->closeResultSet(Result);
			int temp_m;
			if(ZKTD_Detector[temp_t].flow != 0)   //流量不等于0
			{
				switch(ZKTD_Detector[temp_t].dir)
				{
					case 1:			//入口方向是东
					{
						//循环所有的渠化ID,通过ID判断出口方向，得出流量分配比例
						for(temp_m=0;temp_m<canalization_count;temp_m++)
						{
							if(canalization_id[temp_m] == (EN+1))     		//东右转
							{
								RegionsCoord.Unit[i].ZKTD_LaneFlow[EN] = 	ZKTD_Detector[temp_t].flow * right_percent;
							}
							else if(canalization_id[temp_m] == (EW+1))		//东直行
							{
								RegionsCoord.Unit[i].ZKTD_LaneFlow[EW] = 	ZKTD_Detector[temp_t].flow * straight_percent;
							}
							else if(canalization_id[temp_m] == (ES+1))		//东左转
							{
								RegionsCoord.Unit[i].ZKTD_LaneFlow[ES] = 	ZKTD_Detector[temp_t].flow * straight_percent;
							}
						}
						if( (RegionsCoord.Unit[i].ZKTD_LaneFlow[EN] == 0) && (RegionsCoord.Unit[i].ZKTD_LaneFlow[EW] !=0) && (RegionsCoord.Unit[i].ZKTD_LaneFlow[ES] != 0) )	//没有右转 将右转流量加到直行
						{
							RegionsCoord.Unit[i].ZKTD_LaneFlow[EW] = ZKTD_Detector[temp_t].flow * (right_percent + straight_percent);
						}
						else if((RegionsCoord.Unit[i].ZKTD_LaneFlow[EN] != 0) && (RegionsCoord.Unit[i].ZKTD_LaneFlow[EW] ==0) && (RegionsCoord.Unit[i].ZKTD_LaneFlow[ES] != 0))	//没有直行  将直行数据加到左转
						{
							RegionsCoord.Unit[i].ZKTD_LaneFlow[ES] = ZKTD_Detector[temp_t].flow * (left_percent + straight_percent);
						}
						else if((RegionsCoord.Unit[i].ZKTD_LaneFlow[EN] != 0) && (RegionsCoord.Unit[i].ZKTD_LaneFlow[EW] !=0) && (RegionsCoord.Unit[i].ZKTD_LaneFlow[ES] == 0))    //没有左转  将左转数据加到直行
						{
							RegionsCoord.Unit[i].ZKTD_LaneFlow[EW] = ZKTD_Detector[temp_t].flow * (left_percent + straight_percent);
						}
						else if((RegionsCoord.Unit[i].ZKTD_LaneFlow[EN] == 0) && (RegionsCoord.Unit[i].ZKTD_LaneFlow[EW] !=0) && (RegionsCoord.Unit[i].ZKTD_LaneFlow[ES] == 0))		//没有转弯，只有直行
						{
							RegionsCoord.Unit[i].ZKTD_LaneFlow[EW] = ZKTD_Detector[temp_t].flow;
						}
						break;
					}
					case 2:			//入口方向是西
					{
						//循环所有的渠化ID,通过ID判断出口方向，得出流量分配比例
						for(temp_m=0;temp_m<canalization_count;temp_m++)
						{
							if(canalization_id[temp_m] == (WS+1))     		//西右转
							{
								RegionsCoord.Unit[i].ZKTD_LaneFlow[WS] = 	ZKTD_Detector[temp_t].flow * right_percent;
							}
							else if(canalization_id[temp_m] == (WE+1))		//西直行
							{
								RegionsCoord.Unit[i].ZKTD_LaneFlow[WE] = 	ZKTD_Detector[temp_t].flow * straight_percent;
							}
							else if(canalization_id[temp_m] == (WN+1))		//西左转
							{
								RegionsCoord.Unit[i].ZKTD_LaneFlow[WN] = 	ZKTD_Detector[temp_t].flow * straight_percent;
							}
						}
						if( (RegionsCoord.Unit[i].ZKTD_LaneFlow[WS] == 0) && (RegionsCoord.Unit[i].ZKTD_LaneFlow[WE] !=0) && (RegionsCoord.Unit[i].ZKTD_LaneFlow[WN] != 0) )	//没有右转 将右转流量加到直行
						{
							RegionsCoord.Unit[i].ZKTD_LaneFlow[WE] = ZKTD_Detector[temp_t].flow * (right_percent + straight_percent);
						}
						else if((RegionsCoord.Unit[i].ZKTD_LaneFlow[WS] != 0) && (RegionsCoord.Unit[i].ZKTD_LaneFlow[WE] ==0) && (RegionsCoord.Unit[i].ZKTD_LaneFlow[WN] != 0))	//没有直行  将直行数据加到左转
						{
							RegionsCoord.Unit[i].ZKTD_LaneFlow[WN] = ZKTD_Detector[temp_t].flow * (left_percent + straight_percent);
						}
						else if((RegionsCoord.Unit[i].ZKTD_LaneFlow[WS] != 0) && (RegionsCoord.Unit[i].ZKTD_LaneFlow[WE] !=0) && (RegionsCoord.Unit[i].ZKTD_LaneFlow[WN] == 0))    //没有左转  将左转数据加到直行
						{
							RegionsCoord.Unit[i].ZKTD_LaneFlow[WE] = ZKTD_Detector[temp_t].flow * (left_percent + straight_percent);
						}
						else if((RegionsCoord.Unit[i].ZKTD_LaneFlow[WS] == 0) && (RegionsCoord.Unit[i].ZKTD_LaneFlow[WE] !=0) && (RegionsCoord.Unit[i].ZKTD_LaneFlow[WN] == 0))		//没有转弯，只有直行
						{
							RegionsCoord.Unit[i].ZKTD_LaneFlow[WE] = ZKTD_Detector[temp_t].flow;
						}
						break;
					}
					case 3:			//入口方向是南
					{
						//循环所有的渠化ID,通过ID判断出口方向，得出流量分配比例
						for(temp_m=0;temp_m<canalization_count;temp_m++)
						{
							if(canalization_id[temp_m] == (SE+1))     		//南右转
							{
								RegionsCoord.Unit[i].ZKTD_LaneFlow[SE] = 	ZKTD_Detector[temp_t].flow * right_percent;
							}
							else if(canalization_id[temp_m] == (SN+1))		//南直行
							{
								RegionsCoord.Unit[i].ZKTD_LaneFlow[SN] = 	ZKTD_Detector[temp_t].flow * straight_percent;
							}
							else if(canalization_id[temp_m] == (SW+1))		//南左转
							{
								RegionsCoord.Unit[i].ZKTD_LaneFlow[SW] = 	ZKTD_Detector[temp_t].flow * straight_percent;
							}
						}
						if( (RegionsCoord.Unit[i].ZKTD_LaneFlow[SE] == 0) && (RegionsCoord.Unit[i].ZKTD_LaneFlow[SN] !=0) && (RegionsCoord.Unit[i].ZKTD_LaneFlow[SW] != 0) )	//没有右转 将右转流量加到直行
						{
							RegionsCoord.Unit[i].ZKTD_LaneFlow[SN] = ZKTD_Detector[temp_t].flow * (right_percent + straight_percent);
						}
						else if((RegionsCoord.Unit[i].ZKTD_LaneFlow[SE] != 0) && (RegionsCoord.Unit[i].ZKTD_LaneFlow[SN] ==0) && (RegionsCoord.Unit[i].ZKTD_LaneFlow[SW] != 0))	//没有直行  将直行数据加到左转
						{
							RegionsCoord.Unit[i].ZKTD_LaneFlow[SW] = ZKTD_Detector[temp_t].flow * (left_percent + straight_percent);
						}
						else if((RegionsCoord.Unit[i].ZKTD_LaneFlow[SE] != 0) && (RegionsCoord.Unit[i].ZKTD_LaneFlow[SN] !=0) && (RegionsCoord.Unit[i].ZKTD_LaneFlow[SW] == 0))    //没有左转  将左转数据加到直行
						{
							RegionsCoord.Unit[i].ZKTD_LaneFlow[SN] = ZKTD_Detector[temp_t].flow * (left_percent + straight_percent);
						}
						else if((RegionsCoord.Unit[i].ZKTD_LaneFlow[SE] == 0) && (RegionsCoord.Unit[i].ZKTD_LaneFlow[SN] !=0) && (RegionsCoord.Unit[i].ZKTD_LaneFlow[SW] == 0))		//没有转弯，只有直行
						{
							RegionsCoord.Unit[i].ZKTD_LaneFlow[SN] = ZKTD_Detector[temp_t].flow;
						}
						break;
					}
					case 4:			//入口方向是北
					{
						//循环所有的渠化ID,通过ID判断出口方向，得出流量分配比例
						for(temp_m=0;temp_m<canalization_count;temp_m++)
						{
							if(canalization_id[temp_m] == (NW+1))     		//北右转
							{
								RegionsCoord.Unit[i].ZKTD_LaneFlow[NW] = ZKTD_Detector[temp_t].flow * right_percent;
							}
							else if(canalization_id[temp_m] == (NS+1))		//北直行
							{
								RegionsCoord.Unit[i].ZKTD_LaneFlow[NS] = 	ZKTD_Detector[temp_t].flow * straight_percent;
							}
							else if(canalization_id[temp_m] == (NE+1))		//北左转
							{
								RegionsCoord.Unit[i].ZKTD_LaneFlow[NE] = 	ZKTD_Detector[temp_t].flow * straight_percent;
							}
						}
						if( (RegionsCoord.Unit[i].ZKTD_LaneFlow[NW] == 0) && (RegionsCoord.Unit[i].ZKTD_LaneFlow[NS] !=0) && (RegionsCoord.Unit[i].ZKTD_LaneFlow[NE] != 0) )	//没有右转 将右转流量加到直行
						{
							RegionsCoord.Unit[i].ZKTD_LaneFlow[NS] = ZKTD_Detector[temp_t].flow * (right_percent + straight_percent);
						}
						else if((RegionsCoord.Unit[i].ZKTD_LaneFlow[NW] != 0) && (RegionsCoord.Unit[i].ZKTD_LaneFlow[NS] ==0) && (RegionsCoord.Unit[i].ZKTD_LaneFlow[NE] != 0))	//没有直行  将直行数据加到左转
						{
							RegionsCoord.Unit[i].ZKTD_LaneFlow[NE] = ZKTD_Detector[temp_t].flow * (left_percent + straight_percent);
						}
						else if((RegionsCoord.Unit[i].ZKTD_LaneFlow[NW] != 0) && (RegionsCoord.Unit[i].ZKTD_LaneFlow[NS] !=0) && (RegionsCoord.Unit[i].ZKTD_LaneFlow[NE] == 0))    //没有左转  将左转数据加到直行
						{
							RegionsCoord.Unit[i].ZKTD_LaneFlow[NS] = ZKTD_Detector[temp_t].flow * (left_percent + straight_percent);
						}
						else if((RegionsCoord.Unit[i].ZKTD_LaneFlow[NW] == 0) && (RegionsCoord.Unit[i].ZKTD_LaneFlow[NS] !=0) && (RegionsCoord.Unit[i].ZKTD_LaneFlow[NE] == 0))		//没有转弯，只有直行
						{
							RegionsCoord.Unit[i].ZKTD_LaneFlow[NS] = ZKTD_Detector[temp_t].flow;
						}
						break;
					}
				}//END 	switch(ZKTD_Detector[temp_t].dir)
			}//END	if(ZKTD_Detector[temp_t].flow != 0)
		}//END	if(Result->next() != 0)
	}//END	for(temp_t=0;temp_t < detect_count;temp_t++)

	//查询阶段链中包含的相位  和相位中包含的通道

	//查询一个路口中包含的阶段（中科理解为相位）
	sprintf(sqlbuf,"select t.stage_no,t.stage_id from CFG_STAGE_CHAIN t  where t.unit_id=%d  order by t.stage_no",RegionsCoord.Unit[i].ID);
	Result = _stmt->executeQuery(sqlbuf);
	while(Result->next() != 0)
	{
		RegionsCoord.Unit[i].Stage[j].num = Result->getInt(1);
		RegionsCoord.Unit[i].Stage[j].id = Result->getInt(2);
		j++;
	}
	_stmt->closeResultSet(Result);
	RegionsCoord.Unit[i].StageCount = j;

	char temp_buf[20];
	for(j = 0; j<RegionsCoord.Unit[i].StageCount;j++)
	{
		memset(sqlbuf,'\0',sizeof(sqlbuf));
		//通过渠化关联,查询放行相位包含的ID，然后配合上面得到的各个检测器出口流量分配的值，算出各个阶段的流量
		sprintf(sqlbuf,"select canalization_id from UNIT_PHASE_CANALIZATION_CFG t where t.signal_id=%d and t.phase_id=%d",RegionsCoord.Unit[i].ID);
		Result = _stmt->executeQuery(sqlbuf);
		if(Result->next() != 0)
		{
			memset(RegionsCoord.Unit[i].Stage[j].ZKTD_Lane,'\0',20);
			sprintf(RegionsCoord.Unit[i].Stage[j].ZKTD_Lane,Result->getString(1).c_str());
			_stmt->closeResultSet(Result);
			memset(temp_buf,'\0',sizeof(temp_buf));
			sprintf(temp_buf,RegionsCoord.Unit[i].Stage[j].ZKTD_Lane);
			cana_id = strtok(temp_buf,",");
			RegionsCoord.Unit[i].Stage[j].flow = 0;
			while(cana_id != NULL)
			{
				temp_t = 0;
				temp_t = atoi(cana_id);
				if((temp_t >= 0) && (temp_t <= 16 ))		//渠化关联ID有效
				{
					RegionsCoord.Unit[i].Stage[j].flow = RegionsCoord.Unit[i].Stage[j].flow + RegionsCoord.Unit[i].ZKTD_LaneFlow[temp_t-1];
				}
				cana_id = strtok(NULL,",");
			}
		}
		else
		{
			//待协商处理
			_stmt->closeResultSet(Result);
		}
	}



//	memset(sqlbuf,'\0',sizeof(sqlbuf));
//	//查询相位中各个入口的流量
//	sprintf(sqlbuf,"select t.detector_id,sum(t.flow),avg(t.occ) from DETECTOR_REAL_DATA t where t.unit_id=%d and t.detector_id in("
//							"select d.detector_id from ITC_CFG_DETECTOR_DETAIL d where d.signal_id=t.unit_id and d.direction in("
//								"select distinct c.direction_enter from UNIT_CANALIZATION_CFG c where c.id in(%s)))"
//						"and t.update_time>=sysdate -((1/24/60)*%d) group by t.detector_id;",RegionsCoord.Unit[i].ID,
//																													RegionsCoord.Unit[i].Stage[j].ZK_PhaseDir,
//																													RegionsCoord.PLAN_UPDATE_CYCLE );
//
//	Result = _stmt->executeQuery(sqlbuf);
//	while(Result->next() != 0)
//	{
//
//	}

}

/*
 * 根据当前路口ID和协调方向查找下一个要协调的路口
 */
void SetNextUnitCoordinate(int index,Connection *_conn,Statement *_stmt )
{
	int i = index;
	ResultSet *Result;
	char sqlbuf[200];
	int next_unit_id = -1;
	sprintf(sqlbuf,"select t.connector_unit_id from UNIT_CONNECTOR t where t.unit_id = %d and t.unit_direction = %d",RegionsCoord.Unit[i].ID,RegionsCoord.Unit[i].Coordinate_Dir);
	try
	{
		Result = _stmt->executeQuery(sqlbuf);
		if(Result->next() != 0)
		{
			next_unit_id = Result->getInt(1);
		}
		_stmt->closeResultSet(Result);
		for(i = 0;i<RegionsCoord.UnitCount;i++)
		{
			if(RegionsCoord.Unit[i].ID == next_unit_id)
			{
				//如果要协调的路口已经被设置成协调路口
				if(RegionsCoord.Unit[i].IS_Coordinated == 1)
				{
					return ;
				}
				else
				{
					RegionsCoord.Unit[i].IS_Coordinated = 1;
					break;
				}
			}
		}
	}
	catch(SQLException &ex)
	{
		cout << " Error Number : "<< ex.getErrorCode() << endl; //获得异常代码
		cout << ex.getMessage() << endl; //获得异常信息
	}
}
