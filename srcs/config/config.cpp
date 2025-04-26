#include "Server.hpp"

// ********************************************************************
// Utils
// ********************************************************************

static int extract_status_code(const std::string &s)
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
// LocationData
// ********************************************************************

LocationData::LocationData() {}

LocationData::LocationData(const ft_json::JsonObject &json_directives)
{
	for (ft_json::JsonObject::const_iterator json_directive = json_directives.begin(); json_directive != json_directives.end(); json_directive++)
	{
		const std::string &s = (*json_directive).first;
		const ft_json::JsonValue &v = (*json_directive).second;
		try
		{
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
				throw std::runtime_error("Unknown directive.");
		}
		catch (const std::exception &e)
		{
			throw std::runtime_error(s + ": " + e.what());
		}
	}
}

void LocationData::setMethods(const ft_json::JsonArray &input)
{
	for (ft_json::JsonArray::const_iterator i = input.begin(); i != input.end(); i++)
	{
		const std::string &s = (*i).asString();
		if (!s.compare("GET"))
			methods.push_back(GET);
		else if (!s.compare("POST"))
			methods.push_back(POST);
		else if (!s.compare("DELETE"))
			methods.push_back(DELETE);
		else
			throw std::runtime_error("Unknown method: " + s);
	}
}

void LocationData::setReturnPair(const ft_json::JsonObject &input)
{
	if (input.size() != 1)
		throw std::runtime_error("Multiple returns detected for a location.");
	returnPair.first = extract_status_code((*input.begin()).first.c_str());
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
	for (ft_json::JsonArray::const_iterator i = input.begin(); i != input.end(); i++)
	{
		indexes.push_back((*i).asString());
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
	for (std::vector<Method>::const_iterator i = methods.begin(); i != methods.end(); ++i)
	{
		if (i != methods.begin())
			os << ", ";
		switch (*i)
		{
		case GET:
			os << "GET";
			break;
		case POST:
			os << "POST";
			break;
		case DELETE:
			os << "DELETE";
			break;
		default:
			break;
		}
	}
	os << "]" << std::endl;
	os << "    ReturnPair: (" << locationData.getReturnPair().first << ", \"" << locationData.getReturnPair().second << "\")" << std::endl;
	os << "    Root: \"" << locationData.getRoot() << "\"" << std::endl;
	os << "    AutoIndex: " << (locationData.getAutoIndex() ? "true" : "false") << std::endl;
	os << "    Indexes: [";
	const std::vector<std::string> &indexes = locationData.getIndexes();
	for (std::vector<std::string>::const_iterator i = indexes.begin(); i != indexes.end(); ++i)
	{
		if (i != indexes.begin())
			os << ", ";
		os << "\"" << *i << "\"";
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

// perform checks
void Server::setAddress(const std::string &input)
{
	address = input;
}

void Server::setPort(int64_t input)
{
	std::cout << input << std::endl;
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