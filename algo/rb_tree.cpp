#include <algorithm>
#include <chrono>
#include <map>
#include <memory>
#include <string>
#include <vector>

struct Node {
    int Key = -1;
    bool Red = false;
    Node* Parent = nullptr;
    std::unique_ptr<Node> Son[2];

    static Node* MakeRoot(int k);
    static Node* MakeLeaf(int k, Node* parent);

    Node* Find(int k);
    Node* Insert(int k);

    bool MatchParent() const;

    Node* Next();
    Node* Prev();

    Node* MostLeft();
    Node* MostRight();

    Node* FirstParentInLeft();
    Node* FirstParentInRight();

    int Depth() const;

    std::unique_ptr<Node> Clone(Node* parent);

    void Debug() const;
    bool Test(int* colorDepth, bool compareNumber) const;

private:
    explicit Node(int k);
    Node(int k, Node* parent);
};

struct RBTree {
    std::unique_ptr<Node> Root;

    bool Empty() const;
    Node* Find(int k) const;
    Node* Insert(int k);
    bool Erase(int k);
    void Clear();

    Node* MostLeft();
    Node* MostRight();

    void SwapNode(Node* a, Node* b);

    void Rotate(Node* node, int dir); // dir == 0 for left, dir == 1 for right

    void Print();
    bool Test(bool compareNumber) const;

    RBTree Clone();

private:
    std::unique_ptr<Node>& OwnerPtr(Node* n);
};

/****************************************
 * Node
 ***************************************/
Node* Node::MakeRoot(int k) {
    return new Node(k);
}

Node* Node::MakeLeaf(int k, Node* parent) {
    return new Node(k, parent);
}

Node::Node(int k) {
    Key = k;
}

Node::Node(int k, Node* parent) {
    Key = k;
    Red = true;
    Parent = parent;
}

Node* Node::Find(int k) {
    if (Key == k) {
        return this;
    }

    auto& s = Son[k > Key];
    if (s) {
        return s->Find(k);
    }

    return nullptr;
}

Node* Node::Insert(int k) {
    if (Key == k) {
        return this;
    }

    auto& s = Son[k > Key];
    if (s) {
        return s->Insert(k);
    }

    s.reset(Node::MakeLeaf(k, this));
    //printf("insert %d to %d\n", k, Key);
    return s.get();
}

bool Node::MatchParent() const {
    if (!Parent) {
        return true;
    }

    if (Parent->Red && Red) {
        return false;
    }

    return true;
}

Node* Node::Next() {
    if (Son[1]) {
        return Son[1]->MostLeft();
    }

    return FirstParentInRight();
}

Node* Node::Prev() {
    if (Son[0]) {
        return Son[0]->MostRight();
    }

    return FirstParentInLeft();
}

Node* Node::MostLeft() {
    Node* n = this;
    while (n->Son[0]) {
        n = n->Son[0].get();
    }

    return n;
}

Node* Node::MostRight() {
    Node* n = this;
    while (n->Son[1]) {
        n = n->Son[1].get();
    }

    return n;
}

Node* Node::FirstParentInLeft() {
    Node* n = this;
    while (n->Parent) {
        if (n->Parent->Son[0].get() == n) {
            n = n->Parent;
        } else {
            return n->Parent;
        }
    }

    return nullptr;
}

Node* Node::FirstParentInRight() {
    Node* n = this;
    while (n->Parent) {
        if (n->Parent->Son[1].get() == n) {
            n = n->Parent;
        } else {
            return n->Parent;
        }
    }

    return nullptr;
}

int Node::Depth() const {
    int c = 1;
    Node* p = Parent;
    while (p) {
        p = p->Parent;
        c++;
    }

    return c;
}

void Node::Debug() const {
    char p[16] = "nil";
    if (Parent) {
        sprintf(p, "%d", Parent->Key);
    }

    char l[16] = "nil";
    if (Son[0]) {
        sprintf(l, "%d", Son[0]->Key);
    }

    char r[16] = "nil";
    if (Son[1]) {
        sprintf(r, "%d", Son[1]->Key);
    }

    printf("Node %5d Color %s Parent(%5s), Son[%5s, %5s]\n",
           Key, (Red ? "R" : "B"), p, l, r);
}

bool Node::Test(int* colorDepth, bool compareNumber) const {
    bool ok = true;
    if (Parent) {
        if ((Parent->Son[0].get() == this) + (Parent->Son[1].get() == this) != 1) {
            printf("can't find %d in parent %d\n", Key, Parent->Key);
            ok = false;
        }
    }

    int d[2] = {1, 1};
    for (int i = 0; i <= 1; i++) {
        if (Son[i]) {
            const char* side = (i == 0) ? "left" : "right";
            if (Son[i]->Parent != this) {
                printf("parent %d of %5s son %d is not %d\n", Son[i]->Parent->Key, side, Son[i]->Key, Key);
                ok = false;
            }

            if (!Son[i]->Test(&d[i], compareNumber)) {
                ok = false;
            }

            auto cmp = [i](int a, int b) -> bool {
                return (i == 0) ? (a < b) : (a > b);
            };

            if (compareNumber && !cmp(Son[i]->Key, Key)) {
                printf("key of %5s son %4d unmatch %4d\n", side, Son[i]->Key, Key);
                ok = false;
            }
        }
    }

    if (d[0] != d[1]) {
        printf("unmatch color depth of sons: %d vs %d\n", d[0], d[1]);
        ok = false;
    }

    *colorDepth = std::max(d[0], d[1]) + (Red ? 0 : 1);

    return ok;
}

std::unique_ptr<Node> Node::Clone(Node* parent) {
    std::unique_ptr<Node> c(MakeLeaf(Key, parent));
    c->Red = Red;
    for (int i = 0; i <= 1; i++) {
        if (Son[i]) {
            c->Son[i] = Son[i]->Clone(c.get());
        }
    }

    return c;
}

/********************************
 * RBTree
 *******************************/
bool RBTree::Empty() const {
    return !Root;
}

Node* RBTree::Find(int k) const {
    if (!Root) {
        return nullptr;
    }

    return Root->Find(k);
}

Node* RBTree::Insert(int k) {
    if (!Root) {
        Root.reset(Node::MakeRoot(k));
        return Root.get();
    }

    Node* n = Root->Insert(k);
    while (!n->MatchParent()) {
        Node* a = n->Parent->Parent;
        int b = 1 - (a->Son[1].get() == n->Parent);
        if (a->Son[b] && a->Son[b]->Red) {
            a->Red = true;
            a->Son[0]->Red = false;
            a->Son[1]->Red = false;
            //printf("insert case 1: %d -> %d -> %d <- %d\n", n->Key, n->Parent->Key, a->Key, a->Son[b]->Key);
            n = a;
            continue;
        }

        if (n->Parent->Son[b].get() == n) {
            //printf("insert case 2: %d <- %d\n", n->Parent->Key, n->Key);
            n = n->Parent;
            Rotate(n, 1 - b);
        }

        //printf("insert case 3: %d -> %d -> %d\n", n->Key, n->Parent->Key, n->Parent->Parent->Key);
        n->Parent->Red = false;
        n->Parent->Parent->Red = true;
        Rotate(n->Parent->Parent, b);
    }

    Root->Red = false;
    return n;
}

bool RBTree::Erase(int k) {
    if (!Root) {
        return false;
    }

    Node* n = Root->Find(k);
    if (!n) {
        return false;
    }

    if (n->Son[0] && n->Son[1]) {
        Node* e = n->Son[1]->MostLeft();
        SwapNode(n, e);
    }

    Node* p = n->Parent;
    if (true) {
        std::unique_ptr<Node>& np = OwnerPtr(n);
        std::unique_ptr<Node>& sp = n->Son[0] ? n->Son[0] : n->Son[1];
        bool red = n->Red;

        if (sp) {
            sp->Parent = p;
            if (!p) {
                sp->Red = false;
            }
        }

        np = std::move(sp);
        if (red || !p) {
            return true;
        }

        n = np.get();
    }

    while (n != Root.get() && (!n || !n->Red)) {
        int b = 1 - (p->Son[1].get() == n);
        Node* w = p->Son[b].get();
        if (w->Red) {
            p->Red = true;
            w->Red = false;

            Rotate(p, 1 - b);
            w = p->Son[b].get();
        }

        bool r0 = w->Son[1-b] && w->Son[1-b]->Red;
        bool r1 = w->Son[b] && w->Son[b]->Red;
        if (!r0 && !r1) {
            w->Red = true;
            n = p;
            p = p->Parent;
            continue;
        }

        if (!r1) {
            w->Red = true;
            w->Son[1-b]->Red = false;
            Rotate(w, b);
            w = p->Son[b].get();
        }

        w->Red = p->Red;
        p->Red = false;
        w->Son[b]->Red = false;
        Rotate(p, 1 - b);
        n = Root.get();
    }

    if (n) {
        n->Red = false;
    }

    return true;
}

void RBTree::Clear() {
    Root.reset();
}

Node* RBTree::MostLeft() {
    if (Root) {
        return Root->MostLeft();
    } else {
        return nullptr;
    }
}

Node* RBTree::MostRight() {
    if (Root) {
        return Root->MostRight();
    } else {
        return nullptr;
    }
}

void RBTree::SwapNode(Node* a, Node* b) {
    std::unique_ptr<Node>& pa = OwnerPtr(a);
    std::unique_ptr<Node>& pb = OwnerPtr(b);

    pa.swap(pb);
    // 上一行要在下面两行之前执行，否则当 a、b 存在父子关系的时候会出错
    a->Son[0].swap(b->Son[0]);
    a->Son[1].swap(b->Son[1]);

    std::swap(a->Red, b->Red);
    std::swap(a->Parent, b->Parent);

    if (a->Son[0]) {
        a->Son[0]->Parent = a;
    }

    if (a->Son[1]) {
        a->Son[1]->Parent = a;
    }

    if (b->Son[0]) {
        b->Son[0]->Parent = b;
    }

    if (b->Son[1]) {
        b->Son[1]->Parent = b;
    }

    int depth;
    if (!a->Test(&depth, false) || !b->Test(&depth, false)) {
        printf("SwapNode failed:\n");
        a->Debug();
        b->Debug();
    }
}

void RBTree::Rotate(Node* n, int dir) {
    std::unique_ptr<Node>& p = OwnerPtr(n);
    if (dir == 0) {
        //printf("rotate  left: %d <- %d\n", n->Key, n->Son[1]->Key);
    } else {
        //printf("rotate right: %d -> %d\n", n->Son[0]->Key, n->Key);
    }

    std::unique_ptr<Node> rn = std::move(p);

    p = std::move(n->Son[1-dir]);
    (p)->Parent = n->Parent;

    std::unique_ptr<Node>& nSon = n->Son[1-dir];
    nSon = std::move((p)->Son[dir]);
    if (nSon) {
        nSon->Parent = n;
    }

    n->Parent = p.get();
    p->Son[dir] = std::move(rn);
}

RBTree RBTree::Clone() {
    RBTree bee;
    if (Root) {
        bee.Root = Root->Clone(nullptr);
    }

    return std::move(bee);
}

void RBTree::Print() {
    printf("<<<<<=====================================================\n");
    std::vector<Node*> nodes;
    if (Root) {
        struct Collect {
            void Do(Node* n, std::vector<Node*>& nodes) {
                if (n->Son[1]) {
                    Do(n->Son[1].get(), nodes);
                }

                nodes.push_back(n);
                if (n->Son[0]) {
                    Do(n->Son[0].get(), nodes);
                }
            }
        } collect;
        collect.Do(Root.get(), nodes);
    }

    int maxDepth = 0;
    std::map<Node*, int> index;
    for (size_t i = 0; i < nodes.size(); i++) {
        Node* n = nodes[i];
        int depth = n->Depth();
        if (depth > maxDepth) {
            maxDepth = depth;
        }

        index[n] = i;
    }

    std::vector<std::vector<std::string>> grids;
    grids.resize(nodes.size(), std::vector<std::string>(maxDepth, "\t"));

    for (int i = 0; i < int(nodes.size()); i++) {
        Node* n = nodes[i];
        int depth = n->Depth();
        if (n->Parent) {
            Node* p = n->Parent;
            int j = index[p];
            if (j < i) {
                for (int k = j + 1; k < i; k++) {
                    grids[k][depth-2] = " \\\t";
                }

                grids[i][depth-2] = " \\----- ";
            } else {
                for (int k = i + 1; k < j; k++) {
                    grids[k][depth-2] = " /\t";
                }

                grids[i][depth-2] = " /----- ";
            }
        }

        char s[32];
        sprintf(s, "%d %s\t", n->Key, (n->Red ? "R" : " "));
        grids[i][depth-1] = s;
    }

    for (const auto& row : grids) {
        for (const auto& cell : row) {
            printf("%s", cell.c_str());
        }

        printf("\n");
    }
    printf("=====================================================>>>>>\n");
}

bool RBTree::Test(bool compareNumber) const {
    if (!Root) {
        return true;
    }

    if (Root->Parent) {
        printf("parent %d of Root is not nil\n", Root->Parent->Key);
        return false;
    }

    int colorDepth = 0;
    if (!Root->Test(&colorDepth, compareNumber)) {
        return false;
    }

    if (colorDepth <= 0) {
        printf("color depth invalid: %d\n", colorDepth);
        return false;
    }

    return true;
}

std::unique_ptr<Node>& RBTree::OwnerPtr(Node* n) {
    if (n == Root.get()) {
        return Root;
    }

    int b = n->Parent->Son[1].get() == n;
    return n->Parent->Son[b];
}

int ms(std::chrono::steady_clock::time_point a,
       std::chrono::steady_clock::time_point b) {
    return std::chrono::duration_cast<std::chrono::milliseconds>(b - a).count();
}

void test(int n, int t, bool performance) {
    RBTree bee;

    std::vector<int> numbers;
    for (int i = 0; i < n; i++) {
        numbers.push_back(numbers.size() + 1);
    }

    auto ti0 = std::chrono::steady_clock::now();
    for (int i = 0; i < t; i++) {
        bee.Clear();

        std::random_shuffle(numbers.begin(), numbers.end());
        for (int n : numbers) {
            bee.Insert(n);
            if (!performance) {
                //printf("insert %d\n", n);
                //bee.Print();

                if (!bee.Find(n)) {
                    printf("can't find %d\n", n);
                }

                if (!bee.Test(true)) {
                    printf("state fail\n");
                }
            }
        }
    }

    auto ti1 = std::chrono::steady_clock::now();

    if (!performance) {
        //printf("all inserted:\n");
        //bee.Print();

        int c = 1;
        for (Node* i = bee.MostLeft(); i; i = i->Next(), c++) {
            if (c != i->Key) {
                printf("can't match %d vs %d\n", c, i->Key);
                i->Debug();
                break;
            }
        }

        c = numbers.size();
        for (Node* i = bee.MostRight(); i; i = i->Prev(), c--) {
            if (c != i->Key) {
                printf("can't match %d vs %d\n", c, i->Key);
                i->Debug();
                break;
            }
        }
    }

    auto ts0 = std::chrono::steady_clock::now();
    int miss = 0;
    for (int i = 0; i < t; i++) {
        std::random_shuffle(numbers.begin(), numbers.end());
        for (int n : numbers) {
            if (!bee.Find(n)) {
                miss++;
            }
        }
    }

    auto ts1 = std::chrono::steady_clock::now();

    auto te0 = std::chrono::steady_clock::now();
    for (int i = 0; i < t; i++) {
        RBTree tree = bee.Clone();
        std::random_shuffle(numbers.begin(), numbers.end());
        for (int n : numbers) {
            bool erased = tree.Erase(n);
            if (!performance) {
                //printf("erase %d\n", n);
                if (!erased) {
                    printf("can't remove %d from tree\n", n);
                }

                if (bee.Find(n)) {
                    printf("found a erased key: %d\n", n);
                }

                //bee.Print();
                if (!bee.Test(true)) {
                    printf("state fail\n");
                }
            }
        }
    }

    auto te1 = std::chrono::steady_clock::now();
    printf(" RBTree  %8d numbers in %8d times, clear&insert %6dms, search %6dms, clone&erase %6dms\n",
           n, t, ms(ti0, ti1), ms(ts0, ts1), ms(te0, te1));
}

void testSTL(int n, int t) {
    std::map<int, int> m;

    std::vector<int> numbers;
    for (int i = 0; i < n; i++) {
        numbers.push_back(numbers.size() + 1);
    }

    auto ti0 = std::chrono::steady_clock::now();
    for (int i = 0; i < t; i++) {
        m.clear();

        std::random_shuffle(numbers.begin(), numbers.end());
        for (int n : numbers) {
            m[n] = 0;
        }
    }

    auto ti1 = std::chrono::steady_clock::now();

    auto ts0 = std::chrono::steady_clock::now();
    int miss = 0;
    for (int i = 0; i < t; i++) {
        std::random_shuffle(numbers.begin(), numbers.end());
        for (int n : numbers) {
            if (m.find(n) == m.end()) {
                miss++;
            }
        }
    }

    auto ts1 = std::chrono::steady_clock::now();

    auto te0 = std::chrono::steady_clock::now();
    for (int i = 0; i < t; i++) {
        std::map<int, int> tree = m;
        std::random_shuffle(numbers.begin(), numbers.end());
        for (int n : numbers) {
            tree.erase(n);
        }
    }

    auto te1 = std::chrono::steady_clock::now();
    printf("std::map %8d numbers in %8d times, clear&insert %6dms, search %6dms, clone&erase %6dms\n",
           n, t, ms(ti0, ti1), ms(ts0, ts1), ms(te0, te1));
}

int main() {
    srand(time(0));

    int total = 10000000;
    for (int n = 100; n <= total; n *= 10) {
        test(n, total / n, true);
        testSTL(n, total / n);
    }
}
