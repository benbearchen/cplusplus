#include <chrono>
#include <iostream>
#include <list>
#include <mutex>
#include <stdlib.h>
#include <thread>

int C = 20000;
std::list<int> list;
std::mutex mutex;

bool policy_rand_insert = true;
bool policy_write_wait = true;
bool policy_round_clear = true;

std::list<int>::iterator begin(int* count = nullptr) {
    std::unique_lock<std::mutex> lock(mutex);
    if (count != nullptr) {
        *count = list.size();
    }

    return std::begin(list);
}

std::list<int>::iterator end(int* count = nullptr) {
    std::unique_lock<std::mutex> lock(mutex);
    if (count != nullptr) {
        *count = list.size();
    }

    return std::end(list);
}

std::list<int>::iterator inc(std::list<int>::iterator i) {
    std::unique_lock<std::mutex> lock(mutex);
    return ++i;
}

void insert(int v) {
    std::unique_lock<std::mutex> lock(mutex);
    auto p = policy_rand_insert ? std::lower_bound(std::begin(list), std::end(list), v) : list.begin();
    list.insert(p, v);
}

void clear() {
    std::unique_lock<std::mutex> lock(mutex);
    list.clear();
}

bool empty() {
    std::unique_lock<std::mutex> lock(mutex);
    return list.empty();
}

void readThread() {
    for (;;) {
        int c0 = -1;
        int c1 = -1;
        int c = 0;
        for (auto i = begin(&c0); i != end(&c1); i = inc(i)) {
            *i = -1;
            ++c;
        }

        std::cout << "from " << c0 << " to " << c << " real " << c1 << std::endl;
        if (policy_round_clear || c >= C) {
            clear();
        } else if (c == 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds(0));
        }
    }
}

void writeThread() {
    for (;;) {
        for (int i = 0; i < C; ++i) {
            insert(rand());
        }

        if (policy_write_wait) {
            while (!empty()) {
                std::this_thread::yield();
            }
        }
    }
}

int main() {
    std::thread wt(writeThread);
    std::thread w2(writeThread);
    std::thread rt(readThread);

    rt.join();
    w2.join();
    wt.join();
}
