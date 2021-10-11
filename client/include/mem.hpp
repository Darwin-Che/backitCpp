#include "client.h"

// how do we keep the freelist?
/* first free -> second free -> ... -> allfree (< tbend) and all others are allocated
 * so that if the frlst is not empty, 
 * then the head is free, and can be allocated, 
 * otherwise need to expand.
 * As for free, only loop until the index is bigger than i, 
 * then insert before that. */

template<typename T>
int memctrl_t<T>::memalloc(midx<T> & ip) {
	while(frlst.size == 0){
		memexpand();
	}
	ip = frlst.head;
	frlst.remove(frlst.head);
	return 0;
}

template<typename T>
int memctrl_t<T>::memfree(midx<T> i) {
	midx<T> h = frlst.head;
	int j = 0;
	while (true) {
		if (i.idx < h.idx) {
			frlst.insert_before(h, i);
			break;
		}
		++j;
		if (j >= frlst.size()) break;
		h = ptr<T>(h)->frlst.next;
	}
	if (j >= frlst.size()) {
		frlst.insert_tail(i);
	}
	return 0;
}

template<typename T>
int memctrl_t<T>::memexpand() {
	size_t oldsz = tbend.idx;
	size_t newsz = 2 * oldsz;
	printf("before cast : %p\n", &T::data);
	printf("after cast : %p\n", (void **) &(T::data));
	resize_mmap(T::dir, (void **) &(T::data), oldsz, newsz);
	
	// this is costly, need a better data structure
	// need to represent a block of free space without writing to every block
	for (size_t i = oldsz; i < newsz; ++i) {
		frlst.insert_tail(i);
	}

	tbend = newsz;
	return newsz;
}
