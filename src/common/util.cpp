#include "header.h"

void errExit(const char * msg) {
	printf("%s\n", msg);
	exit(1);
}

/* Return values:
 * -1 : fails
 * >=0  : number of bytes read
 */
ssize_t bi_readn(int fd, void * buffer, size_t sz) {
	char * buf = (char *) buffer;
	ssize_t totRead = 0;
	ssize_t numRead;

	if (buf == NULL) {
		errno = EINVAL;
		return -1;
	}

	while (sz > 0) {
		numRead = read(fd, buf, sz);

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
ssize_t bi_writen(int fd, const void * buffer, size_t sz) {
	char * buf = (char *) buffer;
	ssize_t totWrite = 0;
	ssize_t numWrite;

	if (buf == NULL) {
		errno = EINVAL;
		return -1;
	}

	while (sz > 0) {
		numWrite = write(fd, buf, sz);

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
int bi_sync_read(int fd, char*** filenames, size_t* numfiles) {
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
int bi_sync_write(int fd, char const * const * filenames, size_t numfiles) {
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

// int bi_files_read(int fd) {

// }

// int bi_files_write(int fd) {

// }