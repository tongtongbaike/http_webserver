#include"sql_api.h"

int main()
{
	string query_string;
	gain_query(query_string);
	vector<string> ret;

	anly_query(query_string,ret);
	sql_api _tb;
	cout<<ret[1]<<ret[0]<<endl;
	_tb.connect_mysql();
	_tb.select_mysql(ret[1],ret[0]);
	return 0;
}



