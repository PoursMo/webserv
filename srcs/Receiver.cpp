#include "Receiver.hpp"
#include "http_error.hpp"

#include <cstring>
#include <cerrno>
#include <algorithm>

#include <list>
#include <sys/socket.h>

#include <iostream>

#define WS_CLIENT_HEADER_BUFFER_SIZE 8192 // 4 max
#define WS_CLIENT_BODY_BUFFER_SIZE 16384

// ********************************************************************
// Debug
// ********************************************************************

void debug_print(const char *first, const char *const last)
{
	while (1)
	{
		if (*first == '\n')
			std::cout << "\\n";
		else if (*first == '\r')
			std::cout << "\\r";
		else
			std::cout << *first;
		if (first == last)
			break;
		first++;
	}
	std::cout << std::endl;
}

// ********************************************************************
// Receiver class
// ********************************************************************

// Public
Receiver::Receiver()
{
}

Receiver::Receiver(int fd)
	: fd(fd),
	  headerBufferCount(0),
	  readingHeader(true),
	  bodyBytesRecvd(0)
{
}

Receiver::~Receiver()
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

bool Receiver::receive()
{
	if (readingHeader)
	{
		std::cout << "Receiver: recving header" << std::endl; // debug
		if (headerBufferCount >= 4)
			throw http_error(414);
		fillBuffer(HEADER);
		flushHeaderBuffers();
		if (!readingHeader) // && !request.body
			return false;
	}
	else
	{
		std::cout << "Receiver: recving body" << std::endl; // debug
		fillBuffer(BODY);
		// if (bodyBytesRecvd == request.bodysize)
		// return false;
	}
	return true;
}

// Private
char *Receiver::ws_strchr(char *first, const char *const last, char c)
{
	while (1)
	{
		if (*first == c)
			return first;
		if (first == last)
			break;
		++first;
	}
	return 0;
}

void Receiver::sendLineToParsing(const Buffer *lbuffer, char *lf)
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
		std::cout << "Receiver: stitched line of size " << line.size() << ": "; // debug
		debug_print(&line[0], &line[line.size() - 1]);							// debug
		this->request.parseRequestLine(&line[0], &line[line.size()]);
		if (!std::strncmp(line.c_str(), "\r\n", 2))
			readingHeader = false;
	}
	else
	{
		std::cout << "Receiver: non-stitched line of size " << lf - lbuffer->pos + 1 << ": "; // debug
		debug_print(lbuffer->pos, lf);														  // debug
		this->request.parseRequestLine(lbuffer->pos, lf);
		if (!std::strncmp(lbuffer->pos, "\r\n", 2))
			readingHeader = false;
	}
}

void Receiver::flushHeaderBuffers()
{
	Buffer *const lbuffer = buffers.back();
	char *lf;
	while ((lf = ws_strchr(lbuffer->pos, lbuffer->last, '\n')))
	{
		sendLineToParsing(lbuffer, lf);
		lbuffer->pos = lf + 1;
		if (lf == lbuffer->last)
		{
			if (lbuffer->last == lbuffer->first + lbuffer->capacity - 1)
			{
				delete[] lbuffer->first;
				delete lbuffer;
				buffers.clear();
			}
			break;
		}
	}
}

Buffer *Receiver::createBuffer(BufferType type)
{
	std::cout << "Receiver: creating new buffer of size "; // debug
	Buffer *buffer = new Buffer;
	switch (type)
	{
	case HEADER:
		headerBufferCount++;
		buffer->first = new char[WS_CLIENT_HEADER_BUFFER_SIZE];
		buffer->capacity = WS_CLIENT_HEADER_BUFFER_SIZE;
		std::cout << WS_CLIENT_HEADER_BUFFER_SIZE << std::endl; // debug
		break;
	case BODY:
		buffer->first = new char[WS_CLIENT_BODY_BUFFER_SIZE]; // std::min(request.bodysize - bodyBytesRecvd, WS_CLIENT_BODY_BUFFER_SIZE)
		buffer->capacity = WS_CLIENT_BODY_BUFFER_SIZE;		  // std::min(request.bodysize - bodyBytesRecvd, WS_CLIENT_BODY_BUFFER_SIZE)
		std::cout << WS_CLIENT_BODY_BUFFER_SIZE << std::endl; // debug
		break;
	default:
		break;
	}
	buffer->pos = buffer->first;
	buffers.push_back(buffer);
	return buffer;
}

ssize_t Receiver::handleRecv(void *buf, size_t len)
{
	std::cout << "Receiver: recving for " << len << " bytes" << std::endl; // debug
	ssize_t bytes_received = recv(fd, buf, len, 0);
	std::cout << "Receiver: recved " << bytes_received << " bytes" << std::endl;
	if (bytes_received == -1)
		throw http_error(std::strerror(errno), 500);
	if (bytes_received == 0)
	{
		std::cout << "Client with fd: " << fd << " disconnected" << std::endl; // debug
																			   // handle disconnection
	}
	return bytes_received;
}

void Receiver::fillBuffer(BufferType type)
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
		std::cout << "Receiver: adding to existing buffer" << std::endl; // debug
		char *buf = buffer->last + 1;
		size_t readSize = buffer->capacity - (buffer->last - buffer->first) - 1;
		ssize_t bytes_received = handleRecv(buf, readSize);
		buffer->last += bytes_received;
	}
	std::cout << "Receiver: buffer: ";		  // debug
	debug_print(buffer->first, buffer->last); // debug
}
