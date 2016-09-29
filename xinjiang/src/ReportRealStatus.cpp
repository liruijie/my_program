/*
 * ReportRealStatus.cpp
 *
 *  Created on: 2016年7月26日
 *      Author: root
 */
#include "include.h"

extern StatelessConnectionPool *pConnPool;
extern oracle::occi::Environment *OraEnviroment;

extern pthread_mutex_t UpdateRealData;
Connection *Conn;
Statement *Stmt;

void *ReportRealStatus(void *arg)
{
	int ThreadNum;
	int i,j;       //i:连接信息中 上报信息数组下标    j:实时状态结构体数组下标
	int temp;
	int outlen;
	char sqlbuf[200];
	char temp_buf[10];
	char SignalID[10];
	ResultSet *Result;


	//用来标记各个Operation节点下的Object节点中是否有子节点
	int SignalState_flag = 0,
		 ControlMode_flag = 0,
		 Plan_flag = 0 ,
		 Cycle_flag=0,
		 Stage_flag = 0,
		 Phase_flag = 0,
		 Traffic_flag = 0,
		 Fault_flag = 0;
	int order_value;
	char orderbuf[3];
	xmlChar *OutBuf;
	xmlDocPtr doc = NULL;
	xmlNodePtr Message_Node = NULL;
	xmlNodePtr Body_Node = NULL;
	xmlNodePtr FromNode;
	xmlNodePtr ToNode;


	xmlNodePtr OperateNode_SysState = NULL;
	xmlNodePtr ObjectNode_SysState = NULL;

	xmlNodePtr OperateNode_RegionState = NULL;
	xmlNodePtr ObjectNode_RegionStatus = NULL;

	xmlNodePtr OperateNode_SignalState = NULL;
	xmlNodePtr ObjectNode_SignalState = NULL;

	xmlNodePtr OperateNode_ControlMode = NULL;
	xmlNodePtr ObjectNode_ControlMode = NULL;

	xmlNodePtr OperateNode_Plan = NULL;
	xmlNodePtr ObjectNode_Plan = NULL;

	xmlNodePtr OperateNode_Cycle = NULL;
	xmlNodePtr ObjectNode_Cycle = NULL;

	xmlNodePtr OperateNode_Stage = NULL;
	xmlNodePtr ObjectNode_Stage = NULL;

	xmlNodePtr OperateNode_Phase = NULL;
	xmlNodePtr ObjectNode_Phase = NULL;

	xmlNodePtr OperateNode_Traffic = NULL;
	xmlNodePtr ObjectNode_Traffic = NULL;

	xmlNodePtr OperateNode_Fault = NULL;
	xmlNodePtr ObjectNode_Fault = NULL;

	xmlNodePtr SysState_Node;
	xmlNodePtr RegionState_Node;
	xmlNodePtr CrossState_Node;
	xmlNodePtr SignalError_Node;
	xmlNodePtr ControlMode_Node;
	xmlNodePtr CrossPlan_Node;
	xmlNodePtr CrossCycle_Node;
	xmlNodePtr CrossStage_Node;
	xmlNodePtr CrossPhaseLampStatus_Node;
	xmlNodePtr PhaseLampStatusList_Node;
	xmlNodePtr PhaseLampStatus_Node;
	xmlNodePtr CrossTrafficData_Node;
	xmlNodePtr DataList_Node;
	xmlNodePtr Data_Node;


	printf("Creat thread ReportRealStatus success,%s,%s,%d\n",__FILE__,__FUNCTION__,__LINE__);
	sleep(5);
	while(1)
	{
		pthread_mutex_lock(&UpdateRealData);
		for(ThreadNum = 0;ThreadNum <MaxConnect;ThreadNum++)
		{
			if((TcpThread[ThreadNum].IsConnect == true) &&  (TcpThread[ThreadNum].ToNode !=NULL) && (TcpThread[ThreadNum].FromNode !=NULL) )     //循环所有已经建立的连接
			{
				SignalState_flag = false;
				ControlMode_flag = false;
				Plan_flag = false ;
				Cycle_flag= false;
				Stage_flag = false;
				Phase_flag = false;
				Traffic_flag = false;
				Fault_flag = false;
				doc = xmlNewDoc(BAD_CAST "1.0");
				Message_Node = xmlNewNode(NULL,BAD_CAST "Message");
				xmlDocSetRootElement(doc, Message_Node);
				xmlNewChild(Message_Node,NULL,BAD_CAST "Version",BAD_CAST "1.0");
				xmlNewChild(Message_Node,NULL,BAD_CAST "Token",BAD_CAST TcpThread[ThreadNum].Token);
				//From
				FromNode = xmlCopyNode(TcpThread[ThreadNum].FromNode,1);
				xmlAddChild(Message_Node,FromNode);
				//To
				ToNode = xmlCopyNode(TcpThread[ThreadNum].ToNode,1);
				xmlAddChild(Message_Node,ToNode);

				xmlNewChild(Message_Node,NULL,BAD_CAST "Type",BAD_CAST "PUSH");
				xmlNewChild(Message_Node,NULL,BAD_CAST "Seq",BAD_CAST "1");
				Body_Node = xmlNewNode(NULL,BAD_CAST"Body");
				xmlAddChild(Message_Node,Body_Node);

				//路口状态
				OperateNode_SignalState = xmlNewNode(NULL,BAD_CAST"Operation");
				//xmlSetProp(OperateNode_SignalState,BAD_CAST"order",BAD_CAST"1");
				xmlSetProp(OperateNode_SignalState,BAD_CAST"name",BAD_CAST"Notify");
				ObjectNode_SignalState =  xmlNewNode(NULL,BAD_CAST"Object");
				xmlAddChild(OperateNode_SignalState,ObjectNode_SignalState);
				xmlAddChild(Body_Node,OperateNode_SignalState);

				//控制方式
				OperateNode_ControlMode = xmlNewNode(NULL,BAD_CAST"Operation");
				//xmlSetProp(OperateNode_ControlMode,BAD_CAST"order",BAD_CAST"2");
				xmlSetProp(OperateNode_ControlMode,BAD_CAST"name",BAD_CAST"Notify");
				ObjectNode_ControlMode =  xmlNewNode(NULL,BAD_CAST"Object");
				xmlAddChild(OperateNode_ControlMode,ObjectNode_ControlMode);
				xmlAddChild(Body_Node,OperateNode_ControlMode);

				//控制方案
				OperateNode_Plan = xmlNewNode(NULL,BAD_CAST"Operation");
				//xmlSetProp(OperateNode_Plan,BAD_CAST"order",BAD_CAST"3");
				xmlSetProp(OperateNode_Plan,BAD_CAST"name",BAD_CAST"Notify");
				ObjectNode_Plan =  xmlNewNode(NULL,BAD_CAST"Object");
				xmlAddChild(OperateNode_Plan,ObjectNode_Plan);
				xmlAddChild(Body_Node,OperateNode_Plan);

				//周期
				OperateNode_Cycle = xmlNewNode(NULL,BAD_CAST"Operation");
				//xmlSetProp(OperateNode_Cycle,BAD_CAST"order",BAD_CAST"4");
				xmlSetProp(OperateNode_Cycle,BAD_CAST"name",BAD_CAST"Notify");
				ObjectNode_Cycle =  xmlNewNode(NULL,BAD_CAST"Object");
				xmlAddChild(OperateNode_Cycle,ObjectNode_Cycle);
				xmlAddChild(Body_Node,OperateNode_Cycle);

				//阶段
				OperateNode_Stage= xmlNewNode(NULL,BAD_CAST"Operation");
				//xmlSetProp(OperateNode_Stage,BAD_CAST"order",BAD_CAST"5");
				xmlSetProp(OperateNode_Stage,BAD_CAST"name",BAD_CAST"Notify");
				ObjectNode_Stage =  xmlNewNode(NULL,BAD_CAST"Object");
				xmlAddChild(OperateNode_Stage,ObjectNode_Stage);
				xmlAddChild(Body_Node,OperateNode_Stage);

				//相位灯态
				OperateNode_Phase = xmlNewNode(NULL,BAD_CAST"Operation");
				//xmlSetProp(OperateNode_Phase,BAD_CAST"order",BAD_CAST"6");
				xmlSetProp(OperateNode_Phase,BAD_CAST"name",BAD_CAST"Notify");
				ObjectNode_Phase =  xmlNewNode(NULL,BAD_CAST"Object");
				xmlAddChild(OperateNode_Phase,ObjectNode_Phase);
				xmlAddChild(Body_Node,OperateNode_Phase);

				//交通流数据
				OperateNode_Traffic = xmlNewNode(NULL,BAD_CAST"Operation");
				//xmlSetProp(OperateNode_Traffic,BAD_CAST"order",BAD_CAST"7");
				xmlSetProp(OperateNode_Traffic,BAD_CAST"name",BAD_CAST"Notify");
				ObjectNode_Traffic =  xmlNewNode(NULL,BAD_CAST"Object");
				xmlAddChild(OperateNode_Traffic,ObjectNode_Traffic);
				xmlAddChild(Body_Node,OperateNode_Traffic);

				//信号机故障
				OperateNode_Fault = xmlNewNode(NULL,BAD_CAST"Operation");
				//xmlSetProp(OperateNode_Fault,BAD_CAST"order",BAD_CAST"8");
				xmlSetProp(OperateNode_Fault,BAD_CAST"name",BAD_CAST"Notify");
				ObjectNode_Fault =  xmlNewNode(NULL,BAD_CAST"Object");
				xmlAddChild(OperateNode_Fault,ObjectNode_Fault);
				xmlAddChild(Body_Node,OperateNode_Fault);

				for(j = 0;j< SignalMaxNum;j++)     //把所有信号机的实时数据加入到xml中
				{
					if(SignalRealData[j].CrossID == -1)   //要查询的实时数据结构体中的路口ID=-1，说明已经查询完所有存在的信号机了
					{
						break;
					}
					sprintf(SignalID,"%d",SignalRealData[j].CrossID);
					//路口状态
					if( SignalRealData[j].IsStatusUpdate )
					{
						SignalState_flag = true;
						CrossState_Node = xmlNewNode(NULL, BAD_CAST "CrossState");
						xmlNewChild(CrossState_Node,NULL,BAD_CAST "CrossID",BAD_CAST SignalID);
						if( SignalRealData[j].SignalState != Online )   //信号机不在线  只上报状态
						{
							xmlNewChild(CrossState_Node,NULL,BAD_CAST "Value",BAD_CAST"Offline");
							continue;
						}
						else
						{
							xmlNewChild(CrossState_Node,NULL,BAD_CAST "Value",BAD_CAST"Online");
						}
						xmlAddChild(ObjectNode_SignalState,CrossState_Node);
					}
					//信号机故障
					if(SignalRealData[j].IsSignalErrorUpdate == true )
					{
						Fault_flag = true;
						SignalError_Node = xmlNewNode(NULL,BAD_CAST "SignalControlerError");
						xmlNewChild(SignalError_Node,NULL,BAD_CAST "SignalControlerID",BAD_CAST SignalID);
						sprintf(temp_buf,"%d",SignalRealData[j].SignalError);
						xmlNewChild(SignalError_Node,NULL,BAD_CAST "ErrorType",BAD_CAST temp_buf);
						xmlNewChild(SignalError_Node,NULL,BAD_CAST "ErrorDesc",BAD_CAST SignalRealData[j].FaultDesc);
						xmlNewChild(SignalError_Node,NULL,BAD_CAST "OccerTime",BAD_CAST SignalRealData[j].FaultTime);
						xmlAddChild(ObjectNode_Fault,SignalError_Node);
					}
					//控制方式
					if(SignalRealData[j].IsControlModeUpdate == true )
					{
						ControlMode_flag = true;
						ControlMode_Node = xmlNewNode(NULL,BAD_CAST "CrossControlMode");
						xmlNewChild(ControlMode_Node,NULL,BAD_CAST "CrossID",BAD_CAST SignalID);
						xmlNewChild(ControlMode_Node,NULL,BAD_CAST "Value",BAD_CAST SignalRealData[j].CurControlMode);
						xmlAddChild(ObjectNode_ControlMode,ControlMode_Node);
					}
					//控制方案
					if(SignalRealData[j].IsCrossPlanUpdate == true )
					{
						Plan_flag = true ;
						CrossPlan_Node = xmlNewNode(NULL,BAD_CAST "CrossPlan");
						xmlNewChild(CrossPlan_Node,NULL,BAD_CAST "CrossID",BAD_CAST SignalID);
						sprintf(temp_buf,"%d",SignalRealData[j].CurPlan);
						xmlNewChild(CrossPlan_Node,NULL,BAD_CAST "PlanNo",BAD_CAST temp_buf);
						xmlAddChild(ObjectNode_Plan,CrossPlan_Node);
					}
					//判断订阅的内容是否需要上报
					for(i = 0;  i < SignalMaxNum ;i++)                           	//在连接信息中找到要上报信号机的信息
					{
						if(TcpThread[ThreadNum].ReportInfo[i].CrossID == SignalRealData[j].CrossID)
						{
							break;
						}
					}
					if(i == SignalMaxNum)      //i作为数组下标，有效值是SignalMaxNum-1  i等于最大值就是没有找到
						continue;
					//路口周期
					//if( ( TcpThread[ThreadNum].ReportInfo[i].CrossCycle_flag == true ) && (SignalRealData[j].IsCrossCycleUpdate))
					if( (SignalRealData[j].IsCrossCycleUpdate))
					{
						Cycle_flag = true;
						CrossCycle_Node = xmlNewNode(NULL, BAD_CAST "CrossCycle");
						xmlNewChild(CrossCycle_Node,NULL,BAD_CAST "CrossID",BAD_CAST SignalID);
						xmlNewChild(CrossCycle_Node,NULL,BAD_CAST "StartTime",BAD_CAST SignalRealData[j].StartTime);
						sprintf(temp_buf,"%d",SignalRealData[j].LastCycleLen);
						xmlNewChild(CrossCycle_Node,NULL,BAD_CAST "LastCycleLen",BAD_CAST temp_buf);
						xmlAddChild(ObjectNode_Cycle,CrossCycle_Node);
					}
					//路口阶段
					//if( ( TcpThread[ThreadNum].ReportInfo[i].CrossStage_flag == true ) && (SignalRealData[j].IsCrossStageUpdate))
					if( (SignalRealData[j].IsCrossStageUpdate))
					{
						Stage_flag = true;
						CrossStage_Node = xmlNewNode(NULL, BAD_CAST "CrossStage");
						xmlNewChild(CrossStage_Node,NULL,BAD_CAST "CrossID",BAD_CAST SignalID);
						xmlNewChild(CrossStage_Node,NULL,BAD_CAST "LastStageNo",BAD_CAST SignalRealData[j].LastStageNo);
						xmlNewChild(CrossStage_Node,NULL,BAD_CAST "LastStageLen",BAD_CAST SignalRealData[j].LastStageLen);
						xmlNewChild(CrossStage_Node,NULL,BAD_CAST "CurStageNo",BAD_CAST SignalRealData[j].CurStageNo);
						xmlNewChild(CrossStage_Node,NULL,BAD_CAST "CurStageLen",BAD_CAST SignalRealData[j].CurStageLen);
						xmlAddChild(ObjectNode_Stage,CrossStage_Node);
					}
					//相位灯态
					//if(( TcpThread[ThreadNum].ReportInfo[i].CrossPhaseLampStatus_flag == true ) && (SignalRealData[j].IsPhaseUpdate))
					if(SignalRealData[j].IsPhaseUpdate)
					{
						Phase_flag = true;
						CrossPhaseLampStatus_Node = xmlNewNode(NULL, BAD_CAST "CrossPhaseLampStatus");
						xmlNewChild(CrossPhaseLampStatus_Node,NULL,BAD_CAST "CrossID",BAD_CAST SignalID);
						PhaseLampStatusList_Node = xmlNewNode(NULL, BAD_CAST "PhaseLampStatusList");
						for(temp = 0;temp<32;temp++)
						{
							PhaseLampStatus_Node = xmlNewNode(NULL, BAD_CAST "PhaseLampStatus");

							sprintf(temp_buf,"%d",(temp+1));                //将相位编号转换成字符串
							xmlNewChild(PhaseLampStatus_Node,NULL,BAD_CAST "PhaseNo",BAD_CAST temp_buf);
							// 灭灯:11     红灯:21    绿灯:22   黄灯:23   红黄:31    绿闪:41   黄闪:40
							switch(SignalRealData[j].PhaseLampStatus[temp])
							{
								case 0x01:  //灭灯
									xmlNewChild(PhaseLampStatus_Node,NULL,BAD_CAST "LampStatus",BAD_CAST"11");
									break;
								case 0x10:  //红灯
								case 0x11:  //红闪
								case 0x12:  //红快闪
									xmlNewChild(PhaseLampStatus_Node,NULL,BAD_CAST "LampStatus",BAD_CAST"21");
									break;
								case 0x20:  //绿黄
								case 0x41:  //绿灯
									xmlNewChild(PhaseLampStatus_Node,NULL,BAD_CAST "LampStatus",BAD_CAST"22");
									break;
								case 0x21:  //绿闪
								case 0x22:  //绿快闪
									xmlNewChild(PhaseLampStatus_Node,NULL,BAD_CAST "LampStatus",BAD_CAST"41");
									break;
								case 0x30:  //黄灯
									xmlNewChild(PhaseLampStatus_Node,NULL,BAD_CAST "LampStatus",BAD_CAST"23");
									break;
								case 0x31:  //黄闪
								case 0x32:  //黄快闪
									xmlNewChild(PhaseLampStatus_Node,NULL,BAD_CAST "LampStatus",BAD_CAST"40");
									break;
								case 0x40:  //红黄
									xmlNewChild(PhaseLampStatus_Node,NULL,BAD_CAST "LampStatus",BAD_CAST"31");
									break;
								default:          //
									break;
							}
							xmlAddChild(PhaseLampStatusList_Node,PhaseLampStatus_Node);
						}
						xmlAddChild(CrossPhaseLampStatus_Node,PhaseLampStatusList_Node);
						xmlAddChild(ObjectNode_Phase,CrossPhaseLampStatus_Node);
					}
					//路口交通流数据
					if( ( TcpThread[ThreadNum].ReportInfo[i].CrossTrafficData_flag == true ) && (SignalRealData[j].IsTrafficDataUpdate))
					{
						Traffic_flag = true;
						CrossTrafficData_Node = xmlNewNode(NULL, BAD_CAST "CrossTrafficData");
						xmlNewChild(CrossTrafficData_Node,NULL,BAD_CAST "CrossID",BAD_CAST SignalID);
						xmlNewChild(CrossTrafficData_Node,NULL,BAD_CAST "EndTime",BAD_CAST SignalRealData[j].EndTime);
						xmlNewChild(CrossTrafficData_Node,NULL,BAD_CAST "Interval",BAD_CAST SignalRealData[j].Interval);

						DataList_Node = xmlNewNode(NULL, BAD_CAST "DataList");
						for(temp = 0; temp < 32 ; temp++)
						{
							if(SignalRealData[j].TrafficData[temp].IsDataUpdate == true)
							{
								Data_Node = xmlNewNode(NULL, BAD_CAST "Data");
								sprintf(temp_buf,"%d",(temp+1));
								xmlNewChild(Data_Node,NULL,BAD_CAST "LaneNo",BAD_CAST temp_buf);
								xmlNewChild(Data_Node,NULL,BAD_CAST "Volume",BAD_CAST SignalRealData[j].TrafficData[temp].Volume);
								xmlNewChild(Data_Node,NULL,BAD_CAST "AvgVehLen",BAD_CAST "0");
								xmlNewChild(Data_Node,NULL,BAD_CAST "Pcu",BAD_CAST "0");
								xmlNewChild(Data_Node,NULL,BAD_CAST "HeadDistance",BAD_CAST "0");
								xmlNewChild(Data_Node,NULL,BAD_CAST "HeadTime",BAD_CAST "0");
								xmlNewChild(Data_Node,NULL,BAD_CAST "Speed",BAD_CAST "0");
								xmlNewChild(Data_Node,NULL,BAD_CAST "Saturation",BAD_CAST "0");
								xmlNewChild(Data_Node,NULL,BAD_CAST "Density",BAD_CAST "0");
								xmlNewChild(Data_Node,NULL,BAD_CAST "QueueLength",BAD_CAST "0");
								xmlNewChild(Data_Node,NULL,BAD_CAST "Occupancy",BAD_CAST SignalRealData[j].TrafficData[temp].Occupancy);
								xmlAddChild(DataList_Node,Data_Node);
							}
						}
						xmlAddChild(CrossTrafficData_Node,DataList_Node);
						xmlAddChild(ObjectNode_Traffic,CrossTrafficData_Node);
					}
				}

				order_value = 1;
				if(SignalState_flag == true)
				{
					printf("SignalState_flag\n");
					sprintf(orderbuf,"%d",order_value);
					xmlSetProp(OperateNode_SignalState,BAD_CAST"order",BAD_CAST orderbuf);
					order_value++;
				}
				else
				{
					xmlUnlinkNode(OperateNode_SignalState);
					xmlFreeNode(OperateNode_SignalState);
				}
				if(ControlMode_flag == true)
				{
					printf("ControlMode_flag\n");
					sprintf(orderbuf,"%d",order_value);
					xmlSetProp(OperateNode_ControlMode,BAD_CAST"order",BAD_CAST orderbuf);
					order_value++;
				}
				else
				{
					xmlUnlinkNode(OperateNode_ControlMode);
					xmlFreeNode(OperateNode_ControlMode);
				}
				if(Plan_flag == true )
				{
					printf("Plan_flag\n");
					sprintf(orderbuf,"%d",order_value);
					xmlSetProp(OperateNode_Plan,BAD_CAST"order",BAD_CAST orderbuf);
					order_value++;
				}
				else
				{
					xmlUnlinkNode(OperateNode_Plan);
					xmlFreeNode(OperateNode_Plan);
				}
				if(Cycle_flag == true)
				{
					printf("Cycle_flag\n");
					sprintf(orderbuf,"%d",order_value);
					xmlSetProp(OperateNode_Cycle,BAD_CAST"order",BAD_CAST orderbuf);
					order_value++;
				}
				else
				{
					xmlUnlinkNode(OperateNode_Cycle);
					xmlFreeNode(OperateNode_Cycle);
				}
				if(Stage_flag == true)
				{
					printf("Stage_flag\n");
					sprintf(orderbuf,"%d",order_value);
					xmlSetProp(OperateNode_Stage,BAD_CAST"order",BAD_CAST orderbuf);
					order_value++;
				}
				else
				{
					xmlUnlinkNode(OperateNode_Stage);
					xmlFreeNode(OperateNode_Stage);
				}
				if(Phase_flag == true)
				{
					printf("Phase_flag\n");
					sprintf(orderbuf,"%d",order_value);
					xmlSetProp(OperateNode_Phase,BAD_CAST"order",BAD_CAST orderbuf);
					order_value++;
				}
				else
				{
					xmlUnlinkNode(OperateNode_Phase);
					xmlFreeNode(OperateNode_Phase);
				}
				if(Traffic_flag == true)
				{
					printf("Traffic_flag\n");
					sprintf(orderbuf,"%d",order_value);
					xmlSetProp(OperateNode_Traffic,BAD_CAST"order",BAD_CAST orderbuf);
					order_value++;
				}
				else
				{
					xmlUnlinkNode(OperateNode_Traffic);
					xmlFreeNode(OperateNode_Traffic);
				}
				if(Fault_flag == true)
				{
					printf("Fault_flag\n");
					sprintf(orderbuf,"%d",order_value);
					xmlSetProp(OperateNode_Fault,BAD_CAST"order",BAD_CAST orderbuf);
					order_value++;
				}
				else
				{
					xmlUnlinkNode(OperateNode_Fault);
					xmlFreeNode(OperateNode_Fault);
				}
				//系统状态
				if(TcpThread[ThreadNum].IsSysStateUpdate)
				{
					sprintf(orderbuf,"%d",order_value);
					OperateNode_SysState = xmlNewNode(NULL,BAD_CAST"Operation");
					xmlSetProp(OperateNode_SysState,BAD_CAST"order",BAD_CAST orderbuf);
					xmlSetProp(OperateNode_SysState,BAD_CAST"name",BAD_CAST"Notify");
					ObjectNode_SysState =  xmlNewNode(NULL,BAD_CAST"Object");
					SysState_Node =  xmlNewNode(NULL,BAD_CAST"SysState");
					xmlNewChild(SysState_Node,NULL,BAD_CAST "Value",BAD_CAST "Online");
					xmlAddChild(ObjectNode_SysState,SysState_Node);
					xmlAddChild(OperateNode_SysState,ObjectNode_SysState);
					xmlAddChild(Body_Node,OperateNode_SysState);
					order_value++;
					TcpThread[ThreadNum].IsSysStateUpdate = false;
				}
				//区域状态
				if(TcpThread[ThreadNum].IsRegionStateUpdate)
				{
					if(GetOracleConnect())
					{
						sprintf(orderbuf,"%d",order_value);
						OperateNode_RegionState = xmlNewNode(NULL,BAD_CAST"Operation");
						xmlSetProp(OperateNode_RegionState,BAD_CAST"order",BAD_CAST orderbuf);
						xmlSetProp(OperateNode_RegionState,BAD_CAST"name",BAD_CAST"Notify");
						ObjectNode_RegionStatus =  xmlNewNode(NULL,BAD_CAST"Object");

						sprintf(sqlbuf,"select t.disrict_id from DISRICT_INFOR t order by t.disrict_id");
						try
						{
							Stmt->setSQL(sqlbuf);
							Result = Stmt->executeQuery();
							while(Result->next() != 0)
							{
								RegionState_Node =  xmlNewNode(NULL,BAD_CAST"RegionState");
								xmlNewChild(RegionState_Node, NULL, BAD_CAST "RegionID",BAD_CAST Result->getString(1).c_str());
								xmlNewChild(RegionState_Node,NULL,BAD_CAST "Value",BAD_CAST "Online");
								xmlAddChild(ObjectNode_RegionStatus,RegionState_Node);
							}
							Stmt->closeResultSet(Result);
						}
						catch(SQLException &sqlExcp)
						{
							cout << " Error Number : "<< sqlExcp.getErrorCode() << endl; //获得异常代码
							cout << sqlExcp.getMessage() << endl; //获得异常信息
						}
						TerminateConnection();
						xmlAddChild(OperateNode_RegionState,ObjectNode_RegionStatus);
						xmlAddChild(Body_Node,OperateNode_RegionState);
						TcpThread[ThreadNum].IsRegionStateUpdate = false;
					}
				}
				if(order_value == 1)
				{
					xmlFreeDoc(doc);
					break;
				}
				xmlDocDumpFormatMemoryEnc(doc, &OutBuf, &outlen, "UTF-8", 1);
				xmlFreeDoc(doc);
				TCPService_Data_Send(TcpThread[ThreadNum].commt_fd,(char *)OutBuf,outlen);
				//puts( (char *)OutBuf);
				xmlFree(OutBuf);
			}
		}
		for(j = 0;j< SignalMaxNum;j++)     //清空所有更新标志
		{
			if(SignalRealData[j].CrossID == -1)   //要查询的实时数据结构体中的路口ID=-1，说明已经查询完所有存在的信号机了
			{
				break;
			}
			SignalRealData[j].IsStatusUpdate = false;
			SignalRealData[j].IsCrossCycleUpdate = false;
			SignalRealData[j].IsCrossPlanUpdate = false;
			SignalRealData[j].IsControlModeUpdate = false;
			SignalRealData[j].IsCrossStageUpdate = false;
			SignalRealData[j].IsSignalErrorUpdate = false;
			SignalRealData[j].IsPhaseUpdate = false;
			SignalRealData[j].IsTrafficDataUpdate = false;
		}
		pthread_mutex_unlock(&UpdateRealData);
		sleep(60);
	}
	return (void *)0;
}

int GetOracleConnect()
{
	try
	{
		Conn=pConnPool->getConnection();
		if(Conn)
		{
			 Stmt = Conn->createStatement();
			 Stmt->setAutoCommit(true);
		}
		else
		{
			return fail;
		}
	}
	catch(SQLException &sqlExcp)
	{
		return fail;
	}
	return succeed;
}
int TerminateConnection()
{
	try
	{
		Conn->terminateStatement(Stmt);
		OraEnviroment->terminateConnection(Conn);
	}
	catch(SQLException &sqlExcp)
	{
		return fail;
	}
	return succeed;

}


