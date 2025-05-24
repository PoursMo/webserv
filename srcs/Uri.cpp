#include "Uri.hpp"
#include "Request.hpp"
#include "utils.hpp"

Uri::Uri(const Request &request)
	: path(request.getTarget()),
	  query("")
{
	std::size_t query_index = path.find("?");

	if (query_index != std::string::npos)
	{
		this->query = Uri::decode(path.substr(query_index + 1));
		this->path = path.substr(0, query_index);
	}
	this->path = Uri::decode(this->path);
}

const std::string &Uri::getPath() const
{
	return this->path;
}

const std::string &Uri::getQuery() const
{
	return this->query;
}

std::string Uri::decode(const std::string &src)
{
	std::stringstream decoded;

	for (std::size_t i = 0; i < src.size(); i++)
	{
		if (src[i] != '%' || i + 3 > src.size())
		{
			decoded << src[i];
			continue;
		}
		decoded << (char)str_to_int(src.substr(i + 1, 2));
		i += 2;
	}
	return decoded.str();
}

std::string Uri::encode(const std::string &src)
{
	static std::string set = ":/?#[]@!$&'()*+,;=%";
	std::stringstream encoded;

	for (std::size_t i = 0; i < src.size(); i++)
	{
		if (set.find(src[i]) == std::string::npos)
		{
			encoded << src[i];
			continue;
		}
		encoded << '%' << int_to_str(src[i], "0123456789ABCDEF");
	}
	return encoded.str();
}
