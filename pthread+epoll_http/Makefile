ROOT_PATH=$(shell pwd)
LDFLAGS=-lpthread#-static
FLAGS=-g #-D_DEBUG 
CC=gcc
BIN=httpd
RIO=$(ROOT_PATH)/rio
INCLUDE = -I$(RIO)

CGI_BIN=htdocs/cgi_bin
SQL_API=htdocs/sql_api

SRC=$(shell ls *.c)
SRC+=$(shell ls -R $(RIO) | grep -E '*.c')
OBJ=$(SRC:.c=.o)

.PHONY:all
all:$(BIN)

$(BIN):http_server.o webmain.o rio.o
	@echo $(OBJ)
	@echo "Linking [$^] to [$@]"
	$(CC) -o $@ $^ $(LDFLAGS) 
	@echo "Linking done..."

%.o:$(RIO)/%.c
	@echo "Compling [$<] to [$@]"
	@$(CC) -c $< $(FLAGS) $(INCLUDE)
	@echo "Compling done..."
%.o:%.c
	@echo "Compling [$<] to [$@]"
	@$(CC) -c $< $(FLAGS) $(INCLUDE)
	@echo "Compling done..."

.PHONY:clean
clean:
	@rm -rf *.o $(BIN) output
	@for name in `echo $(CGI_BIN)`;\
	do\
		cd $$name;\
		make clean;\
		cd -;\
	done
	@for name in `echo $(SQL_API)`;\
	do\
		cd $$name;\
		make clean;\
		cd -;\
	done

.PHONY:cgi
cgi:
	@for name in `echo $(CGI_BIN)`;\
	do\
		cd $$name;\
		make;\
		cd -;\
	done

.PHONY:sql
sql:
	@for name in `echo $(SQL_API)`;\
	do\
		cd $$name;\
		make;\
		cd -;\
	done

.PHONY:output
output: $(BIN) cgi
	@mkdir -p output/log
	@cp -rf htdocs output/
	@mkdir -p output/htdoc/cgi_bin
	@mkdir -p output/htdoc/sql_api
	@cp -f httpd output/
	@cp -rf conf output
	@for name in `echo $(CGI_BIN)`;\
	do\
		cd $$name;\
		make output;\
		cd -;\
	done
	@for name in `echo $(SQL_API)`;\
	do\
		cd $$name;\
		make output;\
		cd -;\
	done

#lib
#log
