#include "client.h"
#include "header.h"
#include "dentry.h"

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

int cl_ls(int argc, char ** argv) {
	int cfd = cl_connect();

	char * fsabs = normalize_path(argc > 1 ? argv[1] : ".");
	printf("File system abs path : %s\n", fsabs);
	char * reporel = cl_getrepopath(fsabs);
	printf("reporel : %s\nfsabs : %s\n", reporel, fsabs);
	if (reporel == nullptr)
		errExit("Given path is not under a repo. Exit...");
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

	dirvec_t locvec, remvec, syncvec;
	comb_loc_rem(
		to_dirvec(fsabs),
		dvec,
		&locvec,
		&remvec,
		&syncvec);

	printf("LOCAL : \n");
	print_dirvec(locvec);

	printf("REMOTE : \n");
	print_dirvec(remvec);

	printf("SYNC : \n");
	print_dirvec(syncvec);

	return 0;
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
