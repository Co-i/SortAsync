#include"SortAsync.h"
#include"Random.h"
#include<iostream>
#include<list>

int main() {
	std::cout << "hardware_concurrency: " << std::thread::hardware_concurrency() << std::endl;
	ThreadsPool tp(std::thread::hardware_concurrency());

	using namespace std::chrono;
	steady_clock::time_point time;
	duration<double> span;
	int s = 10000000;
	std::list<duration<double>> list1, list2;

	for (int i = 0; i != 10; ++i) {
		auto iArray1 = random_array(s, -100000, 100000);
		auto iArray2 = new int[s];
		std::copy(iArray1, iArray1 + s, iArray2);
		std::cout << "start" << std::endl;

		time = steady_clock().now();
		auto task = SortAsync(iArray1, iArray1 + s, tp, 10000);
		task.get();
		span = duration_cast<duration<double>>(steady_clock().now() - time);
		list1.emplace_back(span);
		std::cout << span.count() << " seconds elapsed in multithreads process" << std::endl;

		time = steady_clock().now();
		std::sort(iArray2, iArray2 + s);
		span = duration_cast<duration<double>>(steady_clock().now() - time);
		list2.emplace_back(span);
		std::cout << span.count() << " seconds elapsed in singlethreads process" << std::endl;

		int CompareResult;
		for (int idx = 0; idx < s; ++idx) {
			CompareResult = iArray2[idx] - iArray1[idx];
			if (CompareResult != 0)
				std::cout << idx << " not equal\n";
			break;
		}
		std::cout << "compare result: " << CompareResult << "\n";
		std::cout << "speed rate: " << list2.back().count() / list1.back().count() << "\n";
		delete[]iArray1;
		delete[]iArray2;
	}
	duration<double> total1;
	for (auto e : list1)
		total1 += e;
	duration<double> total2;
	for (auto e : list2)
		total2 += e;
	std::cout << "average speed rate: " << total2.count() / total1.count() << "\n";
	std::cout << "press 'enter' to exit.\n";
	std::ignore = getchar();
	return 0;
}