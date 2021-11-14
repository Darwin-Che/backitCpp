#include "client.h"

/* Doubly-Linked List 
 * Inplemented by a template class
 * T : the content of the list
 * A : how to access prev/next in T (a reference)
 * midx<T> prev -- A::prev(T* t)
 * midx<T> next -- A::next(T* t)
 * midx<T> prev -- A::prev(midx<T> t)
 * midx<T> next -- A::next(midx<T> t)
 */

template<typename T, typename A>
list_t<T,A>::list_t() 
	: size(0), head(MIDX_MAX), tail(MIDX_MAX){
}

template<typename T, typename A>
void list_t<T,A>::init() {
	size = 0;
	head = tail = MIDX_MAX;
}

template<typename T, typename A>
void list_t<T,A>::insert_head(midx<T> ti) {
	T* tp = ptr<T>(ti);
	A::prev(tp) = ti;
	if (size == 0) {
		head = tail = A::next(tp) = ti;
	} else {
		A::next(tp) = head;
		head = A::prev(head) = ti;
	}
	size += 1;
}

template<typename T, typename A>
void list_t<T,A>::insert_after(midx<T> pi, midx<T> ti) {
	T* tp = ptr<T>(ti);
	T* pp = ptr<T>(pi);
	A::prev(tp) = pi;
	if (tail == pi) {
		A::next(pp) = A::next(tp) = ti;
	} else {
		A::next(tp) = A::next(pp);
		A::next(pp) = A::prev(A::next(pp)) = ti;
	}
	size += 1;
}

template<typename T, typename A>
void list_t<T,A>::insert_before(midx<T> pi, midx<T> ti) {
	T* tp = ptr<T>(ti);
	T* pp = ptr<T>(pi);
	A::next(tp) = pi;
	if (head == pi) {
		A::prev(pp) = A::prev(tp) = ti;
	} else {
		A::prev(tp) = A::prev(pp);
		A::prev(pp) = A::next(A::prev(pp)) = ti;
	}
	size += 1;
}

template<typename T, typename A>
void list_t<T,A>::insert_tail(midx<T> ti) {
	T* tp = ptr<T>(ti);
	A::next(tp) = ti;
	if (size == 0) {
		head = tail = A::prev(tp) = ti;
	} else {
		A::prev(tp) = tail;
		tail = A::next(tail) = ti;
	}
	size += 1;
}

template<typename T, typename A>
void list_t<T,A>::remove(midx<T> ti) {
	T* tp = ptr<T>(ti);
	if (head == ti && tail == ti) { // head & tail
		size = 0;
		head = tail = MIDX_MAX;
	} else if (head == ti) { // head
		A::prev(head) = head = A::next(tp);
	} else if (tail == ti) { // tail
		A::next(tail) = tail = A::prev(tp);
	} else {
		A::prev(A::next(tp)) = A::prev(tp);
		A::next(A::prev(tp)) = A::next(tp);
	}
	A::prev(tp) = A::next(tp) = MIDX_MAX;
	size -= 1;
}

