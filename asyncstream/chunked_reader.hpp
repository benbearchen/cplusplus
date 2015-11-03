#ifndef __ASYNC_STREAM_CHUNKED_READER_HPP__
#define __ASYNC_STREAM_CHUNKED_READER_HPP__

#include "asyncstream/reader.hpp"

#include <string>
#include <vector>

#include <stdio.h>
#include <string.h>

namespace asyncstream {

class ChunkedReader: public Reader {
public:
	ChunkedReader(Reader* next) :
		next_(next)
	{
	}

	virtual void OnRead(const char* buffer, int len) override {
		while (len > 0) {
			if (state_ == ChunkState::INIT) {
				InitChunk(std::ref(buffer), std::ref(len));
			} else if (state_ == ChunkState::CONTENT) {
				int left = leftContent_;
				if (left > len) {
					left = len;
				}

				next_->OnRead(buffer, left);
				buffer += left;
				len -= left;
				leftContent_ -= left;
				if (leftContent_ == 0) {
					state_ = ChunkState::CONTENT_CRLF;
				}
			} else if (state_ == ChunkState::OVER) {
				buffer_.insert(buffer_.end(), buffer, buffer + len);
				buffer += len;
				len = 0;
			} else if (state_ == ChunkState::ERROR) {
				len = 0;
			} else {
				CheckCRLF(std::ref(buffer), std::ref(len));
			}
		}
	}

	virtual void OnEOF(int state) override {
		next_->OnEOF(state);
	}

	bool CheckTail(std::vector<char>* left) {
		if (state_ != ChunkState::OVER) {
			return false;
		} else {
			left->swap(buffer_);
			buffer_.clear();
			return true;
		}
	}

private:
	enum class ChunkState {
		INIT,
		INIT_CRLF,
		CONTENT,
		CONTENT_CRLF,
		FINAL_CRLF,
		OVER,
		ERROR,
	};

	void InitChunk(const char*& buffer, int& len) {
		const char* cr = reinterpret_cast<const char*>(memchr(buffer, 0x0D, len));
		if (!cr) {
			buffer_.insert(buffer_.end(), buffer, buffer + len);
			buffer += len;
			len = 0;
			return;
		}

		int b = cr - buffer;
		buffer_.insert(buffer_.end(), buffer, cr);
		buffer = cr;
		len -= b;

		if (ParseContentLength()) {
			state_ = ChunkState::INIT_CRLF;
		} else {
			state_ = ChunkState::ERROR;
		}
	}

	bool ParseContentLength() {
		std::string buffer(buffer_.begin(), buffer_.end());
		buffer_.clear();

		int len = -1;
		if (sscanf(buffer.c_str(), "%x", &len) == 1 && len >= 0) {
			leftContent_ = len;
			return true;
		} else {
			return false;
		}
	}

	void CheckCRLF(const char*& buffer, int& len) {
		// check full CRLF
		bool crlf = false;
		if (!buffer_.empty()) {
			buffer_.clear();
			if (buffer[0] == 0x0A) {
				buffer += 1;
				len -= 1;
				crlf = true;
			} else {
				state_ = ChunkState::ERROR;
				len = 0;
			}
		} else if (len >= 2) {
			if (buffer[0] == 0x0D && buffer[1] == 0x0A) {
				buffer += 2;
				len -= 2;
				crlf = true;
			} else {
				state_ = ChunkState::ERROR;
				len = 0;
			}
		} else {
			buffer_.push_back(buffer[0]);
			buffer += 1;
			len = 0;
		}

		if (crlf) {
			if (state_ == ChunkState::INIT_CRLF) {
				if (leftContent_ == 0) {
					state_ = ChunkState::FINAL_CRLF;
				} else {
					state_ = ChunkState::CONTENT;
				}
			} else if (state_ == ChunkState::CONTENT_CRLF) {
				state_ = ChunkState::INIT;
			} else if (state_ == ChunkState::FINAL_CRLF) {
				state_ = ChunkState::OVER;
			} else {
				state_ = ChunkState::ERROR;
			}
		}
	}

	Reader* next_;
	// buffering chunk's leading HEX bytes when INIT state,
	// or bytes after all chunks when OVER state, or CR when *_CRLF
	std::vector<char> buffer_;
	ChunkState state_ = ChunkState::INIT;
	int leftContent_ = 0;
};

}

#endif // __ASYNC_STREAM_CHUNKED_READER_HPP__

