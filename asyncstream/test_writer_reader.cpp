// g++ -std=c++11 -I../ -I../threading test_writer_reader.cpp

#include "asyncstream/reader.hpp"
#include "asyncstream/writer_reader.hpp"

#include "threadpool.hpp"
#include "threadpool_timer.hpp"

#include <iostream>

using namespace asyncstream;

auto boot = std::chrono::steady_clock::now();

double tick() {
	auto d = std::chrono::steady_clock::now() - boot;
	return std::chrono::duration<double>(d).count();
}

class TestReader: public Reader {
public:
	virtual void OnRead(const char* buffer, int len) override {
		std::cout << tick() << " read: " << std::string(buffer, buffer + len) << std::endl;
	}

	virtual void OnEOF(int state) override {
		std::cout << tick() << " eof" << std::endl;
	}
};

int test_writer_reader() {
	common::ThreadPool<> p_;
	common::ThreadPoolTimer<> pool([&](std::function<void()> task){p_.post(task);});

	std::shared_ptr<Reader> p(new TestReader());
	std::shared_ptr<WriterReader> w(new WriterReader(p));

	auto post = [&](int ms, const std::string& str) {
		pool.post(ms, [=]() { w->Write(str.c_str(), str.size()); });
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

	pool.post(3000, [=]() { w->Close(); });
	pool.post(3100, [&]() { pool.stop(); });

	pool.join();
}

int main() {
	test_writer_reader();
}
