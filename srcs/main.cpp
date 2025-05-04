#include "config.hpp"
#include "VirtualServer.hpp"
#include "Poll.hpp"

#include <iostream>
#include <fstream>
#include <algorithm>

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

	poll_loop();

	for (std::map<int, std::vector<VirtualServer *> >::iterator i = servers.begin(); i != servers.end(); i++)
	{
		for (std::vector<VirtualServer *>::const_iterator j = (*i).second.begin(); j != (*i).second.end(); j++)
		{
			delete (*j);
		}
	}
}