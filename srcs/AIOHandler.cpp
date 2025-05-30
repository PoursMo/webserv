#include "AIOHandler.hpp"
#include "Logger.hpp"
#include "Poller.hpp"

AIOHandler::AIOHandler(Poller &poller, Connection &connection) 
: inputFd(-1),
  outputFd(-1),
  poller(poller),
  connection(connection)
{
}

AIOHandler::~AIOHandler()
{
	for (std::list<Buffer *>::iterator i = buffers.begin(); i != buffers.end(); i++)
	{
		delete[] (*i)->first;
		delete (*i);
	}
	this->delInputFd();
	this->delOutputFd();
}

void AIOHandler::delFd(int fd)
{
	if (fd == -1)
		return ;
	this->poller.del(fd);
	this->poller.ioHandlers.erase(fd);
}

void AIOHandler::delInputFd()
{
	this->delFd(this->inputFd);
}

void AIOHandler::delOutputFd()
{
	this->delFd(this->outputFd);
}

void AIOHandler::setInputFd(int fd)
{
	this->inputFd = fd;
	this->poller.add(fd, EPOLLIN);
	this->poller.ioHandlers[fd] = this;
}

void AIOHandler::setOutputFd(int fd)
{
	this->outputFd = fd;
	this->poller.add(fd, EPOLLOUT);
	this->poller.ioHandlers[fd] = this;
}

// ********************************************************************
// Getters
// ********************************************************************

int AIOHandler::getInputFd() const
{
	return this->inputFd;
}

int AIOHandler::getOutputFd() const
{
	return this->outputFd;
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

