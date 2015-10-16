#ifndef __COMMON_THREAD_POOL_HPP__
#define __COMMON_THREAD_POOL_HPP__

#include <algorithm>
#include <list>
#include <thread>

#include "doze.hpp"

namespace common {

class ThreadPool {
public:
	typedef std::function<void()> Task;

	ThreadPool() : work_doze(1000) {
		auto t = std::thread([this](){thread_timer();});
		timer_thread.swap(t);

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
		timer_doze.notify();
		work_doze.notify();
	}

	void post(Task task) {
		std::unique_lock<std::mutex> u(mutex);
		tasks.push_back(task);
		work_doze.notify();
	}

	void post_dealyed(int ms, Task task) {
		std::unique_lock<std::mutex> u(mutex);
		auto until = std::chrono::steady_clock::now() + std::chrono::milliseconds(ms);
		auto f = [=](const DelayTask& dt) { return dt.until >= until; };
		auto i = std::find_if(delay_tasks.begin(), delay_tasks.end(), f);
		delay_tasks.insert(i, DelayTask{until, task});
		timer_doze.notify();
	}

	void join() {
		if (timer_thread.joinable())
			timer_thread.join();

		if (work_thread.joinable())
			work_thread.join();
	}

private:
	struct DelayTask {
		std::chrono::steady_clock::time_point until;
		Task task;
	};

	void thread_timer() {
		while (!stopped) {
			timer_doze.wait_until(next_time_point());
			if (!stopped) {
				check_delay();
			}
		}
	}

	void thread_work() {
		while (!stopped) {
			work_doze.wait();
			check_work();
		}
	}

	void check_delay() {
		std::unique_lock<std::mutex> u(mutex);
		if (delay_tasks.empty()) {
			return;
		}

		auto now = std::chrono::steady_clock::now();
		while (!delay_tasks.empty() && delay_tasks.front().until <= now) {
			tasks.push_back(delay_tasks.front().task);
			delay_tasks.pop_front();
		}

		work_doze.notify();
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

	std::chrono::steady_clock::time_point next_time_point() {
		std::unique_lock<std::mutex> u(mutex);
		if (!delay_tasks.empty()) {
			return delay_tasks.front().until;
		} else {
			return std::chrono::steady_clock::now() + std::chrono::seconds(1);
		}
	}

	std::thread timer_thread;
	Doze timer_doze;

	std::thread work_thread;
	Doze work_doze;

	std::mutex mutex;
	std::list<DelayTask> delay_tasks;
	std::list<Task> tasks;

	bool stopped = false;
};

}

#endif // __COMMON_THREAD_POOL_HPP__
