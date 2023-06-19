#include <algorithm>
#include <inttypes.h>
#include <set>
#include <map>
#include <stdio.h>
#include <unordered_set>
#include <vector>

void add(const std::vector<std::pair<int64_t, int64_t>>& a1,
         const std::vector<std::pair<int64_t, int64_t>>& v,
         std::vector<std::pair<int64_t, int64_t>>* result) {
    std::vector<std::pair<int64_t, int64_t>> r;
    r.reserve(a1.size() * v.size());
    int64_t max = a1.back().first;
    for (auto a : a1) {
        for (auto b : v) {
            int64_t c = a.first + b.first;
            if (c < max) {
                r.emplace_back(c, (b.second << 12) + a.second);
            } else {
                break;
            }
        }
    }

    if (r.empty()) {
        result->clear();
        return;
    }

    std::sort(r.begin(), r.end());
    auto i = r.begin();
    auto j = r.begin();

    // 过滤头部不重复的元素
    while (++j != r.end()) {
        if (i->first != j->first) {
            ++i;
        } else {
            break;
        }
    }

    while (j != r.end()) {
        if (i->first != j->first) {
            *++i = *j;
        }

        ++j;
    }

    ++i;

    r.erase(i, r.end());
    *result = std::move(r);
}

void dec(const std::vector<std::pair<int64_t, int64_t>>& a1,
         const std::vector<std::pair<int64_t, int64_t>>& v,
         std::vector<std::pair<int64_t, int64_t>>* result) {
    std::vector<std::pair<int64_t, int64_t>> r;
    r.reserve(a1.size() * a1.size());
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
    auto i = r.begin();
    auto j = r.begin();

    // 过滤头部不重复的元素
    while (++j != r.end()) {
        if (i->first != j->first) {
            ++i;
        } else {
            break;
        }
    }

    while (j != r.end()) {
        if (i->first != j->first) {
            *++i = *j;
        }

        ++j;
    }

    ++i;

    r.erase(i, r.end());
    *result = std::move(r);
}

int64_t reorder(int64_t a, int64_t b) {
    int64_t i1 = (a >> 24) & 0x0FFF;
    int64_t i2 = (a >> 12) & 0x0FFF;
    int64_t i3 = (a >> 0) & 0x0FFF;
    int64_t i4 = (b >> 12) & 0x0FFF;
    int64_t i5 = (b >> 0) & 0x0FFF;

    std::vector<int64_t> i = {i1, i2, i3, i4, i5};
    std::sort(i.begin(), i.end());
    return (i[0] << 0) + (i[1] << 12) + (i[2] << 24) + (i[3] << 36) + (i[4] << 48);
}

void print(int64_t a, int64_t b) {
    int i1 = (a >> 0) & 0x0FFF;
    int i2 = (a >> 12) & 0x0FFF;
    int i3 = (a >> 24) & 0x0FFF;
    int i4 = (a >> 36) & 0x0FFF;
    int i5 = (a >> 48) & 0x0FFF;

    printf("%4d + %4d + %4d + %4d + %4d = %4d\n", i1, i2, i3, i4, i5, int(b));
}

int main() {
    std::vector<std::pair<int64_t, int64_t>> a1;

    for (int i = 1; i <= 1024; i++) {
        int64_t i2 = i * i;
        a1.emplace_back(i2 * i2 * i, i);
    }

    std::vector<std::pair<int64_t, int64_t>> a2;
    add(a1, a1, &a2);
    printf("a2.size: %lu\n", a2.size());

    std::vector<std::pair<int64_t, int64_t>> a3;
    add(a1, a2, &a3);
    printf("a3.size: %lu\n", a3.size());

    std::vector<std::pair<int64_t, int64_t>> b3;
    dec(a1, a2, &b3);
    printf("b3.size: %lu\n", b3.size());

    std::map<int64_t, int64_t> n5;
    auto ia = a3.begin();
    auto ib = b3.begin();
    while (ia != a3.end() && ib != b3.end()) {
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

    std::vector<std::pair<int64_t, int64_t>> g;
    for (auto n : n5) {
        g.emplace_back(n.second, n.first);
    }

    std::sort(g.begin(), g.end());
    for (auto n : g) {
        print(n.second, n.first);
    }

    if (!b3.empty()) {
        exit(0);
    }

    n5.clear();
    for (auto v2 : a2) {
        auto i = std::lower_bound(a1.begin(), a1.end(), std::pair<int64_t, int64_t>(v2.first, 0));
        for (auto v3 : a3) {
            int64_t n = v2.first + v3.first;
            while (i != a1.end() && i->first < n) {
                ++i;
            }

            if (i == a1.end()) {
                break;
            } else if (i->first == n) {
                n5[reorder(v3.second, v2.second)] = i->second;
            }
        }
    }

    for (auto n : n5) {
        print(n.first, n.second);
    }

    exit(0);

    n5.clear();
    for (auto n : a1) {
        auto i3 = a3.end();
        for (auto n2 : a2) {
            int64_t d = n.first - n2.first;
            if (d <= 0) {
                break;
            } else {
                auto i = std::lower_bound(a3.begin(), i3, std::pair<int64_t, int64_t>(d, 0));
                if (i != i3 && i->first == d) {
                    n5[reorder(i->second, n2.second)] = n.second;
                }

                i3 = i;
            }
        }
    }

    for (auto n : n5) {
        print(n.first, n.second);
    }

    exit(0);

}
