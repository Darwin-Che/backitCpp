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
		errExit("fail translate address");

	if (connect(sfd, (struct sockaddr*) &svaddr, sizeof(struct sockaddr_in)) == -1)
		errExit("connect");

	if (write64b(sfd, OP_SV_DIRLST) < 0) 
		errExit("failed write sv op");

	if (write(sfd, argv[2], strlen(argv[2])) != strlen(argv[2]))
		errExit("failed write pathname");
	if (write(sfd, "\n", 1) != 1)
		errExit("failed write newline");

	uint64_t totalNum;
	if(read64b(sfd, &totalNum) < 0)
		errExit("read total");
	
	printf("Number of Entries: %llu\n", totalNum);
	lst->len = totalNum;

	uint64_t mtime_rem;
	for (; totalNum > 0; --totalNum) {
		*mdp = new mdirent_t;

		if (read64b(sfd, &mtime_rem) < 0) 
			errExit("read time");
		(*mdp)->m_mtime_loc = (*mdp)->m_mtime_rem = mtime_rem;

		numRead = readLine(sfd, (*mdp)->m_name, NAME_MAX);
		if (numRead == -1)
			errExit("readLine");
		if (numRead == 0)
			errExit("Unexpect EOF");

		mdp = &(*mdp)->m_next;
	}
	*mdp = nullptr;

	dirlst_t *loclst, *remlst, *synclst;
	comb_loc_rem(
		to_dirlst("."),
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
