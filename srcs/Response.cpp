#include "Response.hpp"
#include "Connection.hpp"
#include "LocationData.hpp"
#include "http_error.hpp"
#include "http_status.hpp"
#include "utils.hpp"
#include "autoindex.hpp"
#include "VirtualServer.hpp"
#include "Logger.hpp"
#include "Poller.hpp"

Response::Response(Poller& poller, Connection& connection)
	: AIOHandler(poller, connection),
	cgiPid(0)
{
}

Response::~Response()
{
	if (this->cgiPid)
	{
		kill(this->cgiPid, SIGKILL);
		waitpid(this->cgiPid, NULL, 0);
	}
	if (this->inputFd != -1)
		close(this->inputFd);
}

// ********************************************************************
// AInputHandler
// ********************************************************************

bool Response::isInputEnd()
{
	return this->inputFd == -1 || this->bytesInput == 0;
}

void Response::onInputEnd()
{
	this->unsubscribeInputFd();
}

bool Response::parseLine(char* lstart, char* lend)
{
	if (lstart == NULL || lend == NULL || *lend != '\n')
		return false;
	if (AIOHandler::isEmptyline(lstart, lend))
	{
		if (this->headers.size() < 1)
		{
			this->setError(502);
			return false;
		}
		int status = 200;
		try
		{
			status = extract_status_code(headers.at("Status"));
			headers.erase("Status");
		}
		catch (const std::exception& e)
		{
			// std::cerr << e.what() << '\n'; // remove ?
		}
		this->set(status);
		return false;
	}
	try
	{
		this->parseHeaderLine(lstart, lend);
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << '\n';
	}
	return true;
}

void Response::onUpdateBodyBytes()
{
}

ssize_t Response::handleInputSysCall(void* buf, size_t len)
{
	logger.log() << "Response: reading " << len << " bytes" << std::endl;
	ssize_t bytesRead = read(this->inputFd, buf, len);
	if (bytesRead == -1)
		throw http_error("read: " + std::string(strerror(errno)), 500);
	logger.log() << "Response: read " << bytesRead << " bytes" << std::endl;
	return bytesRead;
}

void Response::onHeaderBufferCreation()
{
}

// ********************************************************************
// AOutputHandler
// ********************************************************************

ssize_t Response::handleOutputSysCall(const void* buf, size_t len)
{
	logger.log() << "Response: sending " << len << " bytes" << std::endl;
	ssize_t bytesSent = send(this->outputFd, buf, len, MSG_NOSIGNAL);
	if (bytesSent == -1)
		throw std::runtime_error("send: " + std::string(strerror(errno)));
	logger.log() << "Response: bytesSent: " << bytesSent << std::endl;
	return bytesSent;
}

bool Response::isOutputEnd()
{
	return this->stringContent.empty() && this->buffers.empty() && this->isInputEnd();
}

void Response::onOutputEnd()
{
	this->poller.terminateConnection(this->outputFd);
}

// ********************************************************************
// Logic
// ********************************************************************

void Response::handleReturn(const std::pair<int, std::string>& returnPair)
{
	if (returnPair.first == 301 || returnPair.first == 302 || returnPair.first == 303 || returnPair.first == 307)
	{
		this->addHeader("Location", returnPair.second);
		this->setError(returnPair.first);
	}
	else
	{
		this->addHeader("Content-Type", "text/plain");
		this->set(returnPair.first, returnPair.second);
	}
}

std::string Response::getIndexPage(const std::string& path)
{
	const std::vector<std::string>& indexes = this->connection.request.getLocation()->getIndexes();
	for (std::vector<std::string>::const_iterator i = indexes.begin(); i != indexes.end(); i++)
	{
		std::string fullPath = path + *i;
		if (access(fullPath.c_str(), R_OK) != -1)
			return fullPath;
	}
	return "";
}

void Response::handleFile(const std::string& path, int status)
{
	CgiHandler cgi(this->connection.request);
	if (cgi.isCgiResource())
	{
		this->headers.erase("Content-Length");
		this->cgiPid = cgi.cgiExecution();
		this->connection.print("Response: bind CGI process");
		this->connection.request.subscribeOutputFd(cgi.getFdIn());
		this->subscribeInputFd(cgi.getFdOut());
		this->connection.print();
	}
	else
	{
		if (this->connection.request.getMethod() != GET)
			throw http_error("Only GET method can be handled without CGI", 405);
		int fileFd = open(path.c_str(), O_RDONLY);
		if (fileFd == -1)
			throw http_error("open: " + std::string(strerror(errno)), 500);
		this->connection.print("Response: set fileFd on input");
		this->isReadingHeader = false;
		this->subscribeInputFd(fileFd);
		this->set(status);
	}
}

void Response::handlePath(const std::string& path, int status)
{
	logger.log() << "Response: handling path: " << path << std::endl;
	if (access(path.c_str(), R_OK) == -1)
		throw http_error("access: " + path + ": " + std::string(strerror(errno)), 404);
	struct stat statBuffer;
	if (stat(path.c_str(), &statBuffer) == -1)
		throw http_error("stat: " + std::string(strerror(errno)), 500);
	if (S_ISDIR(statBuffer.st_mode))
	{
		if (path[path.size() - 1] != '/')
		{
			std::string loc = path.substr(this->connection.request.getLocation()->getRoot().size());
			this->addHeader("Location", loc + '/');
			this->setError(301);
			return;
		}
		std::string indexPagePath = getIndexPage(path);
		if (indexPagePath.empty())
		{
			if (!this->connection.request.getLocation()->getAutoIndex())
				throw http_error("Target is dir without index pages and autoindex is disabled", 404);
			if (this->connection.request.getMethod() != GET)
				throw http_error("Only GET method is handled for autoindex", 405);
			this->set(200, getAutoIndexHtml(path, this->connection.request.getLocation()->getRoot()));
		}
		else
		{
			this->handlePath(indexPagePath, status);
		}
	}
	else if (S_ISREG(statBuffer.st_mode))
	{
		this->addHeader("Content-Length", statBuffer.st_size);
		this->handleFile(path, status);
	}
	else
		throw http_error("Target is neither a directory nor a regular file", 422);
}

void Response::setError(int status)
{
	const VirtualServer* vServer = this->connection.request.getVServer();
	if (vServer && this->connection.request.getLocation())
	{
		try
		{
			const std::string& path = vServer->getErrorPages().at(status);
			this->handlePath(path, status);
			return;
		}
		catch (const std::exception& e)
		{
			// std::cerr << e.what() << '\n'; // remove ?
		}
	}
	std::string body = generateErrorBody(status);
	this->set(status, body);
}

void Response::set(int status, const std::string& body)
{
	this->addHeader("Content-Length", body.size());
	this->stringContent = this->generateHeader(status) + body;
	this->connection.request.unsubscribeInputFd();
	this->connection.request.unsubscribeOutputFd();
	this->unsubscribeInputFd();
	this->subscribeOutputFd(this->connection.clientFd);
	this->connection.print();
}

void Response::set(int status)
{
	this->stringContent = this->generateHeader(status);
	this->subscribeOutputFd(this->connection.clientFd);
	this->connection.print();
}

std::string Response::generateHeader(int status) const
{
	std::stringstream header;
	const std::string& statusName = http_status::get(status);

	header << "HTTP/1.1 " << status << " " << statusName << CRLF;
	header << "Server: Webserv_42" << CRLF;
	header << "Date: " << getDateString() << CRLF;

	for (std::map<std::string, std::string>::const_iterator i = this->headers.begin(); i != this->headers.end(); i++)
	{
		header << i->first << ": " << i->second << CRLF;
	}
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
	this->addHeader(key, ulong_to_str(value));
}

void Response::addHeader(std::string key, long value)
{
	this->addHeader(key, long_to_str(value));
}
