#ifndef CGIHANDLER_HPP
#define CGIHANDLER_HPP

#include "webserv.hpp"

class Request;

class CgiHandler
{
private:
	const Request &request;
	std::string resource;
	std::string query;
	std::string cgiExtension;
	std::string cgiPath;
	std::string cgiPathInfo;
	char **getCgiEnv();
	char **getCgiArgv();
	void initQuery();
	void initCgi();
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

class execve_error : public std::exception
{
public:
	const char *what() const throw();
};

#endif
