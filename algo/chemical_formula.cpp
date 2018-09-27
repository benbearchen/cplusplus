#include <map>
#include <string>
#include <string.h>
#include <vector>

int takeNumber(const char** b) {
    const char* p = *b;
    int n = 0;
    while (*p && *p >= '0' && *p <= '9') {
        n = n * 10 + *p - '0';
        p++;
    }

    if (p == *b) {
    } else {
        *b = p;
    }

    return n;
}

std::map<std::string, int> SplitFormula(const char* b) {
    std::vector<std::map<std::string, int>> m(1);

    while (*b) {
        if (*b == '(') {
            m.resize(m.size() + 1);
            b++;
        } else if (*b == ')') {
            if (m.size() < 2) {
                printf("unmatch ) at: `%s'\n", b);
                exit(-1);
            }

            const char* p = b + 1;
            int n = takeNumber(&p);
            if (n < 2) {
                printf("invalid number after ): `%s'\n", b + 1);
                exit(-1);
            }

            std::map<std::string, int> v = std::move(m.back());
            m.pop_back();
            for (const auto& kv : v) {
                m.back()[kv.first] += kv.second * n;
            }

            b = p;
        } else if (*b >= 'A' && *b <= 'Z') {
            const char* p = b + 1;
            while (*p && *p >= 'a' && *p <= 'z') {
                p++;
            }

            std::string elem(b, p);
            const char* np = p;
            int n = takeNumber(&np);
            if (np == p) {
                n = 1;
            } else if (n < 2) {
                printf("invalid number after %s: %d\n", elem.c_str(), n);
                exit(-1);
            }

            m.back()[elem] += n;
            b = np;
        } else {
            printf("invalid begin char: %c\n", *b);
            exit(-1);
        }
    }

    if (m.size() != 1) {
        printf("miss %d ')'\n", m.size() - 1);
        exit(-1);
    }

    return m.front();
}

void show(const char* f) {
    auto m = SplitFormula(f);
    printf("elements of %s: \n\t", f);
    for (const auto& kv : m) {
        printf("%s%d", kv.first.c_str(), kv.second);
    }

    printf("\n");
}

int main() {
    show("Mg(OH)2");
}
