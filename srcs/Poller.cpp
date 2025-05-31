#include "Poller.hpp"
#include "http_error.hpp"
#include "Connection.hpp"
#include "VirtualServer.hpp"
#include "Logger.hpp"
#include "utils.hpp"

/*
edge-trigerred mode: Epoll_wait will return only when a new event is enqueued with the epoll object.
level-triggered mode (default): Epoll_wait will return as long as the condition holds.
*/

Poller::Poller(const std::map<int, std::vector<VirtualServer *> > &servers)
	: pollFds(),
	  servers(servers)
{
	for (std::map<int, std::vector<VirtualServer *> >::const_iterator i = servers.begin(); i != servers.end(); i++)
	{
		this->add(i->first, POLLIN);
	}
}

Poller::~Poller()
{
}

void Poller::add(int fd, short events)
{
	logger.log() << "ADD FD: " << int_to_str(fd) << " MODE=" << int_to_str(events) << std::endl;
	try
	{
		std::vector<struct pollfd>::iterator i = findPollFd(fd);
		i->events = events;
	}
	catch(const std::exception& e)
	{
		this->pollFds.push_back((struct pollfd){
			.fd = fd,
			.events = events,
			.revents = 0
		});
	}
}

void Poller::del(int fd)
{
	this->pollFds.erase(this->findPollFd(fd));
}

int Poller::waitEvents(int timeout)
{
	int nready = poll(pollFds.data(), pollFds.size(), timeout);
	if (nready == -1)
		throw std::runtime_error("poll: " + std::string(strerror(errno)));
	return nready;
}

std::vector<struct pollfd>::iterator Poller::findPollFd(int fd)
{
	for (std::vector<struct pollfd>::iterator i = this->pollFds.begin(); i != this->pollFds.end(); i++)
	{
		if (i->fd == fd)
			return i;
	}
	throw std::runtime_error("expected fd in pollFds: " + int_to_str(fd));
}

bool Poller::isServerFd(int fd)
{
	for (std::map<int, std::vector<VirtualServer *> >::const_iterator i = servers.begin(); i != servers.end(); i++)
	{
		if (fd == i->first)
			return true;
	}
	return false;
}

void Poller::terminateConnection(int fd)
{
	logger.log() << "Terminating socket: " << fd << std::endl;
	try
	{
		delete connections.at(fd);
		connections.erase(fd);
	}
	catch (...)
	{
	}
}

void Poller::handleNewConnection(int fd)
{
	struct sockaddr_in client_addr;
	int addrlen = sizeof(client_addr);
	int clientFd = accept(fd, (struct sockaddr *)&client_addr, (socklen_t *)&addrlen);
	if (clientFd == -1)
		throw std::runtime_error("accept: " + std::string(strerror(errno)));
	logger.log() << "New connection on socket " << fd << ", created socket fd: " << clientFd << std::endl;
	connections[clientFd] = new Connection(*this, clientFd, servers.at(fd));
}

void Poller::handlePollin(int fd)
{
	logger.log() << "POLLIN: " << int_to_str(fd) << std::endl;
	if (connections.count(fd))
		connections.at(fd)->updateTime();
	logger.log() << "In operations on socket " << fd << ":" << std::endl;
	try
	{
		ioHandlers.at(fd)->handleInput();
		// dans handleInput
		// if (!ioHandlers.at(fd). == 0)
		// 	this->mod(fd, EPOLLET);
		// else
		// 	this->mod(fd, POLLIN);
	}
	catch (const http_error &e)
	{
		std::cerr << e.what() << '\n';
		connections.at(fd)->response.sendError(e.getStatusCode());
		// this->mod(fd, POLLOUT);
	}
}

void Poller::handlePollout(int fd)
{
	logger.log() << "POLLOUT: " << int_to_str(fd) << std::endl;
	if (connections.count(fd))
		connections.at(fd)->updateTime();
	logger.log() << "Out operations on socket " << fd << ":" << std::endl;
	ioHandlers.at(fd)->handleOutput();
}

void Poller::timeoutTerminator(int &timeout)
{
	std::vector<int> timedOuts;
	time_t currentTime = std::time(NULL);
	time_t highestElapsedTime = 0;
	for (std::map<int, Connection *>::iterator i = connections.begin(); i != connections.end(); i++)
	{
		time_t elapsedTime = currentTime - i->second->lastEventTime;
		if (elapsedTime >= WS_CONNECTION_TIMEOUT_TIMER)
			timedOuts.push_back(i->first);
		else if (elapsedTime > highestElapsedTime)
			highestElapsedTime = elapsedTime;
	}
	timeout = WS_CONNECTION_TIMEOUT_TIMER - highestElapsedTime;
	for (std::vector<int>::iterator i = timedOuts.begin(); i != timedOuts.end(); i++)
	{
		logger.log() << "Timeout: socket " << *i << ", connection terminated after timeout of " << WS_CONNECTION_TIMEOUT_TIMER << " seconds." << std::endl;
		terminateConnection(*i);
	}
}

void Poller::loop()
{
	while (1)
	{
		int timeout = WS_CONNECTION_TIMEOUT_TIMER;
		if (!connections.empty())
			timeoutTerminator(timeout);
		logger.log() << "Polling..." << std::endl; // debug
		this->waitEvents(timeout * 1000);
		std::vector<struct pollfd> fds = this->pollFds;
		for (std::vector<struct pollfd>::const_iterator i = fds.begin(); i != fds.end(); i++)
		{
			try
			{
				if (i->revents & POLLIN)
				{
					if (isServerFd(i->fd))
						handleNewConnection(i->fd);
					else
						this->handlePollin(i->fd);
				}
				else if (i->revents & POLLOUT)
					this->handlePollout(i->fd);
			}
			catch (const std::runtime_error &e)
			{
				std::cerr << e.what() << '\n';
				terminateConnection(i->fd);
			}
			catch (const child_accident &e)
			{
				std::cerr << e.what() << '\n';
				return;
			}
			// i->revents = 0;
		}
	}
}

void Poller::closeAll() const
{
	for (std::map<int, Connection *>::const_iterator i = connections.begin(); i != connections.end(); i++)
	{
		close(i->first);
	}
	for (std::map<int, std::vector<VirtualServer *> >::const_iterator i = servers.begin(); i != servers.end(); i++)
	{
		close(i->first);
	}
}
