ROOT_PAHT=$(shell pwd)
LDFLAGS=-lpthread#-static
FLAGS=#-D_DEBUG #-g
CC=gcc
BIN=httpd

CGI_BIN=htdocs/cgi_bin
SQL_API=htdocs/sql_api

SRC=$(shell ls *.c)
OBJ=$(SRC:.c=.o)

.PHONY:all
all:$(BIN)
$(BIN):$(OBJ)
	@echo "Linking [$^] to [$@]"
	@$(CC) -o $@ $^ $(LDFLAGS)
	@echo "Linking done..."

%.o:%.c
	@echo "Compling [$<] to [$@]"
	@$(CC) -c $< $(FLAGS)
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
