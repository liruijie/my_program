/*
 * include.h
 *
 *  Created on: 2016年9月19日
 *      Author: root
 */


#ifndef INCLUDE_H_
#define INCLUDE_H_


#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <termios.h>
#include <unistd.h>
#include <signal.h>
#include "iconv.h"
#include <stdbool.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>

#include <libxml/parser.h>
#include <libxml/xmlreader.h>
#include <libxml/tree.h>
#include <libxml/xmlmemory.h>
#include <libxml/xmlerror.h>
#include <libxml/xpath.h>

#include <occi.h>


//#define MaxTextLen    	10000
#define TcpFrameMaxLen  10000
#define ClientMaxNum		100
struct TcpThreadInfo
{
	int IsConnect;                    	//连接标志
	int commt_fd;                			//链接文件描述符
	pthread_t thread_id;        			//线程ID
};
struct QueueInfo
{
	char buf[TcpFrameMaxLen];
	int TcpNum;
	struct QueueInfo *Next;
};

int GetConfig();
bool InitDevice();
bool CreateUdpToModem();
int CreateTcp();
int RecvFromModem( char *rcv_buf,int time_s);
int SendToModem(char *send_buf,int data_len);
bool SendMessage(unsigned char *PhoneNum,unsigned char *Text);

int u2g(char *inbuf,int inlen,char *outbuf, int outlen);
int g2u(char *inbuf,int inlen,char *outbuf, int outlen);
int code_convert(char *from_charset,char *to_charset,char *inbuf,size_t inlen,char *outbuf,size_t outlen);
int ConverHex2ASC(unsigned char *Bin, int BinLen, unsigned char *Hex, int HexLen);
int Hex2ASC(unsigned char Bin, unsigned char *Hex);
int SendPhoneNum(unsigned char *PhoneNum);
int SendText(unsigned char *Text);

void EnQueue(char *recvbuf,int TcpNum);
void InitQueue();
void DelQueue(char *outbuf,int *TcpNum);
int IsQueueFull();
int IsQueueEmpty();

void *TcpCommunicate(void *arg);
int TCPService_Data_Receive(int Rec_fd,char *Rec_buf);
int TCPService_Data_Send(int Send_fd,char *Send_buf,int Data_Length);
int GetEmptyThread();

void *AnalyzeBuf(void *arg);
void AnalyzeXml(char *buf,int TcpNum);

int WriteLog(char *log);
/*bool OpenSerial();
int SetSerialOpt(int fd,int nSpeed, int nBits, char nEvent, int nStop);
int UART_Recv(int fd, char *rcv_buf,int time_s);
int UART_Send(int fd, char *send_buf,int data_len);*/



#endif /* INCLUDE_H_ */
