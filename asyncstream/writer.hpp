#ifndef __ASYNC_STREAM_WRITER_HPP__
#define __ASYNC_STREAM_WRITER_HPP__

#include <functional>

namespace asyncstream {

class Writer {
public:
	virtual ~Writer() {}

	virtual void SetOnCanWrite(std::function<void(int suggestLength)> onCanWrite) = 0;
	virtual int Write(const char* buffer, int len) = 0;
	virtual void Close() = 0;
};

}

#endif // __ASYNC_STREAM_WRITER_HPP__
