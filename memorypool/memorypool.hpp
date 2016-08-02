#ifndef __CPP_LOOOONG_MEMORY_POOL_HPP__
#define __CPP_LOOOONG_MEMORY_POOL_HPP__

#include <memory>
#include <mutex>
#include <unordered_map>

namespace memorypool {

class MemoryPool {
public:
	std::shared_ptr<uint8_t> Shared(size_t size) {
		size = align_size(size);
		uint8_t* m = alloc_memory(size);
		return std::shared_ptr<uint8_t>(m, [=](uint8_t* m) { release_memory(m, size); });
	}

	size_t Count() {
		std::lock_guard<std::mutex> lock(mutex);
		return mems.size();
	}

	void Clear() {
		std::lock_guard<std::mutex> lock(mutex);
		for (auto i = begin(mems); i != end(mems); ++i) {
			delete[] i->first;
		}

		mems.clear();
	}

private:
	static size_t align_size(size_t size) {
		switch (size) {
			case 0:
			case 1:
				return 1;
			case 2:
				return 2;
			case 3:
			case 4:
				return 4;
			case 5:
			case 6:
			case 7:
			case 8:
				return 8;
			default:
				return (size + 7) & ~0x07;
		}
	}

	uint8_t* alloc_memory(size_t size) {
		std::lock_guard<std::mutex> lock(mutex);
		for (auto i = begin(mems); i != end(mems); ++i) {
			if (i->second == size) {
				mems.erase(i);
				return i->first;
			}
		}

		return new uint8_t[size];
	}

	void release_memory(uint8_t* m, size_t size) {
		std::lock_guard<std::mutex> lock(mutex);
		if (mems.size() < 128) {
			mems.insert(std::make_pair(m, size));
		} else {
			delete[] m;
		}
	}

	std::mutex mutex;
	std::unordered_map<uint8_t*, size_t> mems;
};

}

#endif

