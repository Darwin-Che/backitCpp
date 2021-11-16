CC = g++ --std=c++14
CFLAGS = -Wall -Iinclude

HEADER = $(wildcard include/*)

SERVER_SRC = $(wildcard src/server/*)
SERVER_CPP = $(filter %.cpp, $(SERVER_SRC))
SERVER_OBJ = $(patsubst %.cpp, %.o, $(SERVER_CPP))

COMMON_SRC = $(wildcard src/common/*)
COMMON_CPP = $(filter %.cpp, $(COMMON_SRC))
COMMON_OBJ = $(patsubst %.cpp, %.o, $(COMMON_CPP))


# server depends on all files under folders 
#      include/    src/common/   src/server
server : $(HEADER) $(COMMON_SRC) $(SERVER_SRC)
	$(CC) $(CFLAGS) $(SERVER_OBJ) $(COMMON_OBJ) -o server

# server depends on all files under folders 
#      include/    src/common/   src/client
# client :$(HEADER) $(COMMON_SRC) $(CLIENT_SRC)

%.o : %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: clean
clean : 
	rm -f $(SERVER_OBJ) $(COMMON_OBJ)
	rm -f server client

