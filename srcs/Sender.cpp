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

Sender::~Sender()
{
	if (resourceFd != -1)
		close(resourceFd);
}

ssize_t Sender::trySend(const char *buffer, size_t len)
{
	ssize_t bytesSent = send(this->clientFd, buffer, len, MSG_NOSIGNAL);
	if (bytesSent == -1)
		throw std::runtime_error("send: " + std::string(strerror(errno)));
	std::cout << "Sender: bytesSent: " << bytesSent << std::endl;
	return bytesSent;
}

bool Sender::handleSend()
{
	if (this->resourceFd == -1 && this->contentSent)
		return false;
	if (!this->contentSent)
	{
		std::cout << "Sender: sending content" << std::endl;
		this->bytesSent += trySend(this->content.c_str() + this->bytesSent, this->content.size() - this->bytesSent);
		if ((size_t)this->bytesSent == this->content.size())
		{
			this->contentSent = true;
			this->bytesSent = 0;
			return this->resourceFd != -1;
		}
	}
	else
	{
		std::cout << "Sender: sending buffer" << std::endl;
		ssize_t rest = this->bytesRead - this->bytesSent;
		if (rest)
		{
			bytesSent += trySend(this->buffer + this->bytesSent, rest);
			return true;
		}
		this->bytesRead = read(resourceFd, this->buffer, WS_SENDER_BUFFER_SIZE);
		if (this->bytesRead == 0)
			return false;
		if (this->bytesRead == -1)
			throw std::runtime_error("read: " + std::string(strerror(errno)));
		this->bytesSent = trySend(this->buffer, this->bytesRead);
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
// 			bytesSent = 0;
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
