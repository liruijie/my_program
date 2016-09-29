/*
 * ControlCenterCommt.h
 *
 *  Created on: 2016年7月5日
 *      Author: root
 */

#ifndef CONTROLCENTERCOMMT_H_
#define CONTROLCENTERCOMMT_H_
#include "include.h"

extern struct TcpThreadInfo TcpThread[MaxConnect];

void Tcp();
int GetEmptyThread();
void *TcpCommunicate(void *arg);
int InitTCPLink(int ThreadNum);
void *ControlCenterCommt(void *arg);
int TCPService_Data_Receive(int Rec_fd,char *Rec_buf);
int TCPService_Data_Send(int Send_fd,char *Send_buf,int Data_Length);

#endif /* CONTROLCENTERCOMMT_H_ */
