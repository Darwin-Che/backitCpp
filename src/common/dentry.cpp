#include "dentry.h"
#include <sstream>

void print_dirvec(const dirvec_t & dv) {
	static char mtime_loc[30];
	static char mtime_rem[30];
	unsigned count = 0;
	printf("Printing the dirlst :\n");
	for (mdirent_t * mdp : dv.arr) {
		prtime(mtime_loc, &mdp->m_mtime_loc, 30);
		prtime(mtime_rem, &mdp->m_mtime_rem, 30);
		printf("%-10s  |  %s  |  %s\n", mdp->m_name, mtime_loc, mtime_rem);
		++count;
	}
	printf("Length = %u. Count = %zu. \n\n", count, dv.arr.size());

}

dirvec_t to_dirvec(const char * pathname) {
	dirvec_t dvec;
	DIR * dirp = opendir(pathname);
	if (dirp == nullptr) 
		errExit("opendir");
	dirent * dp;

	// preparing filename prefix
	struct stat st;
	size_t pathlen = strlen(pathname);
	char * filename = new char[pathlen + NAME_MAX + 2];
	strcpy(filename, pathname);
	filename[pathlen++] = '/';

	mdirent_t * mdp;
	for(;;) {
		errno = 0;
		dp = readdir(dirp);
		if (dp == NULL)
			break;
		if (strcmp(dp->d_name, ".") == 0 || strcmp(dp->d_name, "..") == 0) 
			continue;

		mdp = new mdirent_t;
		// file name
		strcpy(mdp->m_name, dp->d_name);
		mdp->m_name_len = strlen(mdp->m_name);
		// read mtime
		strcpy(filename + pathlen, dp->d_name);
		memset(&st, 0x0, sizeof(st));
		if (stat(filename, &st) == -1)
			errExit("stat a file");
		mdp->m_mtime_loc = st.st_mtime;
		mdp->m_mtime_rem = mtime_null;
		// add to vector
		dvec.arr.push_back(mdp);
	}
	delete[] filename;
	return dvec;
}

// should only be used temporarily by comb_loc_rem, 
// drawback: allocated a copy of m_name, which could be large
// replace: sort the dirvec_t to combine without extra memory usage
static dirtbl_t to_dirtbl(const dirvec_t & vec) {
	dirtbl_t tbl;

	for (mdirent_t * mdp : vec.arr)
		tbl.data[mdp->m_name] = mdp;

	return tbl;
}

dircomb_t comb_loc_rem(dirvec_t inloc, dirvec_t inrem)
{
	dircomb_t dirdata;

	// always make inloc table
	dirtbl_t tbl = to_dirtbl(inrem); 
	std::map<std::string, mdirent_t *>::iterator it;
	for (mdirent_t * mdp : inloc.arr) {
		if ((it = tbl.data.find(mdp->m_name)) != tbl.data.end()) {
			mdirent_t * mdp_copy = new mdirent_t;
			memcpy(mdp_copy, mdp, sizeof(mdirent_t));
			mdp_copy->m_mtime_rem = (*it).second->m_mtime_rem;
			dirdata.sync.arr.push_back(mdp_copy);
			tbl.data.erase(mdp->m_name);
		} else {
			mdirent_t * mdp_copy = new mdirent_t;
			memcpy(mdp_copy, mdp, sizeof(mdirent_t));
			dirdata.loc.arr.push_back(mdp_copy);
		}
	}
	for (auto p : tbl.data) {
		mdirent_t * mdp_copy = new mdirent_t;
		memcpy(mdp_copy, p.second, sizeof(mdirent_t));
		dirdata.rem.arr.push_back(mdp_copy);
	}

	return dirdata;
}
