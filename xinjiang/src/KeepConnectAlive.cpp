/*
 * KeepConnectAlive.cpp
 *
 *  Created on: 2016年8月25日
 *      Author: root
 */

#include "include.h"

void * KeepConnectAlive(void *arg)
{
	int i;
	xmlChar *OutBuf;
	int outlen;
	xmlDocPtr doc = NULL;
	xmlNodePtr Message_Node = NULL;
	xmlNodePtr Body_Node = NULL;
	xmlNodePtr Operate_Node = NULL;
	xmlNodePtr FromNode;
	xmlNodePtr ToNode;
	printf("Creat thread KeepConnectAlive success,%s,%s,%d\n",__FILE__,__FUNCTION__,__LINE__);
	sleep(1);

	while(1)
	{
		for(i = 0;i < MaxConnect; i++)
		{
			if( ( TcpThread[i].IsConnect ) && (TcpThread[i].ToNode !=NULL) && (TcpThread[i].FromNode !=NULL) )
			{
				doc = xmlNewDoc(BAD_CAST "1.0");
				Message_Node = xmlNewNode(NULL,BAD_CAST "Message");
				xmlDocSetRootElement(doc, Message_Node);
				xmlNewChild(Message_Node,NULL,BAD_CAST "Version",BAD_CAST "1.0");
				xmlNewChild(Message_Node,NULL,BAD_CAST "Token",BAD_CAST TcpThread[i].Token);
				//From
				FromNode = xmlCopyNode(TcpThread[i].FromNode,1);
				xmlAddChild(Message_Node,FromNode);
				//To
				ToNode = xmlCopyNode(TcpThread[i].ToNode,1);
				xmlAddChild(Message_Node,ToNode);

				xmlNewChild(Message_Node,NULL,BAD_CAST "Type",BAD_CAST "PUSH");
				xmlNewChild(Message_Node,NULL,BAD_CAST "Seq",BAD_CAST "1");
				Body_Node = xmlNewNode(NULL,BAD_CAST"Body");
				xmlAddChild(Message_Node,Body_Node);

				Operate_Node = xmlNewNode(NULL,BAD_CAST"Operation");
				xmlSetProp(Operate_Node,BAD_CAST"order",BAD_CAST"1");
				xmlSetProp(Operate_Node,BAD_CAST"name",BAD_CAST"notify");
				xmlNewChild(Operate_Node,NULL,BAD_CAST "SDO_HeartBeat",NULL);
				xmlAddChild(Body_Node,Operate_Node);

				xmlDocDumpFormatMemoryEnc(doc, &OutBuf, &outlen, "UTF-8", 1);
				xmlFreeDoc(doc);
				TCPService_Data_Send(TcpThread[i].commt_fd,(char *)OutBuf,outlen);
				xmlFree(OutBuf);
			}
		}
		sleep(60);
	}
	return (void *)0;
}


