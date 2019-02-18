#include <algorithm>
#include <set>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <vector>

std::vector<int> genLights() {
    srand(time(0));

    std::vector<int> lights;
    int total = 100;

    if (bool byCount = true) {
        int on = rand() % (total + 1); // 也可指定数量

        printf("default turn on %d lights, total %d\n", on, total);

        lights.resize(on, 1);
        lights.resize(total);
        std::random_shuffle(lights.begin(), lights.end());
    } else {
        int count = 0;
        for (int i = 0; i < total; i++) {
            lights.push_back(rand() % 2);
            if (lights.back()) {
                ++count;
            }
        }

        printf("random turn on %d lights, total %d\n", count, total);
    }

    return lights;
}

void printLights(const std::vector<int>& lights) {
    for (auto& light : lights) {
        printf("%c", (light ? 'O' : '-'));
    }

    printf("\n");
}

void turn(int& light) {
    light = !light;
}

void turn3(std::vector<int>& t, int p) {
    int last = t.size() - 1;
    if (p == 0) {
        turn(t[last]);
        turn(t[0]);
        turn(t[1]);
    } else if (p < last){
        turn(t[p-1]);
        turn(t[p]);
        turn(t[p+1]);
    } else {
        turn(t[last-1]);
        turn(t[last]);
        turn(t[0]);
    }
}

void press(std::set<int>& p, int light) {
    auto i = p.lower_bound(light);
    if (i != p.end() && *i == light) {
        p.erase(i);
    } else {
        p.insert(i, light);
    }
}

std::vector<int> turnAllOn(const std::vector<int>& lights) {
    std::set<int> p;
    std::vector<int> t = lights;
    int total = t.size();

    // 第一轮，从 0 到 97 盏灯碰到灭的都打开
    for (int i = 0; i <= total - 3; i++) {
        if (!t[i]) {
            press(p, i + 1);
            turn3(t, i + 1);
        }
    }

    bool a = t[total - 2];
    bool b = t[total - 1];
    if (a && b) {
        // 已经全亮，结束
        return std::vector<int>(p.begin(), p.end());
    }

    // 单灯灭变成双灯灭
    int start = -1;
    if (a) {
        // OOO...OOF 变成  FFO...OOO
        press(p, 0);
        turn3(t, 0);
        start = 1;
    } else if (b) {
        // OOO...OFO 变成  FOO...OOF
        press(p, total - 1);
        turn3(t, total - 1);
        start = 0;
    } else {
        start = total - 1;
    }

    // 双灯灭的情况，从后一盏灯灭的灯开始顺序开灯，
    // 最后会刚好跟前一盏灯组成三连灭
    for (int i = 0; i <= total - 3; i++) {
        int c = start++;
        if (start == total) {
            start = 0;
        }

        if (!t[c]) {
            press(p, start);
            turn3(t, start);
        }
    }

    return std::vector<int>(p.begin(), p.end());
}

void replay(const std::vector<int>& lights, const std::vector<int>& presses) {
    std::vector<int> t = lights;
    printLights(t);
    for (auto p : presses) {
        turn3(t, p);
        printLights(t);
    }
}

int main() {
    auto lights = genLights();
    auto presses = turnAllOn(lights);
    printf("press %d times\n", presses.size());
    replay(lights, presses);
}
