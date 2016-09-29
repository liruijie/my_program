/*************************************************** 
 * 文件名：web_message_recv.h
 * 版权：
 * 描述：接收WEB 界面发送来的处理请求
 * 修改人：栾宇
 * 修改时间：2014-1-10
 * 修改版本：V0.1
 * 修改内容：
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

