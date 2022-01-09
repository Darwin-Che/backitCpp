#include "header.h"

const char * log_path = "log.txt";
FILE * log_fptr = fopen(log_path, "w");

void errExit(const char * msg) {
	printf("%s\n", msg);
	exit(1);
}

/* Return -1 for error, strlen of return path for success
 * Modify the input path directly
 *
 * Operate on one component at a time: 
 * 1. compress consecutive / into one /
 * 2. remove the . component
 * 3. remove the xx/.. component
 * 4. remove trailing /
 */
#define IS_CPNEND(c) (c == '/' || c == '\0')
ssize_t canon_abspath(char * path) {
	// r always points to '/'
	size_t w = 0, r = 0, e; // write pos, read pos, read cpn end pos
	while (path[r]) {
		// e = end of read component
		for (e = r+1; ! IS_CPNEND(path[e]); ++e);

		if (e == r+1) { 
			// if the component is empty
			// do nothing

		} else if (e == r+2 && path[r+1] == '.') { 
			// if the component is equal to '.'
			// do nothing

		} else if (e == r+3 && path[r+1] == '.' && path[r+2] == '.') { 
			// if the component is equal to '..'
			for (--w; path[w] != '/'; --w) {
				if (w < 0) 
					errExit("invalid path -- \'..\'");
			}

		} else {
			memcpy(&path[w], &path[r], e-r);
			w += e-r;
		}
		
		r = e;
	}
	// the result is empty
	if (w == 0)
		path[w++] = '/';

	path[w] = '\0';

	return w;
}

/* Convert any path into an absolute path and return it
 * The result doesn't contain . or .., and doesn't expand symbolic link
 * caller shall free the return string
 */
char * normalize_path(const char * input, char * output) {
	if (output == nullptr) {
		output = new char[PATH_MAX];
	}
	size_t input_len = strlen(input); // input len
	const char * input_pos = input; // position to read
	char * output_pos = output; // position to write
	size_t output_len = PATH_MAX; // length left in output 
	size_t lentmp;

	// if input is relative path, then add curdir in the front
	if (input[0] != '/') {
		if (getcwd(output_pos, output_len) == NULL)
			errExit("cwd fail");
		lentmp = strlen(output_pos);
		output_pos += lentmp; output_len -= lentmp;
	}
	strcpy(output_pos, "/");
	output_pos += 1; output_len -= 1;

	strcpy(output_pos, input_pos);
	output_pos += input_len; output_len -= input_len;

	// canonicalize the absolute path
	if (canon_abspath(output) < 0)
		errExit("canon_abspath fail");

	return output;
}

void bi_pathcombine(char * path, const char * prefix) {
	size_t pathlen = strlen(path);
	size_t prefixlen = strlen(prefix);

	memcpy(&path[prefixlen + 1], &path[0], pathlen + 1);
	memcpy(&path[0], &prefix[0], prefixlen);
	path[prefixlen] = '/';
}

/* Return values:
 * -1 : fails
 * >=0  : number of bytes read
 */
ssize_t bi_readn(int fd, void * buffer, size_t sz, size_t perlimit) {
	char * buf = (char *) buffer;
	ssize_t totRead = 0;
	ssize_t numRead;

	if (buf == NULL) {
		errno = EINVAL;
		return -1;
	}

	if (perlimit <= 0)
		perlimit = sz;

	while (sz > 0) {
		numRead = read(fd, buf, std::min(sz, perlimit));

		if (numRead == -1) {
			if (errno == EINTR)
				continue;
			else
				return -1;

		} else if (numRead == 0) {
			break;
		
		} else {
			totRead += numRead;
			buf += numRead;
			sz -= numRead;
		}
	}

	return totRead;
}

/* Return values:
 * -1 : fails
 * >=0  : number of bytes written
 */
ssize_t bi_writen(int fd, const void * buffer, size_t sz, size_t perlimit) {
	char * buf = (char *) buffer;
	ssize_t totWrite = 0;
	ssize_t numWrite;

	if (buf == NULL) {
		errno = EINVAL;
		return -1;
	}

	if (perlimit <= 0)
		perlimit = sz;

	while (sz > 0) {
		numWrite = write(fd, buf, std::min(sz, perlimit));

		if (numWrite == -1) {
			if (errno == EINTR)
				continue;
			else
				return -1;

		} else if (numWrite == 0) {
			break;
		
		} else {
			totWrite += numWrite;
			buf += numWrite;
			sz -= numWrite;
		}
	}

	return totWrite;
}

/* Return values:
 * -1 : fails
 * 0  : case 1 : nothing is read, no newline appended; 
 *      case 2 : read one '\n' only
 * >0 : number of bytes read, excluding the endding '\n'
 */
ssize_t readLine(int fd, void * buffer, size_t n, bool rstrip) {
	ssize_t numRead;
	size_t totRead;
	char *buf;
	char ch;

	if (n <= 0 || buffer == NULL) {
		errno = EINVAL;
		return -1;
	}

	buf = (char *) buffer;
	totRead = 0;
	for(;;) {
		numRead = read(fd, &ch, 1);

		if (numRead == -1) {
			if (errno == EINTR)
				continue;
			else
				return -1;

		} else if (numRead == 0) {
			if (totRead == 0)
				return 0;
			else 
				break;
		
		} else {
			if (totRead < n-1) {
				totRead++;
				*buf++ = ch;
			}
			if (ch == '\n')
				break;
		}
	}
	*buf = '\0';
	if (rstrip && *(buf - 1) == '\n')
		*(buf - 1) = '\0';
	return totRead;
}

/* Return values:
 * -1 : fails
 * 8  : number of bytes read
 */
int read64b(int fd, uint64_t * buffer) {
	uint32_t hilo[2];
	char * buf = (char *) &hilo[0];
	size_t toRead = sizeof(hilo);
	ssize_t numRead;

	if (buffer == NULL) {
		errno = EINVAL;
		return -1;
	}

	while (toRead > 0) {
		numRead = read(fd, buf, toRead);

		if (numRead == -1) {
			if (errno == EINTR)
				continue;
			else
				return -1;

		} else if (numRead == 0) {
			return -1;
		
		} else {
			buf += numRead;
			toRead -= numRead;
		}
	}

	*buffer = ((((uint64_t) ntohl(hilo[0])) << 32) | ntohl(hilo[1]));

	return 8;
}

/* Return values:
 * -1 : fails
 * 8  : number of bytes written
 */
int write64b(int fd, uint64_t data) {
	uint32_t hi_lo[2];
	hi_lo[0] = htonl(data >> 32);
	hi_lo[1] = htonl(data);

	if (write(fd, hi_lo, sizeof(hi_lo)) != sizeof(hi_lo))
		return -1;

	return 8;
}

/* Allocates filenames on heap,
 * caller needs to free those
 *
 * Return values:
 * -1 : fails
 * 0 : success
 */
int bi_paths_read(int fd, char*** filenames, size_t* numfiles) {
	uint64_t nf;
	
	if (numfiles == nullptr)
		return -1;

	if (read64b(fd, &nf) < 0)
		return -1;
	
	*numfiles = nf;

	*filenames = new char*[*numfiles];

	for (uint64_t n = 0; n < nf; ++n) {
		(*filenames)[n] = new char[PATH_MAX + 1];
		if (readLine(fd, (*filenames)[n], PATH_MAX + 1, 1) <= 0)
			return -1;
	}

	return 0;
}

/* Return values:
 * -1 : fails
 * 0 : success
 */
int bi_paths_write(int fd, char const * const * filenames, size_t numfiles) {
	if (write64b(fd, numfiles) < 0) {
		return -1;
	}
	for (unsigned n = 0; n < numfiles; ++n) {
		if (bi_writen(fd, filenames[n], strlen(filenames[n])) < 0 ||
				bi_writen(fd, "\n", 1) < 0)
			return -1;
	}
	return 0;
}

int bi_socket_to_disk(int in_fd, int out_fd, uint64_t sz, uint64_t in_bufsz, uint64_t out_bufsz) {
	unsigned sbuf_shift, dbuf_shift;
	char * buffer;
	uint64_t bufsz, worksz; // the data sz in the buffer; = min(bufsz, sz)
	ssize_t res;

	// find the biggest power of 2 that is <= bufsz
	for (sbuf_shift = 1; (1U << sbuf_shift) <= in_bufsz; ++sbuf_shift);
	--sbuf_shift;
	in_bufsz = 1 << sbuf_shift;
	for (dbuf_shift = 1; (1U << dbuf_shift) <= out_bufsz; ++dbuf_shift);
	--dbuf_shift;
	out_bufsz = 1 << dbuf_shift;
	
	printf("in_bufsz : %llu, out_bufsz : %llu.\n", in_bufsz, out_bufsz);

	// create a buffer equal to the bigger of them
	bufsz = std::max(in_bufsz, out_bufsz);
	buffer = new char[bufsz];

	while (sz > 0) {
		worksz = std::min(sz, bufsz);

		// copy into buffer
		if ((res = bi_readn(in_fd, buffer, worksz, in_bufsz)) < 0)
			errExit("bi_readn fails");
		
		// copy out of buffer
		
		if ((res = bi_writen(out_fd, buffer, worksz, out_bufsz)) < 0)
			errExit("bi_writen fails");

		sz -= worksz;
	}

	delete[] buffer;
	return 0;
}

int bi_disk_to_socket(int in_fd, int out_fd, uint64_t sz, uint64_t in_bufsz, uint64_t out_bufsz) {
#ifdef __unix__
	off_t fileoffset = 0;
	ssize_t numsent = sendfile(out_fd, in_fd, &fileoffset, sz);
	if (numsent < 0 || (uint64_t) numsent != sz)
		errExit("sendfile partial");

#elif __APPLE__
#include <sys/syslimits.h>
	off_t fileoffset = sz;
	if (sendfile(in_fd, out_fd, 0, &fileoffset, NULL, 0) == -1) 
		errExit("sendfile fails");
	if (fileoffset != sz)
		errExit("sendfile partial");

#else
	errExit("sendfile not supported");

#endif
	return 0;
}

int bi_files_read(int fd) {
	uint64_t numfiles;
	uint64_t mtime;
	uint64_t filesz;
	char pathname[PATH_MAX + 1];
	int filefd;

	if (read64b(fd, &numfiles) < 0) 
		errExit("read numfiles");
	
	for (uint64_t n = 0; n < numfiles; ++n) {
		if (readLine(fd, pathname, sizeof(pathname)) <= 0) 
			errExit("read pathname");
		
		if (read64b(fd, &mtime) < 0) 
			errExit("read mtime");
		
		if (read64b(fd, &filesz) < 0)
			errExit("read filesz");
		
		filefd = open(pathname, O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU);

		if (filefd == -1)
			errExit("cannot open file");
		
		printf("writing to file %s : %llu bytes\n", pathname, filesz);
		if (bi_socket_to_disk(fd, filefd, filesz) < 0)
			errExit("downloading the file");
		
		close(filefd);
	}

	return 0;
}

int bi_files_write(int fd, char const * const * filenames, size_t numfiles) {
	struct stat st;
	int filefd;

	if (write64b(fd, numfiles) < 0) {
		return -1;
	}
	for (unsigned n = 0; n < numfiles; ++n) {
		if (bi_writen(fd, filenames[n], strlen(filenames[n])) < 0 ||
				bi_writen(fd, "\n", 1) < 0)
			return -1;
		
		memset(&st, 0x0, sizeof(struct stat));
		if (stat(filenames[n], &st) == -1)
			errExit("stat the file");
		if (write64b(fd, st.st_mtime) < 0)
			errExit("write the mtime");
		if (write64b(fd, st.st_size) < 0)
			errExit("write the filesz");
		
		filefd = open(filenames[n], O_RDONLY);
		if (bi_disk_to_socket(filefd, fd, st.st_size) < 0)
			errExit("upload the file");
		
	}
	return 0;
}
