#include"http_server.h"

//输出错误信息

void print_error(const char * const  str,const char * const func,int line)
{
	printf("%s:,%s,%d\n",str,func,line);
}


//回显错误信息网页
void echo_err(int sock,int _errno)
{
	switch(_errno)
	{
		case 404:
			echo_html(sock,"htdocs/404.html",1);
			break;
		case 400:
			echo_html(sock,"htdocs/400.html",1);
			break;
		case 401:
			echo_html(sock,"htdocs/401.html",1);
			break;
		case 403:
			echo_html(sock,"htdocs/403.html",1);
			break;
		case 500:
			echo_html(sock,"htdocs/500.html",1);
			break;
		case 503:
			echo_html(sock,"htdocs/503.html",1);
			break;
		default:
			break;
	}
}

//服务器监听开始
int server_listen(char *ip,int port)
{
	int sock = socket(AF_INET,SOCK_STREAM,0);
	
	if(sock < 0)
	{
		print_error(strerror(errno),__FUNCTION__,__LINE__);
		exit(1);
	}

	int opt = 1;
	if(setsockopt(sock,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt)) < 0)
	{
		print_error(strerror(errno),__FUNCTION__,__LINE__);
		exit(2);	
	}

	struct sockaddr_in listen_server;
	listen_server.sin_family = AF_INET;
	listen_server.sin_port = htons(port);
	if(strcasecmp(ip,"any") == 0)
		listen_server.sin_addr.s_addr = htonl(INADDR_ANY);
	else
		listen_server.sin_addr.s_addr = inet_addr(ip); 

	if(bind(sock,(struct sockaddr*)&listen_server,sizeof(listen_server)) < 0)
	{
		print_error(strerror(errno),__FUNCTION__,__LINE__);
		exit(3);
	}

	if(listen(sock,_BACKLOG_) < 0 )
	{
		print_error(strerror(errno),__FUNCTION__,__LINE__);
		exit(4);
	}

//	printf("build listen successs\n");	
	return sock;
}

//清楚请求头
static void clear_head(int sock)
{
	rio_t* _rd = (rio_t*)malloc(sizeof(rio_t));
	rio_readinitb(_rd,sock);
//	printf("join clear\n");
	char buf[_MAXLINE_];
	buf[0] = '\0';
	ssize_t _s = 1;
	while(_s > 0 && strcmp(buf,"\n") != 0)
	{
		//_s = read_lineb(sock,buf,_MAXLINE_);
		_s = rio_readlineb(_rd,buf,_MAXLINE_);
	}
//	printf("clear success\n");
}

//消息cgi情况回显
//对收取到的方法进行判断，然后添加到环境变量中
void echo_cgi(int sock,char *buf_ptr,const char* path,const char* method,char *query_string)
{
//	printf("join echo_cgi\n");
	path_e path_set;
	rio_t* _rd = (rio_t*)malloc(sizeof(rio_t));
	_rd->rio_fd = sock;
	_rd->rio_bufptr = buf_ptr;
	_rd->rio_cnt = strlen(buf_ptr);
	memset(path_set.method_env,'\0',sizeof(path_set.method_env));
	memset(path_set.query_string_env,'\0',sizeof(path_set.query_string_env));
	memset(path_set.content_length_env,'\0',sizeof(path_set.content_length_env));

	char line[_MAXLINE_];
	memset(line,'\0',sizeof(line));

	int content_length = -1;
	int line_num = -1;
	int input_cgi[2];
	int output_cgi[2];

	pid_t pid;

//	printf("method:%s,length:%d",method,strlen(method));
	if(strcmp(method,"GET")==0)
	{
		clear_head(sock);
//		printf("join get\n");
	}else if(strcasecmp(method,"POST")== 0)
	{
//		printf("join post\n");
		do
		{
			//line_num = read_lineb(sock,line,sizeof(line));
			line_num = rio_readlineb(_rd,line,sizeof(line));
//			printf("line_num:%d  line:%s\n",line_num,line);
			if(strncasecmp(line,"Content-Length: ",16) == 0)
			{
	//			printf("join\n");
				content_length = atoi(line+16);
				break;
			}
		}while(line_num > 1);
		//read_lineb(sock,line,sizeof(line));
		rio_readlineb(_rd,line,sizeof(line));
		rio_readlineb(_rd,line,sizeof(line));
	//	printf("line_num:%d  line:%s\n",line_num,line);
//		printf("content_length:%d\n",content_length);
		if(content_length == -1)
		{
			printf("request content is error\n");
			return ;
		}
	}else
	{
		printf("error\n");
	}

	if(pipe(input_cgi) < 0)
	{
		printf("intput_cgi create failed\n");
		return ;
	}
	if(pipe(output_cgi) <0)
	{
		printf("output_cgi create failed\n");
		return ;
	}

//	printf("fork_\n");
	if((pid = fork()) < 0)
	{
		printf("cgi create fork failed\n");
		return;
	}
	else if(pid == 0)
	{
		close(input_cgi[1]);
		close(output_cgi[0]);

		dup2(input_cgi[0],0);
		dup2(output_cgi[1],1);

		sprintf(path_set.method_env,"REQUEST_METHOD=%s",method);
		putenv(path_set.method_env);
	
		if(strcmp(method,"GET") == 0)
		{
			
			sprintf(path_set.query_string_env,"QUERY_STRING=%s",query_string);
			putenv(path_set.query_string_env);
		}
		else
		{
		sprintf(path_set.content_length_env,"CONTENT_LENGTH=%d",content_length);
		putenv(path_set.content_length_env);
		}	

		execl(path,path,NULL);
		
		close(input_cgi[0]);
		close(output_cgi[1]);
	}
	else
	{
		char ch;
		close(input_cgi[0]);
		close(output_cgi[1]);
		
		if(strcasecmp(method,"POST") == 0)
		{
//			printf("post while\n");
			write(input_cgi[1],&line,strlen(line));
		}
	sprintf(line,"HTTP/1.1 200 OK\r\n\r\n");
	send(sock,line,strlen(line),0);
		
		while(read(output_cgi[0],&ch,1) > 0)
		{
			send(sock,&ch,1,0);
		}

		
		close(input_cgi[1]);
		close(output_cgi[0]);
		waitpid(pid);
		close(sock);
	}
	
}
//非cgi模式的网页回显，直接发送文件
void echo_html(int sock,char *path,int  is_errno)
{
//	printf("join echo_html success\n");
//	printf("e_path:%s\n",path);
	struct stat st;
//	clear_head(sock);
	stat(path,&st);
	int fd = open(path,O_RDONLY);
	if(fd <0)
	{
		print_error(strerror(errno),__FUNCTION__,__LINE__);
		exit(1);
	}
//	printf("%s\n",path);
	if(is_errno != 1)
	{
		char *status_line = "HTTP/1.0 200 OK\r\n\r\n";
		send(sock,status_line,strlen(status_line),0);
	}
//	printf("start send file\n");

	if(sendfile(sock,fd,NULL,st.st_size) < 0)
	{
		print_error(strerror(errno),__FUNCTION__,__LINE__);
	}
//	printf("echo success\n");
//	printf("fd%d\n",fd);
	close(fd);
	close(sock);
}

//读取行函数
static int read_lineb(int fdsock,char *buf,ssize_t maxlen)
{
	int i = 0;
	ssize_t _s = -1;
	char ch = '\0';
//	printf("join lineb\n");
	while(i < maxlen-1 && ch != '\n')
	{
		_s = recv(fdsock,&ch,1,0);

		if(_s > 0)
		{
			if(ch == '\r')
			{
				if(_s = recv(fdsock,&ch,1,MSG_PEEK))
				{
					if(_s>0&& ch=='\n')
					{
						recv(fdsock,&ch,1,0);
					}
				}
			}
			buf[i++] = ch;
		}
		else
		{
			buf[i++] = '\n';
			break;
		}
	}
	buf[i] = '\0';
	return i;
}


//请求行接受进行的请求行数据拆分
void http_start(int fdsock,hd_p http_head)
{
	//hd_t http_head;
	rio_t* _rd = (rio_t*)malloc(sizeof(rio_t));
	rio_readinitb(_rd,fdsock);
	char buf[_MAXLINE_];
	//char *buf = (char *)malloc(sizeof(_MAXLINE_));
	memset(buf,'\0',_MAXLINE_);
	memset(http_head,'\0',sizeof(*http_head));
	http_head->rio_bufptr = _rd->rio_bufptr;
	
//	printf("fd:%d",fdsock);
//	printf("start join lineb\n");
	//读取一行数据.
	//int rc = read_lineb(fdsock,buf,_MAXLINE_);
	int rc = rio_readlineb(_rd,buf,_MAXLINE_);
	if(rc  < 0)
	{
		print_error(strerror(errno),__FUNCTION__,__LINE__);
	}
	if(rc == 0)
	{
		printf("请求为空.");
	}

		
//	printf("buf:%s\n",buf);

	int i,j;
	i = j = 0;

	while(i < strlen(buf)&&\
			!isspace(buf[i])&&\
			j < sizeof((http_head->method))-1)
	{
//		printf("1\n");
		http_head->method[j] = buf[i++];
		++j;
	}
//	printf("2\n");
	http_head->method[j] = '\0';
//	printf("method:%s\n",http_head->method);
	while(i<strlen(buf)&&isspace(buf[i]))
	{
		i++;
	}
	
//	printf("i:%d",i);

	j = 0;
	while(i < strlen(buf) &&!isspace(buf[i])&&\
			j < sizeof((http_head->url))-1)
	{
		http_head->url[j++] = buf[i++];
	}
	http_head->url[j] = '\0';
//	printf("url:%s\n",http_head->url);
	//判断是不是get/post方法
	if(strcasecmp(http_head->method,"GET") != 0&&\
			strcasecmp(http_head->method,"POST") != 0)
	{
		print_error(strerror(errno),__FUNCTION__,__LINE__);
		return ;
	}

	//判断是不是post方法。
	if(strcasecmp(http_head->method,"POST") == 0)
	{
		http_head->cgi = 1;
	}
	//get方法判断
	http_head->query_string = NULL;
	if(strcasecmp(http_head->method,"GET") == 0)
	{
		http_head->query_string = http_head->url;
		while(*(http_head->query_string) != '\0'&& *(http_head->query_string) != '?')
		{
			++(http_head->query_string);
		}
		if(*(http_head->query_string) == '?')
		{
			*(http_head->query_string) = '\0';
			http_head->cgi = 1;
			++(http_head->query_string);
		}
//		printf("query_string:%s\n",http_head->query_string);
	}
	//将url赋值到path,文件路径为htodocs/xxxx.xx
//	if(strcmp(http_head->url,"/") == 0)
		sprintf(http_head->path,"htdocs%s",http_head->url);
//	else
//		strcpy(http_head->path,(http_head->url)+1);
	//防止htdocs/xxx/出现,后面添加index.html
	if(http_head->path[strlen(http_head->path)-1] == '/')
	{
		strcat(http_head->path,_DEF_PAGE_);		
	}
//	printf("path:%s\n",http_head->path);
	struct stat st;
	if(stat(http_head->path,&st)<0)
	{
//		printf("ehco_error join\n");
		echo_err(fdsock,404);
		print_error(strerror(errno),__FUNCTION__,__LINE__);
//		printf("success\n");
		close(fdsock);
		return ;
	}

	//若请求的时dir（文件夹）,
	else if(st.st_mode & S_IFDIR)
	{
//		printf("join dir\n");
		strcpy(http_head->path,"htdocs/");
		strcmp(http_head->path,_DEF_PAGE_);
	}
	else if(st.st_mode & S_IXUSR ||\
			st.st_mode & S_IXGRP ||\
			st.st_mode & S_IXOTH 
			)
	{
		http_head->cgi = 1;
	}
	//if(cgi == 1)
	//{
	//	echo_cgi(fdsock,http_head.path,http_head.method,query_string);
	//}	
	//else
	//{
	//	printf("join get static\n");
	//	fflush(stdout);
	//	echo_html(fdsock,http_head.path,0);
	//}

	//close(fdsock);
}

//void* handle_client(void *arg)
//{
//	int sock = (int) arg;
//	http_start(sock);
//	return NULL;
//}
