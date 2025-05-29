#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include "webserv.hpp"
#include "CgiHandler.hpp"

class Connection;
class Sender;

class Response: public AInputHandler, public AOutputHandler
{
private:
	const Connection &connection;
	std::map<std::string, std::string> headers;
	Sender *sender;
	pid_t cgiPid;
	bool isCGI;

	// Setters
	void setSender(int status, const std::string &content);
	void setSender(int status, int targetFd);
	std::string generateHeader(int status) const;
	std::string getIndexPage(const std::string &path);
	int fileHandler(const std::string &path);
public:
	Response(const Connection &connection);
	~Response();

	// Setters
	void addHeader(std::string key, std::string value);
	void addHeader(std::string key, unsigned long value);
	void addHeader(std::string key, long value);
	void setTargetSender(const std::string &path, int status = 200);
	void setErrorSender(int status);
	bool send() const;

	void handleReturn(const std::pair<int, std::string> &returnPair);
};

#endif
