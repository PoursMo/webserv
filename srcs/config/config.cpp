#include "Server.hpp"
#include "config.hpp"
#include "utils.hpp"

#include <stdexcept>

// ********************************************************************
// Utils
// ********************************************************************

int extract_status_code(const std::string &s)
{
	for (std::string::const_iterator i = s.begin(); i != s.end(); i++)
	{
		if (!std::isdigit(*i))
			throw std::runtime_error("Invalid status code.");
	}
	char *end;
	unsigned long code = std::strtoul(s.c_str(), &end, 10);
	if (*end != '\0' || code < 300 || code > 599)
		throw std::runtime_error("Invalid status code.");
	return code;
}

// ********************************************************************
//
// ********************************************************************

std::vector<Server> create_servers(const ft_json::JsonValue &json)
{
	std::vector<Server> servers; // allocate on heap ?
	const ft_json::JsonObject &root = json.asObject();
	try
	{
		if (root.size() != 1 || root.at("servers").getType() != ft_json::ARRAY)
			throw std::exception();
	}
	catch (...)
	{
		throw std::runtime_error("Root level must be a single \"servers\" object with an array of objects as i value.");
	}

	int count = 1;
	const ft_json::JsonArray &json_servers = root.at("servers").asArray();
	for (ft_json::JsonArray::const_iterator json_server = json_servers.begin(); json_server != json_servers.end(); json_server++)
	{
		try
		{
			Server server((*json_server).asObject());
			servers.push_back(server);
		}
		catch (const std::exception &e)
		{
			throw std::runtime_error("server " + int_to_str(count) + ": " + e.what());
		}
		count++;
	}
	return servers;
}