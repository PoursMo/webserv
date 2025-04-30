#include "config.hpp"
#include "Server.hpp"

#include <iostream>
#include <fstream>

// void temp()
// {
// 	int epoll_fd = epoll_create(10); // adjust 10 value
// 	if (epoll_fd == -1)
// 	{
// 		// close(socketFd);
// 		throw std::runtime_error(strerror(errno));
// 	}

// 	struct epoll_event event;
// 	event.events = EPOLLIN;
// 	event.data.fd = socketFd;
// 	if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, socketFd, &event) == -1)
// 	{
// 		// close(socketFd);
// 		throw std::runtime_error(strerror(errno));
// 	}

// 	struct epoll_event events[10]; // adjust 10 value

// 	while (1)
// 	{
// 		int nready = epoll_wait(epoll_fd, events, 10, -1); // adjust 10 value
// 		if (nready < -1)
// 		{
// 			throw std::runtime_error(strerror(errno));
// 		}

// 		for (int i = 0; i < nready; i++)
// 		{
// 			int fd = events[i].data.fd;
// 			if (fd == socketFd)
// 			{
// 				int addrlen = sizeof(address);
// 				int sock = accept(socketFd, (struct sockaddr *)&address, (socklen_t *)&addrlen);
// 				if (sock < 0)
// 				{
// 					throw std::runtime_error(strerror(errno)); // not throw
// 				}

// 				std::cout << "New connection, socket fd: " << sock << std::endl;

// 				event.events = EPOLLIN;
// 				event.data.fd = sock;
// 				if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, sock, &event) == -1)
// 				{
// 					// close(socketFd);
// 					throw std::runtime_error(strerror(errno));
// 				}
// 			}
// 			else
// 			{
// 				char buffer[clientMaxBodySize];
// 				if (recv(fd, buffer, clientMaxBodySize, 0) == 0)
// 				{
// 					std::cout << "Client with fd: " << fd << " disconnected" << std::endl;
// 					if (epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, 0) == -1)
// 					{
// 						throw std::runtime_error(strerror(errno));
// 					}
// 					close(fd);
// 				}
// 				else
// 				{
// 					const char *response = "test"; // craft response
// 					send(fd, response, strlen(response), 0);
// 					if (epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, 0) == -1)
// 					{
// 						throw std::runtime_error(strerror(errno));
// 					}
// 					close(fd);
// 				}
// 			}
// 		}
// 	}

// 	close(epoll_fd);

// 	std::cout << "Server initialized and listening on " << address << ":" << port << std::endl;
// }

int main(int argc, char **argv)
{
	const char *config_path;
	if (argc > 1)
		config_path = argv[1];
	else
		config_path = "conf/conf.json"; // create
	std::ifstream file(config_path);
	if (!file)
	{
		std::cerr << config_path << ": Could not open file " << std::endl;
		return 1;
	}

	ft_json::JsonValue json;
	try
	{
		std::cout << "\033[1m\033[31m" << "json:" << "\033[0m" << std::endl;
		json = ft_json::parse_json(file);
		std::cout << json << std::endl
				  << std::endl;
	}
	catch (const std::exception &e)
	{
		std::cerr << e.what() << '\n';
	}

	std::vector<Server *> servers;
	try
	{
		std::cout << "\033[1m\033[31m" << "servers:" << "\033[0m" << std::endl;
		servers = create_servers(json);
		for (std::vector<Server *>::iterator i = servers.begin(); i != servers.end(); i++)
		{
			std::cout << *(*i);
		}
		std::cout << std::endl;
	}
	catch (const std::exception &e)
	{
		std::cerr << e.what() << '\n';
	}

	// poll loop

	for (std::vector<Server *>::iterator i = servers.begin(); i != servers.end(); i++)
	{
		delete *i;
	}
}