#include "client.h"
#include "header.h"
#include "dentry.h"

/* Given an absolute path, find closest ancestor that is a backit repo
 * Return nullptr if fail
 * Return a heap allocated string containing the relative path to that ancestor
 * Modify the input so that the input is the path to that ancestor
 */
char * cl_getrepopath(char * abspath, bool set_repoabs, char * ret) {
	if (ret == nullptr) ret = new char[PATH_MAX + 1]; // use ret also for constructing paths
	strcpy(ret, abspath);
	size_t e = strlen(abspath); // the strlen absolute path
	struct stat st;

	while (e >= 0) {
		ret[e] = '/';
		strcpy(&ret[e+1], META_DIR);
		memset(&st, 0x0, sizeof(struct stat));
		if (stat(ret, &st) != -1)
			break;
		
		if (e == 0) {
			// already checked all of ancestors
			delete[] ret;
			return nullptr;
		}

		while (--e >= 0 && ret[e] != '/') ;
	}

	if (e < 0) {
		delete[] ret;
		return nullptr;
	}

	// Populate abspath and ret

	if (e == strlen(abspath)) {
		// Special Case : ancestor is abspath
		strcpy(ret, ".");
	} else {
		strcpy(ret, &abspath[e+1]);
	}

	if (set_repoabs) {
		if (e == 0) {
			// Special Case : ancestor is '/'
			abspath[e+1] = '\0';
		} else {
			abspath[e] = '\0';
		}
	}

	return ret;
}

