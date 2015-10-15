#include "threadpool.hpp"

#include <iostream>

auto boot = std::chrono::steady_clock::now();

double tick() {
	return std::chrono::duration<double>(std::chrono::steady_clock::now() - boot).count();
}

int main() {
	common::ThreadPool pool;

	pool.post([](){
			std::cout << tick() << " right now" << std::endl;
			std::this_thread::sleep_for(std::chrono::milliseconds(50));
			});
	pool.post_dealyed(520, [](){std::cout << tick() << " delay 520ms" << std::endl;});
	pool.post_dealyed(10, [](){std::cout << tick() << " delay 10ms" << std::endl;});
	pool.post_dealyed(149, [](){std::cout << tick() << " delay 149ms" << std::endl;});

	std::this_thread::sleep_for(std::chrono::seconds(1));
	pool.stop();
	pool.join();
}
