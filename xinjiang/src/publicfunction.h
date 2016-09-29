/*
 * publicfunction.h
 *
 *  Created on: 2016年7月8日
 *      Author: root
 */

#ifndef PUBLICFUNCTION_H_
#define PUBLICFUNCTION_H_

int IsUserValid(char *UserName,char *PassWd);
void UpdateToken(char *Token);
int Get_Object_ID(xmlNodePtr OperationNode);
int Get_Node(xmlNodePtr Node,char *NodeName);
int request_error(int ret,xmlXPathContextPtr Xpath,struct xmlResponseError ErrorInfo);
int LoginLogout_Error(int ret,int ThreadNum);
#endif /* PUBLICFUNCTION_H_ */
