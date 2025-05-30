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

	Connection(Poller &poller, int clientFd, const std::vector<VirtualServer *> &vServers);
	void updateTime();
};

#endif
