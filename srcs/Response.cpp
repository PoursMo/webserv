#include "Response.hpp"
#include "Request.hpp"
#include "LocationData.hpp"
#include "http_error.hpp"
#include "http_status.hpp"
#include "Sender.hpp"
#include "utils.hpp"
#include "VirtualServer.hpp"

Response::Response(const Request &request)
	: request(request),
	  sender(NULL)
{
}

Response::~Response()
{
	if (this->sender)
		delete this->sender;
}

bool Response::send() const
{
	if (!this->sender)
		throw std::runtime_error("response->sender not defined !");
	return this->sender->handleSend();
}

// static void handleReturn(const std::pair<int, std::string> &returnPair)
// {
// 	// TODO
// 	if (returnPair.first != -1)
// 	{
// 		if (returnPair.first >= 400 && returnPair.first < 600)
// 			throw http_error("Returned error", returnPair.first);
// 		// < 400
// 	}
// }

bool Response::setFileSender(const std::string &path, int status)
{
	// header << "Last-Modified: " << getDateString(statBuffer.st_mtim.tv_sec) << CRLF;
	// header << "Content-Length: " << statBuffer.st_size << CRLF;
	int fd = open(path.c_str(), O_RDONLY);
	if (fd == -1)
	{
		std::cerr << "error page open: " << strerror(errno) << std::endl;
		return false;
	}
	std::string header = this->generateHeader(status);
	this->sender = new Sender(this->request.getClientFd(), header, fd);
	std::cout << "SENDER PTR" << this->sender << std::endl;
	return true;
}

void Response::setErrorSender(int status)
{
	const VirtualServer *vServer = this->request.getVServer();

	if (vServer && vServer->getErrorPages().count(status))
	{
		const std::string &path = vServer->getErrorPages().at(status);
		this->setStat(path);
		if (this->setFileSender(path, status))
			return;
	}
	std::string body = generateErrorBody(status);
	this->addHeader("Content-Type", "text/body");
	if (!body.empty())
		this->addHeader("Content-Length", body.size());
	std::string header = this->generateHeader(status);
	this->sender = new Sender(this->request.getClientFd(), header + body);
}

std::string Response::generateHeader(int status) const
{
	std::stringstream header;
	const std::string &statusName = http_status::get(status);

	header << "HTTP/1.1 " << status << " " << statusName << CRLF;
	header << "Server: Webserv_42" << CRLF;
	header << "Date: " << getDateString() << CRLF;

	for (std::map<std::string, std::string>::const_iterator i = headers.begin(); i != headers.begin(); i++)
	{
		header << i->first << ": " << i->second << CRLF;
	}
	header << CRLF;
	return header.str();
}

// ********************************************************************
// Setters
// ********************************************************************

void Response::setStat(const std::string &path)
{
	if (stat(path.c_str(), &this->statBuffer) == -1)
		throw http_error("stat: " + std::string(strerror(errno)), 500);
}

void Response::addHeader(std::string key, std::string value)
{
	headers[key] = value;
}
void Response::addHeader(std::string key, unsigned long value)
{
	headers[key] = ulong_to_str(value);
}
void Response::addHeader(std::string key, long value)
{
	headers[key] = long_to_str(value);
}
