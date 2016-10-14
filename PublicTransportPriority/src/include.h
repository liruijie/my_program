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
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <errno.h>

#include <occi.h>
#include "iconv.h"

using namespace oracle::occi;
using namespace std;

#include "oracle.h"
#include "CommunicateWithDevice.h"
#include "PublicDefine.h"
#include "Queue.h"
#include "CodeConvert.h"
#include "CommunicateWithWeb.h"

#endif /* INCLUDE_H_ */
