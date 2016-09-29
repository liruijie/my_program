/*
 * ITC_Server.h
 *
 *  Created on: 2016年7月7日
 *      Author: root
 */

#ifndef ITC_SERVER_H_
#define ITC_SERVER_H_

#define QueueNum 5000
struct QueueInfo
{
	unsigned char buf[512];
	struct QueueInfo *Next;
};


extern struct SignalData SignalRealData[SignalMaxNum];

void *ItcServerCommt(void *arg);
void *AnalyzeSignalData(void *arg);
void ConnetcServer();
int RecvFromServer(int Rec_fd,unsigned char *Rec_buf);
void SendToServer(int Socket_fd,char *recvbuf,int len);
int checksum(unsigned char * rcv_buf);
void AnalyzeBuffer(unsigned char *Rec_buf);
void realdata(unsigned char *recvbuf,int signal_num);
void faultdata(unsigned char *recvbuf,int signal_num);
void occodata(unsigned char *recvbuf,int signal_num);
void voldata(unsigned char *recvbuf,int signal_num);
void fault_0xA1(unsigned char *recvbuf,int signal_num);
void fault_0xA2(unsigned char *recvbuf,int signal_num);
void fault_0xA3(unsigned char *recvbuf,int signal_num);
void fault_0xA4(unsigned char *recvbuf,int signal_num);
void fault_0xA5(unsigned char *recvbuf,int signal_num);
int UpdateParam(int SignalID);

void InitQueue();
void EnQueue(unsigned char *recvbuf);
void DelQueue(unsigned char *outbuf);
int IsQueueFull();
int IsQueueEmpty();

#endif /* ITC_SERVER_H_ */
