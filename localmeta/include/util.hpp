#include "client.h"

template<typename T>
T * ptr(midx<T> i) {
	return &T::data[i.idx];
}

template<typename T> 
midx<T> idx(T * p) {
	return (p - T::data) / sizeof(T);
}
