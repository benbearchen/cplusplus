#ifndef __COMMON_THREAD_POOL_HPP__
#define __COMMON_THREAD_POOL_HPP__

#include <list>
#include <thread>

#include "doze.hpp"

namespace common {

template <typename Task = std::function<void()>>
class ThreadPool {
public:
	ThreadPool() : work_doze(1000) {
		auto w = std::thread([this](){thread_work();});
	       	work_thread.swap(w);
	}

	~ThreadPool() {
		stop();
		join();
	}

	ThreadPool(const ThreadPool&) = delete;
	ThreadPool& operator=(const ThreadPool&) = delete;

	void stop() {
		stopped = true;
		work_doze.notify();
	}

	void post(Task task) {
		std::unique_lock<std::mutex> u(mutex);
        if (tasks.empty()) {
            work_doze.notify();
        }

		tasks.push_back(task);
	}

	void join() {
		if (work_thread.joinable())
			work_thread.join();
	}

private:
	void thread_work() {
		while (!stopped) {
			work_doze.wait();
			check_work();
		}
	}

	void check_work() {
		while (!stopped) {
			Task task;
			{
				std::unique_lock<std::mutex> u(mutex);
				if (tasks.empty()) {
					return;
				}

				task = tasks.front();
				tasks.pop_front();
			}

			if (task) {
				task();
			}
		}
	}

	std::thread work_thread;
	Doze work_doze;

	std::mutex mutex;
	std::list<Task> tasks;

	bool stopped = false;
};

}

#endif // __COMMON_THREAD_POOL_HPP__
