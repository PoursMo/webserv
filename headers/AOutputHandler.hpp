#ifndef AOUTPUTHANDLER_HPP
#define AOUTPUTHANDLER_HPP

#include "AIOHandler.hpp"
#include "webserv.hpp"

class AOutputHandler : virtual public AIOHandler
{
protected:
	ssize_t bytesOutputCount;
	std::string stringContent;

	AOutputHandler();
	virtual ~AOutputHandler();
public:
	void handleOutput();
	void subscribeOutputFd(int fd);
	void unsubscribeOutputFd();
};

#endif
