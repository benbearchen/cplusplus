#include "asyncstream/reader_writer.hpp"

#include "threadpool.hpp"
#include "threadpool_timer.hpp"

#include <chrono>
#include <iostream>

#include <stdlib.h>
#include <time.h>

using namespace asyncstream;

auto boot = std::chrono::steady_clock::now();

double tick() {
	auto d = std::chrono::steady_clock::now() - boot;
	return std::chrono::duration<double>(d).count();
}

class TestWriter: public asyncstream::Writer {
public:
	TestWriter(common::ThreadPoolTimer<>& t) : timer_(t) {
	}

	virtual void SetOnCanWrite(std::function<void(int suggestLength)> onCanWrite) override {
		this->onCanWrite_ = onCanWrite;
	}

	virtual int Write(const char* buffer, int len) override {
		if (busy_) {
			return 0;
		}

		int r = rand() % 8 + 1;
		if (r > len) {
			r = len;
		}

		std::cout << tick() << " w: \'" << std::string(buffer, buffer + r) << "\'" << std::endl;

		busy_ = true;
		timer_.post(500, [this](){
			busy_ = false;
			onCanWrite_(0);
		});

		return r;
	}

	virtual void Close() override {
		timer_.stop();
	}

private:
	std::function<void(int)> onCanWrite_;
	common::ThreadPoolTimer<>& timer_;
	bool busy_ = false;
};

int main() {
	common::ThreadPool<> p;
	common::ThreadPoolTimer<> t([&](std::function<void()> task) {
		p.post(task);
	});

	p.post([](){srand(time(nullptr));});


	std::shared_ptr<TestWriter> writer(new TestWriter(t));
	std::shared_ptr<ReaderWriter> rw(new ReaderWriter());
	rw->Bind(writer);

	auto w = [=](std::string str) {
		rw->OnRead(str.c_str(), str.size());
	};

	t.post( 100, [=](){w("helloworld");});
	t.post( 500, [=](){w("for89456729345762345");});
	t.post(1000, [=](){w("");});
	t.post(2000, [=](){rw->OnEOF(1);});

	t.join();
}
