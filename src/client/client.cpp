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
		errExit("connect");
	
	printf("cl_connect succeed!\n");
	return cfd;
}

int cl_ls(int argc, char ** argv) {
	int cfd = cl_connect();

	char * fsabs = normalize_path(argv[1]);
	printf("File system abs path : %s\n", fsabs);
	char * reporel = bi_repopath(fsabs);
	printf("reporel : %s\n", reporel);
	ssize_t numRead;
	dirlst_t * lst = new dirlst_t;
	mdirent_t ** mdp = &lst->head;


	if (write64b(cfd, OP_SV_DIRLST) < 0) 
		errExit("failed write sv op");

	if (write(cfd, reporel, strlen(reporel)) != strlen(reporel))
		errExit("failed write pathname");
	if (write(cfd, "\n", 1) != 1)
		errExit("failed write newline");

	uint64_t totalNum;
	if(read64b(cfd, &totalNum) < 0)
		errExit("read total");
	
	printf("Number of Entries: %llu\n", totalNum);
	lst->len = totalNum;

	uint64_t mtime_rem;
	for (; totalNum > 0; --totalNum) {
		*mdp = new mdirent_t;

		if (read64b(cfd, &mtime_rem) < 0) 
			errExit("read time");
		(*mdp)->m_mtime_loc = (*mdp)->m_mtime_rem = mtime_rem;

		numRead = readLine(cfd, (*mdp)->m_name, NAME_MAX);
		if (numRead == -1)
			errExit("readLine");
		if (numRead == 0)
			errExit("Unexpect EOF");

		mdp = &(*mdp)->m_next;
	}
	*mdp = nullptr;

	dirlst_t *loclst, *remlst, *synclst;
	comb_loc_rem(
		to_dirlst(fsabs),
		lst,
		&loclst,
		&remlst,
		&synclst);

	printf("LOCAL : \n");
	print_dirlst(loclst);

	printf("REMOTE : \n");
	print_dirlst(remlst);

	printf("SYNC : \n");
	print_dirlst(synclst);

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
		reporel = bi_repopath(fsabs);
		printf("reporel : %s\n", reporel);
		pathnames[n] = reporel;
		delete[] fsabs;
	}

	if (write64b(cfd, OP_SV_SYNC_DFILES) < 0) 
		errExit("failed write sv op");

	if (bi_paths_write(cfd, pathnames, numfiles) < 0)
		errExit("bi_paths_write fail");

	if (bi_files_read(cfd) < 0)
		errExit("bi_files_read fail");

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
		reporel = bi_repopath(fsabs);
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
