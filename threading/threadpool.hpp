#ifndef __COMMON_THREAD_POOL_HPP__
#define __COMMON_THREAD_POOL_HPP__

#include <list>
#include <thread>
#include <vector>

#include "doze.hpp"

namespace common {

template <typename Task = std::function<void()>>
class ThreadPool {
public:
    explicit ThreadPool(int num = 1) : work_doze(1000) {
        if (num < 1) {
            num = 1;
        }

        max_num = num;
        work_threads.reserve(max_num);
        new_thread();
    }

    ~ThreadPool() {
        stop();
        join();
    }

    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;

    void stop() {
        stopped = true;
        work_doze.notify();
    }

    void post(Task task) {
        std::unique_lock<std::mutex> u(mutex);
        if (tasks.empty()) {
            work_doze.notify();
        } else {
            new_thread();
        }

        tasks.push_back(task);
    }

private:
    void join() {
        for (auto& work_thread : work_threads) {
            if (work_thread.joinable())
                work_thread.join();
        }

        work_threads.clear();
    }

    void thread_work() {
        while (!stopped) {
            work_doze.wait();
            check_work();
        }
    }

    void check_work() {
        while (!stopped) {
            Task task;
            {
                std::unique_lock<std::mutex> u(mutex);
                if (tasks.empty()) {
                    return;
                }

                task = tasks.front();
                tasks.pop_front();
                if (!tasks.empty()) {
                    work_doze.notify();
                }
            }

            if (task) {
                task();
            }
        }
    }

    void new_thread() {
        int c = int(work_threads.size());
        if (c < max_num) {
            work_threads.resize(c + 1);
            auto w = std::thread([this](){ thread_work(); });
            work_threads[c].swap(w);
        }
    }

    std::vector<std::thread> work_threads;
    Doze work_doze;

    std::mutex mutex;
    std::list<Task> tasks;

    int max_num = 1;
    bool stopped = false;
};

}

#endif // __COMMON_THREAD_POOL_HPP__
