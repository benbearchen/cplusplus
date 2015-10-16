#ifndef __COMMON_DOZE_HPP__
#define __COMMON_DOZE_HPP__

#include <condition_variable>
#include <chrono>
#include <mutex>

namespace common {

class Doze {
public:
	explicit Doze(int ms = 50) {
		timeout = std::chrono::milliseconds(ms);
	}

	Doze(const Doze&) = delete;
	Doze& operator=(const Doze&) = delete;

	void notify() {
		std::unique_lock<std::mutex> u(mutex);
		flag = true;
		condvar.notify_one();
	}

	bool wait() {
		std::unique_lock<std::mutex> u(mutex);
		bool e = condvar.wait_for(u, timeout, [this](){return flag;});
		flag = false;
		return e;
	}

	bool wait_until(std::chrono::steady_clock::time_point until) {
		std::unique_lock<std::mutex> u(mutex);
		bool e = condvar.wait_until(u, until, [this](){return flag;});
		flag = false;
		return e;
	}

private:
	std::condition_variable condvar;
	std::mutex mutex;
	bool flag = false;
	std::chrono::milliseconds timeout;
};

}

#endif // __COMMON_DOZE_HPP__
