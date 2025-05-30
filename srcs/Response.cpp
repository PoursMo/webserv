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

Response::Response(Poller &poller, Connection &connection)
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
}

// ********************************************************************
// AInputHandler
// ********************************************************************

bool Response::isInputEnd()
{
	return this->bytesInput == 0;
}

void Response::onInputEnd()
{
	logger.log() << "Response ON_INPUT_END event !" << std::endl;
	close(this->inputFd);
}

bool Response::parseLine(char *lstart, char *lend)
{
	if (lstart == NULL || lend == NULL || *lend != '\n')
		return false;
	if (AIOHandler::isEmptyline(lstart, lend))
	{
		// TODO: if no headers : 502
		int status = 200;
		try
		{
			status = extract_status_code(headers.at("Status"));
			headers.erase("Status");
		}
		catch(const std::exception& e)
		{
			// std::cerr << e.what() << '\n'; // remove ?
		}
		set(status);
		return false;
	}
	try
	{
		this->parseHeaderLine(lstart, lend);
	}
	catch(const std::exception& e)
	{
		std::cerr << e.what() << '\n';
	}
	return true;
}

void Response::onUpdateBodyBytes()
{
}

ssize_t Response::handleInputSysCall(void *buf, size_t len)
{
	ssize_t bytesRead = read(this->inputFd, buf, len);
	if (bytesRead == -1)
		throw http_error("read: " + std::string(strerror(errno)), 500);
	return bytesRead;
}

void Response::onHeaderBufferCreation()
{
}

// ********************************************************************
// AOutputHandler
// ********************************************************************

ssize_t Response::handleOutputSysCall(const void *buf, size_t len)
{
	ssize_t bytesSent = send(this->outputFd, buf, len, MSG_NOSIGNAL);
	if (bytesSent == -1)
		throw std::runtime_error("send: " + std::string(strerror(errno)));
	logger.log() << "Response: bytesSent: " << bytesSent << std::endl;
	return bytesSent;
}

bool Response::isOutputEnd()
{
	return this->isStringContentSent && this->buffers.empty() && this->isInputEnd();
}

void Response::onOutputEnd()
{
	logger.log() << "Response ON_OUTPUT_END event !" << std::endl;
	this->poller.terminateConnection(this->outputFd);
}

// ********************************************************************
// Logic
// ********************************************************************

void Response::handleReturn(const std::pair<int, std::string> &returnPair)
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

std::string Response::getIndexPage(const std::string &path)
{
	const std::vector<std::string> &indexes = this->connection.request.getLocation()->getIndexes();
	for (std::vector<std::string>::const_iterator i = indexes.begin(); i != indexes.end(); i++)
	{
		std::string fullPath = path + *i;
		if (access(fullPath.c_str(), R_OK) != -1)
			return fullPath;
	}
	return "";
}

void Response::handleFile(const std::string &path)
{
	CgiHandler cgi(this->connection.request);
	int fdIn;
	int fdOut;
	if (cgi.isCgiResource())
	{
		this->cgiPid = cgi.cgiExecution();
		fdIn = cgi.getFdIn();
		fdOut = cgi.getFdOut();
		this->setInputFd(fdOut);
	}
	else
	{
		if (this->connection.request.getMethod() != GET)
			throw http_error("Only GET method can be handled without CGI", 405);
		fdIn = open("/dev/null", O_CREAT | O_WRONLY | O_TRUNC, 0644); // opti: open once
		fdOut = open(path.c_str(), O_RDONLY);
		if (fdIn == -1 || fdOut == -1)
			throw http_error("open: " + std::string(strerror(errno)), 500);
		this->isReadingHeader = false;
		this->inputFd = fdOut;
		while (this->bytesInput != 0)
			this->handleInput();
		this->set(200);
	}
	this->connection.request.setOutputFd(fdIn);

}

void Response::handlePath(const std::string &path)
{
	logger.log() << "Response: accessing path: " << path << std::endl;
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
				throw http_error("This target is a dir not have index pages and autoindex is disabled", 404);
			if (this->connection.request.getMethod() != GET)
				throw http_error("Only GET method is handled for autoindex", 405);
			this->set(200, getAutoIndexHtml(path, this->connection.request.getLocation()->getRoot()));
		}
		else
		{
			this->handlePath(indexPagePath);
		}
	}
	else if (S_ISREG(statBuffer.st_mode))
	{
		this->handleFile(path);
	}
	else
		throw http_error("Target is neither a directory nor a regular file", 422);
}

void Response::setError(int status)
{
	const VirtualServer *vServer = this->connection.request.getVServer();
	if (vServer && this->connection.request.getLocation())
	{
		try
		{
			const std::string &path = vServer->getErrorPages().at(status);
			this->handlePath(path);
			return;
		}
		catch(const std::exception& e)
		{
			std::cerr << e.what() << '\n'; // remove ?
		}
	}
	std::string body = generateErrorBody(status);
	if (this->connection.request.getVServer())
	{
		this->addHeader("Content-Type", "text/html");
		this->addHeader("Content-Length", body.size());
	}
	this->set(status, body);
}

void Response::set(int status, const std::string &body)
{
	this->stringContent = this->generateHeader(status) + body;
}

void Response::set(int status)
{
	this->stringContent = this->generateHeader(status);
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
	return header.str();
}

// ********************************************************************
// Setters
// ********************************************************************

void Response::addHeader(std::string key, std::string value)
{
	try
	{
		std::string &headerValue = this->headers.at(key);
		headerValue += ", " + value;
	}
	catch(const std::exception& e)
	{
		this->headers[key] = value;
	}
}

void Response::addHeader(std::string key, unsigned long value)
{
	this->addHeader(key, ulong_to_str(value));
}

void Response::addHeader(std::string key, long value)
{
	this->addHeader(key, long_to_str(value));
}
