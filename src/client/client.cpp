#include "header.h"
#include "dentry.h"

int main(int argc, char *argv[])
{
	struct sockaddr_in svaddr;
	int sfd;
	ssize_t numRead;
	dirlst_t * lst = new dirlst_t;
	mdirent_t ** mdp = &lst->head;

	if (argc < 3 || strcmp(argv[1], "--help") == 0)
		errExit("usage error");

	sfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sfd == -1)
		errExit("socket");

	memset(&svaddr, 0, sizeof(struct sockaddr_in));
	svaddr.sin_family = AF_INET;
	svaddr.sin_port = htons(PORT_NUM);
	if (inet_pton(AF_INET, argv[1], &svaddr.sin_addr) <= 0)
		errExit("fail translate addres");

	if (connect(sfd, (struct sockaddr*) &svaddr, sizeof(struct sockaddr_in)) == -1)
		errExit("connect");

	if (write(sfd, argv[2], strlen(argv[2])) != strlen(argv[2]))
		errExit("failed write str");
	if (write(sfd, "\n", 1) != 1)
		errExit("failed write newline");

	uint64_t curttime;
	if (read64b(sfd, &curttime) < 0) 
		errExit("read time");

	printf("time is %llu\n", curttime);

	char msg[100];
	numRead = readLine(sfd, msg, sizeof(msg));
	if (numRead == -1)
		errExit("readLine");
	if (numRead == 0)
		errExit("Unexpect EOF");

	printf("name is %s\n", msg);

	return 0;
}
