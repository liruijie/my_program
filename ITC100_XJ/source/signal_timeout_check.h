/*************************************************** 
 * 文件名：signal_timeout_check.h
 * 版权：
 * 描述：处理检查信号机在线离线状态
 * 修改人：栾宇
 * 修改时间：2014-1-7
 * 修改版本：V0.1
 * 修改内容：
***************************************************/
#ifndef _SIGNAL_TIMEOUT_CHECK_H_
#define _SIGNAL_TIMEOUT_CHECK_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>


void signal_timeout_check();
void signal_control_state_timeout_check();

#endif

