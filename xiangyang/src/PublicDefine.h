/*
 * PublicDefine.h
 *
 * Created on: 2016年11月30日
 * Author: LIRUIJIE
 */

#ifndef PUBLICDEFINE_H_
#define PUBLICDEFINE_H_

#define 		FrameMaxLen			10000
#define		DISCONNECT			-1
#define 		MaxSignalNum		1024
#define 		XmlVersion			"1.0"
struct ServiceInfo{
		char my_ip[10][20];
		char ip[20];
		int port;
		long ReportTime;
		char token[30];
		int IsConnect;
};
typedef struct ServiceInfo ServiceInfo;


#define 	WAIT	-1

struct SignalFlag{
		int GetBaseInfo;
		int GetPhase;
		int GetPlan;

		int SetPlan;
		int SetPhase;

		int CtrlNextStep;
		int CtrlChangePlan;
		int CtrlSpecialService;
		int CtrlDemoteMode;
		int CtrlRegionsCoordinate;
		int CtrlCheckTime;
		int CtrlRestartSignal;
};

struct SignalBaseInfo
{
		int unit_id;
		char cross_name[100];
		int signal_type;
		int dir_type;
		char area_name[100];
		int area_id;
};

struct SignalPhase
{
		int PhaseID;		// 相位号
		int Yellow;			//	黄灯时长 单位：秒
		int ClearRed;		//	清场红  单位：秒
		int PGreenFlash;	//	人行绿闪时长 单位：秒
		int VGreenFlash;	//	车道绿闪时长 单位：秒
		int PRedFlash;		//	人行红闪时长 单位：秒
		int VRedFlash;		//	车道红闪时长 单位：秒
		int LaneCount;		//	包含车道的数量
		int LaneNo[24];	//	通道号，至少一个 通道列表详见附录6

};

struct SiganlRealPhaseStatus
{
		int PhaseNo;		//	相位号
		int LampStatus;	//	灯态 灯态列表详见附录4
		int Duration;		//	灯态持续时间，单位：秒
		int ControlMode;	//	控制模式 控制模式列表详见附录7
		int PlanNo;			//	方案号
		int StageNo;		//	阶段号
		int LaneNo[24];	//	通道号，至少一个 通道列表详见附录6
		int LaneCount;		//车道数量
};

struct PlanPeriod
{
		int PeriodNum;			//时段序号
		int StartHour;			//开始时间 时
		int StartMinute;		//开始时间 分
		int ControlMode;		//控制模式 控制模式列表详见附录7
		int ModeNo;				//模式号
		int CoordPhaseNo;		//协调相位号
		int OffSet;				//相位差 单位：秒
		int StageCount;			//阶段的数量
		int PhaseNo[16];			//相位号
		int Green[16];				//绿灯时长 单位：秒
};
struct SignalPlan
{
		int PlanID;				//方案号
		int PeriodCount;			//方案中时段的数量
		struct PlanPeriod Period[24];

};
struct SignalInfo{
		int cross_id;
		int status;
		int IsSubscribe;
		int RecvCtrlHeartbeat;
		int IsWebControled;
		long WebHeartbeatTime;
		long LastReportTime;
		char ip[20];
		int port;
		struct SignalFlag flag;
		struct SignalBaseInfo baseinfo;
};

enum OperateObject
{
	BaseInfo = 1,
	Phase = 2,
	Plan = 3,
	NextStep = 4,
	ChangePlan = 5,
	SpecialService = 6,
	DemoteMode = 7,
	RegionsCoordinate = 8,
	CheckTime = 9,
	RestartSignal = 10
};

struct FlowInfo
{
		int signal_id;
		int dirction;
		int flow;
};




typedef void (*GetCoordPlan)(int index,Connection *_conn,Statement *_stmt);
struct AreaCoordSingalInfo
{
		int cross_id;
		struct AreaCoordSingalInfo *Next_E;			//东方向路口
		struct AreaCoordSingalInfo *Next_W;			//西方向路口
		struct AreaCoordSingalInfo *Next_S;			//南方向路口
		struct AreaCoordSingalInfo *Next_N;			//北方向路口
};


#define PlanTime 200

struct RegionsCoord_ZKTD_Detector
{
		int id;
		int dir;
		int flow;
};

struct RegionsCoord_PhaseInfo
{
		int id;			//相位ID
		int dir;			//相位方向    当该相位需要协调时有用
		int flow;				//相位所包的含所有检测器在一定周期内的流量和
		int detector[16];
		int detector_flow[16];

};
struct RegionsCoord_StageInfo
{
		int num;		//阶段序号
		int id;		//阶段ID
		int green_time;		//需要放行的绿灯时间（通过流量数据计算得出）
		int phase_count;		//包含相位个数
		int flow;				//阶段在一定周期内的流量和
		struct RegionsCoord_PhaseInfo phase[16];			//相位ID

		char ZKTD_Lane[20];

};
struct RegionsCoord_unit_info
{
		int ID;								//路口ID
		char Signal_type[10];			//信号机类型
		int IS_MASTER;						//主要路口标志   1表示主要路口  2表示次要路口
		int IS_Coordinated;				//是否参与协调		0表示不参与   1表示参与协调
		int Coordinate_Dir;				//协调方向
		int StageCount;					//阶段数量
		int flow;
		struct RegionsCoord_StageInfo Stage[16];	//阶段信息
		GetCoordPlan GetCoordinatePlan;

};
struct RegionsCoordInfo
{
		int CYCLE_MAX;					//最大周期
		int CYCLE_MIN;					//最小周期
		int PLAN_UPDATE_CYCLE;		//方案跟新周期
		int MasterUnitCycle;			//主要路口方案周期
		int UnitCount;					//路口数量
		struct RegionsCoord_unit_info Unit[100];		//路口信息
};


struct CANALIZATION_CFG
{
		int id;
		int enter;
		int exit;
};
void GetSeq(char *seqbuf);




#endif /* PUBLICDEFINE_H_ */
