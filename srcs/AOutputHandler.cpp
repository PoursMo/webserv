#include "AOutputHandler.hpp"
#include "Logger.hpp"


AOutputHandler::AOutputHandler()
	: bytesOutputCount(0)
{
}

bool AOutputHandler::handleOutput()
{
	ssize_t bytesOutput;
    if (!this->stringContent.empty())
	{
        logger.log() << "Sender: sending content" << std::endl;
		const char *buffer_pos = this->stringContent.c_str() + this->bytesOutputCount;
		size_t len = this->stringContent.size() - this->bytesOutputCount;
		bytesOutput = handleOutputSysCall(buffer_pos, len);
		this->bytesOutputCount += bytesOutput;
		if ((size_t)bytesOutputCount == this->stringContent.size())
            this->stringContent = "";
	}
	else
	{
        logger.log() << "Sender: sending buffer" << std::endl;
		Buffer *buffer = this->buffers.front();
		size_t len = buffer->last - buffer->pos + 1;
		bytesOutput = handleOutputSysCall(buffer->pos, len);
		if (bytesOutput != len)
		{
			buffer->pos += bytesOutput;
		}
		else
		{
			delete[] buffer->first;
			delete buffer;
			buffers.pop_front();
		}
	}
	return !this->isOutputEnd();
}