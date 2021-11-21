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
	if (inet_pton(AF_INET, "192.168.2.23", &svaddr.sin_addr) <= 0)
		errExit("fail translate address");

	if (connect(cfd, (struct sockaddr*) &svaddr, sizeof(struct sockaddr_in)) == -1)
		errExit("connect");
	
	printf("cl_connect succeed!\n");
	return cfd;
}

int cl_ls(int argc, char ** argv) {
	int cfd = cl_connect();

	char * rel_path = argv[1];
	ssize_t numRead;
	dirlst_t * lst = new dirlst_t;
	mdirent_t ** mdp = &lst->head;


	if (write64b(cfd, OP_SV_DIRLST) < 0) 
		errExit("failed write sv op");

	if (write(cfd, rel_path, strlen(rel_path)) != strlen(rel_path))
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
		to_dirlst(rel_path),
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
	
	ssize_t numfiles = 1;
	char * pathnames[1];
	pathnames[0] = argv[1];

	if (write64b(cfd, OP_SV_SYNC_DOWNLOAD) < 0) 
		errExit("failed write sv op");

	if (bi_sync_write(cfd, pathnames, numfiles) < 0)
		errExit("bi_sync_write fail");

	if (bi_files_read(cfd) < 0)
		errExit("bi_files_read fail");

	return 0;
}
