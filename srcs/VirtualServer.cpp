#include "VirtualServer.hpp"
#include "utils.hpp"
#include "ft_json.hpp"
#include "config.hpp"
#include "LocationData.hpp"

#include <stdexcept>
#include <iostream>

#define WS_MAX_CLIENT_MAX_BODY_SIZE 104857600

VirtualServer::VirtualServer(const ft_json::JsonObject &json_directives)
{
	// Default values
	address = "0.0.0.0";
	port = 8000;
	clientMaxBodySize = 1048576; // 1 Mo

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
			else if (!s.compare("server_names"))
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

// ********************************************************************
// Setters using JsonValue
// ********************************************************************

void VirtualServer::setAddress(const std::string &input)
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

void VirtualServer::setPort(int64_t input)
{
	if (input < 1 || input > 65535)
		throw std::runtime_error("Invalid port.");
	port = static_cast<uint16_t>(input);
}

void VirtualServer::setServerNames(const ft_json::JsonArray &input)
{
	for (ft_json::JsonArray::const_iterator i = input.begin(); i != input.end(); i++)
	{
		serverNames.push_back(i->asString());
	}
}

void VirtualServer::setErrorPages(const ft_json::JsonObject &input)
{
	for (ft_json::JsonObject::const_iterator i = input.begin(); i != input.end(); i++)
	{
		errorPages[extract_status_code(i->first.c_str())] = i->second.asString();
	}
}

void VirtualServer::setClientMaxBodySize(int64_t input)
{
	if (input < 1 || input > WS_MAX_CLIENT_MAX_BODY_SIZE)
		throw std::runtime_error("Invalid client_max_body_size.");
	clientMaxBodySize = input;
}

void VirtualServer::setLocations(const ft_json::JsonArray &input)
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

// ********************************************************************
// Getters
// ********************************************************************

const std::string &VirtualServer::getAddress() const
{
	return address;
}

in_port_t VirtualServer::getPort() const
{
	return port;
}

const std::vector<std::string> &VirtualServer::getServerNames() const
{
	return serverNames;
}

const std::map<int, std::string> &VirtualServer::getErrorPages() const
{
	return errorPages;
}

int32_t VirtualServer::getClientMaxBodySize() const
{
	return clientMaxBodySize;
}

// DEBUG
const std::map<std::string, LocationData> &VirtualServer::getLocations() const
{
	return locations;
}

static size_t locationMatchScore(const std::string &path, const std::string &resource)
{
	size_t score = 0;

	while (path[score] && resource[score] && path[score] == resource[score])
		score++;
	if (path[score])
		return (0);
	return score;
}

const LocationData *VirtualServer::getLocation(const std::string &resource) const
{
	const LocationData *bestMatch = NULL;
	size_t matchScore = 0;

	for (std::map<std::string, LocationData>::const_iterator i = locations.begin(); i != locations.end(); i++)
	{
		size_t score = locationMatchScore(i->first, resource);
		if (score > matchScore)
		{
			matchScore = score;
			bestMatch = &i->second;
		}
	}
	return bestMatch;
}

in_addr_t VirtualServer::getAddressAsNum() const
{
	in_addr_t num = 0;
	size_t dot_pos = 0;
	for (int i = 3; i >= 0; i--)
	{
		uint32_t value = std::strtoul(address.c_str() + dot_pos, 0, 10);
		num += value << (8 * i);
		dot_pos = address.find('.', dot_pos) + 1;
	}
	return num;
}

// ********************************************************************
// Debug
// ********************************************************************

std::ostream &operator<<(std::ostream &os, const VirtualServer &server)
{
	os << "  VirtualServer:" << std::endl;
	os << "    Address: " << server.getAddress() << std::endl;
	os << "    Port: " << server.getPort() << std::endl;
	os << "    VirtualServer Names: ";
	for (std::vector<std::string>::const_iterator i = server.getServerNames().begin(); i != server.getServerNames().end(); ++i)
	{
		if (i != server.getServerNames().begin())
			os << ", ";
		os << *i;
	}
	os << std::endl;
	os << "    Error Pages: ";
	for (std::map<int, std::string>::const_iterator i = server.getErrorPages().begin(); i != server.getErrorPages().end(); ++i)
	{
		os << "[" << i->first << ": " << i->second << "] ";
	}
	os << std::endl;
	os << "    Client Max Body Size: " << server.getClientMaxBodySize() << std::endl;
	os << "    Locations:";
	for (std::map<std::string, LocationData>::const_iterator i = server.getLocations().begin(); i != server.getLocations().end(); ++i)
	{
		os << std::endl
		   << "    [" << std::endl
		   << "      Path: " << i->first << std::endl
		   << i->second << "    ]";
	}
	os << std::endl;
	return os;
}