CC      = gcc

BIN_NAME = mud

LUA_LIB_DIR = /usr/local/lib
LUA_INCLUDE = /usr/local/include

MYSQL_LIB_DIR = /usr/lib64/mysql
MYSQL_INCLUDE = /usr/include/mysql

LIB_DIRS =  -L$(LUA_LIB_DIR) -L$(MYSQL_LIB_DIR)
INCLUDES = -I$(SHARED_STR_INCLUDE) -I$(LUA_INCLUDE) -I$(MYSQL_INCLUDE)

C_FLAGS = -Wall -Wextra -Wno-long-long -ggdb -pedantic -std=c99 $(INCLUDES)  -fstack-protector
L_FLAGS = -lz -lpthread -lcrypt -llua -lm $(LIB_DIRS) -lpthread -lssl -lcrypto -ldl -lmysqlclient

O_FILES = socket.o io.o strings.o utils.o interpret.o help.o  \
	  action_safe.o mccp.o save.o event.o handler_events.o\
	  list.o stack.o handler_mysql.o handler_connections.o \
     handler_lua.o accounts.o world.o comm.o rand.o

all: $(O_FILES)
	rm -f $(BIN_NAME)
   
	$(CC) -o $(BIN_NAME) $(O_FILES) $(L_FLAGS) $(C_FLAGS)

.c.o: all
	@echo [`date +%T`] Compiling $< ...
	@$(CC) -c $(C_FLAGS) $<

clean:
	@echo Cleaning code $< ...
	@rm -f *.o
	@rm -f $(BIN_NAME)
	@rm -f *.*~
	@rm -f core*
