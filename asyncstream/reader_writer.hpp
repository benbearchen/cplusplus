#ifndef __ASYNC_STREAM_READER_WRITER_HPP__
#define __ASYNC_STREAM_READER_WRITER_HPP__

#include "asyncstream/reader.hpp"
#include "asyncstream/writer.hpp"

#include <memory>
#include <vector>

namespace asyncstream {

class ReaderWriter: public Reader, public std::enable_shared_from_this<ReaderWriter> {
public:
	ReaderWriter()
	{
	}

	void Bind(std::shared_ptr<Writer> writer) {
		writer_ = writer;

		std::weak_ptr<ReaderWriter> p = shared_from_this();
		writer_->SetOnCanWrite([=](int suggestLength) {
			auto r = p.lock();
			if (r) {
				r->TryWrite(suggestLength);
			}
		});
	}

	virtual void OnRead(const char* buffer, int len) override {
		TryWrite(0);
		if (buffer_.empty()) {
			size_t w = writer_->Write(buffer, len);
			if (w != len) {
				buffer_.assign(buffer + w, buffer + len);
			}
		} else {
			buffer_.insert(buffer_.end(), buffer, buffer + len);
		}
	}

	virtual void OnEOF(int state) override {
		closed_ = true;
		TryWrite(0);
	}

private:
	void TryWrite(int len) {
		if (!buffer_.empty()) {
			int left = static_cast<int>(buffer_.size());
			if (len != 0 && left > len) {
				left = len;
			}

			int w = writer_->Write(&buffer_.front(), left);
			buffer_.erase(buffer_.begin(), buffer_.begin() + w);
		}

		if (buffer_.empty() && closed_) {
			writer_->Close();
			writer_ = nullptr;
		}
	}

	std::shared_ptr<Writer> writer_;
	std::vector<char> buffer_;
	bool closed_ = false;
};

}

#endif // __ASYNC_STREAM_READER_WRITER_HPP__
