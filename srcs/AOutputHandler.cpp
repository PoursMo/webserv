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
	this->connection.updateTime();
	ssize_t bytesOutput;
	if (!this->stringContent.empty())
	{
		logger.log() << "AOutputHandler: sending string content" << std::endl;
		const char* bufferPos = this->stringContent.c_str() + this->bytesOutputCount;
		size_t len = this->stringContent.size() - this->bytesOutputCount;
		bytesOutput = handleOutputSysCall(bufferPos, len);
		this->bytesOutputCount += bytesOutput;
		if ((size_t)bytesOutputCount == this->stringContent.size())
			this->stringContent.clear();
	}
	else if (!this->buffers.empty())
	{
		logger.log() << "AOutputHandler: sending buffer" << std::endl;
		Buffer* frontBuffer = this->buffers.front();
		size_t len = frontBuffer->last - frontBuffer->outputPos + 1;
		bytesOutput = handleOutputSysCall(frontBuffer->outputPos, len);
		frontBuffer->outputPos += bytesOutput;
		if (frontBuffer->outputPos - 1 == frontBuffer->last)
		{
			logger.log() << "AOutputHandler: deleting frontBuffer" << std::endl;
			delete[] frontBuffer->first;
			delete frontBuffer;
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
