#ifndef POLLER_HPP
#define POLLER_HPP

#include <vector>
#include <sys/epoll.h>

class Poller
{
private:
	int epollFd;
	std::vector<struct epoll_event> events;

public:
	Poller(size_t nb_events = 512);
	~Poller();

	void add(int fd, uint32_t events = EPOLLIN); // edge trigerred ?
	void del(int fd);
	int poll();
	const struct epoll_event &getEvent(size_t index) const;
};

#endif