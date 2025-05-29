#include "Connection.hpp"

Connection::Connection(int clientFd, const std::vector<VirtualServer *> &vServers, const Poller &poller)
	: request(response, vServers, poller),
	  response(request, poller)
{
	request.setInputFd(clientFd);
	this->updateTime();
}

void Connection::updateTime()
{
	this->lastEventTime = std::time(NULL);
}
