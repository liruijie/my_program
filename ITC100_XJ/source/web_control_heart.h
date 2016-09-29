/*************************************************** 
 * 文件名：web_control_heart.h
 * 版权：
 * 描述：接收WEB 界面发送来的控制心跳
 * 修改人：栾宇
 * 修改时间：2014-1-10
 * 修改版本：V0.1
 * 修改内容：
***************************************************/
#ifndef _WEB_CONTROL_HEART_H_
#define _WEB_CONTROL_HEART_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <errno.h>

void revice_web_control_heart();
int check_buf_web( unsigned char * rcv_buf);


#endif

