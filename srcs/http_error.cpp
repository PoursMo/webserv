#include "http_error.hpp"
#include "utils.hpp"
#include <algorithm>
#include <iostream>

std::map<int, std::string> http_error::errors;

static void init_errors()
{
	http_error::errors[400] = "Bad Request";
	http_error::errors[404] = "Not Found";
	http_error::errors[500] = "Internal Server Error";
}

http_error::http_error(const char *ainfo, int status_code)
	: statusCode(status_code),
	  additionalInfo(ainfo)
{
	if (errors.empty())
		init_errors();
	std::string serror = errors.at(statusCode);
	whatMessage = int_to_str(statusCode) + " " + serror;
	if (!additionalInfo.empty())
		whatMessage.append(": " + additionalInfo);
}

http_error::http_error(const std::string &ainfo, int status_code)
	: statusCode(status_code),
	  additionalInfo(ainfo)
{
	if (errors.empty())
		init_errors();
	std::string serror = errors.at(statusCode);
	whatMessage = int_to_str(statusCode) + " " + serror;
	if (!additionalInfo.empty())
		whatMessage.append(": " + additionalInfo);
}

http_error::http_error(int status_code)
	: statusCode(status_code)
{
	if (errors.empty())
		init_errors();
	std::string serror = errors.at(statusCode);
	whatMessage = int_to_str(statusCode) + " " + serror;
}

http_error::~http_error() throw()
{
}

const char *http_error::what() const throw()
{
	return whatMessage.c_str();
}
