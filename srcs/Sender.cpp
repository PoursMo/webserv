#include "Sender.hpp"

Sender::Sender(int clientFd, const std::string &content, int resourceFd)
	: clientFd(clientFd),
	  resourceFd(resourceFd),
	  content(content),
	  bytesSent(0),
	  bytesRead(0),
	  contentSent(content.empty())
{
	std::cout << "Sender: new sender with content size: " << content.size() << ", resourceFd: " << resourceFd << std::endl;
}

static ssize_t trySend(int fd, const char *buffer, size_t len)
{
	ssize_t bytesSent = send(fd, buffer, len, 0);
	if (bytesSent == -1)
		throw std::runtime_error("send: " + std::string(strerror(errno)));
	std::cout << "> bytesSent: " << bytesSent << std::endl;
	return bytesSent;
}

bool Sender::handleSend()
{
	std::cout << "> Call Sender::handleSend() " << std::endl;
	if (this->resourceFd == -1 && this->contentSent)
		return false;
	if (!this->contentSent)
	{
		bytesSent += trySend(clientFd, content.c_str() + bytesSent, content.size() - bytesSent);
		if ((size_t)bytesSent == content.size())
		{
			this->contentSent = true;
			bytesSent = 0;
			return this->resourceFd != -1;
		}
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

// bool Sender::handleSend()
// {
// 	if (!this->contentSent && this->resourceFd == -1)
// 		return false;
// 	if (!this->contentSent)
// 	{
// 		bytesSent += trySend(clientFd, content.c_str() + bytesSent, content.size() - bytesSent);
// 		if ((size_t)bytesSent == content.size())
// 		{
// 			this->contentSent = true;
//			bytesSent = 0;
// 			return this->resourceFd != -1;
// 		}
// 		return true;
// 	}

// 	ssize_t rest = bytesRead - bytesSent;
// 	if (rest)
// 	{
// 		bytesSent += trySend(clientFd, buffer + bytesSent, rest);
// 		return true;
// 	}
// 	bytesRead = read(resourceFd, buffer, WS_SENDER_BUFFER_SIZE);
// 	if (bytesRead == 0)
// 		return false;
// 	if (bytesRead == -1)
// 		throw std::runtime_error("read: " + std::string(strerror(errno)));
// 	bytesSent = trySend(clientFd, buffer, bytesRead);
// 	return true;
// }
