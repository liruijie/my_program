/*************************************************** 
 * 文件名：occi_opration.h
 * 版权：
 * 描述：oracle 数据库操作头文件
 * 修改人：栾宇
 * 修改时间：2014-1-6
 * 修改版本：V0.1
 * 修改内容：
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

