#include <algorithm>
#include <assert.h>
#include <map>
#include <stdlib.h>
#include <vector>

struct Cord {
    int x = -1;
    int y = -1;

    Cord() = default;
    Cord(int x, int y) {
        this->x = x;
        this->y = y;
    }

    bool operator == (const Cord& a) {
        return x == a.x && y == a.y;
    }

    bool In(int w, int h);
};

struct Grids {
    std::vector<std::vector<int>> grids;

    Grids()=default;
    Grids(const std::vector<int>& v, int w, int h);

    int At(Cord cord) const;
    bool Erase(Cord c);
};

struct LineFree {
    std::map<int, int> freeRanges;

    LineFree(std::vector<int> pos, int n);
    LineFree(int p0, int p1);

    void Free(int p);
    void NearFree(int p, int* p0, int* p1) const;
    void MatchFree(LineFree* line) const;

    bool IsFree(int p) const;
    int Nearest(int p0, int p1);

    bool Empty() const;
};

struct Guider {
    Grids grids;
    std::map<int, std::vector<Cord>> blocks;
    std::vector<LineFree> rows;
    std::vector<LineFree> cols;

    void init();
    bool twoLineMatch(int a0, int a1, int b0, int b1,
            const std::vector<LineFree>& lines,
            int* bridge);

public:
    explicit Guider(Grids grids);

    void Clear();

    bool Match(Cord a, Cord b, std::vector<Cord>* path);
    bool Erase(Cord c);

    void Print(const std::vector<Cord>& path);
};

bool Cord::In(int w, int h) {
    return 0 <= x && x < w && 0 <= y && y < h;
}

Grids::Grids(const std::vector<int>& v, int w, int h) {
    grids.resize(h, std::vector<int>(w));
    for (size_t i = 0; i < v.size(); i++) {
        int y = i / w;
        int x = i % w;
        grids[y][x] = v[i];
    }
}

int Grids::At(Cord cord) const {
    if (grids.empty() || grids[0].empty()) {
        return 0;
    }

    if (cord.In(grids[0].size(), grids.size())) {
        return grids[cord.y][cord.x];
    } else {
        return 0;
    }
}

bool Grids::Erase(Cord cord) {
    if (grids.empty() || grids[0].empty()) {
        return false;
    }

    if (cord.In(grids[0].size(), grids.size())) {
        grids[cord.y][cord.x] = 0;
        return true;
    } else {
        return false;
    }
}

LineFree::LineFree(std::vector<int> pos, int n) {
    auto i = pos.begin();
    int p = 0;
    while (p != n) {
        int e = 0;
        if (i != pos.end()) {
            if (p == *i) {
                p++;
                i++;
                continue;
            } else {
                e = *i;
            }
        } else {
            e = n;
        }

        freeRanges[p] = e;
        p = e;
    }
}

LineFree::LineFree(int p0, int p1) {
    freeRanges[p0] = p1;
}

void LineFree::Free(int p) {
    auto i = freeRanges.upper_bound(p);
    bool match = false;
    if (i != freeRanges.begin()) {
        auto h = i;
        h--;
        if (p < h->second) {
            return;
        } else if (p == h->second) {
            h->second++;
            i = h;
            match = true;
        }
    }

    if (!match) {
        i = freeRanges.insert(i, std::make_pair(p, p + 1));
    }

    while (i != freeRanges.end()) {
        auto j = i;
        j++;
        if (j == freeRanges.end()) {
            break;
        } else if (i->second != j->first) {
            break;
        } else {
            i->second = j->second;
            freeRanges.erase(j);
        }
    }
}

void LineFree::NearFree(int p, int* p0, int* p1) const {
    auto i = freeRanges.upper_bound(p);
    if (i != freeRanges.begin()) {
        auto h = i;
        h--;
        if (h->second == p) {
            *p0 = h->first;
        } else {
            *p0 = p;
        }
    } else {
        *p0 = p;
    }

    if (i != freeRanges.end()) {
        if (i->first == p + 1) {
            *p1 = i->second;
        } else {
            *p1 = p + 1;
        }
    } else {
        *p1 = p + 1;
    }
}

void LineFree::MatchFree(LineFree* line) const {
    auto i = line->freeRanges.begin();
    auto j = freeRanges.begin();

    while (i != line->freeRanges.end() && j != freeRanges.end()) {
        if (i->second <= j->first) {
            i = line->freeRanges.erase(i);
        } else if (j->second <= i->first) {
            j++;
        } else {
            int b = std::max(i->first, j->first);
            int e = std::min(i->second, j->second);
            if (e != i->second) {
                line->freeRanges.insert(i, std::make_pair(e, i->second));
            }

            if (b != i->first) {
                auto h = i;
                i = line->freeRanges.insert(i, std::make_pair(b, e));
                line->freeRanges.erase(h);
            } else {
                i->second = e;
            }

            i++;
        }
    }

    while (i != line->freeRanges.end()) {
        i = line->freeRanges.erase(i);
    }
}

bool LineFree::IsFree(int p) const {
    auto i = freeRanges.upper_bound(p);
    if (i != freeRanges.begin()) {
        i--;
        return p < i->second;
    }

    return false;
}

int LineFree::Nearest(int p0, int p1) {
    if (p0 > p1) {
        std::swap(p0, p1);
    }

    auto i = freeRanges.upper_bound(p0);
    if (i != freeRanges.begin()) {
        auto h = i;
        h--;
        if (p0 < h->second) {
            return p0;
        }
    }

    auto j = freeRanges.upper_bound(p1);
    if (j != freeRanges.begin()) {
        auto h = j;
        h--;
        if (p1 < h->second) {
            return p1;
        }
    }

    if (i != j) {
        return i->first;
    }

    if (j != freeRanges.end()) {
        return j->first;
    } else if (i != freeRanges.begin()) {
        return (--i)->second;
    } else {
        return -1;
    }
}

bool LineFree::Empty() const {
    return freeRanges.empty();
}

Guider::Guider(Grids grids) {
    this->grids = grids;
    init();
}

void Guider::init() {
    if (grids.grids.empty() || grids.grids[0].empty()) {
        return;
    }

    int w = grids.grids[0].size();
    int h = grids.grids.size();

    for (int y = 0; y < h; y++) {
        auto& row = grids.grids[y];
        std::vector<int> rb;
        for (int x = 0; x < w; x++) {
            int b = row[x];
            if (b > 0) {
                blocks[b].push_back(Cord(x, y));
                rb.push_back(x);
            }
        }

        rows.push_back(LineFree(rb, w));
    }

    for (int x = 0; x < w; x++) {
        std::vector<int> cb;
        for (int y = 0; y < h; y++) {
            if (grids.grids[y][x] > 0) {
                cb.push_back(y);
            }
        }

        cols.push_back(LineFree(cb, h));
    }
}

bool Guider::twoLineMatch(int a0, int a1, int b0, int b1,
        const std::vector<LineFree>& lines,
        int* bridge) {
    int n00 = -1;
    int n01 = -1;
    lines[a0].NearFree(b0, &n00, &n01);

    int n10 = -1;
    int n11 = -1;
    lines[a1].NearFree(b1, &n10, &n11);

    LineFree free(n00, n01);
    LineFree(n10, n11).MatchFree(&free);

    int b = a0;
    int e = a1;
    if (b > e) {
        std::swap(b, e);
    }

    for (int i = b + 1; i != e && !free.Empty(); i++) {
        lines[i].MatchFree(&free);
    }

    *bridge = free.Nearest(b0, b1);
    return *bridge >= 0;
}

bool Guider::Match(Cord a, Cord b, std::vector<Cord>* path) {
    int block = grids.At(a);
    if (block <= 0) {
        return false;
    }

    if (grids.At(b) != block) {
        return false;
    }

    if (a.x != b.x) {
        int bridge = -1;
        if (twoLineMatch(a.x, b.x, a.y, b.y, cols, &bridge)) {
            path->push_back(a);
            if (bridge != a.y) {
                path->push_back(Cord{a.x, bridge});
            }

            if (bridge != b.y) {
                path->push_back(Cord{b.x, bridge});
            }

            path->push_back(b);
            return true;
        }
    }

    if (a.y != b.y) {
        int bridge = -1;
        if (twoLineMatch(a.y, b.y, a.x, b.x, rows, &bridge)) {
            path->push_back(a);
            if (bridge != a.x) {
                path->push_back(Cord(bridge, a.y));
            }

            if (bridge != b.x) {
                path->push_back(Cord(bridge, b.y));
            }

            path->push_back(b);
            return true;
        }
    }

    return false;
}

bool Guider::Erase(Cord c) {
    int b = grids.At(c);
    if (b <= 0) {
        return false;
    }

    grids.Erase(c);

    auto p = blocks.find(b);
    if (p == blocks.end()) {
        assert(false && "Guider.blocks can't find block");
        return false;
    }

    for (size_t i = 0; i < p->second.size(); i++) {
        if (p->second[i] == c) {
            p->second.erase(p->second.begin() + i);

            if (p->second.empty()) {
                blocks.erase(p);
            }

            break;
        }
    }

    rows[c.y].Free(c.x);
    cols[c.x].Free(c.y);
    return true;
}

void Guider::Print(const std::vector<Cord>& path) {
    std::vector<std::vector<std::string>> g;

    for (auto& row : grids.grids) {
        g.resize(g.size() + 1);
        for (auto& v : row) {
            if (v > 0) {
                char str[64];
                sprintf(str, "%d", v);
                g.back().push_back(str);
            } else {
                g.back().push_back(".");
            }
        }
    }

    if (!path.empty()) {
        printf("match %2d: ", grids.At(path[0]));

        const char* arrow = "";
        for (Cord c : path) {
            printf("%s(%2d, %2d)", arrow, c.x, c.y);
            arrow = " --> ";

            if (g[c.y][c.x] == ".") {
                g[c.y][c.x] = "+";
            }
        }

        printf("\n");
    }

    for (size_t i = 1; i < path.size(); i++) {
        Cord a = path[i - 1];
        Cord b = path[i];
        if (a.x == b.x) {
            int f = std::min(a.y, b.y);
            int e = std::max(a.y, b.y);
            for (int j = f + 1; j < e; j++) {
                g[j][a.x] = "|";
            }
        } else {
            int f = std::min(a.x, b.x);
            int e = std::max(a.x, b.x);
            for (int j = f + 1; j < e; j++) {
                g[a.y][j] = "--";
            }

            if (g[a.y][e] == "+") {
                g[a.y][e] = "-+";
            }
        }
    }

    for (auto& row : g) {
        for (auto& s : row) {
            printf("%3s", s.c_str());
        }

        printf("\n");
    }
}

Grids BuildGrids(int w, int h, int n) {
    std::vector<int> g(w * h);
    for (int i = 0; i < n; i++) {
        g[i] = i / 2 + 1;
    }

    std::random_shuffle(g.begin(), g.end());
    return Grids(g, w, h);
}

int main() {
    srand(time(0));

    Guider g(BuildGrids(16, 24, 98));
    g.Print(std::vector<Cord>());
    printf("\n");

    for (auto& b : g.blocks) {
        if (b.second.size() != 2) {
            printf("Guider.blocks[%d].size() != 2\n", b.first);
            continue;
        }

        printf("Try find a path of %d (%d, %d) <-> (%d, %d)\n",
                b.first, b.second[0].x, b.second[0].y, b.second[1].x, b.second[1].y);
        std::vector<Cord> path;
        if (g.Match(b.second[0], b.second[1], &path)) {
            //g.Print(path);
            printf("match %2d: ", g.grids.At(path[0]));

            const char* arrow = "";
            for (Cord c : path) {
                printf("%s(%2d, %2d)", arrow, c.x, c.y);
                arrow = " --> ";
            }

            printf("\n");
        } else {
            printf("Guider can't find a path\n");
        }

        printf("\n");
    }

    printf("-------------------------\n\n");

    int round = 0;
    while (!g.blocks.empty()) {
        round++;
        bool match = false;
        for (auto i = g.blocks.begin(); i != g.blocks.end(); ) {
            auto& b = *i++;

            std::vector<Cord> path;
            if (g.Match(b.second[0], b.second[1], &path)) {
                match = true;

                g.Print(path);

                g.Erase(b.second[0]);
                g.Erase(b.second[0]);

                printf("round %d, left %d pair blocks\n\n", round, g.blocks.size());
            }
        }

        if (!match) {
            break;
        }
    }

    if (!g.blocks.empty()) {
        printf("can't clear all blocks\n");
        g.Print({});
    }
}
