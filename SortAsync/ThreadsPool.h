#pragma once
#include <future>
#include <vector>
#include <queue>

class ThreadsPool
{
public:
	ThreadsPool(unsigned threads);
	template<typename Fn, typename ...Args>
	auto Enqueue(Fn&& fn, Args&&... args)->std::future<typename std::result_of<Fn(Args...)>::type>;
	template<typename Fn, typename ...Args>
	void Enqueue_NoRet(Fn&& fn, Args&&... args);
	~ThreadsPool();
private:
	std::vector<std::thread> Workers;
	std::queue<std::function<void()>> Tasks;

	std::mutex Mtx;
	bool bStop;
	std::condition_variable CV;
};

inline
ThreadsPool::ThreadsPool(unsigned threads) : bStop(false)
{
	for (unsigned i = 0; i < threads; ++i) {
		Workers.emplace_back(
			[this] {
				while (true) {
					std::function<void()> task;
					{
						std::unique_lock<std::mutex> lck(Mtx);
						CV.wait(lck, [this] { return bStop || !Tasks.empty(); });
						if (bStop && Tasks.empty())
							return;

						task = std::move(Tasks.front());
						Tasks.pop();
					}
					task();
				}
			}
		);
	}
}

inline ThreadsPool::~ThreadsPool()
{
	{
		std::unique_lock<std::mutex> lck(Mtx);
		bStop = true;
	}
	CV.notify_all();
	for (auto& e : Workers)
		e.join();
}

template<typename Fn, typename ...Args>
inline auto ThreadsPool::Enqueue(Fn&& fn, Args && ...args) -> std::future< typename std::result_of<Fn(Args...)>::type >
{
	using result_type = typename std::result_of<Fn(Args...)>::type;

	auto sp_task = std::make_shared<std::packaged_task<result_type()>>(
		std::bind(std::forward<Fn>(fn), std::forward<Args>(args)...)
		);
	{
		std::unique_lock<std::mutex> lck(Mtx);
		if (bStop)
			throw std::runtime_error("enqueue on stopped ThreadsPool");
		Tasks.emplace([sp_task] {(*sp_task)(); });
	}
	CV.notify_one();
	return sp_task->get_future();
}

template<typename Fn, typename ...Args>
inline void ThreadsPool::Enqueue_NoRet(Fn&& fn, Args && ...args)
{
	{
		std::unique_lock<std::mutex> lck(Mtx);
		if (bStop)
			throw std::runtime_error("enqueue on stopped ThreadsPool");
		Tasks.emplace(std::bind(std::forward<Fn>(fn), std::forward<Args>(args)...));
	}
	CV.notify_one();
	return;
}