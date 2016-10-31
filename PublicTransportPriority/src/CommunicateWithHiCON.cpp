/*
 * CommunicateWithHiCON.cpp
 *
 * Created on: 2016年10月20日
 * Author: LIRUIJIE
 */

#include "include.h"
struct HiCON_Info HiCON={
		"admin",
		"admin",
};
#define		DISCONNECT	-1
#define		fail			0
#define 		success		1

extern int CurrentExistSignal;
int Sockfd_HiCON;

int CommunicateWithHiCON()
{
	pthread_t pth_ConnectHiCON;
	int ret;
	ret = pthread_create(&pth_ConnectHiCON,NULL,Connect_HiCON,NULL);
	if(ret != 0)
	{
		perror("创建HiCON通讯线程失败\n");
		exit(EXIT_FAILURE);
	}
	return true;
}

void * Connect_HiCON(void *arg)
{

	struct sockaddr_in HiCON_Addr;
	char recv_buf[1000];
	int ret;
	printf("create pthread CommunicateWithHiCON\n");
	pthread_t pth_subscribe;
	bzero(&HiCON_Addr, sizeof(HiCON_Addr));
	HiCON_Addr.sin_family = PF_INET;
	HiCON_Addr.sin_port = htons(HiCON.port);
	HiCON_Addr.sin_addr.s_addr = inet_addr("192.168.1.38");
	Sockfd_HiCON=InitHiCON_TCP();
	while(1)
	{
		if(connect(Sockfd_HiCON,(struct sockaddr *) &HiCON_Addr, sizeof(HiCON_Addr)) <0)
		{
			perror("连接HiCON失败\n");
			sleep(5);
		}
		else
		{
			//登陆操作
			_login:
			ret = Login();
			if(ret == fail)
			{
				printf("登陆失败，重新登陆\n");
				goto _login;
			}
			else if(ret ==  DISCONNECT)
			{
				printf("连接断开，重新连接\t 登陆\n");
				close(Sockfd_HiCON);
				Sockfd_HiCON=InitHiCON_TCP();
				continue;
			}
			//订阅操作
			ret = pthread_create(&pth_subscribe,NULL,Subscribe_Pthread,NULL);
			if(ret != 0)
			{
				perror("创建订阅消息线程失败\n");
				exit(EXIT_FAILURE);
			}
/*
			_subscribe:
			ret = Subscribe();
			if(ret == fail)
			{
				printf("订阅失败，重新订阅\t \n");
				goto _subscribe;
			}
			else if(ret ==  DISCONNECT)
			{
				printf("连接断开，重新连接\t 订阅信息\n");
				close(Sockfd_HiCON);
				Sockfd_HiCON=InitHiCON_TCP();
				continue;
			}
*/
			while(1)
			{
				ret = Receive_HiCON(recv_buf,5);
				if(ret == DISCONNECT)
				{
					printf("连接断开，重新连接\t 接收数据\n");
					close(Sockfd_HiCON);
					Sockfd_HiCON=InitHiCON_TCP();
					break;
				}
				else if(ret > 0)
				{
					puts(recv_buf);
				}
			}
		}
	}
	return (void *)0;
}

int InitHiCON_TCP()
{
	int sockfd;
	while(1)
	{

		if ((sockfd = socket(PF_INET, SOCK_STREAM, 0)) < 0)
		{
			perror("fail to socket");
			continue;
		}
		int on=1;
		if((setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(on)))<0)
		{
			perror("setsockopt failed");
			close(sockfd);
			continue;
		}
		return sockfd;
	}
}


int Login()
{
	char send_buf[300] ;
	char recv_buf[1000];
	int ret;
	sprintf(send_buf,"<?xml version=\"1.0\" encoding=\"GB2312\"?>\
							<systemScription System=\"TCIP\" Version=\"1.0\" >\
								<subSystem></subSystem>\
								<messageType>1</messageType>\
								<isRequest>1</isRequest>\
								<needResponse>1</needResponse >\
								<result></result>\
								<flag></flag >\
								<messageContent>\
									<User>%s</User>\
								   <Password>%s</Password>\
								</messageContent>\
							</systemScription>",
							HiCON.user,HiCON.passwd);
	ret= Send_HiCON(send_buf,strlen(send_buf));
	if(ret != true)
	{
		return ret;
	}
	ret = Receive_HiCON(recv_buf,5);
	if(ret <= 0)
	{
		return ret;
	}
	else
	{
		//pasing data
	}
	return DISCONNECT;
}


void * Subscribe_Pthread(void * arg)
{
	pthread_detach(pthread_self());
	cout << "HiCON Subscribe_Pthread\t" << __FILE__ << __FUNCTION__<< __LINE__ <<endl;
	int i;
	int ret;
	for(i = 0;	i< CurrentExistSignal; )
	{
		Signal[i].subscribe_phase_status = 0;
		Signal[i].subscribe_priority_status = 0;
		Signal[i].subscribe_signal_status = 0;
		ret = Subscribe(i);
		if(ret == false)
		{
			continue;
		}
		if(ret == DISCONNECT)
		{
			exit(0);
		}
		i++;
	}
	return (void *)0;
}

int Subscribe(int index)
{
	char send_buf[300] ;
	int ret;
	/*
	 * 订阅联机状态
	 */
	sprintf(send_buf,"<?xml version=\"1.0\" encoding=\"GB2312\"?>\
							<systemScription System=\"TCIP\" Version=\"1.0\">\
								<subSystem></subSystem>\
								<messageType>2</messageType>\
								<isRequest>1</isRequest>\
								<needResponse>1</needResponse>\
								<result></result>\
								<flag></flag>\
								<messageContent>\
									<Spot>%d</Spot>\
									<SubType>3</SubType>\
									<Flag>0</Flag>\
								</messageContent>\
							</systemScription>",Signal[index].signal_id);
	ret= Send_HiCON(send_buf,strlen(send_buf));
	if(ret != true)
	{
		return ret;
	}
	int i=0;
	while(i)
	{
		usleep(1000 *10);
		if(Signal[index].subscribe_phase_status == 1)
			break;
		i++;
		if(i> 5*100)
		{
			return false;
		}
	}
	/*
	 * 订阅相位状态
	 */
	sprintf(send_buf,"<?xml version=\"1.0\" encoding=\"GB2312\"?>\
							<systemScription System=\"TCIP\" Version=\"1.0\">\
								<subSystem></subSystem>\
								<messageType>2</messageType>\
								<isRequest>1</isRequest>\
								<needResponse>1</needResponse>\
								<result></result>\
								<flag></flag>\
								<messageContent>\
									<Spot>%d</Spot>\
									<SubType>10</SubType>\
									<Flag>0</Flag>\
								</messageContent>\
							</systemScription>",Signal[index].signal_id);
	ret= Send_HiCON(send_buf,strlen(send_buf));
	if(ret != true)
	{
		return ret;
	}
	i=0;
	while(i)
	{
		usleep(1000 *10);
		if(Signal[index].subscribe_phase_status == 1)
			break;
		i++;
		if(i> 5*100)
		{
			return false;
		}
	}
	/*
	 * 订阅优先状态
	 */
	sprintf(send_buf,"<?xml version=\"1.0\" encoding=\"GB2312\"?>\
							<systemScription System=\"TCIP\" Version=\"1.0\">\
								<subSystem></subSystem>\
								<messageType>2</messageType>\
								<isRequest>1</isRequest>\
								<needResponse>1</needResponse>\
								<result></result>\
								<flag></flag>\
								<messageContent>\
									<Spot>%d</Spot>\
									<SubType>19</SubType>\
									<Flag>0</Flag>\
								</messageContent>\
							</systemScription>",Signal[index].signal_id);
	ret= Send_HiCON(send_buf,strlen(send_buf));
	if(ret != true)
	{
		return ret;
	}
	i = 0;
	while(i)
	{
		usleep(1000 *10);
		if(Signal[index].subscribe_priority_status == 1)
			break;
		i++;
		if(i> 5*100)			//5s
		{
			return false;
		}
	}
	return true;
}

int Send_HiCON(char *data,long len)
{
	long ret = send(Sockfd_HiCON,data,len,0);
	if(ret == -1)
	{
		return DISCONNECT;
	}
	else if(ret != len)
	{
		return false;
	}
	return true;
}

int Receive_HiCON(char *data,int timeout_s)
{
	fd_set  HiCON_fdset;
	struct timeval 	timeVal;
	int ret;
	timeVal.tv_sec = timeout_s;
	timeVal.tv_usec= 0;
	FD_ZERO(&HiCON_fdset);
	FD_SET(Sockfd_HiCON, &HiCON_fdset);
	ret = select((int) Sockfd_HiCON + 1, &HiCON_fdset,NULL, NULL, &timeVal);
	if( ret == 0 )    //select timeout
	{
		return 0;
	}
	if( !ret )    //select error
	{
		printf("HiCON receive error result=%d,file=%s,function=%s,line=%d\n",ret,__FILE__,__FUNCTION__,__LINE__);
		return DISCONNECT;
	}
	else          //select success
	{
		ret = recv(Sockfd_HiCON,data,1000,0);
		if(ret==-1)
		{
			return DISCONNECT;
		}
		else if(ret == 0)
		{
			return DISCONNECT;
		}
		return ret;
	}
}

int PasingData(char *buf)
{
	xmlDocPtr pdoc;
	xmlXPathContextPtr Xpath = NULL;    //XPATH上下文指针
	xmlNodePtr node=NULL;
	xmlChar *text = NULL;
	xmlXPathObjectPtr Xresult =NULL;
	int messageType;
	int Return = fail;
	xmlKeepBlanksDefault(0);

	pdoc = xmlReadMemory(buf,(int )strlen(buf), NULL, "GB2312", XML_PARSE_NOERROR|XML_PARSE_NOBLANKS);//
	if(pdoc == NULL)
	{
		cout << "Buff is error,can not parse" << endl;
		return Return;
	}

	Xpath = xmlXPathNewContext(pdoc);     //创建一个XPath上下文指针
	if (Xpath == NULL)
	{
	   printf("context is NULL\n");
	   goto __FreeDoc;
	}

	Xresult = xmlXPathEvalExpression((xmlChar*)"//messageType", Xpath);
	if(CheckXmlResult(Xresult) == fail)
		goto __FreeXpath;
	node = Xresult ->nodesetval->nodeTab[0];
	text = xmlNodeGetContent(node);
	xmlXPathFreeObject(Xresult);
	Xresult = NULL;
	if(text == NULL)
		goto __FreeXpath;
	messageType = atoi((char *)text);
	xmlFree(text);
	switch (messageType)
	{
		//心跳消息
		case 0:
			break;
		//登录消息
		case 1:
			Return = Response_Login(Xpath);
			break;
		//订阅消息
		case 2:
			Return = Response_Subscribe(Xpath);
			break;
		//联机状态
		case 3:
			Response_SignalStatus(Xpath);
			Return = success;
			break;
		//相位状态
		case 10:
			Response_Phase(Xpath);
			Return = success;
			break;
		//公交优先状态
		case 19:
			Response_Priority(Xpath);
			Return = success;
			break;
		default:
			break;
	}
__FreeXpath:
	xmlXPathFreeContext(Xpath);
	Xpath = NULL;
__FreeDoc:
	xmlFreeDoc(pdoc);
   xmlCleanupParser();
   xmlMemoryDump();
   return Return;
}

int CheckXmlResult(xmlXPathObjectPtr Xresult)
{
		if (Xresult == NULL)
		{
		   printf("xmlXPathEvalExpression return NULL\n");
		   return false;
		}
		if (xmlXPathNodeSetIsEmpty(Xresult->nodesetval))   //检查查询结果是否为空
		{
		   xmlXPathFreeObject(Xresult);
		   return false;
		}
		return true;
}


int Response_Login(xmlXPathContextPtr Xpath)
{
	xmlXPathObjectPtr Xresult =NULL;
	xmlNodePtr node=NULL;
	xmlChar *text;
	int result;
	Xresult = xmlXPathEvalExpression((xmlChar*)"//result", Xpath);
	if(CheckXmlResult(Xresult) == fail)
	{
		return fail;
	}
	node = Xresult ->nodesetval->nodeTab[0];
	text = xmlNodeGetContent(node);
	xmlXPathFreeObject(Xresult);
	Xresult = NULL;
	if(text == NULL)
	{
		return fail;
	}
	result = atoi((char *)text);
	xmlFree(text);
	if(1 == result)
		return success;
	else
		return fail;
}

int Response_Subscribe(xmlXPathContextPtr Xpath)
{
	xmlXPathObjectPtr Xresult =NULL;
	xmlNodePtr node=NULL;
	xmlChar *text;
	int result;
	int i;
	int Spot,SubType;
	//首先判断返回结果是否正确
	Xresult = xmlXPathEvalExpression((xmlChar*)"//result", Xpath);
	if(CheckXmlResult(Xresult) == fail)
	{
		return fail;
	}
	node = Xresult ->nodesetval->nodeTab[0];
	text = xmlNodeGetContent(node);
	xmlXPathFreeObject(Xresult);
	Xresult = NULL;
	if(text == NULL)
	{
		return fail;
	}
	result = atoi((char *)text);
	xmlFree(text);
	if(1 != result)
		return fail;
	//在信号机结构体数组中找到相应的位置
	Xresult = xmlXPathEvalExpression((xmlChar*)"//Spot", Xpath);
	if(CheckXmlResult(Xresult) == fail)
	{
		return fail;
	}
	node = Xresult ->nodesetval->nodeTab[0];
	text = xmlNodeGetContent(node);
	xmlXPathFreeObject(Xresult);
	Xresult = NULL;
	if(text == NULL)
	{
		return fail;
	}
	Spot = atoi((char *)text);
	xmlFree(text);
	for(i = 0;i <= CurrentExistSignal;i++)
	{
		if(i == CurrentExistSignal)
		{
			return fail;
		}
		if(Spot == Signal[i].signal_id)
		{
			break;
		}
	}
	//判断是那种消息类型的订阅
	Xresult = xmlXPathEvalExpression((xmlChar*)"//SubType", Xpath);
	if(CheckXmlResult(Xresult) == fail)
	{
		return fail;
	}
	node = Xresult ->nodesetval->nodeTab[0];
	text = xmlNodeGetContent(node);
	xmlXPathFreeObject(Xresult);
	Xresult = NULL;
	if(text == NULL)
	{
		return fail;
	}
	SubType = atoi((char *)text);
	xmlFree(text);
	switch (SubType)
	{
		case 3:
			Signal[i].subscribe_signal_status = 1;
			break;
		case 10:
			Signal[i].subscribe_phase_status = 1;
			break;
		case 19:
			Signal[i].subscribe_priority_status = 1;
			break;
		default:
			return fail;
	}
	return success;
}

void Response_SignalStatus(xmlXPathContextPtr Xpath)
{
	xmlXPathObjectPtr Xresult =NULL;
	xmlNodePtr node=NULL;
	xmlChar *text;
	int i;
	int Spot,Status;
	Xresult = xmlXPathEvalExpression((xmlChar*)"//Spot", Xpath);
	if(CheckXmlResult(Xresult) == fail)
	{
		return ;
	}
	node = Xresult ->nodesetval->nodeTab[0];
	text = xmlNodeGetContent(node);
	xmlXPathFreeObject(Xresult);
	Xresult = NULL;
	if(text == NULL)
	{
		return ;
	}
	Spot = atoi((char *)text);
	xmlFree(text);
	for(i = 0;i <= CurrentExistSignal;i++)
	{
		if(i == CurrentExistSignal)
		{
			return ;
		}
		if(Spot == Signal[i].signal_id)
		{
			break;
		}
	}

	//判断信号机状态
	Xresult = xmlXPathEvalExpression((xmlChar*)"//Status", Xpath);
	if(CheckXmlResult(Xresult) == fail)
	{
		return ;
	}
	node = Xresult ->nodesetval->nodeTab[0];
	text = xmlNodeGetContent(node);
	xmlXPathFreeObject(Xresult);
	Xresult = NULL;
	if(text == NULL)
	{
		return ;
	}
	Status = atoi((char *)text);
	xmlFree(text);
	if(Status == 0)
	{
		Signal[i].signal_status = 2;
	}
	else
	{
		Signal[i].signal_status = 3;
	}
	char sqlbuf[200];
	sprintf(sqlbuf,"update UNIT_CUR_STATE set CONTROL_MODE = %d where UNIT_ID = %d",Signal[i].signal_status,Spot);
	//stmt->excute(sqlbuf);
}

void Response_Phase(xmlXPathContextPtr Xpath)
{
	xmlXPathObjectPtr Xresult =NULL;
	xmlNodePtr node=NULL;
	xmlChar *text;
	int i;
	int Spot;
	Xresult = xmlXPathEvalExpression((xmlChar*)"//Spot", Xpath);
	if(CheckXmlResult(Xresult) == fail)
	{
		return ;
	}
	node = Xresult ->nodesetval->nodeTab[0];
	text = xmlNodeGetContent(node);
	xmlXPathFreeObject(Xresult);
	Xresult = NULL;
	if(text == NULL)
	{
		return ;
	}
	Spot = atoi((char *)text);
	xmlFree(text);
	for(i = 0;i <= CurrentExistSignal;i++)
	{
		if(i == CurrentExistSignal)
		{
			return ;
		}
		if(Spot == Signal[i].signal_id)
		{
			break;
		}
	}
	//获取相位数量
	Xresult = xmlXPathEvalExpression((xmlChar*)"//Phases", Xpath);
	if(CheckXmlResult(Xresult) == fail)
	{
		return ;
	}
	xmlNodePtr phase_node=NULL,phase_child = NULL;
	phase_node = Xresult ->nodesetval->nodeTab[0];
	text = xmlGetProp(phase_node,(xmlChar *)"count");
	xmlXPathFreeObject(Xresult);
	Xresult = NULL;
	if(text == NULL)
	{
		return ;
	}
	Signal[i].phases_count = atoi((char *)text);
	xmlFree(text);

	phase_child = phase_node->children;
	xmlNodePtr red_node,yellow_node,green_node;
	int temp_t;
	for(temp_t=0;temp_t <Signal[i].phases_count;temp_t++)
	{
		if(phase_child == NULL)
		{
			return;
		}
		red_node = phase_child->children;
		yellow_node = red_node->next;
		green_node = yellow_node->next;
		if( (red_node == NULL) || (yellow_node == NULL)  || (green_node == NULL))
		{
			return;
		}
		text = xmlNodeGetContent(red_node);
		if(text == NULL)
		{
			return ;
		}
		Signal[i].phase[temp_t].red = atoi((char *)text);
		xmlFree(text);

		text = xmlNodeGetContent(yellow_node);
		if(text == NULL)
		{
			return ;
		}
		Signal[i].phase[temp_t].yellow = atoi((char *)text);
		xmlFree(text);

		text = xmlNodeGetContent(green_node);
		if(text == NULL)
		{
			return ;
		}
		Signal[i].phase[temp_t].green = atoi((char *)text);
		xmlFree(text);

		phase_child = phase_child->next;
	}
	//update sql
	return;
}

void Response_Priority(xmlXPathContextPtr Xpath)
{
	xmlXPathObjectPtr Xresult =NULL;
		xmlNodePtr node=NULL;
		xmlChar *text;
		int i;
		int Spot;
		Xresult = xmlXPathEvalExpression((xmlChar*)"//Spot", Xpath);
		if(CheckXmlResult(Xresult) == fail)
		{
			return ;
		}
		node = Xresult ->nodesetval->nodeTab[0];
		text = xmlNodeGetContent(node);
		xmlXPathFreeObject(Xresult);
		Xresult = NULL;
		if(text == NULL)
		{
			return ;
		}
		Spot = atoi((char *)text);
		xmlFree(text);
		for(i = 0;i <= CurrentExistSignal;i++)
		{
			if(i == CurrentExistSignal)
			{
				return ;
			}
			if(Spot == Signal[i].signal_id)
			{
				break;
			}
		}
		//获取优先数量
		Xresult = xmlXPathEvalExpression((xmlChar*)"//Preempt", Xpath);
		if(CheckXmlResult(Xresult) == fail)
		{
			return ;
		}
		xmlNodePtr preempt_node=NULL,preempt_child = NULL;
		preempt_node = Xresult ->nodesetval->nodeTab[0];
		text = xmlGetProp(preempt_node,(xmlChar *)"count");
		xmlXPathFreeObject(Xresult);
		Xresult = NULL;
		if(text == NULL)
		{
			return ;
		}
		Signal[i].priority_count = atoi((char *)text);
		xmlFree(text);

		preempt_child = preempt_node->children;
		xmlNodePtr Time_node,PreemptNo_node,PhaseNo_node,Type_node,PreemptTime_node;
		int temp_t;
//		char temp_buf[20];
		for(temp_t=0;temp_t <Signal[i].phases_count;temp_t++)
		{
			if(preempt_child == NULL)
			{
				return;
			}
			Time_node = preempt_child->children;
			PreemptNo_node = Time_node->next;
			PhaseNo_node = PreemptNo_node->next;
			Type_node = PhaseNo_node->next;
			PreemptTime_node = Type_node->next;

			if( (Time_node == NULL) || (PreemptNo_node == NULL)  || (PhaseNo_node == NULL) || (Type_node == NULL) || (PreemptTime_node == NULL))
			{
				return;
			}
			//Time：		公交优先请求发生时间，格式：DWORD
			text = xmlNodeGetContent(Time_node);
			if(text == NULL)
			{
				return ;
			}
			//memcpy(temp_buf,text,4);
			Signal[i].priority[temp_t].time = strtol((char *)text,NULL,10);
			xmlFree(text);

			//PreemptNo：	公交优先号，取值范围 5 ~ 8
			text = xmlNodeGetContent(PreemptNo_node);
			if(text == NULL)
			{
				return ;
			}
			Signal[i].priority[temp_t].PreemptNo = atoi((char *)text);
			xmlFree(text);

			//公交优先请求相位号，共8个相位，长度16，每2位代表一个相位号，取值 1 ~ 16
			text = xmlNodeGetContent(PhaseNo_node);
			if(text == NULL)
			{
				return ;
			}
			memcpy(Signal[i].priority[temp_t].PreemptPhaseNo,text,16);
			xmlFree(text);
			//公交优先类型，共8个相位，长度16，每2位代表一个相位的优先类型
			text = xmlNodeGetContent(Type_node);
			if(text == NULL)
			{
				return ;
			}
			memcpy(Signal[i].priority[temp_t].PreemptType,text,16);
			xmlFree(text);

			//公交优先时间，共8个相位，长度16，每2位代表一个相位的优先时间，每一个字节取值范围 0 ~ 255秒
			text = xmlNodeGetContent(PreemptTime_node);
			if(text == NULL)
			{
				return ;
			}
			memcpy(Signal[i].priority[temp_t].PreemptTime,text,16);
			xmlFree(text);

			preempt_child = preempt_child->next;
		}
		//update sql
		return;
}
