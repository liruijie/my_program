/*************************************************** 
 * 文件名：queue.h
 * 版权：
 * 描述：队列头文件
 * 修改人：栾宇
 * 修改时间：2014-1-6
 * 修改版本：V0.1
 * 修改内容：
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


