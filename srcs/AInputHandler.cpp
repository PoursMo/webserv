#include "AInputHandler.hpp"
#include "Logger.hpp"
#include "AIOHandler.hpp"

// ********************************************************************
// AInputHandler class
// ********************************************************************

AInputHandler::AInputHandler()
	: isReadingHeader(true),
	bytesInput(0),
	bodyBytesCount(0),
	headerBufferCount(0)
{
}

// ********************************************************************
// AInputHandler public
// ********************************************************************

void AInputHandler::handleInput()
{
	if (isReadingHeader)
	{
		logger.log() << "AInputHandler: inputting header" << std::endl;
		if (fillBuffer(HEADER))
			flushHeaderBuffers();
	}
	else
	{
		logger.log() << "AInputHandler: inputting body" << std::endl;
		fillBuffer(BODY);
	}
	if (this->isInputEnd())
	{
		this->delInputFd();
		this->onInputEnd();
	}
}

ssize_t AInputHandler::getBytesInput() const
{
	return bytesInput;
}

// ********************************************************************
// AInputHandler private
// ********************************************************************

char *AInputHandler::ws_strchr(char *first, const char *const last, char c)
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

void AInputHandler::sendHeaderLineToParsing(const Buffer *lbuffer, char *lf)
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
		logger.log() << "AInputHandler: stitched line of size " << line.size() << ": ";
		debugPrint(&line[0], &line[line.size() - 1]);
		isReadingHeader = this->parseLine(&line[0], &line[line.size() - 1]);
	}
	else
	{
		logger.log() << "AInputHandler: non-stitched line of size " << lf - lbuffer->pos + 1 << ": ";
		debugPrint(lbuffer->pos, lf);
		isReadingHeader = this->parseLine(lbuffer->pos, lf);
	}
}

void AInputHandler::addBodyBytes(size_t bytes)
{
	bodyBytesCount += bytes;
	this->onUpdateBodyBytes();
}

void AInputHandler::flushHeaderBuffers()
{
	Buffer *const lbuffer = buffers.back();
	char *lf;
	while ((lf = ws_strchr(lbuffer->pos, lbuffer->last, '\n')))
	{
		sendHeaderLineToParsing(lbuffer, lf);
		lbuffer->pos = lf + 1;
		if (!isReadingHeader && lbuffer->last != lf)
		{
			this->addBodyBytes(lbuffer->last - lbuffer->pos + 1);
			logger.log() << "AInputHandler: copying remains of header buffer in body buffer " << std::endl;
			Buffer *bodyBuffer = createBuffer(BODY);
			std::memcpy(bodyBuffer->first, lbuffer->pos, bodyBytesCount);
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

AIOHandler::Buffer *AInputHandler::createBuffer(BufferType type)
{
	Buffer *buffer = new Buffer;
	switch (type)
	{
	case HEADER:
		this->onHeaderBufferCreation();
		buffer->first = new char[WS_HEADER_BUFFER_SIZE];
		buffer->capacity = WS_HEADER_BUFFER_SIZE;
		headerBufferCount++;
		break;
	case BODY:
		buffer->first = new char[WS_BODY_BUFFER_SIZE];
		buffer->capacity = WS_BODY_BUFFER_SIZE;
		break;
	default:
		break;
	}
	logger.log() << "AInputHandler: created new buffer of size " << buffer->capacity << std::endl;
	buffer->pos = buffer->first;
	buffers.push_back(buffer);
	return buffer;
}

bool AInputHandler::fillBuffer(BufferType type)
{
	Buffer *buffer = buffers.back();
	if (buffers.empty() || (size_t)(buffer->last - buffer->first) + 1 == buffer->capacity)
	{
		logger.log() << "AInputHandler: filling new buffer" << std::endl;
		buffer = createBuffer(type);
		switch (type)
		{
		case HEADER:
			this->bytesInput = this->handleInputSysCall(buffer->first, WS_HEADER_BUFFER_SIZE);
			if (bytesInput == 0)
				return false;
			break;
		case BODY:
			this->bytesInput = this->handleInputSysCall(buffer->first, WS_BODY_BUFFER_SIZE);
			this->addBodyBytes(bytesInput);
			break;
		default:
			break;
		}
		buffer->last = buffer->first + bytesInput - 1;
	}
	else
	{
		logger.log() << "AInputHandler: filling existing buffer" << std::endl;
		char *buf = buffer->last + 1;
		size_t readSize = buffer->capacity - (buffer->last - buffer->first) - 1;
		this->bytesInput = this->handleInputSysCall(buf, readSize);
		if (bytesInput == 0)
			return false;
		buffer->last += bytesInput;
	}
	logger.log() << "AInputHandler: buffer: ";
	debugPrint(buffer->first, buffer->last);
	return true;
}

