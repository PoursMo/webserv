#include "config.hpp"
#include "Server.hpp"

#include <iostream>
#include <fstream>

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
	try
	{
		ft_json::JsonValue parsed = ft_json::parse_json(file);
		// ft_json::print_json_value(parsed);
		std::vector<Server> servers = create_servers(parsed);
		std::cout << servers[0] << std::endl;
	}
	catch (const std::exception &e)
	{
		std::cerr << e.what() << '\n';
	}
}