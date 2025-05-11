#ifndef RECEIVER_HPP
#define RECEIVER_HPP

#include "http_error.hpp"

#include <cstring>
#include <cerrno>
#include <algorithm>

#include <list>
#include <sys/socket.h>

#include <iostream>

#define WS_CLIENT_HEADER_BUFFER_SIZE 12 // 4 max
// #define WS_CLIENT_HEADER_BUFFER_SIZE 8192 // 4 max
#define WS_CLIENT_BODY_BUFFER_SIZE 16384

struct Buffer
{
	char *first;
	char *last;
	char *pos;
	size_t capacity;
};

void debug_print(const char *first, const char *const last)
{
	while (1)
	{
		std::cout << *first;
		if (first == last)
			break;
		first++;
	}
	std::cout << std::endl;
}

class Receiver
{
private:
	int fd; // keep elsewhere ?
	// request object reference
	std::list<Buffer *> buffers;
	size_t headerBufferCount;
	bool readingHeader;
	size_t bodyBytesRecvd;

	static char *ws_strchr(char *first, const char *const last, char c)
	{
		while (1)
		{
			if (*first == c)
				return first;
			if (first == last)
				break;
			first++;
		}
		return 0;
	}

	enum BufferType
	{
		HEADER,
		BODY
	};

	void flushHeaderBuffers()
	{
		Buffer *const lbuffer = buffers.back();
		char *lf;
		while ((lf = ws_strchr(lbuffer->pos, lbuffer->last, '\n')))
		{
			if (lbuffer != buffers.front())
			{
				std::string line;
				for (std::list<Buffer *>::iterator j = buffers.begin(); *j != lbuffer;)
				{
					line.append((*j)->pos, (*j)->last + 1);
					std::list<Buffer *>::iterator tmp = j;
					++j;
					delete[] (*tmp)->first;
					delete (*tmp);
					buffers.erase(tmp);
				}
				line.append(lbuffer->pos, lf + 1);
				std::cout << "stitched line of size " << line.size() << ": ";
				debug_print(&line[0], &line[line.size() - 1]);
				// request.line(&line[0], &line[line.size()])
				if (!std::strcmp(line.c_str(), "\r\n"))
				{
					readingHeader = false;
					return;
				}
			}
			else
			{
				std::cout << "made line of size " << lf - lbuffer->pos + 1 << ": ";
				debug_print(lbuffer->pos, lf);
				// request.line(pos, nl)
				if (!std::strcmp(lbuffer->pos, "\r\n"))
				{
					readingHeader = false;
					return;
				}
			}
			if (lf != lbuffer->last) // line feed was not the last character of buffer
				lbuffer->pos = lf + 1;
			else if (lbuffer->last == lbuffer->first + lbuffer->capacity - 1) // line feed is last character of buffer and is exactly the last character the buffer can hold
			{
				delete[] lbuffer->first;
				delete lbuffer;
				buffers.clear();
				break;
			}
			else
				break;
		}
	}

	Buffer *createBuffer(BufferType type)
	{
		std::cout << "creating new buffer" << std::endl;
		Buffer *buffer = new Buffer;
		switch (type)
		{
		case HEADER:
			headerBufferCount++;
			buffer->first = new char[WS_CLIENT_HEADER_BUFFER_SIZE];
			buffer->capacity = WS_CLIENT_HEADER_BUFFER_SIZE;
			break;
		case BODY:
			buffer->first = new char[WS_CLIENT_BODY_BUFFER_SIZE]; // std::min(request.bodysize - bodyBytesRecvd, WS_CLIENT_BODY_BUFFER_SIZE)
			buffer->capacity = WS_CLIENT_BODY_BUFFER_SIZE;		  // std::min(request.bodysize - bodyBytesRecvd, WS_CLIENT_BODY_BUFFER_SIZE)
			break;
		default:
			break;
		}
		buffer->pos = buffer->first;
		buffers.push_back(buffer);
		return buffer;
	}

	ssize_t handleRecv(void *buf, size_t len)
	{
		std::cout << "recving " << len << " bytes" << std::endl;
		ssize_t bytes_received = recv(fd, buf, len, 0);
		if (bytes_received == -1)
			throw http_error(std::strerror(errno), 500);
		if (bytes_received == 0)
		{
			std::cout << "Client with fd: " << fd << " disconnected" << std::endl;
			// handle disconnection
		}
		return bytes_received;
	}

	void fillBuffer(BufferType type)
	{
		Buffer *buffer = buffers.back();
		if (buffers.empty() || (size_t)(buffer->last - buffer->first) + 1 == buffer->capacity)
		{
			buffer = createBuffer(type);
			ssize_t bytes_received;
			switch (type)
			{
			case HEADER:
				bytes_received = handleRecv(buffer->first, WS_CLIENT_HEADER_BUFFER_SIZE);
				break;
			case BODY:
				bytes_received = handleRecv(buffer->first, WS_CLIENT_BODY_BUFFER_SIZE); // std::min(request.bodysize - bodyBytesRecvd, WS_CLIENT_BODY_BUFFER_SIZE)
				break;
			default:
				break;
			}
			buffer->last = buffer->first + bytes_received - 1;
		}
		else
		{
			std::cout << "adding to existing buffer" << std::endl;
			char *buf = buffer->last + 1;
			size_t readSize = buffer->capacity - (buffer->last - buffer->first) - 1;
			ssize_t bytes_received = handleRecv(buf, readSize);
			buffer->last += bytes_received - 1;
		}
		std::cout << "buffer: ";
		debug_print(buffer->first, buffer->last);
	}

public:
	Receiver()
	{
	}

	Receiver(int fd)
		: fd(fd),
		  headerBufferCount(0),
		  readingHeader(true),
		  bodyBytesRecvd(0)
	{
	}

	~Receiver()
	{
		for (std::list<Buffer *>::iterator i = buffers.begin(); i != buffers.end();)
		{
			std::list<Buffer *>::iterator tmp = i;
			++i;
			delete[] (*tmp)->first;
			delete (*tmp);
			buffers.erase(tmp);
		}
	}

	bool receive()
	{
		if (readingHeader)
		{
			if (headerBufferCount >= 4)
				throw http_error(414);
			fillBuffer(HEADER);
			flushHeaderBuffers();
			if (!readingHeader) // && !request.body
				return false;
		}
		else
		{
			fillBuffer(BODY);
			// if (bodyBytesRecvd == request.bodysize)
			// return false;
		}
		return true;
	}
};

#endif