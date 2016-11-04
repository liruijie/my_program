/*
 * include.h
 *
 *  Created on: 2016年10月9日
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
#include <sys/socket.h>
#include <sys/wait.h>
#include <sys/ioctl.h>
#include <sys/prctl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <linux/hdreg.h>

#include <libxml/parser.h>
#include <libxml/xmlreader.h>
#include <libxml/tree.h>
#include <libxml/xmlmemory.h>
#include <libxml/xmlerror.h>
#include <libxml/xpath.h>
#include <occi.h>
#include "iconv.h"

using namespace oracle::occi;
using namespace std;

#include "oracle.h"
#include "CommunicateWithDevice.h"
#include "PublicDefine.h"
#include "Queue.h"
#include "CommunicateWithWeb.h"
#include "CommunicateWithHiCON.h"


#endif /* INCLUDE_H_ */
