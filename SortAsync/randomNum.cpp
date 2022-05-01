#include"Random.h"
#include<random>
#include<future>
#include<vector>
#include<iostream>

long long randomNum(long long min, long long max) {
	static std::default_random_engine e;
	return std::uniform_int_distribution<long long>(min,max)(e);
}

int* random_array(size_t sz, int min, int max) {
	int* ret = new int[sz];
	auto loadrandom = [min, max](int* begin, int* end) {
		for (int* curr = begin; curr != end; ++curr)
			*curr = int(randomNum(min, max));
	};
	auto max_threads = std::thread::hardware_concurrency();
	if (max_threads == 0)
		max_threads = 1;
	size_t part_size = sz / max_threads;
	if (part_size == 0) {
		loadrandom(ret, ret + sz);
	}
	else {
		std::vector<std::future<void>> tasks;
		tasks.reserve(max_threads);

		for (unsigned i = 0; i < max_threads; ++i)
			tasks.emplace_back(std::async(loadrandom, ret + (part_size * i), ret + (part_size * (size_t(i) + 1))));
		for (auto& task : tasks)
			task.wait();

		loadrandom(ret + sz - sz % max_threads, ret + sz);
	}
	return ret;
}