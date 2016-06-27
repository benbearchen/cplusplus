#include <iostream>
#include <map>
#include <random>
#include <string>

int main() {
    std::random_device rd;
    for (int i = 0; i < 100; i++) {
        std::cout << std::to_string(rd()) << "\t";
    }

    std::cout << std::endl;

    std::uniform_int_distribution<int> uid(1, 100);
    std::default_random_engine e(rd());

    std::map<int, int> statInt;
    for (int i = 0; i < 100000; i++) {
        statInt[uid(e)]++;
    }

    for (auto p : statInt) {
        std::cout << p.first << " - " << p.second << std::endl;
    }
}
