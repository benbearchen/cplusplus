#include "lazy_buffer.hpp"

#include <iostream>
#include <string>

int main() {
	buffer::LazyBuffer<char> lazy;
	std::vector<char>* v = reinterpret_cast<std::vector<char>*>(&lazy);
	int* o = reinterpret_cast<int*>(v+1);

	auto push = [&](const std::string& buf, int offset, int size){
		std::string ov(v->begin(), v->end());
		int oo = *o;

		lazy.PushBack(buf.c_str(), buf.size());
		if (*o != offset || v->size() != size) {
			std::cout << "error: (`" << ov << "', " << oo << ") push(`" << buf << "'), out (`" << std::string(v->begin(), v->end()) << "', " << *o << ")" << std::endl;
		}
	};

	auto pop = [&](int n, int offset, int size){
		std::string ov(v->begin(), v->end());
		int oo = *o;

		lazy.PopFront(n);
		if (*o != offset || v->size() != size) {
			std::cout << "error: (`" << ov << "', " << oo << ") pop(" << n << "), out (`" << std::string(v->begin(), v->end()) << "', " << *o << ")" << std::endl;
		}
	};

	auto shrink = [&](int offset, int size){
		std::string ov(v->begin(), v->end());
		int oo = *o;

		lazy.Shrink();
		if (*o != offset || v->size() != size) {
			std::cout << "error: (`" << ov << "', " << oo << ") shrink(), out (`" << std::string(v->begin(), v->end()) << "', " << *o << ")" << std::endl;
		}
	};

	auto read = [&](int n, const std::string& data) {
		std::vector<char> buf(n);
		int r = lazy.Read(&buf.front(), n);
		std::string read(buf.begin(), buf.begin() + r);
		if (read != data) {
			std::cout << "error: read(n) get `" << read << "' != `" << data << "'" << std::endl;
		}
	};

	push("abc", 0, 3);
	read(4, "abc");
	read(1, "a");
	push("123", 0, 6);
	read(4, "abc1");
	pop(2, 2, 6);
	read(4, "c123");
	read(3, "c12");
	shrink(0, 4);
	read(4, "c123");
	read(5, "c123");
}
