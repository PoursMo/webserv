#include "Poller.hpp"
#include "http_error.hpp"
#include "Request.hpp"
#include "Receiver.hpp"

Connection::Connection(int clientFd, const std::vector<VirtualServer *> &vServers)
	: creationTime(std::time(NULL)),
	  request(clientFd, vServers),
	  receiver(clientFd, request)
{
}

/*
edge-trigerred mode: Epoll_wait will return only when a new event is enqueued with the epoll object.
level-triggered mode (default): Epoll_wait will return as long as the condition holds.
*/

Poller::Poller(const std::map<int, std::vector<VirtualServer *> > &servers)
	: epollFd(epoll_create(1)),
	  events(WS_EPOLL_NB_EVENTS),
	  servers(servers)
{
	if (epollFd == -1)
		throw std::runtime_error(strerror(errno));
	for (std::map<int, std::vector<VirtualServer *> >::const_iterator i = servers.begin(); i != servers.end(); i++)
	{
		this->add(i->first, EPOLLIN);
	}
}

Poller::~Poller()
{
	close(epollFd);
}

void Poller::add(int fd, uint32_t events)
{
	struct epoll_event event;
	event.events = events;
	event.data.fd = fd;
	if (epoll_ctl(epollFd, EPOLL_CTL_ADD, fd, &event) == -1)
		throw std::runtime_error("epoll_ctl add: " + std::string(strerror(errno)));
}

void Poller::del(int fd)
{
	if (epoll_ctl(epollFd, EPOLL_CTL_DEL, fd, 0) == -1)
		throw std::runtime_error("epoll_ctl del: " + std::string(strerror(errno)));
}

void Poller::mod(int fd, uint32_t events)
{
	struct epoll_event event;
	event.events = events;
	event.data.fd = fd;
	if (epoll_ctl(epollFd, EPOLL_CTL_MOD, fd, &event) == -1)
		throw std::runtime_error("epoll_ctl mod: " + std::string(strerror(errno)));
}

int Poller::waitEvents(int timeout)
{
	int nready = epoll_wait(epollFd, events.data(), events.size(), timeout);
	if (nready == -1)
		throw std::runtime_error("epoll_wait: " + std::string(strerror(errno)));
	return nready;
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
	try
	{
		delete connections.at(fd);
		connections.erase(fd);
	}
	catch (...)
	{
	}
	try
	{
		this->del(fd);
	}
	catch (...)
	{
	}
	if (!isServerFd(fd))
		close(fd);
}

void Poller::handleNewConnection(int fd)
{
	struct sockaddr_in client_addr;
	int addrlen = sizeof(client_addr);
	int clientFd = accept(fd, (struct sockaddr *)&client_addr, (socklen_t *)&addrlen);
	if (clientFd == -1)
		throw std::runtime_error("accept: " + std::string(strerror(errno)));
	std::cout << "New connection on socket " << fd << ", created socket fd: " << clientFd << std::endl;
	connections[clientFd] = new Connection(clientFd, servers.at(fd));
	this->add(clientFd, EPOLLIN);
}

void Poller::handleInput(int fd)
{
	std::cout << "In operations on socket " << fd << ":" << std::endl;
	try
	{
		if (!connections.at(fd)->receiver.receive())
			this->mod(fd, EPOLLOUT);
	}
	catch (const http_error &e)
	{
		std::cerr << e.what() << '\n';
		connections.at(fd)->request.response.setErrorSender(e.getStatusCode());
		this->mod(fd, EPOLLOUT);
	}
}

void Poller::handleOutput(int fd)
{
	std::cout << "Out operations on socket " << fd << ":" << std::endl;
	if (!connections.at(fd)->request.response.send())
	{
		std::cout << "Terminating socket: " << fd << std::endl;
		terminateConnection(fd);
	}
}

void Poller::timeoutTerminator(int *timeout)
{
	std::vector<int> timedOuts;
	time_t currentTime = std::time(NULL);
	time_t highestElapsedTime = WS_CONNECTION_TIMEOUT_TIMER;
	for (std::map<int, Connection *>::iterator i = connections.begin(); i != connections.end(); i++)
	{
		time_t elapsedTime = currentTime - i->second->creationTime;
		if (elapsedTime >= WS_CONNECTION_TIMEOUT_TIMER)
			timedOuts.push_back(i->first);
		if (elapsedTime > highestElapsedTime)
			highestElapsedTime = elapsedTime;
	}
	*timeout = WS_CONNECTION_TIMEOUT_TIMER - highestElapsedTime;
	for (std::vector<int>::iterator i = timedOuts.begin(); i != timedOuts.end(); i++)
	{
		std::cout << "Timeout: socket " << *i << ", connection terminated after timeout of " << WS_CONNECTION_TIMEOUT_TIMER << " seconds." << std::endl;
		terminateConnection(*i);
	}
}

void Poller::loop()
{
	while (1)
	{
		int timeout = WS_CONNECTION_TIMEOUT_TIMER;
		if (!connections.empty())
			timeoutTerminator(&timeout);
		//std::cout << "Polling..." << std::endl; // debug
		int nb_ready = this->waitEvents(timeout * 1000);
		for (int i = 0; i < nb_ready; i++)
		{
			struct epoll_event event = events.at(i);
			try
			{
				if (isServerFd(event.data.fd))
				{
					handleNewConnection(event.data.fd);
				}
				else
				{
					if (event.events & EPOLLIN)
					{
						handleInput(event.data.fd);
					}
					else if (event.events & EPOLLOUT)
					{
						handleOutput(event.data.fd);
					}
				}
			}
			catch (const std::exception &e)
			{
				std::cerr << e.what() << '\n';
				terminateConnection(event.data.fd);
			}
		}
	}
}