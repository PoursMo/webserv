#include "Sender.hpp"
#include "Logger.hpp"

Sender::Sender(int clientFd, const std::string &content, int targetFd)
	: clientFd(clientFd),
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

// ssize_t Request::handleOutputSysCall(const char *buffer, size_t len)
// {
// 	ssize_t bytesSent = send(this->clientFd, buffer, len, MSG_NOSIGNAL);
// 	if (bytesSent == -1)
// 		throw std::runtime_error("send: " + std::string(strerror(errno)));
// 	logger.log() << "Sender: bytesSent: " << bytesSent << std::endl;
// 	return bytesSent;
// }
