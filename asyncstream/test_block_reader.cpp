// g++ -std=c++11 -I../ -I../threading test_block_reader.cpp

#include "asyncstream/reader.hpp"
#include "asyncstream/block_reader.hpp"

#include "threadpool.hpp"
#include "threadpool_timer.hpp"

#include <iostream>

using namespace asyncstream;

auto boot = std::chrono::steady_clock::now();

double tick() {
	auto d = std::chrono::steady_clock::now() - boot;
	return std::chrono::duration<double>(d).count();
}

int test_block_input_2n() {
	common::ThreadPool<> p_;
	common::ThreadPoolTimer<> pool([&](std::function<void()> task){p_.post(task);});

	std::shared_ptr<Reader> p(new BlockReaderUtil(1, [](const char* buffer, int len) {
		std::cout << tick() << " input:" << std::string(buffer, buffer+len) << std::endl;
		if (len < 16) {
			return len * 2;
		} else if (buffer[0] <= '9') {
			return 1;
		} else {
			return len / 2;
		}
	}));

	auto post = [&](int ms, const std::string& str) {
		pool.post(ms, [=]() { p->OnRead(str.c_str(), str.size()); });
	};

	post( 100, "helloworld");
	post( 500, "0198569238456273865872562345234958723945");
	post(1000, "kas");
	post(1000, "fjwie");
	post(1000, "FJWIE");
	post(1200, "!%$&^#$%^");
	post(1200, "weer");
	post(2000, "A");
	post(2000, "B");
	post(2000, "C");
	post(2000, "D");
	post(2000, "E");
	post(2000, "F");
	post(2000, "G");
	post(2500, "wiefjawieawefoijawofjaowifjoawijefoaiwjfoawjefoawjefo;awjefoawjefoawjefo;awjefoiajwefoiawejfawf");

	pool.post(3000, [=]() { p->OnEOF(1); });
	pool.post(3100, [&]() { pool.stop(); });

	pool.join();
}

int main() {
	test_block_input_2n();
}
