#ifndef HTTP_ERROR_HPP
#define HTTP_ERROR_HPP

#include <stdexcept>
#include <map>

class http_error : public std::exception
{
private:
	int statusCode;
	std::string additionalInfo;
	std::string whatMessage;

public:
	static std::map<int, std::string> errors;

	http_error(const char *ainfo, int status_code);
	http_error(const std::string &ainfo, int status_code);
	http_error(int status_code);
	~http_error() throw();
	const char *what() const throw();
	int getStatusCode() const;
};

std::string generateErrorResponse(int status);

#endif