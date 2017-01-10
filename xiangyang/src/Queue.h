/*
 * Queue.h
 *
 * Created on: 2016年12月12日
 * Author: LIRUIJIE
 */

#include "include.h"
#ifndef QUEUE_H_
#define QUEUE_H_
#define QueueNum 				10000				//报文存储队列个数
struct QueueInfo
{
	char buf[FrameMaxLen];
	struct QueueInfo *Next;
};

void InitQueue();
void EnQueue(char *recvbuf);
void DelQueue(char *outbuf);
int IsQueueFull();
int IsQueueEmpty();



#endif /* QUEUE_H_ */
