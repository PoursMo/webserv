#include "Poll.hpp"

#include <unistd.h>
#include <sys/epoll.h>

#include <vector>
#include <stdexcept>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>

/*
level-triggered mode: Only receive events when the state of the watched file descriptors change
edge-trigerred mode: Continue to receive events until the underlying file descriptor is no longer in a ready state
*/

class Poll
{
private:
	int pollFd;
	std::vector<struct epoll_event> events;

public:
	Poll()
	{
		int pollFd = epoll_create(1); // adjust size ?
		if (pollFd == -1)
			throw std::runtime_error(strerror(errno));
	}
	~Poll()
	{
		close(pollFd);
	}

	void add(int fd)
	{
		struct epoll_event event;
		event.events = EPOLLIN;
		event.data.fd = fd;
		if (epoll_ctl(pollFd, EPOLL_CTL_ADD, fd, &event) == -1)
			throw std::runtime_error(strerror(errno)); // other error ?
	}
	void del(int fd)
	{
		if (epoll_ctl(pollFd, EPOLL_CTL_DEL, fd, 0) == -1)
			throw std::runtime_error(strerror(errno)); // other error ?
	}
	int wait()
	{
		int nready = epoll_wait(pollFd, events.data(), std::min(events.size(), (size_t)10), -1); // change 10 ?
		if (nready == -1)
			throw std::runtime_error(strerror(errno)); // other error ?
		return nready;
	}
};

void poll_loop()
{
	// create poll
	while (1)
	{
		// wait

		for (int i = 0; i < nready; i++)
		{
			int fd = events[i].data.fd;
			if (fd == socketFd)
			{
				int addrlen = sizeof(client_addr);
				int new_fd = accept(socketFd, (struct sockaddr *)&client_addr, (socklen_t *)&client_addr);
				if (new_fd == -1)
					// 5xx error

					std::cout << "New connection, socket fd: " << new_fd << std::endl;

				event.events = EPOLLIN;
				event.data.fd = new_fd;
				if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, new_fd, &event) == -1)
				// 5xx error
			}
			else
			{
				char buffer[clientMaxBodySize];
				if (recv(fd, buffer, clientMaxBodySize, 0) == 0)
				{
					std::cout << "Client with fd: " << fd << " disconnected" << std::endl;
					// remove
					close(fd);
				}
				else
				{
					const char *response = "test";			 // craft response
					send(fd, response, strlen(response), 0); // can send less than expected
					if (epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, 0) == -1)
					{
						throw std::runtime_error(strerror(errno));
					}
					close(fd);
				}
			}
		}
	}
}