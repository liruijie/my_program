/*************************************************** 
 * �ļ�����queue.cpp
 * ��Ȩ��
 * ���������в���
 * �޸��ˣ�����
 * �޸�ʱ�䣺2014-1-6
 * �޸İ汾��V0.1
 * �޸����ݣ�
***************************************************/
#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <occi.h>
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include "/home/ITC100/occi_opration.h"
#include "/home/ITC100/public_data.h"
#include "/home/ITC100/queue.h"

using namespace oracle::occi;

using namespace std;

unsigned char temp[512];  
 


/*************************************************** 
 * ��������queue_create
 * �����������ڶ���β����������
 *�����ʵı�
 *���޸ĵı�
* ����˵����hq :���нṹ
				 buf:����������
*����ֵ: ���нṹ��
***************************************************/
sequeue * queue_create()
{
    sequeue *sq;
    sq = (sequeue *)malloc(sizeof(sequeue));
    if (sq == NULL)
        return sq;
    sq->front = sq->rear = 0;
	
    return sq;
}

int enqueue(sequeue *sq, unsigned char * buf)
{
    if ((sq->rear + 1) % N == sq->front )
    {
        printf("queue is full\n");
        return -1;
    }
    memcpy((char*)sq->data[sq->rear],(char*)buf,512);
//	printf("buf[10] = %x\n",buf[10]);
//	printf("sq->data[sq->rear][10] = %x\n",sq->data[sq->rear][10]);
	
  //  sq->data[sq->rear] = x;
    sq->rear = (sq->rear+1) % N;

    return 0;
}

int dequeue(sequeue *sq, unsigned char * *buf)
{
    if (sq->front == sq->rear)
    {
        printf("queue is empty\n");
        return -1;
    }
	*buf = sq->data[sq->front];
  //  strcpy((char*)*buf ,(char*)sq->data[sq->front]);
    sq->front = (sq->front + 1) % N;

    return 0;
}

int queue_empty(sequeue *sq)
{
    return (sq->front == sq->rear);
}