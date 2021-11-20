#include "header.h"
#include "dentry.h"
#include "server.h"

#define BACKLOG 3

int main(int argc, char *argv[])
{
	struct sockaddr_in svaddr, *claddr;
	int sfd, optval, cfd;
	socklen_t cllen;

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
		claddr = new struct sockaddr_in;
		cllen = sizeof(struct sockaddr_in);
		cfd = accept(sfd, (struct sockaddr *) claddr, &cllen);

		if (cfd == -1) 
			errExit("accept");
		
		std::thread entry_thread(sv_entry, cfd, claddr, cllen);
		entry_thread.detach();
	}

	return 0;
}
