/*
 * Queue.cpp
 *
 * Created on: 2016年12月12日
 * Author: LIRUIJIE
 */

#include "include.h"

QueueInfo Queue[QueueNum];
QueueInfo *QueueHead,*QueueTail;
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
void EnQueue(char *recvbuf)
{
	if(IsQueueFull())
			return;
	memset(QueueTail->buf,'\0',FrameMaxLen);
	memcpy(QueueTail->buf,recvbuf,FrameMaxLen);
	QueueTail = QueueTail->Next;
}
void DelQueue(char *outbuf)
{
	memset(outbuf,'\0',FrameMaxLen);
	memcpy(outbuf,QueueHead->buf,FrameMaxLen);
	QueueHead = QueueHead->Next;
}
int IsQueueEmpty()
{
	return( QueueHead == QueueTail );
}



