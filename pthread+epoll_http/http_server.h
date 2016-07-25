#ifndef __HTTP_SERVER__
#define __HTTP_SERVER__ 
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<arpa/inet.h>
#include<netinet/in.h>
#include<sys/sendfile.h>
#include<sys/stat.h>
#include<pthread.h>
#include<fcntl.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<sys/epoll.h>
#include<errno.h>
#include "rio.h"

#define _BACKLOG_ 5
#define _MAXLINE_ 8196 //Linux下最大行长度 
#define _DEF_PAGE_ "index.html"

typedef struct path_env
{
	char method_env[256];
	char content_length_env[20];
	char query_string_env[256];
}path_e;

typedef struct http_head
{
	char url[256]; //请求地址
	char method[16]; //请求方法
	char version[16]; // 协议版本；
	char path[256]; //请求文件命（绝对路径）
	char* query_string;
	int cgi;
	int flag;
	char *rio_bufptr;
}hd_t,*hd_p;


void echo_err(int sock,int _errno);

void print_error(const char * const  str,const char * const func,int line);

int server_listen(char *ip,int port);


static void clear_head(int sock);
void echo_cgi(int sock,char *rio_ptr,const char* path,const char* method,char *query_string);
void echo_html(int sock,char *path,int  is_errno);
static int read_lineb(int fdsock,char *buf,ssize_t maxlen);

void http_start(int fdsock,hd_p http_head);
//void* handle_client(void *arg);
#endif 
