#include "AIOHandler.hpp"
#include "Logger.hpp"
#include "Poller.hpp"
#include "http_error.hpp"

AIOHandler::AIOHandler(Poller &poller, Connection &connection) 
: poller(poller),
  connection(connection)
{
}

AIOHandler::~AIOHandler()
{
	logger.log() << "AIOHandler destructor" << std::endl;
	for (std::list<Buffer *>::iterator i = buffers.begin(); i != buffers.end(); i++)
	{
		delete[] (*i)->first;
		delete (*i);
	}
}

void AIOHandler::delFd(int fd)
{
	if (fd == -1)
		return ;
	this->poller.del(fd);
	this->poller.ioHandlers.erase(fd);
}

// ********************************************************************
// Utils
// ********************************************************************

bool AIOHandler::isEmptyline(char *lstart, char *lend)
{
	if (*lstart != '\r' && *lstart != '\n')
		return false;
	if (lstart == lend && *lstart == '\n')
		return true;
	lstart++;
	if (lstart == lend && *lstart == '\n')
		return true;
	return false;
}

void AIOHandler::parseHeaderLine(char *lstart, char *lend)
{
	std::string key;
	std::string value = "";

	while (lstart != lend && *lstart != '\r' && *lstart != ':' && *lstart != ' ' && *lstart != '\t')
	{
		key = key + *lstart;
		lstart++;
	}
	if (*lstart != ':')
		throw http_error("No ':' in header line", 400);
	lstart++;
	while (*lstart == ' ' || *lstart == '\t')
		lstart++;
	while (lstart != lend && *lstart != '\r')
	{
		if (!std::isprint(*lstart) && *lstart != '\t')
			throw http_error("Invalid character in header value", 400);
		value = value + *lstart;
		lstart++;
	}
	value.erase(value.find_last_not_of(" \t") + 1);
	if (*lstart != '\r' && lstart != lend)
		throw http_error("Header line not correctly ended", 400);
	if (*lstart == '\r' && *(lstart + 1) != '\n')
		throw http_error("Header line not correctly ended", 400);
	this->addHeader(key, value);
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

