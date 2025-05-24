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
Receiver::Receiver(int fd, Request &request)
	: fd(fd),
	  request(request),
	  headerBufferCount(0),
	  readingHeader(true),
	  bodySize(0),
	  bodyBytesRecvd(0)
{
}

Receiver::~Receiver()
{
	for (std::list<Buffer *>::iterator i = headerBuffers.begin(); i != headerBuffers.end();)
	{
		std::list<Buffer *>::iterator tmp = i;
		++i;
		delete[] (*tmp)->first;
		delete (*tmp);
		headerBuffers.erase(tmp);
	}
}

bool Receiver::receive()
{
	if (readingHeader)
	{
		logger.log() << "Receiver: recving header" << std::endl; // debug
		if (fillHeaderBuffer())
			flushHeaderBuffers();
		if (!readingHeader && (bodyBytesRecvd == bodySize || bodySize == 0))
			return false;
	}
	else
	{
		logger.log() << "Receiver: recving body" << std::endl; // debug
		char buffer[WS_CLIENT_BODY_BUFFER_SIZE];
		handleRecv(buffer, WS_CLIENT_BODY_BUFFER_SIZE);
		bodyBytesRecvd += this->bytesRecvd;
		if (bodyBytesRecvd > bodySize)
			throw http_error("bodyBytesRecvd > bodySize", 400);
		logger.log() << "Receiver: writing body buffer in body fd " << request.getBodyFd() << std::endl;
		write(request.getBodyFd(), buffer, this->bytesRecvd);
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

void Receiver::sendLineToParsing(const Buffer *lbuffer, char *lf)
{
	if (lbuffer != headerBuffers.front())
	{
		std::string line;
		for (std::list<Buffer *>::iterator j = headerBuffers.begin(); *j != lbuffer;)
		{
			line.append((*j)->pos, (*j)->last + 1);
			std::list<Buffer *>::iterator tmp = j;
			++j;
			delete[] (*tmp)->first;
			delete (*tmp);
			headerBuffers.erase(tmp);
		}
		line.append(lbuffer->pos, lf + 1);
		logger.log() << "Receiver: stitched line of size " << line.size() << ": "; // debug
		debug_print(&line[0], &line[line.size() - 1]);							   // debug
		readingHeader = this->request.parseRequestLine(&line[0], &line[line.size() - 1]);
		if (!readingHeader)
			bodySize = request.getBodySize();
	}
	else
	{
		logger.log() << "Receiver: non-stitched line of size " << lf - lbuffer->pos + 1 << ": "; // debug
		debug_print(lbuffer->pos, lf);															 // debug
		readingHeader = this->request.parseRequestLine(lbuffer->pos, lf);
		if (!readingHeader)
			bodySize = request.getBodySize();
	}
}

void Receiver::flushHeaderBuffers()
{
	Buffer *const lbuffer = headerBuffers.back();
	char *lf;
	while ((lf = ws_strchr(lbuffer->pos, lbuffer->last, '\n')))
	{
		sendLineToParsing(lbuffer, lf);
		lbuffer->pos = lf + 1;
		if (!readingHeader && lbuffer->last != lf)
		{
			bodyBytesRecvd = lbuffer->last - lbuffer->pos + 1;
			if (bodyBytesRecvd > bodySize)
				throw http_error("bodyBytesRecvd > bodySize", 400);
			logger.log() << "Receiver: writing remains of header buffer in body fd " << request.getBodyFd() << std::endl;
			write(request.getBodyFd(), lbuffer->pos, bodyBytesRecvd);
			delete[] lbuffer->first;
			delete lbuffer;
			headerBuffers.clear();
			break;
		}
		else if (lf == lbuffer->last)
		{
			if (lbuffer->last == lbuffer->first + lbuffer->capacity - 1)
			{
				delete[] lbuffer->first;
				delete lbuffer;
				headerBuffers.clear();
			}
			break;
		}
	}
}

ssize_t Receiver::handleRecv(void *buf, size_t len)
{
	logger.log() << "Receiver: recving for " << len << " bytes" << std::endl; // debug
	this->bytesRecvd = recv(fd, buf, len, 0);
	logger.log() << "Receiver: recved " << this->bytesRecvd << " bytes" << std::endl;
	if (this->bytesRecvd == -1)
		throw http_error("recv: " + std::string(strerror(errno)), 500);
	return this->bytesRecvd;
}

Buffer *Receiver::createHeaderBuffer()
{
	if (headerBufferCount >= 4)
		throw http_error("Header too large", 400);
	logger.log() << "Receiver: creating new buffer of size "; // debug
	Buffer *buffer = new Buffer;
	headerBufferCount++;
	buffer->first = new char[WS_CLIENT_HEADER_BUFFER_SIZE];
	buffer->capacity = WS_CLIENT_HEADER_BUFFER_SIZE;
	logger.log() << WS_CLIENT_HEADER_BUFFER_SIZE << std::endl; // debug
	buffer->pos = buffer->first;
	headerBuffers.push_back(buffer);
	return buffer;
}

bool Receiver::fillHeaderBuffer()
{
	Buffer *buffer = headerBuffers.back();
	if (headerBuffers.empty() || (size_t)(buffer->last - buffer->first) + 1 == buffer->capacity)
	{
		buffer = createHeaderBuffer();
		handleRecv(buffer->first, WS_CLIENT_HEADER_BUFFER_SIZE);
		if (this->bytesRecvd == 0)
			return false;
		buffer->last = buffer->first + this->bytesRecvd - 1;
	}
	else
	{
		logger.log() << "Receiver: adding to existing buffer" << std::endl; // debug
		char *buf = buffer->last + 1;
		size_t readSize = buffer->capacity - (buffer->last - buffer->first) - 1;
		handleRecv(buf, readSize);
		if (this->bytesRecvd == 0)
			return false;
		buffer->last += this->bytesRecvd;
	}
	logger.log() << "Receiver: buffer: ";	  // debug
	debug_print(buffer->first, buffer->last); // debug
	return true;
}

ssize_t Receiver::getBytesRecvd() const
{
	return this->bytesRecvd;
}