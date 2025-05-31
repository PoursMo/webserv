#include "AOutputHandler.hpp"
#include "Poller.hpp"
#include "Connection.hpp"
#include "Logger.hpp"
#include "utils.hpp"

AOutputHandler::AOutputHandler()
	: bytesOutputCount(0)
{
}

AOutputHandler::~AOutputHandler()
{
	this->unsubscribeOutputFd();
}

void AOutputHandler::handleOutput()
{
	ssize_t bytesOutput;
	if (!this->stringContent.empty())
	{
		logger.log() << "AOutputHandler: sending string content" << std::endl;
		const char* buffer_pos = this->stringContent.c_str() + this->bytesOutputCount;
		size_t len = this->stringContent.size() - this->bytesOutputCount;
		bytesOutput = handleOutputSysCall(buffer_pos, len);
		this->bytesOutputCount += bytesOutput;
		if ((size_t)bytesOutputCount == this->stringContent.size())
			this->stringContent.clear();
	}
	else if (!this->buffers.empty())
	{
		logger.log() << "AOutputHandler: sending buffer" << std::endl;
		Buffer* buffer = this->buffers.front();
		size_t len = buffer->last - buffer->pos + 1;
		bytesOutput = handleOutputSysCall(buffer->pos, len);
		if ((size_t)bytesOutput != len)
			buffer->pos += bytesOutput;
		else
		{
			delete[] buffer->first;
			delete buffer;
			buffers.pop_front();
		}
	}
	if (this->isOutputEnd())
	{
		this->connection.print("AOutputHandler output end");
		this->onOutputEnd();
	}
}

void AOutputHandler::unsubscribeOutputFd()
{
	this->unsubscribeFd(this->outputFd);
}

void AOutputHandler::subscribeOutputFd(int fd)
{
	this->outputFd = fd;
	if (this->outputFd == -1)
		return;
	this->poller.add(fd, POLLOUT);
	this->poller.ioHandlers[fd] = this;
}
