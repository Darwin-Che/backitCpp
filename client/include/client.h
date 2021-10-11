#ifndef CLIENT_H
#define CLIENT_H

#include <ctime>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdexcept>
#include <climits>

/*************
 * new types *
 *************/

#define MIDX_MAX 	ULONG_MAX
template<typename T>
struct midx {
	size_t 		idx;
	midx() : idx(MIDX_MAX) {}
	midx(const size_t u) : idx(u) {}
	midx<T>& operator=(const size_t u) {idx = u; return *this;}
};

template<typename T>
bool operator==(const midx<T> & i1, const midx<T> & i2) {
	return i1.idx == i2.idx;
}

enum SyncStatus {UNTRACKED, LOCAL, REMOTE, 
				SYNC, SYNCLESS, SYNCMORE};

/***********************
 * forward declaration *
 ***********************/

template<typename T>
struct lst_a;
template<typename T, typename A>
struct list_t;
template<typename T>
struct frlst_a;
template<typename T>
struct memctrl_t;
struct app_t;

struct super_t;
struct mdt_t;
struct dirmap_t;
struct fname_t;

/* funcs */

void load();
void unload();
void init_create();
void resize_mmap(const char * dir, void ** ptr, size_t oldsz, size_t newsz);

/********************
 * mindex functions *
 ********************/

template<typename T>
T * ptr(midx<T> i) {
	return &T::data[i.idx];
}

template<typename T> 
midx<T> idx(T * p) {
	return (p - T::data) / sizeof(T);
}

/*******************
 * shortcuts macro *
 *******************/

using mdt_idx = midx<mdt_t>;
using dirmap_idx = midx<dirmap_t>;
using fname_idx = midx<fname_t>;

/**********************
 * doubly linked list *
 **********************/

template<typename T>
struct lst_a {
	static midx<T> & prev(T* t) {
		return t->prev;
	}
	static midx<T> & next(T* t) {
		return t->next;
	}
	static midx<T> & prev(midx<T> t) {
		return ptr<T>(t)->prev;
	}
	static midx<T> & next(midx<T> t) {
		return ptr<T>(t)->next;
	}
};

template<typename T, typename A = lst_a<T>>
struct list_t {
	int 		size;
	midx<T>	head;
	midx<T>	tail;
	list_t();
	void init();
	void insert_head(midx<T> ti);
	void insert_after(midx<T> pi, midx<T> ti);
	void insert_before(midx<T> pi, midx<T> ti);
	void insert_tail(midx<T> ti);
	void remove(midx<T> ti);
};

#include "list.hpp"

/***********************
 * memory alloc & free *
 ***********************/

template<typename T>
struct frlst_a {
	static midx<T> & prev(T* t) {
		return t->frlst.prev;
	}
	static midx<T> & next(T* t) {
		return t->frlst.next;
	}
	static midx<T> & prev(midx<T> t) {
		return ptr<T>(t)->frlst.prev;
	}
	static midx<T> & next(midx<T> t) {
		return ptr<T>(t)->frlst.next;
	}
};

template<typename T>
struct memctrl_t {
	list_t<T, frlst_a<T>>		frlst;
	midx<T> 		tbend;
	public:
	int memalloc(midx<T> &);
	int memfree(midx<T> );
	int memexpand();
};

#include "mem.hpp"

/*********************
 * in memory structs *
 *********************/

struct app_t {
	memctrl_t<mdt_t> * 		mdt_memctrl;
	memctrl_t<dirmap_t> *	dirmap_memctrl;
	memctrl_t<fname_t> * 	fname_memctrl;

	app_t();
	~app_t();
	midx<mdt_t> add_track_unit(midx<mdt_t> dir, const char * str);
	midx<mdt_t> add_track_file(midx<mdt_t> dir, const char * str);
	midx<mdt_t> add_track_dir(midx<mdt_t> dir, const char * str);
};


/*******************
 * in file structs *
 *******************/

struct super_t {
	list_t<mdt_t, frlst_a<mdt_t>>				mdt_fl;
	list_t<dirmap_t, frlst_a<dirmap_t>>		dirmap_fl;
	list_t<fname_t, frlst_a<fname_t>>			fname_fl;
	unsigned long		mdt_sz;
	unsigned long 		dirmap_sz;
	unsigned long		fname_sz;

	static super_t * data;
	static const char dir[];
};

template<typename T>
struct frlst_t{
	midx<T>		prev;
	midx<T>		next;
};

struct dirmap_t {
	frlst_t<dirmap_t>		frlst;
	midx<dirmap_t>			prev;
	midx<dirmap_t>			next;
	midx<fname_t>			d_fname;
	midx<mdt_t>			d_mdtpos;

	static dirmap_t * data;
	static const char dir[];
};


struct mdt_t {
	frlst_t<mdt_t>		frlst;
	midx<mdt_t>		m_parent;
	midx<fname_t>		m_fname;
	bool				m_is_dir;
	list_t<dirmap_t>	m_dirmap;
	
	enum SyncStatus	m_sync_status;
	std::time_t		m_sync_time;

	static mdt_t * data;
	static const char dir[];
};

struct fname_t {
	static const size_t MAXLEN = 512;
	frlst_t<fname_t>		frlst;
	char					fname[MAXLEN];

	static fname_t * data;
	static const char dir[];
};



#endif
