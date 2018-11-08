#include <chrono>
#include <stdio.h>
#include <tuple>

std::tuple<uint64_t, uint64_t> fib2(int n) {
    if (n <= 0) {
        return std::make_tuple(0, 0);
    } else if (n == 1) {
        return std::make_tuple(1, 0);
    } else {
        auto t = fib2(n-1);
        uint64_t a = std::get<0>(t);
        uint64_t b = std::get<1>(t);
        return std::make_tuple(a + b, a);
    }
}

std::tuple<uint64_t, uint64_t> fib_bin(int n) {
    if (n <= 4) {
        return fib2(n);
    } else {
        int h = n / 2;
        auto t = fib_bin(h);
        uint64_t a = std::get<1>(t);
        uint64_t b = std::get<0>(t);
        uint64_t c = a + b;
        uint64_t f2h = a * b + b * c;
        if (n % 2 == 0) {
            uint64_t prev2 = (b - a) * a + a * b;
            return std::make_tuple(f2h, f2h - prev2);
        } else {
            uint64_t next2 = b * c + c * (b + c);
            return std::make_tuple(next2 - f2h, f2h);
        }
    }
}

int64_t fib(int n) {
    return std::get<0>(fib2(n));
}

template <typename Duration>
int us(Duration t) {
    return std::chrono::duration_cast<std::chrono::microseconds>(t).count();
}

int main() {
    for (int i = 1; i < 94; i++) {
        uint64_t f = fib(i);
        printf("fib(%2d) = %20llu, ", i, f);
        auto fb = fib_bin(i);
        printf("bin: %20llu( prev %20llu)", std::get<0>(fb), std::get<1>(fb));
        if (f != std::get<0>(fb)) {
            printf(", unmatch!!!");
        } else {
            printf(" match");
        }

        printf("\n");
    }

    uint64_t f = 0;
    int times = 100000;
    auto b1 = std::chrono::steady_clock::now();
    for (int i = 0; i < times; i++) {
        f = fib(93);
    }
    auto e1 = std::chrono::steady_clock::now();

    auto b2 = std::chrono::steady_clock::now();
    for (int i = 0; i < times; i++) {
        f = std::get<0>(fib_bin(93));
    }
    auto e2 = std::chrono::steady_clock::now();

    printf("%llf vs %llf\n", us(e1 - b1) / double(times), us(e2 - b2) / double(times));
}
