#ifndef __COMMON_THREAD_POOL_HPP__
#define __COMMON_THREAD_POOL_HPP__

#include <algorithm>
#include <atomic>
#include <list>
#include <map>
#include <set>
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
        timer_doze.notify();
    }

    void post(Task task) {
        std::unique_lock<std::mutex> u(mutex);
        if (1 + tasks.size() + run_num > work_threads.size()) {
            new_thread();
        }

        tasks.push_back(std::move(IDTask{0, std::move(task)}));
        work_doze.notify();
    }

    void post(int ms, Task task) {
        std::unique_lock<std::mutex> u(mutex);
        new_timer_thread();

        auto until = std::chrono::steady_clock::now() + std::chrono::milliseconds(ms);
        auto f = [=](const DelayTask& dt) { return dt.until > until; };
        auto i = std::find_if(delay_tasks.begin(), delay_tasks.end(), f);
        delay_tasks.insert(i, std::move(DelayTask{until, std::move(task)}));
        timer_doze.notify();
    }

    typedef int EVENT;

    EVENT event_id() {
        std::unique_lock<std::mutex> u(mutex);
        return next_event_id++;
    }

    void post_overwrite(EVENT e, int ms, Task task) {
        auto until = std::chrono::steady_clock::now() + std::chrono::milliseconds(ms);
        post_overwrite(e, until, task);
    }

    void post_overwrite(EVENT e, std::chrono::steady_clock::time_point until, Task task) {
        std::unique_lock<std::mutex> u(mutex);
        new_timer_thread();

        if (arrival_event_tasks.find(e) != arrival_event_tasks.end()) {
            return;
        }

        auto p = event_tasks.find(e);
        if (p == event_tasks.end()) {
            event_tasks[e] = std::move(DelayTask{until, std::move(task)});
        } else {
            if (p->second.until > until) {
                p->second = std::move(DelayTask{until, std::move(task)});
            } else {
                return;
            }
        }

        timer_doze.notify();
    }

    void remove(EVENT e) {
        std::unique_lock<std::mutex> u(mutex);
        auto p = arrival_event_tasks.find(e);
        if (p != arrival_event_tasks.end()) {
            for (auto i = std::begin(tasks); i != std::end(tasks); ++i) {
                if (i->id == e) {
                    tasks.erase(i);
                    break;
                }
            }

            arrival_event_tasks.erase(p);
        }

        event_tasks.erase(e);
    }

private:
    struct IDTask {
        EVENT id;
        Task task;
    };

    struct DelayTask {
        std::chrono::steady_clock::time_point until;
        Task task;
    };

    class AutoCount {
        std::atomic<size_t>& c_;
    public:
        AutoCount(std::atomic<size_t>& c) : c_(c) {
            ++c_;
        }

        ~AutoCount() {
            --c_;
        }
    };

    void join() {
        for (auto& work_thread : work_threads) {
            if (work_thread.joinable())
                work_thread.join();
        }

        work_threads.clear();

        if (timer_thread) {
            if (timer_thread->joinable()) {
                timer_thread->join();
            }

            timer_thread.reset();
        }
    }

    void thread_work() {
        while (!stopped) {
            check_work();

            if (!stopped) {
                work_doze.wait();
            }
        }
    }

    void thread_timer() {
        while (!stopped) {
            auto t = flush_delay();
            timer_doze.wait_until(t);
        }
    }

    void check_work() {
        while (!stopped) {
            Task task;
            if (take_task(&task)) {
                AutoCount ac(run_num);

                task();
            } else {
                return;
            }
        }
    }

    bool take_task(Task* task) {
        std::unique_lock<std::mutex> u(mutex);
        if (tasks.empty()) {
            return false;
        }

        int id = tasks.front().id;
        *task = std::move(tasks.front().task);
        tasks.pop_front();

        if (id > 0) {
            arrival_event_tasks.erase(id);
        }

        if (!tasks.empty()) {
            if (1 + tasks.size() + run_num > work_threads.size()) {
                new_thread();
            }

            work_doze.notify();
        }

        return true;
    }

    void new_thread() {
        int c = int(work_threads.size());
        if (c < max_num) {
            work_threads.resize(c + 1);
            auto w = std::thread([this](){ thread_work(); });
            work_threads[c].swap(w);
        }
    }

    void new_timer_thread() {
        if (!timer_thread) {
            timer_thread.reset(new std::thread([this](){thread_timer();}));
        }
    }

    std::chrono::steady_clock::time_point flush_delay() {
        std::unique_lock<std::mutex> u(mutex);
        std::chrono::steady_clock::time_point zero;
        std::chrono::steady_clock::time_point t;

        size_t arrival = 0;
        auto now = std::chrono::steady_clock::now();

        for (auto i = std::begin(event_tasks); i != std::end(event_tasks); ) {
            auto et = i->second.until;
            if (et <= now) {
                ++arrival;
                tasks.push_back(std::move(IDTask{i->first, std::move(i->second.task)}));
                arrival_event_tasks.insert(i->first);
                event_tasks.erase(i++);
                continue;
            }

            ++i;
            if (t == zero || t > et) {
                t = et;
            }
        }

        while (!delay_tasks.empty()) {
            auto dt = delay_tasks.front().until;
            if (dt <= now) {
                ++arrival;
                tasks.push_back(std::move(IDTask{0, std::move(delay_tasks.front().task)}));
                delay_tasks.pop_front();
                continue;
            }

            if (t == zero || t > dt) {
                t = dt;
            }

            break;
        }

        if (t == zero) {
            t = now + std::chrono::seconds(1);
        }

        if (arrival > 0) {
            if (tasks.size() + run_num > work_threads.size()) {
                new_thread();
            }

            work_doze.notify();
        }

        return t;
    }

    int max_num = 1;
    std::atomic<size_t> run_num;
    int volatile next_event_id = 1;

    std::vector<std::thread> work_threads;
    Doze work_doze;

    std::unique_ptr<std::thread> timer_thread;
    Doze timer_doze;

    std::mutex mutex;

    std::list<IDTask> tasks;
    std::list<DelayTask> delay_tasks;
    std::set<EVENT> arrival_event_tasks;
    std::map<EVENT, DelayTask> event_tasks;

    bool volatile stopped = false;
};

}

#endif // __COMMON_THREAD_POOL_HPP__
