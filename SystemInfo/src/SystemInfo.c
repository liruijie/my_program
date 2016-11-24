/*
 ============================================================================
 Name        : SystemInfo.c
 Author      : LIRUIJIE
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

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
#include <sys/prctl.h>
#include <linux/hdreg.h>
#include <sys/ioctl.h>


 int main()
 {
	struct hd_driveid id;
	int fd = open("/dev/sda2", O_RDONLY|O_NONBLOCK);
	if (fd < 0)
	{
		perror("/dev/sda");
		return 1;
	}
	if(!ioctl(fd, HDIO_GET_IDENTITY, &id))
	{
		printf("Serial Number=%s\n",id.serial_no);
	}
	close(fd);
	sleep(1);
	getchar();
	return 0;
 }
