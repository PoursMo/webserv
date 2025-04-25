#include "ft_json.hpp"

#include <algorithm>
#include <functional>

#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <netdb.h>

enum Method
{
	GET,
	POST,
	DELETE
};

struct LocationData
{
	std::vector<Method>			methods;
	// return
	std::string					root;
	bool						autoindex;
	std::vector<std::string>	index;
	std::string					upload_store;
	// cgi
};

class Server
{
private:
	std::string							listenAddress;
	in_port_t							port;
	std::vector<std::string>			serverNames;
	std::map<int, std::string>			errorPages;
	uint16_t							clientMaxBodySize;
	std::map<std::string, LocationData>	locations;

public:
	Server() : clientMaxBodySize(0) {}

	// Setters
	void setListenAddress(const std::string &address)
	{
		listenAddress = address;
	}

	void setPort(const in_port_t &p)
	{
		port = p;
	}

	void addServerName(const std::string &name)
	{
		serverNames.push_back(name);
	}

	void addErrorPage(const int &code, const std::string &path)
	{
		errorPages[code] = path;
	}

	void setClientMaxBodySize(const uint16_t &size)
	{
		clientMaxBodySize = size;
	}

	void addLocation(const std::string &path, const LocationData &location)
	{
		locations[path] = location;
	}

	// Getters
	// const std::string &getListenAddress() const
	// {
	// 	return listenAddress;
	// }

	// const std::string &getServerName() const
	// {
	// 	return serverName;
	// }

	// const std::map<int, std::string> &getErrorPages() const
	// {
	// 	return errorPages;
	// }

	// uint16_t getClientMaxBodySize() const
	// {
	// 	return clientMaxBodySize;
	// }

	// const std::map<std::string, std::string> &getLocations() const
	// {
	// 	return locations;
	// }

	// void initializeSocket() {
	// 	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	// 	if (sockfd < 0) {
	// 		throw std::runtime_error("Failed to create socket");
	// 	}

	// 	struct sockaddr_in serverAddr;
	// 	serverAddr.sin_family = AF_INET;
	// 	serverAddr.sin_port = htons(80); // Default HTTP port
	// 	serverAddr.sin_addr.s_addr = inet_addr(listenAddress.c_str());

	// 	int opt = 1;
	// 	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
	// 		close(sockfd);
	// 		throw std::runtime_error("Failed to set socket options");
	// 	}

	// 	if (bind(sockfd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
	// 		close(sockfd);
	// 		throw std::runtime_error("Failed to bind socket");
	// 	}

	// 	if (listen(sockfd, SOMAXCONN) < 0) {
	// 		close(sockfd);
	// 		throw std::runtime_error("Failed to listen on socket");
	// 	}

	// 	std::cout << "Server initialized and listening on " << listenAddress << ":80" << std::endl;
	// 	close(sockfd); // Close the socket after initialization
	// }
};

void set_listen(Server &server, const std::string &input)
{
	std::string portStr;
	std::string addressStr;
	size_t colonPos = input.find(':');
	addressStr = input.substr(0, colonPos);
	if (colonPos != std::string::npos)
	{
		portStr = input.substr(colonPos + 1);
		if (portStr.empty())
			throw std::runtime_error("Invalid listen directive format. Expected \"host:port\" or \"host\" or \":port\".");
	}
	else
	{
		if (addressStr.empty())
			throw std::runtime_error("Invalid listen directive format. Expected \"host:port\" or \"host\" or \":port\".");
	}
	server.setListenAddress(addressStr);
	server.setPort(static_cast<in_port_t>(std::atoi(portStr.c_str())));
}

void set_server_name(Server &server, const ft_json::JsonArray &input)
{
	for (ft_json::JsonArray::const_iterator it = input.begin(); it != input.end(); it++)
	{
		server.addServerName((*it).asString());
	}
}

void set_error_page(Server &server, const ft_json::JsonObject &input)
{
	for (ft_json::JsonObject::const_iterator it = input.begin(); it != input.end(); it++)
	{
		server.addErrorPage(std::atoi((*it).first.c_str()), (*it).second.asString());
	}
}

void set_client_max_body_size(Server &server, const int64_t &input)
{
	server.setClientMaxBodySize(static_cast<uint16_t>(input));
}

void set_locations(Server &server, const ft_json::JsonArray &input)
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
		LocationData loc;
		for (ft_json::JsonObject::const_iterator it = (*location).asObject().begin(); it != (*location).asObject().end(); it++)
		{
			// setup Location
		}
		server.addLocation(path, loc);
	}
}

void func(ft_json::JsonValue json)
{
	std::vector<Server> servers; // allocate on heap ?
	ft_json::JsonObject &root = json.asObject();
	try
	{
		if (root.size() != 1 || root.at("servers").getType() != ft_json::ARRAY)
			throw std::exception();
	}
	catch (...)
	{
		throw std::runtime_error("Root level must be a single \"servers\" object with an array of objects as it value.");
	}

	ft_json::JsonArray &json_servers = root.at("servers").asArray();
	for (ft_json::JsonArray::const_iterator json_server = json_servers.begin(); json_server != json_servers.end(); json_server++)
	{
		Server server;
		ft_json::JsonObject &json_directives = (*json_server).asObject();
		for (ft_json::JsonObject::const_iterator json_directive = json_directives.begin(); json_directive != json_directives.end(); json_directive++)
		{
			std::string s = (*json_directive).first;
			if (!s.compare("listen"))
				set_listen(server, (*json_directive).second.asString());
			else if (!s.compare("server_name"))
				set_server_name(server, (*json_directive).second.asArray());
			else if (!s.compare("error_page"))
				set_error_page(server, (*json_directive).second.asObject());
			else if (!s.compare("client_max_body_size"))
				set_client_max_body_size(server, (*json_directive).second.asNumber());
			else if (!s.compare("locations"))
				set_locations(server, (*json_directive).second.asArray());
			else
				throw std::runtime_error("Unknown directive: " + s);
		}
		servers.push_back(server);
	}
	// return servers;
}