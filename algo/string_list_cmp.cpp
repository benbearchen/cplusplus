#include <vector>
#include <random>
#include <functional>
#include <stdio.h>
#include <set>
#include <unordered_set>
#include <algorithm>

uint64_t Rand64() {
    static std::mt19937_64 ee = []() {
        std::seed_seq sq = {
            uint64_t(std::chrono::system_clock::now().time_since_epoch().count()),
            uint64_t(&sq),
        };

        return std::mt19937_64(sq);
    }();

    return ee();
}

std::string makeString() {
    std::string s;
    int len = Rand64() % 256;
    s.resize(len);
    for (int i = 0; i < len; i++) {
	s[i] = Rand64() % 95 + 32;
    }

    return s;
}

std::vector<std::string> makeStringList(int n) {
    std::vector<std::string> a(n);
    for (int i = 0; i < n; i++) {
	a[i] = makeString();
    }

    return a;
}

int for0(std::vector<std::string> a, std::vector<std::string> b, std::string* tag) {
    *tag = "for&for";

    int c = 0;
    for (const auto& s : b) {
	for (const auto& t : a) {
	    if (s == t) {
		c++;
		break;
	    }
	}
    }

    return c;
}

int sort0(std::vector<std::string> a, std::vector<std::string> b, std::string* tag) {
    *tag = "sort&sort";

    std::sort(a.begin(), a.end());
    std::sort(b.begin(), b.end());

    auto i = a.begin();
    auto j = b.begin();
    int c = 0;
    while (i != a.end() && j != b.end()) {
	int d = i->compare(*j);
	if (d < 0) {
	    i++;
	} else if (d > 0) {
	    j++;
	} else if (d == 0) {
	    j++;
	    c++;
	}
    }

    return c;
}

struct Index {
    const std::string* p;
    size_t h;

    Index(const std::string& s) {
	p = &s;
	h = 0;

	if (!s.empty()) {
	    memcpy(&h, s.c_str(), std::min(sizeof(h), s.size()));
	}
    }

    bool operator< (const Index& b) const {
	return d(b) < 0;
    }

    int d(const Index& b) const {
	int d = h - b.h;
	if (d != 0) {
	    return d;
	} else {
	    return p->compare(*b.p);
	}
    }

    struct hash {
	size_t operator () (const Index& i) const {
	    return i.h;
	}
    };

    struct equal {
	bool operator() (const Index& ia, const Index& ib) const {
	    return ia.d(ib) == 0;
	}
    };
};

int sort0i(std::vector<std::string> a, std::vector<std::string> b, std::string* tag) {
    *tag = "sort-index";

    std::vector<Index> ia(a.begin(), a.end());
    std::vector<Index> ib(b.begin(), b.end());

    std::sort(ia.begin(), ia.end());
    std::sort(ib.begin(), ib.end());

    auto i = ia.begin();
    auto j = ib.begin();
    int c = 0;
    while (i != ia.end() && j != ib.end()) {
	int d = i->d(*j);
	if (d < 0) {
	    i++;
	} else if (d > 0) {
	    j++;
	} else if (d == 0) {
	    j++;
	    c++;
	}
    }

    return c;
}

int sort1(std::vector<std::string> a, std::vector<std::string> b, std::string* tag) {
    *tag = "sort&find";

    std::sort(a.begin(), a.end());

    int c = 0;
    for (const auto& s : b) {
	auto i = std::lower_bound(a.begin(), a.end(), s);
	if (i != a.end() && *i == s) {
	    c++;
	}
    }

    return c;
}

int set0(std::vector<std::string> a, std::vector<std::string> b, std::string* tag) {
    *tag = "set&find";

    std::set<std::string> s(a.begin(), a.end());
    int c = 0;
    for (size_t i = 0; i < b.size(); i++) {
	if (s.find(b[i]) != s.end()) {
	    ++c;
	}
    }

    return c;
}

int set1(std::vector<std::string> a, std::vector<std::string> b, std::string* tag) {
    *tag = "hashset&find";

    std::unordered_set<std::string> s(a.begin(), a.end());
    int c = 0;
    for (size_t i = 0; i < b.size(); i++) {
	if (s.find(b[i]) != s.end()) {
	    ++c;
	}
    }

    return c;
}

int set1i(std::vector<std::string> a, std::vector<std::string> b, std::string* tag) {
    *tag = "hash-index&find";

    std::vector<Index> ia(a.begin(), a.end());

    std::unordered_set<Index, Index::hash, Index::equal> s(ia.begin(), ia.end());
    int c = 0;
    for (size_t i = 0; i < b.size(); i++) {
	if (s.find(Index(b[i])) != s.end()) {
	    ++c;
	}
    }

    return c;
}

typedef std::function<int(std::vector<std::string>, std::vector<std::string>, std::string*)> foo;

void test(std::vector<foo> foos,
	  const std::vector<std::string>& a,
	  const std::vector<std::string>& b) {
    printf("string list size: %zu, %zu\n", a.size(), b.size());

    for (size_t i = 0; i < foos.size(); i++) {
	auto t0 = std::chrono::steady_clock::now();
	std::string tag;
	int n = foos[i](a, b, &tag);
	auto t1 = std::chrono::steady_clock::now();
	int d = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();
	printf("test%zu, dup %d, used %6dms, tag: %s\n", i, n, d, tag.c_str());
    }
}

int main() {
    auto a = makeStringList(Rand64() % 100000 + 100000);
    auto b = makeStringList(Rand64() % 100000 + 100000);

    test({for0, sort0, sort0i, sort1, set0, set1, set1i}, a, b);
}
