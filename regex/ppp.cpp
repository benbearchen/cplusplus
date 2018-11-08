#include <regex>
#include <stdio.h>
#include <string>
#include <vector>

// %xxx%-%yyyy%-%zzzzzzz%-%aaaaa%
const std::string first_word = "%([^%]*)%";
const std::string left_word = "-" + first_word;

std::vector<std::string> split(const std::string& line) {
    std::regex fw(first_word);
    std::regex lw(left_word);

    std::vector<std::string> ws;

    std::smatch m;
    if (!std::regex_search(line.cbegin(), line.cend(), m, fw)) {
        return ws;
    }

    ws.push_back(m[1].str());
    auto i = m.suffix().first;
    while (std::regex_search(i, line.cend(), m, lw)) {
        ws.push_back(m[1].str());
        i = m.suffix().first;
    }

    return ws;
}

void test(const std::string& line) {
    auto ws = split(line);
    if (ws.empty()) {
        printf("%s match 0\n", line.c_str());
    } else {
        printf("%s match %d word:\n", line.c_str(), ws.size());
        for (auto s : ws) {
            printf("\t%s\n", s.c_str());
        }
    }

    printf("\n");
}

int main() {
    test("%xxx%");
    test("%xxx%-%yyyy%-%zzzzzzz%-%aaaaa%");
}
