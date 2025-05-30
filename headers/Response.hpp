#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include "webserv.hpp"
#include "CgiHandler.hpp"
#include "AInputHandler.hpp"
#include "AOutputHandler.hpp"

class Connection;

class Response: public AInputHandler, public AOutputHandler
{
private:
	pid_t cgiPid;

	// Setters
	void setSender(int status, const std::string &content);
	void setSender(int status, int targetFd);
	std::string generateHeader(int status) const;
	std::string getIndexPage(const std::string &path);
	void handleFile(const std::string &path);

	// AInputHandler
	bool isInputEnd();
	void onInputEnd();
	bool parseLine(char *lstart, char *lend);
	void onUpdateBodyBytes();
	ssize_t handleInputSysCall(void *buf, size_t len);
	void onHeaderBufferCreation();

	// AOutputHandler
	ssize_t handleOutputSysCall(const void *buf, size_t len);
	bool isOutputEnd();
	void onOutputEnd();

public:
	Response(Connection &connection, Poller &poller);
	~Response();

	// Setters
	void addHeader(std::string key, std::string value);
	void addHeader(std::string key, unsigned long value);
	void addHeader(std::string key, long value);
	void handlePath(const std::string &path);
	void setErrorSender(int status);

	void handleReturn(const std::pair<int, std::string> &returnPair);
};

#endif
