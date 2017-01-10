/*
 * CommunicateWithServer.h
 *
 * Created on: 2016年11月30日
 * Author: LIRUIJIE
 */

#ifndef COMMUNICATEWITHSERVER_H_
#define COMMUNICATEWITHSERVER_H_

void CommunicateWithServer();
void * ReceiveFromServer(void * arg);
void * HandleServerData(void * arg);
void * HeartBeat(void *arg);
int InitTCP();
int ReceiveServerData(int sockfd,char *data,int timeout_s);
int SendData(char *data,long len);
int Login();
bool check_xml(char *data);
int ParseData(char *data);
int GetSignalIndex(xmlXPathContextPtr Xpath);
int Server_RESPONSE(xmlXPathContextPtr Xpath);
int CheckXmlResult(xmlXPathObjectPtr Xresult);
int Subscribe_RESPONSE(xmlXPathContextPtr Xpath);
int Get_Node(xmlNodePtr *Node,char *NodeName);
int GetBaseInfo(xmlXPathContextPtr Xpath);
int GetPhaseInfo(xmlXPathContextPtr Xpath);
int GetPlanInfo(xmlXPathContextPtr Xpath);
int JudgeOperateObject(xmlXPathContextPtr Xpath);
int Server_Error(xmlXPathContextPtr Xpath);
int Server_PUSH(xmlXPathContextPtr Xpath);

void * Subscribe(void *arg);
int Subscribe_RealPhaseStatus(xmlXPathContextPtr Xpath);
int Subscribe_Error(xmlXPathContextPtr Xpath);
#endif /* COMMUNICATEWITHSERVER_H_ */
