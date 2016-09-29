/*************************************************** 
 * 文件名：signal_report.h
 * 版权：
 * 描述：接收信号机主动上报信息接头头文件
 * 修改人：栾宇
 * 修改时间：2014-1-6
 * 修改版本：V0.1
 * 修改内容：
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

