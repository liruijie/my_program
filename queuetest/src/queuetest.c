/*
 ============================================================================
 Name        : queuetest.c
 Author      : LIRUIJIE
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#define QueueNum 100
struct QueueInfo
{
	char buf[10];
	int TcpNum;
	struct QueueInfo *Next;
};
struct QueueInfo Queue[QueueNum];
struct QueueInfo *QueueHead,*QueueTail;
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
void EnQueue(char *recvbuf,int TcpNum)
{
	if(IsQueueFull())
			return;
	QueueTail->TcpNum = TcpNum;
	/*memset(QueueTail->buf,'\0',TcpFrameMaxLen);
	memcpy(QueueTail->buf,recvbuf,TcpFrameMaxLen);*/

	QueueTail = QueueTail->Next;
}
void DelQueue(char *outbuf,int *TcpNum)
{
	*TcpNum = QueueHead->TcpNum;
	/*memset(outbuf,'\0',TcpFrameMaxLen);
	memcpy(outbuf,QueueHead->buf,TcpFrameMaxLen);*/
	QueueHead = QueueHead->Next;
}
int IsQueueEmpty()
{
	return( QueueHead == QueueTail );

}


int main(void)
{
	puts("!!!Hello World!!!"); /* prints !!!Hello World!!! */
	int i = 0;
	int ret;
	char buf[10];
	InitQueue();
	while(1)
	{
		i=0;
		while( !IsQueueFull())
		{
			EnQueue((char *)"123",i++);
		}
		i=0;
		while(!IsQueueEmpty())
		{
			DelQueue(buf,&ret);
			printf("%d--%d  \n ",i++,ret);
			//sleep(1);
		}
		sleep(2);
	}
	//while(1);
	return EXIT_SUCCESS;
}
