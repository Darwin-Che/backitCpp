#ifndef SERVER_H
#define SERVER_H

#include <thread>
#include "header.h"
#include "dentry.h"

extern char REPOABS[PATH_MAX];

int sv_entry(int cfd, struct sockaddr_in * claddr, socklen_t cllen);
int sv_dirlst(int cfd);
// int sv_file(int cfd);
int sv_sync_download(int cfd);
int sv_sync_upload(int cfd);
int sv_remove_files(int cfd);

#endif
