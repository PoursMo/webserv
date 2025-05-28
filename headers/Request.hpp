#ifndef REQUEST_HPP
#define REQUEST_HPP

#include "webserv.hpp"
#include "Response.hpp"
#include "AInputHandler.hpp"
#include "AOutputHandler.hpp"

class VirtualServer;
class LocationData;
class Poller;
class Uri;

class Request: public AInputHandler, public AOutputHandler
{
private:
	// Attributes
	Method method;
	std::string target;
	std::map<std::string, std::string> headers;
	int32_t contentLength;
	int bodyFd;
	int clientFd;
	bool firstLineParsed;
	const std::vector<VirtualServer *> &vServers;
	const VirtualServer *vServer;
	const LocationData *locationData;
	const Poller &poller;
	Uri *uri;

	// Line received parsing
	void parseFirstLine(char *lstart, char *lend);
	bool checkEmptyline(char *lstart, char *lend);
	void parseHeaderLine(char *lstart, char *lend);

	// Setters
	std::string setTarget(char **lstart, char *lend);
	void addHeader(std::string key, std::string value);
	Method setMethod(char *lstart, char *lend);

	// Processing
	const VirtualServer *selectVServer();
	void processRequest();
	void setContentLength();

public:
	Request(int clientFd, const std::vector<VirtualServer *> &vServers, const Poller &poller);
	~Request();

	// Input handling
	bool isInputEnd();
	bool parseLine(char *lstart, char *lend);
	void onUpdateBodyBytes();
	ssize_t handleInputSysCall(void *buf, size_t len);
	void onHeaderBufferCreation();
	// Setter
	// void setBodyFd(int fd);
	// Getters
	const std::string &getTarget() const;
	const std::string &getPath() const;
	const std::string &getQuery() const;
	const std::string getHeaderValue(const std::string key) const;
	enum Method getMethod() const;
	int getBodyFd() const;
	int getClientFd() const;
	const VirtualServer *getVServer() const;
	const LocationData *getLocation() const;
	const Poller &getPoller() const;

	Response response;
};

#endif
