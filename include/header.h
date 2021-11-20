#ifndef HEADER_H
#define HEADER_H

#include <iostream>
#include <cstring>
#include <ctype.h>
#include <errno.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#ifdef __APPLE__
#include <sys/syslimits.h>
#endif

#ifdef __unix__
#include <limits.h>
#endif

#define PORT_NUM 50002

#define OP_SV_DIRLST	0x0001

#define prtime(s, t) strftime(s, 30, "%c", localtime((time_t *)t))

// UTILITIES

void errExit(const char * msg);

ssize_t readLine(int fd, void * buffer, size_t n, bool rstrip = true);

int read64b(int fd, uint64_t * buffer);

int write64b(int fd, uint64_t data);

#endif
