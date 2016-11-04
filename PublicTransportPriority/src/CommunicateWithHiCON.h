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

void CreateParsePthread(char *buf);
void *Pth_ParsingData(void *arg);
int ParsingData(char *buf);

int HiCONResponse_Login(xmlXPathContextPtr Xpath);
int HiCONResponse_Subscribe(xmlXPathContextPtr Xpath);
void HiCON_SignalStatus(xmlXPathContextPtr Xpath);
void HiCON_Phase(xmlXPathContextPtr Xpath);
void HiCON_Priority(xmlXPathContextPtr Xpath);

#endif /* COMMUNICATEWITHHICON_H_ */
