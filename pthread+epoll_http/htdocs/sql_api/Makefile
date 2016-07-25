ROOT_PATH=$(shell pwd)
MYSQL_INCLUDE=$(ROOT_PATH)/bin/include
MYSQL_LIB=$(ROOT_PATH)/lib/lib
BIN_INSERT=sql_cgi_insert
BIN_SELECT=sql_cgi_select
BIN_UPDATE=sql_cgi_update
BIN_DELETE=sql_cgi_delete


INCLUDE=-I$(MYSQL_INCLUDE)
LIB=-L$(MYSQL_LIB)
LIB_LINK=-lmysqlclient

.PHONY:all
all:$(BIN_INSERT) $(BIN_UPDATE) $(BIN_SELECT) $(BIN_DELETE)
$(BIN_INSERT):sql_api.cpp insert.cpp
	g++ -o $@ $^ $(INCLUDE) $(LIB) $(LIB_LINK)
$(BIN_UPDATE):sql_api.cpp update.cpp
	g++ -o $@ $^ $(INCLUDE) $(LIB) $(LIB_LINK)
$(BIN_DELETE):sql_api.cpp delete.cpp
	g++ -o $@ $^ $(INCLUDE) $(LIB) $(LIB_LINK)
$(BIN_SELECT):sql_api.cpp select.cpp
	g++ -o $@ $^ $(INCLUDE) $(LIB) $(LIB_LINK)

.PHONY:clean
clean:
	rm -f $(BIN_INSERT) $(BIN_UPDATE) $(BIN_SELECT) $(BIN_DELETE)

.PHONY:output
output:all
	cp -r $(BIN_INSERT) $(BIN_UPDATE) $(BIN_SELECT) $(BIN_DELETE) ../../output/htdoc/sql_api

#lib
#log
