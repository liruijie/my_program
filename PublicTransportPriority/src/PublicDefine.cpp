/*
 * PublicDefine.cpp
 *
 * Created on: 2016年10月9日
 * Author: LIRUIJIE
 */
#include "include.h"

int CurrentExistDevice;					//当前从数据库中查出的公交优先设备数
int CurrentExistSignal;					//当前从数据库中查出的海信信号机数
DeviceInfo device[DeviceMaxNum];		//公交优先设备信息结构体
QueueInfo Queue[QueueNum];				//接收设备信息的队列
QueueInfo *QueueHead,*QueueTail;		//队列头和队列尾

HiCON_Signal_Info Signal[SignalMaxNum];	//海信信号机结构体


