#include "server.h"

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
	
	switch(sv_op) {
		case OP_SV_DIRLST:
			if (sv_dirlst(cfd) < 0){
				free(claddr);
				errExit("sv_dirlst fail");
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
