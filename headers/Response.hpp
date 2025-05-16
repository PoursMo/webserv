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
	struct stat statBuffer;
	std::string generateHeader(int status) const;
	
	public:
	Response(const Request &request);
	~Response();
	void addHeader(std::string key, std::string value);
	void addHeader(std::string key, unsigned long value);
	void addHeader(std::string key, long value);
	void setStat(const std::string &path);
	bool setFileSender(const std::string &path, int status);
	void setErrorSender(int status);
	bool send() const;
};

#endif
