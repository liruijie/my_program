/*
 * ControlCenterCommt.cpp
 *
 *  Created on: 2016年7月5日
 *      Author: root
 */
#include "include.h"
#define ControlCenterPort   10186
#define TcpTimeOutFlag		 -1
#define TcpRecvError			 -2


struct TcpThreadInfo TcpThread[MaxConnect];
extern StatelessConnectionPool *pConnPool;
extern oracle::occi::Environment *OraEnviroment;
void *ControlCenterCommt(void *arg)
{
	printf("Creat thread ControlCenterCommt success,%s,%s,%d\n",__FILE__,__FUNCTION__,__LINE__);
	sleep(1);
	Tcp();
	return (void *)0;
}

void Tcp()
{
		int sockfd, commt_fd; 		/* 监听端口描述符，通讯描述符 */
		int flags;
		int sin_size;
		struct sockaddr_in my_addr; 	/* 自身的地址信息 */
		struct sockaddr_in client_addr; 	/* 连接对方的地址信息 */
		//struct timeval 	timeVal;
		fd_set  fdSet;
		int ret;
		int ConnectNum;
		int flag = 1;
		//char buf[1000];
		//pthread_t ConnectThread[MaxConnect];
		if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
		{
			perror("socket");
			exit(1);
		}
		if( setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag)) == -1)
		{
			perror("setsockopt");
			exit(1);
		}



		my_addr.sin_family = AF_INET;
		my_addr.sin_port = htons(ControlCenterPort); 		/* 网络字节顺序 */
		my_addr.sin_addr.s_addr = INADDR_ANY; 					/* 自动填本机IP */
		bzero(&(my_addr.sin_zero), 8); 							/* 其余部分置0 */
		if (bind(sockfd, (struct sockaddr *)&my_addr,sizeof(struct sockaddr)) == -1)
		{
			perror("bind");
			exit(1);
		}
		if (listen(sockfd, 1) == -1)
		{
		perror("listen");
		exit(1);
		}
		flags = fcntl(sockfd, F_GETFL);
		if( (flags<0)||((fcntl(sockfd, F_SETFL, flags |O_ASYNC))<0) )
		{
			perror("fcntl\n");
			exit(1);
		}
		while(1)
		{
			FD_ZERO(&fdSet);
			FD_SET(sockfd, &fdSet);

			ret = select((int) sockfd + 1, &fdSet,NULL, NULL, (struct timeval *)0);           //阻塞，等待请求连接
			//printf("%d\n",FD_ISSET(sockfd,&fdSet));
			if( ret == 0 )    //select timeout
			{
				continue;
			}
			if( !ret )    //select error
			{
				printf("sockfd select failed with result=%d\n",ret);
				continue;
			}
			else          //select success
			{
				ConnectNum = GetEmptyThread();
				if( -1 != ConnectNum)            //you kong xian lian jie
				{
					printf("stuct %d is empty\n",ConnectNum);
					TcpThread[ConnectNum].IsConnect = true;
					sin_size = sizeof(struct sockaddr_in);
					commt_fd = accept(sockfd, (struct sockaddr *)&client_addr, (socklen_t *)&sin_size);
					if(commt_fd == -1)
					 {
						perror("accept");
						continue;
					 }
					TcpThread[ConnectNum].commt_fd = commt_fd;
					ret = pthread_create(&TcpThread[ConnectNum].thread_id,NULL,TcpCommunicate,&ConnectNum);
					printf("new commt_fd is %d\n",commt_fd);
					printf("Got connection from %s:%d\n", inet_ntoa( client_addr.sin_addr ),ntohs(client_addr.sin_port));
				}
				else
				{
					commt_fd = accept(sockfd, (struct sockaddr *)&client_addr, (socklen_t *)&sin_size);
					close(commt_fd);
				}
			}
		}
}

void *TcpCommunicate(void *arg)
{
	int ThreadNum = *(int *)arg;
	//char buf[1000];
	int ret;

	pthread_detach(pthread_self());
	ret = InitTCPLink(ThreadNum);
	if(ret == fail)
		pthread_exit(NULL);

	printf("This is a new TCP Connection thread,ThreadNum = %d,Tcp_Connection fd = %d\n",ThreadNum,TcpThread[ThreadNum].commt_fd);
	while(true)
	{
		memset(TcpThread[ThreadNum].Message,'\0',SingleRecvMaxLen);
		ret = TCPService_Data_Receive(TcpThread[ThreadNum].commt_fd,TcpThread[ThreadNum].Message);
		if(ret>0)
		{
			//printf("process id = %d,commt fd = %d\n",ThreadNum,TcpThread[ThreadNum].commt_fd);
			printf("process id = %d,commt fd = %d  \nreceive data:\n%s\n",ThreadNum,TcpThread[ThreadNum].commt_fd,TcpThread[ThreadNum].Message);
			ReadXmlFromBuf(ThreadNum);
		}
		else if(ret==TcpRecvError || ret == TcpTimeOutFlag)     //TCP Error
		{
			printf("TCP error\n");
			printf("Thread process id=%d will close the TCPLink num = %d\n",ThreadNum,TcpThread[ThreadNum].commt_fd);
			close(TcpThread[ThreadNum].commt_fd);
			TcpThread[ThreadNum].IsConnect = false;
			TcpThread[ThreadNum].conn->terminateStatement(TcpThread[ThreadNum].stmt);
			OraEnviroment->terminateConnection(TcpThread[ThreadNum].conn);
			memset(TcpThread+ThreadNum,'\0',sizeof(struct TcpThreadInfo));
			pthread_exit(NULL);
		}
	}
	return (void *)0;
}




int TCPService_Data_Receive(int Rec_fd,char *Rec_buf)
{
	struct timeval 	Rec_timeVal;
	fd_set  Rec_fdSet;
	int ret,numbytes;

	//printf("here is TCPService_Data_Receive process,process ID %d,commt_fd = %d\n",getpid(),Rec_fd);
	Rec_timeVal.tv_sec = 60;
	Rec_timeVal.tv_usec = 0;

	FD_ZERO(&Rec_fdSet);
	FD_SET(Rec_fd, &Rec_fdSet);

	//ret = select((int) Rec_fd + 1, &Rec_fdSet,NULL, NULL, &Rec_timeVal);
	ret = select((int) Rec_fd + 1, &Rec_fdSet,NULL, NULL, (struct timeval *)0);

	if( ret == 0 )    //select timeout
	{
		printf("Rec select timeout with result=%d\n",ret);
		return TcpTimeOutFlag;
	}
	if( !ret )    //select error
	{
		printf("TCP Select error result=%d,file=%s,function=%s,line=%d\n",ret,__FILE__,__FUNCTION__,__LINE__);
		return 0;
	}
	else          //select success
	{

		if ((numbytes=recv(Rec_fd,Rec_buf,SingleRecvMaxLen,0))==-1)
		 {
			perror("recv ");
			return TcpRecvError;
		 }
		if(numbytes == 0)
		{
			return TcpRecvError;
		}

		Rec_buf[numbytes] = '\0';

		return numbytes;
	}
	//printf("exit TCPService_Data_Receive\n");
	//return 0;
}

int TCPService_Data_Send(int Send_fd,char *Send_buf,int Data_Length)
{
	int len;
	//printf("start send\n");
	len =send(Send_fd,Send_buf,Data_Length,0);
	//printf("send over length is %d\n",len);
	if(len == Data_Length)
	{
		//printf("send over length is %d\n",len);
		return len;
	}
	else
	{
		perror("send");
		return 0;
	}
}


int GetEmptyThread()
{
	int i;
	for(i=0;i<MaxConnect;i++)
	{
		if(TcpThread[i].IsConnect == false)
			return i;
	}
	return -1;
}

int InitTCPLink(int ThreadNum)
{
	int i;
	//初始化上报信息中的路口ID
	for(i=0;i<SignalMaxNum;i++)
	{
		if(SignalRealData[i].CrossID == -1)
		{
			break;
		}
		TcpThread[ThreadNum].ReportInfo[i].CrossID = SignalRealData[i].CrossID;
	}
	//初始化数据库连接
	//TcpThread[ThreadNum].conn=pConnPool->getConnection(oracle_username,oracle_password,oracle_connectstring);
	TcpThread[ThreadNum].conn=pConnPool->getConnection();
	if(TcpThread[ThreadNum].conn)
	{
		 TcpThread[ThreadNum].stmt = TcpThread[ThreadNum].conn->createStatement();
		 TcpThread[ThreadNum].stmt->setAutoCommit(true);
	}
	else
	{
		return fail;
	}
	TcpThread[ThreadNum].FromNode = TcpThread[ThreadNum].ToNode = NULL;
	TcpThread[ThreadNum].IsSysStateUpdate = true;
	TcpThread[ThreadNum].IsRegionStateUpdate = true;

	return succeed;
}
