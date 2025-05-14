#include "config.hpp"
#include "VirtualServer.hpp"

#include <iostream>
#include <fstream>
#include <string>
#include <unistd.h>

int main(int argc, char **argv)
{
	std::string config_path = "conf/default.json";
	std::map<int, std::vector<VirtualServer *> > servers;
	if (argc > 1)
		config_path = argv[1];
	std::ifstream file(config_path.c_str());
	if (!file)
	{
		std::cerr << config_path << ": Could not open file " << std::endl;
		return 1;
	}
	try
	{
		ft_json::JsonValue json = ft_json::parse_json(file);
		create_servers(json, servers);
		for (std::map<int, std::vector<VirtualServer *> >::iterator i = servers.begin(); i != servers.end(); i++)
		{
			for (std::vector<VirtualServer *>::const_iterator j = i->second.begin(); j != i->second.end(); j++)
			{
				delete (*j);
			}
			close(i->first);
		}
	}
	catch (const std::exception &e)
	{
		std::cerr << e.what() << '\n';
		return 1;
	}
}