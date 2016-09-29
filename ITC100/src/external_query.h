/*************************************************** 
 * 文件名：external_query.h
 * 版权：
 * 描述：给外部留下的查询接口
 * 修改人：栾宇
 * 修改时间：2014-1-10
 * 修改版本：V0.1
 * 修改内容：
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

