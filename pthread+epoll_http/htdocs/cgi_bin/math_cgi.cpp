#include<iostream>
#include<string.h>
#include<string>
#include<stdio.h>
#include<stdlib.h>
using namespace std;


struct data
{
	char name[10];
	int number;
};

struct data value[2];

void acquire_data(string &query_string)
{

	int i = query_string.size();
	int index = 1;
	while(i > 0&& index >= 0)
	{
		while((query_string[i] != '=')&&(query_string[i] != '&')&& i > 0)
		{
			--i;
		}
		value[index].number = atoi(&query_string[i+1]);
		query_string[i] ='\0';
		while((query_string[i] != '&')&& i > 0)
		{
			--i;
		}
		if(i == 0)
			strcpy(value[index].name,&query_string[i]);
		else
			strcpy(value[index].name,&query_string[i+1]);
		query_string[i] = '\0';
		--index;
	}
}

void show_page()
{
	
	cout<<"<html>"<<endl;	
	cout<<"<html>"<<endl;	
	cout<<"<head>"<<endl;	
	cout<<"<h>computer</h>"<<endl;	
	cout<<"</head>"<<endl;	
	cout<<"<body>"<<endl;	
	cout<<"<p>";
	printf("%d+%d = %d",value[0].number,value[1].number,value[0].number+value[1].number);
	cout<<"</p>"<<endl;	
	cout<<"</body>"<<endl;	
	cout<<"</html>"<<endl;	
}

int main()
{
	string  content_len;
	string method;
	string query_str;
	char buf[1024];

	memset(buf,'\0',sizeof(buf));
	if(getenv("REQUEST_METHOD"))
	{
		method = getenv("REQUEST_METHOD");
	}
	else
	{
		method = "";
	}
	if(method == "GET")
	{
		if(getenv("QUERY_STRING"))
			query_str = getenv("QUERY_STRING");
	
		else 
			query_str = "";
	}
	else if(method == "POST")
	{	
		if(getenv("CONTENT_LENGTH"))
		{
			content_len = getenv("CONTENT_LENGTH");
			int len = atoi(content_len.c_str());
			char ch;
			int index = 0;
			while(len)
			{
				read(0,&ch,1);
				buf[index++] = ch;
				len--;
			}
			query_str = buf;
		}
		else
		{
			query_str = "";
		}
	}
	else{}
	acquire_data(query_str);

	show_page();
	return 0;
}
