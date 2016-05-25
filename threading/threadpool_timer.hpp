#ifndef __COMMON_THREAD_POOL_TIMER_HPP__
#define __COMMON_THREAD_POOL_TIMER_HPP__

#include <algorithm>
#include <list>
#include <map>
#include <thread>

#include "doze.hpp"

namespace common {

template <typename Task = std::function<void()>,
		  typename Pool = std::function<void(Task)>>
class ThreadPoolTimer {
public:
	ThreadPoolTimer(Pool pool) {
		this->pool = pool;

		auto t = std::thread([this](){thread_timer();});
		timer_thread.swap(t);
	}

	~ThreadPoolTimer() {
		stop();
		join();
	}

	ThreadPoolTimer(const ThreadPoolTimer&) = delete;
	ThreadPoolTimer& operator=(const ThreadPoolTimer&) = delete;

	void stop() {
		stopped = true;
		timer_doze.notify();
	}

	void post(int ms, Task task) {
		std::unique_lock<std::mutex> u(mutex);
		auto until = std::chrono::steady_clock::now() + std::chrono::milliseconds(ms);
		auto f = [=](const DelayTask& dt) { return dt.until > until; };
		auto i = std::find_if(delay_tasks.begin(), delay_tasks.end(), f);
		delay_tasks.insert(i, DelayTask{until, task});
		timer_doze.notify();
	}

	typedef int EVENT;

	EVENT event_id() {
		std::unique_lock<std::mutex> u(mutex);
		return next_event_id++;
	}

	void post_overwrite(EVENT e, int ms, Task task) {
		auto until = std::chrono::steady_clock::now() + std::chrono::milliseconds(ms);
		post_overwrite(e, until, task);
	}

	void post_overwrite(EVENT e, std::chrono::steady_clock::time_point until, Task task) {
		std::unique_lock<std::mutex> u(mutex);

		auto p = event_tasks.find(e);
		if (p == event_tasks.end()) {
			event_tasks[e] = DelayTask{until, task};
		} else {
			if (p->second.until > until) {
				p->second = DelayTask{until, task};
			}
		}

		timer_doze.notify();
	}

	void remove(EVENT e) {
		std::unique_lock<std::mutex> u(mutex);
		event_tasks.erase(e);
	}

	void join() {
		if (timer_thread.joinable())
			timer_thread.join();
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

	void check_delay() {
		std::unique_lock<std::mutex> u(mutex);
		if (delay_tasks.empty() && event_tasks.empty()) {
			return;
		}

		auto now = std::chrono::steady_clock::now();
		while (!delay_tasks.empty() && delay_tasks.front().until <= now) {
			pool(delay_tasks.front().task);
			delay_tasks.pop_front();
		}

		for (auto i = event_tasks.begin(); i != event_tasks.end(); ) {
			if (i->second.until <= now) {
				pool(i->second.task);
				event_tasks.erase(i++);
			} else {
				++i;
			}
		}
	}

	std::chrono::steady_clock::time_point next_time_point() {
		std::unique_lock<std::mutex> u(mutex);
		std::chrono::steady_clock::time_point zero;
		std::chrono::steady_clock::time_point t;

		if (!event_tasks.empty()) {
			for (auto& e : event_tasks) {
				if (t == zero || t > e.second.until) {
					t = e.second.until;
				}
			}
		}

		if (!delay_tasks.empty()) {
			if (t == zero || t > delay_tasks.front().until) {
				t = delay_tasks.front().until;
			}
		}

		if (t == zero) {
			t = std::chrono::steady_clock::now() + std::chrono::seconds(1);
		}

		return t;
	}

	std::thread timer_thread;
	Doze timer_doze;

	Pool pool;

	std::mutex mutex;
	std::list<DelayTask> delay_tasks;
	std::map<EVENT, DelayTask> event_tasks;

	int volatile next_event_id = 1;

	bool stopped = false;
};

}

#endif // __COMMON_THREAD_POOL_TIMER_HPP__
