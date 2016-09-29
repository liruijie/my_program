/*************************************************** 
 * 文件名：public_data.h
 * 版权：
 * 描述：记录所有的全局变量
 * 修改人：栾宇
 * 修改时间：2014-1-6
 * 修改版本：V0.1
 * 修改内容：
***************************************************/
#ifndef _PUBLIC_DATA_H_
#define _PUBLIC_DATA_H_

#include <iostream>
#include <string.h>
#include <occi.h>
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include "/home/ITC100/queue.h"
#include <errno.h>

using namespace oracle::occi;

using namespace std;


//const string oracle_username = "atc";
// const string oracle_password = "atc";
// const string oracle_connectstring= "192.168.1.24:1521/oracle";
extern  string oracle_username ;
 extern string oracle_password ;
 extern string oracle_connectstring;

 //string oracle_file_path = "/home/TC100_SERVER/confige";

// const string oracle_connectstring= "192.168.1.101:1521/utms";

/*信号机初始信息结构体*/
struct signal_info
{
	/*信号机ID*/
	int signal_id;

	/*信号机IP*/
	char signal_ip[16];

	/*信号机端口*/
	int signal_port;

	/*路口ID*/
	int unit_id;

	/*信号机类型: TC100  /  ITC100*/
	char device_type[10];
	
	/*信号机实时状态信息*/
	char recv_buf[512];
	
	/*信号机最后主动上报时间*/
	long last_time;
	

	/*信号机如果一次报文没收到为0  收到未1*/
	int signal_info_state;
	
	/*信号机在线状态 0离线 1在线*/
	int signal_state;
	
	/*信号机临时离线故障1故障0无故障*/
	int l_online_alarm;
	
	/*信号机临时离线故障时间*/
	char l_online_alarm_time[100];

	/*写oracle数据库的标志位，0，不写，1 写，在收到状态数据时，值为1*/
	int oracleFlag[2];

	
	/*信号机板在线状态*/
	char board_state [54];
	
	/*信号机相位灯态*/
	char phase_lht[100];

	/*信号机相位电流*/
	char phase_current[400];

	/*信号机检测器状态*/
	char detect_status[210];

	/*信号机检测器数据*/
	char detect_data_status[210];

	/*信号机紧急输入状态*/
	char detect_urgent_status[10];

	/*信号机优先输入状态*/
	char detect_pri_status[10];

	/*信号机门状态*/
	char door_status[8];

	/*信号机更新时间*/
	char update_time[21];


	/*信号机上一次模式*/
	int control_model;
		
	/*信号机当前模式*/
	int control_last_model;
	
	
	/*信号机当前方案*/
	int cur_plan;

	/*信号机当前周期*/
	int cur_week;

	/*信号机当前阶段*/
	int cur_stage;

	/*信号机协调序号*/
	int coordination_number;
	
	/*信号机协调时间*/
	int coordination_time;
	
	/*信号机放行时间*/
	int release_time;
	
	/*信号机剩余时间*/
	int rest_time;

	

	/*绿冲突数组[16] 16个相位[2] : [0] 故障标志 故障 -1  正常0  [1] 故障时间*/
	int green_conflicts[17][2];

	/*绿冲突时间数组[16] 16个相位[100]存放时间字符串*/
	char green_conflicts_time[17][100];

	/*绿冲突标志。0为无故障1为有故障*/
	int green_conflicts_flag;

	

	
	/*红绿冲突数组[16] 16个相位[2] : [0] 故障标志 故障 -1  正常0  [1] 故障时间*/
	int red_green_conflicts[17][2];

	/*红绿冲突时间数组[16] 16个相位[100]存放时间字符串*/
	char red_green_conflicts_time[17][100];

	/*红绿冲突标志。0为无故障1为有故障*/
	int red_green_conflicts_flag;

/*灯故障标志。0为无故障1为有故障*/
	int lamp_failures_flag;
		
	/*灯故障数组[16] 16个相位[2] : [0] 故障标志 故障 1  ,正常0  [1] 故障时间*/
	int lamp_failures[32][2];

	/*灯故障时间数组[16] 16个相位[100]存放时间字符串*/
	char lamp_failures_time[32][100];


	/*检测器故障数组[105] 104个检测器[2] : [0] 故障标志 故障 -1  正常0  [1] 故障时间*/
	int detector_failures[105][2];

	/*检测器故障时间数组[105] 104个检测器[100]存放时间字符串*/
	char detector_failures_time[105][100];


	/*检测板缺失数组[5] 4个检测板[2] : [0] 故障标志 故障 -1  正常0  [1] 故障时间*/
	int detector_loss[16][2];

	/*检测板缺失时间数组[5] 4个检测板[100]存放时间字符串*/
	char detector_loss_time[16][100];

	/*相位板缺失数组[5] 4个相位板[2] : [0] 故障标志 故障 -1  正常0  [1] 故障时间*/
	int phase_plate_loss[5][2];

	/*相位板缺失时间数组[5] 4个相位板[100]存放时间字符串*/
	char phase_plate_loss_time[5][100];


	/*检测器长时?洳欢魇閇105] 104个检测器[2] : [0] 故障标志 故障 -1  正常0  [1] 故障时间*/
	int detector_timeout[105][2];

	/*检测器长时间不动组[105] 104个检测器[100]存放时间字符串*/
	char detector_timeout_time[105][100];

	/*门状态数组[3] 2个门[2] : [0] 故障标志 故障 -1  正常0  [1] 故障时间*/
	int door_state[3][2];

	/*门状态数组时间数组[3] 2个门[100]存放时间字符串*/
	char door_state_time[3][100];

/************************************************************************************/	
	/*信号机是否 没控制是 0   手动控制是1 勤务是2   滤波是3 */
	int  signal_control_flag;

	/*控制心跳发送标志位*/
	int  control_heart_flag,control_d1_flag;

	/*信号机上一次控制0是没有3是滤波*/
	int  signal_last_control_flag;

	/*信号机上一次滤波的报文*/
	unsigned char last_C6_date_lin[85];	

	/*信号机发送控制心跳时长 0xffff为永久发送*/
	int  signal_control_heart_time;	

	/*信号机停止手动*/
	unsigned char stop_C0_date[14];	

/************************************************************************************/

	/*信号机最后一次被控制的时间*/
	long signal_control_time;

	/*检测器的流量占有率信息[105] 104个检测器 [3] [0 和1] 流量[2]占有率*/
	char signal_traffic_share[105][3];

	/*检测器流量占有率标志位，有1个加1，等于2则插入数据库*/
	int signal_traffic_share_flag[105];
	
};

/*信号机初始信息*/
extern signal_info signal_info_data[512];

/*信号机总个数*/
extern int signal_number;

/*数据库操作句柄*/
extern Statement *stmt ;

/*队列数据*/
extern sequeue *sq;




/*存放收到web界面的地址信息和报文*/
struct web_info
{
	struct sockaddr_in  peeraddr;	
	unsigned char recv_buf [512];
};

#endif

