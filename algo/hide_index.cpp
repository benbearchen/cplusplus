#include <algorithm>
#include <stdio.h>
#include <vector>

template <typename Iter>
int findIndex(Iter begin, Iter end, int index) {
    if (begin == end) {
        return index;
    }

    Iter b = begin;
    Iter e = end;
    while (b != e) {
        Iter m = b + (e - b) / 2;
        int s = *m - (m - begin);
        if (s <= index) {
            b = m + 1;
        } else if (s > index) {
            e = m;
        }
    }

    return index + (e - begin);
}

template <typename Iter>
int diffIndex(Iter db, Iter de, int index) {
    Iter p = std::upper_bound(db, de, index);
    return index + (p - db);
}

int main() {
    std::vector<int> hides = {2, 3, 7, 9};
    std::vector<int> diffs;

    printf("hide index: \t");

    for (auto i = hides.begin(); i != hides.end(); i++) {
        diffs.push_back(*i - (i - hides.begin()));

        printf("%d,", *i);
    }

    printf("\n");

    for (int i = 0; i < 10; i++) {
        printf("index=%2d ==> %2d,  ===> %2d\n",
               i,
               findIndex(hides.begin(), hides.end(), i),
               diffIndex(diffs.begin(), diffs.end(), i));
    }
}
