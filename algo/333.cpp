#include <algorithm>
#include <inttypes.h>
#include <map>
#include <stdio.h>
#include <vector>

void add(const std::vector<std::pair<int64_t, int64_t>>& a1,
         const std::vector<std::pair<int64_t, int64_t>>& v,
         std::vector<std::pair<int64_t, int64_t>>* result) {
    std::vector<std::pair<int64_t, int64_t>> r;
    r.reserve(a1.size() * v.size());
    int64_t max = a1.back().first;
    for (auto a : a1) {
        for (auto b : v) {
            if (a.first > b.first) {
                continue;
            }

            int64_t c = a.first + b.first;
            if (c < max) {
                r.emplace_back(c, (b.second << 12) + a.second);
            } else {
                break;
            }
        }
    }

    std::sort(r.begin(), r.end());
    r.erase(std::unique(r.begin(), r.end()), r.end());

    *result = std::move(r);
}

void dec(const std::vector<std::pair<int64_t, int64_t>>& a1,
         const std::vector<std::pair<int64_t, int64_t>>& v,
         std::vector<std::pair<int64_t, int64_t>>* result) {
    std::vector<std::pair<int64_t, int64_t>> r;
    r.reserve(a1.size() * v.size());
    for (auto a : a1) {
        for (auto b : v) {
            int64_t c = a.first - b.first;
            if (c <= 0) {
                break;
            } else {
                r.emplace_back(c, (b.second << 12) + a.second);
            }
        }
    }

    std::sort(r.begin(), r.end());
    r.erase(std::unique(r.begin(), r.end()), r.end());

    *result = std::move(r);
}

int64_t gcd(int64_t a, int64_t b) {
    do {
        if  (a <= 1 || b <= 1) {
            return 1;
        }

        int64_t c = a % b;
        if (c == 0) {
            return b;
        }

        a = b;
        b = c;
    } while (true);
}

int64_t gcd31(int64_t a, int64_t b) {
    int i1 = (a >> 0) & 0x0FFF;
    int i2 = (a >> 12) & 0x0FFF;
    int i3 = (a >> 24) & 0x0FFF;

    return gcd(gcd(i1, i2), gcd(i3, b));
}

int64_t reorder(int64_t a, int64_t b) {
    int64_t i1 = (a >> 12) & 0x0FFF;
    int64_t i2 = (a >> 0) & 0x0FFF;
    int64_t i3 = (b >> 0) & 0x0FFF;

    std::vector<int64_t> i = {i1, i2, i3};
    std::sort(i.begin(), i.end());
    return (i[0] << 0) + (i[1] << 12) + (i[2] << 24);
}

void print(int64_t a, int64_t b) {
    int i1 = (a >> 0) & 0x0FFF;
    int i2 = (a >> 12) & 0x0FFF;
    int i3 = (a >> 24) & 0x0FFF;

    printf("%4d + %4d + %4d = %4d\n", i1, i2, i3, int(b));
}

int main() {
    std::vector<std::pair<int64_t, int64_t>> a1;

    for (int i = 1; i <= 100; i++) {
        int64_t i1 = i;
        a1.emplace_back(i1 * i1 * i, i);
    }

    std::vector<std::pair<int64_t, int64_t>> a2;
    add(a1, a1, &a2);
    printf("a2.size: %lu\n", a2.size());

    std::vector<std::pair<int64_t, int64_t>> b1;
    dec(a1, a1, &b1);
    printf("b1.size: %lu\n", b1.size());

    std::map<int64_t, int64_t> n5;
    auto ia = a2.begin();
    auto ib = b1.begin();
    while (ia != a2.end() && ib != b1.end()) {
        int64_t d = ia->first - ib->first;
        if (d < 0) {
            ia++;
        } else if (d > 0) {
            ib++;
        } else {
            n5[reorder(ia->second, ib->second >> 12)] = ib->second & 0x0FFF;
            ia++;
            ib++;
        }
    }

    printf("pass:\n");
    std::vector<std::pair<int64_t, int64_t>> g;
    for (auto n : n5) {
        if (gcd31(n.first, n.second) > 1) {
            print(n.first, n.second);
            continue;
        }

        g.emplace_back(n.second, n.first);
    }

    printf("uniq:\n");
    std::sort(g.begin(), g.end());
    for (auto n : g) {
        print(n.second, n.first);
    }
}
