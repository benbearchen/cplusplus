#include <condition_variable>
#include <thread>
#include <mutex>
#include <chrono>
#include <iostream>

#include "doze.hpp"

std::condition_variable condvar;
std::mutex mutex;
bool flag = false;
const int TIMES = 10;
auto boot = std::chrono::steady_clock::now();
bool stop1 = false;

double tick() {
	return std::chrono::duration<double>(std::chrono::steady_clock::now() - boot).count();
}

void notifyThread() {
	for (int i = 1; i <= TIMES; ++i) {
		std::this_thread::sleep_for(std::chrono::milliseconds(i*31));
		{
			std::unique_lock<std::mutex> u(mutex);
			std::cout << tick() << " notify " << i << std::endl;
			flag = true;
			condvar.notify_one();
		}
	}

	stop1 = true;
}

void workThread() {
	do {
		std::unique_lock<std::mutex> u(mutex);
		auto start = std::chrono::steady_clock::now();
		bool e = condvar.wait_for(u, std::chrono::milliseconds(50), [](){return flag;});
		flag = false;
		auto end = std::chrono::steady_clock::now();
		std::chrono::duration<double> d = end - start;
		if (e) {
			std::cout << tick() << " notified in " << d.count() << std::endl;
		} else {
			std::cout << tick() << " timeout in " << d.count() << std::endl;
		}

		if (u) {
			std::cout << "yeah, locked" << std::endl;
		}
	} while (!stop1);
}

common::Doze doze(100);
int stop2 = 0;

void n2(int w) {
	for (int i = 1; i <= TIMES; ++i) {
		std::this_thread::sleep_for(std::chrono::milliseconds(w));
		doze.notify();
	}

	--stop2;
}

void w2() {
	int c = 0;
	while (stop2 != 0) {
		boot = std::chrono::steady_clock::now();
		bool e = doze.wait();
		std::cout << tick() << " wake " << e << std::endl;
		if (e) {
			++c;
		}
	}

	std::cout << "wake " << c << " times" << std::endl;
}

int main() {
	std::thread notify(notifyThread);
	std::thread work(workThread);
	notify.join();
	work.join();

	std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	std::cout << std::endl;

	stop2 = 2;
	std::thread n2t(n2, 25);
	std::thread n2T(n2, 50);
	std::thread w2t(w2);
	n2t.join();
	n2T.join();
	w2t.join();
}
