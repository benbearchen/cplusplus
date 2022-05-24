#include <inttypes.h>
#include <random>
#include <set>
#include <stdio.h>

struct engine {
    uint64_t x;

    uint64_t operator()() {
        x = x * 16807 % 2147483647;
        return x;
    }
};

struct uniform {
    engine e;
    uint64_t operator()() {
        uint64_t v = 0;
        do {
            uint64_t h = (e() - 1) * 3 / 2147483646;
            v = h * 2147483646 + e() - 1;
        } while (v > 4294967295);

        return v;
    }
};

void loop(uint64_t seed) {
    uint64_t min = 1llu << 32;
    std::set<uint64_t> s;
    int times = 2;
    int c = 0;

    uniform u;
    u.e.x = seed;

    do {
        uint64_t v = u();
        //s.insert(v);
        if (v < min) {
            printf("seed(%12" PRIu64 ") min(%12" PRIu64 ") count(%12d) v(%12" PRIu64 ")\n", u.e.x, min, c, v);
            min = v;
        } else if (v == min) {
            times--;
        }

        if (times == 1) {
            c++;
            if (c % 10000000 == 0) {
                //printf("seed(%12" PRIu64 ") min(%12" PRIu64 ") count(%12d) v(%12" PRIu64 ")\n", u.e.x, min, c, v);
            }

            if (v < 100) {
                s.insert(v);
            }
        }
    } while (times > 0);

    printf("seed(%12" PRIu64 ") min(%12" PRIu64 ") count(%12d) c100(%lu)\n", seed, min, c, s.size());
    printf("----------------------\n");
}

void loop2(uint64_t seed) {
    engine e = {seed};
    uint64_t end = 0;
    int c = 0;
    int sum[2][3] = {};
    do {
        uint64_t v = e();
        if (end == 0) {
            end = v;
        } else if (end == v) {
            break;
        }

        sum[c++ % 2][(v - 1) / 715827882]++;
    } while (true);

    printf("[%10d][%10d][%10d]\n", sum[0][0], sum[0][1], sum[0][2]);
    printf("[%10d][%10d][%10d]\n", sum[1][0], sum[1][1], sum[1][2]);
}

void loopDefaultStd(uint64_t seed) {
    std::default_random_engine e(seed);
    std::uniform_int_distribution<uint32_t> u;
    uint64_t end = u(e);
    int c = 0;
    int times = 1;
    uint64_t min = 1llu << 32;
    std::set<uint64_t> s;
    do {
        uint64_t v = u(e);
        c++;

        if (min > v) {
            min = v;
        }

        if (v < 100) {
            s.insert(v);
        }

        if (end == v) {
            printf("std seed(%20" PRIu64 ") v[%20" PRIu64 "] min[%20" PRIu64 "] loop(%d) c100(%lu)\n", seed, end, min, c, s.size());

            c = 0;
            times--;
            s.clear();
        }
    } while (times > 0);
}

int main() {
    loop2(1407677000);
    loop2(1);
    loop2(16807);

    loopDefaultStd(1407677000);
    loopDefaultStd(1);

    loop(1407677000);
    loop(1);
    loop(2);
    loop(935590599);
    loop(595934059);
    loop(5);
}
