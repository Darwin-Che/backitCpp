#include "server.h"

char REPOABS[PATH_MAX];

/* The entry point from the loop of the server
 * Procondition : cfd and claddr and len are valid
 * Need to free the heap allocated claddr
 */
int sv_entry(int cfd, struct sockaddr_in * claddr, socklen_t cllen) {
	uint64_t sv_op;
	char claddr_str[INET_ADDRSTRLEN];

	if (inet_ntop(AF_INET, &claddr->sin_addr, claddr_str,
				INET_ADDRSTRLEN) == NULL)
		printf("cannot convert client addr\n");
	else
		printf("RECEIVING FROM (%s, %u)\n", 
				claddr_str, ntohs(claddr->sin_port));

	if (read64b(cfd, &sv_op) < 0) {
		free(claddr);
		errExit("read sv_op");
	}
	
	printf("sv_op : %llu\n", sv_op);
	switch(sv_op) {
		case OP_SV_DIRLST:
			if (sv_dirlst(cfd) < 0){
				free(claddr);
				errExit("sv_dirlst fail");
			}
			break;

		case OP_SV_SYNCD_FILES:
			if (sv_sync_download(cfd) < 0) {
				free(claddr);
				errExit("sv_sync_download fail");
			}
			break;

		case OP_SV_REMOVE_FILES:
			if (sv_remove_files(cfd) < 0) {
				errExit("sv_remove_files fail");
			}
			break;

		default:
			free(claddr);
			errExit("invalid server opcode");
	}

	free(claddr);

	if (close(cfd) == -1)
		errExit("close");
	
	return 0;
}

int sv_dirlst(int cfd) {
	dirlst_t * dl;
	char pathname[PATH_MAX]; // big stack allocation

	if (readLine(cfd, pathname, PATH_MAX) <= 0) {
		close(cfd);
	}

	printf("pathname : %s\n", pathname);
	bi_pathcombine(pathname, REPOABS);
	printf("pathname : %s\n", pathname);
	dl = to_dirlst(pathname);
	if (write64b(cfd, dl->len) < 0)
		errExit("write total");

	for (mdirent_t * mdp = dl->head; mdp; mdp = mdp->m_next) {
		if (write64b(cfd, mdp->m_mtime_rem) < 0) 
			errExit("write time");
		
		if (write(cfd, mdp->m_name, mdp->m_name_len) != (ssize_t) mdp->m_name_len)
			errExit("write name");

		if (write(cfd, "\n", 1) != 1)
			errExit("write newline");
	}

	return 0;
}

int sv_sync_download(int cfd) {
	char ** pathnames;
	size_t numfiles;
	if (bi_paths_read(cfd, &pathnames, &numfiles) < 0)
		errExit("sync_read fails");
	
	printf("numfiles : %u\n", numfiles);
	// print the received pathnames
	for (size_t n = 0; n < numfiles; ++n) {
		printf("%s\n", pathnames[n]);
	}

	if (bi_files_write(cfd, pathnames, numfiles) < 0)
		errExit("files_write fails");

	return 0;
}

int sv_remove_files(int cfd) {
	char ** pathnames;
	size_t numfiles;
	if (bi_paths_read(cfd, &pathnames, &numfiles) < 0)
		errExit("sync_read fails");
	
	printf("numfiles : %u\n", numfiles);
	// print the received pathnames
	for (size_t n = 0; n < numfiles; ++n) {
		printf("%s\n", pathnames[n]);
	}

	char ** rmpaths = new char*[numfiles];
	size_t rmcnt = 0;

	for (size_t n = 0; n < numfiles; ++n) {
		if (remove(pathnames[n]) < 0) {
			continue;
		} else {
			rmpaths[rmcnt] = pathnames[n];
			++rmcnt;
		}
	}

	printf("rmcnt : %u\n", rmcnt);
	// print the received pathnames
	for (size_t n = 0; n < rmcnt; ++n) {
		printf("%s\n", rmpaths[n]);
	}

	if (bi_paths_write(cfd, rmpaths, rmcnt) < 0) 
		errExit("paths_write");
	
	return 0;
}


#if 0
int sv_file(int cfd) {
	char pathname[PATH_MAX]; // big stack allocation
	struct stat st;
	size_t filesz;
	ssize_t sendsz;
	off_t fileoffs;
	int ffd;

	if (readLine(cfd, pathname, PATH_MAX) <= 0) {
		close(cfd);
	}

	memset(&st, 0x0, sizeof(st));
	if (stat(pathname, &st) == -1) 
		errExit("sv_file : cannot stat file");
	
	ffd = open(pathname, O_RDONLY);
	if (ffd < 0)
		errExit("sv_file : cannot read file");

	filesz = st.st_size;
	write64b(cfd, filesz);

	sendsz = sendfile(cfd, ffd, &fileoffs, filesz);
	if (sendsz < 0 || (size_t) sendsz != filesz) 
		errExit("sv_file : tranmission error");
	
	close(cfd);
	return 0;
}
#endif


