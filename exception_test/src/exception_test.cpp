//============================================================================
// Name        : exception_test.cpp
// Author      : LIRUIJIE
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
using namespace std;
struct ex_test
{
		int code;
		char descr[200];
};


void exception_throw(int a,int b)
{
	struct ex_test *ex = (struct ex_test *)malloc(sizeof(ex_test));
	if(a==0)
	{

		ex->code=1;
		sprintf(ex->descr,"a is 0\n");
		throw(ex);
	}
	else if(a==1){

		ex->code=2;
		sprintf(ex->descr,"a is 1\n");
		throw(ex);
	}
	else
		printf("b/a = %d\n",b/a);
}

int main()
{
	cout << "!!!Hello World!!!" << endl; // prints !!!Hello World!!!

	try
	{
		exception_throw(3,6);
	}
	catch(ex_test *ex)
	{
		printf("ex.code = %d\nex.descr = %s",ex->code,ex->descr);
		free(ex);
	}
	try
	{
		exception_throw(0,6);
	}
	catch(ex_test *ex)
	{
		printf("ex.code = %d\nex.descr = %s",ex->code,ex->descr);
		free(ex);
	}
	try
	{
		exception_throw(1,6);
	}
	catch(ex_test *ex)
	{
		printf("ex.code = %d\nex.descr = %s",ex->code,ex->descr);
		free(ex);
	}

	getchar();

	return 0;
}
