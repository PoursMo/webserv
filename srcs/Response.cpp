#include "Response.hpp"
#include "Request.hpp"
#include "LocationData.hpp"
#include "http_error.hpp"
#include "http_status.hpp"
#include "utils.hpp"
#include "autoindex.hpp"
#include "Sender.hpp"
#include "VirtualServer.hpp"

Response::Response(Request &request)
	: request(request),
	  sender(NULL),
	  cgiPid(0),
	  isCGI(false)
{
}

Response::~Response()
{
	if (this->cgiPid)
	{
		kill(this->cgiPid, SIGKILL);
		waitpid(this->cgiPid, NULL, 0);
	}
	if (this->sender)
		delete this->sender;
}

// ********************************************************************
// Logic
// ********************************************************************

bool Response::send() const
{
	if (!this->sender)
		throw std::runtime_error("response->sender not defined !");
	return this->sender->handleSend();
}

void Response::handleReturn(const std::pair<int, std::string> &returnPair)
{
	if (returnPair.first == 301 || returnPair.first == 302 || returnPair.first == 303 || returnPair.first == 307)
	{
		this->addHeader("Location", returnPair.second);
		this->setErrorSender(returnPair.first);
	}
	else
	{
		this->addHeader("Content-Type", "text/plain");
		this->setSender(returnPair.first, returnPair.second);
	}
}

std::string Response::getIndexPage(const std::string &path)
{
	const std::vector<std::string> &indexes = this->request.getLocation()->getIndexes();
	for (std::vector<std::string>::const_iterator i = indexes.begin(); i != indexes.end(); i++)
	{
		std::string fullPath = path + *i;
		if (access(fullPath.c_str(), R_OK) != -1)
			return fullPath;
	}
	return "";
}

int Response::fileHandler(const std::string &path)
{
	CgiHandler cgi(this->request);
	int fdIn;
	int fdOut;
	this->isCGI = cgi.isCgiResource();
	if (this->isCGI)
	{
		this->cgiPid = cgi.cgiExecution();
		fdIn = cgi.getFdIn();
		fdOut = cgi.getFdOut();
	}
	else
	{
		if (this->request.getMethod() != GET)
			throw http_error("Only GET method can be handled without CGI", 405);
		fdIn = open("/dev/null", O_CREAT | O_WRONLY | O_TRUNC, 0644);
		fdOut = open(path.c_str(), O_RDONLY);
		if (fdIn == -1 || fdOut == -1)
			throw http_error("open: " + std::string(strerror(errno)), 500);
	}
	this->request.setBodyFd(fdIn);
	return fdOut;
}

void Response::setResourceSender(const std::string &path, int status)
{
	std::cout << "accessing path: " << path << std::endl;
	if (access(path.c_str(), R_OK) == -1)
		throw http_error("access: " + std::string(strerror(errno)), 404);
	struct stat statBuffer;
	if (stat(path.c_str(), &statBuffer) == -1)
		throw http_error("stat: " + std::string(strerror(errno)), 500);

	// POST/DELETE
	if (S_ISDIR(statBuffer.st_mode))
	{
		if (path[path.size() - 1] != '/')
		{
			std::string loc = path.substr(this->request.getLocation()->getRoot().size());
			this->addHeader("Location", loc + '/');
			this->setErrorSender(301);
			return;
		}
		std::string indexPagePath = getIndexPage(path);
		if (indexPagePath.empty())
		{
			if (!this->request.getLocation()->getAutoIndex())
				throw http_error("This resource is a dir not have index pages and autoindex is disabled", 404);
			this->setSender(200, getAutoIndexHtml(path, this->request.getLocation()->getRoot()));
		}
		else
		{
			this->setResourceSender(indexPagePath);
		}
	}
	else if (S_ISREG(statBuffer.st_mode))
	{
		// this->addHeader("Last-Modified", getDateString(statBuffer.st_mtim.tv_sec));
		// this->addHeader("Content-Length", statBuffer.st_size);
		this->setSender(status, this->fileHandler(path));
	}
	else
		throw http_error("Resource is neither a directory nor a regular file", 422);
}

void Response::setErrorSender(int status)
{
	const VirtualServer *vServer = this->request.getVServer();

	if (vServer && this->request.getLocation() && vServer->getErrorPages().count(status))
	{
		const std::string &path = vServer->getErrorPages().at(status);
		try
		{
			this->setResourceSender(path, status);
			return;
		}
		catch (const http_error &e)
		{
			std::cerr << e.what() << '\n';
		}
	}
	std::string body = generateErrorBody(status);
	this->addHeader("Content-Type", "text/html");
	this->setSender(status, body);
}

std::string Response::generateHeader(int status) const
{
	std::stringstream header;
	const std::string &statusName = http_status::get(status);

	header << "HTTP/1.1 " << status << " " << statusName << CRLF;
	header << "Server: Webserv_42" << CRLF;
	header << "Date: " << getDateString() << CRLF;

	for (std::map<std::string, std::string>::const_iterator i = this->headers.begin(); i != this->headers.end(); i++)
	{
		header << i->first << ": " << i->second << CRLF;
	}
	if (!this->isCGI || status >= 300)
		header << CRLF;
	return header.str();
}

// ********************************************************************
// Setters
// ********************************************************************

void Response::addHeader(std::string key, std::string value)
{
	this->headers[key] = value;
}
void Response::addHeader(std::string key, unsigned long value)
{
	this->headers[key] = ulong_to_str(value);
}
void Response::addHeader(std::string key, long value)
{
	this->headers[key] = long_to_str(value);
}

void Response::setSender(int status, const std::string &content)
{
	std::string header = "";
	if (this->request.getVServer())
	{
		this->addHeader("Content-Length", content.size());
		header = this->generateHeader(status);
	}
	if (this->sender)
		delete this->sender;
	this->sender = new Sender(this->request.getClientFd(), header + content);
}

void Response::setSender(int status, int resourceFd)
{
	std::string header = "";
	if (this->request.getVServer())
		header = this->generateHeader(status);
	if (this->sender)
		delete this->sender;
	this->sender = new Sender(this->request.getClientFd(), header, resourceFd);
}
