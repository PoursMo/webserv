#ifndef POLLER_HPP
#define POLLER_HPP

#include "webserv.hpp"

class VirtualServer;
class Connection;
class AIOHandler;    

class Poller
{
private:
	std::vector<struct pollfd> pollFds;
	const std::map<int, std::vector<VirtualServer *> > &servers;
	std::map<int, Connection *> connections;
	
	int waitEvents(int timeout);
	
	
	void handleNewConnection(int fd);
	void handlePollin(int fd);
	void handlePollout(int fd);
	void timeoutTerminator(int &timeout);
	std::vector<struct pollfd>::iterator findPollFd(int fd);
	
	public:
	Poller(const std::map<int, std::vector<VirtualServer *> > &servers);
	std::map<int, AIOHandler *> ioHandlers;
	void add(int fd, short events);
	void del(int fd);
	~Poller();
	void loop();
	void closeAll() const;
public:
	bool isServerFd(int fd);
	void terminateConnection(int fd);
};

#endif
