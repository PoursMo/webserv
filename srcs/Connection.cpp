#include "Connection.hpp"

Connection::Connection(int clientFd, const std::vector<VirtualServer *> &vServers, const Poller &poller)
	: request(clientFd, vServers, poller),
	  receiver(clientFd, request)
{
	this->updateTime();
}

void Connection::updateTime()
{
	this->lastEventTime = std::time(NULL);
}
