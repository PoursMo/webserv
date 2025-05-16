#include "config.hpp"
#include "VirtualServer.hpp"
#include "Poller.hpp"
#include "Receiver.hpp"
#include "http_status.hpp"

#include <iostream>
#include <fstream>
#include <algorithm>
#include <string.h>
#include <unistd.h>

bool init(const char *config_path, std::map<int, std::vector<VirtualServer *> > &servers)
{
	std::ifstream file(config_path);
	if (!file)
	{
		std::cerr << config_path << ": Could not open file " << std::endl;
		return false;
	}
	try
	{
		std::cout << "\033[1m\033[31m" << "json:" << "\033[0m" << std::endl;
		ft_json::JsonValue json = ft_json::parse_json(file);
		std::cout << json << std::endl
				  << std::endl;
		std::cout << "\033[1m\033[31m" << "servers:" << "\033[0m" << std::endl;
		create_servers(json, servers);
		for (std::map<int, std::vector<VirtualServer *> >::iterator i = servers.begin(); i != servers.end(); i++)
		{
			std::cout << "Socket: " << i->first << std::endl;
			for (std::vector<VirtualServer *>::const_iterator j = i->second.begin(); j != i->second.end(); j++)
			{
				std::cout << *(*j);
			}
		}
		std::cout << std::endl;
		http_status::init();
	}
	catch (const std::exception &e)
	{
		std::cerr << e.what() << '\n';
		return false;
	}
	return true;
}

int main(int argc, char **argv)
{
	const char *config_path = "conf/default.json";
	if (argc > 1)
		config_path = argv[1];
	std::map<int, std::vector<VirtualServer *> > servers;
	if (!init(config_path, servers))
		return 1;

	std::cout << "\033[1m\033[31m" << "poll loop:" << "\033[0m" << std::endl;
	Poller poller(servers);
	poller.loop();

	// this is never reached
	for (std::map<int, std::vector<VirtualServer *> >::iterator i = servers.begin(); i != servers.end(); i++)
	{
		for (std::vector<VirtualServer *>::const_iterator j = i->second.begin(); j != i->second.end(); j++)
		{
			delete (*j);
		}
	}
}