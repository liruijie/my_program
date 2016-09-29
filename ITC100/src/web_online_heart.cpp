/*************************************************** 
 * 文件名：web_online_heart.CPP
 * 版权：
 * 描述：接收WEB 界面发送来的在线心跳
 * 修改人：栾宇
 * 修改时间：2014-1-10
 * 修改版本：V0.1
 * 修改内容：
***************************************************/
#include <iostream>
#include <string.h>
#include <occi.h>
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include "/home/ITC100/signal_report.h"
#include "/home/ITC100/occi_opration.h"
#include "/home/ITC100/public_data.h"
#include "/home/ITC100/web_control_heart.h"
#include "/home/ITC100/web_online_heart.h"

using namespace std;
using namespace oracle::occi;

#define BUFF_NUM  512



/*************************************************** 
 * 函数名：revice_web_online_heart
 * 功能描述：接收web发来的控制心跳 更新信号机控制状态
 *被访问的表：
 *被修改的表：CONTROL_CMD
* 参数说明：无
*返回值:  无
***************************************************/

void revice_web_online_heart()
{
	/*创建UDP服务器*/
	int sockfd;
	unsigned char recv_buf[BUFF_NUM] ;
	struct sockaddr_in myaddr, peeraddr;
	 struct sockaddr SA;
	socklen_t peerlen;
	if ((sockfd = socket(PF_INET, SOCK_DGRAM, 0)) < 0)
	{
		perror("fail to socket");
		exit(-1);
	}
	   // 设置套接字选项避免地址使用错误  
    	int on=1;  
   	 if((setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(on)))<0)  
   	 {  
      	  	perror("setsockopt failed");  
       		 exit(EXIT_FAILURE);  
    	} 
	
	bzero(&myaddr, sizeof(myaddr));
	myaddr.sin_family = PF_INET;
	myaddr.sin_port = htons(10002);
	myaddr.sin_addr.s_addr = INADDR_ANY;
	//int bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
	if ( bind( sockfd, (struct sockaddr *)&myaddr, sizeof(myaddr) ) < 0 )
	{
		perror("fail to bind");
		exit(-1);
	}

	int signal_num;
	peerlen = sizeof(peeraddr);

	printf("new pthread, file=%s,func=%s,line=%d\n\n",__FILE__,__FUNCTION__,__LINE__);	
	int i;
	while(1)
	{
		
		bzero(recv_buf,sizeof(recv_buf));
		int num = recvfrom(sockfd, recv_buf, BUFF_NUM, 0, (struct sockaddr *)&peeraddr, &peerlen);

	
#if 0		
		printf("recv revice_web_online_heart :\n");
		for(i = 0; i< num;i++)
		{
			printf("%x",recv_buf[i]);
		}
		printf("\n");
#endif	
		
		if(check_buf_web(recv_buf) == 0)
		{
			if(recv_buf[9] == 0x70)
			{
			//	printf("receive a web online heart_beat\n");
				sendto(sockfd, recv_buf, num , 0, (struct sockaddr*)&peeraddr, sizeof(peeraddr));
			}
		}
		
	}
	
}


