#ifndef CLIENT_H
#define CLIENT_H

#include "header.h"

struct cl_config_t {
	char REPO_PREFIX[PATH_MAX];
};

extern cl_config_t cl_config_obj;

char * cl_getrepopath(char * abspath, bool set_repoabs = false);

int cl_connect();
int cl_ls(int argc, char ** argv);
int cl_sync_download(int argc, char ** argv);
int cl_rm(int argc, char ** argv);
int cl_sync_upload(int argc, char ** argv);

void cl_config_read(const char * repoabs);

#endif
