#ifndef RECEIVER_HPP
#define RECEIVER_HPP

#include "webserv.hpp"

class Request;
class Poller;

struct Buffer
{
	char *first;
	char *last;
	char *pos;
	size_t capacity;
};

class Receiver
{
private:
	int fd;
	Request &request;
	std::list<Buffer *> buffers;
	size_t headerBufferCount;
	bool readingHeader;
	size_t bodySize;
	size_t bodyBytesRecvd;

	enum BufferType
	{
		HEADER,
		BODY
	};

	void sendLineToParsing(const Buffer *lbuffer, char *lf);
	void flushHeaderBuffers();
	Buffer *createBuffer(BufferType type);
	ssize_t handleRecv(void *buf, size_t len);
	void fillBuffer(BufferType type);

	static char *ws_strchr(char *first, const char *const last, char c);

public:
	Receiver(int fd, Request &request);
	~Receiver();

	bool receive();
};

#endif
