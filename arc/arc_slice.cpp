#include <math.h>
#include <stdio.h>

struct ScreenSlidePercent {
    int x0 = 0;
    int y0 = 0;
    int a0 = -30; // 右边的角度（最小 -90 度）
    int a1 = 210; // 左边的角度（最大 270 度）
    double pi = atan2(0, -100);

    double percent(int x, int y) {
        x -= x0;
        y -= y0;
        if (x == 0 && y == 0) {
            return 360;
        }

        double d = atan2(-y, x);
        if (d < -pi / 2) {
            // 比 -90 度还小的，调到大于 180~270 度
            d += 2 * pi;
        }

        // 换算成角度
        d *= 180 / pi;

        //printf("                          atan2(%4d, %4d) -> %7.2f\n", x, -y, d);
        return 100 - (d - a0) * 100 / (a1 - a0);
    }
};

void test(int x, int y) {
    ScreenSlidePercent ssp;
    printf("xy(%4d, %4d) -> %7.2f%%\n", x, y, ssp.percent(x, y));
}

int main() {
    test(100, 100);
    test(100, 80);
    test(100, 50);
    test(100, 30);
    test(100, 0);
    test(100, -30);
    test(100, -50);
    test(100, -80);
    test(100, -100);
    test(80, -100);
    test(50, -100);
    test(30, -100);
    test(0, -100);
    test(-30, -100);
    test(-50, -100);
    test(-80, -100);
    test(-100, -100);
    test(-100, -80);
    test(-100, -50);
    test(-100, -30);
    test(-100, 0);
    test(-100, 30);
    test(-100, 50);
    test(-100, 80);
    test(-100, 100);
}
