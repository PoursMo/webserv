#ifndef HTTP_STATUS_HPP
#define HTTP_STATUS_HPP

#include <map>
#include <string>

class http_status
{
private:
	static std::map<int, std::string> statuses;

public:
	static void init();
	static const std::string &get(int code);
};

#endif