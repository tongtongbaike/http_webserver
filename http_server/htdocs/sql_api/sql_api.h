#pragma once
#include <mysql/mysql.h>
#include <iostream>
#include <stdlib.h>
#include <vector>
#include <errno.h>
#include <string.h>
#include <string>
using namespace std;

class sql_api
{
public:

sql_api();
~sql_api();
bool select_mysql(string options,string content);
bool connect_mysql();
bool insert_mysql(string name,string school,string hobby);
bool update_mysql(string name,string modify,string content);
bool delete_mysql(string id,string name);
private:
bool _select_mysql();
bool _op_sql(string _sql);
private:
	MYSQL* _conn_fd;
	string _host;
	string _user;
	string _passwd;
	string _db;
	short _port;

	MYSQL_RES* _res;
};

void gain_query(string &query_string);

void anly_query(string &query,vector<string>& ret);
