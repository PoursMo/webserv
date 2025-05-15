#include "Sender.hpp"

#include <string>
#include <sys/socket.h>
#include <iostream>

Sender::Sender(int clientFd, std::string str)
	: clientFd(clientFd),
	  resourceFd(-1),
	  str(str)
{
}

Sender::Sender(int clientFd, int resourceFd)
	: clientFd(clientFd),
	  resourceFd(resourceFd)
{
}

bool Sender::handleSend()
{
	if (resourceFd == -1)
	{
		ssize_t bytesSent = send(clientFd, str.c_str(), str.size(), 0);
		if (bytesSent < (ssize_t)str.size())
			std::cout << "bytesSent < str.size()" << std::endl;
	}
	else
	{
		// fd
	}
	return false;
}