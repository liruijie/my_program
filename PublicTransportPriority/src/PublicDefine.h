/*
 * PublicDefine.h
 *
 * Created on: 2016年10月9日
 * Author: LIRUIJIE
 */

#ifndef PUBLICDEFINE_H_
#define PUBLICDEFINE_H_

#define DeviceMaxNum			1024
#define SingleRecvMaxLen 	1000
#define QueueNum 50000

struct DeviceRealData
{
		char  DeviceTime[30];


		char line_number[20];				//公交车次
		char plate_number[20];				//车牌号
		char RFID[40];							//RFID卡编号
		unsigned long detect_time;			//检测时间
		bool is_priority;						//是否优先：01 为优先（01为已优先，00为没优先）
		char priority_level;					//优先级：01 （1~9）
		unsigned char priority_time;		//优先时间：04 为优先4秒
		int output_port;						//控制板输出端口：00 01 为1号端口
		char detect_direction;				//检测方向：01 代表东方向（1代表东，2代表西，3代表南，4代表北）
		int cross_number;						//路口编号：00 03
		unsigned long request_time;		//请求时间：4F DC 68 2E 表示该报文的上报时间距1970年0时0分0秒的秒数为1339844654s

		char fault_type;
		unsigned fault_time;
		int fault_number;
};
struct Device_CardReaderInfo
{
		int id;									//读卡器编号
		int direction;							//检测方向：02
		int dBm;									//读卡器发送功率：00 03
		int zigbee_addr;						//无线模块地址：00 04
		int transport_priority_relay;		//公交优先对应的继电器编号：00 05
		int emergency_priority_relay;		//紧急优先对应的继电器编号：00 06
		int detection_range;					//检测器检测范围：00 07
		int cross_id;							//路口编号：00 08
};

struct DeviceInfo
{
		int id;
		int status;
		char ip[20];
		char mask[20];
		char gateway[20];
		char center_ip[20];
		int center_port;
		int control_uart;
		int wireless_uart;
		struct Device_CardReaderInfo cardreader[8];
		struct DeviceRealData realdata;

};
extern DeviceInfo device[DeviceMaxNum];

struct QueueInfo
{
	unsigned char buf[SingleRecvMaxLen];
	struct QueueInfo *Next;
};

#endif /* PUBLICDEFINE_H_ */