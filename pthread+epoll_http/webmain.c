//httpserver设划
//实现基础的http请求行分析,返回对请的请求处理,cgi编程。。。。。。。。OK
//实现mysql数据库的消息处理回显处理，了解数据库的底层实现 。。。。。。。。。OK
//实现epoll多路复用版本,简单剖析epoll实现机制及源码...................OK
//实现epoll/多线程版本，分流消息处理/消息回显.................设计思路错误，无法实现accept并发。任然需要继续学习，暂时放弃，先剖析nginx

//实现rio......................................ok
//
#include "http_server.h"

#define _MAX_FD_NUM_ 64

typedef struct data_buf
{
	int fd;
	char buf[1024];
}data_buf_t,*data_buf_p;

typedef struct fd_sum
{
	int _sock_fd;
	int _epoll_fd;
	struct epoll_event* _ev_arry;
}fd_s;

typedef struct deal_head
{
	int _sock_fd;
	hd_p _head;
}d_head;

typedef	struct epoll_event ep_e;

d_head read_head_arry[_MAX_FD_NUM_];
struct epoll_event ev_out[_MAX_FD_NUM_];

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

void *in_deal(void *arg)
{

	d_head* _tmp = (d_head*)arg;
//	printf("pthread_a[i]:%s\n",_tmp->_head->method);
	http_start(_tmp->_sock_fd,_tmp->_head);
//	printf("pthread_a[i]:%s\n",_tmp->_head->method);
	return NULL;
}

void *out_deal_cgi(void *arg)
{
	d_head* _tmp = (d_head*)arg;
	echo_cgi(_tmp->_sock_fd,_tmp->_head->rio_bufptr,_tmp->_head->path,_tmp->_head->method,_tmp->_head->query_string);
	return NULL;
}

void *out_deal_uncgi(void *arg)
{
	fflush(stdout);
	d_head* _tmp = (d_head*)arg;
	echo_html(_tmp->_sock_fd,_tmp->_head->path,0);
//	printf("re_h_a[i]:%s\n",_tmp->_head->method);
//	printf("re_h_a[i]:%s\n",_tmp->_head->path);
//	printf("re_h_a[i]:%s\n",_tmp->_head->url);
//	printf("re_h_a[i]:%s\n",_tmp->_head->cgi);
	return NULL;
}
void *epoll_wait_add(void *arg)
{
	//pthread_detach(pthread_self());
	fd_s* _tmp = (fd_s *)arg;
	ep_e ev;
	struct sockaddr_in client;
	socklen_t len = sizeof(client);
	while(1)
	{
		int newsock = accept(_tmp->_sock_fd,(struct sockaddr*)&client,&len);
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
//		printf("events%d\n",ev.events);
		ev.data.fd = newsock;
		epoll_ctl(_tmp->_epoll_fd,EPOLL_CTL_ADD,newsock,&ev);

	}
	return NULL;
}

void* read_deal(void *arg)
{
	
	fd_s* _tmp = (fd_s *)arg;
	ep_e ev;
	int timeout =5000;
	int max =_MAX_FD_NUM_;
	int num = -1;
	int done = 0;
	while(!done)
	{
	//	printf("join read_deal\n");
		int i = 0;
		num = epoll_wait(_tmp->_epoll_fd,_tmp->_ev_arry,max,timeout);
		data_buf_p mem = (data_buf_p)malloc(sizeof(data_buf_t));
	//	printf("read num %d\n",num);	
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
					if(_tmp->_ev_arry[i].events & (EPOLLIN | EPOLLET))
					{
						int fd = _tmp->_ev_arry[i].data.fd;
						mem->fd = fd;
		//				printf("fd:%d",fd);
						read_head_arry[i]._sock_fd = fd;
						read_head_arry[i]._head = (hd_p)malloc(sizeof(hd_t));
						pthread_t read_deal_tid;
						pthread_create(&read_deal_tid,NULL,in_deal,(void *)&read_head_arry[i]);
						pthread_join(read_deal_tid,NULL);
						
				//		printf("read_h_a[i]:%s\n",read_head_arry[i]._head->method);
				//		printf("read_h_a[i]:%s\n",read_head_arry[i]._head->path);
				//		printf("read_h_a[i]:%s\n",read_head_arry[i]._head->url);
				//		printf("read_h_a[i]:%s\n",read_head_arry[i]._head->cgi);
						ev.events = EPOLLOUT | EPOLLET;
						ev.data.ptr = mem;
						epoll_ctl(_tmp->_epoll_fd,EPOLL_CTL_MOD,fd,&ev);
					}
				else if(_tmp->_ev_arry[i].events & (EPOLLOUT | EPOLLET))
					{
						data_buf_p mem = (data_buf_p)malloc(sizeof(data_buf_t));
						mem = (data_buf_p)_tmp->_ev_arry[i].data.ptr;
						int fd = mem->fd;
		//				printf("out fd %d\n",fd);	
						if(read_head_arry[i]._head->cgi == 1)
						{
//							printf("join cgi\n");
							//echo_cgi(fd,http_head.path,http_head.method,http_head.query_string);	
							pthread_t write_deal_cgi;
							pthread_create(&write_deal_cgi,NULL,out_deal_cgi,(void *)&(read_head_arry[i]));
							pthread_detach(write_deal_cgi);

						}
						else
						{
		//					printf("joinc uncgi\n");
							fflush(stdout);
							pthread_t write_deal_uncgi;
							pthread_create(&write_deal_uncgi,NULL,out_deal_uncgi,(void *)&read_head_arry[i]);
							pthread_detach(write_deal_uncgi);
							//echo_html(fd,http_head.path,0);
						}
						
				//		ev.events = EPOLLIN | EPOLLET;
				//		ev.data.fd = mem->fd;
				//		epoll_ctl(_tmp->_epoll_fd,EPOLL_CTL_MOD,fd,&ev);
				//		close(fd);
					//	printf("success close\n");
					}
				}
				break;
			}
		}
	}
}


//void* write_deal (void *arg)
//{
//	printf("joinc wirte_deal\n");
//	fd_s* _tmp = (fd_s *)arg;
//	ep_e ev;
//	int timeout =5000;
//	int max =_MAX_FD_NUM_;
//	int num = -1;
//	int done = 0;
//	while(!done)
//	{	
//		int i = 0;
//	
//		num = epoll_wait(_tmp->_epoll_fd,_tmp->_ev_arry,max,timeout);
//	//	printf("write e_fd%d\n",_tmp->_epoll_fd);	
//		printf("num%d\n",num);
//		switch(num)
//		{	
//			
//			case 0://timeout
//				break;
//			case -1:
//				perror("epoll_wait");
//				break;
//			default:
//			{
//				data_buf_p mem = (data_buf_p)malloc(sizeof(data_buf_t));
//				
//				for(i = 0; i < num;++i)
//				{	printf("join out for\n");
//					if(_tmp->_ev_arry[i].events & (EPOLLOUT | EPOLLET))
//					{
//						int fd = mem->fd;
//						
//						printf("read_head_arry[i]:cgi:%d",read_head_arry[i]._head->cgi);
//						if(read_head_arry[i]._head->cgi == 1)
//						{
//							printf("join cgi\n");
//							//echo_cgi(fd,http_head.path,http_head.method,http_head.query_string);	
//							pthread_t write_deal_cgi;
//							pthread_create(&write_deal_cgi,NULL,out_deal_cgi,(void *)&(read_head_arry[i]));
//							pthread_detach(write_deal_cgi);
//
//						}
//						else
//						{
//							printf("joinc uncgi\n");
//							fflush(stdout);
//							pthread_t write_deal_uncgi;
//							pthread_create(&write_deal_uncgi,NULL,out_deal_uncgi,(void *)&read_head_arry[i]);
//							pthread_detach(write_deal_uncgi);
//							//echo_html(fd,http_head.path,0);
//						}
//						
//						ev.events = EPOLLIN | EPOLLET;
//						ev.data.fd = mem->fd;
//						epoll_ctl(_tmp->_epoll_fd,EPOLL_CTL_MOD,fd,&ev);
//						close(fd);
//					}
//				}
//				break;
//			}
//		}
//	}
//	return NULL;
//}



//设计思路,使用3个分离线程进行操作，
//1.socket的接受线程,主要处理epoll_ctl的listen状态的客户端添加。
//2.通过死循环while不断去获取当前的events数组进行events判断,读事件丢到线程进行处理,读完成后进行改写为写事件
//3.任然是死循环while不断去获取当前的events进行写事件判断,是写事件,丢到线程进行处理。
int main(int argc,char *argv[])
{
	
	char* ip = argv[1];
	int port = atoi(argv[2]);
	
	int listen_sock = server_listen(ip,port);
	pid_t pid;
	int epoll_fd = epoll_create(256);
	if(epoll_fd < 0)
	{
		perror("epoll_create");
		exit(4);
	}

//	printf("1\n");
	fd_s* listen_fd = (fd_s*)malloc(sizeof(fd_s));
	listen_fd->_sock_fd = listen_sock;
	listen_fd->_epoll_fd = epoll_fd;
	//设置一个输出的参数数组;
//	struct epoll_event ev_out[_MAX_FD_NUM_];
	listen_fd->_ev_arry = ev_out;
//	printf("success\n");
	//3个线程
	//pthread_t sock_tid,read_tid,write_tid;
	while(1)
	{
		pthread_t sock_tid,read_tid,write_tid;
		pthread_create(&sock_tid,NULL,epoll_wait_add,(void *)listen_fd);
		pthread_create(&read_tid,NULL,read_deal,(void *)listen_fd);
		pthread_detach(read_tid);
		pthread_join(sock_tid,NULL);
	}
//	pthread_create(&write_tid,NULL,write_deal,(void *)listen_fd);
	//pthread_join(sock_tid,NULL);
	//pthread_detach(read_tid);
//	pthread_join(write_tid,NULL);

	return 0;
}

