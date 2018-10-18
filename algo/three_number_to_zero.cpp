#include <algorithm>
#include <stdio.h>
#include <vector>

void next(int*& p, int* q) {
    if (p == q) {
        return;
    }

    int v = *p;
    ++p;
    while (p != q && *p == v) {
        ++p;
    }
}

void prev(int* p, int*& q) {
    if (p == q) {
        return;
    }

    int v = *q;
    --q;
    while (p != q && *q == v) {
        --q;
    }
}

std::vector<std::vector<int>> find3(const std::vector<int>& numbers) {
    std::vector<int> a(numbers);
    std::sort(a.begin(), a.end());

    std::vector<std::vector<int>> result;
    if (a.size() < 3) {
        return result;
    }

    int* c = &a.front();
    int* e = c + (a.size() - 2);
    for (; c != e;) {
        int* p = c + 1;
        int* q = e + 1;

        while (p != q) {
            int s = *c + *p + *q;
            if (s == 0) {
                result.push_back({*c, *p, *q});
                next(p, q);
                prev(p, q);
            } else if (s < 0) {
                next(p, q);
            } else {
                prev(p, q);
            }
        }

        next(c, e);
    }

    return result;
}

int main() {
    std::vector<int> nums = {-1, 0, 1, 2, -1, 4, -3};
    auto result = find3(nums);
    for (auto& a : result) {
        for (auto& n : a) {
            printf("%d\t", n);
        }

        printf("\n");
    }

    printf("total %d triplets\n", result.size());
}
