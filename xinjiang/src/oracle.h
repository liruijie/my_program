/*
 * oracle.h
 *
 *  Created on: 2016年7月4日
 *      Author: root
 */

#ifndef ORACLE_H_
#define ORACLE_H_



int occi_open();
int JudgeIsExist(char *sqlbuf);
int Init_SignalInfo();

#endif /* ORACLE_H_ */
