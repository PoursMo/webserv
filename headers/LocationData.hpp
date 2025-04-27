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
	std::string root;
	bool autoIndex;
	std::vector<std::string> indexes;
	std::string uploadStore;
	// cgi


	void setMethods(const ft_json::JsonArray &input);
	void setReturnPair(const ft_json::JsonObject &input);
	void setRoot(const std::string &input);
	void setAutoIndex(bool input);
	void setIndexes(const ft_json::JsonArray &input);
	void setUploadStore(const std::string &input);
	// setCgi

public:
	// default constructor with everything default then delete when calling set function ?
	LocationData(const ft_json::JsonObject &json_directives);
	LocationData();

	// Getters
	const std::vector<Method> &getMethods() const;
	const std::pair<int, std::string> &getReturnPair() const;
	const std::string &getRoot() const;
	bool getAutoIndex() const;
	const std::vector<std::string> getIndexes() const;
	const std::string &getUploadStore() const;
	// getcgi
};

std::ostream &operator<<(std::ostream &os, const LocationData &locationData);

#endif