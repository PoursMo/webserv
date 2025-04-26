#include "Server.hpp"

// ********************************************************************
// LocationData
// ********************************************************************

LocationData::LocationData() {}

LocationData::LocationData(const ft_json::JsonObject &json_directives)
{
	for (ft_json::JsonObject::const_iterator json_directive = json_directives.begin(); json_directive != json_directives.end(); json_directive++)
	{
		const std::string &s = (*json_directive).first;
		const ft_json::JsonValue &v = (*json_directive).second;
		if (!s.compare("path"))
			continue;
		else if (!s.compare("method"))
			setMethods(v.asArray());
		else if (!s.compare("return"))
			setReturnPair(v.asObject());
		else if (!s.compare("root"))
			setRoot(v.asString());
		else if (!s.compare("autoindex"))
			setAutoIndex(v.asBoolean());
		else if (!s.compare("index"))
			setIndexes(v.asArray());
		else if (!s.compare("upload_store"))
			setUploadStore(v.asString());
		// cgi
		else
			throw std::runtime_error("Unknown directive: " + s);
	}
}

void LocationData::setMethods(const ft_json::JsonArray &input)
{
	for (ft_json::JsonArray::const_iterator it = input.begin(); it != input.end(); it++)
	{
		const std::string &s = (*it).asString();
		Method m;
		if (s.compare("GET"))
			m = GET;
		else if (s.compare("POST"))
			m = POST;
		else if (s.compare("DELETE"))
			m = DELETE;
		else
			throw std::runtime_error("Unknown method: " + s);
		methods.push_back(m);
	}
}

// nginx checks that number is only digits (no minus), and isnt above long max
// code must be between 300 and 599
void LocationData::setReturnPair(const ft_json::JsonObject &input)
{
	if (input.size() != 1)
		throw std::runtime_error("Multiple returns detected for a location.");
	returnPair.first = std::atoi((*input.begin()).first.c_str());
	returnPair.second = (*input.begin()).second.asString();
}

void LocationData::setRoot(const std::string &input)
{
	root = input;
}

void LocationData::setAutoIndex(bool input)
{
	autoIndex = input;
}

void LocationData::setIndexes(const ft_json::JsonArray &input)
{
	for (ft_json::JsonArray::const_iterator it = input.begin(); it != input.end(); it++)
	{
		indexes.push_back((*it).asString());
	}
}

void LocationData::setUploadStore(const std::string &input)
{
	uploadStore = input;
}

// Getters
const std::vector<Method> &LocationData::getMethods() const
{
	return methods;
}

const std::pair<int, std::string> &LocationData::getReturnPair() const
{
	return returnPair;
}

const std::string &LocationData::getRoot() const
{
	return root;
}

bool LocationData::getAutoIndex() const
{
	return autoIndex;
}

const std::vector<std::string> LocationData::getIndexes() const
{
	return indexes;
}

const std::string &LocationData::getUploadStore() const
{
	return uploadStore;
}

// for debugging purpose
std::ostream &operator<<(std::ostream &os, const LocationData &locationData)
{
	os << "    Methods: [";
	const std::vector<Method> &methods = locationData.getMethods();
	for (std::vector<Method>::const_iterator it = methods.begin(); it != methods.end(); ++it)
	{
		if (it != methods.begin())
			os << ", ";
		os << *it;
	}
	os << "]" << std::endl;
	os << "    ReturnPair: (" << locationData.getReturnPair().first << ", \"" << locationData.getReturnPair().second << "\")" << std::endl;
	os << "    Root: \"" << locationData.getRoot() << "\"" << std::endl;
	os << "    AutoIndex: " << (locationData.getAutoIndex() ? "true" : "false") << std::endl;
	os << "    Indexes: [";
	const std::vector<std::string> &indexes = locationData.getIndexes();
	for (std::vector<std::string>::const_iterator it = indexes.begin(); it != indexes.end(); ++it)
	{
		if (it != indexes.begin())
			os << ", ";
		os << "\"" << *it << "\"";
	}
	os << "]" << std::endl;
	os << "    UploadStore: \"" << locationData.getUploadStore() << "\"" << std::endl;
	return os;
}

// ********************************************************************
// Server
// ********************************************************************

Server::Server(const ft_json::JsonObject &json_directives)
{
	for (ft_json::JsonObject::const_iterator json_directive = json_directives.begin(); json_directive != json_directives.end(); json_directive++)
	{
		const std::string &s = (*json_directive).first;
		const ft_json::JsonValue &v = (*json_directive).second;
		if (!s.compare("listen"))
			setListen(v.asString());
		else if (!s.compare("server_name"))
			setServerNames(v.asArray());
		else if (!s.compare("error_page"))
			setErrorPages(v.asObject());
		else if (!s.compare("client_max_body_size"))
			setClientMaxBodySize(v.asNumber());
		else if (!s.compare("locations"))
			setLocations(v.asArray());
		else
			throw std::runtime_error("Unknown directive: " + s);
	}
}

void Server::setListen(const std::string &input)
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
	listenAddress = addressStr;
	port = static_cast<in_port_t>(std::atoi(portStr.c_str()));
}

void Server::setServerNames(const ft_json::JsonArray &input)
{
	for (ft_json::JsonArray::const_iterator it = input.begin(); it != input.end(); it++)
	{
		serverNames.push_back((*it).asString());
	}
}

// nginx checks that number is only digits (no minus), and isnt above long max
// code must be between 300 and 599
void Server::setErrorPages(const ft_json::JsonObject &input)
{
	for (ft_json::JsonObject::const_iterator it = input.begin(); it != input.end(); it++)
	{
		errorPages[std::atoi((*it).first.c_str())] = (*it).second.asString();
	}
}

// check for < 0 || > 65536 ?
void Server::setClientMaxBodySize(int64_t input)
{
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
const std::string &Server::getListenAddress() const
{
	return listenAddress;
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
	os << "  Listen Address: " << server.getListenAddress() << std::endl;
	os << "  Port: " << server.getPort() << std::endl;
	os << "  Server Names: ";
	for (std::vector<std::string>::const_iterator it = server.getServerNames().begin(); it != server.getServerNames().end(); ++it)
	{
		if (it != server.getServerNames().begin())
			os << ", ";
		os << *it;
	}
	os << std::endl;
	os << "  Error Pages: ";
	for (std::map<int, std::string>::const_iterator it = server.getErrorPages().begin(); it != server.getErrorPages().end(); ++it)
	{
		os << "[" << it->first << ": " << it->second << "] ";
	}
	os << std::endl;
	os << "  Client Max Body Size: " << server.getClientMaxBodySize() << std::endl;
	os << "  Locations:";
	for (std::map<std::string, LocationData>::const_iterator it = server.getLocations().begin(); it != server.getLocations().end(); ++it)
	{
		os << std::endl << "[" << std::endl << "    Path: " << it->first << std::endl << it->second << "]";
	}
	return os;
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
		throw std::runtime_error("Root level must be a single \"servers\" object with an array of objects as it value.");
	}

	const ft_json::JsonArray &json_servers = root.at("servers").asArray();
	for (ft_json::JsonArray::const_iterator json_server = json_servers.begin(); json_server != json_servers.end(); json_server++)
	{
		Server server((*json_server).asObject());
		servers.push_back(server);
	}
	return servers;
}