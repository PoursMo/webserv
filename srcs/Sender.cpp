#include "Sender.hpp"
#include "Logger.hpp"

Sender::Sender(int clientFd, const std::string &content, int targetFd)
	: clientFd(clientFd),
	  targetFd(targetFd),
	  content(content),
	  bytesSent(0),
	  bytesRead(0),
	  contentSent(content.empty())
{

	logger.log() << "Sender: new sender with content size: " << content.size() << ", targetFd: " << targetFd << std::endl;
}

Sender::~Sender()
{
	if (targetFd != -1)
		close(targetFd);
}

ssize_t Sender::trySend(const char *buffer, size_t len)
{
	ssize_t bytesSent = send(this->clientFd, buffer, len, MSG_NOSIGNAL);
	if (bytesSent == -1)
		throw std::runtime_error("send: " + std::string(strerror(errno)));
	logger.log() << "Sender: bytesSent: " << bytesSent << std::endl;
	return bytesSent;
}

bool Sender::handleSend()
{
	if (this->targetFd == -1 && this->contentSent)
		return false;
	if (!this->contentSent)
	{
		logger.log() << "Sender: sending content" << std::endl;
		this->bytesSent += trySend(this->content.c_str() + this->bytesSent, this->content.size() - this->bytesSent);
		if ((size_t)this->bytesSent == this->content.size())
		{
			this->contentSent = true;
			this->bytesSent = 0;
			return this->targetFd != -1;
		}
	}
	else
	{
		logger.log() << "Sender: sending buffer" << std::endl;
		ssize_t rest = this->bytesRead - this->bytesSent;
		if (rest)
		{
			bytesSent += trySend(this->buffer + this->bytesSent, rest);
			return true;
		}
		logger.log() << "Sender: read before" << std::endl;
		this->bytesRead = read(targetFd, this->buffer, WS_SENDER_BUFFER_SIZE);
		logger.log() << "Sender: read after" << std::endl;
		if (this->bytesRead == 0)
			return false;
		if (this->bytesRead == -1)
			throw std::runtime_error("read: " + std::string(strerror(errno)));
		this->bytesSent = trySend(this->buffer, this->bytesRead);
	}
	return true;
}
