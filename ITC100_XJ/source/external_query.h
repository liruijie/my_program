/*************************************************** 
 * �ļ�����external_query.h
 * ��Ȩ��
 * ���������ⲿ���µĲ�ѯ�ӿ�
 * �޸��ˣ�����
 * �޸�ʱ�䣺2014-1-10
 * �޸İ汾��V0.1
 * �޸����ݣ�
***************************************************/
#ifndef _WXTERNAL_QUERY_H_
#define _WXTERNAL_QUERY_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

void function_external_query();

int check_buf_external( unsigned char * rcv_buf);

int check_unid_id(int unid_id);

unsigned  char buf_check_external_num( unsigned  char* buf );



#endif

