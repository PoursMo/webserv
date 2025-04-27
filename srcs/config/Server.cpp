#include "Server.hpp"
#include "utils.hpp"
#include "ft_json.hpp"
#include "config.hpp"
#include "LocationData.hpp"

#include <stdexcept>
#include <iostream>

Server::Server(const ft_json::JsonObject &json_directives)
{
	// Default values
	address = "0.0.0.0";
	port = 8000;
	clientMaxBodySize = 1024;
	// each error code is empty = use default string, or make html file for each error

	for (ft_json::JsonObject::const_iterator json_directive = json_directives.begin(); json_directive != json_directives.end(); json_directive++)
	{
		const std::string &s = (*json_directive).first;
		const ft_json::JsonValue &v = (*json_directive).second;
		try
		{
			if (!s.compare("address"))
				setAddress(v.asString());
			else if (!s.compare("port"))
				setPort(v.asNumber());
			else if (!s.compare("server_name"))
				setServerNames(v.asArray());
			else if (!s.compare("error_page"))
				setErrorPages(v.asObject());
			else if (!s.compare("client_max_body_size"))
				setClientMaxBodySize(v.asNumber());
			else if (!s.compare("locations"))
				setLocations(v.asArray());
			else
				throw std::runtime_error("Unknown directive.");
		}
		catch (const std::exception &e)
		{
			throw std::runtime_error(s + ": " + e.what());
		}
	}
}

void Server::setAddress(const std::string &input)
{
	int dot_count = 0;
	std::size_t left = 0;
	std::size_t right = 0;
	while (right != std::string::npos)
	{
		right = input.find('.', left);
		std::string segment = input.substr(left, right - left);
		if (segment.empty() || segment.size() > 3 || (segment.size() > 1 && segment[0] == '0'))
			throw std::runtime_error("Invalid IPv4 address.");
		for (std::string::const_iterator i = segment.begin(); i != segment.end(); i++)
		{
			if (!std::isdigit(*i))
				throw std::runtime_error("Invalid IPv4 address.");
		}
		int value = std::strtol(segment.c_str(), 0, 10);
		if (value < 0 || value > 255)
			throw std::runtime_error("Invalid IPv4 address.");
		left = right + 1;
		++dot_count;
	}
	if (dot_count != 4)
		throw std::runtime_error("Invalid IPv4 address.");
	address = input;
}

void Server::setPort(int64_t input)
{
	if (input < 1 || input > 65535)
		throw std::runtime_error("Invalid port.");
	port = static_cast<uint16_t>(input);
}

void Server::setServerNames(const ft_json::JsonArray &input)
{
	for (ft_json::JsonArray::const_iterator i = input.begin(); i != input.end(); i++)
	{
		serverNames.push_back((*i).asString());
	}
}

void Server::setErrorPages(const ft_json::JsonObject &input)
{
	for (ft_json::JsonObject::const_iterator i = input.begin(); i != input.end(); i++)
	{
		errorPages[extract_status_code((*i).first.c_str())] = (*i).second.asString();
	}
}

void Server::setClientMaxBodySize(int64_t input)
{
	if (input < 1 || input > 65536)
		throw std::runtime_error("Invalid client_max_body_size.");
	clientMaxBodySize = static_cast<uint16_t>(input);
}

void Server::setLocations(const ft_json::JsonArray &input)
{
	for (ft_json::JsonArray::const_iterator location = input.begin(); location != input.end(); location++)
	{
		std::string path;
		try
		{
			path = (*location).asObject().at("path").asString();
		}
		catch (...)
		{
			throw std::runtime_error("Location is missing a path.");
		}
		LocationData location_data((*location).asObject());
		locations[path] = location_data;
	}
}

// Getters
const std::string &Server::getAddress() const
{
	return address;
}

in_port_t Server::getPort() const
{
	return port;
}

const std::vector<std::string> &Server::getServerNames() const
{
	return serverNames;
}

const std::map<int, std::string> &Server::getErrorPages() const
{
	return errorPages;
}

uint16_t Server::getClientMaxBodySize() const
{
	return clientMaxBodySize;
}

const std::map<std::string, LocationData> &Server::getLocations() const
{
	return locations;
}

// for debugging purpose
std::ostream &operator<<(std::ostream &os, const Server &server)
{
	os << "Server:" << std::endl;
	os << "  Address: " << server.getAddress() << std::endl;
	os << "  Port: " << server.getPort() << std::endl;
	os << "  Server Names: ";
	for (std::vector<std::string>::const_iterator i = server.getServerNames().begin(); i != server.getServerNames().end(); ++i)
	{
		if (i != server.getServerNames().begin())
			os << ", ";
		os << *i;
	}
	os << std::endl;
	os << "  Error Pages: ";
	for (std::map<int, std::string>::const_iterator i = server.getErrorPages().begin(); i != server.getErrorPages().end(); ++i)
	{
		os << "[" << i->first << ": " << i->second << "] ";
	}
	os << std::endl;
	os << "  Client Max Body Size: " << server.getClientMaxBodySize() << std::endl;
	os << "  Locations:";
	for (std::map<std::string, LocationData>::const_iterator i = server.getLocations().begin(); i != server.getLocations().end(); ++i)
	{
		os << std::endl
		   << "[" << std::endl
		   << "    Path: " << i->first << std::endl
		   << i->second << "]";
	}
	return os;
}