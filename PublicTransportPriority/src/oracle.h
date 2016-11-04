/*
 * oracle.h
 *
 *  Created on: 2016年10月9日
 *      Author: root
 */

#ifndef ORACLE_H_
#define ORACLE_H_

int OCCI_Open();
int GetConnectFromPool(Connection **new_conn,Statement **new_stmt);
void DisconnectOracle(Connection **_conn,Statement **_stmt);
int JudgeIsExist(char *sqlbuf);
int InitSignalInfo();
int InitDeviceInfo();
int InitDeviceStatus();

#endif /* ORACLE_H_ */
