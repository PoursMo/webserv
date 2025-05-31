#include "Request.hpp"
#include "VirtualServer.hpp"
#include "Connection.hpp"
#include "LocationData.hpp"
#include "Logger.hpp"
#include "utils.hpp"
#include "http_error.hpp"
#include "Uri.hpp"

Request::Request(Poller& poller, Connection& connection, const std::vector<VirtualServer*>& vServers)
	: AIOHandler(poller, connection),
	contentLength(0),
	firstLineParsed(false),
	vServers(vServers),
	vServer(NULL),
	locationData(NULL),
	uri(NULL)
{
}

Request::~Request()
{
	if (this->uri)
		delete this->uri;
	if (this->outputFd != -1)
		close(this->outputFd);
}

// ********************************************************************
// Processing
// ********************************************************************

const VirtualServer* Request::selectVServer()
{
	const std::string& hostName = this->getHeaderValue("host");
	for (std::vector<VirtualServer*>::const_iterator i = vServers.begin(); i != vServers.end(); i++)
	{
		for (std::vector<std::string>::const_iterator j = (*i)->getServerNames().begin(); j != (*i)->getServerNames().end(); j++)
		{
			if (*j == hostName)
				return *i;
		}
	}
	return (vServers.front());
}

void Request::processRequest()
{
	if (!this->headers.count("host"))
		throw http_error("No host header field-line in request", 400);
	this->vServer = selectVServer();
	if (this->headers.at("host").empty())
		throw http_error("Empty host header", 400);
	this->locationData = vServer->getLocation(this->getPath());
	if (this->getPath()[0] != '/')
		throw http_error("No '/' in path", 400);
	if (!this->locationData)
		throw http_error("Target not found in location data", 404);
	const std::pair<int, std::string>& returnPair = this->locationData->getReturnPair();
	if (returnPair.first != -1)
	{
		this->connection.response.handleReturn(returnPair);
		return;
	}
	if (!isInVector(this->locationData->getMethods(), this->method))
		throw http_error("Method not in location data", 403);
	if (this->method == POST && !headers.count("content-length"))
		throw http_error("No content-length in POST request", 411);
	if (this->contentLength > vServer->getClientMaxBodySize())
		throw http_error("Body size > Client max body size", 413);
	std::string fullPath = locationData->getRoot() + this->getPath();
	this->connection.response.handlePath(fullPath);
}

void Request::setContentLength()
{
	std::string str = getHeaderValue("content-length");
	long res = std::strtoul(str.c_str(), 0, 10);
	if (res > UINT32_MAX)
		throw http_error("content-length > UINT32_MAX", 413);
	this->contentLength = res;
}

// ********************************************************************
// Setters
// ********************************************************************

std::string Request::setTarget(char** lstart, char* lend)
{
	std::string target;

	while (**lstart != *lend && **lstart != ' ' && **lstart != '\r')
	{
		target = target + **lstart;
		(*lstart)++;
	}
	if (target == "")
		throw http_error("Empty target in header-field line", 400);
	return (target);
}

void Request::addHeader(std::string key, std::string value)
{
	str_to_lower(key);
	try
	{
		std::string& headerValue = this->headers.at(key);
		headerValue += ", " + value;
	}
	catch (const std::exception& e)
	{
		this->headers[key] = value;
	}
}

Method Request::setMethod(char* lstart, char* lend)
{
	char* backup = lstart;

	std::string method;

	const char* methods_array[] = { "GET", "HEAD", "POST", "PUT", "DELETE", "CONNECT", "OPTIONS", "TRACE" };
	std::vector<std::string> existing_methods(methods_array, methods_array + sizeof(methods_array) / sizeof(methods_array[0]));
	for (std::vector<std::string>::iterator s = existing_methods.begin(); s != existing_methods.end(); s++)
	{
		method = *s;
		lstart = backup;
		size_t i = 0;
		while (*lstart != ' ' && lstart != lend && i < method.size() && *lstart != '\r')
		{
			if (*lstart != method[i])
			{
				method = "";
				break;
			}
			lstart++;
			i++;
		}
		if (method != "")
		{
			if (*lstart != ' ')
				throw http_error("Method not followed by space in header-field line", 400);
			else
				break;
		}
	}
	if (method == "")
		throw http_error("Method not available in HTTP/1.1", 405);
	if (method == "GET")
		return GET;
	else if (method == "POST")
		return POST;
	else if (method == "DELETE")
		return DELETE;
	else
		throw http_error("Method not supported by this server", 501);
	return GET; // Unreachable
}

// ********************************************************************
// AInputHandler
// ********************************************************************

bool Request::parseLine(char* lstart, char* lend)
{
	if (lstart == NULL || lend == NULL || *lend != '\n')
		throw http_error("Empty request line", 400);
	if (AIOHandler::isEmptyline(lstart, lend))
	{
		this->processRequest();
		this->setContentLength();
		return false;
	}
	if (!this->firstLineParsed)
		parseFirstLine(lstart, lend);
	else
		this->parseHeaderLine(lstart, lend);
	return true;
}

ssize_t Request::handleInputSysCall(void* buf, size_t len)
{
	logger.log() << "Request: recving for " << len << " bytes on fd " << this->inputFd << std::endl;
	ssize_t bytesRecved = recv(this->inputFd, buf, len, 0);
	logger.log() << "Request: recved " << bytesRecved << " bytes" << std::endl;
	if (bytesRecved == -1)
		throw http_error("recv: " + std::string(strerror(errno)), 500);
	return bytesRecved;
}

bool Request::isInputEnd()
{
	return (this->inputFd == -1 || (!this->isReadingHeader && this->bodyBytesCount == this->contentLength));
}

void Request::onInputEnd()
{
	this->connection.print("Request: On Input end");
}

void Request::onUpdateBodyBytes() {
	if (this->bodyBytesCount > this->contentLength)
		throw http_error("bodyBytesCount > contentLength", 400);
}

void Request::onHeaderBufferCreation()
{
	if (this->headerBufferCount >= 4)
		throw http_error("Header too large", 400);
}

// ********************************************************************
// AOutputHandler
// ********************************************************************

ssize_t Request::handleOutputSysCall(const void* buf, size_t len)
{
	logger.log() << "Request: writing " << len << " bytes" << std::endl;
	ssize_t bytesWritten = write(this->outputFd, buf, len);
	if (bytesWritten == -1)
		throw http_error("write: " + std::string(strerror(errno)), 500);
	logger.log() << "Request: wrote " << len << " bytes" << std::endl;
	return bytesWritten;
}

bool Request::isOutputEnd()
{
	return this->buffers.empty() && this->isInputEnd();
}

void Request::onOutputEnd()
{
	this->unsubscribeOutputFd();
}

// ********************************************************************
// Input parsing
// ********************************************************************

bool isValidProtocol(char** lstart, char* lend)
{
	size_t i = 0;
	std::string protocol = "HTTP/1.1";
	while (*lstart != lend && i < protocol.size() && **lstart != '\r')
	{
		if (**lstart != protocol[i])
			return false;
		i++;
		(*lstart)++;
	}
	return true;
}

void Request::parseFirstLine(char* lstart, char* lend)
{
	this->method = setMethod(lstart, lend);
	while (lstart != lend && *lstart != ' ' && *lstart != '\r')
		lstart++;
	if (*lstart != ' ' || lstart == lend || *lstart == '\r')
		throw http_error("No space after method in header-field line", 400);
	lstart++;
	this->target = setTarget(&lstart, lend);
	if (this->target.size() > WS_MAX_URI_SIZE)
		throw http_error(414);
	this->uri = new Uri(*this);
	if (*lstart != ' ' || lstart == lend || *lstart == '\r')
		throw http_error("No space after target name in header-field line", 400);
	lstart++;
	if (!isValidProtocol(&lstart, lend))
		throw http_error("Invalid Protocol", 400);
	if (!(*lstart == '\r' || lstart == lend))
		throw http_error("First line not correctly terminated", 400);
	if (*lstart == '\r')
	{
		lstart++;
		if (lstart != lend)
			throw http_error("First line not correctly terminated", 400);
	}
	this->firstLineParsed = true;
}

// ********************************************************************
// Getters
// ********************************************************************

const std::string& Request::getTarget() const
{
	return (this->target);
}

const std::string& Request::getPath() const
{
	if (!this->uri)
		throw std::runtime_error("Uri is not defined");
	return this->uri->getPath();
}

const std::string& Request::getQuery() const
{
	if (!this->uri)
		throw std::runtime_error("Uri is not defined");
	return this->uri->getQuery();
}

const std::string Request::getHeaderValue(const std::string key) const
{
	if (this->headers.count(key))
		return this->headers.at(key);
	return "";
}

enum Method Request::getMethod() const
{
	return this->method;
}

const VirtualServer* Request::getVServer() const
{
	return this->vServer;
}

const LocationData* Request::getLocation() const
{
	return this->locationData;
}

const Poller& Request::getPoller() const
{
	return this->poller;
}
