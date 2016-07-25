#include "sql_api.h"

sql_api::sql_api()
	:_conn_fd(NULL)
	 ,_host("127.0.0.1")
	 ,_user("root")
	 ,_passwd("q321596")
	 ,_db("9_class")
	 ,_port(3306)
	 ,_res(NULL)
{	
}
sql_api::~sql_api()
{
	if(_res)
	{
		free(_res);
	}
	mysql_close(_conn_fd);
}
bool sql_api::select_mysql(string options,string content)
{
	string _sql="select * from stu";
	if(strcmp(options.c_str(),"1") == 0)
	{
		_sql+=" where name='";
		_sql+=content;
		_sql+="'";
	}
	if(strcmp(options.c_str(),"2")==0)
	{
		_sql+=" where school='";
		_sql+=content;
		_sql+="'";
	}
	if(strcmp(options.c_str(),"3")==0)
	{
		_sql+=" where hobby='";
		_sql+=content;
		_sql+="'";
	}
	_op_sql(_sql);
	return _select_mysql();
}
bool sql_api::connect_mysql()
{
	_conn_fd = mysql_init(NULL);
	mysql_real_connect(_conn_fd,_host.c_str(),_user.c_str(),_passwd.c_str(),_db.c_str(),_port,NULL,0);
	return true;
}
bool sql_api::insert_mysql(string name,string school,string hobby)
{
	string _sql="INSERT INTO stu";
	_sql+= "(name,school,hobby)";
	_sql+= "VALUES('";
	_sql+=name;
	_sql+= "','";
	_sql+= school;
	_sql+= "','";
	_sql+= hobby;
	_sql+= "')";
	return _op_sql(_sql);
}
bool sql_api::update_mysql(string name,string modify,string content)
{
	bool ret = false;
	string _sql= "update stu set ";
	
	if(strcmp(modify.c_str(),"1")==0)
	{
		_sql+="name";
	}
	if(strcmp(modify.c_str(),"2")==0)
	{
		_sql+="school";
	}
	if(strcmp(modify.c_str(),"3")==0)
	{
		_sql+="hobby";
	}
	_sql+="='";
	_sql+=content;
	_sql+="'";
	_sql+=" where name='";
	_sql+=name;
	_sql+="'";
	return _op_sql(_sql);
}
bool sql_api::delete_mysql(string id,string name)
{
	bool ret=false;
	string _sql="delete from stu where id=";
	_sql+=id;
	_sql+=" and name='";
	_sql+=name;
	_sql+="'";
	
	return _op_sql(_sql);
}

bool sql_api::_select_mysql()
{
	int _row=0;
	int _field=0;

	_res=mysql_store_result(_conn_fd);
	if(_res)
	{
		_row=mysql_num_rows(_res);
		_field=mysql_num_fields(_res);
		cout<<"row:"<<"field:"<<_field<<endl;
		MYSQL_FIELD* _fd;
		
		for(;_fd = (mysql_fetch_field(_res));)
		{
			cout<<_fd->name<<'\t';
		}
		cout<<endl;
	}

	MYSQL_ROW row_line;

	while(_row)
	{
		row_line = mysql_fetch_row(_res);
		int i =0;
		for(;i<_field;++i)
		{
			cout<<row_line[i]<<'\t';
		}
		cout<<endl;
		--_row;
	}
	return true;
}

bool sql_api::_op_sql(string _sql)
{
	bool ret = false;
	if(0 == mysql_query(_conn_fd,_sql.c_str()))
	{
		ret=true;
		cout<<_sql<<" success"<<endl;
	}
	else
	{
		cout<<_sql<<"error"<<endl;
	}
	return ret;
}


void anly_query(string & query,vector<string>& ret)
{
	int index = query.size()-1;

	string str=query;
	while(index>=0)
	{
		if(str[index] == '=')
		{
			ret.push_back(&str[index+1]);
		}
		if(str[index] == '&')
		{
			str[index] = '\0';
		}
		--index;
	}

}

void gain_query(string &query_string)
{
	char method[16];
	char buf[1024];
	memset(buf,'\0',1024);
	int content_length=0;
	
	if(getenv("REQUEST_METHOD"))
	{
		strcpy(method,getenv("REQUEST_METHOD"));
	}
	else
	{
		cout<<strerror(errno)<<endl;
		return;
	}

	if(strcasecmp(method,"GET") == 0)
	{
		query_string+=getenv("QUERY_STRING");
	}
	else
	{
		ssize_t _s = -1;
		if((_s = read(0,buf,sizeof(buf))) > 0)
		{
			buf[_s]='\0';
			query_string+=buf;
		}
	}
}



