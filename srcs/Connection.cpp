#include "Connection.hpp"

Connection::Connection(int clientFd, const std::vector<VirtualServer *> &vServers)
	: request(clientFd, vServers),
	  receiver(clientFd, request)
{
	this->updateTime();
}

void Connection::updateTime()
{
	this->lastEventTime = std::time(NULL);
}
