#include "Connection.hpp"
#include "Logger.hpp"

Connection::Connection(Poller& poller, int clientFd, const std::vector<VirtualServer*>& vServers)
	: request(poller, *this, vServers),
	response(poller, *this)
{
	request.setInputFd(clientFd);
	this->updateTime();
}

void Connection::updateTime()
{
	this->lastEventTime = std::time(NULL);
}

void Connection::print(const std::string title)
{
	if (!title.empty())
		logger.log() << title << '\n';
	this->request.printIOHandler() << " -> ";
	this->response.printIOHandler() << std::endl;
}
