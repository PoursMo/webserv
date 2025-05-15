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

VirtualServer *find_virtual_server_with_host_name(const std::map<int, std::vector<VirtualServer *> > &m, const std::string &name)
{
	for (std::map<int, std::vector<VirtualServer *> >::const_iterator i = m.begin(); i != m.end(); i++)
	{
		for (std::vector<VirtualServer *>::const_iterator j = i->second.begin(); j != i->second.end(); j++)
		{
			for (std::vector<std::string>::const_iterator k = (*j)->getServerNames().begin(); k != (*j)->getServerNames().end(); k++)
			{
				if (*k == name)
					return *j;
			}
		}
	}
	return 0;
}

Request::Request(int clientFd)
	: bodyFd(-1),
	  clientFd(clientFd),
	  error(false),
	  contentLength(0),
	  firstLineParsed(false)
{
}

Request::~Request()
{
	if (bodyFd != -1)
		close(bodyFd);
}

void Request::RequestError(int code)
{
	try
	{
		throw http_error(code);
	}
	catch (std::exception &e)
	{
		if (this->error == false)
		{
			this->error = true;
			// Manage error
			std::cerr << e.what() << '\n';
		}
	}
	// REDIRECT TOWARDS ERROR MANAGEMENT
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
		throw http_error(400);
	return (resource);
}

void Request::setVirtualServer(const std::map<int, std::vector<VirtualServer *> > &servers)
{
	try
	{
		vServer = find_virtual_server_with_host_name(servers, headers.at("host"));
	}
	catch (...)
	{
		throw http_error(400);
	}
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
		throw http_error(400);
	lstart++;
	this->resource = setResource(&lstart, lend);
	if (*lstart != ' ' || lstart == lend || *lstart == '\r')
		throw http_error(400);
	lstart++;
	if (!isValidProtocol(&lstart, lend))
		throw http_error(400);
	if (!(*lstart == '\r' || lstart == lend))
		throw http_error(400);
	if (*lstart == '\r')
	{
		lstart++;
		if (lstart != lend)
			throw http_error(400);
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
		throw http_error(400);
	lstart++;
	if (*lstart != ' ' || lstart == lend)
		throw http_error(400);
	lstart++;
	while (lstart != lend && *lstart != '\r')
	{
		value = value + *lstart;
		lstart++;
	}
	if (*lstart != '\r' && lstart != lend)
		throw http_error(400);
	if (*lstart == '\r' && *(lstart + 1) != '\n')
		throw http_error(400);
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
		throw http_error(400);
	if (checkEmptyline(lstart, lend))
	{
		bodyFd = open("./logs/body.out", O_CREAT | O_WRONLY | O_TRUNC, 0644);
		if (bodyFd == -1)
			throw http_error("open: " + std::string(strerror(errno)), 500);
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

size_t Request::getBodySize()
{
	std::string str = getHeaderValue("content-length");
	return std::strtoul(str.c_str(), 0, 10);
}