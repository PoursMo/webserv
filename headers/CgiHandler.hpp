#ifndef CGIHANDLER_HPP
#define CGIHANDLER_HPP

#include "webserv.hpp"

class Request;

class CgiHandler
{
private:
	const Request &request;
	std::string extension;
	std::string pathExecutable;
	std::string pathFile;
	std::string pathFileAbsolute;
	std::string pathInfo;
	char **getCgiEnv();
	char **getCgiArgv();
	int fdIn;
	int fdOut;

public:
	CgiHandler(const Request &request);
	std::string getMethodString() const;
	bool isCgiResource() const;
	int cgiExecution();
	int getFdIn() const;
	int getFdOut() const;
};

class child_accident : public std::exception
{
public:
	const char *what() const throw();
};

#endif
