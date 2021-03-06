/*
 ============================================================================
 Name        : signal_function_test.c
 Author      : LIRUIJIE
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <pthread.h>
void * killfunc();
void * getsig();

void signalfunc(int sig)
{
	if(sig == SIGUSR1)
	{
		printf("get sigusr1\n");
	}
	else if(sig == SIGUSR2)
	{
		printf("get sigusr2\n");
	}
	else
	{
		printf("can not get signal\n");
	}

}

int main(void)
{
	puts("!!!Hello World!!!"); /* prints !!!Hello World!!! */
	printf("pid  = %d\n",getpid());
	pthread_t pth_killfun;
	pthread_t pth_get_sig_fun1;

	int ret;
	ret = pthread_create(&pth_killfun,NULL,killfunc,NULL);
	if(ret != 0)
	{
		perror("Fail to create pthread");
		exit(EXIT_FAILURE);
	}
	sleep(1);

	sleep(1);
	ret = pthread_create(&pth_get_sig_fun1,NULL,getsig,NULL);
	if(ret != 0)
	{
		perror("Fail to create pthread");
		exit(EXIT_FAILURE);
	}
	while(1);
	return EXIT_SUCCESS;
}


void * killfunc()
{
	int i;
	printf("pid  = %d\n",getpid());
	for(i = 0;i<1;i++)
	{
		kill(getpid(),SIGUSR1);
		sleep(1);
		kill(getpid(),SIGUSR2);
		sleep(1);
	}

	return (void *) 0;
}

void * getsig(void *arg)
{
	if(signal(SIGUSR1,signalfunc) == SIG_ERR)
	{
		printf("get sigusr1 error\n");
	}
	if(signal(SIGUSR2,signalfunc) == SIG_ERR)
	{
		printf("get sigusr2 error\n");
	}
	while(1);
	return (void *) 0;
}

