#include <map>
#include <math.h>
#include <stdio.h>
#include <tuple>

int f(int r) {
    return r * (r + 1) / 2;
}

int value(int x, int y, int n) {
    int s = x + y;
    if (s < n) {
        return f(s) + x;
    } else {
        return n * n - 1 - value(n - 1 - x, n - 1 - y, n);
    }
}

std::tuple<int, int> root(int v, int n) {
    if (v >= n * n) {
        return std::make_tuple(-1, -1);
    }

    if (v * 2 >= n * n) {
        auto xy = root(n * n - 1 - v, n);
        int x = n - 1 - std::get<0>(xy);
        int y = n - 1 - std::get<1>(xy);
        return std::make_tuple(x, y);
    }

    int r = sqrt(v * 2);
    int v0 = f(r);
    if (v0 > v) {
        v0 -= r;
        --r;
    }

    int x = v - v0;
    int y = r - x;
    return std::make_tuple(x, y);
}

bool test(int n) {
    std::map<int, std::tuple<int, int>> vxy;

    for (int y = 0; y < n; y++) {
        for (int x = 0; x < n; x++) {
            vxy[value(x, y, n)] = std::make_tuple(x, y);
        }
    }

    for (int i = 0; i < n * n; i++) {
        auto p = vxy.find(i);
        if (p == vxy.end()) {
            printf("can't find %d of %d\n", i, n);
            return false;
        }

        auto xy = root(i, n);
        if (p->second != xy) {
            printf("unmatch %d of %d: <%d, %d>\n", i, n, std::get<0>(xy), std::get<1>(xy));
            return false;
        } else {
            vxy.erase(p);
        }
    }

    return vxy.empty();
}

int main(int argc, char** argv) {
    for (int i = 1; i < 100; i++) {
        if (!test(i)) {
            printf("failed on %d\n", i);
        }
    }

    int n = 7;
    for (int y = 0; y < n; y++) {
        for (int x = 0; x < n; x++) {
            printf("%d\t", value(x, y, n));
        }

        printf("\n");
    }

    int v = 31;
    auto r = root(v, n);
    int x = std::get<0>(r);
    int y = std::get<1>(r);
    printf("xy(%d) => (%d, %d)\n", v, x, y);
    if (x < 0 || y < 0) {
        return 0;
    }

    for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
            if (dy == 0 || dx == 0) {
                int nx = x + dx;
                int ny = y + dy;
                if (0 <= nx && nx < n && 0 <= ny && ny < n) {
                    printf("%d\t", value(nx, ny, n));
                    continue;
                }
            }

            printf("\t");
        }

        printf("\n");
    }
}
