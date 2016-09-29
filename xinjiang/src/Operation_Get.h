/*
 * Operate_Get.h
 *
 *  Created on: 2016年7月25日
 *      Author: root
 */

#ifndef OPERATION_GET_H_
#define OPERATION_GET_H_


int Get_SysInfo(xmlNodePtr OperationNode,int ThreadNum);
int Get_RegionParam(xmlNodePtr OperationNode,int ThreadNum);
int Get_SubRegionParam(xmlNodePtr OperationNode,int ThreadNum);
int Get_CrossParam(xmlNodePtr OperationNode,int ThreadNum);
int Get_SignalControler(xmlNodePtr OperationNode,int ThreadNum);
int Get_LampGroup(xmlNodePtr OperationNode,int ThreadNum);
int Get_DetParam(xmlNodePtr OperationNode,int ThreadNum);
int Get_LaneParam(xmlNodePtr OperationNode,int ThreadNum);
int Get_PhaseParam(xmlNodePtr OperationNode,int ThreadNum);
int Get_StageParam(xmlNodePtr OperationNode,int ThreadNum);
int Get_PlanParam(xmlNodePtr OperationNode,int ThreadNum);



#endif /* OPERATION_GET_H_ */
