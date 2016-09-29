/*
 * publicdefine.h
 *
 *  Created on: 2016年7月4日
 *      Author: root
 */

#ifndef PUBLICDEFINE_H_
#define PUBLICDEFINE_H_


#define SingleRecvMaxLen    5000                                //单次接收报文最大长度   决定了接收时所用buffer的大小
//#define OracleConfigeFile          "./confige"                 //控制台运行时用这个目录
#define OracleConfigeFile         "/home/eclipseprogram/xinjiang/Debug/confige"              //eclipse调试用这个目录

#define succeed   1
#define fail      0
#define Online 	1
#define Offline 	0


#define SDE_Version 		-1
#define SDE_Token  		-2
#define SDE_Addr			-3
#define SDE_MsgType		-4
#define SDE_OperName		-5
#define SDE_UserName		-6
#define SDE_Pwd			-7
#define SDE_NotAllow		-8
#define SDE_Failure		-9
#define SDE_Unknown		-10

#define MaxConnect 			 200
#define SignalMaxNum 		 512

struct CrossReportInfo
{
	int CrossID;							//路口ID

	//bool SysState;							//系统状态
	//bool RegionState;						//区域状态
	//bool CrossState;                	//路口状态
	//bool SignalControlerError_flag;	//是否上报信号机故障
	//bool CrossControlMode_flag;       //是否上报路口控制方式
	bool CrossCycle_flag;             	//是否上报路口周期
	bool CrossStage_flag;					//是否上报路口阶段
	bool CrossPhaseLampStatus_flag;		//是否上报路口相位灯态
	//bool CrossPlan_flag;					//路口控制方案
	bool CrossTrafficData_flag;			//是否上报交通流数据


};

struct xmlResponseError
{
	char OperateProp_Order[2];
	char OperateProp_Name[10];
	char ObjectnName[20];
};
struct TcpThreadInfo
{
	int IsConnect;                    	//连接标志
	int commt_fd;                			//链接文件描述符
	pthread_t thread_id;        			//线程ID
	char Message[SingleRecvMaxLen]; 		//接收缓存区
	char Token[35];              			//令牌
	struct xmlResponseError xmlErrorInfo;       				//错误信息（错误节点所在的operation的order和name，以及对象的名字）
	struct CrossReportInfo ReportInfo[SignalMaxNum];      //上报信息标志  1上报  0不上报
	Connection *conn;                 //oracle连接描述符
	Statement *stmt;                  //oracle操作描述符


	xmlNodePtr FromNode;
	xmlNodePtr ToNode;
	//连接建立后要上报一次系统状态和区域状态
	bool IsSysStateUpdate;
	bool IsRegionStateUpdate;
};

struct CrossPhaseLampStatus
{
	int PhaseNo;
	int LampStatus;
};
struct CrossTrafficData
{
	bool IsDataUpdate;
	//int LaneNo;				//车道编号
	char Volume[10];		//交通流量
	int AvgVehLen;			//平均车长
	int Pcu;					//当量小汽车
	int HeadDistance;		//平均车头间距
	int HeadTime;			//平均车头时距
	int Speed;				//平均速度
	int Saturation;		//饱和度
	int Density;			//密度
	int QueueLength;		//平均排队长度
	char Occupancy[10];		//占有率
};
enum Control
{
	Ctr_Idle,
	Ctr_LockTraffic,
	Ctr_UnLockTraffic,

	Ctr_ControlMode,
	Ctr_CrossPlan,
	Ctr_CrossStage,

	Cancel_ControlMode,
	Cancel_CrossPlan,
	Cancel_CrossStage
};
struct SignalData
{
	int CrossID;           		//路口ID

	unsigned long ReportTime;
/**********************************/
	bool IsStatusUpdate;        //
	int SignalState;			//信号机状态


/**********************************/
	bool IsCrossCycleUpdate;
	char StartTime[20];        //周期开始时间(当前方案开始时间)
	int CurCycleLen;
	int LastCycleLen;         	//上周期长度(上个方案的周期长度)


/**********************************/
	bool IsCrossStageUpdate;
	int CurStageNo_Value;
	char LastStageNo[5];				//上个阶段号
	char LastStageLen[5];				//上个阶段长度
	char CurStageNo[5];				//当前阶段号
	char CurStageLen[5];				//当前阶段长度


/**********************************/
	bool IsPhaseUpdate;
	char PhaseLampStatus[32];  //路口相位灯态



/**********************************/
	bool IsTrafficDataUpdate;
	char EndTime[20];
	char Interval[5];
	struct CrossTrafficData TrafficData[32];			//路口交通流数据


/**********************************/
	bool IsCrossPlanUpdate;
	char LastPlan;
	char CurPlan;              //路口控制方式


/**********************************/
	bool IsControlModeUpdate;
	char CurControlMode[3];       //当前路口控制方式    对应公安协议中的值
	char ControlMode_Value;		//通讯服务器转发报文中的值 对应公司的通讯协议

/**********************************/
	int IsSignalErrorUpdate;	//信号机故障是否更新（避免重复上报）
	int SignalError;           //信号机故障类型
	char FaultDesc[300];       //故障描述
	char FaultTime[30];        //故障发生时间


//////////////////////////////////////////////////////////////////////////

	bool IsControlled;						//是否被控制
	enum Control ControlFunction;       //处在哪种控制状态
/*********锁定交通流信息*********/
	bool IsExistLockTask;
	int LockEnter;
	int LockExit;
	long LockStartTime;
	int LockDuration;
	int LockStageNum;
	/**********************/
	int CrossContolMode;
	int CtrCrossStage;
	int CtrStageLen;
	int CtrCrossPlan;

};

#endif /* PUBLICDEFINE_H_ */
