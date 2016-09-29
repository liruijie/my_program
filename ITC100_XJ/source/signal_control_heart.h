/*************************************************** 
 * 文件名：signal_control_heart.h
 * 版权：
 * 描述：发送到信号机的控制心跳
 * 修改人：栾宇
 * 修改时间：2014-1-19
 * 修改版本：V0.1
 * 修改内容：
***************************************************/
#ifndef _SIGNAL_CONTROL_HEART_H_
#define _SIGNAL_CONTROL_HEART_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>


void send_signal_control_heart();

void *signal_control_heart(void * arg);

#endif

