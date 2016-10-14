/*
 * CodeConvert.cpp
 *
 * Created on: 2016年10月11日
 * Author: LIRUIJIE
 */
/*
 ============================================================================
 Name        : iconv_test.c
 Author      : LIRUIJIE
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include "include.h"

//int UTF8_2_GBK(char *inbuf,int inlen,char *outbuf, int outlen);
//int u2g(char *inbuf,int inlen,char *outbuf, int outlen);

/*int main(void)
{
	puts("!!!Hello World!!!");  prints !!!Hello World!!!
	char chinese[]="运通105";
	unsigned char OutText[MaxLen*2];
	int OutLen;
	unsigned char OutText1[MaxLen*2];
	//int OutLen1;
	unsigned char Text_ASC[MaxLen*4];
	int i;
	OutLen = UTF8_2_GBK((char *)chinese,sizeof(chinese),(char *)(OutText),MaxLen*2);
	printf("OutLen = %d\n",OutLen);
	for(i = 0; i<OutLen;i++)
	{
		printf("%#02X\t",OutText[i]);
	}
	printf("\n\n");
	u2g((char *)OutText,OutLen,(char *)(OutText1),MaxLen*2);
	puts((char *)OutText1);
		printf("\n\n\n");
	//memset(Text_ASC,'\0',MaxLen*4);
	ConverHex2ASC(OutText,OutLen,Text_ASC,MaxLen*4);
	printf("%s\n",Text_ASC);
	return EXIT_SUCCESS;
}*/
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

/*int u2g(char *inbuf,int inlen,char *outbuf, int outlen)
{
	return code_convert((char *)"GBK",(char *)"UTF-8",inbuf,inlen,outbuf,outlen);
}

int UTF8_2_GBK(char *inbuf,int inlen,char *outbuf, int outlen)
{
	return code_convert((char *)"UTF-8",(char *)"GBK",inbuf,inlen,outbuf,outlen);
}*/








