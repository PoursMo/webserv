#ifndef URI_HPP
#define URI_HPP

#include "webserv.hpp"

class Request;

class Uri
{
private:
	std::string path;
	std::string query;

public:
	Uri(const Request &request);

	const std::string &getPath() const;
	const std::string &getQuery() const;

	static std::string decode(const std::string &src);
	static std::string encode(const std::string &src);
};

#endif
