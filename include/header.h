#include <iostream>
#include <cstring>
#include <ctype.h>
#include <errno.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define BUF_SIZE 10
#define PORT_NUM 50002

// UTILITIES

void errExit(char* msg);

ssize_t readLine(int fd, void * buffer, size_t n);
