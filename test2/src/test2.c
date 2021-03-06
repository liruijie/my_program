/*
 ============================================================================
 Name        : test.c
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
#define SingleRecvMaxLen 100
struct web_message_info
{
		unsigned char RecvBuf[SingleRecvMaxLen];
		struct sockaddr_in WebServerAddr;
};
void * pth_ttt(void *arg)
{
	struct web_message_info web_message = * (struct web_message_info *)arg;
	unsigned char data[100];
	struct sockaddr_in addr = web_message.WebServerAddr;
	memcpy(data,web_message.RecvBuf,100);
	printf("there is pthread\n");
	printf("string = %s\n",data);
	printf("addr.port = %d\t",addr.sin_port);
	printf("addr.ip = %d\n",addr.sin_addr.s_addr);
	return (void *)0;

}
void test(unsigned char *data,struct sockaddr_in WebAddr)
{
	struct web_message_info web_message;
	memcpy(web_message.RecvBuf,data,100);
	web_message.WebServerAddr = WebAddr;
	pthread_t ptr;
	pthread_create(&ptr,NULL,pth_ttt,&web_message);
}

int main(void)
{
	unsigned char buf[100] = "hello pthread\n";
	struct sockaddr_in myaddr;
	myaddr.sin_family = PF_INET;
	myaddr.sin_port = htons(10010);
	myaddr.sin_addr.s_addr  = inet_addr("192.168.1.1");
	printf("this is main\n");
	printf("string = %s\n",buf);
	printf("addr.port = %d\t",myaddr.sin_port);
	printf("addr.ip = %d\n",myaddr.sin_addr.s_addr);

	sleep(1);

	test(buf,myaddr);
	return EXIT_SUCCESS;
}



