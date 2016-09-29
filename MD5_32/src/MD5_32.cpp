//============================================================================
// Name        : MD5_32.cpp
// Author      : LIRUIJIE
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "md5.h"
using namespace std;

int main()
{
	 int i;
	unsigned char encrypt[] ="1";//21232f297a57a5a743894a0e4a801fc3
	unsigned char decrypt[16];
	char Hex32[32];
	memset(Hex32,'\0',32);
	MD5_CTX md5;
	MD5Init(&md5);
	MD5Update(&md5,encrypt,strlen((char *)encrypt));
	MD5Final(&md5,decrypt);
	char buf[2];
	printf("加密前:%s\n加密后:",encrypt);
	for(i=0;i<16;i++)
	{
		printf("%02x",decrypt[i]);
		sprintf(buf,"%02x",decrypt[i]);
		strncat(Hex32,buf,2);
	}

	puts(Hex32);
	getchar();


	return 0;
}
