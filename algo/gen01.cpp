#include <stdio.h>
#include <stdlib.h>

#include <vector>
#include <set>
#include <algorithm>

std::vector<double> gen1(int n) {
    std::vector<double> r;
    double x = 1.0;
    double y = 0.0;
    for (int i = 0; i < n; i++) {
        double v = rand() / double(RAND_MAX) * x / (n - i);

        y += v;
        x -= v;

        r.push_back(x);
    }

    std::reverse(r.begin(), r.end());
    return r;
}

std::vector<double> gen2(int n) {
    std::set<int> r;
    int max = 0;
    while (r.size() < size_t(n)) {
        int v = rand();
        if (r.find(v) == r.end()) {
            r.insert(v);
            if (v > max) {
                max = v;
            }
        }
    }

    std::vector<double> d;
    for (double v : r) {
        d.push_back(v / double(max));
    }

    return d;
}

std::vector<double> gen3(int n) {
    std::vector<double> r;
    for (int i = 0; i < n; i++) {
        double v = (i + rand() / double(RAND_MAX)) / n;
        r.push_back(v);
    }

    return r;
}

void show(const char* f, std::vector<double> a) {
    printf("%10s:", f);
    for (double v : a) {
        printf(" %.3f", v);
    }

    printf("\n");
}

int main() {
    srand(time(0));
    rand();

    int n = 10;
    for (int i = 0; i < 5; i++) {
        printf("----------\n");
        show("add1/n", gen1(10));
        show("rand/max", gen2(10));
        show("split1/n", gen3(10));
    }
}
