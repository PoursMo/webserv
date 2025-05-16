#ifndef VIRTUALSERVER_HPP
#define VIRTUALSERVER_HPP

#include "webserv.hpp"
#include "ft_json.hpp"
#include "LocationData.hpp"

class VirtualServer
{
private:
	std::string address;
	in_port_t port;
	std::vector<std::string> serverNames;
	std::map<int, std::string> errorPages;
	int32_t clientMaxBodySize;
	std::map<std::string, LocationData> locations;

	void setAddress(const std::string &input);
	void setPort(int64_t input);
	void setServerNames(const ft_json::JsonArray &input);
	void setErrorPages(const ft_json::JsonObject &input);
	void setClientMaxBodySize(int64_t input);
	void setLocations(const ft_json::JsonArray &input);

public:
	VirtualServer(const ft_json::JsonObject &json_directives);

	// Getters
	const std::string &getAddress() const;
	in_addr_t getAddressAsNum() const;
	in_port_t getPort() const;
	const std::vector<std::string> &getServerNames() const;
	const std::map<int, std::string> &getErrorPages() const;
	int32_t getClientMaxBodySize() const;
	const std::map<std::string, LocationData> &getLocations() const;
	const LocationData *getLocation(const std::string &resource) const;
};

std::ostream &operator<<(std::ostream &os, const VirtualServer &server);

#endif