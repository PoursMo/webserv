#include "http_error.hpp"
#include "utils.hpp"

#include <algorithm>
#include <iostream>
#include <sstream>

std::map<int, std::string> http_error::errors;

static void init_errors()
{
	http_error::errors[400] = "Bad Request";
	http_error::errors[401] = "Unauthorized";
	http_error::errors[402] = "Payment Required";
	http_error::errors[403] = "Forbidden";
	http_error::errors[404] = "Not Found";
	http_error::errors[405] = "Method Not Allowed";
	http_error::errors[406] = "Not Acceptable";
	http_error::errors[407] = "Proxy Authentication Required";
	http_error::errors[408] = "Request Timeout";
	http_error::errors[409] = "Conflict";
	http_error::errors[410] = "Gone";
	http_error::errors[411] = "Length Required";
	http_error::errors[412] = "Precondition Failed";
	http_error::errors[413] = "Payload Too Large";
	http_error::errors[414] = "URI Too Long";
	http_error::errors[415] = "Unsupported Media Type";
	http_error::errors[416] = "Range Not Satisfiable";
	http_error::errors[417] = "Expectation Failed";
	http_error::errors[418] = "Im A Teapot";
	http_error::errors[421] = "Misdirected Request";
	http_error::errors[422] = "Unprocessable Entity";
	http_error::errors[423] = "Locked";
	http_error::errors[424] = "Failed Dependency";
	http_error::errors[425] = "Too Early";
	http_error::errors[426] = "Upgrade Required";
	http_error::errors[428] = "Precondition Required";
	http_error::errors[429] = "Too Many Requests";
	http_error::errors[431] = "Request Header Fields Too Large";
	http_error::errors[451] = "Unavailable For Legal Reasons";
	http_error::errors[500] = "Internal Server Error";
	http_error::errors[501] = "Not Implemented";
	http_error::errors[502] = "Bad Gateway";
	http_error::errors[503] = "Service Unavailable";
	http_error::errors[504] = "Gateway Timeout";
	http_error::errors[505] = "HTTP Version Not Supported";
	http_error::errors[506] = "Variant Also Negotiates";
	http_error::errors[507] = "Insufficient Storage";
	http_error::errors[508] = "Loop Detected";
	http_error::errors[509] = "Bandwidth Limit Exceeded";
	http_error::errors[510] = "Not Extended";
	http_error::errors[511] = "Network Authentication Required";
}

std::string generateErrorPage(int status)
{
	std::stringstream html;

	if (http_error::errors.empty())
		init_errors();
	std::string errorName = "Unknown error";
	if (http_error::errors.count(status))
		errorName = http_error::errors.at(status);

	html << "<!DOCTYPE html>";
	html << "<html lang=\"en\">";
	html << "<head>";
	html << "<meta charset=\"UTF-8\">";
	html << "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">";
	html << "<title>" << status << ": " << errorName << "</title>";
	html << "</head>";
	html << "</body>";
	html << "<div>";
	html << "<title>" << status << ": " << errorName << "</title>";
	html << "</div>";
	return html.str();
}

http_error::http_error(const char *ainfo, int status_code)
	: statusCode(status_code),
	  additionalInfo(ainfo)
{
	if (errors.empty())
		init_errors();
	whatMessage = int_to_str(statusCode) + " " + errors.at(statusCode);
	if (!additionalInfo.empty())
		whatMessage.append(": " + additionalInfo);
}

http_error::http_error(const std::string &ainfo, int status_code)
	: statusCode(status_code),
	  additionalInfo(ainfo)
{
	if (errors.empty())
		init_errors();
	whatMessage = int_to_str(statusCode) + " " + errors.at(statusCode);
	if (!additionalInfo.empty())
		whatMessage.append(": " + additionalInfo);
}

http_error::http_error(int status_code)
	: statusCode(status_code)
{
	if (errors.empty())
		init_errors();
	whatMessage = int_to_str(statusCode) + " " + errors.at(statusCode);
}

http_error::~http_error() throw()
{
}

const char *http_error::what() const throw()
{
	return whatMessage.c_str();
}
