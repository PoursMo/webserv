#include "AInputHandler.hpp"
#include "Logger.hpp"

// ********************************************************************
// Debug
// ********************************************************************

static void debug_print(const char *first, const char *const last)
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
// AInputHandler class
// ********************************************************************

AInputHandler::AInputHandler(int inputFd, void *parser)
	: inputFd(inputFd),
	parser(parser),
	readingHeader(true),
	bytesInput(0)
{}

AInputHandler::~AInputHandler()
{
	for (std::list<Buffer *>::iterator i = buffers.begin(); i != buffers.end(); i++;)
	{
		delete[] (*i)->first;
		delete (*i);
	}
}

ssize_t AInputHandler::getBytesInput() const
{
	return bytesInput;
}

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