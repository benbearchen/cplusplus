#include <vector>

void p(const std::vector<int>& v2) {
	printf("p.v2.size() %lu\n", v2.size());
}

int main() {
	std::vector<int> v(2);
	//std::vector<int>& vv = v;
	const std::vector<int>& v2(std::move(v));
	printf("v.size() %lu, v2.size() %lu\n", v.size(), v2.size());
	p(std::move(v));
	v.push_back(1);
	printf("v.size() %lu, v2.size() %lu\n", v.size(), v2.size());
}
