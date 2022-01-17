#include "server.h"

char repoabs[PATH_MAX + 1];

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

		case OP_SV_SYNCU_FILES:
			if (sv_sync_upload(cfd) < 0) {
				free(claddr);
				errExit("sv_sync_download fail");
			}
			break;

		case OP_SV_REMOVE_FILES:
			if (sv_remove_files(cfd) < 0) {
				free(claddr);
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
	char pathname[PATH_MAX + 1]; // big stack allocation

	if (readLine(cfd, pathname, PATH_MAX + 1) <= 0) {
		close(cfd);
	}

	printf("pathname : %s\n", pathname);
	bi_pathaddprefix(pathname, repoabs);
	printf("pathname : %s\n", pathname);
	dirvec_t dl = to_dirvec(pathname);
	if (write64b(cfd, dl.arr.size()) < 0)
		errExit("write total");

	for (mdirent_t * mdp : dl.arr) {
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
	
	printf("numfiles : %zu\n", numfiles);
	// print the received pathnames
	for (size_t n = 0; n < numfiles; ++n) {
		printf("%s\n", pathnames[n]);
	}

	if (bi_files_write(cfd, pathnames, numfiles) < 0)
		errExit("files_write fails");

	return 0;
}

int sv_sync_upload(int cfd) {
	if (bi_files_read(cfd) < 0)
		errExit("bi_files_read fail");
	
	return 0;
}

int sv_remove_files(int cfd) {
	char ** pathnames;
	size_t numfiles;
	if (bi_paths_read(cfd, &pathnames, &numfiles) < 0)
		errExit("sync_read fails");
	
	printf("numfiles : %zu\n", numfiles);
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

	printf("rmcnt : %zu\n", rmcnt);
	// print the received pathnames
	for (size_t n = 0; n < rmcnt; ++n) {
		printf("%s\n", rmpaths[n]);
	}

	if (bi_paths_write(cfd, rmpaths, rmcnt) < 0) 
		errExit("paths_write");
	
	return 0;
}


