#include <chrono>
#include <iostream>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

int main() {
	const size_t CPU = 4;

	std::shared_ptr<int> ptr;
	std::mutex mutex;

	auto end = std::chrono::steady_clock::now() + std::chrono::seconds(3);
	auto rf = [&]() {
		for (; std::chrono::steady_clock::now() < end;) {
			// would abort without lock while writing
			std::lock_guard<std::mutex> lock(mutex);
			if (auto v = ptr) {
				//std::cout << "v:" << *v << std::endl;
			} else {
				std::cout << "nullptr" << std::endl;
			}
		}

		std::lock_guard<std::mutex> lock(mutex);
		std::cout << "lock read over" << std::endl;
	};

	auto r = std::thread(rf);

	const size_t N = CPU-1;

	// test read and write
	auto end2 = std::chrono::steady_clock::now() + std::chrono::seconds(2);
	std::vector<std::thread> ws(N);
	for (auto& w : ws) {
		auto wf = [&]() {
			for (; std::chrono::steady_clock::now() < end2;) {
				// would abort without lock
				std::lock_guard<std::mutex> lock(mutex);
				ptr = std::shared_ptr<int>(new int(1));
			}

			std::lock_guard<std::mutex> lock(mutex);
			std::cout << "lock write over" << std::endl;
		};

		auto t = std::thread(wf);
		w.swap(t);
	}

	for (auto& w : ws) {
		w.join();
	}

	// test read only
	std::vector<std::thread> rs(N);
	auto rf2 = [&]() {
		for (; std::chrono::steady_clock::now() < end;) {
			// ALL-read-only without lock is ok
			//std::lock_guard<std::mutex> lock(mutex);
			if (auto v = ptr) {
				//std::cout << "v:" << *v << std::endl;
			} else {
				std::cout << "nullptr" << std::endl;
			}
		}

		std::lock_guard<std::mutex> lock(mutex);
		std::cout << "unlock read over" << std::endl;
	};

	for (auto& r : rs) {
		auto t = std::thread(rf2); 
		r.swap(t);
	}

	r.join();
	for (auto& r : rs) {
		r.join();
	}
}

