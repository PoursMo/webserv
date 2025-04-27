#include "LocationData.hpp"
#include "config.hpp"

#include <stdexcept>
#include <iostream>

LocationData::LocationData() {}

LocationData::LocationData(const ft_json::JsonObject &json_directives)
{
	methods.push_back(GET);
	methods.push_back(POST);
	methods.push_back(DELETE);
	root = "/var/www/html";
	autoIndex = false;
	indexes.push_back("index.html");

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
	methods.clear();
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
		if ((*i).asString().compare("index.html") != 0)
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
