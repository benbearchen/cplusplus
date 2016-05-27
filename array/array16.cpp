#include <array>
#include <iostream>
#include <numeric>
#include <vector>

struct MD5: public std::array<uint8_t, 16> {
    MD5() :
            std::array<uint8_t, 16>()
    {
    }

    MD5(const std::array<uint8_t, 16>& a) :
            std::array<uint8_t, 16>(a)
    {
    }

    uint32_t sum() {
        return std::accumulate(begin(), end(), uint32_t(0));
    }

    std::string hex() {
        char buf[33];
        char* p = buf;
        for (auto i = begin(); i != end(); i++) {
            sprintf(p, "%02x", *i);
            p += 2;
        }

        return std::move(std::string(buf));
    }
};

int main() {
    std::cout << "sizeof MD5: " << sizeof(MD5) << std::endl;

    MD5 md5;
    std::vector<uint8_t> v(16);

    std::cout << "addr MD5: " << &md5 << " vs " << (void*)(md5.data()) << std::endl;
    std::cout << "addr vec: " << &v << " vs " << (void*)(&v.front()) << std::endl;

    std::cout << md5.hex() << "/" << md5.size() << " sum:" << md5.sum() << std::endl;

    md5 = std::array<uint8_t, 16>{1, 2};
    std::cout << md5.hex() << "/" << md5.size() << " sum:" << md5.sum() << std::endl;

    std::array<uint8_t, 16> a = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16};
    md5 = a;
    std::cout << md5.hex() << "/" << md5.size() << " sum:" << md5.sum() << std::endl;
}
