#ifndef REQUEST_HPP
#define REQUEST_HPP

#include "webserv.hpp"
#include "Response.hpp"
#include "uri.hpp"

class VirtualServer;
class LocationData;
class Poller;

class Request
{
private:
	// Attributes
	Method method;
	std::string resource;
	std::map<std::string, std::string> headers;
	int bodyFd;
	int clientFd;
	bool firstLineParsed;
	const std::vector<VirtualServer *> &vServers;
	const VirtualServer *vServer;
	const LocationData *locationData;
	const Poller &poller;

	// Line received parsing
	void parseFirstLine(char *lstart, char *lend);
	bool checkEmptyline(char *lstart, char *lend);
	void parseHeaderLine(char *lstart, char *lend);

	// Setters
	std::string setResource(char **lstart, char *lend);
	void addHeader(std::string key, std::string value);
	Method setMethod(char *lstart, char *lend);

	// Processing
	const VirtualServer *selectVServer();

public:
	Request(int clientFd, const std::vector<VirtualServer *> &vServers, const Poller &poller);
	~Request();

	// Line received parsing
	bool parseRequestLine(char *lstart, char *lend);

	// Processing
	void processRequest();
	// Setter
	void setBodyFd(int fd);
	// Getters
	std::string getResource() const;
	std::string getHeaderValue(const std::string key) const;
	enum Method getMethod() const;
	int getBodyFd() const;
	int getClientFd() const;
	int32_t getBodySize() const;
	const VirtualServer *getVServer() const;
	const LocationData *getLocation() const;
	const Poller &getPoller() const;

	Response response;
};

#endif
