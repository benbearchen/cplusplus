#ifndef __ASYNC_STREAM_BLOCK_READER_HPP__
#define __ASYNC_STREAM_BLOCK_READER_HPP__

#include "asyncstream/reader.hpp"

#include <functional>
#include <vector>

namespace asyncstream {

class BlockReader: public Reader {
public:
	explicit BlockReader(int firstBlockSize = 0) :
		nextBlockSize_(firstBlockSize)
	{
	}

	virtual void OnRead(const char* buffer, int len) override {
		while (len > 0) {
			if (nextBlockSize_ == 0) {
				nextBlockSize_ = OnBlockRead(buffer, len);
				return;
			} else if (!buffer_.empty()) {
				int c = static_cast<int>(buffer_.size());
				int d = nextBlockSize_ - c;
				if (d > len) {
					buffer_.insert(buffer_.end(), buffer, buffer+len);
					return;
				}

				buffer_.insert(buffer_.end(), buffer, buffer + d);
				buffer += d;
				len -= d;

				nextBlockSize_ = OnBlockRead(&buffer_.front(), nextBlockSize_);
				buffer_.clear();
			} else if (nextBlockSize_ <= len) {
				int block = nextBlockSize_;
				nextBlockSize_ = OnBlockRead(buffer, block);
				buffer += block;
				len -= block;
			} else {
				buffer_.assign(buffer, buffer + len);
				return;
			}
		}
	}

	virtual void OnEOF(int state) override {
		if (!buffer_.empty()) {
			const char* buffer = &buffer_.front();
			int len = static_cast<int>(buffer_.size());
			nextBlockSize_ = OnBlockRead(buffer, len);
			buffer_.clear();
		}
	}

protected:
	virtual int OnBlockRead(const char* buffer, int len) = 0;

private:
	int nextBlockSize_ = 0;
	std::vector<char> buffer_;
};

class BlockReaderUtil: public BlockReader {
public:
	typedef std::function<int(const char*, int)> BlockFunc;

	explicit BlockReaderUtil(int firstBlockSize, BlockFunc f) :
		BlockReader(firstBlockSize),
		blockFunc_(f)
	{
	}

	BlockReaderUtil(const BlockReaderUtil&) = delete;
	BlockReaderUtil& operator= (const BlockReaderUtil&) = delete;

protected:
	virtual int OnBlockRead(const char* buffer, int len) override {
		if (blockFunc_) {
			return blockFunc_(buffer, len);
		} else {
			return 0;
		}
	}

private:
	BlockFunc blockFunc_;
};

}

#endif // __ASYNC_STREAM_BLOCK_READER_HPP__
