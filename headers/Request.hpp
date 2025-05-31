#ifndef REQUEST_HPP
#define REQUEST_HPP

#include "webserv.hpp"
#include "AInputHandler.hpp"
#include "AOutputHandler.hpp"

class Connection;
class VirtualServer;
class LocationData;
class Poller;
class Uri;

class Request : public AInputHandler, public AOutputHandler
{
private:
	// Attributes
	Method method;
	std::string target;
	int32_t contentLength;
	bool firstLineParsed;
	const std::vector<VirtualServer*>& vServers;
	const VirtualServer* vServer;
	const LocationData* locationData;
	Uri* uri;

	// Line received parsing
	void parseFirstLine(char* lstart, char* lend);

	// Setters
	std::string setTarget(char** lstart, char* lend);
	void addHeader(std::string key, std::string value);
	Method setMethod(char* lstart, char* lend);

	// Processing
	const VirtualServer* selectVServer();
	void processRequest();
	void setContentLength();

	// AInputHandler
	bool isInputEnd();
	void onInputEnd();
	bool parseLine(char* lstart, char* lend);
	void onUpdateBodyBytes();
	ssize_t handleInputSysCall(void* buf, size_t len);
	void onHeaderBufferCreation();

	// AOutputHandler
	ssize_t handleOutputSysCall(const void* buf, size_t len);
	bool isOutputEnd();
	void onOutputEnd();

public:
	Request(Poller& poller, Connection& connection, const std::vector<VirtualServer*>& vServers);
	~Request();

	// Getters
	const std::string& getTarget() const;
	const std::string& getPath() const;
	const std::string& getQuery() const;
	const std::string getHeaderValue(const std::string key) const;
	enum Method getMethod() const;
	const VirtualServer* getVServer() const;
	const LocationData* getLocation() const;
	const Poller& getPoller() const;
};

#endif
