#ifndef POLLER_HPP
#define POLLER_HPP

#include "webserv.hpp"
#include "Request.hpp"
#include "Receiver.hpp"

class VirtualServer;

struct Connection
{
	Request request;
	Receiver receiver;

	Connection(int clientFd, const std::vector<VirtualServer *> &vServers);
};

class Poller
{
private:
	int epollFd;
	std::vector<struct epoll_event> events;
	const std::map<int, std::vector<VirtualServer *> > &servers;
	std::map<int, Connection *> connections;

	void add(int fd, uint32_t events);
	void del(int fd);
	int waitEvents();

	void terminateConnection(int fd);
	bool isServerFd(int fd);
	void handleNewConnection(int fd);
	void handleInput(int fd);
	void handleOutput(int fd);

public:
	Poller(const std::map<int, std::vector<VirtualServer *> > &servers);
	void mod(int fd, uint32_t events);
	~Poller();

	void loop();
};

#endif
