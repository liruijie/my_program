/*************************************************** 
 * �ļ�����occi_opration.h
 * ��Ȩ��
 * ������oracle ���ݿ����ͷ�ļ�
 * �޸��ˣ�����
 * �޸�ʱ�䣺2014-1-6
 * �޸İ汾��V0.1
 * �޸����ݣ�
***************************************************/
#ifndef _OCCI_OPRATION_H_
#define _OCCI_OPRATION_H_

#include <iostream>
#include <string.h>
#include <occi.h>
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <errno.h>

using namespace oracle::occi;

using namespace std;

int  occi_open(void);

int occi_singal_Init(void);

int occi_singal_Init_alalrm(void);

int occi_singal_Init_control_heart(void);

#endif

