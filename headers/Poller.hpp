#ifndef POLLER_HPP
#define POLLER_HPP

#include "webserv.hpp"

class VirtualServer;
class Connection;

class Poller
{
private:
	int epollFd;
	std::vector<struct epoll_event> events;
	const std::map<int, std::vector<VirtualServer *> > &servers;
	std::map<int, Connection *> connections;

	void add(int fd, uint32_t events);
	void del(int fd);
	int waitEvents(int timeout);

	void terminateConnection(int fd);
	bool isServerFd(int fd);
	void handleNewConnection(int fd);
	void handleInput(int fd);
	void handleOutput(int fd);
	void timeoutTerminator(int &timeout);

public:
	Poller(const std::map<int, std::vector<VirtualServer *> > &servers);
	void mod(int fd, uint32_t events);
	~Poller();

	void loop();
	void closeAll() const;
};

#endif
