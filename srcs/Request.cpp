#include "../headers/Request.hpp"
#include "VirtualServer.hpp"
#include "Receiver.hpp"
#include "utils.hpp"

#include <fcntl.h>
#include <unistd.h>
#include <stdexcept>
#include <cstring>
#include <cerrno>

// REMOVE ME
#include <iostream>

#define WS_MAX_URI_SIZE 6144

VirtualServer *Request::selectVServer()
{
	const std::string &hostName = this->getHeaderValue("host");
	if (hostName.empty())
		throw http_error(400);
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

Request::Request(int clientFd, const std::vector<VirtualServer *> &vServers)
	: bodyFd(-1),
	  clientFd(clientFd),
	  contentLength(0),
	  firstLineParsed(false),
	  sender(NULL),
	  vServers(vServers)
{
}

Request::~Request()
{
	if (bodyFd != -1)
		close(bodyFd);
	if (this->sender)
		delete this->sender;
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
				throw http_error(400);
			else
				break;
		}
	}
	if (method == "")
		throw http_error(400);
	if (method == "GET")
		return GET;
	else if (method == "POST")
		return POST;
	else if (method == "DELETE")
		return DELETE;
	else
		throw http_error(501);
	return GET; // Unreachable
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
		throw http_error("Empty resource in request", 400);
	return (resource);
}

void Request::setError(int status)
{
	this->sender = new Sender(clientFd, generateErrorResponse(status));
}

void Request::processRequest()
{
	vServer = selectVServer();
	if (this->getBodySize() > vServer->getClientMaxBodySize())
		throw http_error("Body size > Client max body size", 413);
	this->sender = new Sender(clientFd, "HTTP/1.1 200 OK\r\n");
}

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
		throw http_error("No space after method", 400);
	lstart++;
	this->resource = setResource(&lstart, lend);
	if (this->resource.size() > WS_MAX_URI_SIZE)
		throw http_error(414);
	if (*lstart != ' ' || lstart == lend || *lstart == '\r')
		throw http_error("No space after resource name", 400);
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

void Request::addHeader(std::string key, std::string value)
{
	headers[str_to_lower(key)] = value;
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

void Request::parseRequestLine(char *lstart, char *lend)
{
	if (lstart == NULL || lend == NULL || *lend != '\n')
		throw http_error("Empty request line", 400);
	if (checkEmptyline(lstart, lend))
	{
		if (this->method == POST && !headers.count("content-length"))
			throw http_error("No content-length in POST request", 411);
		bodyFd = open("./logs/body.out", O_CREAT | O_WRONLY | O_TRUNC, 0644); // tmp
		if (bodyFd == -1)													  // tmp
			throw http_error("open: " + std::string(strerror(errno)), 500);	  // tmp
		return;
	}
	if (!this->firstLineParsed)
		parseFirstLine(lstart, lend);
	else
		parseHeaderLine(lstart, lend);
}

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
	return (this->method);
}

int Request::getBodyFd()
{
	return bodyFd;
}

int32_t Request::getBodySize()
{
	std::string str = getHeaderValue("content-length");
	long res = std::strtoul(str.c_str(), 0, 10);
	if (res > UINT32_MAX)
		throw http_error("content-length > UINT32_MAX", 413);
	return res;
}

bool Request::sendResponse()
{
	return this->sender->handleSend();
}