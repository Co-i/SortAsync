#pragma once
#include<exception>
#include<algorithm>
#include<atomic>
#include<iostream>
#include"ThreadsPool.h"
#include"Random.h"

struct Promise : std::promise<void> {
	~Promise() { set_value(); }
};

template<typename T>
void Sort(T* first, T* last, ThreadsPool& pool, size_t threshold, std::shared_ptr<Promise> pro);

template<typename T>
T* Partition(T* first, T* last);

template<typename T>
std::future<void> SortAsync(T* begin, T* end, ThreadsPool& pool, size_t threshold = 10000) 
{
	if (begin == nullptr || end <= begin)
		throw new std::exception("invalid pointers to SortAsync");
	if (threshold == 0)
		threshold = 1;
	auto pro = std::make_shared<Promise>();
	pool.Enqueue_NoRet(Sort<T>, begin, end - 1, std::ref(pool), threshold, pro);
	return pro->get_future();
}

template<typename T>
void Sort(T* first, T* last, ThreadsPool& pool, size_t threshold, std::shared_ptr<Promise> pro) {
	if (last < first + threshold) {
		std::sort(first, last + 1);
	}
	else {
		auto pivot = Partition<T>(first, last);
		if (first < pivot) {
			pool.Enqueue_NoRet(Sort<T>, first, pivot - 1, std::ref(pool), threshold, pro);
		}
		if (pivot < last) {
			pool.Enqueue_NoRet(Sort<T>, pivot + 1, last, std::ref(pool), threshold, pro);
		}
	}
	return;
}

template<typename T>
T* Partition(T* first, T* last) {
	using std::swap;
	swap(*(first + randomNum(0, last - first)), *last);
	T* larger = first;
	for (T* smaller = first; smaller < last; ++smaller) {
		if (*smaller < *last) {
			if (smaller != larger)
				swap(*larger, *smaller);
			++larger;
		}
	}
	swap(*larger, *last);
	return larger;
}