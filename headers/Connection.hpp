#ifndef CONNECTION_HPP
#define CONNECTION_HPP

#include "webserv.hpp"
#include "Receiver.hpp"
#include "Request.hpp"

class VirtualServer;

struct Connection
{
	time_t lastEventTime;
	Request request;
	Receiver receiver;

	Connection(int clientFd, const std::vector<VirtualServer *> &vServers, const Poller &poller);
	void updateTime();
};

#endif
