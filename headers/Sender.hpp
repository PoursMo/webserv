#ifndef SENDER_HPP
#define SENDER_HPP

#include <string>
#include <sys/types.h>

#define WS_SENDER_BUFFER_SIZE 32768

class Sender
{
private:
	int clientFd;
	int resourceFd;
	std::string content;
	char buffer[WS_SENDER_BUFFER_SIZE];
	ssize_t bytesSent;
	ssize_t bytesRead;
	bool contentSent;

public:
	Sender(int clientFd, const std::string &content, int resourceFd = -1);

	bool handleSend();
};

#endif