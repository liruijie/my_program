/*************************************************** 
 * �ļ�����web_message_recv.h
 * ��Ȩ��
 * ����������WEB ���淢�����Ĵ�������
 * �޸��ˣ�����
 * �޸�ʱ�䣺2014-1-10
 * �޸İ汾��V0.1
 * �޸����ݣ�
***************************************************/
#ifndef _WEB_MESSAGE_RECV_H_
#define _WEB_MESSAGE_RECV_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>


void revice_web_message(void);

int revice_web_message_opretion(   unsigned  char* buf );

void all_send_signal_message_info(void);

unsigned  char buf_check_num( unsigned  char* buf );

#endif

