#ifndef SERVER_HPP
#define SERVER_HPP

#include "ft_json.hpp"
#include "LocationData.hpp"

#include <algorithm>
#include <functional>

#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <netdb.h>

class Server
{
private:
	std::string address;
	in_port_t port;
	std::vector<std::string> serverNames;
	std::map<int, std::string> errorPages;
	uint16_t clientMaxBodySize;
	std::map<std::string, LocationData> locations;


	void setAddress(const std::string &input);
	void setPort(int64_t input);
	void setServerNames(const ft_json::JsonArray &input);
	void setErrorPages(const ft_json::JsonObject &input);
	void setClientMaxBodySize(int64_t input);
	void setLocations(const ft_json::JsonArray &input);

public:
	Server(const ft_json::JsonObject &json_directives);

	// Getters
	const std::string &getAddress() const;
	in_port_t getPort() const;
	const std::vector<std::string> &getServerNames() const;
	const std::map<int, std::string> &getErrorPages() const;
	uint16_t getClientMaxBodySize() const;
	const std::map<std::string, LocationData> &getLocations() const;
};

std::ostream &operator<<(std::ostream &os, const Server &server);

#endif