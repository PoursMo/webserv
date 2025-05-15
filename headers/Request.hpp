#ifndef REQUEST_HPP
#define REQUEST_HPP

#include <map>
#include <string>
#include "LocationData.hpp"
#include "http_error.hpp"
#include "Sender.hpp"

class VirtualServer;

class Request
{
private:
	Method method;
	std::string resource;
	std::map<std::string, std::string> headers;
	int bodyFd;
	int clientFd;
	unsigned long contentLength;
	bool firstLineParsed;
	VirtualServer *vServer;
	Sender *sender;
	const std::vector<VirtualServer *> &vServers;

	void parseFirstLine(char *lstart, char *lend);
	bool checkEmptyline(char *lstart, char *lend);
	Method setMethod(char *lstart, char *lend);
	std::string setResource(char **lstart, char *lend);
	void parseHeaderLine(char *lstart, char *lend);
	void addHeader(std::string key, std::string value);
	VirtualServer *selectVServer();

public:
	Request(int clientFd, const std::vector<VirtualServer *> &vServers);
	~Request();
	void parseRequestLine(char *lstart, char *lend);
	std::string getResource() const;
	enum Method getMethod() const;
	std::string getHeaderValue(const std::string key) const;
	int32_t getBodySize();
	int getBodyFd();

	void setError(int status);
	void processRequest();
	bool sendResponse();
};

#endif
