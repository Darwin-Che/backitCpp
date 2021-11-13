#include "client.h"

// write to depot_dir
int find_depot(char * depot_dir) {
	struct stat st = {0};

	getcwd(depot_dir, fname_t::MAXLEN - 8);
	int i = strlen(depot_dir);
	depot_dir[i] = '/';
	depot_dir[i+1] = 0;
	for (; i >= 0; --i) {
		if (depot_dir[i] == '/') {
			strcpy(depot_dir + i + 1, ".backit");
			if (stat(depot_dir, &st) != -1) {
				break;
			}
		}
	}
	if (i < 0) {
		printf("Please init first\n");
		throw std::runtime_error("Error finding depot dir\n");
	}
	depot_dir[i+1] = 0;
	printf("Using depot dir : %s\n", depot_dir);
	return 0;
}

void print_mdt(midx<mdt_t> mi, unsigned depth) {
	for (unsigned i=0; i<depth; ++i) {
		printf("|-");
	}
	printf("%c %s\n", ptr(mi)->m_is_dir ? '|' : '-', ptr(ptr(mi)->m_fname)->fname);
	midx<dirmap_t> hd,tl;
	tl = ptr(mi)->m_dirmap.tail;
	hd = ptr(mi)->m_dirmap.head;
	if (ptr(mi)->m_is_dir && ptr(mi)->m_dirmap.size != 0) {
		while (true) {
			print_mdt(ptr(hd)->d_mdtpos, depth+1);
			if (hd == tl) break;
			hd = ptr(hd)->next;
		}
	}
}


