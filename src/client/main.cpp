#include "header.h"
#include "dentry.h"
#include "client.h"

int main(int argc, char *argv[])
{
	if (argc < 1 || strcmp(argv[1], "--help") == 0) 
		errExit("usage error");
	
	if (strcmp(argv[1], "ls") == 0) {
		if (cl_ls(argc - 1, argv + 1) < 0)
			errExit("cl_ls fail");

	} else if (strcmp(argv[1], "syncd") == 0) {
		if (cl_sync_download(argc - 1, argv + 1) < 0)
			errExit("cl_sync_download fail");

	} else {
		errExit("usage error");
	}

	return 0;
}
