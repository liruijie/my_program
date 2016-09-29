/*
 * xml.h
 *
 *  Created on: 2016年7月7日
 *      Author: root
 */

#ifndef XML_H_
#define XML_H_


#define Login			1
#define Logout			2
#define Get				3
#define Set				4
#define Subscribe		5
#define Unsubscribe	6
#define Notify			7
#define Other			8




int ReadXmlFromBuf(int ThreadNum);
int OutputXml();
void AnalyseXml(xmlDocPtr pdoc,int ThreadNum);
void TraversalNode(xmlNodePtr cur);
int CheckXmlResult(xmlXPathObjectPtr Xresult);
int IsTokenValid(xmlXPathContextPtr Xpath,int ThreadNum);
int IsUserLogin(xmlXPathContextPtr Xpath,int ThreadNum);
int LoginLogout_Response(int ThreadNum);
int SwitchFromTo(xmlXPathContextPtr Xpath);
int GetOperateType(char *type,xmlXPathContextPtr Xpath);



#endif /* XML_H_ */
