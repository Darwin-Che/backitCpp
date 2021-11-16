#include "header.h"
#include "dentry.h"

#define BACKLOG 3

int main(int argc, char *argv[])
{
	struct sockaddr_in svaddr, claddr;
	int sfd, optval, cfd;
	socklen_t len;
	char claddrStr[INET_ADDRSTRLEN];
	char pathname[PATH_MAX];

	sfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sfd == -1)
		errExit("socket");

	memset(&svaddr, 0, sizeof(struct sockaddr_in));
	svaddr.sin_family = AF_INET;
	svaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	svaddr.sin_port = htons(PORT_NUM);

	if (setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR,
				&optval, sizeof(optval)) == -1)
		errExit("setsockopt");

	if (bind(sfd, (struct sockaddr *) &svaddr,
			sizeof(struct sockaddr_in)) == -1)
		errExit("bind");

	if (listen(sfd, BACKLOG) == -1)
		errExit("listen");

	for (;;) {
		len = sizeof(struct sockaddr_in);
		cfd = accept(sfd, (struct sockaddr *) &claddr, &len);

		if (cfd == -1) 
			errExit("accept");

		if (inet_ntop(AF_INET, &claddr.sin_addr, claddrStr,
					INET_ADDRSTRLEN) == NULL)
			printf("cannot convert client addr\n");
		else
			printf("RECEIVING FROM (%s, %u)\n", 
					claddrStr, ntohs(claddr.sin_port));

		if (readLine(cfd, pathname, PATH_MAX) <= 0) {
			close(cfd);
			continue;
		}

		printf("pathname : %s\n", pathname);
		dirlst_t * dl = to_dirlst(pathname);
		if (write64b(cfd, dl->len) <= 0)
			errExit("write total");

		for (mdirent_t * mdp = dl->head; mdp; mdp = mdp->m_next) {
			if (write64b(cfd, mdp->m_mtime_rem) <= 0) 
				errExit("write time");
			
			if (write(cfd, mdp->m_name, mdp->m_name_len) != (ssize_t) mdp->m_name_len)
				errExit("write name");

			if (write(cfd, "\n", 1) != 1)
				errExit("write newline");
		}

		if (close(cfd) == -1)
			errExit("close");
	}

	return 0;
}
