/*
 * CommunicateWithWeb.h
 *
 * Created on: 2016年12月13日
 * Author: LIRUIJIE
 */

#ifndef COMMUNICATEWITHWEB_H_
#define COMMUNICATEWITHWEB_H_

void CommunicateWithWeb();
int InitWebSocket();
void * ReceiveWebData(void *arg);
void * WebControlReceive(void * arg);
void * Handle_Pthread(void *arg);
void ParsingData_Web(unsigned char *data,struct sockaddr_in WebAddr);
bool check_buf(  char * rcv_buf);
int makecheck(unsigned char * rcv_buf);
int Control_NextStep_0x01(int index);
int Control_RegionsCoordinate_0x05(int Coord_ID);
int Control_RestartServer_0xFF();
int Control_RestartSignal_0xF3(int index);
int Control_ChangePlane_0x02(int index,int plan_id);
int Control_SpecialService_0x03(int index,int type);
int Control_DemoteMode_0x04(int index,int type);

int Control_CheckTime_0xF1(int index);
int GetDeviceIndex(unsigned char *buf);
int Request_BaseInfo_0x01(int index);
int Request_Phase_0x23(int index,int phase_id);
int Request_Plan_0x2B(int index,int phase_id);
int Set_Phase_0x23(int index,int phase_id);
int Set_Plan_0x2B(int index,int plan_id);

void * RegionsCoordinate_pthread(void *arg);
void GetMasterUnitPlan(Connection *_conn,Statement *_stmt );
void ZKTD_CoordinatePlan(int index,Connection *_conn,Statement *_stmt );
void ITC_CoordinatePlan(int index,Connection *_conn,Statement *_stmt );

#endif /* COMMUNICATEWITHWEB_H_ */
