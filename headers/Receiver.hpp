#ifndef RECEIVER_HPP
#define RECEIVER_HPP

#include "webserv.hpp"
#include "IInputHandler.hpp"

class Request;
class Poller;

class Receiver: public AInputHandler
{
	private:
		size_t headerBufferCount;
		size_t bodySize;
		size_t bodyBytesRecvd;

	public:
		Receiver(int clientFd, Request &request);
		~Receiver();
};

#endif
