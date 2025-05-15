#ifndef SENDER_HPP
#define SENDER_HPP

#include <string>
#include <sys/types.h>

#define WS_SENDER_BUFFER_SIZE 16384

class Sender
{
private:
	int clientFd;
	int resourceFd;
	std::string str;
	char buffer[WS_SENDER_BUFFER_SIZE];
	ssize_t bytesSent;
	ssize_t bytesRead;

public:
	Sender(int clientFd, std::string str);
	Sender(int clientFd, int resourceFd);

	bool handleSend();
};

#endif