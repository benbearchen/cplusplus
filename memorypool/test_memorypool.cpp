#include "memorypool.hpp"

#include <algorithm>
#include <iostream>
#include <stdlib.h>
#include <time.h>
#include <vector>

int main() {
	memorypool::MemoryPool mp;
	int C = 10000;
	std::vector<std::shared_ptr<uint8_t>> cache(C);
	std::vector<int> times(C);

	srand(time(nullptr));

	int T = 10000000;
	for (int i = 0; i < T; ++i) {
		std::shared_ptr<uint8_t> ptr = mp.Shared(1024);
		int index = rand() % C;
		cache[index] = ptr;
		++times[index];
	}

	std::sort(std::begin(times), std::end(times));
	for (int i = 0; i < C; ++i) {
		std::cout << times[i] << " ";
	}

	std::cout << std::endl;

	std::cout << "pool count: " << mp.Count() << std::endl;

	cache.clear();

	std::cout << "pool count after clear cache: " << mp.Count() << std::endl;

	mp.Clear();

	std::cout << "pool count after clear pool: " << mp.Count() << std::endl;

	auto p = mp.Shared(3);
	uint8_t* addr = p.get();
	p.reset();
	p = mp.Shared(4);
	if (addr != p.get()) {
		std::cout << "pool size 3 & 4 fail" << std::endl;
	}

	p = mp.Shared(5);
	addr = p.get();
	for (int i = 6; i <= 8; ++i) {
		p.reset();
		p = mp.Shared(i);
		if (addr != p.get()) {
			std::cout << "pool size 5 & " << i << " fail" << std::endl;
		}
	}

	p = mp.Shared(9);
	addr = p.get();
	for (int i = 10; i <= 16; ++i) {
		p.reset();
		p = mp.Shared(i);
		if (addr != p.get()) {
			std::cout << "pool size 9 & " << i << " fail" << std::endl;
		}
	}

	p.reset();
}
