#include "Poll.hpp"

#include <unistd.h>
#include <sys/epoll.h>

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
	int fd;

public:
	Poll()
	{
		int fd = epoll_create(1); // adjust size ?
		if (fd == -1)
			throw std::runtime_error(strerror(errno));
	}
	~Poll()
	{
		close(fd);
	}
	void add(int new_fd)
	{
		struct epoll_event event;
		event.events = EPOLLIN;
		event.data.fd = new_fd;
		if (epoll_ctl(fd, EPOLL_CTL_ADD, new_fd, &event) == -1)
			throw std::runtime_error(strerror(errno));
	}
	// add fd
	// remove fd
};

void poll_loop()
{

	struct epoll_event events[10]; // adjust 10 value

	while (1)
	{
		int nready = epoll_wait(epoll_fd, events, 10, -1); // adjust 10 value
		if (nready < -1)
		{
			throw std::runtime_error(strerror(errno));
		}

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
					if (epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, 0) == -1)
					{
						throw std::runtime_error(strerror(errno));
					}
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

	close(epoll_fd);

	std::cout << "Server initialized and listening on " << address << ":" << port << std::endl;
}