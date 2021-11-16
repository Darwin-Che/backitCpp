#include "header.h"

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

		if (read(cfd, pathname, PATH_MAX) <= 0) {
			close(cfg);
			continue;
		}

		uint64_t curttime = 3;
		uint32_t hi_lo[2];
		hi_lo[0] = htonl(curttime >> 32);
		hi_lo[1] = htonl(curttime);
 
		if (write(cfd, hi_lo, sizeof(uint32_t)) != sizeof(uint32_t))
			errExit("write time");
		
		char msg[] = "filename1\n"
		if (write(cfd, msg, sizeof(msg)) != sizeof(msg))
			errExit("wrtie msg");


		if (write(cfd, "ab\n", 3) != 3)
			errExit("Error on Write");

		if (close(cfd) == -1)
			errExit("close");
	}

	return 0;
}
