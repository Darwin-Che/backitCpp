CC = g++ --std=c++14
CFLAGS = -Wall -Iinclude -g

HEADER = $(wildcard include/*)

SERVER_SRC = $(wildcard src/server/*)
SERVER_CPP = $(filter %.cpp, $(SERVER_SRC))
SERVER_OBJ = $(patsubst %.cpp, %.o, $(SERVER_CPP))

COMMON_SRC = $(wildcard src/common/*)
COMMON_CPP = $(filter %.cpp, $(COMMON_SRC))
COMMON_OBJ = $(patsubst %.cpp, %.o, $(COMMON_CPP))

CLIENT_SRC = $(wildcard src/client/*)
CLIENT_CPP = $(filter %.cpp, $(CLIENT_SRC))
CLIENT_OBJ = $(patsubst %.cpp, %.o, $(CLIENT_CPP))

server : $(HEADER) $(SERVER_OBJ) $(COMMON_OBJ)
	$(CC) $(CFLAGS) -lpthread $(SERVER_OBJ) $(COMMON_OBJ) -o server


client :$(HEADER) $(CLIENT_OBJ) $(COMMON_OBJ)
	$(CC) $(CFLAGS) $(CLIENT_OBJ) $(COMMON_OBJ) -lcurses -o client

cinstall: client
	cp client /usr/local/bin
	
sinstall: server
	cp server /usr/local/bin

%.o : %.cpp $(HEADER)
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: clean
clean : 
	rm -f $(CLIENT_OBJ) $(SERVER_OBJ) $(COMMON_OBJ)
	rm -f server client

