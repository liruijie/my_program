/*
 * xml.cpp
 *
 *  Created on: 2016年7月7日
 *      Author: root
 */
#include "include.h"

using namespace std;
using namespace oracle::occi;

int ReadXmlFromBuf(int ThreadNum)
{
		xmlDocPtr pdoc;

		xmlKeepBlanksDefault(0);

		pdoc = xmlReadMemory(TcpThread[ThreadNum].Message,(int )strlen(TcpThread[ThreadNum].Message), NULL, "utf-8", XML_PARSE_NOERROR|XML_PARSE_NOBLANKS);//
		if(pdoc == NULL)
		{
			cout << "Buff is error,can not parse" << endl;
			return fail;
		}
		AnalyseXml(pdoc,ThreadNum);

		xmlFreeDoc(pdoc);
	   xmlCleanupParser();
	   xmlMemoryDump();

		return 1;
}
void AnalyseXml(xmlDocPtr pdoc,int ThreadNum)
{

	xmlXPathContextPtr Xpath = NULL;    //XPATH上下文指针

	//xmlNodePtr root_node = xmlDocGetRootElement(pdoc);
	//TraversalNode(root_node);

	Xpath = xmlXPathNewContext(pdoc);     //创建一个XPath上下文指针
	if (Xpath == NULL)
	{
	   printf("context is NULL");

	}
	if(IsTokenValid(Xpath,ThreadNum) == fail)
	{
		xmlXPathFreeContext(Xpath);
		return;
	}
	//获取操作类型
	char Type[10];

	 if(GetOperateType(Type, Xpath))
	 {

		 if( strncmp(Type,"REQUEST",7) == 0)
		 {
			 RequestOperate(pdoc,Xpath,ThreadNum);
		 }
		 else if(strncmp(Type,"RESPONSE",7) == 0)
		 {

		 }
		 else if(strncmp(Type,"PUSH",4) == 0)
		 {

		 }
		 else if(strncmp(Type,"ERROR",5) == 0)
		 {

		 }
	 }
	xmlXPathFreeContext(Xpath);
	Xpath = NULL;
}


/*函数只有在令牌有效是才返回succeed   令牌无效和请求登录都返回fail*/
int IsTokenValid(xmlXPathContextPtr Xpath,int ThreadNum)
{
	xmlXPathObjectPtr Xresult;       //XPATH对象指针，用来存储查询结果

	Xresult = xmlXPathEvalExpression((xmlChar*)"//Token", Xpath); //查询Token节点，得到一个查询结果

	if (Xresult == NULL)
	{
	   printf("xmlXPathEvalExpression return NULL");
	   return SDE_Unknown;
	}
	if (xmlXPathNodeSetIsEmpty(Xresult->nodesetval))   //检查查询结果是否为空	令牌为空判断是不是请求登录的操作
	{
	   xmlXPathFreeObject(Xresult);
	   printf("No Token Node\n");
	   return SDE_Token;
	}
	xmlNodePtr node_Token = Xresult ->nodesetval->nodeTab[0];
	xmlChar *Token = xmlNodeGetContent(node_Token);
	if(strlen((char *)Token)==0)                   //Token节点的文本是空的 即没有令牌
	{
		printf("Token is empty,\n");
		IsUserLogin(Xpath,ThreadNum);     //判断是不是请求登录的报文
		xmlXPathFreeObject(Xresult);
		xmlFree(Token);
		return fail;
	}
	//printf("Token = %s\n",Token);
	// 获取令牌和该TCP链接的令牌进行比较,判断报文中令牌的正确性
	if(	strncmp((char *)Token,TcpThread[ThreadNum].Token,32) == 0 )
	{
		xmlXPathFreeObject(Xresult);
		xmlFree(Token);
		return succeed;
	}
	xmlXPathFreeObject(Xresult);
	xmlFree(Token);
	return fail;

}

int IsUserLogin(xmlXPathContextPtr Xpath,int ThreadNum)
{
	int ret;
	xmlXPathObjectPtr Xresult;       //XPATH对象指针，用来存储查询结果
	//xmlNodeSetPtr nodeset = NULL;
	Xresult = xmlXPathEvalExpression((xmlChar*)"//Type", Xpath); //查找 数据包类型
	if(CheckXmlResult(Xresult) == fail)
	{
		xmlXPathFreeObject(Xresult);
		return fail;
	}
	xmlNodePtr node = Xresult->nodesetval->nodeTab[0];
	xmlChar *text = xmlNodeGetContent(node);
	printf("text = %s\n",text);

	if(	strncmp((char *)text,"REQUEST",7) == 0 )               //如果是请求消息，判断是不是登录操作
	{
		xmlFree(text);
		xmlXPathFreeObject(Xresult);
		Xresult = xmlXPathEvalExpression((xmlChar*)"//Operation", Xpath); //查找 操作命令
		if(CheckXmlResult(Xresult) == fail)
		{
			xmlXPathFreeObject(Xresult);
			return fail;
		}

		node = Xresult->nodesetval->nodeTab[0];
		//text = xmlNodeGetContent(node);                    //xmlChar * xmlGetProp (xmlNodePtr node, const xmlChar * name) //读取节点属性
		text =  xmlGetProp(node,(xmlChar *)"name");
		printf("text = %s\n",text);
		if( strncmp((char *)text,"Login",5) == 0 )                    // 登录操作，生成令牌并回复
		{
			xmlChar *UserName = NULL,*PassWd=NULL;
			xmlFree(text);
			xmlXPathFreeObject(Xresult);
			Xresult = xmlXPathEvalExpression((xmlChar*)"//UserName", Xpath); //查找 UserName
			if(CheckXmlResult(Xresult) == fail)
			{
				xmlXPathFreeObject(Xresult);
				xmlFree(UserName);
				xmlFree(PassWd);
				return fail;
			}
			node = Xresult->nodesetval->nodeTab[0];
			UserName = xmlNodeGetContent(node);

			xmlXPathFreeObject(Xresult);
			Xresult = xmlXPathEvalExpression((xmlChar*)"//Pwd", Xpath); //查找 Pwd
			if(CheckXmlResult(Xresult) == fail)
			{
				xmlXPathFreeObject(Xresult);
				xmlFree(UserName);
				xmlFree(PassWd);
				return fail;
			}
			node = Xresult->nodesetval->nodeTab[0];
			PassWd = xmlNodeGetContent(node);
			ret = IsUserValid((char *)UserName,(char *)PassWd);
			if(ret == succeed)
			{
				char token[32];
				UpdateToken(token);
				memset(TcpThread[ThreadNum].Token,'\0',35);
				memcpy(TcpThread[ThreadNum].Token,token,32);
				LoginLogout_Response(ThreadNum);
				xmlXPathFreeObject(Xresult);

			}
			else
			{
				LoginLogout_Error(ret,ThreadNum);
			}
			xmlFree(UserName);
			xmlFree(PassWd);
		}
		else
		{
			xmlFree(text);
		}
		return succeed;
	}
	else
	{
		xmlFree(text);
	}
	return fail;
}

int LoginLogout_Response(int ThreadNum)
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

		xmlXPathObjectPtr Xresult;       //XPATH对象指针，用来存储查询结果

		Xresult = xmlXPathEvalExpression((xmlChar*)"//Token", Xpath); //查询XPath表达式，得到一个查询结果

		if(CheckXmlResult(Xresult) == fail)
		{

			return fail;
		}
		node = Xresult ->nodesetval->nodeTab[0];
		xmlNodeSetContentLen(node, (const xmlChar *)TcpThread[ThreadNum].Token,  strlen(TcpThread[ThreadNum].Token));
		xmlXPathFreeObject(Xresult);


		Xresult = xmlXPathEvalExpression((xmlChar*)"//Type", Xpath); //查询XPath表达式，得到一个查询结果

		if(CheckXmlResult(Xresult) == fail)
		{
			xmlXPathFreeContext(Xpath);
			xmlFreeDoc(pdoc);
			xmlCleanupParser();
			xmlMemoryDump();
			return fail;
		}
		node = Xresult ->nodesetval->nodeTab[0];
		xmlNodeSetContent(node, (const xmlChar *)"RESPONSE");// xmlNodeSetContent


		if( SwitchFromTo(Xpath) == fail)
		{
			xmlXPathFreeObject(Xresult);
			xmlXPathFreeContext(Xpath);
			xmlFreeDoc(pdoc);
			xmlCleanupParser();
			xmlMemoryDump();
			return fail;
		}
		xmlXPathFreeObject(Xresult);
		Xresult = NULL;

		Xresult = xmlXPathEvalExpression((xmlChar*)"//From", Xpath); //查询XPath表达式，得到一个查询结果
		if(CheckXmlResult(Xresult) == 0)
		{
			xmlXPathFreeContext(Xpath);
			xmlFreeDoc(pdoc);
			xmlCleanupParser();
			xmlMemoryDump();
			return 0;
		}
		TcpThread[ThreadNum].FromNode =  xmlCopyNode(Xresult ->nodesetval->nodeTab[0],1);
		xmlXPathFreeObject(Xresult);
		Xresult = NULL;

		Xresult = xmlXPathEvalExpression((xmlChar*)"//To", Xpath); //查询XPath表达式，得到一个查询结果
		if(CheckXmlResult(Xresult) == 0)
		{
			xmlXPathFreeContext(Xpath);
			xmlFreeDoc(pdoc);
			xmlCleanupParser();
			xmlMemoryDump();
			return 0;
		}
		TcpThread[ThreadNum].ToNode = xmlCopyNode(Xresult ->nodesetval ->nodeTab[0],1);
		xmlXPathFreeObject(Xresult);

		xmlChar *outbuf = NULL;
		int outlen;
		xmlDocDumpFormatMemoryEnc(pdoc, &outbuf, &outlen, "UTF-8", 1);
		//puts((char *)outbuf);

		TCPService_Data_Send(TcpThread[ThreadNum].commt_fd,(char *)outbuf,outlen);
		xmlFree(outbuf);

		xmlXPathFreeContext(Xpath);
		xmlFreeDoc(pdoc);
		xmlCleanupParser();
		xmlMemoryDump();
		return succeed;
}


int CheckXmlResult(xmlXPathObjectPtr Xresult)
{
		if (Xresult == NULL)
		{
		   printf("xmlXPathEvalExpression return NULL");
		   return fail;
		}
		if (xmlXPathNodeSetIsEmpty(Xresult->nodesetval))   //检查查询结果是否为空
		{
		   xmlXPathFreeObject(Xresult);
		   return fail;
		}
		return succeed;
}

int SwitchFromTo(xmlXPathContextPtr Xpath)
{
	xmlXPathObjectPtr ResultFrom;       //XPATH对象指针，用来存储查询结果
	xmlXPathObjectPtr ResultTo;       //XPATH对象指针，用来存储查询结果
	xmlNodePtr FromChild_Node = NULL;
	xmlNodePtr ToChild_Node = NULL;
	xmlNodePtr CopyFrom, CopyTo;

	ResultFrom = xmlXPathEvalExpression((xmlChar*)"//From", Xpath); //查询XPath表达式，得到一个查询结果
	if(CheckXmlResult(ResultFrom) == fail)
	{
		return fail;
	}
	ResultTo = xmlXPathEvalExpression((xmlChar*)"//To", Xpath); //查询XPath表达式，得到一个查询结果
	if(CheckXmlResult(ResultTo) == fail)
	{
		return fail;
	}
	FromChild_Node = ResultFrom ->nodesetval->nodeTab[0]->children;
	if(FromChild_Node == NULL)
	{
		return fail;
	}
	ToChild_Node = ResultTo ->nodesetval ->nodeTab[0]->children;
	if(ToChild_Node == NULL)
	{
		return fail;
	}

	CopyFrom =  xmlCopyNode(FromChild_Node,1);
	CopyTo = xmlCopyNode(ToChild_Node,1);

	xmlUnlinkNode(FromChild_Node);
	xmlUnlinkNode(ToChild_Node);

	xmlAddChild(ResultFrom ->nodesetval->nodeTab[0],CopyTo);
	xmlAddChild(ResultTo ->nodesetval ->nodeTab[0],CopyFrom);

	xmlFreeNode(FromChild_Node);
	xmlFreeNode(ToChild_Node);
	xmlXPathFreeObject(ResultFrom);
	ResultFrom = NULL;
	xmlXPathFreeObject(ResultTo);
	ResultTo = NULL;

	return succeed;
}

int GetOperateType(char *type,xmlXPathContextPtr Xpath)
{
	xmlXPathObjectPtr Xresult =NULL;
	xmlNodePtr node=NULL;
	xmlChar *text = NULL;
	Xresult = xmlXPathEvalExpression((xmlChar*)"//Type", Xpath);
	if(CheckXmlResult(Xresult) == fail)
		return fail;
	node = Xresult ->nodesetval->nodeTab[0];
	text = xmlNodeGetContent(node);
	xmlXPathFreeObject(Xresult);
	Xresult = NULL;
	if(text == NULL)
		return fail;
	memcpy(type,(char *)text,strlen((char *)text));

	xmlFree(text);
	text = NULL;

	return succeed;
}



