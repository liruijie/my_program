/*
 * ControlSignal.h
 *
 *  Created on: 2016年8月31日
 *      Author: root
 */

#ifndef CONTROLSIGNAL_H_
#define CONTROLSIGNAL_H_

void *ControlSignal(void *arg);
int InitControlSocket();
int ChangeOracleFlag(int SignalID);
int MakeCheckSum(unsigned char *rcv_buf);
void SendControlHeartBeat(int SignalID);
int Control_LockTraffic(int index);
int Control_ControlMode(int index);
int Control_CrossStage(int index);
int Control_CrossPlan(int index);
#endif /* CONTROLSIGNAL_H_ */
