#ifndef CLIENT_H
#define CLIENT_H

#include "header.h"
#include "dentry.h"

struct cl_params_t {
	char repoabs[PATH_MAX + 1];
	char absrel[PATH_MAX + 1];
	char REPO_PREFIX[PATH_MAX + 1];
};

extern cl_params_t cl_params_obj;

char * cl_getrepopath(char * abspath, bool set_repoabs = false, char * output = nullptr);

int cl_start(const char * input_path = ".");
char * cl_reporel(char * output = nullptr);
char * cl_fsabs(char * output = nullptr);

int cl_connect();
int cl_ls(int argc, const char * const * argv);
dircomb_t _cl_ls(const char * input_path = ".");
int cl_sync_download(int argc, char ** argv);
int cl_rm(int argc, char ** argv);
int cl_sync_upload(int argc, char ** argv);

void cl_config_read(const char * repoabs);

void start_ui_loop();

#endif
