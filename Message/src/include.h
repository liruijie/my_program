/*
 * include.h
 *
 *  Created on: 2016-6-24
 *      Author: root
 */

#ifndef INCLUDE_H_
#define INCLUDE_H_


#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <termios.h>
#include <unistd.h>
#include <signal.h>
#include "iconv.h"
#include <stdbool.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#define MaxTextLen    250


bool InitDevice();
bool CreateUdpToModem();
int RecvFromModem( char *rcv_buf,int time_s);
int SendToModem(char *send_buf,int data_len);
bool SendMessage(unsigned char *PhoneNum,unsigned char *Text);

int u2g(char *inbuf,int inlen,char *outbuf, int outlen);
int g2u(char *inbuf,int inlen,char *outbuf, int outlen);
int code_convert(char *from_charset,char *to_charset,char *inbuf,size_t inlen,char *outbuf,size_t outlen);
int ConverHex2ASC(unsigned char *Bin, int BinLen, unsigned char *Hex, int HexLen);
int Hex2ASC(unsigned char Bin, unsigned char *Hex);
int SendPhoneNum(unsigned char *PhoneNum);
int SendText(unsigned char *Text);

/*bool OpenSerial();
int SetSerialOpt(int fd,int nSpeed, int nBits, char nEvent, int nStop);
int UART_Recv(int fd, char *rcv_buf,int time_s);
int UART_Send(int fd, char *send_buf,int data_len);*/
#endif /* INCLUDE_H_ */
