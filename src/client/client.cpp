#include "client.h"
#include "header.h"
#include "dentry.h"

char repoabs[PATH_MAX + 1];
char reporel[PATH_MAX + 1];

int cl_start(const char * input_path) {
	normalize_path(input_path, cl_params_obj.repoabs);
	cl_getrepopath(cl_params_obj.repoabs, true, cl_params_obj.absrel);
	cl_config_read(cl_params_obj.repoabs);
}

char * cl_reporel(char * output) {
	if (output == nullptr) 
		output = new char[PATH_MAX + 1];
	if (strcmp(cl_params_obj.REPO_PREFIX, "") != 0) {
		strcpy(output, cl_params_obj.absrel);
		bi_pathaddprefix(output, cl_params_obj.REPO_PREFIX);
	}
	return output;
}

char * cl_fsabs(char * output) {
	if (output == nullptr)
		output = new char[PATH_MAX + 1];
	strcpy(output, cl_params_obj.absrel);
	bi_pathaddprefix(output, cl_params_obj.repoabs);
	return output;
}

int cl_connect() {
	struct sockaddr_in svaddr;
	int cfd;

	cfd = socket(AF_INET, SOCK_STREAM, 0);
	if (cfd == -1)
		errExit("socket");
	
	memset(&svaddr, 0, sizeof(struct sockaddr_in));
	svaddr.sin_family = AF_INET;
	svaddr.sin_port = htons(PORT_NUM);
	if (inet_pton(AF_INET, "192.168.2.25", &svaddr.sin_addr) <= 0)
		errExit("fail translate address");

	if (connect(cfd, (struct sockaddr*) &svaddr, sizeof(struct sockaddr_in)) == -1)
		errExit("Cannot connect to the server. Exit...");
	
	printf("Connected to the server!\n");
	return cfd;
}

int cl_ls(int argc, const char * const * argv) {
	_cl_ls(argc > 1 ? argv[1] : ".");
	return 0;
}

dircomb_t _cl_ls(const char * input_path) {
	int cfd = cl_connect();

	char * absrel_tmp = new char[PATH_MAX + 1];
	strcpy(absrel_tmp, input_path);
	bi_pathaddprefix(absrel_tmp, cl_params_obj.absrel);

	char * fsabs = cl_fsabs();
	char * reporel = cl_reporel();
	eprintf("[ls] fsabs : %s\n", fsabs);
	eprintf("[ls] reporel : %s\n", reporel);

	ssize_t numRead;
	dirvec_t dvec;

	if (write64b(cfd, OP_SV_DIRLST) < 0) 
		errExit("failed write sv op");

	if (write(cfd, reporel, strlen(reporel)) != strlen(reporel))
		errExit("failed write pathname");
	if (write(cfd, "\n", 1) != 1)
		errExit("failed write newline");

	uint64_t totalNum;
	if(read64b(cfd, &totalNum) < 0)
		errExit("read total");
	
	printf("Number of Entries Recieved: %llu\n", totalNum);

	uint64_t mtime_rem;
	for (; totalNum > 0; --totalNum) {
		mdirent_t * mdp = new mdirent_t;

		if (read64b(cfd, &mtime_rem) < 0) 
			errExit("read time");
		mdp->m_mtime_loc = mtime_null;
		mdp->m_mtime_rem = mtime_rem;

		numRead = readLine(cfd, &mdp->m_name[0], NAME_MAX);
		if (numRead == -1)
			errExit("readLine");
		if (numRead == 0)
			errExit("Unexpect EOF");

		dvec.arr.push_back(std::move(mdp));
	}

	return comb_loc_rem(to_dirvec(fsabs), dvec);
}

int cl_sync_download(int argc, char ** argv) {
	int cfd = cl_connect();

	size_t numfiles = argc - 1;
	char ** pathnames = new char*[numfiles];

	char * fsabs;
	char * reporel;
	for (size_t n = 0; n < numfiles; ++n) {
		fsabs = normalize_path(argv[n+1]);
		printf("File system abs path : %s\n", fsabs);
		reporel = cl_getrepopath(fsabs);
		printf("reporel : %s\n", reporel);
		pathnames[n] = reporel;
		delete[] fsabs;
	}

	if (write64b(cfd, OP_SV_SYNCD_FILES) < 0) 
		errExit("failed write sv op");

	if (bi_paths_write(cfd, pathnames, numfiles) < 0)
		errExit("bi_paths_write fail");

	if (bi_files_read(cfd) < 0)
		errExit("bi_files_read fail");

	return 0;
}

int cl_sync_upload(int argc, char ** argv) {
	int cfd = cl_connect();

	size_t numfiles = argc - 1;
	char ** pathnames = new char*[numfiles];

	char * fsabs;
	char * reporel;
	for (size_t n = 0; n < numfiles; ++n) {
		fsabs = normalize_path(argv[n+1]);
		printf("File system abs path : %s\n", fsabs);
		reporel = cl_getrepopath(fsabs);
		printf("reporel : %s\n", reporel);
		pathnames[n] = reporel;
		delete[] fsabs;
	}

	if (write64b(cfd, OP_SV_SYNCU_FILES) < 0) 
		errExit("failed write sv op");
	
	if (bi_files_write(cfd, pathnames, numfiles) < 0)
		errExit("files_write fails");
	
	return 0;
}

int cl_rm(int argc, char ** argv) {
	int cfd = cl_connect();

	size_t numfiles = argc - 1;
	char ** pathnames = new char*[numfiles];

	char * fsabs;
	char * reporel;
	for (size_t n = 0; n < numfiles; ++n) {
		fsabs = normalize_path(argv[n+1]);
		printf("File system abs path : %s\n", fsabs);
		reporel = cl_getrepopath(fsabs);
		printf("reporel : %s\n", reporel);
		pathnames[n] = reporel;
		delete[] fsabs;
	}

	if (write64b(cfd, OP_SV_REMOVE_FILES) < 0) 
		errExit("failed write sv op");

	if (bi_paths_write(cfd, pathnames, numfiles) < 0)
		errExit("bi_paths_write fail");
	
	if (bi_paths_read(cfd, &pathnames, &numfiles) < 0)
		errExit("paths_read fails");
	
	printf("Deleted files are :\n");
	for (size_t n = 0; n < numfiles; ++n) {
		printf("%s\n", pathnames[n]);
	}

	return 0;
}
