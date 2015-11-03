#ifndef __ASYNC_STREAM_READER_HPP__
#define __ASYNC_STREAM_READER_HPP__

namespace asyncstream {

class Reader {
public:
	virtual ~Reader() {}

	virtual void OnRead(const char* buffer, int len) = 0;
	virtual void OnEOF(int state) = 0;
};

}

#endif  // __ASYNC_STREAM_READER_HPP__
