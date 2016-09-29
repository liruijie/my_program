/*
 * ControlSignal.cpp
 *
 *  Created on: 2016年8月29日
 *      Author: root
 */


#include "include.h"


extern StatelessConnectionPool *pConnPool;
//extern oracle::occi::Environment *OraEnviroment;
Connection *ctr_conn;
Statement *ctr_stmt;
int Socket_Ctr;              //
struct sockaddr_in CmdAddr;
struct sockaddr_in HeartBeatAddr;
extern char ITC_ServerAddr[20];

void *ControlSignal(void *arg)
{
	int i;
	time_t time_now;

	printf("Creat thread ControlSignal success,%s,%s,%d\n",__FILE__,__FUNCTION__,__LINE__);
	sleep(1);
	Socket_Ctr = InitControlSocket();
	if( Socket_Ctr < 0 )
	{
		perror("socket::");
		return (void *)0;
	}
	while(1)
	{
		ctr_conn=pConnPool->getConnection();
		if(ctr_conn)
		{
			 ctr_stmt = ctr_conn->createStatement();
			 ctr_stmt->setAutoCommit(true);
			 break;
		}
		sleep(1);
	}

	while(1)
	{
		time ( &time_now );
		for(i=0;i<SignalMaxNum;i++)
		{
			if( (SignalRealData[i].CrossID == -1) || (SignalRealData[i].SignalState == Offline) )
			{
				continue;
			}
			 //没有被控制 , 判断是否需要进行控制
			if(SignalRealData[i].IsControlled == false)
			{
					if(SignalRealData[i].ControlFunction == Ctr_Idle)           //当前控制状态:空闲
					{
						if(SignalRealData[i].IsExistLockTask == true)              //
						{
							if( (SignalRealData[i].LockStartTime == 0) || (SignalRealData[i].LockStartTime == time_now) )          //需要锁定
							{
								SignalRealData[i].ControlFunction = Ctr_LockTraffic;
								SignalRealData[i].IsExistLockTask = false;
								if(Control_LockTraffic(SignalRealData[i].CrossID)==succeed)
								{
									SignalRealData[i].IsControlled = true;
								}
							}
						}
					}
					else if(SignalRealData[i].ControlFunction == Ctr_ControlMode)
					{
						if(Control_ControlMode(i) == succeed)
						{
							SignalRealData[i].IsControlled = true;
						}
					}
					else if(SignalRealData[i].ControlFunction == Ctr_CrossPlan)
					{
						if(Control_CrossPlan(i) == succeed)
						{
							SignalRealData[i].IsControlled = true;
						}
					}
					else if(SignalRealData[i].ControlFunction == Ctr_CrossStage)
					{
						if(Control_CrossStage(i) == succeed)
						{
							SignalRealData[i].IsControlled = true;
						}
					}
			}
			//处在之中控制 , 判断是否要取消控制，不取消控制的就发送控制心跳报文
			else
			{
					if(SignalRealData[i].ControlFunction == Ctr_LockTraffic)
					{
						if(SignalRealData[i].LockDuration == 0 || ((SignalRealData[i].LockStartTime + SignalRealData[i].LockDuration) <time_now ))
						{
							SendControlHeartBeat(SignalRealData[i].CrossID);
							continue;
						}
						else
						{
							unsigned char buf[] = {0x8E, 0x00, 0x0E, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x50, 0xC0, 0x01, 0x00, 0x00, 0x00, 0x8D};
							buf[7] = (SignalRealData[i].CrossID & 0xFF00)>>8;
							buf[8] = (SignalRealData[i].CrossID & 0x00FF);
							buf[14] = (MakeCheckSum( buf ) & 0xFF);
							sendto(Socket_Ctr,buf,sizeof(buf),0,(struct sockaddr *)&CmdAddr, sizeof(CmdAddr));
							SignalRealData[i].IsControlled = false;
						}
					}
					else if(SignalRealData[i].ControlFunction == Ctr_UnLockTraffic)
					{//8e 00 0e 00 01 00 00 03 e8 50 c0 01 00 00 0b 8d
						unsigned char buf[] = {0x8E, 0x00, 0x0E, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x50, 0xC0, 0x01, 0x00, 0x00, 0x00, 0x8D};
						buf[7] = (SignalRealData[i].CrossID & 0xFF00)>>8;
						buf[8] = (SignalRealData[i].CrossID & 0x00FF);
						buf[14] = (MakeCheckSum( buf ) & 0xFF);
						sendto(Socket_Ctr,buf,sizeof(buf),0,(struct sockaddr *)&CmdAddr, sizeof(CmdAddr));

						SignalRealData[i].ControlFunction = Ctr_Idle;
						SignalRealData[i].IsControlled = false;
					}
					else if(SignalRealData[i].ControlFunction == Cancel_ControlMode)
					{
						unsigned char buf[] = {0x8E, 0x00, 0x0E, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x50, 0xF2, 0x01, 0x00, 0xC3, 0x00, 0x8D};
						buf[7] = (SignalRealData[i].CrossID & 0xFF00)>>8;
						buf[8] = (SignalRealData[i].CrossID & 0x00FF);

						buf[14] = (MakeCheckSum( buf ) & 0xFF);
						sendto(Socket_Ctr,buf,sizeof(buf),0,(struct sockaddr *)&CmdAddr, sizeof(CmdAddr));

						SignalRealData[i].ControlFunction = Ctr_Idle;
						SignalRealData[i].IsControlled = false;
					}
					else if(SignalRealData[i].ControlFunction == Cancel_CrossPlan)
					{
						unsigned char buf[] = {0x8E, 0x00, 0x0E, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x50, 0xF2, 0x01, 0x00, 0xC2, 0x00, 0x8D};
						buf[7] = (SignalRealData[i].CrossID & 0xFF00)>>8;
						buf[8] = (SignalRealData[i].CrossID & 0x00FF);
						buf[14] = (MakeCheckSum( buf ) & 0xFF);
						sendto(Socket_Ctr,buf,sizeof(buf),0,(struct sockaddr *)&CmdAddr, sizeof(CmdAddr));

						SignalRealData[i].ControlFunction = Ctr_Idle;
						SignalRealData[i].IsControlled = false;
					}
					else if(SignalRealData[i].ControlFunction == Cancel_CrossStage)
					{
						unsigned char buf[] = {0x8E, 0x00, 0x0E, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x50, 0xC0, 0x01, 0x00, 0x00, 0x00, 0x8D};
						buf[7] = (SignalRealData[i].CrossID & 0xFF00)>>8;
						buf[8] = (SignalRealData[i].CrossID & 0x00FF);
						buf[14] = (MakeCheckSum( buf ) & 0xFF);
						sendto(Socket_Ctr,buf,sizeof(buf),0,(struct sockaddr *)&CmdAddr, sizeof(CmdAddr));

						SignalRealData[i].ControlFunction = Ctr_Idle;
						SignalRealData[i].IsControlled = false;
					}
					else
					{
						SendControlHeartBeat(SignalRealData[i].CrossID);
					}
			}
		}
		usleep(800*100);
	}
}
int InitControlSocket()
{
	//char IP_1[] = "192.168.1.225";
	int ctr_socket;
	if ((ctr_socket = socket(PF_INET, SOCK_DGRAM, 0)) < 0)
	{
		perror("fail to socket");
		return -1;
	}
	int on=1;
	if((setsockopt(ctr_socket,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(on)))<0)
	{
		perror("setsockopt failed");
		return -1;
	}
	CmdAddr.sin_family = AF_INET;
	CmdAddr.sin_addr.s_addr = inet_addr(ITC_ServerAddr);
	CmdAddr.sin_port = htons(10001);

	HeartBeatAddr.sin_family = AF_INET;
	HeartBeatAddr.sin_addr.s_addr = inet_addr(ITC_ServerAddr);
	HeartBeatAddr.sin_port = htons(10000);

	return ctr_socket;
}

int Control_LockTraffic(int index)
{
	if(ChangeOracleFlag(SignalRealData[index].CrossID) == fail)
	{
		SignalRealData[index].ControlFunction = Ctr_Idle;
		return fail;
	}
	else
	{
		SendControlHeartBeat(SignalRealData[index].CrossID);
		//					 			 8e    00    0e    00    01    00    00    03   e8     50   c0     01    00    01    0c 	 8d
		unsigned char buf[] = {0x8E, 0x00, 0x0E, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x50, 0xC0, 0x01, 0x00, 0x00, 0x00, 0x8D};
		buf[7] = (SignalRealData[index].CrossID & 0xFF00)>>8;
		buf[8] = (SignalRealData[index].CrossID & 0x00FF);
		buf[12] = (SignalRealData[index].LockStageNum & 0xFF00)>>8;
		buf[13] = (SignalRealData[index].LockStageNum & 0x00FF);
		buf[14] = (MakeCheckSum( buf ) & 0xFF);
		sendto(Socket_Ctr,buf,sizeof(buf),0,(struct sockaddr *)&CmdAddr, sizeof(CmdAddr));
		return succeed;
	}
}

int Control_ControlMode(int index)
{
	if(ChangeOracleFlag(SignalRealData[index].CrossID) == fail)
	{
		SignalRealData[index].ControlFunction = Ctr_Idle;
		return fail;
	}
	else
	{
		SendControlHeartBeat(SignalRealData[index].CrossID);
		//					 			 8e    00    0e    00    01    00    00    03   e8     50   c3     01    00    01    0c 	 8d
		unsigned char buf[] = {0x8E, 0x00, 0x0E, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x50, 0xC3, 0x01, 0x00, 0x00, 0x00, 0x8D};
		buf[7] = (SignalRealData[index].CrossID & 0xFF00)>>8;
		buf[8] = (SignalRealData[index].CrossID & 0x00FF);
		buf[12] = (SignalRealData[index].CrossContolMode & 0xFF00)>>8;
		buf[13] = (SignalRealData[index].CrossContolMode & 0x00FF);
		buf[14] = (MakeCheckSum( buf ) & 0xFF);
		sendto(Socket_Ctr,buf,sizeof(buf),0,(struct sockaddr *)&CmdAddr, sizeof(CmdAddr));
		return succeed;
	}
}
int Control_CrossPlan(int index)
{
	if(ChangeOracleFlag(SignalRealData[index].CrossID) == fail)
	{
		SignalRealData[index].ControlFunction = Ctr_Idle;
		return fail;
	}
	else
	{
		SendControlHeartBeat(SignalRealData[index].CrossID);
		//					 			 8e    00    0e    00    01    00    00    03   e8     50   c3     01    00    01    0c 	 8d
		unsigned char buf[] = {0x8E, 0x00, 0x0E, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x50, 0xC3, 0x01, 0x00, 0x00, 0x00, 0x8D};
		buf[7] = (SignalRealData[index].CrossID & 0xFF00)>>8;
		buf[8] = (SignalRealData[index].CrossID & 0x00FF);
		buf[12] = (SignalRealData[index].CrossContolMode & 0xFF00)>>8;
		buf[13] = (SignalRealData[index].CrossContolMode & 0x00FF);
		buf[14] = (MakeCheckSum( buf ) & 0xFF);
		sendto(Socket_Ctr,buf,sizeof(buf),0,(struct sockaddr *)&CmdAddr, sizeof(CmdAddr));
		return succeed;

	}
}
int Control_CrossStage(int index)
{
	if(ChangeOracleFlag(SignalRealData[index].CrossID) == fail)
	{
		SignalRealData[index].ControlFunction = Ctr_Idle;
		return fail;
	}
	else
	{
		SendControlHeartBeat(SignalRealData[index].CrossID);
		//					 			 8e    00    0e    00    01    00    00    03   e8     50   c3     01    00    01    0c 	 8d
		unsigned char buf[] = {0x8E, 0x00, 0x0E, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x50, 0xC3, 0x01, 0x00, 0x00, 0x00, 0x8D};
		buf[7] = (SignalRealData[index].CrossID & 0xFF00)>>8;
		buf[8] = (SignalRealData[index].CrossID & 0x00FF);
		buf[12] = (SignalRealData[index].CrossContolMode & 0xFF00)>>8;
		buf[13] = (SignalRealData[index].CrossContolMode & 0x00FF);
		buf[14] = (MakeCheckSum( buf ) & 0xFF);
		sendto(Socket_Ctr,buf,sizeof(buf),0,(struct sockaddr *)&CmdAddr, sizeof(CmdAddr));
		return succeed;
	}
}


int ChangeOracleFlag(int SignalID)
{
	char sqlbuf[200];
	int control_flag;
	sprintf(sqlbuf,"select t.control_flag from CONTROL_CMD t where t.signal_id = %d ",SignalID);
	ResultSet *Result;
	try
	{
		ctr_stmt->setSQL(sqlbuf);
		Result = ctr_stmt->executeQuery();
		if(Result->next() != 0)                //cun zai
		{
			control_flag = Result->getInt(1);
			ctr_stmt->closeResultSet(Result);
			if(control_flag == 0)
			{
				sprintf(sqlbuf,"update CONTROL_CMD t set t.control_flag = 1   where t.signal_id =  %d  ",SignalID);
				ctr_stmt->setSQL(sqlbuf);
				ctr_stmt->executeUpdate();
				return succeed;
			}
			else
			{
				return fail;
			}
		}
		else
		{
			sprintf(sqlbuf,"insert into CONTROL_CMD  (signal_id,CONTROL_FLAG) values(%d,1)",SignalID);
			ctr_stmt->setSQL(sqlbuf);
			ctr_stmt->executeUpdate();
			return succeed;
		}

	}
	catch(SQLException &sqlExcp)
	{
		cout << " Error Number : "<< sqlExcp.getErrorCode() << endl; //获得异常代码
		cout << sqlExcp.getMessage() << endl; //获得异常信息
		return fail;
	}
	return fail;
}

void SendControlHeartBeat(int SignalID)
{
	//8E 00 0D 00 01 00 00 04 4D 60 00 00 00 BF 8D
	unsigned char buf[] = {0x8E, 0x00, 0x0E, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x60, 0x00, 0x01, 0x00, 0x00, 0x00, 0x8D};
	buf[7] = (SignalID & 0xFF00)>>8;
	buf[8] = (SignalID & 0x00FF);
	buf[14] = (MakeCheckSum( buf ) & 0xFF);
	sendto(Socket_Ctr,buf,sizeof(buf),0,(struct sockaddr *)&HeartBeatAddr, sizeof(HeartBeatAddr));
}
int MakeCheckSum(unsigned char *rcv_buf)
{
	int i;
	int rcv_len = rcv_buf[1] * 256 + rcv_buf[2];
	int check_sum = 0;
	for (i = 1; i < rcv_len; i++)
	{
		check_sum += rcv_buf[i];
	}
	return check_sum;
}

