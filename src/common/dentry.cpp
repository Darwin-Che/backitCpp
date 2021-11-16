#include "dentry.h"
#include <sstream>

void print_dirlst(dirlst_t * dl) {
	static char mtime_loc[30];
	static char mtime_rem[30];
	unsigned count = 0;
	printf("Printing the dirlst :\n");
	for (mdirent_t * mdp = dl->head; mdp; mdp = mdp->m_next) {
		prtime(mtime_loc, &mdp->m_mtime_loc);
		prtime(mtime_rem, &mdp->m_mtime_rem);
		printf("%-10s  |  %s  |  %s\n", mdp->m_name, mtime_loc, mtime_rem);
		++count;
	}
	printf("Length = %u.\n\n", count);

}

dirlst_t * to_dirlst(char * pathname) {
	dirlst_t * lst = new dirlst_t;
	lst->head = nullptr;
	lst->len = 0;
	mdirent_t ** tail = &lst->head;
	DIR * dirp = opendir(pathname);
	dirent * dp;
	
	// reading mtime
	struct stat st;
	int pathlen = strlen(pathname);
	char * filename = new char[pathlen + NAME_MAX + 2];
	strcpy(filename, pathname);
	filename[pathlen++] = '/';

	if (dirp == nullptr) {
		std::runtime_error("opendir\n");
	}
	for(;;) {
		errno = 0;
		dp = readdir(dirp);
		if (dp == NULL)
			break;
		if (strcmp(dp->d_name, ".") == 0 || strcmp(dp->d_name, "..") == 0) 
			continue;

		*tail = new mdirent_t;
		// file name
		strcpy((*tail)->m_name, dp->d_name);
		(*tail)->m_name_len = strlen((*tail)->m_name);
		// read mtime
		strcpy(filename + pathlen, dp->d_name);
		memset(&st, 0x0, sizeof(st));
		if (stat(filename, &st) == -1)
			std::runtime_error("stat\n");
		(*tail)->m_mtime_loc = (*tail)->m_mtime_rem = st.st_mtime;
		// add list len
		++lst->len;
		
		tail = &(*tail)->m_next;
	}
	(*tail) = nullptr;
	delete[] filename;

	return lst;
}

dirtbl_t * to_dirtbl(dirlst_t * lst) {
	dirtbl_t * tbl = new dirtbl_t;
	tbl->data = std::map<std::string, mdirent_t *>();

	for (mdirent_t * mdp = lst->head; mdp; mdp = mdp->m_next)
		tbl->data[mdp->m_name] = mdp;

	return tbl;
}

void comb_loc_rem(
		dirlst_t * inloc, 
		dirlst_t * inrem,
		dirlst_t ** outloc,
		dirlst_t ** outrem,
		dirlst_t ** outsync)
{
	// allocates the return structures
	*outloc = new dirlst_t;
	*outrem = new dirlst_t;
	*outsync = new dirlst_t;
	// the tail to append to
	mdirent_t ** outloc_tail = &(*outloc)->head;
	mdirent_t ** outrem_tail = &(*outrem)->head;
	mdirent_t ** outsync_tail = &(*outsync)->head;

	// always make inloc table
	dirtbl_t * tbl = to_dirtbl(inrem); 
	std::map<std::string, mdirent_t *>::iterator it;
	for (mdirent_t * mdp = inloc->head; mdp; mdp = mdp->m_next) {
		if ((it = tbl->data.find(mdp->m_name)) != tbl->data.end()) {
			*outsync_tail = new mdirent_t;
			memcpy(*outsync_tail, mdp, sizeof(mdirent_t));
			(*outsync_tail)->m_mtime_rem = (*it).second->m_mtime_rem;
			outsync_tail = &(*outsync_tail)->m_next;
			tbl->data.erase(mdp->m_name);
		} else {
			*outloc_tail = new mdirent_t;
			memcpy(*outloc_tail, mdp, sizeof(mdirent_t));
			outloc_tail = &(*outloc_tail)->m_next;
		}
	}
	for (auto p : tbl->data) {
		*outrem_tail = new mdirent_t;
		memcpy(*outrem_tail, p.second, sizeof(mdirent_t));
		outrem_tail = &(*outrem_tail)->m_next;
	}
	*outloc_tail = nullptr;
	*outrem_tail = nullptr;
	*outsync_tail = nullptr;

	delete tbl;
}