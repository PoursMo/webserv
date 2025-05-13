#include "../headers/Request.hpp"
#include <stdexcept>

//REMOVE ME
#include <iostream>

Request::Request()
{
	bodyFd = -1;
	firstLineParsed = false;
	headerParsed = false;
	socketFd = -1;
	contentLength = 0;
}

Request::~Request()
{
}

void Request::RequestError(int code) const
{
	try
	{
		throw http_error(code);
	}
	catch(const std::exception& e)
	{
		std::cerr << e.what() << '\n';
	}
}

Method Request::setMethod(char *lstart, char *lend)
{
	char *backup = lstart;

	std::string method;

	const char* methods_array[] = {"GET", "HEAD", "POST", "PUT", "DELETE", "CONNECT", "OPTIONS", "TRACE"};
	std::vector<std::string> existing_methods(methods_array, methods_array + sizeof(methods_array)/sizeof(methods_array[0]));
	for (std::vector<std::string>::iterator s = existing_methods.begin(); s != existing_methods.end(); s++)
	{
		method = *s;
		lstart = backup;
		size_t	i = 0;
		while (*lstart != ' ' && lstart != lend && i < method.size() && *lstart != '\r')
		{
			if (*lstart != method[i])
			{
				method = "";
				break ;
			}
			lstart++;
			i++;
		}
		if (method != "" )
		{
			if (*lstart != ' ')
				RequestError(400);
			else
				break ;
		}
	}
	if (method == "")
		RequestError(400);
	if (method == "GET")
		return GET;
	else if (method == "POST")
		return POST;
	else if (method == "DELETE")
		return DELETE;
	else
		RequestError(501);
	return GET; //Unreachable
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
		RequestError(400);
	return (resource);
}

bool isValidProtocol(char **lstart, char *lend)
{
	size_t	i = 0;
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
		RequestError(400);
	lstart++;
	this->resource = setResource(&lstart, lend);
	if (*lstart != ' ' || lstart == lend || *lstart == '\r')
		RequestError(400);
	lstart++;
	if(!isValidProtocol(&lstart, lend))
		RequestError(400);
	if (*lstart != '\r')
		RequestError(400);
	lstart++;
	if (lstart != lend)
		RequestError(400);
	this->firstLineParsed = true;
}

void Request::addHeader(std::string key, std::string value)
{
	this->headers.insert(std::make_pair(key, value));
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
		RequestError(400);
	lstart++;
	if (*lstart != ' ' || lstart == lend)
		RequestError(400);
	lstart++;
	while (lstart != lend && *lstart != '\r')
	{
		value = value + *lstart;
		lstart++;
	}
	if (!(*lstart == '\r' && *(lstart + 1) == '\n'))
		RequestError(400);
	this->addHeader(key, value);
}

bool Request::checkEmptyline(char *lstart, char *lend)
{
	if (*lstart != '\r')
		return false;
	lstart++;
	if (lstart == lend && *lstart == '\n')
		return true;
	return false;
}

void Request::parseRequestLine(char *lstart, char *lend)
{
	if (this->headerParsed)
		return ;
	if (*lend != '\n' || lstart == lend || lstart == NULL || lend == NULL)
		RequestError(400);
	if (checkEmptyline(lstart, lend))
	{
		this->headerParsed = true;
		return ;
	}
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

std::string Request::getResource() const
{
	return (this->resource);
}

std::string Request::getHeaderValue(const std::string key) const
{
	return ((*((this->headers).find(key))).second);
}

enum Method Request::getMethod() const
{
	return (this->method);
}


// int main()
// {
// 	Request test_request;

// 	char test_line[]="POST / HTTP/1.1\r\n";
// 	char *first = &(test_line[0]);

// 	int i = 0;
// 	while (test_line[i])
// 		i++;
// 	char *last = first + i - 1;

// 	char test_line2[]="Content-Size: test\r\n";
// 	char *first2 = &(test_line2[0]);

// 	int i2 = 0;
// 	while (test_line2[i2])
// 		i2++;
// 	char *last2 = first2 + i2 - 1;

// 	test_request.parseRequestLine(first, last);
// 	test_request.parseRequestLine(first2, last2);
// 	std::string target = "Content-Size";
// 	std::cout << "valid first line" << std::endl;
// 	std::cout << "Header Value of " << target << " is : " << test_request.getHeaderValue(target) << std::endl;
// 	return 0;
// }
