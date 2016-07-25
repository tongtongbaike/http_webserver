//httpserver设计计划
//实现基础的http请求行分析,返回对请的请求处理,cgi编程。。。。。。。。OK
//实现mysql数据库的消息处理回显处理，了解数据库的底层实现 。。。。。。。。。OK
//实现epoll多路复用版本,简单剖析epoll实现机制及源码...................OK
//实现epoll/多线程版本，分流消息处理/消息回显.................
//实现rio,

#include "http_server.h"

#define _MAX_FD_NUM_ 64

typedef struct data_buf
{
	int fd;
	char buf[1024];
}data_buf_t,*data_buf_p;

static int set_non_block(int fd)
{
	int old_fl = fcntl(fd,F_GETFL);
	if(old_fl < 0)
	{
		perror("fcntl");
		return -1;
	}
	if(fcntl(fd,F_SETFL,old_fl |O_NONBLOCK))
	{
		perror("fcntl");
		return -1;
	}
	return 0;
}
static int epoll_server(int sock)
{
	hd_t http_head;
	int epoll_fd = epoll_create(256);
	if(epoll_fd < 0)
	{
		perror("epoll_create");
		exit(4);
	}
	
//	printf("epoll_create success\n");
	struct epoll_event ev;
	ev.events = EPOLLIN |EPOLLET;
	ev.data.fd = sock;
	
	if(epoll_ctl(epoll_fd,EPOLL_CTL_ADD,sock,&ev) < 0)
	{
		perror("epoll_ctl");
		exit(5);
	}

	//设置一个输出的参数数组;
	struct epoll_event ev_out[_MAX_FD_NUM_];
	
	int max = _MAX_FD_NUM_;
	int timeout = 5000;
	int num = -1;
	int i = 0;
	int done = 0;
	data_buf_p mem = (data_buf_p)malloc(sizeof(data_buf_t));
	while(!done)
	{
		 //switch(num = epoll_wait(epoll_fd,ev_out,max,timeout))
		
		num = epoll_wait(epoll_fd,ev_out,max,timeout);
		switch(num)
		{
			case 0://timeout
				break;
			case -1:
				perror("epoll_wait");
				break;
			default:
				{
					for(i = 0; i < num;++i)
					{
						if((ev_out[i].data.fd == sock) && (ev_out[i].events & (EPOLLIN | EPOLLET)))
						{
							struct sockaddr_in client;
							socklen_t len = sizeof(client);

							int fd = ev_out[i].data.fd;
							int newsock = accept(fd,(struct sockaddr*)&client,&len);
							if(newsock < 0)
							{
								perror("newsock");
								continue;	
							}
							int err = set_non_block(newsock);
							if(err <  0)
							{
								printf("non_block error\n");
								close(newsock);
								continue;
							}
							ev.events = EPOLLIN | EPOLLET;
							ev.data.fd = newsock;
							epoll_ctl(epoll_fd,EPOLL_CTL_ADD,newsock,&ev);
						//	printf("get a new connect\n");

						}
						else if(ev_out[i].events & (EPOLLIN |EPOLLET))
						{
						//	printf("join read\n");
							int fd = ev_out[i].data.fd;
							mem->fd = fd;
						//	printf("1\n");
							http_start(mem->fd,&http_head);
							ev.events = EPOLLOUT | EPOLLET;
							ev.data.ptr = mem;
							epoll_ctl(epoll_fd,EPOLL_CTL_MOD,fd,&ev);
						//	printf("change fd success EPOLLOUT\n");
						}
						else if (ev_out[i].events & (EPOLLOUT |EPOLLET))
						{
							data_buf_p mem = (data_buf_p)ev_out[i].data.ptr;
							int fd = mem->fd;
							
							if(http_head.cgi == 1)
							{
								echo_cgi(fd,http_head.path,http_head.method,http_head.query_string);	
							}
							else
							{
								fflush(stdout);
								echo_html(fd,http_head.path,0);
							}
							
							ev.events = EPOLLIN | EPOLLET;
							ev.data.fd = mem->fd;
							epoll_ctl(epoll_fd,EPOLL_CTL_MOD,fd,&ev);
						//	printf("echo write success,change fd EPOLLIN\n");
							close(fd);
						}
						else
						{
								
						}
					}
				}
				break;
		}
	}
}


int main(int argc,char *argv[])
{
//	daemon(0,0);
	char* ip = argv[1];
	int port = atoi(argv[2]);
	
	int listen_sock = server_listen(ip,port);

	//struct sockaddr_in client;
	//socklen_t len = sizeof(client); 

	//while (1)
	//{
	//	int accept_sock = accept(listen_sock,(struct sockaddr*)&client,&len);
	//	if(accept_sock < 0)
	//	{
	//		print_error(strerror(errno),__FUNCTION__,__LINE__);
	//	}	

	//	pthread_t tid;
	//	pthread_create(&tid,NULL,handle_client,(void *)accept_sock);
	//	pthread_detach(tid);
	//}
	epoll_server(listen_sock);
	close(listen_sock);


	return 0;
}
