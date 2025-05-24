#ifndef SENDER_HPP
#define SENDER_HPP

#include "webserv.hpp"

class Sender
{
private:
	int clientFd;
	int targetFd;
	std::string content;
	char buffer[WS_SENDER_BUFFER_SIZE];
	ssize_t bytesSent;
	ssize_t bytesRead;
	bool contentSent;

	ssize_t trySend(const char *buffer, size_t len);

public:
	Sender(int clientFd, const std::string &content, int targetFd = -1);
	~Sender();

	bool handleSend();
};

#endif
