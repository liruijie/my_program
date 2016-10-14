/*
 * Queue.h
 *
 * Created on: 2016年10月10日
 * Author: LIRUIJIE
 */

#ifndef QUEUE_H_
#define QUEUE_H_

struct QueueInfo
{
	unsigned char buf[SingleRecvMaxLen];
	struct QueueInfo *Next;
};
void InitQueue();
void EnQueue(unsigned char *recvbuf);
void DelQueue(unsigned char *outbuf);
int IsQueueFull();
int IsQueueEmpty();


#endif /* QUEUE_H_ */
