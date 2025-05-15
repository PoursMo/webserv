#ifndef SENDER_HPP
#define SENDER_HPP

#include <string>

class Sender
{
private:
	int clientFd;
	int resourceFd;
	std::string str;

public:
	Sender(int clientFd, std::string str);
	Sender(int clientFd, int resourceFd);

	bool handleSend();
};

#endif