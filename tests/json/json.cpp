#include "config.hpp"
#include "Server.hpp"

#include <iostream>

int main()
{
	try
	{
		ft_json::JsonValue json = ft_json::parse_json(std::cin);
		return 0;
	}
	catch (const std::exception &e)
	{
		std::cerr << e.what() << '\n';
		return 5;
	}
}
