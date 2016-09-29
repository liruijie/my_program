#include <stdio.h>

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <libxml/parser.h>
#include <libxml/xmlreader.h>
#include <libxml/tree.h>
#include <libxml/xpath.h>
#include <libxml/xmlmemory.h>
#include <libxml/xmlerror.h>

int SwitchFromTo(xmlXPathContextPtr Xpath);
int CheckXmlResult(xmlXPathObjectPtr Xresult);
const char xmlbuf[] = "\
<?xml version=\"1.0\" encoding=\"UTF-8\"?>  	\
<Message>  												\
	<Version>1.0</Version>							\
	<Token></Token>									\
	<From>Web</From>									\
	<To>													\
		<Address>										\
			<Sys>Sys</Sys>								\
			<SubSys/>									\
			<Instance/>									\
		</Address>										\
	</To>													\
	<Type>REQUEST</Type>								\
	<Seq></Seq>											\
	<Body>												\
		<Operation order=\"1\" name=\"Login\">	\
			<SDO_User>									\
				<UserName>admin</UserName>			\
				<Pwd>1</Pwd>							\
			</SDO_User >								\
		</Operation>									\
	</Body>												\
</Message>";





void test();
int main(int argc, char **argv)
{

	xmlDocPtr pdoc;
	xmlXPathObjectPtr ResultFrom;       //XPATH对象指针，用来存储查询结果
	xmlXPathObjectPtr ResultTo;       //XPATH对象指针，用来存储查询结果
	xmlKeepBlanksDefault(0);
	//xmlNodePtr root_node = NULL, node = NULL, curr = NULL;
	//xmlDocPtr pdoc = xmlParseMemory(XmlBuff11, (int )strlen(XmlBuff));

	pdoc = xmlReadMemory(xmlbuf,(int )strlen(xmlbuf), NULL, "utf-8",XML_PARSE_NOERROR);
	if(pdoc == NULL)
	{

		return 0;
	}

	xmlXPathContextPtr XPath;    //XPATH上下文指针
	XPath = xmlXPathNewContext(pdoc);     //创建一个XPath上下文指针
	xmlNodePtr CopyFrom = NULL;
	xmlNodePtr CopyTo = NULL;
	//xmlXPathObjectPtr Xresult;       //XPATH对象指针，用来存储查询结果
	if( SwitchFromTo(XPath) == 0)
	{
		return 0;
	}
	ResultFrom = xmlXPathEvalExpression((xmlChar*)"//From", XPath); //查询XPath表达式，得到一个查询结果
	if(CheckXmlResult(ResultFrom) == 0)
	{
		return 0;
	}
	ResultTo = xmlXPathEvalExpression((xmlChar*)"//To", XPath); //查询XPath表达式，得到一个查询结果
	if(CheckXmlResult(ResultTo) == 0)
	{
		return 0;
	}

	xmlChar *outbuf = NULL;
	int outlen;
	xmlDocDumpFormatMemoryEnc(pdoc, &outbuf, &outlen, "UTF-8", 1);
	puts((char *)outbuf);


	while(1)
	{
		usleep(500);
		test();
	}

#if 0
  xmlDocPtr doc = NULL;
  xmlNodePtr root_node = NULL, node = NULL, node1 = NULL;
  xmlChar *outbuf;
  int outlen;
  doc = xmlNewDoc(BAD_CAST "1.0");
  root_node = xmlNewNode(NULL,BAD_CAST"root");
  xmlDocSetRootElement(doc, root_node);

  xmlNewChild(root_node, NULL, BAD_CAST "node1",BAD_CAST "content of node1");    //在root节点下建立一个新的节点node1

  node=xmlNewChild(root_node, NULL, BAD_CAST "node3",BAD_CAST"node has attributes");//在root节点下建立一个新的节点node3
  xmlNewProp(node, BAD_CAST "attribute1", BAD_CAST "确定");                       //添加属性
  xmlNewProp(node, BAD_CAST "attribute2", BAD_CAST "取消");                       //添加属性

  node = xmlNewNode(NULL, BAD_CAST "node4");                          //建立一个节点node4
  node1 = xmlNewText(BAD_CAST"other way to create content");
  xmlAddChild(node, node1);
  xmlAddChild(root_node, node);                                         //把节点node4添加到root节点下

  node = xmlNewNode(NULL, BAD_CAST "node5");                          //建立一个节点node5
  xmlNewChild(node, NULL, BAD_CAST "node6",BAD_CAST"node6666666666666"); //在node5节点下建立一个新的节点node6
  node1 = xmlNewNode(NULL, BAD_CAST "node8");
  xmlAddChild(root_node, node1);
  node1 = xmlNewNode(NULL, BAD_CAST "node9");
xmlAddChild(root_node, node1);
  xmlAddChild(root_node, node);                                         //把节点node5添加到root节点下

  xmlDocDumpFormatMemoryEnc(doc, &outbuf, &outlen, "UTF-8", 1);
 // xmlWriteMemory();

  printf("%d\n",outlen);
  puts((char *)outbuf);
  //xmlSaveFormatFileEnc(argc > 1 ? argv[1] : "-", doc, "UTF-8", 1);
  xmlFreeDoc(doc);
  xmlCleanupParser();
  xmlMemoryDump();
#endif
  return(0);

}

void test()
{
	  xmlDocPtr doc = NULL;
	  xmlNodePtr root_node = NULL, node = NULL, node1 = NULL;
	  xmlNodePtr parentnode=NULL,CopyNode = NULL;
	  xmlChar *outbuf;
	 // xmlChar *testbuf;
	  int outlen = 0;
	  char buf[10];
	 // xmlNodePtr node_t[6];
	  doc = xmlNewDoc(BAD_CAST "1.0");
	  root_node = xmlNewNode(NULL,BAD_CAST"root");
	  parentnode = xmlNewNode(NULL,BAD_CAST"parent");
	  xmlDocSetRootElement(doc, root_node);
	  int i,j;
	  for(i=1;i<=5;i++)
	  {
		  sprintf(buf,"node%d",i);
		  node = xmlNewNode(NULL, BAD_CAST buf);
		  for(j=1;j<=5;j++)
		  {
			  sprintf(buf,"child%d",j);
			  node1 = xmlNewNode(NULL, BAD_CAST buf);
			  sprintf(buf,"value = %d",j);
			  xmlNewProp(node1, BAD_CAST "attribute1", BAD_CAST buf);
			  xmlAddChild(node, node1);
		  }
		  xmlAddChild(parentnode, node);
	  }
	 // CopyNode = xmlNewNode(NULL, BAD_CAST "");
	 CopyNode =  xmlCopyNode(parentnode,1);
	  xmlAddChild(root_node, parentnode);

	  xmlDocDumpFormatMemoryEnc(doc, &outbuf, &outlen, "UTF-8", 1);
   // xmlWriteMemory();

	printf("%d\n",outlen);
	puts((char *)outbuf);
	// xmlFreeNode(node1);
	 // node1 = NULL;
	 /// xmlFreeNode(node);
	 // node = NULL;
	 // xmlFreeNode(parentnode);
	 // parentnode = NULL;
	  //xmlFreeNode(root_node);



	 // xmlFree(testbuf);
	 xmlFreeDoc(doc);
	 xmlFree(outbuf);


	 doc = xmlNewDoc(BAD_CAST "1.0");
	  root_node = xmlNewNode(NULL,BAD_CAST"root");
	  parentnode = xmlNewNode(NULL,BAD_CAST"parent");
	  xmlDocSetRootElement(doc, root_node);
	  xmlAddChild(root_node, CopyNode);
	  xmlDocDumpFormatMemoryEnc(doc, &outbuf, &outlen, "UTF-8", 1);

	  printf("%d\n",outlen);
	  puts((char *)outbuf);
	  xmlFreeDoc(doc);
	 xmlFree(outbuf);

	  xmlCleanupParser();
	  xmlMemoryDump();
}
int SwitchFromTo(xmlXPathContextPtr Xpath)
{
	xmlXPathObjectPtr ResultFrom;       //XPATH对象指针，用来存储查询结果
	xmlXPathObjectPtr ResultTo;       //XPATH对象指针，用来存储查询结果
	xmlNodePtr FromChild_Node = NULL;
	xmlNodePtr ToChild_Node = NULL;
	xmlNodePtr Copy1=NULL, Copy2 =NULL;
	//xmlNodePtr Copy3=NULL, Copy4 =NULL;

	ResultFrom = xmlXPathEvalExpression((xmlChar*)"//From", Xpath); //查询XPath表达式，得到一个查询结果
	if(CheckXmlResult(ResultFrom) == 0)
	{
		return 0;
	}
	ResultTo = xmlXPathEvalExpression((xmlChar*)"//To", Xpath); //查询XPath表达式，得到一个查询结果
	if(CheckXmlResult(ResultTo) == 0)
	{
		return 0;
	}
	FromChild_Node = ResultFrom ->nodesetval->nodeTab[0]->children;
	if(FromChild_Node == NULL)
	{
		return 0;
	}
	ToChild_Node = ResultTo ->nodesetval ->nodeTab[0]->children;
	if(ToChild_Node == NULL)
	{
		return 0;
	}

	//CopyFrom =  xmlCopyNode(ResultFrom ->nodesetval->nodeTab[0],1);
	//CopyTo = xmlCopyNode(ResultTo ->nodesetval ->nodeTab[0],1);
	Copy1 =  xmlCopyNode(ResultFrom ->nodesetval->nodeTab[0],1);
	Copy2 = xmlCopyNode(ResultTo ->nodesetval ->nodeTab[0],1);


	xmlUnlinkNode(FromChild_Node);
	xmlUnlinkNode(ToChild_Node);

	xmlAddChild(ResultFrom ->nodesetval->nodeTab[0],Copy2);
	xmlAddChild(ResultTo ->nodesetval ->nodeTab[0],Copy1);

	xmlFreeNode(FromChild_Node);
	xmlFreeNode(ToChild_Node);
	xmlXPathFreeObject(ResultFrom);
	ResultFrom = NULL;
	xmlXPathFreeObject(ResultTo);
	ResultTo = NULL;

	return 1;
}
int CheckXmlResult(xmlXPathObjectPtr Xresult)
{
		if (Xresult == NULL)
		{
		   printf("xmlXPathEvalExpression return NULL");
		   return 0;
		}
		if (xmlXPathNodeSetIsEmpty(Xresult->nodesetval))   //检查查询结果是否为空
		{
		   xmlXPathFreeObject(Xresult);
		   return 0;
		}
		return 1;
}
