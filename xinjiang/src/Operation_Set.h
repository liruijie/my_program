/*
 * Operation_Set.h
 *
 *  Created on: 2016年7月20日
 *      Author: root
 */

#ifndef OPERATION_SET_H_
#define OPERATION_SET_H_

//路口参数
/*struct Crossparam
{
	int CrossID;
	char CrossName[100];
	int Feature;
	int IsKey;
	char DetIDList[32];
	char LaneList[32];
	char PhaseNoList[32];
};

//车道参数
struct LaneParam
{
	int CrossID;
	int LaneNo;
	int Direction;
	int Attribute;
	int Movement;
	int Feature;
};*/

//阶段参数
struct Stage_Param
{
	int CrossID;
	int StageNo;
	char StageName[100];
	int Attribute;
	int Green;
	int RedYellow;
	int Yellow;
	int AllRed;
	int PhaseNoList[32];
};

//配时方案参数
struct Plan_Param
{
	int CrossID;
	int PlanNo;
	int CycleLen;
	int CoordPhaseNo;
	int OffSet;
	char StageNoList[32];
};
struct ITC_CFG_STAGE_DETAIL
{
	int stage_id;
	char stage_image[20];
};
struct Itc_Cfg_Plan_Detail
{
	int PlanID;
	char Detail[50];
};
struct ITC_CFG_PLAN_CHAIN
{
	int stage_id;
	int stage_time;
	int stage_type;
	float green_time;
	float green_flash_time;
	float green_flash_qtime;
	float red_time;
};

int Set_StageParam(xmlNodePtr OperationNode,int ThreadNum);
int Set_PlanParam(xmlNodePtr OperationNode,int ThreadNum);
int Set_CrossReportCtrl(xmlNodePtr OperationNode,int ThreadNum);
/***********************锁定交通流向*******************************/
int Set_LockFlowDirection(xmlNodePtr OperationNode,int ThreadNum);

/************************解锁交通流向*******************************/
int Set_UnLockFlowDirection(xmlNodePtr OperationNode,int ThreadNum);

/***********************指定方案*******************************/
int Set_CrossPlan(xmlNodePtr OperationNode,int ThreadNum);

/***********************指定控制方式*******************************/
int Set_CrossControlMode(xmlNodePtr OperationNode,int ThreadNum);

/***********************指定阶段*******************************/
int Set_CrossStage(xmlNodePtr OperationNode,int ThreadNum);

int GetDirection(int Direction);

void signalfunc(int sig);
#endif /* OPERATION_SET_H_ */
