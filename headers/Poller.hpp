#ifndef POLLER_HPP
#define POLLER_HPP

#include "webserv.hpp"

class VirtualServer;
class Connection;
class AIOHandler;

class Poller
{
private:
	int epollFd;
	std::vector<struct epoll_event> events;
	const std::map<int, std::vector<VirtualServer *> > &servers;
	std::map<int, Connection *> connections;
	

	int waitEvents(int timeout);
	
	bool isServerFd(int fd);
	void handleNewConnection(int fd);
	void handlePollin(int fd);
	void handlePollout(int fd);
	void timeoutTerminator(int &timeout);
	
	public:
	Poller(const std::map<int, std::vector<VirtualServer *> > &servers);
	std::map<int, AIOHandler *> ioHandlers;
	void mod(int fd, uint32_t events);
	void add(int fd, uint32_t events);
	void del(int fd);
	~Poller();

	void terminateConnection(int fd);
	void loop();
	void closeAll() const;
};

#endif
