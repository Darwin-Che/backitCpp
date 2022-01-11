#ifndef DENTRY_H
#define DENTRY_H

#include <dirent.h>
#include <sys/stat.h>
#include <string>
#include <cstring>
#include <map>
#include <vector>
#include "header.h"

const int64_t mtime_null = 0;

// in memory struct for directory entries
struct mdirent_t {
	int64_t		m_mtime_loc;
	int64_t		m_mtime_rem;
	size_t		m_name_len;
	char			m_name[NAME_MAX + 1];
};

// a collection of mdirent_t 
// support insert, search[, delete] based on filenames
struct dirtbl_t {
	std::map<std::string, mdirent_t *>	data;
};

struct dirvec_t {
	std::vector<mdirent_t *> arr;
};

struct dircomb_t {
	dirvec_t rem;
	dirvec_t loc;
	dirvec_t sync;
};

// print dirlst
void print_dirvec(const dirvec_t & dv);

dirvec_t to_dirvec(const char * pathname);

dircomb_t comb_loc_rem(dirvec_t inloc, dirvec_t inrem);

#endif
