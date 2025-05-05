#include "config.hpp"
#include "VirtualServer.hpp"
#include "Poller.hpp"

#include <iostream>
#include <fstream>
#include <algorithm>
#include <string.h>
#include <unistd.h>

VirtualServer *find_virtual_server_with_host_name(const std::map<int, std::vector<VirtualServer *> > &m, const std::string &name)
{
	for (std::map<int, std::vector<VirtualServer *> >::const_iterator i = m.begin(); i != m.end(); i++)
	{
		for (std::vector<VirtualServer *>::const_iterator j = (*i).second.begin(); j != (*i).second.end(); j++)
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
		if (fd == (*i).first)
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
		poller.add((*i).first);
	}
	while (1)
	{
		int nb_ready = poller.poll();
		for (int i = 0; i < nb_ready; i++)
		{
			int fd = poller.getEvent(i).data.fd;
			if (is_server_fd(fd, servers))
			{
				struct sockaddr_in client_addr;
				int addrlen = sizeof(client_addr);
				int client_fd = accept(fd, (struct sockaddr *)&client_addr, (socklen_t *)&addrlen);
				// if (new_fd == -1)
				// 5xx error
				std::cout << "New connection, socket fd: " << client_fd << std::endl;
				poller.add(client_fd);
			}
			else
			{
				std::cout << "I/O operations on socket fd: " << fd << ":" << std::endl;
				const char *response = "test"; // craft response
				char buffer[10000];
				if (recv(fd, buffer, 10000, 0) == 0)
				{
					std::cout << "Client with fd: " << fd << " disconnected" << std::endl;
				}
				else
				{
					send(fd, response, strlen(response), 0); // can send less than expected
				}
				print_recved(buffer);
				print_sended(response);
				poller.del(fd);
				close(fd);
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
		config_path = "conf/conf.json"; // create
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
			std::cout << "Socket: " << (*i).first << std::endl;
			for (std::vector<VirtualServer *>::const_iterator j = (*i).second.begin(); j != (*i).second.end(); j++)
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
		for (std::vector<VirtualServer *>::const_iterator j = (*i).second.begin(); j != (*i).second.end(); j++)
		{
			delete (*j);
		}
	}
}