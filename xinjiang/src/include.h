/*
 * include.h
 *
 *  Created on: 2016年7月4日
 *      Author: root
 */

#ifndef INCLUDE_H_
#define INCLUDE_H_

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <fcntl.h>
#include <wait.h>
#include <time.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <errno.h>

#include <libxml/parser.h>
#include <libxml/xmlreader.h>
#include <libxml/tree.h>
#include <libxml/xmlmemory.h>
#include <libxml/xmlerror.h>
#include <libxml/xpath.h>
#include <occi.h>

using namespace oracle::occi;
using namespace std;

#include "publicdefine.h"
#include "oracle.h"
#include "ControlCenterCommt.h"
#include "xml.h"
#include "ITC_Server.h"
#include "md5.h"
#include "publicfunction.h"
#include "XmlRequest.h"
#include "Operation_Set.h"
#include "Operation_Get.h"
#include "ReportRealStatus.h"
#include "ControlSignal.h"
#include "KeepConnectAlive.h"


#endif /* INCLUDE_H_ */
