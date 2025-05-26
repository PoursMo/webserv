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
	enum BufferType
	{
		HEADER,
		BODY
	};

	int clientFd;
	Request &request;
	std::list<Buffer *> buffers;
	size_t headerBufferCount;
	bool readingHeader;
	size_t bodySize;
	size_t bodyBytesRecvd;
	ssize_t bytesRecvd;

	void sendHeaderLineToParsing(const Buffer *lbuffer, char *lf);
	void flushHeaderBuffers();
	Buffer *createBuffer(BufferType type);
	bool fillBuffer(BufferType type);
	void handleRecv(void *buf, size_t len);

	static char *ws_strchr(char *first, const char *const last, char c);

public:
	Receiver(int clientFd, Request &request);
	~Receiver();

	ssize_t getBytesRecvd() const;
	bool receive();
};

#endif
