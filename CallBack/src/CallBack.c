/*
 ============================================================================
 Name        : CallBack.c
 Author      : LIRUIJIE
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>

struct event_cb;

typedef void (*ptFun_Event)(const struct event_cb *evt, void *user_data);

struct event_cb
{
	ptFun_Event cb_func;
    void *data;
};

static struct event_cb saved = { 0, 0 };

void event_cb_register(ptFun_Event cb, void *user_data)
{
    saved.cb_func = cb;
    saved.data = user_data;
}

static void my_event_cb(const struct event_cb *evt, void *data)
{
    printf("in %s\n", __func__);
    printf("data1: %s\n", (const char *)data);
    printf("data2: %s\n", (const char *)evt->data);
}

int main(void)
{
    char my_custom_data[40] = "Hello!";
    event_cb_register(my_event_cb, my_custom_data);

    saved.cb_func(&saved, saved.data);

    return 0;
}
