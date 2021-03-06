/*
 ============================================================================
 Name        : pthread_arg.c
 Author      : LIRUIJIE
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */



#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <fcntl.h>
#include <wait.h>
#include <time.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <errno.h>
#define SingleRecvMaxLen 100000
struct web_message_info
{

		unsigned char RecvBuf[SingleRecvMaxLen];
		unsigned char RecvBuf1[SingleRecvMaxLen];
		unsigned char RecvBuf2[SingleRecvMaxLen];
		struct sockaddr_in WebServerAddr;
};


void * pth_ttt(void *arg)
{
	struct web_message_info pthweb_message;
	unsigned char data[100];
	//pthread_detach(pthread_self());
	memcpy(&pthweb_message,arg,sizeof(struct web_message_info));
	free(arg);
	struct sockaddr_in addr = pthweb_message.WebServerAddr;
	memcpy(data,pthweb_message.RecvBuf,100);
	printf("there is pthread\n");
	printf("string = %s",data);
	printf("addr.port = %d\t",addr.sin_port);
	printf("addr.ip = %d\n",addr.sin_addr.s_addr);

	//return (void *)0;
	pthread_exit((void *)0);
}
void test(unsigned char *data,struct sockaddr_in WebAddr)
{
	struct web_message_info *web_message = malloc(sizeof(struct web_message_info));
	memcpy(web_message->RecvBuf,data,100);
	web_message->WebServerAddr = WebAddr;
	pthread_t ptr;
	pthread_attr_t attr;
	int detachstate;
	pthread_attr_init(&attr);
	pthread_attr_getdetachstate(&attr,&detachstate);
	if(detachstate==PTHREAD_CREATE_JOINABLE)
	{
		pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);//设置为线程分离属性设置为分离
	}
	pthread_create(&ptr,&attr,pth_ttt,web_message);
	pthread_attr_destroy(&attr);//回收分配给属性的资源


}

int main(void)
{
	unsigned char buf[100] = "hello pthread\n";
	struct sockaddr_in myaddr;
	myaddr.sin_family = PF_INET;
	myaddr.sin_port = htons(10010);
	myaddr.sin_addr.s_addr  = inet_addr("192.168.1.1");
	printf("this is main\n");
	printf("string = %s",buf);
	printf("addr.port = %d\t",myaddr.sin_port);
	printf("addr.ip = %d\n\n\n",myaddr.sin_addr.s_addr);

	sleep(1);
	while(1)
	{
		test(buf,myaddr);
		usleep(1000*10);
	}
	sleep(5);
	return EXIT_SUCCESS;
}



