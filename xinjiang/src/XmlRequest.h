/*
 * XmlRequest.h
 *
 *  Created on: 2016年7月14日
 *  Author: root
 */

#ifndef XMLREQUEST_H_
#define XMLREQUEST_H_

#define SysInfo  						"SysInfo"					//系统参数
#define RegionParam  				"RegionID"				//区域参数
#define SubRegionParam   			"SubRegionParam"     	//子区参数
#define CrossParam    				"CrossParam"       		//路口参数
#define SignalControler   			"SignalControler"     	//信号机参数
#define LampGroup    				"LampGroup"    			//信号灯组参数
#define DetParam       				"DetParam"   				//检测器参数
#define LaneParam       			"LaneParam"  				//车道参数
#define PhaseParam       			"PhaseParam"  				//相位参数
#define StageParam      			"StageParam"    			//阶段参数
#define PlanParam     				"PlanParam"    			//配时方案参数

#define SysState       				"SysState"   				//系统状态
#define RegionState     			"RegionState"  			//区域状态
#define CrossState      			"CrossState"  				//路口状态
#define SignalControlerError   	"SignalControlerError"  //信号机故障
#define CrossControlMode     		"CrossControlMode"     	//路口控制方式
#define CrossCycle    	 			"CrossCycle"           	//路口周期
#define CrossStage       			"CrossStage"         	//路口阶段
#define CrossPhaseLampStatus    	"CrossPhaseLampStatus"  //路口相位状态
#define CrossPlan           		"CrossPlan"      			//路口控制方案
#define CrossTrafficData    		"CrossTrafficData"      //路口交通流数据


#define LockFlowDirection     	"LockFlowDirection"    	//锁定交通流向
#define UnLockFlowDirection    	"UnLockFlowDirection"   //解锁交通流向

#define CrossReportCtrl        	"CrossReportCtrl"   		//路口周期、阶段、相位灯态、交通流数据上传设置



int RequestOperate(xmlDocPtr pdoc,xmlXPathContextPtr Xpath,int ThreadNum);
int Operation_Get(xmlNodePtr OperationNode,int ThreadNum);
int Operation_Set(xmlNodePtr OperationNode,int ThreadNum);
int RequestLogout(xmlNodePtr OperationNode,int ThreadNum);





#endif /* XMLREQUEST_H_ */
