/*
 * CommunicateWithHiCON.h
 *
 * Created on: 2016年10月20日
 * Author: LIRUIJIE
 */

#ifndef COMMUNICATEWITHHICON_H_
#define COMMUNICATEWITHHICON_H_

int CommunicateWithHiCON();
void * Connect_HiCON(void *arg);
int InitHiCON_TCP();
int Login();
void * Subscribe_Pthread(void * arg);
int Subscribe(int signal_id);
int Receive_HiCON(char *data,int timeout_s);
int Send_HiCON(char *data,long len);
int CheckXmlResult(xmlXPathObjectPtr Xresult);

int Response_Login(xmlXPathContextPtr Xpath);
int Response_Subscribe(xmlXPathContextPtr Xpath);
void Response_SignalStatus(xmlXPathContextPtr Xpath);
void Response_Phase(xmlXPathContextPtr Xpath);
void Response_Priority(xmlXPathContextPtr Xpath);

#endif /* COMMUNICATEWITHHICON_H_ */
