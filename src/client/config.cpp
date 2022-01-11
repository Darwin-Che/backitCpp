#include "client.h"
#include "header.h"
#include "dentry.h"

cl_params_t cl_params_obj;

static void cl_config_reset() {
	cl_params_obj.REPO_PREFIX[0] = '\0';
}

void cl_config_read(const char * repoabs) {
	cl_config_reset();
	// open config
	char configabs[PATH_MAX];
	size_t len = strlen(repoabs);
	strcpy(configabs, repoabs);
	strcpy(&configabs[len], "/.backit/config");
	int fd = open(configabs, O_RDONLY);
	if (fd < 0) {
		printf("Fail to find config file\n");
		printf("REPO_PREFIX is \"%s\"\n", cl_params_obj.REPO_PREFIX);
		return;
	}

	char buf[2048]; 
	ssize_t sz;
	while ((sz = readLine(fd, buf, 2048)) > 0) {
		printf("HERE!\n");
		if (sz == 0) continue;
		size_t i = 0;
		while (i < sz) {
			if (buf[i] == '=') 
				break;
			++i;
		}
		if (i == sz) 
			errExit("Config file wrong format");

		if (strncmp("REPO_PREFIX", buf, i) == 0) {
			strcpy(cl_params_obj.REPO_PREFIX, &buf[i+1]);
			printf("REPO_PREFIX is \"%s\"\n", cl_params_obj.REPO_PREFIX);
		}
	}
}
