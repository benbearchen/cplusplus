#include "threadpool.hpp"

#include <functional>
#include <iostream>

#include <stdio.h>

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

    postself(std::function<void(std::function<void()>)> p) {
        poster = p;
    }

    void operator() () {
        --count;
        if (count > 0) {
            poster(std::bind(*this));
        } else {
            doze.notify();
        }
    }

    void operator() (int c) {
        printf("%5.3f multi: %d\n", tick(),  c);
        if (c > 0) {
            --c;
            for (int i = 0; i < c; ++i) {
                poster(std::bind(*this, c));
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }

private:
    std::function<void(std::function<void()>)> poster;
    int count = 0;
    std::chrono::steady_clock::time_point start;
};

void testmulti() {
    std::cout << std::endl << "test multi threadpool" << std::endl;
    boot = std::chrono::steady_clock::now();

    common::ThreadPool<> pool(4);
    postself test_pool([&](std::function<void()> p){pool.post(p);});
    pool.post(std::bind(test_pool, 4));

    std::this_thread::sleep_for(std::chrono::seconds(1));

    boot = std::chrono::steady_clock::now();
    std::cout << std::endl;

    int zerot = 0;
    int zerotimes = 10000;
    auto eid = pool.event_id();
    for (int i = 0; i < zerotimes; i++) {
        pool.post_overwrite(eid, 0, [&]() { zerot++; pool.post([]() { std::this_thread::sleep_for(std::chrono::milliseconds(1)); }); });
    }

    std::cout << tick() << " post zero finished" << std::endl;
    pool.post([&]() { std::cout << tick() << " post zero " << zerotimes << " times, got " << zerot << std::endl; });

    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
}

int main() {
    common::ThreadPool<> pool;

    boot = std::chrono::steady_clock::now();

    pool.post([](){
              std::cout << tick() << " right now" << std::endl;
              std::this_thread::sleep_for(std::chrono::milliseconds(50));
              });
    pool.post(520, [](){std::cout << tick() << " delay 520ms" << std::endl;});
    pool.post(10, [&](){
              std::cout << tick() << " delay 10ms" << std::endl;
              pool.post(50, [](){
                        std::cout << tick() << " delay 50ms on delay 10ms"
                        << std::endl;
                        });
              });
    pool.post(149, [&](){
              std::cout << tick() << " delay 149ms" << std::endl;
              pool.post([](){
                        std::cout << tick() << " again on delay 149ms" << std::endl;
                        });
              });

    std::this_thread::sleep_for(std::chrono::seconds(1));

    std::cout << std::endl;
    boot = std::chrono::steady_clock::now();
    pool.post(0, [](){ std::cout << tick() << " first delay 0ms" << std::endl; });
    pool.post(0, [](){ std::cout << tick() << " second delay 0ms" << std::endl; });
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
        postself test_timer([&](std::function<void()> p){pool.post(0, p);}, c);
        doze.wait_until(boot+std::chrono::minutes(1));
        t = tick(); 
        std::cout << t << "s/" << c << " per " << (t / c * 1000000)
            << "us, post delay self" << std::endl;
    }

    boot = std::chrono::steady_clock::now();
    std::cout << std::endl;

    auto eid = pool.event_id();
    pool.post_overwrite(eid, 1000, [&]() { std::cout << tick() << " event 1s" << std::endl; });
    pool.post_overwrite(eid, 100, [&]() { std::cout << tick() << " event 0.1s" << std::endl; });
    pool.post_overwrite(eid, 2000, [&]() { std::cout << tick() << " event 0.2s" << std::endl; });

    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    pool.post_overwrite(eid, 100, [&]() { std::cout << tick() << " should miss event 0.1s" << std::endl; });
    pool.remove(eid);

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    boot = std::chrono::steady_clock::now();
    std::cout << std::endl;

    int zerot = 0;
    int zerotimes = 10000;
    eid = pool.event_id();
    for (int i = 0; i < zerotimes; i++) {
        pool.post_overwrite(eid, 0, [&]() { zerot++; std::this_thread::sleep_for(std::chrono::milliseconds(1)); });
    }

    std::cout << tick() << " post zero finished" << std::endl;
    pool.post([&]() { std::cout << tick() << " post zero " << zerotimes << " times, got " << zerot << std::endl; });

    std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    testmulti();
}
