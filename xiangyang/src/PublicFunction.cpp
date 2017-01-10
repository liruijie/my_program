/*
 * PublicFunction.cpp
 *
 * Created on: 2016年11月30日
 * Author: LIRUIJIE
 */
//获取序列号
#include "include.h"
unsigned long SeqNum=0;
void GetSeq(char *seqbuf)
{
	//Seq	序列号，格式：yyyyMMddHHmmss+6位顺序号
	time_t time_now;
	struct tm* TimeInfo;
	time(&time_now);
	TimeInfo = localtime(&time_now);
	sprintf(seqbuf,"%d%02d%02d%02d%02d%02d%06ld",1900+TimeInfo->tm_year,TimeInfo->tm_mon+1,
			TimeInfo->tm_mday,TimeInfo->tm_hour,TimeInfo->tm_min,TimeInfo->tm_sec,(SeqNum++));
	SeqNum = SeqNum % 1000000;
}


