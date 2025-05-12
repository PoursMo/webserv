#include "config.hpp"
#include "VirtualServer.hpp"
#include "Poller.hpp"
#include "Receiver.hpp"

#include <iostream>
#include <fstream>
#include <algorithm>
#include <string.h>
#include <unistd.h>

VirtualServer *find_virtual_server_with_host_name(const std::map<int, std::vector<VirtualServer *> > &m, const std::string &name)
{
	for (std::map<int, std::vector<VirtualServer *> >::const_iterator i = m.begin(); i != m.end(); i++)
	{
		for (std::vector<VirtualServer *>::const_iterator j = i->second.begin(); j != i->second.end(); j++)
		{
			for (std::vector<std::string>::const_iterator k = (*j)->getServerNames().begin(); k != (*j)->getServerNames().end(); k++)
			{
				if (*k == name)
					return *j;
			}
		}
	}
	return 0;
}

bool is_server_fd(int fd, const std::map<int, std::vector<VirtualServer *> > &servers)
{
	for (std::map<int, std::vector<VirtualServer *> >::const_iterator i = servers.begin(); i != servers.end(); i++)
	{
		if (fd == i->first)
			return true;
	}
	return false;
}

void print_recved(const char *s)
{
	if (*s)
		std::cout << "< ";
	while (*s)
	{
		std::cout << *s;
		if (*s == '\n' && *(s + 1))
			std::cout << "< ";
		s++;
	}
	std::cout << std::endl;
}

void print_sended(const char *s)
{
	if (*s)
		std::cout << "> ";
	while (*s)
	{
		std::cout << *s;
		if (*s == '\n' && *(s + 1))
			std::cout << "> ";
		s++;
	}
	std::cout << std::endl;
}

void poll_loop(const std::map<int, std::vector<VirtualServer *> > &servers)
{
	Poller poller;
	for (std::map<int, std::vector<VirtualServer *> >::const_iterator i = servers.begin(); i != servers.end(); i++)
	{
		poller.add(i->first, EPOLLIN);
	}
	std::map<int, Receiver> connections;
	while (1)
	{
		std::cout << "Polling..." << std::endl; // debug
		int nb_ready = poller.poll();
		for (int i = 0; i < nb_ready; i++)
		{
			struct epoll_event event = poller.getEvent(i);
			if (is_server_fd(event.data.fd, servers))
			{
				struct sockaddr_in client_addr;
				int addrlen = sizeof(client_addr);
				int client_fd = accept(event.data.fd, (struct sockaddr *)&client_addr, (socklen_t *)&addrlen);
				// if (new_fd == -1)
				// 5xx error
				std::cout << "New connection on socket " << event.data.fd << ", created socket fd: " << client_fd << std::endl;
				connections[client_fd] = Receiver(client_fd);
				poller.add(client_fd, EPOLLIN);
			}
			else
			{
				if (event.events & EPOLLIN)
				{
					std::cout << "In operations on socket " << event.data.fd << ":" << std::endl;
					if (!connections.at(event.data.fd).receive())
						poller.mod(event.data.fd, EPOLLOUT);
				}
				else if (event.events & EPOLLOUT)
				{
					std::cout << "Out operations on socket " << event.data.fd << ":" << std::endl;
					const char *response = "HTTP/1.1 200 OK\n";			// craft response
					send(event.data.fd, response, strlen(response), 0); // can send less than expected
					print_sended(response);

					std::cout << "Terminating socket: " << event.data.fd << std::endl;
					connections.erase(event.data.fd);
					poller.del(event.data.fd); // only if done with I/O
					close(event.data.fd);	   // only if done with I/O
				}
			}
		}
	}
}

int main(int argc, char **argv)
{
	const char *config_path;
	if (argc > 1)
		config_path = argv[1];
	else
		config_path = "conf/default.json";
	std::ifstream file(config_path);
	if (!file)
	{
		std::cerr << config_path << ": Could not open file " << std::endl;
		return 1;
	}
	std::map<int, std::vector<VirtualServer *> > servers; // const VirtualServer * const ?
	try
	{
		std::cout << "\033[1m\033[31m" << "json:" << "\033[0m" << std::endl;
		ft_json::JsonValue json = ft_json::parse_json(file);
		std::cout << json << std::endl
				  << std::endl;
		std::cout << "\033[1m\033[31m" << "servers:" << "\033[0m" << std::endl;
		servers = create_servers(json);
		for (std::map<int, std::vector<VirtualServer *> >::iterator i = servers.begin(); i != servers.end(); i++)
		{
			std::cout << "Socket: " << i->first << std::endl;
			for (std::vector<VirtualServer *>::const_iterator j = i->second.begin(); j != i->second.end(); j++)
			{
				std::cout << *(*j);
			}
		}
		std::cout << std::endl;
	}
	catch (const std::exception &e)
	{
		std::cerr << e.what() << '\n';
		return 1;
	}

	std::cout << "\033[1m\033[31m" << "poll loop:" << "\033[0m" << std::endl;
	poll_loop(servers);

	// this is never reached
	for (std::map<int, std::vector<VirtualServer *> >::iterator i = servers.begin(); i != servers.end(); i++)
	{
		for (std::vector<VirtualServer *>::const_iterator j = i->second.begin(); j != i->second.end(); j++)
		{
			delete (*j);
		}
	}
}