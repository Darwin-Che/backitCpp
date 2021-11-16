#include <dirent.h>
#include <sys/stat.h>
#include <string>
#include <cstring>
#include <map>

// in memory struct for directory entries
struct mdirent_t {
	mdirent_t *	m_next;
	time_t		m_mtime_loc;
	time_t		m_mtime_rem;
	char			m_name[NAME_MAX];
};

// a collection of mdirent_t 
// support insert, search[, delete] based on filenames
struct dirtbl_t {
	std::map<std::string, mdirent_t *>	data;
};

struct dirlst_t {
	mdirent_t	* 	head;
};

// print dirlst
void print_dirlst(dirlst_t * dl);

// convert into dirlst
dirlst_t * to_dirlst(char * pathname);

// construct table
dirtbl_t * to_dirtbl(dirlst_t *);

void comb_loc_rem(
		dirlst_t * inloc, 
		dirlst_t * inrem,
		dirlst_t ** outloc,
		dirlst_t ** outrem,
		dirlst_t ** synclst);