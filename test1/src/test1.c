/*
 ============================================================================
 Name        : test.c
 Author      : LIRUIJIE
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>

int main(void)
{

	puts("!!!Hello World!!!"); /* prints !!!Hello World!!! */

	unsigned int a = 6;
	int b = -20;
	int c=a+b;
	unsigned int d= a+b;
	(a+b > 6) ? puts("> 6") : puts("<= 6");
	printf("c=%d,d=%u\n",c,d);
	return EXIT_SUCCESS;
}



