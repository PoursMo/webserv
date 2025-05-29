#ifndef CONNECTION_HPP
#define CONNECTION_HPP

#include "webserv.hpp"
#include "Request.hpp"
#include "Response.hpp"

class VirtualServer;

struct Connection
{
	time_t lastEventTime;
	Request request;
	Response response;

	Connection(int clientFd, const std::vector<VirtualServer *> &vServers, const Poller &poller);
	void updateTime();
};

#endif
