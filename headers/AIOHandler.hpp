#ifndef AIOHANDLER_HPP
#define AIOHANDLER_HPP

#include "webserv.hpp"

class Poller;
class Connection;

class AIOHandler
{
protected:
	struct Buffer
	{
		char* first;
		char* last;
		char* pos;
		size_t capacity;
	};
	enum BufferType
	{
		HEADER,
		BODY
	};

	Poller& poller;
	Connection& connection;

	std::list<Buffer*> buffers;
	std::map<std::string, std::string> headers;
	void delFd(int& fd);

	int inputFd;
	int outputFd;

	// parser utils
	static bool isEmptyline(char* lstart, char* lend);
	void parseHeaderLine(char* lstart, char* lend);

	AIOHandler(Poller& poller, Connection& connection);
	virtual ~AIOHandler();

public:
	virtual void handleInput() = 0;
	virtual void handleOutput() = 0;
	virtual void addHeader(std::string key, std::string value) = 0;
	static void printBuffer(const char* first, const char* const last);
	std::ostream& printIOHandler();
};

#endif
