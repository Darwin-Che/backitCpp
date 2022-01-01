#include "header.h"
#include "dentry.h"
#include "client.h"
#include <unistd.h>

extern char * optarg;

int main(int argc, char *argv[])
{
	if (argc <= 1 || strcmp(argv[1], "--help") == 0) 
		errExit("usage error");
	
	if (strcmp(argv[1], "ls") == 0) {
		if (cl_ls(argc - 1, argv + 1) < 0)
			errExit("cl_ls fail");

	} else if (strcmp(argv[1], "syncd") == 0) {
		if (cl_sync_download(argc - 1, argv + 1) < 0)
			errExit("cl_sync_download fail");

	} else if (strcmp(argv[1], "syncu") == 0) {
		if (cl_sync_upload(argc - 1, argv + 1) < 0)
			errExit("cl_sync_upload fail");
	
	} else if (strcmp(argv[1], "rm") == 0) {
		if (cl_rm(argc - 1, argv + 1) < 0)
			errExit("cl_rm fail");

	} else if (strcmp(argv[1], "path") == 0) {
		char * x = normalize_path(argv[2]);
		char * y = cl_getrepopath(x, 1);
		printf("ancestor : %s\n", x);
		printf("repopath : %s\n", y);
		delete[] x;
		delete[] y;

	} else {
		errExit("usage error");
	}

	return 0;
}
