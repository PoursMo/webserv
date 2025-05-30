#include "Connection.hpp"

Connection::Connection(Poller &poller, int clientFd, const std::vector<VirtualServer *> &vServers)
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
