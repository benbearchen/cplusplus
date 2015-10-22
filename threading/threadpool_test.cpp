#include "threadpool.hpp"
#include "threadpool_timer.hpp"

#include <functional>
#include <iostream>

auto boot = std::chrono::steady_clock::now();

double tick() {
    auto d = std::chrono::steady_clock::now() - boot;
	return std::chrono::duration<double>(d).count();
}

common::Doze doze;

struct postself {
public:
	postself(std::function<void(std::function<void()>)> p, int c) {
		poster = p;
		count = c;

		p(std::bind(*this));
	}

	void operator() () {
		--count;
		if (count > 0) {
			poster(std::bind(*this));
		} else {
			doze.notify();
		}
	}

private:
	std::function<void(std::function<void()>)> poster;
	int count;
	std::chrono::steady_clock::time_point start;
};

int main() {
	common::ThreadPool<> pool;
    common::ThreadPoolTimer<> timer([&](std::function<void()> task){pool.post(task);});

    pool.post([](){
              std::cout << tick() << " right now" << std::endl;
              std::this_thread::sleep_for(std::chrono::milliseconds(50));
              });
    timer.post(520, [](){std::cout << tick() << " delay 520ms" << std::endl;});
    timer.post(10, [&](){
               std::cout << tick() << " delay 10ms" << std::endl;
               timer.post(50, [](){
                          std::cout << tick() << " delay 50ms on delay 10ms"
                          << std::endl;
                          });
               });
    timer.post(149, [&](){
               std::cout << tick() << " delay 149ms" << std::endl;
               pool.post([](){
                         std::cout << tick() << " again on delay 149ms" << std::endl;
                         });
               });

	std::this_thread::sleep_for(std::chrono::seconds(1));

	std::cout << std::endl;
	boot = std::chrono::steady_clock::now();
    timer.post(0, [](){ std::cout << tick() << " first delay 0ms" << std::endl; });
    timer.post(0, [](){ std::cout << tick() << " second delay 0ms" << std::endl; });
	pool.post([](){ std::cout << tick() << " first no delay" << std::endl; });
	pool.post([](){ std::cout << tick() << " second no delay" << std::endl; });

	std::this_thread::sleep_for(std::chrono::milliseconds(100));

	{
		std::cout << std::endl;
		boot = std::chrono::steady_clock::now();
		int c = 10000;
		postself test_pool([&](std::function<void()> p){pool.post(p);}, c);
		doze.wait_until(boot+std::chrono::minutes(1));
		double t = tick(); 
		std::cout << t << "s/" << c << " per " << (t / c * 1000000)
            << "us, post self" << std::endl;

		boot = std::chrono::steady_clock::now();
		postself test_timer([&](std::function<void()> p){timer.post(0, p);}, c);
		doze.wait_until(boot+std::chrono::minutes(1));
		t = tick(); 
		std::cout << t << "s/" << c << " per " << (t / c * 1000000)
            << "us, post delay self" << std::endl;
	}
}
