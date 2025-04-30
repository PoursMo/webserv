#include "Server.hpp"
#include "config.hpp"
#include "utils.hpp"

#include <stdexcept>
#include <iostream>

static bool is_duplicate(const std::vector<Server *> &servers, const Server *new_server)
{
	for (std::vector<Server *>::const_iterator i = servers.begin(); i != servers.end(); i++)
	{
		if ((*i)->getAddress() == new_server->getAddress() && (*i)->getPort() == new_server->getPort())
			return true;
	}
	return false;
}

std::vector<Server *> create_servers(const ft_json::JsonValue &json)
{
	std::vector<Server *> servers;
	const ft_json::JsonObject &root = json.asObject();
	try
	{
		if (root.size() != 1 || root.at("servers").getType() != ft_json::ARRAY)
			throw std::exception();
	}
	catch (...)
	{
		throw std::runtime_error("Root level must be a single \"servers\" object with an array of objects as it's value.");
	}

	int count = 1;
	const ft_json::JsonArray &json_servers = root.at("servers").asArray();
	for (ft_json::JsonArray::const_iterator json_server = json_servers.begin(); json_server != json_servers.end(); json_server++)
	{
		try
		{
			Server *server = new Server((*json_server).asObject());
			if (is_duplicate(servers, server))
			{
				std::cout << "warning: duplicate host:port detected on " << server->getAddress() << ":" << server->getPort() << ", any duplicate will be ignored" << std::endl;
				delete server;
				continue;
			}
			servers.push_back(server);
		}
		catch (const std::exception &e)
		{
			for (std::vector<Server *>::iterator i = servers.begin(); i != servers.end(); i++)
			{
				delete *i;
			}
			throw std::runtime_error("server " + int_to_str(count) + ": " + e.what());
		}
		count++;
	}
	if (count == 1)
		throw std::runtime_error("Atleast 1 server is required.");
	return servers;
}