/*
 * pulicfunction.cpp
 *
 *  Created on: 2016年7月8日
 *      Author: root
 */

#include "include.h"

int IsUserValid(char *UserName,char *PassWd)
{
	char sqlbuf[200];
	int ret;
	sprintf(sqlbuf,"select t.user_id from USER_INFO t where user_name = '%s'",UserName);
	ret = JudgeIsExist(sqlbuf);
	if(ret != true)
	{
		return SDE_UserName;
	}
	unsigned char OutMD5[16];
	char PassWd_MD5[32];
	//select t.user_id from USER_INFO t where t.user_name = 'admin' and t.user_pwd = 'c4ca4238a0b923820dcc509a6f75849b'

	memset(PassWd_MD5,'\0',32);
	MD5_CTX md5;
	MD5Init(&md5);
	MD5Update(&md5,(unsigned char *)PassWd,strlen((char *)PassWd));
	MD5Final(&md5,OutMD5);
	memset(PassWd_MD5,'\0',32);
	char buf[2];
	int i;
	for(i=0;i<16;i++)
	{
		sprintf(buf,"%02x",OutMD5[i]);
		strncat(PassWd_MD5,buf,2);
		memset(buf,'\0',2);
	}
	sprintf(sqlbuf,"select t.user_id from USER_INFO t where user_name = '%s' and user_pwd = '%s'",UserName,PassWd_MD5);
	printf(sqlbuf);
	ret = JudgeIsExist(sqlbuf);
	if(ret != true)
	{
		return SDE_Pwd;
	}
	return succeed;
}


void UpdateToken(char *Token)
{

	int i;
	unsigned char encrypt[5];
	unsigned char decrypt[16];
	srand(time(0));
	i = 1 + rand()%100;

	sprintf((char *)encrypt,"%d",i);

	//memset(Token,'\0',32);
	MD5_CTX md5;
	MD5Init(&md5);
	MD5Update(&md5,encrypt,strlen((char *)encrypt));
	MD5Final(&md5,decrypt);
	memset(Token,'\0',32);
	char buf[2];
	for(i=0;i<16;i++)
	{
		memset(buf,'\0',2);
		sprintf(buf,"%02x",decrypt[i]);
		strncat(Token,buf,2);
	}
	memset(Token,'\0',32);
	strncat(Token,"0123456789123456",16);
}
int LoginLogout_Error(int ret,int ThreadNum)
{
	xmlDocPtr pdoc =NULL;
	xmlNodePtr node = NULL ;
	xmlXPathContextPtr Xpath = NULL;    //XPATH上下文指针
	xmlKeepBlanksDefault(0);

	pdoc = xmlReadMemory(TcpThread[ThreadNum].Message,(int )strlen(TcpThread[ThreadNum].Message), NULL, "utf-8", XML_PARSE_RECOVER|XML_PARSE_NOBLANKS);//

	Xpath = xmlXPathNewContext(pdoc);     //创建一个XPath上下文指针
	if (Xpath == NULL)
	{
	   printf("context is NULL");
	   return fail;
	}

	if( SwitchFromTo(Xpath) == fail)
	{
		return fail;
	}


	xmlXPathObjectPtr Xresult;       //XPATH对象指针，用来存储查询结果

	Xresult = xmlXPathEvalExpression((xmlChar*)"//Type", Xpath); //查询XPath表达式，得到一个查询结果

	if(CheckXmlResult(Xresult) == fail)
		return fail;
	node = Xresult ->nodesetval->nodeTab[0];
	xmlNodeSetContent(node, (const xmlChar *)"Error");// xmlNodeSetContent

	Xresult = xmlXPathEvalExpression((xmlChar*)"//Operation", Xpath); //查询XPath表达式，得到一个查询结果
	if(CheckXmlResult(Xresult) == fail)
	{
		return fail;
	}
	node = Xresult ->nodesetval->nodeTab[0];
	xmlUnlinkNode(node->children);

	xmlNodePtr node_SDO_Error = xmlNewNode(NULL, BAD_CAST "SDO_Error");
	xmlNewChild(node_SDO_Error, NULL, BAD_CAST "ErrObj",BAD_CAST "SDO_User");
	switch(ret)
	{
		case SDE_UserName:
			xmlNewChild(node_SDO_Error, NULL, BAD_CAST "ErrType",BAD_CAST "SDE_UserName");
			xmlNewChild(node_SDO_Error, NULL, BAD_CAST "ErrDesc",BAD_CAST "用户名错误");
			break;
		case SDE_Pwd:
			xmlNewChild(node_SDO_Error, NULL, BAD_CAST "ErrType",BAD_CAST "SDE_Pwd");
			xmlNewChild(node_SDO_Error, NULL, BAD_CAST "ErrDesc",BAD_CAST "口令错误");
			break;
		default:
			break;
	}
	xmlAddChild(node,node_SDO_Error);

	xmlChar *outbuf = NULL;
	int outlen;
	xmlDocDumpFormatMemoryEnc(pdoc, &outbuf, &outlen, "UTF-8", 1);
	//puts((char *)outbuf);

	TCPService_Data_Send(TcpThread[ThreadNum].commt_fd,(char *)outbuf,outlen);


	xmlXPathFreeObject(Xresult);
	Xresult = NULL;
	xmlXPathFreeContext(Xpath);
	xmlFreeDoc(pdoc);
	xmlFree(outbuf);
	xmlCleanupParser();
	xmlMemoryDump();
	return succeed;

}
int Get_Object_ID(xmlNodePtr OperationNode)
{
	int ID = 0;
	char *ID_buf;
	xmlNodePtr node_ID = OperationNode ->children ->children;          //operation节点->TSCCmdt节点->要操作的对象节点
	while(1)                         //  找到object节点
	{
		if(node_ID == NULL)
		{
			return -1;
		}
		else
		{
			if(strncmp("ID",(char *)node_ID->name,2) == 0)
			{
				break;
			}
			else
			{
				node_ID = node_ID ->next;
			}
		}
	}
	ID_buf = (char *)xmlNodeGetContent(node_ID);
	if(ID_buf[0] == '\0')
	{
		ID = 0;
	}
	else
	{
		ID = atoi(ID_buf);
	}

	return ID;
}

int Get_Node(xmlNodePtr Node,char *NodeName)
{
		while(1)                         //  找到object节点
		{
			if(Node == NULL)
			{
				return fail;
			}
			else
			{
				if(strncmp(NodeName,(char *)Node->name,strlen(NodeName)) == 0)
				{
					return succeed;
				}
				else
				{
					Node = Node ->next;
				}
			}
		}
		return fail;

}
int request_error(int ret,xmlXPathContextPtr Xpath,struct xmlResponseError ErrorInfo)
{
	xmlXPathObjectPtr Xresult_Body = xmlXPathEvalExpression((xmlChar*)"//Body", Xpath);
	if(CheckXmlResult(Xresult_Body) == fail)
		return fail;
	xmlNodePtr Body_node = Xresult_Body->nodesetval->nodeTab[0];
	xmlXPathObjectPtr Xresult_Operate = xmlXPathEvalExpression((xmlChar*)"//Operation", Xpath);
	if(CheckXmlResult(Xresult_Operate) == fail)
		return fail;
	xmlNodePtr Operate_node;
	int i;
	for(i=0;i<Xresult_Operate->nodesetval->nodeNr;i++)
	{
		Operate_node = Xresult_Operate->nodesetval->nodeTab[i];
		xmlUnlinkNode(Operate_node);
		xmlFreeNode(Operate_node);
	}
	Operate_node = NULL;
	Operate_node = xmlNewNode(NULL, BAD_CAST "Operation");
	xmlNewProp(Operate_node, BAD_CAST "order", BAD_CAST ErrorInfo.OperateProp_Order);                       //添加属性
	xmlNewProp(Operate_node, BAD_CAST "name", BAD_CAST ErrorInfo.OperateProp_Name);                       //添加属性
	xmlNodePtr Node_SDE_Error = xmlNewNode(NULL, BAD_CAST "SDE_Error");
	xmlNewChild(Node_SDE_Error, NULL, BAD_CAST "ErrObj",BAD_CAST ErrorInfo.ObjectnName);

	switch(ret)
	{
		case SDE_Version:
			xmlNewChild(Node_SDE_Error, NULL, BAD_CAST "ErrType",BAD_CAST "SDE_Version");
			xmlNewChild(Node_SDE_Error, NULL, BAD_CAST "ErrDesc",BAD_CAST "版本号错误");
			break;
		case SDE_Token:
			xmlNewChild(Node_SDE_Error, NULL, BAD_CAST "ErrType",BAD_CAST "SDE_Token");
			xmlNewChild(Node_SDE_Error, NULL, BAD_CAST "ErrDesc",BAD_CAST "无效令牌，会话错误");
			break;
		case SDE_Addr:
			xmlNewChild(Node_SDE_Error, NULL, BAD_CAST "ErrType",BAD_CAST "SDE_Addr");
			xmlNewChild(Node_SDE_Error, NULL, BAD_CAST "ErrDesc",BAD_CAST "地址错误");
			break;
		case SDE_MsgType:
			xmlNewChild(Node_SDE_Error, NULL, BAD_CAST "ErrType",BAD_CAST "SDE_MsgType");
			xmlNewChild(Node_SDE_Error, NULL, BAD_CAST "ErrDesc",BAD_CAST "数据包类型错误");
			break;
		case SDE_OperName:
			xmlNewChild(Node_SDE_Error, NULL, BAD_CAST "ErrType",BAD_CAST "SDE_OperName");
			xmlNewChild(Node_SDE_Error, NULL, BAD_CAST "ErrDesc",BAD_CAST "操作命令错误");
			break;
		case SDE_NotAllow:
			xmlNewChild(Node_SDE_Error, NULL, BAD_CAST "ErrType",BAD_CAST "SDE_NotAllow");
			xmlNewChild(Node_SDE_Error, NULL, BAD_CAST "ErrDesc",BAD_CAST "操作不允许");
			break;
		case SDE_Failure:
			xmlNewChild(Node_SDE_Error, NULL, BAD_CAST "ErrType",BAD_CAST "SDE_Failure");
			xmlNewChild(Node_SDE_Error, NULL, BAD_CAST "ErrDesc",BAD_CAST "Failure");//操作失败
			break;
		default:
			xmlNewChild(Node_SDE_Error, NULL, BAD_CAST "ErrType",BAD_CAST "SDE_Unknown");
			xmlNewChild(Node_SDE_Error, NULL, BAD_CAST "ErrDesc",BAD_CAST "其他未知错误");
			break;
	}

	xmlAddChild(Operate_node, Node_SDE_Error);
	xmlAddChild(Body_node, Operate_node);

	xmlXPathFreeObject(Xresult_Operate);
	Xresult_Operate = NULL;
	xmlXPathFreeObject(Xresult_Body);
	Xresult_Operate = NULL;
	return succeed;
}
