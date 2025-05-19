#include "Request.hpp"
#include "VirtualServer.hpp"
#include "LocationData.hpp"
#include "Receiver.hpp"
#include "utils.hpp"
#include "http_error.hpp"

Request::Request(int clientFd, const std::vector<VirtualServer *> &vServers)
	: bodyFd(-1),
	  clientFd(clientFd),
	  firstLineParsed(false),
	  vServers(vServers),
	  vServer(0),
	  response(*this)
{
}

Request::~Request()
{
	if (bodyFd != -1)
		close(bodyFd);
}

// ********************************************************************
// Processing
// ********************************************************************

const VirtualServer *Request::selectVServer()
{
	const std::string &hostName = this->getHeaderValue("host");
	if (hostName.empty())
		throw http_error("No host header field-line in request", 400);
	for (std::vector<VirtualServer *>::const_iterator i = vServers.begin(); i != vServers.end(); i++)
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
	this->vServer = selectVServer();
	this->locationData = vServer->getLocation(resource);
	if (resource[0] != '/')
		throw http_error("No '/' in resource", 400);
	if (!this->locationData)
		throw http_error("Resource not found in location data", 404);
	const std::pair<int, std::string> &returnPair = this->locationData->getReturnPair();
	if (returnPair.first != -1)
	{
		this->response.handleReturn(returnPair);
		return;
	}
	if (!isInVector(this->locationData->getMethods(), this->method))
		throw http_error("Method not in location data", 403);
	// TODO: method == GET and body exist without content-length
	if (this->method == POST && !headers.count("content-length"))
		throw http_error("No content-length in POST request", 411);
	if (this->getBodySize() > vServer->getClientMaxBodySize())
		throw http_error("Body size > Client max body size", 413);
	std::string fullPath = locationData->getRoot() + resource;
	this->response.setResourceSender(fullPath);
}

// ********************************************************************
// Setters
// ********************************************************************

void Request::setBodyFd(int fd)
{
	this->bodyFd = fd;
}

std::string Request::setResource(char **lstart, char *lend)
{
	std::string resource;

	while (**lstart != *lend && **lstart != ' ' && **lstart != '\r')
	{
		resource = resource + **lstart;
		(*lstart)++;
	}
	if (resource == "")
		throw http_error("Empty resource in header-field line", 400);
	return (resource);
}

void Request::addHeader(std::string key, std::string value)
{
	headers[str_to_lower(key)] = value;
}

Method Request::setMethod(char *lstart, char *lend)
{
	char *backup = lstart;

	std::string method;

	const char *methods_array[] = {"GET", "HEAD", "POST", "PUT", "DELETE", "CONNECT", "OPTIONS", "TRACE"};
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
		throw http_error("Method not available in HTTP/1.1", 400);
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
// Line received parsing
// ********************************************************************

bool isValidProtocol(char **lstart, char *lend)
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

void Request::parseFirstLine(char *lstart, char *lend)
{
	this->method = setMethod(lstart, lend);
	while (lstart != lend && *lstart != ' ' && *lstart != '\r')
		lstart++;
	if (*lstart != ' ' || lstart == lend || *lstart == '\r')
		throw http_error("No space after method in header-field line", 400);
	lstart++;
	this->resource = setResource(&lstart, lend);
	if (this->resource.size() > WS_MAX_URI_SIZE)
		throw http_error(414);
	if (*lstart != ' ' || lstart == lend || *lstart == '\r')
		throw http_error("No space after resource name in header-field line", 400);
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

void Request::parseHeaderLine(char *lstart, char *lend)
{
	std::string key;
	std::string value;

	while (lstart != lend && *lstart != '\r' && *lstart != ':')
	{
		key = key + *lstart;
		lstart++;
	}
	if (*lstart != ':')
		throw http_error("No ':' in header line", 400);
	lstart++;
	if (*lstart != ' ' || lstart == lend)
		throw http_error("No space between key and value in header line", 400);
	lstart++;
	while (lstart != lend && *lstart != '\r')
	{
		value = value + *lstart;
		lstart++;
	}
	if (*lstart != '\r' && lstart != lend)
		throw http_error("Header line not correctly ended", 400);
	if (*lstart == '\r' && *(lstart + 1) != '\n')
		throw http_error("Header line not correctly ended", 400);
	this->addHeader(key, value);
}

bool Request::checkEmptyline(char *lstart, char *lend)
{
	if (*lstart != '\r' && *lstart != '\n')
		return false;
	if (lstart == lend && *lstart == '\n')
		return true;
	lstart++;
	if (lstart == lend && *lstart == '\n')
		return true;
	return false;
}

bool Request::parseRequestLine(char *lstart, char *lend)
{
	if (lstart == NULL || lend == NULL || *lend != '\n')
		throw http_error("Empty request line", 400);
	if (checkEmptyline(lstart, lend))
	{
		this->processRequest();
		return false;
	}
	if (!this->firstLineParsed)
		parseFirstLine(lstart, lend);
	else
		parseHeaderLine(lstart, lend);
	return true;
}

// ********************************************************************
// Getters
// ********************************************************************

std::string Request::getResource() const
{
	return (this->resource);
}

std::string Request::getHeaderValue(const std::string key) const
{
	if (this->headers.count(key))
		return this->headers.at(key);
	return "";
}

enum Method Request::getMethod() const
{
	return this->method;
}

int Request::getBodyFd() const
{
	return this->bodyFd;
}

int Request::getClientFd() const
{
	return this->clientFd;
}

int32_t Request::getBodySize() const
{
	// TODO: handle transfert encoding
	std::string str = getHeaderValue("content-length");
	long res = std::strtoul(str.c_str(), 0, 10);
	if (res > UINT32_MAX)
		throw http_error("content-length > UINT32_MAX", 413);
	return res;
}

const VirtualServer *Request::getVServer() const
{
	return this->vServer;
}

const LocationData *Request::getLocation() const
{
	return this->locationData;
}
