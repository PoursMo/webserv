#include "AIOHandler.hpp"
#include "Logger.hpp"
#include "Poller.hpp"

AIOHandler::AIOHandler(Poller &poller) 
: inputFd(-1),
  outputFd(-1),
  poller(poller)
{
}

AIOHandler::~AIOHandler()
{
	for (std::list<Buffer *>::iterator i = buffers.begin(); i != buffers.end(); i++)
	{
		delete[] (*i)->first;
		delete (*i);
	}
	//TODO: close()?
	if (this->inputFd != -1)
		this->poller.del(this->inputFd);
	if (this->outputFd != -1)
		this->poller.del(this->outputFd);
}

void AIOHandler::setInputFd(int fd)
{
	this->inputFd = fd;
	this->poller.add(fd, EPOLLIN);
}

void AIOHandler::setOutputFd(int fd)
{
	this->outputFd = fd;
	this->poller.add(fd, EPOLLOUT);
}

// ********************************************************************
// Debug
// ********************************************************************

void AIOHandler::debugPrint(const char *first, const char *const last)
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

