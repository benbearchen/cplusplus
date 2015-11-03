// g++ -std=c++11 -I../ -I../threading test_chunked_reader.cpp

#include "asyncstream/reader.hpp"
#include "asyncstream/block_reader.hpp"
#include "asyncstream/chunked_reader.hpp"

#include "threadpool.hpp"
#include "threadpool_timer.hpp"

#include <iostream>

using namespace asyncstream;

auto boot = std::chrono::steady_clock::now();

double tick() {
	auto d = std::chrono::steady_clock::now() - boot;
	return std::chrono::duration<double>(d).count();
}

int test_chunked_input() {
	common::ThreadPool<> p_;
	common::ThreadPoolTimer<> pool([&](std::function<void()> task){p_.post(task);});

	std::shared_ptr<Reader> p(new BlockReaderUtil(0, [](const char* buffer, int len) {
		std::cout << tick() << " input:" << std::string(buffer, buffer+len) << std::endl;
		return 0;
	}));

	std::shared_ptr<ChunkedReader> c(new ChunkedReader(p.get()));

	auto post = [&](int ms, const std::string& str) {
		pool.post(ms, [=]() { c->OnRead(str.c_str(), str.size()); });
	};

	post( 100, "1\r\n1\r\n");
	post( 500, " 5\r\nhello\r\n");
	post( 500, "5 \r\nworld\r\n");
	post(1000, " ");
	post(1000, " A");
	post(1000, "\r");
	post(1200, "\n");
	post(1200, "weer\r\nop");
	post(2000, "A");
	post(2000, "B\r");
	post(2000, "\n0");
	post(2500, " \r");
	post(2500, "\n\r");
	post(2500, "\n8923472952\\r\\n\r\n983457293845729354");

	pool.post(3000, [=]() { p->OnEOF(1); });
	pool.post(3100, [&]() { pool.stop(); });

	pool.join();

	std::vector<char> tail;
	bool hasTail = c->CheckTail(&tail);
	std::cout << hasTail << " tail: " << std::string(tail.begin(), tail.end()) << std::endl;
}

int main() {
	test_chunked_input();
}
