/*************************************************** 
 * �ļ�����queue.h
 * ��Ȩ��
 * ����������ͷ�ļ�
 * �޸��ˣ�����
 * �޸�ʱ�䣺2014-1-6
 * �޸İ汾��V0.1
 * �޸����ݣ�
***************************************************/
#ifndef _QUEUE_H_
#define _QUEUE_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define N 1000

typedef struct
{
    unsigned char data[N][512];
    int front;
    int rear;
}sequeue;

sequeue * queue_create();
int enqueue(sequeue *sq, unsigned char * buf);
int dequeue(sequeue *sq, unsigned char * *buf);
int queue_empty(sequeue *sq);

#endif


