/*
 ============================================================================
 Name        : GSM_Message.c
 Author      : liruijie
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include "include.h"

#define SerialNum "/dev/ttyS2"
#define MAXDATASIZE 512
static int	GprsFd = -1;
int Udp_fd,Tcp_fd;
struct sockaddr_in Modem_addr;
struct sockaddr_in WebSys_addr; //描述客户端地址
int main(void)
{
	unsigned char Text[] = "感谢您对我公司的认同并前来面试，在面试中您良好的表现给我们印象深刻。——北京博研智通科技有限公司人事部。";

	if(CreateUdpToModem()==false)
	{
		return false;
	}
	if(InitDevice() == false)
	{
		printf("Init Device failure\n");
		return false;
	}

	SendMessage((unsigned char *)"18511300345",(unsigned char *)Text);

	close(GprsFd);
	return EXIT_SUCCESS;
}
bool CreateUdpToModem()
{
	struct sockaddr_in myaddr;
	if ((Udp_fd = socket(PF_INET, SOCK_DGRAM, 0)) < 0)
	{
		perror("fail to socket:udp\n");
		return false;
	}
	int on=1;
	if((setsockopt(Udp_fd,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(on)))<0)
	{
		perror("setsockopt failed:udp\n");
		return false;
	}
	bzero(&myaddr, sizeof(myaddr));
	myaddr.sin_family = AF_INET;
	myaddr.sin_port = htons(10088);
	myaddr.sin_addr.s_addr = INADDR_ANY;
	//int bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
	if ( bind( Udp_fd, (struct sockaddr *)&myaddr, sizeof(myaddr) ) < 0 )
	{
		perror("fail to bind:udp\n");
		return false;
	}
	bzero(&Modem_addr,sizeof(struct sockaddr_in));
	Modem_addr.sin_family=AF_INET;
	Modem_addr.sin_addr.s_addr=inet_addr("192.168.1.222");
	Modem_addr.sin_port=htons(10088);


	return true;
}


bool InitDevice()
{
	char RecvBuf[MAXDATASIZE];
	//测试

	SendToModem((char *)"at\r\n",4);
	if(RecvFromModem( RecvBuf,1))
	{
		printf("设备正常\n");
	}
	else
	{
		printf("Modem response error@Line%d",__LINE__);
		return false;
	}

	//关闭回显
	SendToModem((char *)"ate0\r\n" ,6);
	if(RecvFromModem( RecvBuf,1)>=6)
	{
		printf("关闭回显\n");
	}
	else
	{
		printf("Modem response error: %s@Line%d",RecvBuf,__LINE__);
		return false;
	}

	//设置文本模式
	SendToModem((char *)"at+cmgf=1\r\n" ,11);
	if(RecvFromModem( RecvBuf,1) >= 6)
	{
		if(strcmp(RecvBuf,"\r\nOK\r\n") != 0 )
		{
			printf("Modem response error: %s@Line%d",RecvBuf,__LINE__);
			return false;
		}
	}
	//设置文本模式参数
	SendToModem((char *)"at+csmp=17,167,2,25\r\n" ,21);
	if(RecvFromModem( RecvBuf,1) >= 6)
	{
		if(strcmp(RecvBuf,"\r\nOK\r\n") != 0 )
		{
			printf("Modem response error: %s@Line%d",RecvBuf,__LINE__);
			return false;
		}
	}
	//设置编码类型
	SendToModem((char *)"at+cscs=\"ucs2\"\r\n" ,16);
	if(RecvFromModem(  RecvBuf,1) >= 6)
	{
		if(strcmp(RecvBuf,"\r\nOK\r\n") != 0 )
		{
			printf("Modem response error: %s@Line%d",RecvBuf,__LINE__);
			return false;
		}
	}
	printf("Init Device success\n");
	return true;
}

bool SendMessage(unsigned char *PhoneNum,unsigned char *Text)
{

	//printf("%d",(int )strlen((char *)Text));
	if( SendPhoneNum(PhoneNum) == false )
	{
		return false;
	}

	if( !SendText(Text) )
	{
		return false;
	}
	return true;
}
int SendPhoneNum(unsigned char *PhoneNum)
{
	char AT_Cmd[100];
	unsigned char OutPhoneNum[30];
	char RecvBuf[10];
	char StaticBuf[6] = {0x0D,0x0A,0x3E,0x20,'\0','\0'};
	int OutLen;
	unsigned char PhoneNum_ASC[60];

	OutLen = g2u((char *)PhoneNum,strlen((char *)PhoneNum),(char *)(OutPhoneNum),30);
	memset(PhoneNum_ASC,'\0',60);
	ConverHex2ASC((unsigned char *)OutPhoneNum,OutLen,PhoneNum_ASC,60);

	memset(AT_Cmd,'\0',100);
	sprintf(AT_Cmd,"at+cmgs=\"%s\"\r\n",PhoneNum_ASC);
	printf("%s\n",AT_Cmd);

	if( !SendToModem(AT_Cmd,strlen(AT_Cmd)) )
	{
		printf("Send phone num AT Cmd error\n");
		return false;
	}
	memset(RecvBuf,'\0',10);
	if( !RecvFromModem(RecvBuf,5) )
	{
		printf("Receive phone num AT Cmd response error,receive\n");
		return false;
	}

	if( strcmp(RecvBuf,StaticBuf) != 0 )
	{
		printf("Receive phone num AT Cmd response error,buffer\n");
		return false;
	}

	return true;
}

int SendText(unsigned char *Text)
{

	unsigned char OutText[MaxTextLen*2];
	int OutLen;
	unsigned char Text_ASC[MaxTextLen*4];
	char EndChar = 0x1A;
	int ret;
	char RecvBuf[50];
	OutLen = g2u((char *)Text,strlen((char *)Text),(char *)(OutText),MaxTextLen*2);
	memset(Text_ASC,'\0',MaxTextLen*4);
	ConverHex2ASC(OutText,OutLen,Text_ASC,1000);
	printf("%s\n",Text_ASC);
	if( !SendToModem((char * )Text_ASC,OutLen*2) )
	{
		printf("Send Text error\n");
		return false;
	}
	if( !SendToModem((char *)&(EndChar),1) )
	{
		printf("Send EndChar error\n");
		return false;
	}

	ret = RecvFromModem(RecvBuf,30);
	if( ret > 6)
	{
		if(strncmp(RecvBuf+ret-6,"\r\nOK\r\n",6) != 0)                  //+CMGS: 7\r\n\r\nOK\r\n
		{
			printf("Send Message Error\n");
			return false;
		}
		else
		{
			printf("Message send OK\n");
		}
	}
	else
	{
		printf("receive response timeout\n");
		return false;
	}
	return true;
}



int code_convert(char *from_charset,char *to_charset,char *inbuf,size_t inlen,char *outbuf,size_t outlen)
{
	iconv_t cd;
	//size_t rc;

	size_t outlenleft = outlen;
	char **pin = &inbuf;
	char **pout = &outbuf;
	cd = iconv_open(to_charset,from_charset);
	if (cd==0) return -1;
	memset(outbuf, 0, outlen);
	if(iconv(cd,pin,&inlen,pout,&outlenleft) == -1)
	{
		iconv_close(cd);
		return -1;
	}
	iconv_close(cd);
	return (int)(outlen-outlenleft);
}

int u2g(char *inbuf,int inlen,char *outbuf, int outlen)
{
	return code_convert((char *)"UCS-2",(char *)"UTF-8",inbuf,inlen,outbuf,outlen);
}

int g2u(char *inbuf,int inlen,char *outbuf, int outlen)
{
	return code_convert((char *)"UTF-8",(char *)"UCS-2",inbuf,inlen,outbuf,outlen);
}


int ConverHex2ASC(unsigned char *Hex, int HexLen, unsigned char *ASC, int ASCLen)
{
	int i;
	if(!Hex || !ASC)
	{
		return -1;
	}
	if(HexLen > ASCLen / 2)
	{
		return -1;
	}
	for(i=0; i<HexLen; i ++)
	{
		/*if(Hex2ASC(*(Hex + i+1), ASC + 2*i)!=0)
		{
			return -1;
		}
		if(Hex2ASC(*(Hex + i), ASC + 2*(i+1))!=0)
		{
			return -1;
		}*/

		if(Hex2ASC(*(Hex + i), ASC + 2*i)!=0)
		{
			return -1;
		}
	}
	return 0;
}

int Hex2ASC(unsigned char Hex, unsigned char *ASC)
{
	switch(Hex >> 4)
	{
		case 0: case 1: case 2: case 3:
		case 4: case 5: case 6: case 7:
		case 8: case 9:
			*ASC = (Hex>>4) + '0';
			break;
		case 0xa: case 0xb: case 0xc:
		case 0xd: case 0xe: case 0xf:
			*ASC = (Hex>>4) -0xA + 'A';
			break;
	}
	switch(Hex & 0xF)
	{
		case 0: case 1: case 2: case 3:
		case 4: case 5: case 6: case 7:
		case 8: case 9:
			*(ASC+1) = (Hex & 0xF) + '0';
			break;
		case 0xa: case 0xb: case 0xc:
		case 0xd: case 0xe: case 0xf:
			*(ASC+1) = (Hex & 0xF) - 0xA + 'A';
			break;
	}
	return 0;
}
int SendToModem(char *send_buf,int data_len)
{
		int len;
		len =sendto(Udp_fd,send_buf,data_len,0,(struct sockaddr *)&Modem_addr,sizeof(struct sockaddr_in));
		if(len == data_len)
		{
			return len;
		}
		else
		{
			return 0;
		}
}
int RecvFromModem( char *rcv_buf,int time_s)
{
		struct timeval 	timeVal;
		fd_set  fdSet;
		int ret,numbytes;
		int sin_size = sizeof(Modem_addr);

		//printf("here is TCPClient_Data_Receive\n");
		timeVal.tv_sec = time_s;
		timeVal.tv_usec = 0;

		FD_ZERO(&fdSet);
		FD_SET(Udp_fd, &fdSet);

		ret = select((int) Udp_fd + 1, &fdSet,NULL, NULL, &timeVal);
		if( ret == 0 )    //select timeout
		{
			//printf("Connect select timeout with result=%d\n",ret);
			return 0;
		}
		if( !ret )    //select error
		{
			//printf("Connect select failed with result=%d\n",ret);
			return 0;
		}
		else          //select success
		{
			memset(rcv_buf,'\0',MAXDATASIZE);
			if ((numbytes=recvfrom(Udp_fd,  rcv_buf,  MAXDATASIZE,  0,  (struct sockaddr *)&Modem_addr, (socklen_t *) &sin_size))==-1)
			 {
				perror("recv ");
				sleep(2);
				exit(1);
			 }
			rcv_buf[numbytes] = '\0';
			return numbytes;
		}
		printf("exit TCPClient_Data_Receive\n");
		return 0;
}















#if 0
bool OpenSerial()
{
	GprsFd = open(SerialNum, O_RDWR);
	if(GprsFd < 0)
	{
		return false;
	}
	if( SetSerialOpt(GprsFd,115200,8,'n',1) == false )
	{
		return false;
	}
	return true;
}
int SetSerialOpt(int fd,int nSpeed, int nBits, char nEvent, int nStop)
{
    struct termios newtio,oldtio;
    /*保存测试现有串口参数设置，在这里如果串口号等出错，会有相关的出错信息*/
    if ( tcgetattr( fd,&oldtio) != 0)
    {
        perror("SetupSerial\n");
         return false;
     }
    bzero( &newtio, sizeof( newtio ) );

    /*步骤一，设置字符大小*/
    newtio.c_cflag |= CLOCAL | CREAD;  //修改控制模式，保证程序不会占用串口  使得能够从串口中读取输入数据
                        //CLOCAL      本地连接（不改变端口所有者）
    				    //CREAD       接收使能
    newtio.c_cflag &= ~CSIZE;
    					//CSIZE     数据位的位掩码
    newtio.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);    //配置
    					//ICANON    启用规范模式
						//ECHO    启用本地回显功能
						//ECHOE    若设置ICANON，则允许退格操作
						//ISIG    若收到信号字符（INTR、QUIT等），则会产生相应的信号
    newtio.c_oflag  &= ~OPOST;   /*Output*/
    					//OPOST    启用输出处理功能，如果不设置该标志，则其他标志都被忽略
    /*
    //设置数据流控制
    switch(flow_ctrl)
    {

        case 0 ://不使用流控制
        	newtio.c_cflag &= ~CRTSCTS;
            break;

        case 1 ://使用硬件流控制
        	 newtio.c_cflag |= CRTSCTS;
             break;
        case 2 ://使用软件流控制
        	newtio.c_cflag |= IXON | IXOFF | IXANY;
            break;
     }
    */

    /*设置停止位*/
    switch( nBits )
    {
    	case 5:
    		newtio.c_cflag |= CS5;
            break;
        case 6:
        	newtio.c_cflag |= CS6;
            break;
        case 7:
            newtio.c_cflag |= CS7;
            break;
        case 8:
            newtio.c_cflag |= CS8;
            break;
        default:
        	printf("Unsupported data size\n");
        	printf("Use default data size:8\n");
        	newtio.c_cflag |= CS8;
        	break;
    }
/*设置奇偶校验位*/
    switch( nEvent )
    {
    	case 'o':
        case 'O': //奇校验
            newtio.c_cflag |= PARENB;                 //使能校验
            newtio.c_cflag |= PARODD;                 //设置奇校验
            newtio.c_iflag |= (INPCK | ISTRIP);       //激活c_iflag中的对于输入数据的奇偶校验使能
            break;
        case 'e':
        case 'E': //偶校验
            newtio.c_cflag |= PARENB;                 //使能校验
            newtio.c_cflag &= ~PARODD;				  //设置关闭奇校验  即开启偶校验
            newtio.c_iflag |= (INPCK | ISTRIP);       //激活c_iflag中的对于输入数据的奇偶校验使能
            					//ISTRIP    裁减掉第8位比特
            break;
        case 'n':
        case 'N': //无奇偶校验位
            newtio.c_cflag &= ~PARENB;
            break;

        default:   //默认无校验
        	newtio.c_cflag &= ~PARENB;
        	break;

    }
    /*设置波特率*/
    switch( nSpeed )
    {
        case 2400:
            cfsetispeed(&newtio, B2400);
            cfsetospeed(&newtio, B2400);
        break;
        case 4800:
            cfsetispeed(&newtio, B4800);
            cfsetospeed(&newtio, B4800);
        break;
        case 9600:
            cfsetispeed(&newtio, B9600);
            cfsetospeed(&newtio, B9600);
        break;
        case 115200:
            cfsetispeed(&newtio, B115200);
            cfsetospeed(&newtio, B115200);
        break;
        case 460800:
            cfsetispeed(&newtio, B460800);
            cfsetospeed(&newtio, B460800);
        break;
        default:  //默认 115200
            cfsetispeed(&newtio, B115200);
            cfsetospeed(&newtio, B115200);
        break;
    }
    /*设置停止位*/
    if( nStop == 1 )
        newtio.c_cflag &= ~CSTOPB;
    else if ( nStop == 2 )
        newtio.c_cflag |= CSTOPB;
    /*设置等待时间和最小接收字符  在对接收字符和等待时间没有特别要求的情况下，可以将其设置为0，则在任何情况下read()函数立即返回*/
    newtio.c_cc[VTIME] = 0;
    newtio.c_cc[VMIN] = 0;
    /*处理未接收字符*/
    tcflush(fd,TCIFLUSH);
    /*激活新配置*/
    if((tcsetattr(fd,TCSANOW,&newtio))!=0)      //TCSANOW：不等数据传输完毕就立即改变属性。
    	                                        //TCSADRAIN：等待所有数据传输结束才改变属性。
    											//TCSAFLUSH：清空输入输出缓冲区才改变属性。
    {
        perror("com set error");
        return false;
    }
    printf("set done!\n");
    return true;
}
int UART_Recv(int fd, char *rcv_buf,int time_s)
{
    int len,flag_select;
    fd_set fs_read;

    struct timeval time;

    FD_ZERO(&fs_read);
    FD_SET(fd,&fs_read);

    time.tv_sec = time_s;
    time.tv_usec = 0;


    flag_select = select(fd+1,&fs_read,NULL,NULL,&time);

    if(flag_select)
    {
	   memset(rcv_buf,'\0',MAXDATASIZE);
	   usleep(10);
	   len = read(fd,rcv_buf,MAXDATASIZE);
	   return len;
    }
    else
    {
	  return 0;
    }
}
int UART_Send(int fd, char *send_buf,int data_len)
{
    int len = 0;

    len = write(fd,send_buf,data_len);
    if(len == data_len )
	  {
			 return len;
	  }
    else
      {
			 tcflush(fd,TCOFLUSH);
			 return 0;
      }

}
#endif



