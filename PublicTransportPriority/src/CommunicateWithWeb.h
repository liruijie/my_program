/*
 * CommunicateWithWeb.h
 *
 * Created on: 2016年10月11日
 * Author: LIRUIJIE
 */

#ifndef COMMUNICATEWITHWEB_H_
#define COMMUNICATEWITHWEB_H_

void * ReceiveData_Web(void *arg);
void SendToWeb(struct sockaddr_in WebAddr,unsigned char *buf,int len);
void ParsingData_Web(unsigned char *data,struct sockaddr_in WebAddr);
void * Handle_Pthread(void *arg);


int Request_DeviceParam_0x03(int sockfd,struct sockaddr_in DeviceAddr,unsigned char * send_data);
int Request_DeviceSystemParam_0x04(int sockfd,struct sockaddr_in DeviceAddr,unsigned char * send_data);
int Request_DeviceStrategy_0x05(int sockfd,struct sockaddr_in DeviceAddr,unsigned char * send_data);
int Request_DeviceStrategyTime_0x06(int sockfd,struct sockaddr_in DeviceAddr,unsigned char * send_data);
int Request_DeviceSchedule_0x07(int sockfd,struct sockaddr_in DeviceAddr,unsigned char * send_data);
int Request_DeviceCard_0x08(int sockfd,struct sockaddr_in DeviceAddr,unsigned char * send_data);
int ReceiveDeviceReply(int sockfd,struct sockaddr_in DeviceAddr,unsigned char * data,int timeout_s);
#endif /* COMMUNICATEWITHWEB_H_ */