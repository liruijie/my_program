/*
 * oracle.h
 *
 *  Created on: 2016年10月9日
 *      Author: root
 */

#ifndef ORACLE_H_
#define ORACLE_H_

int OCCI_Open();
int GetConnectFromPool(Connection *new_conn,Statement *new_stmt);
int JudgeIsExist(char *sqlbuf);
int GetDeviceInfo();

#endif /* ORACLE_H_ */