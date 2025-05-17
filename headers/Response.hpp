#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include "webserv.hpp"

class Request;
class Sender;

class Response
{
private:
	const Request &request;
	std::map<std::string, std::string> headers;
	Sender *sender;

	// Setters
	void setSender(int status, const std::string &content = "", int resourceFd = -1);
	std::string generateHeader(int status) const;
	std::string getIndexPage(const std::string &path);
	
	//
	int fileHandler(const std::string &path) const;
	
public:
	Response(const Request &request);
	~Response();

	// Setters
	void addHeader(std::string key, std::string value);
	void addHeader(std::string key, unsigned long value);
	void addHeader(std::string key, long value);
	void setResourceSender(const std::string &path, int status = 200);
	void setErrorSender(int status);
	bool send() const;
};

#endif
