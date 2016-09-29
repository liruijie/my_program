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

#define Test  test(7,'a','c');

void test(int x,char y ,char z)
{
	printf("%d\t%c\t%c\n",x,y,z);
}
void Print(char *p)
{
	//puts(p);
}
int main(void)
{
	int i =5;
	puts("!!!Hello World!!!"); /* prints !!!Hello World!!! */
	Test

	printf("%c",(i+0x30));
	return EXIT_SUCCESS;
}



