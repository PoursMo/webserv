#include "Poller.hpp"
#include "http_error.hpp"
#include <unistd.h>

#include <stdexcept>
#include <string.h>
#include <errno.h>

#define WS_EPOLL_NB_EVENTS 512

/*
Suppose, the reader only consumed part of data from the buffer:
edge-trigerred mode: Epoll_wait will return only when a new event is enqueued with the epoll object.
level-triggered mode (default): Epoll_wait will return as long as the condition holds.
*/

Poller::Poller()
	: epollFd(epoll_create(1)),
	  events(WS_EPOLL_NB_EVENTS)
{
	if (epollFd == -1)
		throw std::runtime_error(strerror(errno));
}

Poller::~Poller()
{
	close(epollFd);
}

void Poller::add(int fd, uint32_t events) // edge trigerred ?
{
	struct epoll_event event;
	event.events = events;
	event.data.fd = fd;
	if (epoll_ctl(epollFd, EPOLL_CTL_ADD, fd, &event) == -1)
		throw std::runtime_error(strerror(errno)); // other error ? 5xx
}

void Poller::del(int fd)
{
	if (epoll_ctl(epollFd, EPOLL_CTL_DEL, fd, 0) == -1)
		throw std::runtime_error(strerror(errno)); // other error ? 5xx
}

void Poller::mod(int fd, uint32_t events)
{
	struct epoll_event event;
	event.events = events;
	event.data.fd = fd;
	if (epoll_ctl(epollFd, EPOLL_CTL_MOD, fd, &event) == -1)
		throw std::runtime_error(strerror(errno)); // other error ? 5xx
}

int Poller::poll()
{
	int nready = epoll_wait(epollFd, events.data(), events.size(), -1);
	if (nready == -1)
		throw std::runtime_error(strerror(errno)); // other error ? 5xx
	return nready;
}

const struct epoll_event &Poller::getEvent(size_t index) const
{
	if (index >= events.size())
		throw std::out_of_range("event index out of range");
	return events[index];
}
