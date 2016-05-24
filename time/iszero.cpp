#include <chrono>
#include <iostream>

int main() {
    auto now = std::chrono::steady_clock::now();
    auto zero = std::chrono::steady_clock::time_point();

    std::cout << "now.time_since_epoch(): " << now.time_since_epoch().count() << std::endl;
    std::cout << "zero.time_since_epoch(): " << zero.time_since_epoch().count() << std::endl;
    std::cout << "zero == time_point(): " << (zero == std::chrono::steady_clock::time_point()) << std::endl;
}
