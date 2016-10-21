/*
 * PublicDefine.h
 *
 * Created on: 2016年10月9日
 * Author: LIRUIJIE
 */

#ifndef PUBLICDEFINE_H_
#define PUBLICDEFINE_H_

#define DeviceMaxNum			1024
#define SingleRecvMaxLen 	10000
#define QueueNum 50000
#define CardMaxNum			5000

/*
 * 设备实时状态
 */
struct DeviceRealData
{
		char  DeviceTime[30];				//设备时间
		char line_number[30];				//公交车次
		char plate_number[30];				//车牌号
		unsigned long RFID;							//RFID卡编号
		long detect_time;			//检测时间
		bool is_priority;						//是否优先：01 为优先（01为已优先，00为没优先）
		char priority_level;					//优先级：01 （1~9）
		unsigned char priority_time;		//优先时间：04 为优先4秒
		int output_port;						//控制板输出端口：00 01 为1号端口
		char detect_direction;				//检测方向：01 代表东方向（1代表东，2代表西，3代表南，4代表北）
		long int request_time;		//请求时间：4F DC 68 2E 表示该报文的上报时间距1970年0时0分0秒的秒数为1339844654s
		unsigned char IsLeave;

//		数据编号	故障类型	故障设备编号	时间信息
//		0x01		1个字节		2字节			4个字节
		char fault_type;						//故障类型
		int fault_number;						//故障设备编号
		long fault_time;			//时间信息
};
/*
 * 读卡器设备信息
 */
struct Device_CardReaderInfo
{
		int id;									//读卡器编号
		int direction;							//检测方向：02
		int dBm;									//读卡器发送功率：00 03
		int zigbee_addr;						//无线模块地址：00 04
		int transport_priority_relay;		//公交优先对应的继电器编号：00 05
		int emergency_priority_relay;		//紧急优先对应的继电器编号：00 06
		int detection_range;					//检测器检测范围：00 07
};

/*
 * 设备系统信息
 */
struct DeviceInfo
{
//		IP地址	子网掩码	网关	上位机IP 	上位机端口 		控制板对应串口		无线模块基站对应串口
//		4字节	4字节		4字节	4字节		2字节			2字节				2字节


		int id;									//设备ID
		int status;								//设备状态  是否在线
		unsigned long last_report_time;	//设备上次上报时间
		char ip[20];                  	//设备IP   192.168.1.100
		char mask[20];							//设备掩码	  255.255.255.0
		char gateway[20];						//设备网关   192.168.1.100
		char center_ip[20];					//中心（通讯服务器）IP	  	192.168.1.200
		int center_port;						//中心（通讯服务器）端口		10086
		int control_uart;						//控制板对应串口
		int wireless_uart;					//无线模块对应串口
		struct DeviceRealData realdata;
};
extern DeviceInfo device[DeviceMaxNum];

/*
 * 策略表
 */
struct Device_Strategy
{
		int strategy_id;								//策略表编号
		int sequence_num;								//方向序号
		unsigned char direction;					//优先方向
		unsigned char level;							//优先级别
		int param1;
		int param2;
		int param3;
		int param4;
		unsigned char threshold;					//优先申请阈值
		unsigned char interval;						//统计间隔
		unsigned char max_time_allowed;			//优先申请权保留时间
		unsigned int card_live_time;          	//数据保存时间
};

/*
 * 时间表
 */
struct Device_StrategyTime
{
//		时段序号	开始时间	结束时间	策略表序号
//		0x01		2字节		2字节		1字节

		int time_table_id;				//时间表ID
		int time_id;						//时段序号
		unsigned char start_time[2];	//开始时间
		unsigned char end_time[2];		//结束时间
		int strategy_id;					//策略表序号
};

/*
 * 调度表
 */
struct Device_Schedule
{
//		调度类型	调度优先级	星期值	月份	日期	时间表序号
//		1字节		1字节		1字节	2字节	4字节	1字节

		int id;									//调度表ID
		unsigned char type;					//调度类型
		unsigned char priority_level;		//调度优先级
		unsigned char week_value;			//星期值
		unsigned int month_value;			//月份
		unsigned long day_value;			//日期
		unsigned char time_table_id;		//时间表序号
};

struct Device_CardInfo
{
//			RFID卡号	车牌号				车牌颜色	优先申请阈值	安装时间	路次				所属运营公司
//			4字节		ASCII码16进制		1字节		1字节			4字节		ASCII码16进制		ASCII码16进制
		unsigned long  RFID;					//RFID卡号
		unsigned char plate_num[50];		//车牌号
		unsigned char plate_color;			//车牌颜色
		unsigned char threshold;			//优先申请阈值
		unsigned long install_time;		//安装时间
		unsigned char line_num[30];		//路次
		unsigned char company[50];			//所属运营公司
};

struct HiCON_Info
{
		char user[20];
		char passwd[20];
		char ip[20];
		int port;

};




#endif /* PUBLICDEFINE_H_ */
