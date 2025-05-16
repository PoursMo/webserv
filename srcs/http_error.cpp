#include "http_error.hpp"
#include "http_status.hpp"
#include "utils.hpp"

std::string generateErrorBody(int status)
{
	std::stringstream body;
	const std::string &statusName = http_status::get(status);

	body << "<!DOCTYPE header>" << CRLF;
	body << "<header lang=\"en\">" << CRLF;
	body << "<head>" << CRLF;
	body << "<meta charset=\"UTF-8\">" << CRLF;
	body << "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">" << CRLF;
	body << "<title>" << status << ": " << statusName << "</title>" << CRLF;
	body << "</head>" << CRLF;
	body << "<body>" << CRLF;
	body << "<div>" << CRLF;
	body << "<h1>" << status << ": " << statusName << "</h1>" << CRLF;
	body << "</div>" << CRLF;
	body << "</body>" << CRLF;

	return body.str();
}

http_error::http_error(const char *ainfo, int status_code)
	: statusCode(status_code),
	  additionalInfo(ainfo)
{
	whatMessage = int_to_str(statusCode) + " " + http_status::get(statusCode);
	if (!additionalInfo.empty())
		whatMessage.append(": " + additionalInfo);
}

http_error::http_error(const std::string &ainfo, int status_code)
	: statusCode(status_code),
	  additionalInfo(ainfo)
{
	whatMessage = int_to_str(statusCode) + " " + http_status::get(statusCode);
	if (!additionalInfo.empty())
		whatMessage.append(": " + additionalInfo);
}

http_error::http_error(int status_code)
	: statusCode(status_code)
{
	whatMessage = int_to_str(statusCode) + " " + http_status::get(statusCode);
}

http_error::~http_error() throw()
{
}

const char *http_error::what() const throw()
{
	return whatMessage.c_str();
}

int http_error::getStatusCode() const
{
	return statusCode;
}
