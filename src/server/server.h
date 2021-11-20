#ifndef SERVER_H
#define SERVER_H

#include <thread>
#include "header.h"
#include "dentry.h"

int sv_entry(int cfd, struct sockaddr_in * claddr, socklen_t cllen);
int sv_dirlst(int cfd);
// int sv_file(int cfd);
int sv_sync_download(int cfd);

#endif
