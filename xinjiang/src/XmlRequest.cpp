/*
 * XmlRequest.cpp
 *
 * Created on: 2016年7月14日
 * Author: root
 */
#include "include.h"
int RequestOperate(xmlDocPtr pdoc,xmlXPathContextPtr Xpath,int ThreadNum)
{
		xmlXPathObjectPtr Xresult;

		xmlNodePtr node;
		xmlChar *OperationProp_Name;
		int node_count;
		int ret;
		xmlChar *temp;
		Xresult = xmlXPathEvalExpression((xmlChar*)"//Operation", Xpath);
		if(CheckXmlResult(Xresult) == fail)
			return fail;
		for(node_count = 0;node_count < Xresult->nodesetval->nodeNr;node_count++)             //循环Operate节点
		{
			node = Xresult->nodesetval->nodeTab[node_count];
			OperationProp_Name = xmlGetProp(node, (xmlChar *)"name");
			if(strncmp("Get",(char *)OperationProp_Name,3) == 0)
			{
				ret = Operation_Get(node,ThreadNum);
			}
			else if(strncmp("Set",(char *)OperationProp_Name,3) == 0)
			{
				ret = Operation_Set(node,ThreadNum);
			}
			else if(strncmp("Logout",(char *)OperationProp_Name,6) == 0)
			{

				ret = RequestLogout(node,ThreadNum);
				if(ret == succeed)
				{
					LoginLogout_Response(ThreadNum);
				}
				else
				{
					LoginLogout_Error(ret,ThreadNum);
				}
				//xmlFree(OperationProp_Name);
				//OperationProp_Name = NULL;

				//xmlXPathFreeObject(Xresult);
				//Xresult = NULL;
				xmlFree(OperationProp_Name);
				return succeed;
			}

			if(ret != succeed)       //解析报文过程中出错
			{
				memset(&TcpThread[ThreadNum].xmlErrorInfo,'\0',sizeof(struct xmlResponseError));
				strcpy(TcpThread[ThreadNum].xmlErrorInfo.ObjectnName,(char *)Xresult->nodesetval->nodeTab[node_count]->children->children->name); //operation节点->TSCCmdt节点->要操作的对象节点 ->name
				strcpy(TcpThread[ThreadNum].xmlErrorInfo.OperateProp_Name,(char *)OperationProp_Name);
				temp = xmlGetProp(node, (xmlChar *)"order");
				strcpy(TcpThread[ThreadNum].xmlErrorInfo.OperateProp_Order,(char *)temp);
				xmlFree(temp);
				break;
			}
		}
		xmlFree(OperationProp_Name);
		xmlXPathFreeObject(Xresult);
		Xresult = NULL;

		Xresult = xmlXPathEvalExpression((xmlChar*)"//Type", Xpath);
		if(CheckXmlResult(Xresult) == fail)
			ret =  SDE_MsgType;
		node = Xresult ->nodesetval->nodeTab[0];
		if(ret == succeed)
		{
			xmlNodeSetContent(node, (const xmlChar *)"RESPONSE");
		}
		else           //根据return的错误类型回复
		{
			printf("happen error\n");
			xmlNodeSetContent(node, (const xmlChar *)"ERROR");
			printf("set error\n");
			request_error(ret,Xpath,TcpThread[ThreadNum].xmlErrorInfo);
			printf("make error\n");
		}
		xmlXPathFreeObject(Xresult);
		Xresult = NULL;
		if( SwitchFromTo(Xpath) == fail)
		{
			return fail;
		}
		xmlChar *outbuf;
		int outlen;
		xmlKeepBlanksDefault(0);
		xmlDocDumpFormatMemoryEnc(pdoc, &outbuf, &outlen, "UTF-8", 1);
		puts((char *)outbuf);

		TCPService_Data_Send(TcpThread[ThreadNum].commt_fd,(char *)outbuf,outlen);
		xmlFree(outbuf);
		return succeed;
}

int Operation_Get(xmlNodePtr OperationNode,int ThreadNum)
{
		xmlNodePtr Object;
		char *ObjectName;
		Object = OperationNode ->children ->children;          //operation节点->TSCCmdt节点->要操作的对象节点
		while(1)                         //  找到object节点
		{
			if(Object == NULL)
			{
				return SDE_Failure;
			}
			else
			{
				if(strncmp("ObjName",(char *)Object->name,6) == 0)
				{
					ObjectName = (char *)xmlNodeGetContent(Object);
					break;
				}
				else
				{
					Object = Object ->next;
				}
			}
		}
		//系统参数
		if( strncmp(ObjectName,(char *)&SysInfo,sizeof(SysInfo)) == 0)
		{
			xmlFree(ObjectName);
			return Get_SysInfo(OperationNode,ThreadNum);
		}
		//区域参数
		else if( strncmp(ObjectName,(char *)&RegionParam,sizeof(RegionParam)) == 0)
		{
			return Get_RegionParam(OperationNode,ThreadNum);
		}
		//子区参数
		else if( strncmp(ObjectName,(char *)&SubRegionParam,sizeof(SubRegionParam)) == 0)
		{
			return Get_SubRegionParam(OperationNode,ThreadNum);
		}
		//路口参数
		else if( strncmp(ObjectName,(char *)&CrossParam,sizeof(CrossParam)) == 0)
		{
			return Get_CrossParam(OperationNode,ThreadNum);
		}
		//信号机参数
		else if( strncmp(ObjectName,(char *)&SignalControler,sizeof(SignalControler)) == 0)
		{
			return Get_SignalControler(OperationNode,ThreadNum);
		}
		//信号灯组参数
		else if( strncmp(ObjectName,(char *)&LampGroup,sizeof(LampGroup)) == 0)
		{
			return Get_LampGroup(OperationNode,ThreadNum);
		}
		//检测器参数
		else if( strncmp(ObjectName,(char *)&DetParam,sizeof(DetParam)) == 0)
		{
			return Get_DetParam(OperationNode,ThreadNum);
		}
		//车道参数
		else if( strncmp(ObjectName,(char *)&LaneParam,sizeof(LaneParam)) == 0)
		{
			return Get_LaneParam(OperationNode,ThreadNum);
		}
		//相位参数
		else if( strncmp(ObjectName,(char *)&PhaseParam,sizeof(PhaseParam)) == 0)
		{
			return Get_PhaseParam(OperationNode,ThreadNum);
		}
		//阶段参数
		else if( strncmp(ObjectName,(char *)&StageParam,sizeof(StageParam)) == 0)
		{
			return Get_StageParam(OperationNode,ThreadNum);
		}
		//配时方案参数
		else if( strncmp(ObjectName,(char *)&PlanParam,sizeof(PlanParam)) == 0)
		{
			return Get_PlanParam(OperationNode,ThreadNum);
		}
		xmlFree(ObjectName);
		return SDE_Failure;
}


/*设置操作目前只支持设置阶段和方案*/
int Operation_Set(xmlNodePtr OperationNode,int ThreadNum)
{
		int ret = 0;
		xmlNodePtr Object;
		char *ObjectName;
		Object = OperationNode ->children ->children;          //operation节点->TSCCmdt节点->要操作的对象节点
		if(Object == NULL)
		{
			return SDE_Failure;
		}

		ObjectName = (char *)Object ->name;
		if( strncmp(ObjectName,(char *)&SysInfo,sizeof(SysInfo)) == 0)
		{
			return SDE_NotAllow;
		}
		/*	else if( strncmp(ObjectName,(char *)&RegionParam,sizeof(RegionParam)) == 0)
		{
			return SDE_NotAllow;
		}
		else if( strncmp(ObjectName,(char *)&SubRegionParam,sizeof(SubRegionParam)) == 0)
		{
			return SDE_NotAllow;
		}
		else if( strncmp(ObjectName,(char *)&CrossParam,sizeof(CrossParam)) == 0)
		{
			return SDE_NotAllow;
		}
		else if( strncmp(ObjectName,(char *)&SignalControler,sizeof(SignalControler)) == 0)
		{
			return SDE_NotAllow;
		}
		else if( strncmp(ObjectName,(char *)&LampGroup,sizeof(LampGroup)) == 0)
		{
			return SDE_NotAllow;
		}
		else if( strncmp(ObjectName,(char *)&DetParam,sizeof(DetParam)) == 0)
		{
			return SDE_NotAllow;
		}
		else if( strncmp(ObjectName,(char *)&LaneParam,sizeof(LaneParam)) == 0)
		{
			return SDE_NotAllow;
		}
		else if( strncmp(ObjectName,(char *)&PhaseParam,sizeof(PhaseParam)) == 0)
		{
			return SDE_NotAllow;
		}
		else if( strncmp(ObjectName,(char *)&SysState,sizeof(SysState)) == 0)
		{
			return SDE_NotAllow;
		}
		else if( strncmp(ObjectName,(char *)&RegionState,sizeof(RegionState)) == 0)
		{
			return SDE_NotAllow;
		}
		else if( strncmp(ObjectName,(char *)&CrossState,sizeof(CrossState)) == 0)
		{
			return SDE_NotAllow;
		}
		else if( strncmp(ObjectName,(char *)&SignalControlerError,sizeof(SignalControlerError)) == 0)
		{
			return SDE_NotAllow;
		}
		else if( strncmp(ObjectName,(char *)&CrossCycle,sizeof(CrossCycle)) == 0)
		{
			return SDE_NotAllow;
		}
		else if( strncmp(ObjectName,(char *)&CrossPhaseLampStatus,sizeof(CrossPhaseLampStatus)) == 0)
		{
			return SDE_NotAllow;
		}
		else if( strncmp(ObjectName,(char *)&CrossTrafficData,sizeof(CrossTrafficData)) == 0)
		{
			return SDE_NotAllow;
		}
		else if( strncmp(ObjectName,(char *)&LockFlowDirection,sizeof(LockFlowDirection)) == 0)
		{
			return SDE_NotAllow;
		}
		else if( strncmp(ObjectName,(char *)&UnLockFlowDirection,sizeof(UnLockFlowDirection)) == 0)
		{
			return SDE_NotAllow;
		}*/
		//设置阶段
		else if( strncmp(ObjectName,(char *)&StageParam,sizeof(StageParam)) == 0)
		{
			ret = Set_StageParam(OperationNode,ThreadNum);
			if(ret != succeed)
				return ret;
		}
		//设置方案
		else if( strncmp(ObjectName,(char *)&PlanParam,sizeof(PlanParam)) == 0)
		{
			ret = Set_PlanParam(OperationNode,ThreadNum);
			if(ret != succeed)
				return ret;
		}
		//设置实时上报内容
		else if( strncmp(ObjectName,(char *)&CrossReportCtrl,sizeof(CrossReportCtrl)) == 0)
		{
			ret = Set_CrossReportCtrl(OperationNode,ThreadNum);
		}
		//锁定交通流向
		else if(strncmp(ObjectName,(char *)&LockFlowDirection,sizeof(LockFlowDirection)) == 0)
		{
			ret = Set_LockFlowDirection(OperationNode,ThreadNum);
		}
		//解锁交通流向
		else if(strncmp(ObjectName,(char *)&UnLockFlowDirection,sizeof(UnLockFlowDirection)) == 0)
		{
			ret = Set_UnLockFlowDirection(OperationNode,ThreadNum);
		}
		//指定方案
		else if( strncmp(ObjectName,(char *)&CrossPlan,sizeof(CrossPlan)) == 0)
		{
			ret = Set_CrossPlan(OperationNode,ThreadNum);
		}
		//指定控制方式
		else if( strncmp(ObjectName,(char *)&CrossControlMode,sizeof(CrossControlMode)) == 0)
		{
			ret = Set_CrossControlMode(OperationNode,ThreadNum);
		}
		//指定阶段
		else if( strncmp(ObjectName,(char *)&CrossStage,sizeof(CrossStage)) == 0)
		{
			ret =Set_CrossStage(OperationNode,ThreadNum);
		}
		else
		{
			return SDE_NotAllow;
		}
		return succeed;
}

int RequestLogout(xmlNodePtr OperationNode,int ThreadNum)
{

	xmlChar  *UserName,*PassWd;
	xmlNodePtr Node;
	int ret;
	Node =  OperationNode ->children ->children;
	if(Node == NULL)
		return SDE_UserName;
	UserName = xmlNodeGetContent(Node);
	Node = Node->next;
	if(Node == NULL)
	{
		xmlFree(UserName);
		return SDE_Pwd;
	}
	PassWd = xmlNodeGetContent(Node);
	ret = IsUserValid((char *)UserName,(char *)PassWd);
	xmlFree(UserName);
	xmlFree(PassWd);
	return  ret;
}

