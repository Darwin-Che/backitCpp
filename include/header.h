#include <iostream>
#include <cstring>
#include <ctype.h>
#include <errno.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>


#define PORT_NUM 50002
#define PATH_MAX 1024


// UTILITIES

void errExit(const char * msg);

ssize_t readLine(int fd, void * buffer, size_t n);
