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
	Poller();
	~Poller();

	void add(int fd, uint32_t events);
	void del(int fd);
	void mod(int fd, uint32_t events);
	int poll();
	const struct epoll_event &getEvent(size_t index) const;
};

#endif