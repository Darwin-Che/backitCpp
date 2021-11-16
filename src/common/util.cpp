#include "header.h"

void errExit(const char * msg) {
	printf("%s\n", msg);
	exit(1);
}

ssize_t readLine(int fd, void * buffer, size_t n) {
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
	return totRead;
}
	
int read64b(int fd, uint64_t * buffer) {
	uint32_t hilo[2];
	char * buf = (char *) &hilo[0];
	size_t toRead = sizeof(hilo);
	size_t numRead;

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
