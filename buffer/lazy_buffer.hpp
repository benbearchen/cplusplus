#ifndef __BUFFER_LASY_BUFFER_HPP__
#define __BUFFER_LASY_BUFFER_HPP__

#include <vector>

namespace buffer {

template <typename T>
class LazyBuffer {
public:
	void Reserve(int capacity) {
		int realCapacity = buffer_.capacity() - offset_;
		if (realCapacity >= capacity) {
			return;
		}

		if (offset_ != 0) {
			buffer_.erase(buffer_.begin(), buffer_.begin() + offset_);
			offset_ = 0;
		}

		buffer_.reserve(capacity);
	}

	void Shrink() {
		if (offset_ != 0 || buffer_.size() < buffer_.capacity()) {
			std::vector<T> buffer(buffer_.begin() + offset_, buffer_.end());
			buffer_.swap(buffer);
			offset_ = 0;
		}
	}

	void PushBack(const T* buffer, int len) {
		EnsureCapacity(len);
		buffer_.insert(buffer_.end(), buffer, buffer + len);
	}

	void PopFront(int len) {
		offset_ += len;
	}

	int Read(T* buffer, int len) {
		int left = static_cast<int>(buffer_.size()) - offset_;
		if (left - len > 0) {
			left = len;
		}

		std::copy(buffer_.begin() + offset_, buffer_.begin() + offset_ + left, buffer);
		return left;
	}

private:
	void EnsureCapacity(int len) {
		int needCapacity = static_cast<int>(buffer_.size()) - offset_ + len;
		Reserve(needCapacity);
	}

	std::vector<T> buffer_;
	int offset_ = 0;
};

}

#endif // __BUFFER_LASY_BUFFER_HPP__
