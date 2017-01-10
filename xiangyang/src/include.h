/*
 * include.h
 *
 * Created on: 2016年11月30日
 * Author: LIRUIJIE
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
#include <sys/socket.h>
#include <sys/wait.h>
#include <sys/ioctl.h>
#include <sys/prctl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <ifaddrs.h>
#include <net/if.h>
#include <errno.h>
#include <linux/hdreg.h>

#include <libxml/parser.h>
#include <libxml/xmlreader.h>
#include <libxml/tree.h>
#include <libxml/xmlmemory.h>
#include <libxml/xmlerror.h>
#include <libxml/xpath.h>
#include <occi.h>
using namespace oracle::occi;
using namespace std;

#include "PublicDefine.h"
#include "OracleOperate.h"
#include "Queue.h"
#include "CommunicateWithServer.h"
#include "CommunicateWithWeb.h"
#include "GetFlowData.h"

#endif /* INCLUDE_H_ */
