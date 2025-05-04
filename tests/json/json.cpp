#include "config.hpp"
#include "VirtualServer.hpp"

#include <iostream>
#include <fstream>

int main(int ac, char **av)
{
	try
	{
		if (ac == 1)
		{
			ft_json::JsonValue json = ft_json::parse_json(std::cin);
			return 0;
		}
		std::ifstream file(av[1]);
		if (!file)
			return 1;
		ft_json::JsonValue json = ft_json::parse_json(file);
		return 0;
	}
	catch (const std::exception &e)
	{
		std::cerr << e.what() << '\n';
		return 5;
	}
}
