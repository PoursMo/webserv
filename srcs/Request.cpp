#include "Request.hpp"
#include <stdexcept>

//REMOVE ME
#include <iostream>

Request::Request()
{
}

Request::~Request()
{
}

bool isSupportedMethod(char **lstart, char *lend, std::string method)
{
	int	i = 0;

	while (**lstart != ' ' && *lstart != lend)
	{
		if (**lstart != method[i])
			return false,
		(*lstart)++;
	}
	return true;
}

Method Request::setMethod(char *lstart, char *lend)
{
	char *backup = lstart;
	std::string method;

	std::vector<std::string> existing_methods = {"GET", "HEAD", "POST", "PUT", "DELETE", "CONNECT", "OPTIONS", "TRACE"};
	for (std::vector<std::string>::iterator s = existing_methods.begin(); s != existing_methods.end(); s++)
	{
		method = *s;
		lstart = backup;
		int	i = 0;
		while (*lstart != ' ' && lstart != lend && i < method.size())
		{
			if (*lstart != method[i])
			{
				method = "";
				break ;
			}
			lstart++;
			i++;
		}
		if (method != "")
			break ;
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
	return GET; //Unreachable
}

std::string Request::setResource(char **lstart, char *lend)
{
	std::string resource;

	while (**lstart != *lend && **lstart != ' ')
	{
		resource = resource + **lstart;
		(*lstart)++;
	}
	if (resource == "")
		throw http_error(400);
	return (resource);
}

bool isValidProtocol(char **lstart, char *lend)
{
	int	i = 0;
	std::string protocol = "HTTP/1.1";
	while (*lstart != lend && i < protocol.size())
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
	while (lstart != lend && *lstart != ' ')
		lstart++;
	if (*lstart != ' ' || lstart == lend)
		throw http_error(400);
	lstart++;
	this->resource = setResource(&lstart, lend);
	if (*lstart != ' ' || lstart == lend)
		throw http_error(400);
	lstart++;
	if(!isValidProtocol(&lstart, lend))
		throw http_error(400);
	if (lstart != lend)
		throw http_error(400);
	this->firstLineParsed = true;
}

void Request::parseHeaderLine(char *lstart, char *lend)
{
	while (lstart != lend)
	char *key;
}

void Request::parseRequestLine(char *lstart, char *lend)
{
	if (!this->firstLineParsed)
		parseFirstLine(lstart, lend);
	else
		parseHeaderLine(lstart, lend);
}

const char *Request::UnrecognizedMethod::what() const throw()
{
	return ("Method not recognized");
}

const char *Request::UnsupportedMethod::what() const throw()
{
	return ("Method not supported");
}

// int main()
// {
// 	Request test_request;

// 	char test_line[]="GET / HTTP/1.1";
// 	char *first = &(test_line[0]);

// 	int i = 0;
// 	while (test_line[i])
// 		i++;
// 	char *last = first + i;
// 	test_request.parseRequestLine(first, last);
// 	std::cout << "valid first line" << std::endl;
// 	return 0;
// }

