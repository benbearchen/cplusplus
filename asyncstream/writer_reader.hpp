#ifndef __ASYNC_STRAM_WRITER_READER_HPP__
#define __ASYNC_STRAM_WRITER_READER_HPP__

#include "asyncstream/reader.hpp"
#include "asyncstream/writer.hpp"

#include <memory>

namespace asyncstream {

class WriterReader: public Writer {
public:
	WriterReader(std::shared_ptr<Reader> reader) :
		reader_(reader)
	{
	}

	virtual void SetOnCanWrite(std::function<void(int suggestLength)> onCanWrite) override {
	}

	virtual int Write(const char* buffer, int len) override {
		reader_->OnRead(buffer, len);
		return len;
	}

	virtual void Close() override {
		reader_->OnEOF(1);
		reader_.reset();
	}

private:
	std::shared_ptr<Reader> reader_;
};

}

#endif // __ASYNC_STRAM_WRITER_READER_HPP__
