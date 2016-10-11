/*
 * Queue.cpp
 *
 * Created on: 2016年10月10日
 * Author: LIRUIJIE
 */

#include "include.h"

extern QueueInfo Queue[QueueNum];
extern QueueInfo *QueueHead,*QueueTail;
void InitQueue()
{
	int i;
	for(i = 0;i< QueueNum-1;i++)
	{
		Queue[i].Next = &Queue[i+1];
	}
	Queue[i].Next = &Queue[0];
	QueueTail = QueueHead = &Queue[0];

}
int IsQueueFull()
{
	return( QueueTail->Next == QueueHead );   //这种判断方式导致最后一个无法使用，即队用可用的位置比创建的位置少1
}
void EnQueue(unsigned char *recvbuf)
{
	if(IsQueueFull())
			return;
	memset(QueueTail->buf,'\0',SingleRecvMaxLen);
	memcpy(QueueTail->buf,recvbuf,SingleRecvMaxLen);
	QueueTail = QueueTail->Next;
}
void DelQueue(unsigned char *outbuf)
{
	memset(outbuf,'\0',SingleRecvMaxLen);
	memcpy(outbuf,QueueHead->buf,SingleRecvMaxLen);
	QueueHead = QueueHead->Next;
}
int IsQueueEmpty()
{
	return( QueueHead == QueueTail );
}