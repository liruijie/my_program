/*************************************************** 
 * �ļ�����signal_report.h
 * ��Ȩ��
 * �����������źŻ������ϱ���Ϣ��ͷͷ�ļ�
 * �޸��ˣ�����
 * �޸�ʱ�䣺2014-1-6
 * �޸İ汾��V0.1
 * �޸����ݣ�
***************************************************/
#ifndef _SINNAL_REPORT_H_
#define _SINNAL_REPORT_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>


void receive_signal_report();

void receive_signal_report_udp_server(int port);

int check_buf(unsigned char * buf);

int check_signal_id(int signal_id);

int parsing_recv_buf(int signal_num, unsigned char * buf);
#endif

