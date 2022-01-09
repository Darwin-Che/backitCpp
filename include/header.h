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
#include <fcntl.h>
#include <sys/stat.h>

#ifdef __unix__
#include <limits.h>
#include <sys/sendfile.h>

#elif __APPLE__
#include <sys/syslimits.h>

#endif

#define PORT_NUM 50002

#define OP_SV_DIRLST			0x0001
#define OP_SV_REMOVE_FILES		0x0002
#define OP_SV_SYNCD_FILES		0x0004
#define OP_SV_SYNCU_FILES		0x0008

#define META_DIR	".backit"

#define prtime(s, t, len) \
	if (*t == 0) s[0] = '\0'; \
	else strftime(s, len, "%c", localtime((time_t *)t))

extern FILE * log_fptr;
#define eprintf(...) fprintf (log_fptr, __VA_ARGS__); fflush (log_fptr)
#define flush() fflush (log_fptr)

// Utils

void errExit(const char * msg);

char * normalize_path(const char * input, char * output = nullptr);

void bi_pathcombine(char * path, const char * prefix);

ssize_t bi_readn(int fd, void * buf, size_t sz, size_t perlimit = 0);

ssize_t bi_writen(int fd, const void * buf, size_t sz, size_t perlimit = 0);

ssize_t readLine(int fd, void * buffer, size_t n, bool rstrip = true);

int read64b(int fd, uint64_t * buffer);

int write64b(int fd, uint64_t data);

int bi_socket_to_disk(int in_fd, int out_fd, uint64_t sz, 
		uint64_t in_bufsz = 1024, uint64_t out_bufsz = 4096);

int bi_disk_to_socket(int in_fd, int out_fd, uint64_t sz, 
		uint64_t in_bufsz = 4096, uint64_t out_bufsz = 1024);

int bi_paths_read(int fd, char*** filenames, size_t* numfiles);

int bi_paths_write(int fd, char const * const * filenames, size_t numfiles);

int bi_files_read(int fd);

int bi_files_write(int fd, char const * const * filenames, size_t numfiles);

#endif
