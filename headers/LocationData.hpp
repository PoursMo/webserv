#ifndef LOCATIONDATA_HPP
#define LOCATIONDATA_HPP

#include "ft_json.hpp"

enum Method
{
	GET,
	POST,
	DELETE
};

class LocationData
{
private:
	std::vector<Method> methods;
	std::pair<int, std::string> returnPair;
	std::vector<std::string> indexes;
	bool autoIndex;
	std::string root;
	std::string uploadStore;
	std::map<std::string, std::string> cgiConfig;

	void setMethods(const ft_json::JsonArray &input);
	void setReturnPair(const ft_json::JsonObject &input);
	void setIndexes(const ft_json::JsonArray &input);
	void setCgiConfig(const ft_json::JsonObject &input);

public:
	LocationData(const ft_json::JsonObject &json_directives);
	LocationData();

	// Getters
	const std::vector<Method> &getMethods() const;
	const std::pair<int, std::string> &getReturnPair() const;
	const std::vector<std::string> getIndexes() const;
	bool getAutoIndex() const;
	const std::string &getUploadStore() const;
	const std::string &getRoot() const;
	const std::map<std::string, std::string> &getCgiConfig() const;
};

std::ostream &operator<<(std::ostream &os, const LocationData &locationData);

#endif