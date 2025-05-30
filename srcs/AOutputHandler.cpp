#include "AOutputHandler.hpp"
#include "Logger.hpp"
#include "Poller.hpp"

AOutputHandler::AOutputHandler()
	:outputFd(-1),
	 bytesOutputCount(0),
	 isOutputRegularFile(false)
{
}

AOutputHandler::~AOutputHandler()
{
	logger.log() << "AOutputHandler destructor" << std::endl;
	this->delOutputFd();
}

void AOutputHandler::handleOutput()
{
	ssize_t bytesOutput;
    if (!this->stringContent.empty())
	{
        logger.log() << "AOutputHandler: sending string content" << std::endl;
		const char *buffer_pos = this->stringContent.c_str() + this->bytesOutputCount;
		size_t len = this->stringContent.size() - this->bytesOutputCount;
		bytesOutput = handleOutputSysCall(buffer_pos, len);
		this->bytesOutputCount += bytesOutput;
		if ((size_t)bytesOutputCount == this->stringContent.size())
            this->stringContent.clear();
	}
	else if (!this->buffers.empty())
	{
        logger.log() << "AOutputHandler: sending buffer" << std::endl;
		Buffer *buffer = this->buffers.front();
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
		logger.log() << "IS END OUTPUT" << std::endl;
		this->delOutputFd();
		this->onOutputEnd();
	}
}

void AOutputHandler::delOutputFd()
{
	if (!this->isOutputRegularFile)
		this->delFd(this->outputFd);
}

void AOutputHandler::setOutputFd(int fd)
{
	this->outputFd = fd;
	logger.log() << "setOutputFd: " << fd << std::endl;
	logger.log() << "isOutputRegularFile: " << this->isOutputRegularFile << std::endl;
	if (!this->isOutputRegularFile)
	{
		this->poller.add(fd, EPOLLOUT);
		this->poller.ioHandlers[fd] = this;
	}
}
