#include "Receiver.hpp"
#include "http_error.hpp"
#include "Poller.hpp"
#include "Request.hpp"
#include "Logger.hpp"

// ********************************************************************
// Debug
// ********************************************************************

void debug_print(const char *first, const char *const last)
{
	while (1)
	{
		if (*first == '\n')
			logger.log() << "\\n";
		else if (*first == '\r')
			logger.log() << "\\r";
		else
			logger.log() << *first;
		if (first == last)
			break;
		first++;
	}
	logger.log() << std::endl;
}

// ********************************************************************
// Receiver class
// ********************************************************************

// Public
Receiver::Receiver(int clientFd, Request &request)
	: clientFd(clientFd),
	  request(request),
	  headerBufferCount(0),
	  readingHeader(true),
	  bodySize(0),
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
		logger.log() << "Receiver: recving header" << std::endl;
		if (fillBuffer(HEADER))
			flushHeaderBuffers();
		if (!readingHeader && (bodyBytesRecvd == bodySize || bodySize == 0))
			return false;
	}
	else
	{
		logger.log() << "Receiver: recving body" << std::endl;
		fillBuffer(BODY);
		if (bodyBytesRecvd == bodySize)
			return false;
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

void Receiver::sendHeaderLineToParsing(const Buffer *lbuffer, char *lf)
{
	if (lbuffer != buffers.front())
	{
		std::string line;
		for (std::list<Buffer *>::iterator j = buffers.begin(); *j != lbuffer;)
		{
			line.append((*j)->pos, (*j)->last + 1);
			delete[] (*j)->first;
			delete (*j);
			++j;
			buffers.pop_front();
		}
		line.append(lbuffer->pos, lf + 1);
		logger.log() << "Receiver: stitched line of size " << line.size() << ": ";
		debug_print(&line[0], &line[line.size() - 1]);
		readingHeader = request.parseRequestLine(&line[0], &line[line.size() - 1]);
		if (!readingHeader)
			bodySize = request.getBodySize();
	}
	else
	{
		logger.log() << "Receiver: non-stitched line of size " << lf - lbuffer->pos + 1 << ": ";
		debug_print(lbuffer->pos, lf);
		readingHeader = request.parseRequestLine(lbuffer->pos, lf);
		if (!readingHeader)
			bodySize = request.getBodySize();
	}
}

void Receiver::flushHeaderBuffers()
{
	Buffer *const lbuffer = buffers.back();
	char *lf;
	while ((lf = ws_strchr(lbuffer->pos, lbuffer->last, '\n')))
	{
		sendHeaderLineToParsing(lbuffer, lf);
		lbuffer->pos = lf + 1;
		if (!readingHeader && lbuffer->last != lf)
		{
			bodyBytesRecvd = lbuffer->last - lbuffer->pos + 1;
			if (bodyBytesRecvd > bodySize)
				throw http_error("bodyBytesRecvd > bodySize", 400);
			logger.log() << "Receiver: copying remains of header buffer in body buffer " << std::endl;
			Buffer *bodyBuffer = createBuffer(BODY);
			std::memcpy(bodyBuffer->first, lbuffer->pos, bodyBytesRecvd);
			delete[] lbuffer->first;
			delete lbuffer;
			buffers.pop_front();
			break;
		}
		else if (lf == lbuffer->last)
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

void Receiver::handleRecv(void *buf, size_t len)
{
	logger.log() << "Receiver: recving for " << len << " bytes" << std::endl;
	bytesRecvd = recv(clientFd, buf, len, 0);
	logger.log() << "Receiver: recved " << bytesRecvd << " bytes" << std::endl;
	if (bytesRecvd == -1)
		throw http_error("recv: " + std::string(strerror(errno)), 500);
}

Buffer *Receiver::createBuffer(BufferType type)
{
	Buffer *buffer = new Buffer;
	switch (type)
	{
	case HEADER:
		if (headerBufferCount >= 4)
			throw http_error("Header too large", 400);
		headerBufferCount++;
		buffer->first = new char[WS_CLIENT_HEADER_BUFFER_SIZE];
		buffer->capacity = WS_CLIENT_HEADER_BUFFER_SIZE;
		break;
	case BODY:
		buffer->first = new char[WS_CLIENT_BODY_BUFFER_SIZE];
		buffer->capacity = WS_CLIENT_BODY_BUFFER_SIZE;
		break;
	default:
		break;
	}
	logger.log() << "Receiver: created new buffer of size " << buffer->capacity;
	buffer->pos = buffer->first;
	buffers.push_back(buffer);
	return buffer;
}

bool Receiver::fillBuffer(BufferType type)
{
	Buffer *buffer = buffers.back();
	if (buffers.empty() || (size_t)(buffer->last - buffer->first) + 1 == buffer->capacity)
	{
		buffer = createBuffer(type);
		switch (type)
		{
		case HEADER:
			handleRecv(buffer->first, WS_CLIENT_HEADER_BUFFER_SIZE);
			break;
		case BODY:
			handleRecv(buffer->first, WS_CLIENT_BODY_BUFFER_SIZE);
			bodyBytesRecvd += bytesRecvd;
			if (bodyBytesRecvd > bodySize)
				throw http_error("bodyBytesRecvd > bodySize", 400);
			break;
		default:
			break;
		}
		if (bytesRecvd == 0)
			return false;
		buffer->last = buffer->first + bytesRecvd - 1;
	}
	else
	{
		logger.log() << "Receiver: adding to existing buffer" << std::endl;
		char *buf = buffer->last + 1;
		size_t readSize = buffer->capacity - (buffer->last - buffer->first) - 1;
		handleRecv(buf, readSize);
		if (bytesRecvd == 0)
			return false;
		buffer->last += bytesRecvd;
	}
	logger.log() << "Receiver: buffer: ";
	debug_print(buffer->first, buffer->last);
	return true;
}

ssize_t Receiver::getBytesRecvd() const
{
	return bytesRecvd;
}