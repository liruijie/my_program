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

#include	<sys/types.h>	/* basic system data types */
#include	<sys/socket.h>	/* basic socket definitions */
#include	<sys/time.h>	/* timeval{} for select() */
#include	<time.h>		/* timespec{} for pselect() */
#include	<netinet/in.h>	/* sockaddr_in{} and other Internet defns */
#include	<arpa/inet.h>	/* inet(3) functions */
#include	<errno.h>
#include	<fcntl.h>		/* for nonblocking */
#include	<netdb.h>
#include	<signal.h>
#include	<sys/stat.h>	/* for S_xxx file mode constants */
#include	<sys/uio.h>		/* for iovec{} and readv/writev */
#include	<sys/wait.h>
#include	<sys/un.h>		/* for Unix domain sockets */


#define CALLBACK

enum
{
		value,
#define Value value
		char_value='c'
#define CharValue char_value
};


//1111
typedef char (* CALLBACK _PTRFUN)(int,char *);
CALLBACK _PTRFUN pFun;
//2222
char (*pFun2)(int,char *);
char glFun(int a,char *p)
{
	printf(p);
	return (char )a;
}

void get_char(char **aa,char b)
{
	while(**aa != '\0')
	{
		if( **aa == b)
		{
			printf("get\n");
			return;
		}
		else
			*aa += 1;
	}
}
int main()
{
char buf[6]="abcdf";
buf[5] = '\0';
char *a ;
	a= buf;
	get_char(&a,'c');
	printf("buf = %s ; a = %s",buf,a);



		char c;
		time_t time_now = 0;
		time_t time_now_1 = 0;
		time(&time_now);
		time_now_1 = time(NULL);
		printf("time_now = %ld,time_now_1= %ld",time_now,time_now_1);
		 struct tm *p = gmtime(&time_now);
		 printf("week value = %d\n",p->tm_wday);



		printf("int  = %d,char = %c",Value,CharValue);

		printf("%05ldaaaaa\n",time_now);
    pFun = glFun;
    pFun2 = (_PTRFUN )glFun;
    c = (*pFun)(0x32,"this is pFun\n");
    printf("pFun      %c\n",c);
    c = (*pFun2)(0x33,"this is pFun2\n");
    printf("pFun2     %c",c);
    return 0;
}


/*
int main ()
{
	time_t rawtime;
	struct tm * timeinfo;
	struct tm timeinfo1;
	char buf[] = "2000-01-01 00:00:00";
	char fmt[] = "%Y-%m-%d %H:%M:%S";
	long timevalue;
	time ( &rawtime );
	timeinfo = localtime ( &rawtime );

	sprintf(buf,"%d-%02d-%02d %02d:%02d:%02d\n",timeinfo->tm_year+1900,timeinfo->tm_mon+1,timeinfo->tm_mday,timeinfo->tm_hour,timeinfo->tm_min,timeinfo->tm_sec);
	printf(buf);


	if (strptime(buf, fmt, &timeinfo1) != NULL)
	{
		memset(buf,'\0',sizeof(buf));
		sprintf(buf,"%d-%02d-%02d %02d:%02d:%02d\n",timeinfo1.tm_year+1900,timeinfo1.tm_mon+1,timeinfo1.tm_mday,timeinfo1.tm_hour,timeinfo1.tm_min,timeinfo1.tm_sec);
		printf(buf);
		timevalue = mktime(&timeinfo1);
		printf("%ld\n",timevalue);
	}

	return(0);
}*/




