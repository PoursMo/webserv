#include "config.hpp"
#include "VirtualServer.hpp"

#include <iostream>
#include <fstream>
#include <string>

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
		servers = create_servers(json);
	}
	catch (const std::exception &e)
	{
		std::cerr << e.what() << '\n';
		return 1;
	}
}