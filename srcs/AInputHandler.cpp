#include "AInputHandler.hpp"
#include "Logger.hpp"
#include "AIOHandler.hpp"
#include "Poller.hpp"
#include "utils.hpp"
#include "Connection.hpp"

// ********************************************************************
// AInputHandler class
// ********************************************************************

AInputHandler::AInputHandler()
	: isReadingHeader(true),
	bytesInput(-1),
	bodyBytesCount(0),
	headerBufferCount(0)
{
}

AInputHandler::~AInputHandler()
{
	this->unsubscribeInputFd();
}

// ********************************************************************
// AInputHandler public
// ********************************************************************

void AInputHandler::handleInput()
{
	if (this->isReadingHeader)
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
		this->connection.print("InputHandler input end");
		this->onInputEnd();
	}
}

void AInputHandler::unsubscribeInputFd()
{
	this->unsubscribeFd(this->inputFd);
}

int AInputHandler::getInputFd()
{
	return this->inputFd;
}

void AInputHandler::subscribeInputFd(int fd)
{
	this->inputFd = fd;
	if (this->inputFd == -1)
		return;
	this->poller.add(fd, POLLIN);
	this->poller.ioHandlers[fd] = this;
}

// ********************************************************************
// AInputHandler private
// ********************************************************************

char* AInputHandler::ws_strchr(char* first, const char* const last, char c)
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

void AInputHandler::sendHeaderLineToParsing(const Buffer* buffer, char* lf)
{
	if (buffer != buffers.front())
	{
		std::string line;
		for (std::list<Buffer*>::iterator j = buffers.begin(); *j != buffer;)
		{
			line.append((*j)->inputPos, (*j)->last + 1);
			delete[](*j)->first;
			delete (*j);
			++j;
			buffers.pop_front();
		}
		line.append(buffer->inputPos, lf + 1);
		// logger.log() << "AInputHandler: stitched line of size " << line.size() << ": ";
		printBuffer(&line[0], &line[line.size() - 1]);
		this->isReadingHeader = this->parseLine(&line[0], &line[line.size() - 1]);
	}
	else
	{
		// logger.log() << "AInputHandler: non-stitched line of size " << lf - buffer->inputPos + 1 << ": ";
		printBuffer(buffer->inputPos, lf);
		this->isReadingHeader = this->parseLine(buffer->inputPos, lf);
	}
}

void AInputHandler::addBodyBytes(size_t bytes)
{
	logger.log() << "add " << bytes << " to " << this->bodyBytesCount;
	this->bodyBytesCount += bytes;
	logger.log() << " = " << this->bodyBytesCount << std::endl;
	this->onUpdateBodyBytes();
}

void AInputHandler::flushHeaderBuffers()
{
	Buffer* const backBuffer = buffers.back();
	char* lf;
	while ((lf = ws_strchr(backBuffer->inputPos, backBuffer->last, '\n')))
	{
		sendHeaderLineToParsing(backBuffer, lf);
		backBuffer->inputPos = lf + 1;
		if (!this->isReadingHeader)
		{
			if (lf != backBuffer->last)
			{
				this->addBodyBytes(backBuffer->last - backBuffer->inputPos + 1);
				logger.log() << "AInputHandler: copying remains of header buffer in body buffer " << std::endl;
				Buffer* bodyBuffer = createBuffer(BODY);
				std::memcpy(bodyBuffer->first, backBuffer->inputPos, bodyBytesCount);
				bodyBuffer->last = bodyBuffer->first + bodyBytesCount - 1;
			}
			delete[] backBuffer->first;
			delete backBuffer;
			buffers.pop_front();
			break;
		}
		else if (lf == backBuffer->last)
		{
			if (backBuffer->last == backBuffer->first + backBuffer->capacity - 1)
			{
				delete[] backBuffer->first;
				delete backBuffer;
				buffers.pop_front();
			}
			break;
		}
	}
}

AIOHandler::Buffer* AInputHandler::createBuffer(BufferType type)
{
	Buffer* buffer = new Buffer;
	switch (type)
	{
	case HEADER:
		this->onHeaderBufferCreation();
		buffer->first = new char[WS_HEADER_BUFFER_SIZE];
		buffer->capacity = WS_HEADER_BUFFER_SIZE;
		break;
	case BODY:
		buffer->first = new char[WS_BODY_BUFFER_SIZE];
		buffer->capacity = WS_BODY_BUFFER_SIZE;
		break;
	default:
		break;
	}
	// logger.log() << "AInputHandler: created new buffer of size " << buffer->capacity << std::endl;
	buffer->inputPos = buffer->first;
	buffer->outputPos = buffer->first;
	buffer->last = buffer->first;
	buffers.push_back(buffer);
	return buffer;
}

bool AInputHandler::fillBuffer(BufferType type)
{
	Buffer* buffer = buffers.back();

	if (buffers.empty() || (size_t)(buffer->last - buffer->first) + 1 == buffer->capacity)
	{
		// logger.log() << "AInputHandler: filling new buffer" << std::endl;
		buffer = createBuffer(type);
		switch (type)
		{
		case HEADER:
			this->bytesInput = this->handleInputSysCall(buffer->first, WS_HEADER_BUFFER_SIZE);
			if (this->bytesInput)
				headerBufferCount++;
			break;
		case BODY:
			this->bytesInput = this->handleInputSysCall(buffer->first, WS_BODY_BUFFER_SIZE);
			this->addBodyBytes(bytesInput);
			break;
		default:
			break;
		}
		if (this->bytesInput == 0)
		{
			delete[] buffer->first;
			delete buffer;
			this->buffers.pop_back();
			return false;
		}
		buffer->last = buffer->first + this->bytesInput - 1;
	}
	else
	{
		// logger.log() << "AInputHandler: filling existing buffer" << std::endl;
		char* buf = buffer->last + 1;
		size_t readSize = buffer->capacity - (buffer->last - buffer->first) - 1;
		this->bytesInput = this->handleInputSysCall(buf, readSize);
		if (type == BODY)
			this->addBodyBytes(bytesInput);
		if (bytesInput == 0)
			return false;
		buffer->last += bytesInput;
	}
	// if (buffer)
	// {
	// 	logger.log() << "AInputHandler: buffer: ";
	// 	printBuffer(buffer->first, buffer->last);
	// }
	this->connection.updateTime();
	return true;
}

