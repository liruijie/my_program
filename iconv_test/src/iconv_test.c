/*
 ============================================================================
 Name        : iconv_test.c
 Author      : LIRUIJIE
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "iconv.h"
#define MaxLen 300
int UTF8_2_GBK(char *inbuf,int inlen,char *outbuf, int outlen);
int u2g(char *inbuf,int inlen,char *outbuf, int outlen);
int code_convert(char *from_charset,char *to_charset,char *inbuf,size_t inlen,char *outbuf,size_t outlen);
int ConverHex2ASC(unsigned char *Bin, int BinLen, unsigned char *Hex, int HexLen);
int Hex2ASC(unsigned char Bin, unsigned char *Hex);
int main(void)
{
	puts("!!!Hello World!!!"); /* prints !!!Hello World!!! */
	char chinese[]="运通105";


	char tempbuf[100];

	unsigned char OutText[MaxLen*2];
	int OutLen;
	unsigned char OutText1[MaxLen*2];
	//int OutLen1;
	unsigned char Text_ASC[MaxLen*4];
	int i;

	printf(chinese);
	printf("\n");
	for(i=0; i< strlen(chinese);i++)
		printf("%#02X\t",chinese[i]&0xFF);
	printf("\ni=%d\n",i);
	memcpy(tempbuf,chinese,strlen(chinese));

	OutLen = UTF8_2_GBK((char *)chinese,strlen(chinese),(char *)(OutText),MaxLen*2);
	printf("OutLen = %d\n",OutLen);
	for(i = 0; i<OutLen;i++)
	{
		printf("%#X\t",OutText[i]);
	}
	printf("\n\n");
	u2g((char *)OutText,OutLen,(char *)(OutText1),MaxLen*2);
	puts((char *)OutText1);
		printf("\n\n\n");
	//memset(Text_ASC,'\0',MaxLen*4);
	ConverHex2ASC(OutText,OutLen,Text_ASC,MaxLen*4);
	printf("%s\n",Text_ASC);
	return EXIT_SUCCESS;
}
int code_convert(char *from_charset,char *to_charset,char *inbuf,size_t inlen,char *outbuf,size_t outlen)
{
	iconv_t cd;
	//size_t rc;

	size_t outlenleft = outlen;
	char **pin = &inbuf;
	char **pout = &outbuf;
	cd = iconv_open(to_charset,from_charset);
	if (cd==0) return -1;
	memset(outbuf, 0, outlen);
	if(iconv(cd,pin,&inlen,pout,&outlenleft) == 0xFF)
	{
		iconv_close(cd);
		return -1;
	}
	iconv_close(cd);
	return (int)(outlen-outlenleft);
}

int u2g(char *inbuf,int inlen,char *outbuf, int outlen)
{
	return code_convert((char *)"GBK",(char *)"UTF-8",inbuf,inlen,outbuf,outlen);
}

int UTF8_2_GBK(char *inbuf,int inlen,char *outbuf, int outlen)
{
	return code_convert((char *)"UTF-8",(char *)"GBK",inbuf,inlen,outbuf,outlen);
}


int ConverHex2ASC(unsigned char *Hex, int HexLen, unsigned char *ASC, int ASCLen)
{
	int i;
	if(!Hex || !ASC)
	{
		return -1;
	}
	if(HexLen > ASCLen / 2)
	{
		return -1;
	}
	for(i=0; i<HexLen; i ++)
	{
		/*if(Hex2ASC(*(Hex + i+1), ASC + 2*i)!=0)
		{
			return -1;
		}
		if(Hex2ASC(*(Hex + i), ASC + 2*(i+1))!=0)
		{
			return -1;
		}*/

		if(Hex2ASC(*(Hex + i), ASC + 2*i)!=0)
		{
			return -1;
		}
	}
	return 0;
}

int Hex2ASC(unsigned char Hex, unsigned char *ASC)
{
	switch(Hex >> 4)
	{
		case 0: case 1: case 2: case 3:
		case 4: case 5: case 6: case 7:
		case 8: case 9:
			*ASC = (Hex>>4) + '0';
			break;
		case 0xa: case 0xb: case 0xc:
		case 0xd: case 0xe: case 0xf:
			*ASC = (Hex>>4) -0xA + 'A';
			break;
	}
	switch(Hex & 0xF)
	{
		case 0: case 1: case 2: case 3:
		case 4: case 5: case 6: case 7:
		case 8: case 9:
			*(ASC+1) = (Hex & 0xF) + '0';
			break;
		case 0xa: case 0xb: case 0xc:
		case 0xd: case 0xe: case 0xf:
			*(ASC+1) = (Hex & 0xF) - 0xA + 'A';
			break;
	}
	return 0;
}
