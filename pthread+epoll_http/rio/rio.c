#include"rio.h"

//从描述符fd中读取n个字节到存储器位置usrbuf
ssize_t rio_readn(int fd,void *usrbuf,size_t n)
{
	size_t nleft = n; //剩余字节数保存
	size_t nread;
	char *bufp = usrbuf;

	while(nleft > 0)
	{
		if((nread = read(fd,bufp,nleft)) < 0)
		{
			if(errno == EINTR)
				nread = 0;
			else
				return -1;
		}
		nleft -=nread;
		bufp += nread;
	}

	return (n - nleft);
}

//usrbuf缓冲区中的前n个字节写入fd,并且保证写入完全

ssize_t rio_writen(int fd,void *usrbuf,size_t n)
{
	size_t nleft = n;
	ssize_t nwriten;
	char *bufp = (char *)usrbuf;

	while(nleft > 0)
	{
		if(nwriten = write(fd,bufp,nleft) <= 0)
		{
			if(errno == EINTR)
				nwriten = 0;
			else
				return -1;
		}
		nleft -= nwriten;
		bufp += nwriten;
	}
	return n;
}


//初始化内部缓冲区结构
void rio_readinitb(rio_t *rp,int fd)
{
	rp->rio_fd = fd;
	rp->rio_cnt = 0;
	rp->rio_bufptr = rp->rio_buf;
}

//包装read 增加内部缓冲区
static ssize_t rio_read(rio_t *rp,char *usrbuf,size_t n)
{
	int cnt;

	//内部缓冲区为空，则从文件秒素附中读取填满缓冲区
//	printf("rio_cnt%d\n",rp->rio_cnt);
//	printf("join rio_read\n");
//	printf("rio->bufptr:%s\n",rp->rio_bufptr);
	while(rp->rio_cnt <= 0)
	{
		rp->rio_cnt = read(rp->rio_fd,rp->rio_buf,sizeof(rp->rio_buf));

		if(rp->rio_cnt < 0)
		{
			if(errno != EINTR)
				return -1;
		}
		else if(rp->rio_cnt == 0)
		{
			return 0;
		}
		else
		{
			rp->rio_bufptr = rp->rio_buf;
		}
		
	}
		cnt = n;
		if(rp->rio_cnt < n)
		{
			cnt = rp->rio_cnt;
		}
	//	printf("cnt:%d,rio_bufptr%s\n",cnt,rp->rio_bufptr);
		memcpy(usrbuf,rp->rio_bufptr,cnt);

		rp->rio_bufptr += cnt;
		rp->rio_cnt -=cnt;
		return cnt;

}


//文件rp中读取一行数据,拷贝到usrbuf中用0结束这行数据

ssize_t rio_readlineb(rio_t *rp,void *usrbuf,size_t maxlen)
{
	int n,rc;
	char c, *bufp = usrbuf;
	
	for(n = 1;n < maxlen;n++)
	{
	//	printf("read one bitrc:%d\n",rc);
		rc = rio_read(rp,&c,1);
		if(rc == 1)
		{
			if(c == '\r')
			{
				rc = rio_read(rp,&c,1);
				if(rc > 0 && c == '\n')
				{
					*bufp++ = c;
					break;
				}
				else{
					*bufp++ = '\r';
					*bufp++ = c;
				}
			}
			*bufp++ = c;
			if(c == '\n')
			{	
				break;
			}
		}
		else if(rc == 0)
		{
			if(n == 1)
			{
				return 0;
			}
			else
			{
				break;
			}
		}
		else if (rc < 0)
		{
			return -1;
		}
	}
	*bufp = '\0';
	return n;
}

//从文件rp中读取n字节到usrbuf

ssize_t rio_readnb(rio_t *rp,void *usrbuf,size_t n)
{
	size_t nleft = n;//剩下未读取字节数
	ssize_t nread;
	char *bufp = usrbuf;

	while(nleft > 0)
	{
		if((nread = rio_read(rp,bufp,nleft)) < 0)
		{
			if(errno == EINTR)
				nread = 0;
			else 
				return -1;
		}
		else if(nread == 0 )
		{
			break;
		}
		nleft -= nread;
		bufp += nread;
	}
	return (n - nleft);
}
