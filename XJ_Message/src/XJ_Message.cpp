//============================================================================
// Name        : XJ_Message.cpp
// Author      : LIRUIJIE
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <iostream>
#include "include.h"

using namespace std;
using namespace oracle::occi;

#define MAXDATASIZE 512
#define QueueNum    100
int Udp_fd,Tcp_fd;
struct sockaddr_in Modem_addr;
struct sockaddr_in WebSys_addr; //描述客户端地址
int sin_size = sizeof(struct sockaddr_in);
struct TcpThreadInfo TcpThread[ClientMaxNum];
struct QueueInfo Queue[QueueNum];
struct QueueInfo *QueueHead,*QueueTail;

oracle::occi::Environment *OraEnviroment;
StatelessConnectionPool *pConnPool;
Connection *conn;
Statement *stmt;
char 	oracle_username[50];
char	oracle_password[50];
char	connectstring[100];
char	device_ip[20];
pthread_mutex_t sql_mutex;
const char Response[] = "<?xml version=\"1.0\" encoding=\"UTF-8\"?><Message><Type>RESPONSE</Type><PhoneNumber>1</PhoneNumber></Message>";
const char Error[] = "<?xml version=\"1.0\" encoding=\"UTF-8\"?> \
							<Message>\
								<Type>ERROR</Type>\
							</Message>";
int main(void)
{
	//struct timeval 	timeVal;
	struct sockaddr_in client_addr;
	fd_set  fdSet;
	int ret;
	int commt_fd;
	int ConnectNum;


	if(GetConfig() == 0)
	{
		exit(0);
	}
	if(InitOracleLink() == 0)
	{
		return 0;
	}
	if(CreateUdpToModem()==false)
	{
		return false;
	}
	if(CreateTcp() == false)
	{
		return false;
	}
	if(InitDevice() == false)
	{
		printf("Init Device failure\n");
		return false;
	}

	InitQueue();

	pthread_t message;
	ret = pthread_create(&message,NULL,SendQueueMessage,NULL);
	if(ret <0)
		return false;
	while(1)
	{


		FD_ZERO(&fdSet);
		FD_SET(Tcp_fd, &fdSet);
		//printf("select\n");
		ret = select((int) Tcp_fd + 1, &fdSet,NULL, NULL, (struct timeval *)0);
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
				commt_fd = accept(Tcp_fd, (struct sockaddr *)&client_addr, (socklen_t *)&sin_size);
				if  (commt_fd == -1)
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
				commt_fd = accept(Tcp_fd, (struct sockaddr *)&client_addr, (socklen_t *)&sin_size);
				close(commt_fd);
			}
		}
	}
	return EXIT_SUCCESS;
}

int GetConfig()
{
	char filepath[] = "/home/config";
	char line[64];
	char tmp[64];
	FILE *fp = fopen(filepath,"r+");
	if(fp == NULL)
	{
		perror("open config file error::\n");
		return 0;
	}
	memset(oracle_username,'\0',sizeof(oracle_username));
	fgets(line,64,fp);
	sscanf(line,"%[^=]=%s",tmp,oracle_username);
	printf("temp = %s,config = %s\n",tmp,oracle_username);

	memset(oracle_password,'\0',sizeof(oracle_password));
	fgets(line,64,fp);
	sscanf(line,"%[^=]=%s",tmp,oracle_password);
	printf("temp = %s,config = %s\n",tmp,oracle_password);

	memset(connectstring,'\0',sizeof(connectstring));
	fgets(line,64,fp);
	sscanf(line,"%[^=]=%s",tmp,connectstring);
	printf("temp = %s,config = %s\n",tmp,connectstring);

	memset(device_ip,'\0',sizeof(device_ip));
	fgets(line,64,fp);
	sscanf(line,"%[^=]=%s",tmp,device_ip);
	printf("temp = %s,config = %s\n",tmp,device_ip);
	fclose(fp);
	return 1;
}



bool CreateUdpToModem()
{
	struct sockaddr_in myaddr;
	if ((Udp_fd = socket(PF_INET, SOCK_DGRAM, 0)) < 0)
	{
		perror("fail to socket:udp\n");
		return false;
	}
	int on=1;
	if((setsockopt(Udp_fd,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(on)))<0)
	{
		perror("setsockopt failed:udp\n");
		return false;
	}
	bzero(&myaddr, sizeof(myaddr));
	myaddr.sin_family = AF_INET;
	myaddr.sin_port = htons(10088);
	myaddr.sin_addr.s_addr = INADDR_ANY;

	if ( bind( Udp_fd, (struct sockaddr *)&myaddr, sizeof(myaddr) ) < 0 )
	{
		perror("fail to bind:udp\n");
		return false;
	}
	bzero(&Modem_addr,sizeof(struct sockaddr_in));
	Modem_addr.sin_family=AF_INET;
	Modem_addr.sin_addr.s_addr=inet_addr(device_ip);
	Modem_addr.sin_port=htons(10088);

	return true;
}

int CreateTcp()
{
	struct sockaddr_in myaddr;
	if ((Tcp_fd = socket(PF_INET, SOCK_STREAM, 0)) < 0)
	{
		perror("fail to socket:tcp\n");
		return false;
	}
	int on=1;
	if((setsockopt(Tcp_fd,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(on)))<0)
	{
		perror("setsockopt failed:tcp\n");
		return false;
	}
	bzero(&myaddr, sizeof(myaddr));
	myaddr.sin_family = AF_INET;
	myaddr.sin_port = htons(8801);
	myaddr.sin_addr.s_addr = INADDR_ANY;
	if ( bind( Tcp_fd, (struct sockaddr *)&myaddr, sizeof(myaddr) ) < 0 )
	{
		perror("fail to bind:udp\n");
		return false;
	}
	if (listen(Tcp_fd, 1) == -1)
	 {
		perror("listen");
		return false;
	 }
	printf("listen success\n");
	int flags;
	flags = fcntl(Tcp_fd, F_GETFL);
	if( (flags<0)||((fcntl(Tcp_fd, F_SETFL, flags |O_ASYNC))<0) )
	{
		perror("fcntl\n");
		return false;
	}
	else
	{
		printf("fcntl success\n");
	}

	return true;
}

bool InitDevice()
{
	char RecvBuf[MAXDATASIZE];
	//测试
	SendToModem((char *)"at\r\n",4);
	if(RecvFromModem( RecvBuf,1))
	{
		printf("设备正常\n");
	}
	else
	{
		printf("Modem response error@Line%d",__LINE__);
		return false;
	}

	//关闭回显
	SendToModem((char *)"ate0\r\n" ,6);
	if(RecvFromModem( RecvBuf,1)>=6)
	{
		printf("关闭回显\n");
	}
	else
	{
		printf("Modem response error: %s@Line%d",RecvBuf,__LINE__);
		return false;
	}

	//设置文本模式
	SendToModem((char *)"at+cmgf=1\r\n" ,11);
	if(RecvFromModem( RecvBuf,1) >= 6)
	{
		if(strcmp(RecvBuf,"\r\nOK\r\n") != 0 )
		{
			printf("Modem response error: %s@Line%d",RecvBuf,__LINE__);
			return false;
		}
	}
	//设置文本模式参数
	SendToModem((char *)"at+csmp=17,167,2,25\r\n" ,21);
	if(RecvFromModem( RecvBuf,1) >= 6)
	{
		if(strcmp(RecvBuf,"\r\nOK\r\n") != 0 )
		{
			printf("Modem response error: %s@Line%d",RecvBuf,__LINE__);
			return false;
		}
	}
	//设置编码类型
	SendToModem((char *)"at+cscs=\"ucs2\"\r\n" ,16);
	if(RecvFromModem(  RecvBuf,1) >= 6)
	{
		if(strcmp(RecvBuf,"\r\nOK\r\n") != 0 )
		{
			printf("Modem response error: %s@Line%d",RecvBuf,__LINE__);
			return false;
		}
	}
	printf("Init Device success\n");
	return true;
}
void InitQueue()
{
	int i;
	for(i = 0;i< QueueNum-1;i++)
	{
		Queue[i].Next = &Queue[i+1];
	}
	Queue[i].Next = &Queue[0];
	QueueTail = QueueHead = &Queue[0];

}
int EnQueue(unsigned char *PhoneNum,unsigned char *Text,unsigned char *message_id)
{
	if(IsQueueFull())
			return 0;
	memset(QueueTail->PhoneNum,'\0',20);
	memcpy(QueueTail->PhoneNum,PhoneNum,strlen((char *)PhoneNum));
	memset(QueueTail->Text,'\0',500);
	memcpy(QueueTail->Text,Text,strlen((char *)Text));
	memset(QueueTail->MessageID,'\0',20);
	memcpy(QueueTail->MessageID,message_id,20);

	QueueTail = QueueTail->Next;
	return 1;
}
void DelQueue(unsigned char *Phone_I,unsigned char *Text,unsigned char *message_id)
{

	memset(Phone_I,'\0',20);
	memcpy(Phone_I,QueueHead->PhoneNum,20);
	memset(Text,'\0',500);
	memcpy(Text,QueueHead->Text,500);
	memset(message_id,'\0',20);
	memcpy(message_id,QueueHead->MessageID,20);

	QueueHead = QueueHead->Next;
}
int IsQueueEmpty()
{
	return( QueueHead == QueueTail );
}
int IsQueueFull()
{
	return( QueueTail->Next == QueueHead );   //这种判断方式导致最后一个无法使用，即队用可用的位置比创建的位置少1 但是操作简单
}
bool SendMessage(unsigned char *PhoneNum,unsigned char *Text)
{
	int ret;
	ret = SendPhoneNum(PhoneNum);
	if(  ret== false )
	{
		return false;
	}

	if( !SendText(Text) )
	{
		return false;
	}
	return true;
}






int SendText(unsigned char *Text)
{

	unsigned char OutText[TcpFrameMaxLen*2];
	int OutLen;
	unsigned char Text_ASC[TcpFrameMaxLen*4];
	char EndChar = 0x1A;
	int ret;
	char RecvBuf[50];
	OutLen = g2u((char *)Text,strlen((char *)Text),(char *)(OutText),TcpFrameMaxLen*2);
	memset(Text_ASC,'\0',TcpFrameMaxLen*4);
	ConverHex2ASC(OutText,OutLen,Text_ASC,TcpFrameMaxLen*4);
	printf("%s\n",Text_ASC);

	if( !SendToModem((char * )Text_ASC,OutLen*2) )
	{
		printf("Send Text error\n");
		return false;
	}
	usleep(1000);
	if( !SendToModem((char *)&(EndChar),1) )
	{
		printf("Send EndChar error\n");
		return false;
	}

	ret = RecvFromModem(RecvBuf,30);
	if( ret > 6)
	{
		if(strncmp(RecvBuf+ret-6,"\r\nOK\r\n",6) != 0)                  //+CMGS: 7\r\n\r\nOK\r\n
		{
			printf("Send Message Error\n");
			return false;
		}
		else
		{
			printf("Message send OK\n");
		}
	}
	else
	{
		printf("receive response error\n");
		return false;
	}
	return true;
}

int SendPhoneNum(unsigned char *PhoneNum)
{
	char AT_Cmd[100];
	unsigned char OutPhoneNum[30];
	char RecvBuf[MAXDATASIZE];
	char StaticBuf[4] = {0x0D,0x0A,0x3E,0x20};
	int OutLen;
	unsigned char PhoneNum_ASC[60];

	OutLen = g2u((char *)PhoneNum,strlen((char *)PhoneNum),(char *)(OutPhoneNum),30);
	memset(PhoneNum_ASC,'\0',60);
	ConverHex2ASC((unsigned char *)OutPhoneNum,OutLen,PhoneNum_ASC,60);

	memset(AT_Cmd,'\0',100);
	sprintf(AT_Cmd,"at+cmgs=\"%s\"\r\n",PhoneNum_ASC);
	printf("%s\n",AT_Cmd);

	if( !SendToModem(AT_Cmd,strlen(AT_Cmd)) )
	{
		printf("Send phone num AT Cmd error\n");
		return false;
	}
	memset(RecvBuf,'\0',10);
	if( !RecvFromModem(RecvBuf,5) )
	{
		printf("Receive phone num AT Cmd response error,receive\n");
		return false;
	}

	if( strncmp(RecvBuf,StaticBuf,4) == 0 )
	{
		return true;
	}
	else
	{
		printf("Receive phone num AT Cmd response error,buffer\n");
		return false;
	}

}

int code_convert(char *from_charset,char *to_charset,char *inbuf,size_t inlen,char *outbuf,size_t outlen)
{
	iconv_t cd;
	//size_t rc;

	size_t outlenleft = outlen;
	char **pin = &inbuf;
	char **pout = &outbuf;
	cd = iconv_open(to_charset,from_charset);
	if (cd==0) return -1;
	memset(outbuf, 0, outlen);
	if(iconv(cd,pin,&inlen,pout,&outlenleft) == 0xFF)
	{
		iconv_close(cd);
		return -1;
	}
	iconv_close(cd);
	return (int)(outlen-outlenleft);
}

int u2g(char *inbuf,int inlen,char *outbuf, int outlen)
{
	return code_convert((char *)"UCS-2",(char *)"UTF-8",inbuf,inlen,outbuf,outlen);
}

int g2u(char *inbuf,int inlen,char *outbuf, int outlen)
{
	return code_convert((char *)"UTF-8",(char *)"UCS-2",inbuf,inlen,outbuf,outlen);
}


int ConverHex2ASC(unsigned char *Hex, int HexLen, unsigned char *ASC, int ASCLen)
{
	int i;
	if(!Hex || !ASC)
	{
		return -1;
	}
	if(HexLen > ASCLen / 2)
	{
		return -1;
	}
	for(i=0; i<HexLen; i ++)
	{
		/*if(Hex2ASC(*(Hex + i+1), ASC + 2*i)!=0)
		{
			return -1;
		}
		if(Hex2ASC(*(Hex + i), ASC + 2*(i+1))!=0)
		{
			return -1;
		}*/

		if(Hex2ASC(*(Hex + i), ASC + 2*i)!=0)
		{
			return -1;
		}
	}
	return 0;
}

int Hex2ASC(unsigned char Hex, unsigned char *ASC)
{
	switch(Hex >> 4)
	{
		case 0: case 1: case 2: case 3:
		case 4: case 5: case 6: case 7:
		case 8: case 9:
			*ASC = (Hex>>4) + '0';
			break;
		case 0xa: case 0xb: case 0xc:
		case 0xd: case 0xe: case 0xf:
			*ASC = (Hex>>4) -0xA + 'A';
			break;
	}
	switch(Hex & 0xF)
	{
		case 0: case 1: case 2: case 3:
		case 4: case 5: case 6: case 7:
		case 8: case 9:
			*(ASC+1) = (Hex & 0xF) + '0';
			break;
		case 0xa: case 0xb: case 0xc:
		case 0xd: case 0xe: case 0xf:
			*(ASC+1) = (Hex & 0xF) - 0xA + 'A';
			break;
	}
	return 0;
}
int SendToModem(char *send_buf,int data_len)
{
		int len;
		len =sendto(Udp_fd,send_buf,data_len,0,(struct sockaddr *)&Modem_addr,sizeof(struct sockaddr_in));
		if(len == data_len)
		{
			return len;
		}
		else
		{
			return 0;
		}
}
int RecvFromModem( char *rcv_buf,int time_s)
{
		struct timeval 	timeVal;
		fd_set  fdSet;
		int ret,numbytes;
		//printf("here is TCPClient_Data_Receive\n");
		timeVal.tv_sec = time_s;
		timeVal.tv_usec = 0;

		FD_ZERO(&fdSet);
		FD_SET(Udp_fd, &fdSet);

		ret = select((int) Udp_fd + 1, &fdSet,NULL, NULL, &timeVal);
		if( ret == 0 )    //select timeout
		{
			//printf("Connect select timeout with result=%d\n",ret);
			return 0;
		}
		if( !ret )    //select error
		{
			//printf("Connect select failed with result=%d\n",ret);
			return 0;
		}
		else          //select success
		{
			memset(rcv_buf,'\0',MAXDATASIZE);
			if ((numbytes=recvfrom(Udp_fd,  rcv_buf,  MAXDATASIZE,  0,  (struct sockaddr *)&Modem_addr, (socklen_t *) &sin_size))==-1)
			 {
				perror("recv ");
				sleep(2);
				exit(1);
			 }
			rcv_buf[numbytes] = '\0';
			return numbytes;
		}
		return 0;
}



void *TcpCommunicate(void *arg)
{
	int ThreadNum = *(int *)arg;
	char buf[TcpFrameMaxLen];
	int ret;
	int i;
	printf("This is TCP Connect %d thread,connect fd = %d\n",ThreadNum,TcpThread[ThreadNum].commt_fd);


	xmlDocPtr pdoc = xmlReadMemory(Response,(int )strlen(Response), NULL, "utf-8", XML_PARSE_NOERROR|XML_PARSE_NOBLANKS);
	xmlChar *outbuf;
	int outlen;
	xmlDocDumpFormatMemoryEnc(pdoc, &outbuf, &outlen, "UTF-8", 1);
	while(true)
	{
		memset(buf,'\0',TcpFrameMaxLen);
		ret = TCPService_Data_Receive(TcpThread[ThreadNum].commt_fd,buf);
		if(ret>0)
		{
			for(i=0;i<ret;i++)
			{
				if(buf[i]=='<')
					break;
			}
			printf("process id = %d,commt fd = %d  \nreceive data:\n%s\n",ThreadNum,TcpThread[ThreadNum].commt_fd,buf+i);

			TCPService_Data_Send(TcpThread[ThreadNum].commt_fd,(char *)outbuf,outlen);

			//TCPService_Data_Send(TcpThread[ThreadNum].commt_fd,(char *)Response,strlen(Response));
			AnalyzeXml(buf+i,ThreadNum);
		}
		else if(ret==-1)     //TCP Error
		{
			printf("TCP error\n");
			//printf("this link is  %s:%d\n", inet_ntoa( client_addr.sin_addr ),ntohs(client_addr.sin_port));
			printf("Thread process id=%d will close the TCPLink num = %d\n",ThreadNum,TcpThread[ThreadNum].commt_fd);
			close(TcpThread[ThreadNum].commt_fd);
			TcpThread[ThreadNum].IsConnect = false;
			xmlFreeDoc(pdoc);
			xmlFree(outbuf);
			pthread_exit(NULL);
		}
	}
	return (void *)0;
}




int TCPService_Data_Receive(int Rec_fd,char *Rec_buf)
{

	fd_set  Rec_fdSet;
	int ret,numbytes;

	//printf("here is TCPService_Data_Receive process,process ID %d,commt_fd = %d\n",getpid(),Rec_fd);

	FD_ZERO(&Rec_fdSet);
	FD_SET(Rec_fd, &Rec_fdSet);

	ret = select((int) Rec_fd + 1, &Rec_fdSet,NULL, NULL, (struct timeval *)0);


	if( ret == 0 )    //select timeout
	{
		//printf("Rec select timeout with result=%d\n",ret);
		return false;
	}

	if(FD_ISSET(Rec_fd,&Rec_fdSet))        //select success
	{

		if ((numbytes=recv(Rec_fd,Rec_buf,TcpFrameMaxLen,0))==-1)
		 {
			perror("recv ");
			return false;
		 }
		if(numbytes == 0)
		{
			return -1;
		}

		Rec_buf[numbytes] = '\0';

		return numbytes;
	}
	else    //select error
	{
		printf("TCP Select error result=%d,file=%s,function=%s,line=%d\n",ret,__FILE__,__FUNCTION__,__LINE__);
		return -1;
	}

}

int TCPService_Data_Send(int Send_fd,char *Send_buf,int Data_Length)
{
	int len;
	printf("start send\n");
	len =send(Send_fd,Send_buf,Data_Length,0);
	printf("send over length is %d\n",len);
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
	for(i=0;i<ClientMaxNum;i++)
	{
		if(TcpThread[i].IsConnect == false)
			return i;
	}
	return -1;
}

void *SendQueueMessage(void *arg)
{
	char sqlbuf[200];
	unsigned char PhoneNum[20];
	unsigned char Text[500];
	unsigned char msg_id[20];
	while(1)
	{
		if(IsQueueEmpty())        //
		{
			usleep(1000*100);
			continue;
		}
		else
		{
			DelQueue(PhoneNum,Text,msg_id);
			if(SendMessage(PhoneNum,Text))
			{
				sprintf(sqlbuf,"UPDATE MSG_RECORD SET SEND_TIME = SYSDATE,MSG_STATE = 2 WHERE ID = %s",msg_id);
			}
			else
			{
				sprintf(sqlbuf,"UPDATE MSG_RECORD SET SEND_TIME = SYSDATE,MSG_STATE = 0 WHERE ID = %s",msg_id);
			}
			printf(sqlbuf);
			printf("\n");
			ExecuteSql(sqlbuf);
		}
	}
	return (void *)0;
}

int AnalyzeXml(char *buf,int TcpNum)
{
	char msg_id[20];
	xmlDocPtr pdoc;
	xmlXPathContextPtr Xpath = NULL;    //XPATH上下文指针
	xmlXPathObjectPtr Xresult;

	char sqlbuf[1000];
	time_t time_now;
	xmlKeepBlanksDefault(1);

	pdoc = xmlReadMemory(buf,(int )strlen(buf), NULL, "utf-8", XML_PARSE_NOERROR|XML_PARSE_NOBLANKS);//
	if(pdoc == NULL)
	{
		printf( "Buff is error,can not parse\n" );
		return 0;
	}

	Xpath = xmlXPathNewContext(pdoc);     //创建一个XPath上下文指针
	if (Xpath == NULL)
	{
	   printf("context is NULL");
	   return 0;
	}

	Xresult = xmlXPathEvalExpression((xmlChar*)"//msg", Xpath);
	if (Xresult == NULL)
	{
	   printf("xmlXPathEvalExpression return NULL");
	   return 0;
	}
	if (xmlXPathNodeSetIsEmpty(Xresult->nodesetval))   //检查查询结果是否为空
	{
	   xmlXPathFreeObject(Xresult);
	   return 0;
	}
	xmlChar *Text = xmlNodeGetContent(Xresult->nodesetval->nodeTab[0]);
	if(Text == NULL)                     //要发送的信息内容为空
		return 0;

	Xresult = xmlXPathEvalExpression((xmlChar*)"//PhoneNumber", Xpath);
	if (Xresult == NULL)
	{
	   printf("xmlXPathEvalExpression return NULL");
	   return 0;
	}
	if (xmlXPathNodeSetIsEmpty(Xresult->nodesetval))   //检查查询结果是否为空
	{
	   xmlXPathFreeObject(Xresult);
	   return 0;
	}
	int i;
	xmlChar *PhoneNum;
	for(i=0;i<Xresult->nodesetval->nodeNr;i++)
	{
		PhoneNum = xmlNodeGetContent(Xresult->nodesetval->nodeTab[i]);
		if(PhoneNum == NULL)                     //手机号为空
				continue;
		time(&time_now);
		memset(msg_id,'\0',20);
		sprintf(msg_id,"%013ld%03d%04ld",time_now,TcpNum+1,TcpThread[TcpNum].MsgID);
		printf(msg_id);
		printf("\n");
		if(EnQueue(PhoneNum,Text,(unsigned char *)msg_id))
		{
			sprintf(sqlbuf,"insert into MSG_RECORD (ID,MSG_DATA,PHONE_NUMBER,RECEIVE_TIME,MSG_STATE) VALUES(%s,'%s',%s,SYSDATE,1)",msg_id,Text,PhoneNum);
			printf(sqlbuf);
			printf("\n");
			ExecuteSql(sqlbuf);
			TcpThread[TcpNum].MsgID = (TcpThread[TcpNum].MsgID +1) % 10000;
		}
		else	//队列中已经满了
		{
			xmlFree(PhoneNum);
			break;
		}

		xmlFree(PhoneNum);
	}
	xmlFree(Text);
	xmlXPathFreeObject(Xresult);
	xmlXPathFreeContext(Xpath);
	xmlFreeDoc(pdoc);
   xmlCleanupParser();
   xmlMemoryDump();

	return 1;
}

int WriteLog(char *log)
{
	FILE *log_fd;
	char filepath[50];
	char filedir[50];
	time_t time_now;

	time(&time_now);
	struct tm * timeinfo = localtime(&time_now);
	memset(filedir,'\0',50);
	sprintf(filedir,"mkdir -p /home/log/%d%02d/",1900+timeinfo->tm_year,timeinfo->tm_mon+1);
	system(filedir);

	sprintf(filepath,"/home/log/%d%02d/%d%02d%02d.txt",1900+timeinfo->tm_year,timeinfo->tm_mon+1,1900+timeinfo->tm_year,timeinfo->tm_mon+1,timeinfo->tm_mday);

	log_fd = fopen(filepath,"a+");
	if(log_fd == NULL)
	{
		perror("Open Error::");
	}

	if(fputs(log,log_fd)<0)
	{
		fclose(log_fd);
		return false;
	}
	fclose(log_fd);
	return true;
}


int InitOracleLink()
{
	while(1)
	{
		try
		{
		   const string userName = oracle_username;
		   const string password = oracle_password;
		   const string connectString= connectstring;
		   cout <<"name = " << userName    <<"  oracle_password = " <<  password <<        "    oracle_connectstring = " << connectString << endl;
		   unsigned int maxConn=200;
		   unsigned int minConn=5;
		   unsigned int incrConn=2;

			string sql;

			OraEnviroment = oracle::occi::Environment::createEnvironment("UTF8","UTF8");
		   pConnPool= OraEnviroment->createStatelessConnectionPool(userName,password,connectString,minConn,maxConn,incrConn);

			if(pConnPool)
			{
				 pConnPool->setTimeOut(10000);
				 conn=pConnPool->getConnection(userName,password,connectString);
				 if(conn)
				 {
						 stmt = conn->createStatement();
						 stmt->setAutoCommit(true);
						 return 1;
				 }
				if(pthread_mutex_init(&sql_mutex,NULL) != 0 )
				{
					perror("Init metux error：");
					exit(1);
				}
			}
		}
		catch (SQLException &sqlExcp)
		{
		   sqlExcp.getErrorCode();
		   string strinfo=sqlExcp.getMessage();
		   cout<<strinfo;
		   return 0;
		}
		printf("oracle open fail\n");
		sleep(1);
	}
}

void ExecuteSql(char *sql)
{
	int res;
	res = pthread_mutex_lock(&sql_mutex);/*lock the mutex*/
	if(res != 0)
	{
		perror("lock error!!!!\n");
	}
	stmt->setSQL(sql);
	stmt->executeUpdate();

	pthread_mutex_unlock(&sql_mutex);
}



#if 0
bool OpenSerial()
{
	GprsFd = open(SerialNum, O_RDWR);
	if(GprsFd < 0)
	{
		return false;
	}
	if( SetSerialOpt(GprsFd,115200,8,'n',1) == false )
	{
		return false;
	}
	return true;
}
int SetSerialOpt(int fd,int nSpeed, int nBits, char nEvent, int nStop)
{
    struct termios newtio,oldtio;
    /*保存测试现有串口参数设置，在这里如果串口号等出错，会有相关的出错信息*/
    if ( tcgetattr( fd,&oldtio) != 0)
    {
        perror("SetupSerial\n");
         return false;
     }
    bzero( &newtio, sizeof( newtio ) );

    /*步骤一，设置字符大小*/
    newtio.c_cflag |= CLOCAL | CREAD;  //修改控制模式，保证程序不会占用串口  使得能够从串口中读取输入数据
                        //CLOCAL      本地连接（不改变端口所有者）
    				    //CREAD       接收使能
    newtio.c_cflag &= ~CSIZE;
    					//CSIZE     数据位的位掩码
    newtio.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);    //配置
    					//ICANON    启用规范模式
						//ECHO    启用本地回显功能
						//ECHOE    若设置ICANON，则允许退格操作
						//ISIG    若收到信号字符（INTR、QUIT等），则会产生相应的信号
    newtio.c_oflag  &= ~OPOST;   /*Output*/
    					//OPOST    启用输出处理功能，如果不设置该标志，则其他标志都被忽略
    /*
    //设置数据流控制
    switch(flow_ctrl)
    {

        case 0 ://不使用流控制
        	newtio.c_cflag &= ~CRTSCTS;
            break;

        case 1 ://使用硬件流控制
        	 newtio.c_cflag |= CRTSCTS;
             break;
        case 2 ://使用软件流控制
        	newtio.c_cflag |= IXON | IXOFF | IXANY;
            break;
     }
    */

    /*设置停止位*/
    switch( nBits )
    {
    	case 5:
    		newtio.c_cflag |= CS5;
            break;
        case 6:
        	newtio.c_cflag |= CS6;
            break;
        case 7:
            newtio.c_cflag |= CS7;
            break;
        case 8:
            newtio.c_cflag |= CS8;
            break;
        default:
        	printf("Unsupported data size\n");
        	printf("Use default data size:8\n");
        	newtio.c_cflag |= CS8;
        	break;
    }
/*设置奇偶校验位*/
    switch( nEvent )
    {
    	case 'o':
        case 'O': //奇校验
            newtio.c_cflag |= PARENB;                 //使能校验
            newtio.c_cflag |= PARODD;                 //设置奇校验
            newtio.c_iflag |= (INPCK | ISTRIP);       //激活c_iflag中的对于输入数据的奇偶校验使能
            break;
        case 'e':
        case 'E': //偶校验
            newtio.c_cflag |= PARENB;                 //使能校验
            newtio.c_cflag &= ~PARODD;				  //设置关闭奇校验  即开启偶校验
            newtio.c_iflag |= (INPCK | ISTRIP);       //激活c_iflag中的对于输入数据的奇偶校验使能
            					//ISTRIP    裁减掉第8位比特
            break;
        case 'n':
        case 'N': //无奇偶校验位
            newtio.c_cflag &= ~PARENB;
            break;

        default:   //默认无校验
        	newtio.c_cflag &= ~PARENB;
        	break;

    }
    /*设置波特率*/
    switch( nSpeed )
    {
        case 2400:
            cfsetispeed(&newtio, B2400);
            cfsetospeed(&newtio, B2400);
        break;
        case 4800:
            cfsetispeed(&newtio, B4800);
            cfsetospeed(&newtio, B4800);
        break;
        case 9600:
            cfsetispeed(&newtio, B9600);
            cfsetospeed(&newtio, B9600);
        break;
        case 115200:
            cfsetispeed(&newtio, B115200);
            cfsetospeed(&newtio, B115200);
        break;
        case 460800:
            cfsetispeed(&newtio, B460800);
            cfsetospeed(&newtio, B460800);
        break;
        default:  //默认 115200
            cfsetispeed(&newtio, B115200);
            cfsetospeed(&newtio, B115200);
        break;
    }
    /*设置停止位*/
    if( nStop == 1 )
        newtio.c_cflag &= ~CSTOPB;
    else if ( nStop == 2 )
        newtio.c_cflag |= CSTOPB;
    /*设置等待时间和最小接收字符  在对接收字符和等待时间没有特别要求的情况下，可以将其设置为0，则在任何情况下read()函数立即返回*/
    newtio.c_cc[VTIME] = 0;
    newtio.c_cc[VMIN] = 0;
    /*处理未接收字符*/
    tcflush(fd,TCIFLUSH);
    /*激活新配置*/
    if((tcsetattr(fd,TCSANOW,&newtio))!=0)      //TCSANOW：不等数据传输完毕就立即改变属性。
    	                                        //TCSADRAIN：等待所有数据传输结束才改变属性。
    											//TCSAFLUSH：清空输入输出缓冲区才改变属性。
    {
        perror("com set error");
        return false;
    }
    printf("set done!\n");
    return true;
}
int UART_Recv(int fd, char *rcv_buf,int time_s)
{
    int len,flag_select;
    fd_set fs_read;

    struct timeval time;

    FD_ZERO(&fs_read);
    FD_SET(fd,&fs_read);

    time.tv_sec = time_s;
    time.tv_usec = 0;


    flag_select = select(fd+1,&fs_read,NULL,NULL,&time);

    if(flag_select)
    {
	   memset(rcv_buf,'\0',MAXDATASIZE);
	   usleep(10);
	   len = read(fd,rcv_buf,MAXDATASIZE);
	   return len;
    }
    else
    {
	  return 0;
    }
}
int UART_Send(int fd, char *send_buf,int data_len)
{
    int len = 0;

    len = write(fd,send_buf,data_len);
    if(len == data_len )
	  {
			 return len;
	  }
    else
      {
			 tcflush(fd,TCOFLUSH);
			 return 0;
      }

}
#endif



