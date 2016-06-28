#include <chrono>
#include <future>
#include <iostream>
#include <thread>

#include "doze.hpp"

common::Doze w;

int main() {
    auto async = [&]() {
        std::this_thread::sleep_for(std::chrono::seconds(3));
        std::cout << "async wake" << std::endl;
        w.notify();
    };

    auto f = std::async(async);

    f.wait();
    w.wait_for(10000);
    std::cout << "done!" << std::endl;
}
