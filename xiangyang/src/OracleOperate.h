/*
 * OracleOperate.h
 *
 * Created on: 2016年11月30日
 * Author: LIRUIJIE
 */

#ifndef ORACLEOPERATE_H_
#define ORACLEOPERATE_H_

int occi_open();
int Init_SignalInfo();
int GetConnectFromPool(Connection **new_conn,Statement **new_stmt);
void DisconnectOracle(Connection **_conn,Statement **_stmt);
int ExcuteSql(char *sqlbuf);
#endif /* ORACLEOPERATE_H_ */
