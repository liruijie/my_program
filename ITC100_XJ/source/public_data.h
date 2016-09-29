/*************************************************** 
 * �ļ�����public_data.h
 * ��Ȩ��
 * ��������¼���е�ȫ�ֱ���
 * �޸��ˣ�����
 * �޸�ʱ�䣺2014-1-6
 * �޸İ汾��V0.1
 * �޸����ݣ�
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
#include <unistd.h>
#include <errno.h>
#include "queue.h"

using namespace oracle::occi;
using namespace std;



extern  string oracle_username ;
extern string oracle_password ;
extern string oracle_connectstring;

 //string oracle_file_path = "/home/TC100_SERVER/confige";



/*�źŻ���ʼ��Ϣ�ṹ��*/
struct signal_info
{
	/*�źŻ�ID*/
	int signal_id;

	/*�źŻ�IP*/
	char signal_ip[16];

	/*�źŻ��˿�*/
	int signal_port;

	/*·��ID*/
	int unit_id;

	/*�źŻ�����: TC100  /  ITC100*/
	char device_type[10];
	
	/*�źŻ�ʵʱ״̬��Ϣ*/
	char recv_buf[512];
	
	/*�źŻ���������ϱ�ʱ��*/
	long last_time;
	

	/*�źŻ����һ�α���û�յ�Ϊ0  �յ�δ1*/
	int signal_info_state;
	
	/*�źŻ�����״̬ 0���� 1����*/
	int signal_state;
	
	/*�źŻ���ʱ���߹���1����0�޹���*/
	int l_online_alarm;
	
	/*�źŻ���ʱ���߹���ʱ��*/
	char l_online_alarm_time[100];

	/*дoracle���ݿ�ı�־λ��0����д��1 д�����յ�״̬����ʱ��ֵΪ1*/
	int oracleFlag[2];

	
	/*�źŻ�������״̬*/
	char board_state [54];
	
	/*�źŻ���λ��̬*/
	char phase_lht[100];

	/*�źŻ���λ����*/
	char phase_current[400];

	/*�źŻ������״̬*/
	char detect_status[210];

	/*�źŻ����������*/
	char detect_data_status[210];

	/*�źŻ���������״̬*/
	char detect_urgent_status[10];

	/*�źŻ���������״̬*/
	char detect_pri_status[10];

	/*�źŻ���״̬*/
	char door_status[8];

	/*�źŻ�����ʱ��*/
	char update_time[21];


	/*�źŻ���һ��ģʽ*/
	int control_model;
		
	/*�źŻ���ǰģʽ*/
	int control_last_model;
	
	
	/*�źŻ���ǰ����*/
	int cur_plan;

	/*�źŻ���ǰ����*/
	int cur_week;

	/*�źŻ���ǰ�׶�*/
	int cur_stage;

	/*�źŻ�Э�����*/
	int coordination_number;
	
	/*�źŻ�Э��ʱ��*/
	int coordination_time;
	
	/*�źŻ�����ʱ��*/
	int release_time;
	
	/*�źŻ�ʣ��ʱ��*/
	int rest_time;

	

	/*�̳�ͻ����[16] 16����λ[2] : [0] ���ϱ�־ ���� -1  ����0  [1] ����ʱ��*/
	int green_conflicts[17][2];

	/*�̳�ͻʱ������[16] 16����λ[100]���ʱ���ַ���*/
	char green_conflicts_time[17][100];

	/*�̳�ͻ��־��0Ϊ�޹���1Ϊ�й���*/
	int green_conflicts_flag;

	

	
	/*���̳�ͻ����[16] 16����λ[2] : [0] ���ϱ�־ ���� -1  ����0  [1] ����ʱ��*/
	int red_green_conflicts[17][2];

	/*���̳�ͻʱ������[16] 16����λ[100]���ʱ���ַ���*/
	char red_green_conflicts_time[17][100];

	/*���̳�ͻ��־��0Ϊ�޹���1Ϊ�й���*/
	int red_green_conflicts_flag;

/*�ƹ��ϱ�־��0Ϊ�޹���1Ϊ�й���*/
	int lamp_failures_flag;
		
	/*�ƹ�������[16] 16����λ[2] : [0] ���ϱ�־ ���� 1  ,����0  [1] ����ʱ��*/
	int lamp_failures[32][2];

	/*�ƹ���ʱ������[16] 16����λ[100]���ʱ���ַ���*/
	char lamp_failures_time[32][100];


	/*�������������[105] 104�������[2] : [0] ���ϱ�־ ���� -1  ����0  [1] ����ʱ��*/
	int detector_failures[105][2];

	/*���������ʱ������[105] 104�������[100]���ʱ���ַ���*/
	char detector_failures_time[105][100];


	/*����ȱʧ����[5] 4������[2] : [0] ���ϱ�־ ���� -1  ����0  [1] ����ʱ��*/
	int detector_loss[16][2];

	/*����ȱʧʱ������[5] 4������[100]���ʱ���ַ���*/
	char detector_loss_time[16][100];

	/*��λ��ȱʧ����[5] 4����λ��[2] : [0] ���ϱ�־ ���� -1  ����0  [1] ����ʱ��*/
	int phase_plate_loss[5][2];

	/*��λ��ȱʧʱ������[5] 4����λ��[100]���ʱ���ַ���*/
	char phase_plate_loss_time[5][100];


	/*�������ʱ?䲻��������[105] 104�������[2] : [0] ���ϱ�־ ���� -1  ����0  [1] ����ʱ��*/
	int detector_timeout[105][2];

	/*�������ʱ�䲻����[105] 104�������[100]���ʱ���ַ���*/
	char detector_timeout_time[105][100];

	/*��״̬����[3] 2����[2] : [0] ���ϱ�־ ���� -1  ����0  [1] ����ʱ��*/
	int door_state[3][2];

	/*��״̬����ʱ������[3] 2����[100]���ʱ���ַ���*/
	char door_state_time[3][100];

/************************************************************************************/	
	/*�źŻ��Ƿ� û������ 0   �ֶ�������1 ������2   �˲���3 */
	int  signal_control_flag;

	/*�����������ͱ�־λ*/
	int  control_heart_flag,control_d1_flag;

	/*�źŻ���һ�ο���0��û��3���˲�*/
	int  signal_last_control_flag;

	/*�źŻ���һ���˲��ı���*/
	unsigned char last_C6_date_lin[85];	

	/*�źŻ����Ϳ�������ʱ�� 0xffffΪ���÷���*/
	int  signal_control_heart_time;	

	/*�źŻ�ֹͣ�ֶ�*/
	unsigned char stop_C0_date[14];	

/************************************************************************************/

	/*�źŻ����һ�α����Ƶ�ʱ��*/
	long signal_control_time;

	/*�����������ռ������Ϣ[105] 104������� [3] [0 ��1] ����[2]ռ����*/
	char signal_traffic_share[105][3];

	/*���������ռ���ʱ�־λ����1����1������2��������ݿ�*/
	int signal_traffic_share_flag[105];

	/*���ӿ��ϱ�ʵʱ���ݱ�־λ*/
	int TJ_API_Report_Real_Data_Flag; 
	/*���յ���ʵʱ���ݱ���  */
	char Real_Data_Backup[1000];
	int Real_Data_Len;	

	char volume_backup[512];
	int volume_data_len;
	bool volume_flag;

	char occu_backup[512];
	int occu_data_len;
	bool occu_flag;
	
	char fault_backup[512];
	int fault_data_len;
	bool fault_flag;


};

/*�źŻ���ʼ��Ϣ*/
extern signal_info signal_info_data[512];

/*�źŻ��ܸ���*/
extern int signal_number;

/*���ݿ�������*/
extern Statement *stmt ;

/*��������*/
extern sequeue *sq;




/*����յ�web����ĵ�ַ��Ϣ�ͱ���*/
struct web_info
{
	struct sockaddr_in  peeraddr;	
	unsigned char recv_buf [512];
};

#endif

