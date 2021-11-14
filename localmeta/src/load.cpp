#include <cstring>
#include "client.h"
#include <fcntl.h>
#include <cstdio>
#include <errno.h>

super_t * super_t::data;
mdt_t * mdt_t::data;
dirmap_t * dirmap_t::data;
fname_t * fname_t::data;

const char super_t::dir[] = ".backit/super";
const char mdt_t::dir[] = ".backit/mdt";
const char dirmap_t::dir[] = ".backit/dirmap";
const char fname_t::dir[] = ".backit/fname";


static void fail_mmap(const char * dir) {
	printf("mmap fail %s\n", dir);
	throw std::runtime_error("mmap fail\n");
}

char *create_mmap(const char * dir, size_t bsize) {
	struct stat st = {0};
	if (stat(dir, &st) != -1) {
		printf("overwriting %s\n", dir);
		chmod(dir, S_IRWXU|S_IRWXG);
	}
	int fd = open(dir, O_RDWR|O_CREAT|O_TRUNC, S_IRWXU|S_IRWXU);
	ftruncate(fd, bsize);
	char *res = (char *) mmap(nullptr, bsize, PROT_READ|PROT_WRITE, 
				MAP_FILE|MAP_SHARED, fd, 0);
	close(fd);
	if (res == MAP_FAILED) fail_mmap(dir);
	return res;
}

void init_create() {
	int init_size = 4;

	// check .backit/ is created
	struct stat st = {0};
	if (stat(".backit", &st) == -1) {
		printf("creating .backit/ ... \n");
		mkdir(".backit", 0777);
	} else if (!S_ISDIR(st.st_mode)) {
		throw std::runtime_error(".backit is not a dir! \nAbort\n");
	}

	// create super
	super_t::data = (super_t *) create_mmap(".backit/super", sizeof(super_t));
	super_t::data->fname_sz = init_size;
	super_t::data->mdt_sz = init_size;
	super_t::data->dirmap_sz = init_size;
	// create mdt file
	mdt_t::data = (mdt_t *) create_mmap(".backit/mdt", init_size * sizeof(mdt_t));
	// create dirmap file
	dirmap_t::data = (dirmap_t *) create_mmap(".backit/dirmap", init_size * sizeof(dirmap_t));
	// create fname file
	fname_t::data = (fname_t *) create_mmap(".backit/fname", init_size * sizeof(fname_t));

	// the root of .backit
	mdt_t::data[0].m_fname = 0;
	mdt_t::data[0].m_is_dir = 1;
	mdt_t::data[0].m_parent = MIDX_MAX;
	// mdt_t::data[0].m_sync_status = ;
	// mdt_t::data[0].m_sync_time = ;
	mdt_t::data[0].m_dirmap.init();
	getcwd(fname_t::data[0].fname, fname_t::MAXLEN);

	// init the frlst
	super_t::data->dirmap_fl.init();
	super_t::data->mdt_fl.init();
	super_t::data->fname_fl.init();
	super_t::data->dirmap_fl.insert_tail(0);
	for (unsigned i = 1; i < init_size; ++i) {
		super_t::data->dirmap_fl.insert_tail(i);
		super_t::data->mdt_fl.insert_tail(i);
		super_t::data->fname_fl.insert_tail(i);
	}
}

char *load_mmap(const char * dir, size_t bsize) {
	struct stat st = {0};
	if (stat(dir, &st) == -1) {
		printf("no %s found, creating\n", dir);
		return create_mmap(dir, bsize);
	}
	if (st.st_size != bsize) {
		printf("%s : file size != super's info\n", dir);
		throw std::runtime_error("corrupted\n");
	}
	int fd = open(dir, O_RDWR|O_CREAT);
	char *res = (char *) mmap(nullptr, bsize, PROT_READ|PROT_WRITE, 
				MAP_FILE|MAP_SHARED, fd, 0);
	if (res == MAP_FAILED) fail_mmap(dir);
	return res;
}

void load(const char * rootdir) {
	size_t slen = strlen(rootdir);
	char * str = new char[slen + 32];
	strcpy(str, rootdir);
	// read super
	strcpy(str + slen, ".backit/super");
	super_t::data = (super_t *) load_mmap(str, sizeof(super_t));
	// read mdt
	strcpy(str + slen, ".backit/mdt");
	mdt_t::data = (mdt_t *) load_mmap(str, super_t::data->mdt_sz * sizeof(mdt_t));
	// read dirmap
	strcpy(str + slen, ".backit/dirmap");
	dirmap_t::data = (dirmap_t *) load_mmap(str, super_t::data->dirmap_sz * sizeof(dirmap_t));
	// read fname
	strcpy(str + slen, ".backit/fname");
	fname_t::data = (fname_t *) load_mmap(str, super_t::data->fname_sz * sizeof(fname_t));
}

void unload() {
	munmap(mdt_t::data, super_t::data->mdt_sz * sizeof(mdt_t));
	munmap(dirmap_t::data, super_t::data->dirmap_sz * sizeof(dirmap_t));
	munmap(fname_t::data, super_t::data->fname_sz * sizeof(fname_t));
	munmap(super_t::data, sizeof(super_t));
}

void resize_mmap(const char * dir, void ** ptr, size_t oldsz, size_t newsz) {
	munmap(*ptr, oldsz);
	int fd = open(dir, O_RDWR|O_CREAT);
	ftruncate(fd, newsz);
	char * res = (char *) mmap(nullptr, newsz, PROT_READ|PROT_WRITE, 
				MAP_FILE|MAP_SHARED, fd, 0);
	if (res == MAP_FAILED) fail_mmap(dir);
	*ptr = res;
}



