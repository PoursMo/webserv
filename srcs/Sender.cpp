#include "Sender.hpp"

#include <string>
#include <sys/socket.h>
#include <iostream>
#include <unistd.h>
#include <cstring>
#include <cerrno>

Sender::Sender(int clientFd, std::string str)
	: clientFd(clientFd),
	  resourceFd(-1),
	  str(str),
	  bytesSent(0),
	  bytesRead(0)
{
}

Sender::Sender(int clientFd, int resourceFd)
	: clientFd(clientFd),
	  resourceFd(resourceFd)
{
}

static ssize_t trySend(int fd, const char *buffer, size_t len)
{
	ssize_t bytesRead = send(fd, buffer, len, 0);
	if (bytesRead == -1)
		throw std::runtime_error("send: " + std::string(strerror(errno)));
	return bytesRead;
}

bool Sender::handleSend()
{
	if (resourceFd == -1)
	{
		bytesSent += trySend(clientFd, str.c_str() + bytesSent, str.size() - bytesSent);
		if ((size_t)bytesSent == str.size())
			return false;
	}
	else
	{
		ssize_t rest = bytesRead - bytesSent;
		if (rest)
		{
			bytesSent += trySend(clientFd, buffer + bytesSent, rest);
			return true;
		}
		bytesRead = read(resourceFd, buffer, WS_SENDER_BUFFER_SIZE);
		if (bytesRead == 0)
			return false;
		if (bytesRead == -1)
			throw std::runtime_error("read: " + std::string(strerror(errno)));
		bytesSent = trySend(clientFd, buffer, bytesRead);
	}
	return true;
}